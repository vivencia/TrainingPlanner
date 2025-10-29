#include "dbusermodel.h"

#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
#include "tpdatabasetable.h"
#include "tpimage.h"
#include "tputils.h"
#include "translationclass.h"
#include "tpsettings.h"
#include "online_services/onlineuserinfo.h"
#include "online_services/tpmessage.h"
#include "online_services/tpmessagesmanager.h"
#include "online_services/tponlineservices.h"
#include "tpkeychain/tpkeychain.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QQuickWindow>
#include <QTimer>

#ifndef Q_OS_ANDROID
#include "pageslistmodel.h"
#include <QThread>
#endif

#include <utility>

DBUserModel *DBUserModel::_appUserModel(nullptr);

static const QLatin1StringView& userProfileFileName{"profile.txt"_L1};
static const QLatin1StringView& userLocalDataFileName{"user.data"_L1};
static const QString &tpNetworkTitle{qApp->tr("TP Network")};

static inline QString profileFileName(const QString &dir, const QString &userid) { return dir + userid + ".txt"_L1; }

#ifndef QT_NO_DEBUG
#define POLLING_INTERVAL 60*1000 //When testing, poll more frequently
#else
#define POLLING_INTERVAL 20*60*1000
#endif

//A non-confirmed user both has appUseMode set to APP_USE_MODE_PENDING_CLIENT and
//appended to their name an additional string containing the not allowed char '!'
static inline QString userNameWithoutConfirmationWarning(const QString &userName)
{
	const qsizetype sep_idx{userName.indexOf('!')};
	return userName.left(sep_idx-1);
}

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: QObject{parent}, m_tempRow{-1}, n_cmdOrderValue{-1}, m_availableCoaches{nullptr}, m_pendingClientRequests{nullptr},
		m_pendingCoachesResponses{nullptr}, m_tempUserInfo{nullptr}, m_mainTimer{nullptr}
#ifndef Q_OS_ANDROID
	,m_allUsers{nullptr}
#endif
{
	if (bMainUserModel)
	{
		_appUserModel = this;
		mb_MainUserInfoChanged = false;
		connect(this, &DBUserModel::userModified, this, [this] (const uint user_idx, const uint field) {
			if (field != USER_MODIFIED_CREATED)
			{
				if (user_idx == 0)
				{
					mb_MainUserInfoChanged = true;
					if (field == USER_COL_APP_USE_MODE)
						emit appUseModeChanged();
				}
				else
				{
					if (field == USER_MODIFIED_REMOVED)
						appDBInterface()->removeUser(user_idx);
				}
				appDBInterface()->saveUser(user_idx);
			}
		});

		connect(this, &DBUserModel::coachesNamesChanged, this, [this] () {
			emit coachesAndClientsNamesChanged();
		});
		connect(this, &DBUserModel::clientsNamesChanged, this, [this] () {
			emit coachesAndClientsNamesChanged();
		});

		mb_userRegistered = std::nullopt;
		mb_canConnectToServer = appOsInterface()->tpServerOK();
		if (mb_canConnectToServer)
			onlineCheckIn();
#ifdef Q_OS_ANDROID
		connect(appOsInterface(), &OSInterface::internetStatusChanged, this, [this] (const bool connected) {
			if (!connected)
			{
				mb_canConnectToServer = false;
				emit canConnectToServerChanged();
			}
		});
#endif
		connect(appOsInterface(), &OSInterface::serverStatusChanged, this, [this] (const bool online) {
			if (!mb_canConnectToServer && online)
				onlineCheckIn();
			mb_canConnectToServer = online;
			emit canConnectToServerChanged();
		});

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
			if (m_usersData.count() > 0)
				setPhoneBasedOnLocale();
			emit labelsChanged();
		});
		/*connect(appKeyChain(), &TPKeyChain::keyRestored, this, [&] (const QString &key, const QString &value) {
			qDebug() << "Read key:";
			qDebug() << "key: " << key;
			qDebug() << "password: " << value;
			qDebug() << "";
		});
		connect(appKeyChain(), &TPKeyChain::keyStored, this, [&] (const QString &key) {
			qDebug() << "Write key:";
			qDebug() << "key: " << key;
			qDebug() << "";
		});*/
		//appKeyChain()->writeKey("1739296696780", "userpassword");
		//appKeyChain()->readKey("1739296696780");
		//appKeyChain()->writeKey("test", "testpassword");
		//appKeyChain()->readKey("test");
	}
}

void DBUserModel::showFirstTimeUseDialog()
{
	QMetaObject::invokeMethod(appMainWindow(), "showFirstTimeUseDialog");
	connect(this, &DBUserModel::mainUserConfigurationFinished, this, [this] () {
		appOsInterface()->initialCheck();
	}, Qt::SingleShotConnection);
}

void DBUserModel::setOnlineAccount(const bool online_user, const uint user_idx)
{
	if (user_idx == 0 && mainUserConfigured())
	{
		if (onlineAccount(user_idx) && !online_user)
		{
			connect(appMainWindow(), SIGNAL(unregisterUser(bool)), this, SLOT(slot_unregisterUser(bool)), Qt::SingleShotConnection);
			QString message{tr("If you remove your online account you'll not be able to log onto it anymore from any device.")};
			if (isCoach(0))
				message = std::move(tr("You'll not have access to your online client(s) anymore."));
			if (isClient(0))
				message += std::move(tr("You'll not have access to your online coache(s) anymore."));
			QMetaObject::invokeMethod(appMainWindow(), "showUnregisterUserDialog",
				Q_ARG(QString, tr("Remove online account?")), Q_ARG(QString, message));
			setCoachPublicStatus(false);
		}
		else if (!onlineAccount(user_idx) && online_user)
			onlineCheckIn();
	}
	emit onlineUserChanged();
	m_usersData[0][USER_COL_ONLINEACCOUNT] = online_user ? '1' : '0';
	emit userModified(0, USER_COL_ONLINEACCOUNT);
}

void DBUserModel::addUser(QStringList &&user_info)
{
	m_usersData.append(std::move(user_info));
	const qsizetype last_idx{m_usersData.count() - 1};
	if (last_idx > 0)
	{
		if (isCoach(0) && isClient(last_idx))
			m_clientsNames.append(_userName(last_idx));
		else if (isCoach(last_idx))
			m_coachesNames.append(_userName(last_idx));
	}
}

void DBUserModel::createMainUser(const QString &userid, const QString &name)
{
	if (m_usersData.count() == 0)
	{
		m_usersData.insert(0, std::move(QStringList{} << (userid.isEmpty() ? std::move(generateUniqueUserId()) : userid) <<
			QString{} << std::move("0"_L1) << name << std::move("2429630"_L1) << std::move("2"_L1) << QString{} <<
			QString{} << QString{} << QString{} << QString{} << QString{} << std::move("0"_L1)));
		static_cast<void>(appUtils()->mkdir(localDir(0)));
		setPhoneBasedOnLocale();
		appDBInterface()->init();
		emit userModified(0, USER_MODIFIED_CREATED);
	}
}

void DBUserModel::removeMainUser()
{
	if (!m_usersData.isEmpty())
		m_usersData.removeFirst();
}

void DBUserModel::removeUser(const int user_idx, const bool remove_local, const bool remove_online)
{
	if (user_idx >= 1 && user_idx < m_usersData.count())
	{
		if (onlineAccount(user_idx))
		{
			if (!remove_online)
				return;
		}
		else
		{
			if (!remove_local)
				return;
		}

		connect(appDBInterface(), &DBInterface::databaseReady, this, [this,user_idx] (const uint) {
			m_usersData.remove(user_idx);
			uint next_cur_user_idx{0};
			if (m_usersData.count() > 1)
				next_cur_user_idx = user_idx - 1;
			//setCurrentRow(next_cur_user_idx);
		}, Qt::SingleShotConnection);
		if (isCoach(user_idx))
			delCoach(user_idx);
		if (isClient(user_idx))
			delClient(user_idx);
		emit userModified(0, USER_MODIFIED_REMOVED);
	}
}

int DBUserModel::userIdxFromFieldValue(const uint field, const QString &value, const bool exact_match) const
{
	int user_idx{0};
	if (exact_match)
	{
		for (const auto &user : m_usersData)
		{
			if (user.at(field) == value)
				return user_idx;
			++user_idx;
		}
		return -1;
	}
	else
	{
		std::pair<double,int> greatest_similarity{0.0,-1};
		for (const auto &user : m_usersData)
		{
			const double similarity{appUtils()->similarityBetweenString(user.at(field), value)};
			if (greatest_similarity.first < similarity)
			{
				greatest_similarity.first = similarity;
				greatest_similarity.second = user_idx;
			}
			++user_idx;
		}
		return greatest_similarity.second;
	}
}

const QString &DBUserModel::userIdFromFieldValue(const uint field, const QString &value) const
{
	const auto &user{std::find_if(m_usersData.cbegin(), m_usersData.cend(), [field,value] (const auto &user_info) {
		return user_info.at(field) == value;
	})};
	if (user != m_usersData.cend())
		return user->at(USER_COL_ID);
	return m_emptyString;
}

const QString DBUserModel::localDir(const int user_idx) const
{
	switch (user_idx)
	{
		case -1: return QString{};
		case 0: return appSettings()->userDir(userId(0));
		default:
			if (user_idx != m_tempRow)
				return (isCoach(user_idx) ? m_dirForCurrentCoaches : m_dirForCurrentClients) + userId(user_idx) + '/';
			else
				return m_tempUserInfo->sourcePath();
	}
}

void DBUserModel::setPassword(const QString &password)
{
	appKeyChain()->writeKey(userId(0), password);
}

void DBUserModel::getPassword()
{
	if (!m_usersData.isEmpty())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			emit userPasswordAvailable(value);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::setPhone(const int user_idx, QString new_phone_prefix, const QString &new_phone)
{
	switch (new_phone_prefix.length())
	{
		case 0: setPhoneBasedOnLocale(); break;
		case 1:
			if (new_phone_prefix.at(0) == '+')
				setPhoneBasedOnLocale();
		break;
		default:
			new_phone_prefix.truncate(4);
			if (new_phone_prefix.last(1) != ' ')
				new_phone_prefix.append(' ');
	}
	if (new_phone_prefix.at(0) != '+')
		new_phone_prefix.prepend('+');
	m_usersData[user_idx][USER_COL_PHONE] = std::move(new_phone_prefix + new_phone);
	emit userModified(user_idx, USER_COL_PHONE);
}

QString DBUserModel::avatar(const uint user_idx, const bool checkServer)
{
	if (user_idx < m_usersData.count() && !userId(user_idx).isEmpty())
	{
		if (onlineAccount() && checkServer && user_idx > 0)
			downloadAvatarFromServer(user_idx);
		const QDir &localFilesDir{localDir(user_idx)};
		const QFileInfoList &images{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
		const auto &it = std::find_if(images.cbegin(), images.cend(), [] (const auto &image_fi) {
			return image_fi.fileName().contains("avatar."_L1);
		});
		return it != images.cend() ? it->filePath() : defaultAvatar(user_idx);
	}
	return QString {};
}

void DBUserModel::setAvatar(const int user_idx, const QString &new_avatar, const bool saveToDisk, const bool upload)
{
	if (saveToDisk)
	{
		TPImage img{nullptr};
		img.setSource(new_avatar);
		const QString &localAvatarFilePath{localDir(user_idx) + "avatar."_L1 + img.sourceExtension()};
		static_cast<void>(QFile::remove(avatar(user_idx, false)));
		img.saveToDisk(localAvatarFilePath);
	}
	emit userModified(user_idx, USER_COL_AVATAR);

	if (onlineAccount() && user_idx == 0 && upload)
		sendAvatarToServer();
}

void DBUserModel::setAppUseMode(const int user_idx, const int new_use_opt)
{
	if (new_use_opt != appUseMode(user_idx))
	{
		if (user_idx == 0)
		{
			if (isCoach(0) && m_clientsNames.count() > 0)
			{
				if (new_use_opt != APP_USE_MODE_SINGLE_COACH && new_use_opt != APP_USE_MODE_COACH_USER_WITH_COACH)
				{
					connect(appMainWindow(), SIGNAL(revokeCoachStatus(int,bool)), this,
										SLOT(slot_revokeCoachStatus(int,bool)), Qt::SingleShotConnection);
					QMetaObject::invokeMethod(appMainWindow(), "showRevokeCoachStatus",
						Q_ARG(int, new_use_opt),
						Q_ARG(QString, tr("Revoke coach status")),
						Q_ARG(QString, tr("All your clients will be removed and cannot be automatically retrieved")));
				}
			}
			if (isClient(0) && m_coachesNames.count() > 0)
			{
				if (new_use_opt == APP_USE_MODE_SINGLE_COACH)
				{
					connect(appMainWindow(), SIGNAL(revokeClientStatus(int,bool)), this,
							SLOT(slot_revokeClientStatus(int,bool)), Qt::SingleShotConnection);
					QMetaObject::invokeMethod(appMainWindow(), "showRevokeClientStatus",
						Q_ARG(int, new_use_opt),
						Q_ARG(QString, tr("Revoke client status")),
						Q_ARG(QString, tr("All your clients will be removed and cannot be automatically retrieved")));
				}
			}
			m_usersData[user_idx][USER_COL_APP_USE_MODE] = std::move(QString::number(new_use_opt));
			emit userModified(0, USER_COL_APP_USE_MODE);
		}
		else
		{
			m_usersData[user_idx][USER_COL_APP_USE_MODE] = std::move(QString::number(new_use_opt));
			emit userModified(user_idx, USER_COL_APP_USE_MODE);
		}
	}
}

void DBUserModel::addCoach(const uint user_idx)
{
	m_coachesNames.append(_userName(user_idx));
	emit coachesNamesChanged();
	appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings({tr("New coach!"),
		tr("Now that ") + userName(user_idx) + tr(" is your coach, you can send them messages using the Star Button on the Home screen") }, record_separator),
		avatar(user_idx, false), 10000);
}

void DBUserModel::delCoach(const uint user_idx)
{
	m_coachesNames.removeOne(userId(0));
	emit coachesNamesChanged();
	clearUserDir(userId(user_idx));
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,user_idx] (const QString &key, const QString &value) {
		appOnlineServices()->removeCoachFromClient(0, key, value, userId(user_idx));
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::addClient(const uint user_idx)
{
	m_clientsNames.append(_userName(user_idx));
	emit clientsNamesChanged();
}

void DBUserModel::delClient(const uint user_idx)
{
	if (user_idx >= 1)
	{
		m_clientsNames.removeOne(userId(user_idx));
		emit clientsNamesChanged();
		clearUserDir(userId(user_idx));
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,user_idx] (const QString &key, const QString &value) {
			appOnlineServices()->removeClientFromCoach(0, key, value, userId(user_idx));
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

//When client changes name remotely or when changing from not yet accepted coach(main user) to accepted coach
void DBUserModel::changeClient(const uint user_idx, const QString &oldname)
{
	const qsizetype idx{m_clientsNames.indexOf(oldname)};
	if (idx >= 0)
	{
		m_clientsNames[idx] = _userName(user_idx);
		emit clientsNamesChanged();
	}
}

QStringList DBUserModel::coachesAndClientsNames() const
{
	QStringList coaches_and_clients;
	for (const auto &coach : m_coachesNames)
		coaches_and_clients.append(std::move(QString{tr("Coach: ") + coach}));
	for (const auto &client : m_clientsNames)
		coaches_and_clients.append(std::move(QString{tr("Client: ") + client}));
	return coaches_and_clients;
}

#ifndef Q_OS_ANDROID
void DBUserModel::getAllOnlineUsers()
{
	if (canConnectToServer())
	{
		const int requestid{appUtils()->generateUniqueId("getAllOnlineUsers"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
						(const int request_id, const int ret_code, const QStringList &ret_list)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (!m_allUsers)
				{
					m_allUsers = new OnlineUserInfo{this, USER_TOTAL_COLS};
					m_allUsers->setSelectEntireRow(true);
				}
				else
					m_allUsers->clear();
				for (const auto &userid : std::as_const(ret_list))
				{
					const int requestid2{static_cast<int>(userid.toLong())};
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid2]
								(const int request_id, const int ret_code, const QString &ret_string) mutable
					{
						if (request_id == requestid2)
						{
							disconnect(*conn2);
							if (ret_code == TP_RET_CODE_SUCCESS)
							{
								if (m_allUsers->dataFromString(ret_string))
									emit allUsersChanged();
							}
						}
					});
					appOnlineServices()->getOnlineUserData(requestid2, userid);
				}
			}
		});
		appOnlineServices()->getAllUsers(requestid);
	}
}

void DBUserModel::switchUser()
{
	if (m_allUsers->currentRow() >= 0)
	{
		QString userid{m_allUsers->data(m_allUsers->currentRow(), USER_COL_ID)};
		connect(this, &DBUserModel::userSwitchPhase1Finished, this, [this,userid] (const bool success) mutable
		{
			if (success)
				userSwitchingActions(false, std::move(userid));
		}, Qt::SingleShotConnection);
		switchToUser(userid, true);
	}
}

void DBUserModel::createNewUser()
{
	userSwitchingActions(true, std::move(generateUniqueUserId()));
	showFirstTimeUseDialog();
}

void DBUserModel::removeOtherUser()
{
	const QString &userid{m_allUsers->data(m_allUsers->currentRow(), USER_COL_ID)};
	const QLatin1StringView seed{"remove" + userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,userid,conn,requestid]
		(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			clearUserDir(userid);
			m_allUsers->removeUserInfo(m_allUsers->currentRow(), false);
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
				appUtils()->string_strings({tr("User removed"), m_allUsers->data(m_allUsers->currentRow(), USER_COL_NAME) +
				tr(" removed locally and remotely")}, record_separator));
		}
	});

	appOnlineServices()->removeUser(requestid, userid);
}

void DBUserModel::userSwitchingActions(const bool create, QString &&userid)
{
	mb_userRegistered = false;
	m_usersData.clear();
	m_coachesNames.clear();
	m_clientsNames.clear();
	if (m_availableCoaches)
		m_availableCoaches->clear();
	if (m_pendingClientRequests)
		m_pendingClientRequests->clear();
	if (m_pendingCoachesResponses)
		m_pendingCoachesResponses->clear();
	if (m_tempUserInfo)
		m_tempUserInfo->clear();

	appSettings()->importFromUserConfig(userid);
	if (!appMesoModel())
	{
		new DBMesocyclesModel{this};
		new PagesListModel{this};
	}

	if (create)
		createMainUser(appSettings()->currentUser(), tr("New user"));
	else
		appDBInterface()->init();

	emit appUseModeChanged();
	emit onlineUserChanged();
	emit coachesNamesChanged();
	emit clientsNamesChanged();
	if (canConnectToServer())
		onlineCheckIn();

	appMesoModel()->userSwitchingActions();
	appPagesListModel()->userSwitchingActions();
	emit userIdChanged();
}
#endif

int DBUserModel::getTemporaryUserInfo(OnlineUserInfo *tempUser, const uint userInfoRow)
{
	if (m_tempRow >= 1)
	{
		m_usersData.remove(m_tempRow);
		m_tempRow = -1;
		m_tempUserInfo = nullptr;
	}

	if (tempUser && userInfoRow < tempUser->count())
	{
		tempUser->setCurrentRow(userInfoRow);
		m_tempRow = m_usersData.count();
		m_usersData.append(tempUser->modeldata(userInfoRow));
		m_tempUserInfo = tempUser;
		downloadAvatarFromServer(m_tempRow);
		return m_tempRow;
	}
	return -1;
}

bool DBUserModel::mainUserConfigured() const
{
	bool ret{false};
	if (m_usersData.count() >= 1)
	{
		ret = onlineAccount(0) && !email(0).isEmpty();
		ret &= isCoach(0) == !m_usersData.at(0).at(USER_COL_COACHROLE).isEmpty();
		ret &= isClient(0) == !m_usersData.at(0).at(USER_COL_GOAL).isEmpty();
	}
	return ret;
}

void DBUserModel::acceptUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		const bool userIsCoach{userInfo->isCoach(userInfoRow)};
		const int new_app_use_mode{userIsCoach ? APP_USE_MODE_SINGLE_COACH : APP_USE_MODE_PENDING_CLIENT};

		if (userId(m_tempRow) == user_id)
			m_tempRow = -1;
		else
			addUser(std::move(userInfo->modeldata(userInfoRow)));

		m_usersData.last()[USER_COL_APP_USE_MODE] = std::move(QString::number(new_app_use_mode));
		const uint lastidx{userCount()-1};
		if (userIsCoach)
		{
			addCoach(lastidx);
			if (canConnectToServer())
				appOnlineServices()->acceptCoachAnswer(0, key, value, user_id);
		}
		else
		{
			//Only when the user confirms the coach's acceptance, can they be effectively included as client of main user
			m_usersData.last()[USER_COL_NAME] = std::move(_userName(lastidx) + tr(" !Pending confirmation!"));
			addClient(lastidx);
			if (canConnectToServer())
				appOnlineServices()->acceptClientRequest(0, key, value, user_id);
		}
		copyTempUserFilesToFinalUserDir(localDir(user_id), userInfo, userInfoRow);
		userInfo->removeUserInfo(userInfoRow, false);
		emit userModified(lastidx, USER_MODIFIED_ACCEPTED);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::rejectUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		if (userId(m_tempRow) == user_id)
			getTemporaryUserInfo(nullptr, -1);
		if (userInfo->isCoach(userInfoRow))
			appOnlineServices()->rejectCoachAnswer(0, key, value, user_id);
		else
			appOnlineServices()->rejectClientRequest(0, key, value, user_id);
		clearTempUserFiles(userInfo, userInfoRow);
		userInfo->removeUserInfo(userInfoRow, true);

	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::checkUserOnline(const QString &email, const QString &password)
{
	if (appOsInterface()->tpServerOK())
	{
		const int requestid{appUtils()->generateUniqueId("checkUserOnline"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,password]
								(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) //Password matches server's. Store it for the session
				{
					m_onlineAccountId = ret_string;
					m_password = password;
				}
				else
				{
					//If the main user is configured and registed but checkOnlineUser() returned an error that means that,
					//for some reason, the online users database was not updated with the user information upon first time
					//setup. Do it now, then.
					if (mainUserConfigured() && mainUserRegistered())
						sendUserDataToServerDatabase();
				}
				emit userOnlineCheckResult(ret_code == TP_RET_CODE_SUCCESS);
			}
		});
		appOnlineServices()->checkOnlineUser(requestid, "email="_L1 + email, password);
	}
}

void DBUserModel::changePassword(const QString &old_password, const QString &new_password)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,old_password,new_password] (const QString &key, const QString &value) {
		const int requestid{appUtils()->generateUniqueId("changePassword"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,key,new_password]
							(const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS)
				{
					appKeyChain()->deleteKey(key);
					setPassword(new_password);
				}
				else
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_UNKNOWN_ERROR, ret_string);
			}
		});
		appOnlineServices()->changePassword(requestid, key, old_password, new_password);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::importFromOnlineServer()
{
	if (canConnectToServer())
	{
		const int requestid{appUtils()->generateUniqueId("importFromOnlineServer"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS)
				{
					removeMainUser();
					if (importFromString(ret_string))
					{
						mb_userRegistered = true;
						setPassword(m_password);
						switchToUser(m_onlineAccountId);
					}
				}
				else
					emit userOnlineImportFinished(false);
			}
		});
		appOnlineServices()->getOnlineUserData(requestid, m_onlineAccountId);
	}
}

void DBUserModel::setCoachPublicStatus(const bool bPublic)
{
	mb_coachPublic = bPublic;
	if (canConnectToServer())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("setCoachPublicStatus"_L1)};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
									{tr("Coach registration"), ret_string}, record_separator));
					mb_coachRegistered = (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) &&
											mb_coachPublic;
					emit coachOnlineStatus(mb_coachRegistered == true);
					if (!mb_coachRegistered && haveClients())
					{
						for (qsizetype i{m_clientsNames.count() - 1}; i >= 1; --i)
						{
							if (isClient(i))
								removeUser(i, false, true);
						}
					}
				}
			});
			appOnlineServices()->addOrRemoveCoach(requestid, key, value, mb_coachPublic);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

constexpr uint file_upload_max_size{8*1024*1024};
void DBUserModel::uploadResume(const QString &resumeFileName)
{
	const QString &resumeFileName_ok{appUtils()->getCorrectPath(resumeFileName)};
	QFileInfo fi{resumeFileName_ok};
	if (fi.isReadable())
	{
		if (fi.size() > file_upload_max_size)
		{
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
					appUtils()->string_strings({ tr("Cannot upload file"), tr("Maximum file size allowed: 8MB")}, record_separator), "error");
			return;
		}
		const qsizetype idx{resumeFileName_ok.lastIndexOf('.')};
		const QString &extension{idx > 0 ? resumeFileName_ok.last(resumeFileName_ok.length() - idx) : QString{}};
		const QString &localResumeFilePath{localDir(0) + "resume"_L1 + extension};
		const QString &previousResumeFilePath{resume(0)};
		if (appUtils()->copyFile(resumeFileName_ok, localResumeFilePath))
		{
			sendFileToServer(localResumeFilePath, nullptr, tr("Résumé uploaded successfully!"), QString{}, userId(0), true);
			if (previousResumeFilePath != localResumeFilePath)
				removeFileFromServer(appUtils()->getFileName(previousResumeFilePath), QString{}, userId(0));
		}
	}
}

void DBUserModel::setMainUserConfigurationFinished()
{
	if (canConnectToServer())
	{
		if (!mainUserRegistered())
			onlineCheckIn();
		else
		{
			if (mb_MainUserInfoChanged)
			{
				sendProfileToServer();
				sendUserDataToServerDatabase();
				mb_MainUserInfoChanged = false;
			}
		}
	}
	emit mainUserConfigurationFinished();
}

void DBUserModel::sendRequestToCoaches()
{
	if (m_availableCoaches)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			for (uint i{0}; i < m_availableCoaches->nSelected(); ++i)
			{
				if (m_availableCoaches->isSelected(i))
				{
					const int requestid{appUtils()->generateUniqueId(QLatin1StringView{
						QString{"sendRequestToCoaches"_L1 + m_availableCoaches->data(i, USER_COL_ID)}.toLatin1()})};
					auto conn{std::make_shared<QMetaObject::Connection>()};
					*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,i]
										(const int request_id, const int ret_code, const QString &ret_string)
					{
						if (request_id == requestid)
						{
							disconnect(*conn);
							if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
							{
								const QString &coach_id{m_availableCoaches->data(i, USER_COL_ID)};
								if (appUtils()->copyFile(profileFileName(m_onlineCoachesDir, coach_id), profileFileName(m_dirForRequestedCoaches, coach_id)))
								{
									appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
										{tr("Coach contacting"), tr("Online coach contacted ") + m_availableCoaches->data(i, USER_COL_NAME)}, record_separator));
								}
							}
							else
								appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_UNKNOWN_ERROR, ret_string);
						}
					});
					appOnlineServices()->sendRequestToCoach(requestid, key, value, m_availableCoaches->data(i, USER_COL_ID));
				}
			}
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::getOnlineCoachesList(const bool get_list_only)
{
	if (canConnectToServer() && onlineAccount())
	{
		static_cast<void>(appUtils()->mkdir(m_onlineCoachesDir));
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,get_list_only] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("getOnlineCoachesList"_L1)};
			auto conn {std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,get_list_only]
								(const int request_id, const int ret_code, const QString &ret_string)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (ret_code == TP_RET_CODE_SUCCESS)
					{
						QStringList coaches{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
						if (get_list_only)
						{
							emit coachesListReceived(coaches);
							return;
						}
						if (m_availableCoaches->sanitize(coaches, USER_COL_ID))
							emit availableCoachesChanged();

						//First pass
						for (qsizetype i{coaches.count()-1}; i >= 0; --i)
						{
							const int userrow{userIdxFromFieldValue(USER_COL_ID, coaches.at(i))};
							if (userrow != -1 && userrow != m_tempRow)
								coaches.remove(i); //coach is already in the database, therefore not available
						}

						//Second pass
						qsizetype n_connections{coaches.count()};
						auto conn{std::make_shared<QMetaObject::Connection>()};
						*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,coaches,n_connections]
																				(const QString &userid, const bool success) mutable
						{
							if (--n_connections == 0)
								disconnect(*conn);
							if (success)
								addAvailableCoach(userid);
						});
						for (qsizetype x{0}; x < coaches.count(); ++x)
						{
							const QString &coach_profile{profileFileName(m_onlineCoachesDir, coaches.at(x))};
							getUserOnlineProfile(coaches.at(x), coach_profile);
						}
					}
				}
			});
			appOnlineServices()->getOnlineCoachesList(requestid, key, value);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

int DBUserModel::sendFileToServer(const QString &filename, QFile *upload_file, const QString &successMessage,
									const QString &subdir, const QString &targetUser, const bool removeLocalFile)
{
	if (!onlineAccount())
		return -1;
	else if (!canConnectToServer())
	{
		if (!successMessage.isEmpty())
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
				{tpNetworkTitle, tr("Online server unavailable. Try it again once the app is connected to the server.")}, record_separator));
		return -1;
	}
	else {
		if (!mainUserRegistered())
			return -1;
	}

	const int requestid{appUtils()->generateUniqueId(QLatin1StringView{QString{
					"sendFileToServer"_L1 + std::move(appUtils()->getFileName(filename).toLatin1())}.toLatin1()})};

	if (!upload_file)
		upload_file = appUtils()->openFile(filename, true, false, false, false);
	else
	{
		if (upload_file->isOpen())
		{
			upload_file->close();
			if (!upload_file->open(QIODeviceBase::ReadOnly))
				return -1;
		}
	}
	if (upload_file)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					upload_file->close();
					if (removeLocalFile)
						QFile::remove(upload_file->fileName());
					if (ret_code == TP_RET_CODE_SUCCESS)
					{
						if (!successMessage.isEmpty())
							appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
								appUtils()->string_strings({tpNetworkTitle, successMessage}, record_separator));
					}
					else
						appItemManager()->displayMessageOnAppWindow(ret_code, ret_string);
					delete upload_file;
					emit fileUploaded(ret_code == TP_RET_CODE_SUCCESS, requestid);
				}
			});
			appOnlineServices()->sendFile(requestid, key, value, upload_file, subdir, targetUser);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_OPEN_READ_FAILED, filename);
	return requestid;
}

int DBUserModel::downloadFileFromServer(const QString &filename, const QString &local_filename, const QString &successMessage,
											const QString &subdir, const QString &targetUser)
{
	if (!canConnectToServer())
	{
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
					{tpNetworkTitle, tr("Online server unavailable. Try it again once the app is connected to the server.")}, record_separator));
		return -1;
	}
	else
	{
		if (!mainUserRegistered())
			return -2;
	}

	QLatin1StringView v{QString{targetUser + filename}.toLatin1().constData()};
	const int requestid{appUtils()->generateUniqueId(v)};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [=,this]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				bool success{!contents.isEmpty()};
				QString dest_file;
				if (local_filename.isEmpty())
					dest_file = std::move(appSettings()->userDir(userId(0)) + filename);
				else
				{
					static_cast<void>(appUtils()->mkdir(appUtils()->getFilePath(local_filename)));
					dest_file = local_filename;
				}
				switch (ret_code)
				{
					case TP_RET_CODE_SUCCESS: //file downloaded
					{
						if (!success)
							break;
						QFile *local_file{new QFile{dest_file, this}};
						if (!local_file->exists() || local_file->remove())
						{
							if (local_file->open(QIODeviceBase::WriteOnly))
							{
								local_file->write(contents);
								local_file->close();
							}
						}
						delete local_file;
						if (!successMessage.isEmpty())
							appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
									{tpNetworkTitle, successMessage}, record_separator));
					}
					break;
					case TP_RET_CODE_NO_CHANGES_SUCCESS: //online file and local file are the same
						success = true;
					break;
					default: //some error
						success = false;
				}
				if (!success && !successMessage.isEmpty())
				{
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, appUtils()->string_strings(
																		{filename + contents}, record_separator));
				}
				emit fileDownloaded(success, requestid, dest_file);
			}
		});
		appOnlineServices()->getFile(requestid, key, value, filename, subdir, targetUser, local_filename);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
	return requestid;
}

void DBUserModel::removeFileFromServer(const QString &filename, const QString &subdir, const QString &targetUser)
{
	if (!mainUserRegistered())
		return;

	QLatin1StringView v{filename.toLatin1().constData()};
	const int requestid{appUtils()->generateUniqueId(v)};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
		appOnlineServices()->removeFile(requestid, key, value, filename, subdir, targetUser);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::sendCmdFileToServer(const QString &cmd_filename)
{
	if (canConnectToServer())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,cmd_filename] (const QString &key, const QString &value)
		{
			const QString &filename_to_upload{appUtils()->getFilePath(cmd_filename) + QString::number(n_cmdOrderValue++) + ".cmd"_L1};
			if (!appUtils()->rename(cmd_filename, filename_to_upload, true))
				return;
			QFile *cmd_file{appUtils()->openFile(filename_to_upload, true, false, false, false)};
			if (!cmd_file)
				return;
			const QString &subdir{appUtils()->getFileName(appUtils()->getFilePath(cmd_filename))};
			const QLatin1StringView seed{filename_to_upload.toLatin1()};
			const int requestid{appUtils()->generateUniqueId(seed)};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
								(const int request_id, const int ret_code, const QString &ret_string)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					cmd_file->close();
					QFile::remove(cmd_file->fileName());
					delete cmd_file;
					if (ret_code == TP_RET_CODE_SUCCESS)
						appOnlineServices()->executeCommands(requestid, key, value, subdir,
												mb_singleDevice.has_value() ? mb_singleDevice.value() : false);
				}
			});
			appOnlineServices()->sendFile(requestid, key, value, cmd_file, subdir, userId(0));
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::downloadCmdFilesFromServer(const QString &subdir)
{
	if (canConnectToServer())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,subdir] (const QString &key, const QString &value)
		{
			const int requestid{appUtils()->generateUniqueId("downloadCmdFilesFromServer"_L1)};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid,key,value,subdir]
				(const int request_id, const int ret_code, const QStringList &ret_list)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (ret_list.isEmpty())
						return;
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [=,this]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents)
					{
						if (request_id == requestid)
						{
							disconnect(*conn2);
							switch (ret_code)
							{
								case TP_RET_CODE_SUCCESS: //file downloaded
								{
									QFile *local_cmdfile{new QFile{subdir + filename, this}};
									if (!local_cmdfile->exists() || local_cmdfile->remove())
									{
										if (local_cmdfile->open(QIODeviceBase::WriteOnly))
										{
											local_cmdfile->write(contents);
											local_cmdfile->close();
										}
									}
									appUtils()->parseCmdFile(local_cmdfile->fileName());
									delete local_cmdfile;
								}
								break;
								case TP_RET_CODE_NO_CHANGES_SUCCESS: //online file and local file are the same
								break;
								default: //some error
									return;
							}
						}
					});
				}
			});
			appOnlineServices()->listFiles(requestid, key, value, true, false, ".cmd"_L1, subdir, userId(0));
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

int DBUserModel::exportToFile(const uint user_idx, const QString &filename, const bool write_header, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, false, true, true);
		if (!out_file)
			return TP_RET_CODE_OPEN_WRITE_FAILED;
	}

	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	const bool ret{appUtils()->writeDataToFile(out_file, write_header ? appUtils()->userFileIdentifier : QString{}, m_usersData)};
	out_file->close();
	return ret ? TP_RET_CODE_EXPORT_OK : TP_RET_CODE_EXPORT_FAILED;
}

int DBUserModel::exportToFormattedFile(const uint user_idx, const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = {appUtils()->openFile(filename, false, true, true)};
		if (!out_file)
			return TP_RET_CODE_OPEN_CREATE_FAILED;
	}
	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	QList<std::function<QString(void)>> field_description{QList<std::function<QString(void)>>{} <<
											[this] () { return idLabel(); } <<
											[this] () { return nameLabel(); } <<
											[this] () { return birthdayLabel(); } <<
											[this] () { return sexLabel(); } <<
											[this] () { return phoneLabel(); } <<
											[this] () { return emailLabel(); } <<
											[this] () { return socialMediaLabel(); } <<
											[this] () { return userRoleLabel(); } <<
											[this] () { return coachRoleLabel(); } <<
											[this] () { return goalLabel(); } <<
											nullptr
	};

	int ret{TP_RET_CODE_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					appUtils()->userFileIdentifier,
					m_usersData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_user_idx,
					QString{isCoach(user_idx) ? tr("Coach Information") : tr("Client Information") + "\n\n"_L1})
	)
		ret = TP_RET_CODE_EXPORT_OK;
	return ret;
}

int DBUserModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	m_tempUserData.clear();
	int ret{appUtils()->readDataFromFile(in_file, m_tempUserData, USER_TOTAL_COLS, appUtils()->userFileIdentifier)};
	if (ret != TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
		ret = TP_RET_CODE_IMPORT_OK;
	in_file->close();
	return ret;
}

int DBUserModel::importFromFormattedFile(const QString &filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	m_tempUserData.clear();
	int ret{appUtils()->readDataFromFormattedFile(in_file,
												m_tempUserData,
												USER_TOTAL_COLS,
												appUtils()->userFileIdentifier,
												[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	if (ret > 0)
		ret = TP_RET_CODE_IMPORT_OK;
	in_file->close();
	return ret;
}

bool DBUserModel::importFromString(const QString &user_data)
{
	QStringList modeldata{std::move(user_data.split('\n'))};
	if (modeldata.count() < USER_TOTAL_COLS)
		return false;
	if (modeldata.count() > USER_TOTAL_COLS)
		modeldata.resize(USER_TOTAL_COLS); //remove the password field and anything else that does not belong
	m_usersData.append(std::move(modeldata));
	emit userModified(m_usersData.count() - 1, USER_MODIFIED_IMPORTED);
	return true;
}

int DBUserModel::newUserFromFile(const QString &filename, const std::optional<bool> &file_formatted)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value())
	{
		if (file_formatted.value())
			import_result = importFromFormattedFile(filename);
		else
			import_result = importFromFile(filename);
	}
	else
	{
		import_result = importFromFile(filename);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(filename);
	}
	if (import_result < 0)
		return import_result;

	const QString &temp_user_data{m_tempUserData.at(0).join('\n')};
	const uint tempUserAppUseMode{m_tempUserData.at(0).at(USER_COL_APP_USE_MODE).toUInt()};
	const bool tempUserIsCoach{tempUserAppUseMode == APP_USE_MODE_SINGLE_COACH || tempUserAppUseMode == APP_USE_MODE_COACH_USER_WITH_COACH};
	if (tempUserIsCoach)
	{
		if (m_pendingCoachesResponses->dataFromString(temp_user_data))
		{
			m_pendingCoachesResponses->setIsCoach(m_pendingCoachesResponses->count()-1, true);
			emit pendingCoachesResponsesChanged();
		}
	}
	else
	{
		if (m_pendingClientRequests->dataFromString(temp_user_data))
		{
			m_pendingClientRequests->setIsCoach(m_pendingClientRequests->count()-1, false);
			emit pendingClientsRequestsChanged();
		}
	}

	return TP_RET_CODE_IMPORT_OK;
}

void DBUserModel::getPasswordFromUserInput(const int resultCode, const QString &password)
{
	if (resultCode == TP_RET_CODE_SUCCESS)
	{
		if (onlineAccount())
		{
			connect(appKeyChain(), &TPKeyChain::keyStored, this, [this] (const QString &key) {
				registerUserOnline();
			}, Qt::SingleShotConnection);
		}
		setPassword(password);
	}
}

void DBUserModel::slot_unregisterUser(const bool unregister)
{
	if (unregister)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("unregisterUserOnline"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,key,value]
							(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
						if (request_id == requestid)
						{
							disconnect(*conn2);
							if (ret_code == TP_RET_CODE_SUCCESS)
							{
								mb_userRegistered = false;
								emit mainUserOnlineCheckInChanged();
							}
							appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
								appUtils()->string_strings({tpNetworkTitle, ret_code == TP_RET_CODE_SUCCESS ?
								tr("Online account removed") : tr("Failed to remove online account")}, record_separator));
						}
					});
					appOnlineServices()->removeUser(requestid, userId(0));
				}
			});
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::slot_removeNoLongerAvailableUser(const int user_idx, bool remove)
{
	if (remove)
		removeUser(user_idx);
}

void DBUserModel::slot_revokeCoachStatus(int new_use_opt, bool revoke)
{
	if (revoke)
	{
		for (qsizetype i{m_usersData.count() - 1}; i >= 1; --i)
			if (isClient(i))
				delClient(i);
		m_usersData[0][USER_COL_APP_USE_MODE] = std::move(QString::number(new_use_opt));
		emit userModified(0, USER_COL_APP_USE_MODE);
	}
}

void DBUserModel::slot_revokeClientStatus(int new_use_opt, bool revoke)
{
	if (revoke)
	{
		for (qsizetype i{m_usersData.count() - 1}; i >= 1; --i)
			if (isCoach(i))
				delCoach(i);
		m_usersData[0][USER_COL_APP_USE_MODE] = std::move(QString::number(new_use_opt));
		emit userModified(0, USER_COL_APP_USE_MODE);
	}
}

void DBUserModel::setupOnlineUserLocalDirs()
{
	m_onlineCoachesDir = std::move(appSettings()->userDir(userId(0)) + "online_coaches/"_L1);
	m_dirForRequestedCoaches = std::move(appSettings()->userDir(userId(0)) + "requested_coaches/"_L1);
	m_dirForClientsRequests = std::move(appSettings()->userDir(userId(0)) + "clients_requests/"_L1);
	m_dirForCurrentClients = std::move(appSettings()->userDir(userId(0)) + "clients/"_L1);
	m_dirForCurrentCoaches = std::move(appSettings()->userDir(userId(0)) + "coaches/"_L1);
}

QString DBUserModel::getPhonePart(const QString &str_phone, const bool prefix) const
{
	if (str_phone.length() > 0)
	{
		const qsizetype idx{str_phone.indexOf('(')};
		if (prefix)
			return idx >= 0 ? str_phone.left(idx) : str_phone;
		else {
			if (idx >= 0)
				return str_phone.sliced(idx, str_phone.length() - idx);
		}
	}
	return QString{};
}

void DBUserModel::setPhoneBasedOnLocale()
{
	if (phoneCountryPrefix(0).length() <= 0)
	{
		QString phone_country_prefix;
		switch (appSettings()->userLocaleIdx())
		{
			case 0: phone_country_prefix = std::move("+1"_L1); break;
			case 1: phone_country_prefix = std::move("+55"_L1); break;
			case 2: phone_country_prefix = std::move("+49"_L1); break;
			default: return;
		}
		setPhone(0, phone_country_prefix, QString{});
	}
}

inline QString DBUserModel::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}

void DBUserModel::onlineCheckIn()
{
	if (mainUserConfigured() && onlineAccount())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] (const bool first_checkin) {
			if (first_checkin)
			{
				sendUserDataToServerDatabase();
				sendProfileToServer();
				sendAvatarToServer();
			}
			if (m_onlineAccountId.isEmpty()) //When importing an user from the server there is no need to perform any action online
				onlineCheckinActions();
			setupOnlineUserLocalDirs();
			if (!isCoachRegistered() && mb_coachPublic)
				setCoachPublicStatus(mb_coachPublic);
			startServerPolling();
		}, Qt::SingleShotConnection);
		registerUserOnline();
	}
}

void DBUserModel::registerUserOnline()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		const int requestid{appUtils()->generateUniqueId("registerUserOnline"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
				[this,conn,requestid,key,value] (const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				switch (ret_code)
				{
					case TP_RET_CODE_SUCCESS:
						mb_userRegistered = true;
						emit mainUserOnlineCheckInChanged();
					break;
					case TP_RET_CODE_WRONG_PASSWORD:
						connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), this, SLOT(getPasswordFromUserInput(int,QString)));
					break;
					case TP_RET_CODE_USER_DOES_NOT_EXIST: //User does not exist in the online database
					{
						auto conn2{std::make_shared<QMetaObject::Connection>()};
						*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
							[this,conn2,requestid,value] (const int request_id, const int ret_code, const QString &ret_string)
						{
							if (request_id == requestid)
							{
								disconnect(*conn2);
								if (ret_code == TP_RET_CODE_SUCCESS)
								{
									mb_userRegistered = true;
									emit mainUserOnlineCheckInChanged(true);
								}
								appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
											{tpNetworkTitle, tr("User information updated")}, record_separator));
							}
						});
						appOnlineServices()->registerUser(requestid, key, value);
					}
					break;
					default:
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_UNKNOWN_ERROR, ret_string);
						mb_userRegistered = false;
					break;
				}
			}
		});
		appOnlineServices()->checkUser(requestid, key, value);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::onlineCheckinActions()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		//checkUserOnline(email(0), value); Why did I put this here??
		appOnlineServices()->executeCommands(appUtils()->idFromString("execcmds"_L1), key, value, TPDatabaseTable::databaseFilesSubDir);
		if (!mb_singleDevice.has_value())
		{
			connect(this, &DBUserModel::onlineDevicesListReceived, this, [this,value] () {
				onlineCheckinActions();
			}, Qt::SingleShotConnection);
			getOnlineDevicesList();
		}
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::getOnlineDevicesList()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		const int requestid{appUtils()->generateUniqueId("getOnlineDevicesList"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid,value]
					(const int request_id, const int ret_code, const QStringList &ret_list)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				bool device_registered{false};
				n_devices = 0;
				mb_singleDevice = true;
				if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
				{
					device_registered = ret_list.contains(appOsInterface()->deviceID());
					if (ret_list.count() == 0)
						mb_singleDevice = true;
					else if (ret_list.count() == 1)
						mb_singleDevice = device_registered;
					else
					{
						mb_singleDevice = false;
						n_devices = ret_list.count();
					}
				}
				if (!device_registered)
				{
					appOnlineServices()->addDevice(requestid, userId(0), value, appOsInterface()->deviceID());
					++n_devices;
				}
				emit onlineDevicesListReceived();
			}
		});
		appOnlineServices()->getDevicesList(requestid, userId(0), value);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::switchToUser(const QString &new_userid, const bool user_switching_for_testing)
{
	QTimer *download_timeout{new QTimer{this}};
	connect(this, &DBUserModel::allUserFilesDownloaded, this, [this,user_switching_for_testing,new_userid,download_timeout]
																	(const bool success)
	{
		delete download_timeout;
		if (!success)
		{
			#ifndef Q_OS_ANDROID
			if (user_switching_for_testing)
			{
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
				appUtils()->string_strings({ tr("User switching error"),
					tr("Could not download files for user ") +
						m_allUsers->fieldValueFromAnotherFieldValue(USER_COL_NAME, USER_COL_ID, new_userid)},
						record_separator), "error");
			}
			else
			#endif
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
				appUtils()->string_strings({ tr("User switching error"),
					tr("Could not download files for user ") +  m_onlineAccountId}, record_separator), "error");
		}
		else
		{
			if (!user_switching_for_testing)
			{
				appSettings()->importFromUserConfig(new_userid);
				appDBInterface()->init();
			}
		}
		if (!user_switching_for_testing)
			emit userOnlineImportFinished(success);
		#ifndef Q_OS_ANDROID
		else
			emit userSwitchPhase1Finished(success);
		#endif

	}, Qt::SingleShotConnection);
	downloadAllUserFiles(new_userid);
	download_timeout->callOnTimeout([this] () { emit allUserFilesDownloaded(false); });
	//download_timeout->start(60*1000);
}

void DBUserModel::downloadAllUserFiles(const QString &userid)
{
	static int total_dirs{0};
	static int total_files{0};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userid] (const QString &key, const QString &value)
	{
		appUtils()->mkdir(appSettings()->userDir(userid));
		total_dirs = total_files = 0;
		const int requestid{static_cast<int>(userid.toLong())};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,userid,requestid,key,value]
						(const int request_id, const int ret_code, const QStringList &ret_list)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				total_dirs = ret_list.count();
				for (const auto &dir : std::as_const(ret_list))
				{
					if (ret_code != TP_RET_CODE_SUCCESS)
						return;
					QString dest_dir{std::move(appSettings()->userDir(userid))};
					if (dir != '.')
					{
						dest_dir += dir + '/';
						appUtils()->mkdir(dest_dir);
					}
					const QLatin1StringView seed{dir.toLatin1().constData()};
					const int requestid2{appUtils()->generateUniqueId(seed)};
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn2,requestid2,userid,dest_dir]
						(const int request_id, const int ret_code, const QStringList &ret_list) mutable
					{
						if (request_id == requestid2)
						{
							if (--total_dirs <= 0)
								disconnect(*conn2);
							total_files += ret_list.count();
							if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
							{
								if (ret_list.count() == 0) //All files up to date, no need to download them
								{
									if (total_files == 0 && total_dirs == 0)
										emit allUserFilesDownloaded(true);
									return;
								}
								for (const auto &file : std::as_const(ret_list))
								{
									auto conn3{std::make_shared<QMetaObject::Connection>()};
									QString subdir{std::move(appUtils()->getFileName(dest_dir))};
									if (subdir == userid + '/')
										subdir.clear();
									const int requestid3{downloadFileFromServer(file, QString{}, QString{}, subdir, userid)};
									*conn3 = connect(this, &DBUserModel::fileDownloaded, this, [this,conn3,requestid3]
										(const bool success, const uint requestid, const QString &localFileName) mutable
									{
										if (requestid == requestid3)
										{
											disconnect(*conn3);
											if (--total_files <= 0)
												emit allUserFilesDownloaded(true);
										}
									});
								}
							}
						}
					});
					appOnlineServices()->listFiles(requestid2, key, value, true, false, QString{}, dir, userid);
				}
			}
		});
		appOnlineServices()->listDirs(requestid, key, value, QString{}, QString{}, userid, true);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::lastOnlineCmd(const uint requestid, const QString &subdir)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,requestid,subdir] (const QString &key, const QString &value)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QStringList &ret_list)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_list.isEmpty())
					n_cmdOrderValue = 1;
				else
					n_cmdOrderValue = ret_list.last().first(ret_list.last().length() - 4).toInt() + 1;
			}
		});
		appOnlineServices()->listFiles(requestid, userId(0), value, true, false, ".cmd"_L1, subdir, userId(0));
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

QString DBUserModel::resume(const uint user_idx) const
{
	if (user_idx < m_usersData.count() && !userId(user_idx).isEmpty())
	{
		const QDir &localFilesDir{localDir(user_idx)};
		const QFileInfoList &files{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
		for (const auto &it: files)
		{
			if (it.fileName().startsWith("resume."_L1))
				return it.filePath();
		}
	}
	return QString {};
}

//Only applicable to the main user that is a coach
void DBUserModel::checkIfCoachRegisteredOnline()
{
	connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list)
	{
		mb_coachRegistered = coaches_list.contains(userId(0));
		emit coachOnlineStatus(mb_coachRegistered == true);
	}, Qt::SingleShotConnection);
	getOnlineCoachesList(true);
}

void DBUserModel::getUserOnlineProfile(const QString &netID, const QString &save_as_filename)
{
	const int request_id{downloadFileFromServer(userProfileFileName, save_as_filename, QString{}, QString{}, netID)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			emit userProfileAcquired(netID, success);
		}
	});
}

void DBUserModel::sendProfileToServer()
{
	const QString &localProfile{localDir(0) + userProfileFileName};
	if (exportToFile(0, localProfile, true) == TP_RET_CODE_EXPORT_OK)
		static_cast<void>(sendFileToServer(localProfile, nullptr, QString{}, QString{}, userId(0)));
}

void DBUserModel::sendUserDataToServerDatabase()
{
	const QString &main_user_data_filename{localDir(0) + userLocalDataFileName};
	if (exportToFile(0, main_user_data_filename, false) == TP_RET_CODE_EXPORT_OK)
	{
		const int request_id{sendFileToServer(main_user_data_filename, nullptr, tr("Online user information updated"),
							QString{}, userId(0), true)};
		if (request_id != -1)
		{
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(this, &DBUserModel::fileUploaded, this, [this,conn,request_id] (const bool success, const int requestid)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (success)
					{
						connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,request_id] (const QString &key, const QString &value)
						{
							appOnlineServices()->updateOnlineUserInfo(request_id, key, value);
						}, Qt::SingleShotConnection);
						appKeyChain()->readKey(userId(0));
					}
				}
			});
		}
	}
}

void DBUserModel::downloadAvatarFromServer(const uint user_idx)
{
	QString avatar_file{std::move(avatar(user_idx, false))};
	if (avatar_file.startsWith("image://"_L1))
		avatar_file = std::move(localDir(user_idx));
	const int request_id{downloadFileFromServer("avatar", avatar_file, QString{}, QString{}, userId(user_idx))};
	if (request_id >= 0)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (success && user_idx != m_tempRow)
					setAvatar(user_idx, localFileName, false, false);
			}
		});
	}
}

void DBUserModel::downloadResumeFromServer(const uint user_idx)
{
	const int request_id{downloadFileFromServer("resume"_L1, resume(user_idx), QString{}, QString{}, userId(user_idx))};
	if (request_id >= 0)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (success)
					appOsInterface()->openURL(localFileName);
				else
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
						appUtils()->string_strings({tr("Error"), tr("No resumé file provided by the coach")}, record_separator));
			}
		});
	}
}

void DBUserModel::copyTempUserFilesToFinalUserDir(const QString &destDir, OnlineUserInfo *userInfo, const int userInfoRow) const
{
	QDir directory{userInfo->sourcePath()};
	if (!directory.exists(destDir))
		directory.mkdir(destDir);
	const QFileInfoList &tempuser_files{directory.entryInfoList(QStringList{} << "*.*"_L1, QDir::NoDotAndDotDot|QDir::Files)};
	for (const auto &it : tempuser_files)
		static_cast<void>(appUtils()->copyFile(it.filePath(), destDir));
}

void DBUserModel::clearTempUserFiles(OnlineUserInfo *userInfo, const int userInfoRow) const
{
	QDir directory{userInfo->sourcePath()};
	const QFileInfoList &tempuser_files{directory.entryInfoList(QStringList{} << "*.*"_L1, QDir::NoDotAndDotDot|QDir::Files)};
	for (const auto &it : tempuser_files)
	{
		if (it.fileName().startsWith(userInfo->data(userInfoRow, USER_COL_ID)))
			static_cast<void>(QFile::remove(it.filePath()));
	}
}

void DBUserModel::clearUserDir(const QString &dir) const
{
	QDir directory{dir};
	const QFileInfoList &user_files{directory.entryInfoList(QStringList{} << std::move("*.*"_L1),
												QDir::AllEntries|QDir::System|QDir::NoDotAndDotDot)};
	for (const auto &it : user_files)
	{
		if (it.isDir())
			clearUserDir(it.filePath());
		else if (it.isFile())
			static_cast<void>(QFile::remove(it.filePath()));
	}
	static_cast<void>(directory.rmdir(dir));
}

void DBUserModel::startServerPolling()
{
	m_mainTimer = new QTimer{this};
	m_mainTimer->setInterval(POLLING_INTERVAL);
	m_mainTimer->callOnTimeout([this] () { pollServer(); });
	m_mainTimer->start();

	if (isCoach(0))
	{
		m_pendingClientRequests = new OnlineUserInfo{this, USER_TOTAL_COLS};
		static_cast<void>(appUtils()->mkdir(m_dirForClientsRequests));
		static_cast<void>(appUtils()->mkdir(m_dirForCurrentClients));
	}
	if (isClient(0))
	{
		m_pendingCoachesResponses = new OnlineUserInfo{this, USER_TOTAL_COLS};
		m_availableCoaches = new OnlineUserInfo{this, USER_TOTAL_COLS};
		static_cast<void>(appUtils()->mkdir(m_dirForRequestedCoaches));
		static_cast<void>(appUtils()->mkdir(m_dirForCurrentCoaches));
	}
	pollServer();
}

void DBUserModel::pollServer()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		m_password = value;
		if (isCoach(0))
		{
			if (!mb_coachRegistered)
			{
				//poll immediatelly after receiving confirmation the man user is  a registerd coach
				connect(this, &DBUserModel::coachOnlineStatus, this, [this] (bool registered) {
					if (registered)
					{
						pollClientsRequests();
						pollCurrentClients();
					}
				}, Qt::SingleShotConnection);
				checkIfCoachRegisteredOnline();
			}
			else
			{
				if (mb_coachRegistered == true)
				{
					pollClientsRequests();
					pollCurrentClients();
				}
			}
		}
		if (isClient(0))
		{
			pollCoachesAnswers();
			pollCurrentCoaches();
			checkNewMesos();
		}
		//When single device, only query the server once per session
		if (!mb_singleDevice.has_value() || !mb_singleDevice.value())
			lastOnlineCmd(appUtils()->generateUniqueId("lastOnlineCmd"_L1), TPDatabaseTable::databaseFilesSubDir);
		else
		{
			if (!mb_singleDevice && n_cmdOrderValue != -1 )
				downloadCmdFilesFromServer(TPDatabaseTable::databaseFilesSubDir);
		}
		//checkMessages();
		//checkWorkouts();
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::pollClientsRequests()
{
	const int requestid{appUtils()->generateUniqueId("pollClientsRequests"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				QStringList requests_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (m_pendingClientRequests->sanitize(requests_list, USER_COL_ID))
					emit pendingClientsRequestsChanged();

				//First pass
				for (qsizetype i{requests_list.count() - 1}; i >= 0; --i)
				{
					const int userrow{userIdxFromFieldValue(USER_COL_ID, requests_list.at(i))};
					if (userrow != -1 && userrow != m_tempRow)
					{
						appOnlineServices()->removeClientRequest(0, userId(0), m_password, requests_list.at(i));
						requests_list.remove(i);
					}
				}

				//Second pass
				qsizetype n_connections{requests_list.count()};
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn2,requests_list,n_connections]
																(const QString &userid, const bool success) mutable
				{
					if (requests_list.contains(userid))
					{
						if (--n_connections == TP_RET_CODE_SUCCESS)
							disconnect(*conn2);
						if (success)
							addPendingClient(userid);
					}
				});
				for (qsizetype x{0}; x < requests_list.count(); ++x)
				{
					const QString &client_profile{profileFileName(m_dirForClientsRequests, requests_list.at(x))};
					getUserOnlineProfile(requests_list.at(x), client_profile);
				}
			}
		}
	});
	appOnlineServices()->checkClientsRequests(requestid, userId(0), m_password);
}

void DBUserModel::addPendingClient(const QString &user_id)
{
	if (!m_pendingClientRequests->containsUser(user_id))
	{
		const QString &client_profile{profileFileName(m_dirForClientsRequests, user_id)};
		if (m_pendingClientRequests->dataFromFileSource(client_profile))
		{
			m_pendingClientRequests->setIsCoach(m_pendingClientRequests->count()- 1, false);
			emit pendingClientsRequestsChanged();
		}
	}
}

void DBUserModel::pollCoachesAnswers()
{
	const int requestid{appUtils()->generateUniqueId("pollCoachesAnswers"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
			{
				QStringList answers_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (m_pendingCoachesResponses->sanitize(answers_list, USER_COL_ID))
					emit pendingCoachesResponsesChanged();

				//First pass
				for (qsizetype i{answers_list.count()-1}; i >= 0; --i)
				{
					QString coach_id{answers_list.at(i)};
					if (coach_id.endsWith("AOK"_L1))
					{
						coach_id.chop(3);
						const int userrow{userIdxFromFieldValue(USER_COL_ID, coach_id)};
						if (userrow != -1 && userrow != m_tempRow)
						{
							appOnlineServices()->acceptCoachAnswer(requestid, userId(0), m_password, coach_id);
							//appOnlineServices()->removeCoachAnwers(requestid, userId(0), m_password, coach_id);
							answers_list.remove(i);
						}
					}
					else //if (coach_id.endsWith("NAY"_L1))
						answers_list.remove(i);
				}

				//Second pass
				qsizetype n_connections{answers_list.count()};
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn2,answers_list,n_connections]
															(const QString &userid, const bool success) mutable
				{
					const auto &it{std::find_if(answers_list.cbegin(), answers_list.cend(), [userid] (const auto &coach) {
						return coach.startsWith(userid);
					})};
					if (it != answers_list.cend())
					{
						if (--n_connections == 0)
							disconnect(*conn2);
						if (success)
							addCoachAnswer(userid);
					}
				});
				for (auto &coach_id : answers_list)
				{
					coach_id.chop(3);
					const QString &coach_profile{profileFileName(m_dirForRequestedCoaches, coach_id)};
					getUserOnlineProfile(coach_id, coach_profile);
				}
			}
		}
	});
	appOnlineServices()->checkCoachesAnswers(requestid, userId(0), m_password);
}

void DBUserModel::addCoachAnswer(const QString &user_id)
{
	if (!m_pendingCoachesResponses->containsUser(user_id))
	{
		const QString &coach_profile{profileFileName(m_dirForRequestedCoaches, user_id)};
		if (m_pendingCoachesResponses->dataFromFileSource(coach_profile))
		{
			m_pendingCoachesResponses->setIsCoach(m_pendingCoachesResponses->count()-1, true);
			emit pendingCoachesResponsesChanged();
		}
	}
}

void DBUserModel::addAvailableCoach(const QString &user_id)
{
	if (!m_availableCoaches->containsUser(user_id))
	{
		const QString &coach_profile{profileFileName(m_onlineCoachesDir, user_id)};
		if (m_availableCoaches->dataFromFileSource(coach_profile))
		{
			m_availableCoaches->setIsCoach(m_availableCoaches->count() - 1, true);
			emit availableCoachesChanged();
		}
	}
}

void DBUserModel::pollCurrentClients()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentClients"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				QStringList clients_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				bool connected{false};
				for (qsizetype i{m_usersData.count()-1}; i >= 1 ; --i)
				{
					if (i == m_tempRow)
						continue;
					else if (!isClient(i))
						continue;
					if (!clients_list.contains(userId(i)))
					{
						if (appUseMode(i) == APP_USE_MODE_PENDING_CLIENT)
							continue;
						if (!connected)
						{
							connect(appMainWindow(), SIGNAL(removeNoLongerAvailableUser(int,bool)), this,
											SLOT(slot_removeNoLongerAvailableUser(int,bool)), Qt::UniqueConnection);
							connected = true;
						}
						QMetaObject::invokeMethod(appMainWindow(), "showUserNoLongerAvailable",
							Q_ARG(int, i),
							Q_ARG(QString, userId(i) + tr(" - unavailable")),
							Q_ARG(QString, tr("The user is no longer available as your client. If you need to know more about this, contact them to "
							"find out the reason. Remove the user from your list of clients?")));
					}
					else
					{
						if (_userName(i).last(1) == '!')
						{
							const QString &oldUserName{_userName(i)};
							setUserName(i, userNameWithoutConfirmationWarning(oldUserName));
							setAppUseMode(i, APP_USE_MODE_SINGLE_USER);
							changeClient(i, oldUserName);
						}
					}
				}
			}
		}
	});
	appOnlineServices()->checkCurrentClients(requestid, userId(0), m_password);
}

void DBUserModel::pollCurrentCoaches()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentCoaches"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				QStringList coaches_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				bool connected{false};
				for (qsizetype i{m_usersData.count()-1}; i >= 1 ; --i)
				{
					if (i == m_tempRow)
						continue;
					else if (!isCoach(i))
						continue;
					if (!coaches_list.contains(userId(i)))
					{
						if (!connected)
						{
							connect(appMainWindow(), SIGNAL(removeNoLongerAvailableUser(int,bool)), this,
											SLOT(slot_removeNoLongerAvailableUser(int,bool)), Qt::UniqueConnection);
							connected = true;
						}
						QMetaObject::invokeMethod(appMainWindow(), "showUserNoLongerAvailable",
							Q_ARG(int, i),
							Q_ARG(QString, userId(i) + tr(" - unavailable")),
							Q_ARG(QString, tr("The user is no longer available as your coach. If you need to know more about this, contact them to "
							"find out the reason. Remove the user from your list of coaches?")));
					}
				}
			}
		}
	});
	appOnlineServices()->checkCurrentCoaches(requestid, userId(0), m_password);
}

void DBUserModel::checkNewMesos()
{
	for (const auto &coach : static_cast<const QStringList>(m_coachesNames))
	{
		const QString &coach_id{appUserModel()->userIdFromFieldValue(USER_COL_NAME, coach)};
		QLatin1StringView v{QString{"checkNewMesos"_L1 + coach_id.toLatin1()}.toLatin1()};
		const int requestid{appUtils()->generateUniqueId(v)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid,coach,coach_id]
							(const int request_id, const int ret_code, const QStringList &ret_list)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS)
				{
					for (const auto &mesoFileName : ret_list)
					{
						if (!appMesoModel()->mesoPlanExists(appUtils()->getFileName(mesoFileName, true), coach_id, userId(0)))
						{
							const int id{appUtils()->idFromString(mesoFileName)};
							if (appMessagesManager()->message(id) == nullptr)
							{
								TPMessage *new_message{new TPMessage(coach + tr(" has sent you a new Exercises Program"), "message-meso"_L1, appMessagesManager())};
								new_message->setId(id);
								new_message->insertData(mesoFileName);
								new_message->insertAction(tr("View"), [=] (const QVariant &mesofile) {
												appMesoModel()->viewOnlineMeso(coach_id, mesofile.toString()); }, true);
								new_message->insertAction(tr("Delete"), [=,this] (const QVariant &mesofile) {
												appOnlineServices()->removeFile(request_id, userId(0), m_password, mesofile.toString(), mesosDir, coach_id); }, true);
								new_message->plug();
							}
						}
					}
				}
			}
		});
		appOnlineServices()->listFiles(requestid, userId(0), m_password, true, false, QString{}, mesosDir + userIdFromFieldValue(USER_COL_NAME, coach), userId(0));
	}
}

QString DBUserModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case USER_COL_SEX:
			return fieldValue == '0' ? std::move(tr("Male")) : std::move(tr("Female"));
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(record_separator, fancy_record_separator1);
		}
		case USER_COL_APP_USE_MODE:
			switch (fieldValue.at(0).toLatin1())
			{
				case '1': return tr("User");
				case '2': return tr("Coach");
				case '3': return tr("Client");
				default: return tr("Coach and Client");
			}
		default: return QString{};
	}
}

QString DBUserModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return QString::number(appUtils()->getDateFromDateString(fieldValue).toJulianDay());
		case USER_COL_SEX:
			return fieldValue == tr("Male") ? "0"_L1 : "1"_L1;
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(fancy_record_separator1, record_separator);
		}
		case USER_COL_APP_USE_MODE:
			if (fieldValue == tr("User"))
				return "1"_L1;
			else if (fieldValue == tr("Coach"))
				return "2"_L1;
			else if (fieldValue == tr("Client"))
				return "3"_L1;
			else
				return "4"_L1;
		default: return QString{};
	}
}
