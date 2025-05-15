#include "dbusermodel.h"

#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
#include "tpglobals.h"
#include "tpimage.h"
#include "tputils.h"
#include "translationclass.h"
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

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

static const QString &userFileIdentifier{"0x06"_L1};
static const QLatin1StringView& userProfileFileNameName{"profile.txt"_L1};
static const QLatin1StringView& userLocalDataFileName{"user.data"_L1};
static const QString &tpNetworkTitle{qApp->tr("TP Network")};

static inline QString profileFileName(const QString &dir, const QString &userid) { return dir + userid + ".txt"_L1; }

#define POLLING_INTERVAL 1000*60
//#define POLLING_INTERVAL 1000*60*20

//A non-confirmed user both has appUseMode set to APP_USE_MODE_PENDING_CLIENT and
//appended to their name an additional string containing the not allowed char '!'
static inline QString userNameWithoutConfirmationWarning(const QString &userName)
{
	const qsizetype sep_idx{userName.indexOf('!')};
	return userName.left(sep_idx-1);
}

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: QObject{parent}, m_tempRow{-1}, m_availableCoaches{nullptr}, m_pendingClientRequests{nullptr},
		m_pendingCoachesResponses{nullptr}, mb_onlineCheckInInProgress{false}, m_mainTimer{nullptr}
{
	if (bMainUserModel)
	{
		_appUserModel = this;

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBUserModel::labelsChanged);

		m_onlineCoachesDir = std::move(appUtils()->localAppFilesDir() + "online_coaches/"_L1);
		m_dirForRequestedCoaches = std::move(appUtils()->localAppFilesDir() + "requested_coaches/"_L1);
		m_dirForClientsRequests = std::move(appUtils()->localAppFilesDir() + "clients_requests/"_L1);
		m_dirForCurrentClients = std::move(appUtils()->localAppFilesDir() + "clients/"_L1);
		m_dirForCurrentCoaches = std::move(appUtils()->localAppFilesDir() + "coaches/"_L1);
		connect(this, &DBUserModel::userModified, this, [this] (const uint user_idx, const uint field) {
			if (user_idx == 0 || field == 100)
			{
				appDBInterface()->saveUser(user_idx);
				sendUserInfoToServer();
				sendProfileToServer();
			}
		});
		connect(this, &DBUserModel::userRemoved, this, [this] (const uint user_idx) {
			if (user_idx > 0)
				appDBInterface()->removeUser(user_idx);
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


void DBUserModel::addUser(QStringList &&user_info)
{
	m_usersData.append(std::move(user_info));
	const qsizetype last_idx{m_usersData.count()-1};
	if (last_idx == 0)
	{
		//DBUserTable calls here when reading from the database. When we get the data for the main user, initialize the network connection
		static_cast<void>(onlineCheckIn());
		startServerPolling();
	}
	else
	{
		if (isCoach(0) && isClient(last_idx))
			m_clientsNames.append(_userName(last_idx));
		else if (isCoach(last_idx))
			m_coachesNames.append(_userName(last_idx));
	}
}

void DBUserModel::createMainUser()
{
	if (m_usersData.isEmpty())
	{
		m_usersData.insert(0, std::move(QStringList{} << std::move(generateUniqueUserId()) << QString{} << std::move("2424151"_L1) <<
			"2"_L1 << QString{} << QString{} << QString{} << QString{} << QString{} << QString{} << STR_ZERO << STR_ZERO << STR_ZERO));
		static_cast<void>(appUtils()->mkdir(localDir(0)));
		emit userModified(0);
	}
}

void DBUserModel::removeMainUser()
{
	if (!m_usersData.isEmpty())
		m_usersData.removeFirst();
}

void DBUserModel::removeUser(const int user_idx)
{
	if (user_idx >= 1 && user_idx < m_usersData.count())
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [this,user_idx] (const uint) {
			m_usersData.remove(user_idx);
			uint next_cur_user_idx{0};
			if (m_usersData.count() > 1)
				next_cur_user_idx = user_idx - 1;
			//setCurrentRow(next_cur_user_idx);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		emit userRemoved(user_idx);
		if (isCoach(user_idx))
			delCoach(user_idx);
		if (isClient(user_idx))
			delClient(user_idx);
		emit userModified(0);
	}
}

int DBUserModel::userIdxFromFieldValue(const uint field, const QString &value) const
{
	int user_idx{0};
	for (const auto &user : m_usersData)
	{
		if (user.at(field) == value)
			return user_idx;
		++user_idx;
	}
	return -1;
}

const QString &DBUserModel::userIdFromFieldValue(const uint field, const QString &value) const
{
	const auto &user{std::find_if(m_usersData.cbegin(), m_usersData.cend(), [field,value] (const auto user_info) {
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
		case -1: return {};
		case 0: return appUtils()->localAppFilesDir() + userId(0) + '/';
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
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

QString DBUserModel::avatar(const uint user_idx, const bool checkServer)
{
	if (user_idx < m_usersData.count() && !userId(user_idx).isEmpty())
	{
		if (checkServer && user_idx > 0)
			downloadAvatarFromServer(user_idx);
		const QString &userid{userId(user_idx)};
		const QDir &localFilesDir{localDir(user_idx)};
		const QFileInfoList &images{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
		const auto &it = std::find_if(images.cbegin(), images.cend(), [userid] (const auto image_fi) {
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

	if (user_idx == 0 && upload)
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
				sendAvatarToServer();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}
		sendAvatarToServer();
	}
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
					connect(appMainWindow(), SIGNAL(revokeCoachStatus(int,bool)), this, SLOT(slot_revokeCoachStatus(int,bool)), Qt::SingleShotConnection);
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
					connect(appMainWindow(), SIGNAL(revokeClientStatus(int,bool)), this, SLOT(slot_revokeClientStatus(int,bool)), Qt::SingleShotConnection);
					QMetaObject::invokeMethod(appMainWindow(), "showRevokeClientStatus",
						Q_ARG(int, new_use_opt),
						Q_ARG(QString, tr("Revoke client status")),
						Q_ARG(QString, tr("All your clients will be removed and cannot be automatically retrieved")));
				}
			}
			m_usersData[user_idx][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
			emit userModified(0, USER_COL_APP_USE_MODE);
		}
		else
		{
			m_usersData[user_idx][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
			emit userModified(user_idx);
		}
	}
}

void DBUserModel::addCoach(const uint user_idx)
{
	m_coachesNames.append(_userName(user_idx));
	emit coachesNamesChanged();
}

void DBUserModel::delCoach(const uint user_idx)
{
	if (user_idx > 0)
	{
		m_coachesNames.removeOne(userId(0));
		emit coachesNamesChanged();
		clearUserDir(userId(user_idx));
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,user_idx] (const QString &key, const QString &value) {
			appOnlineServices()->removeCoachFromClient(0, key, value, userId(user_idx));
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::addClient(const uint user_idx)
{
	m_clientsNames.append(_userName(user_idx));
	emit clientsNamesChanged();
}

void DBUserModel::delClient(const uint user_idx)
{
	if (user_idx > 0)
	{
		m_clientsNames.removeOne(userId(user_idx));
		emit clientsNamesChanged();
		clearUserDir(userId(user_idx));
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,user_idx] (const QString &key, const QString &value) {
			appOnlineServices()->removeClientFromCoach(0, key, value, userId(user_idx));
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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
	if (m_usersData.count() >= 1)
	{
		if (isCoach(0))
			return (!m_usersData.at(0).at(USER_COL_COACHROLE).isEmpty());
		else if (isClient(0))
			return (!m_usersData.at(0).at(USER_COL_GOAL).isEmpty());
	}
	return false;
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
			appOnlineServices()->acceptCoachAnswer(0, key, value, user_id);
		}
		else
		{
			//Only when the user confirms the coach's acceptance, can they be effectively included as client of main user
			m_usersData.last()[USER_COL_NAME] = std::move(_userName(lastidx) + tr(" !Pending confirmation!"));
			addClient(lastidx);
			appOnlineServices()->acceptClientRequest(0, key, value, user_id);
		}
		copyTempUserFilesToFinalUserDir(localDir(user_id), userInfo, userInfoRow);
		userInfo->removeUserInfo(userInfoRow, false);
		emit userModified(lastidx);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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

	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::checkUserOnline(const QString &email, const QString &password)
{
	if (appOsInterface()->tpServerOK())
	{
		const int requestid{appUtils()->generateUniqueId("checkUserOnline"_L1)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,password]
								(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == 0) //Password matches server's. Store it for the session
				{
					m_onlineUserId = ret_string;
					setPassword(password);
				}
				emit userOnlineCheckResult(ret_code == 0);
			}
		});
		appOnlineServices()->checkOnlineUser(requestid, "email="_L1 + email, password);
	}
}

void DBUserModel::changePassword(const QString &old_password, const QString &new_password)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,old_password,new_password] (const QString &key, const QString &value) {
		const int requestid{appUtils()->generateUniqueId("changePassword"_L1)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,key,new_password]
							(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == 0)
				{
					appKeyChain()->deleteKey(key);
					setPassword(new_password);
				}
				else
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
			}
		});
		appOnlineServices()->changePassword(requestid, key, old_password, new_password);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::importFromOnlineServer()
{
	if (appOsInterface()->tpServerOK())
	{
		const int requestid{appUtils()->generateUniqueId("importFromOnlineServer"_L1)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == 0)
				{
					removeMainUser();
					if (importFromString(ret_string))
					{
						downloadAvatarFromServer(m_usersData.count() - 1);
						emit userOnlineImportFinished(true);
					}
				}
				else
					emit userOnlineImportFinished(false);
			}
		});
		appOnlineServices()->getOnlineUserData(requestid, m_onlineUserId);
	}
}

void DBUserModel::setCoachPublicStatus(const bool bPublic)
{
	if (isCoach(0)) //Only applicable to the main user that is a coach
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,bPublic] () {
				setCoachPublicStatus(bPublic);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}

		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,bPublic] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("setCoachPublicStatus"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,bPublic]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
					mb_coachRegistered = ret_code == 0 && bPublic;
					emit coachOnlineStatus(mb_coachRegistered == true);
					if (!mb_coachRegistered && haveClients())
					{
						for (qsizetype i{m_clientsNames.count() - 1}; i >= 1; --i)
						{
							if (isClient(i))
								removeUser(i);
						}
					}
				}
			});
			appOnlineServices()->addOrRemoveCoach(requestid, key, value, bPublic);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::uploadResume(const QString &resumeFileName)
{
	const QString &resumeFileName_ok{appUtils()->getCorrectPath(resumeFileName)};
	QFileInfo fi{resumeFileName_ok};
	if (fi.isReadable())
	{
		const qsizetype idx{resumeFileName_ok.lastIndexOf('.')};
		const QString &extension{idx > 0 ? resumeFileName_ok.last(resumeFileName_ok.length() - idx) : QString{}};
		const QString &localResumeFilePath{appUtils()->localAppFilesDir() + "resume"_L1 + extension};
		const QString &previousResumeFilePath{resume(0)};
		if (appUtils()->copyFile(resumeFileName_ok, localResumeFilePath))
		{
			sendFileToServer(localResumeFilePath, tr("Résumé uploaded successfully!"), QString{}, userId(0), true);
			if (previousResumeFilePath != localResumeFilePath)
				removeFileFromServer(appUtils()->getFileName(previousResumeFilePath), QString{}, userId(0));
		}
	}
}

void DBUserModel::setMainUserConfigurationFinished()
{
	emit mainUserConfigurationFinished();

	if (!mainUserRegistered())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			setMainUserConfigurationFinished();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		registerUserOnline();
		return;
	}
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			setMainUserConfigurationFinished();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}
	else
	{
		sendProfileToServer();
		sendUserInfoToServer();
		startServerPolling();
	}
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
					auto conn = std::make_shared<QMetaObject::Connection>();
					*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,i]
										(const int request_id, const int ret_code, const QString &ret_string) {
						if (request_id == requestid)
						{
							disconnect(*conn);
							if (ret_code == 0)
							{
								const QString &coach_id{m_availableCoaches->data(i, USER_COL_ID)};
								if (appUtils()->copyFile(profileFileName(m_onlineCoachesDir, coach_id), profileFileName(m_dirForRequestedCoaches, coach_id)))
								{
									appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach contacting") +
										record_separator + tr("Online coach contacted ") + m_availableCoaches->data(i, USER_COL_NAME));
								}
							}
							else
								appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
						}
					});
					appOnlineServices()->sendRequestToCoach(requestid, key, value, m_availableCoaches->data(i, USER_COL_ID));
				}
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::getOnlineCoachesList(const bool get_list_only)
{
	if (onlineCheckIn())
	{
		static_cast<void>(appUtils()->mkdir(m_onlineCoachesDir));
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,get_list_only] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("getOnlineCoachesList"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,get_list_only]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (ret_code == 0)
					{
						QStringList coaches{std::move(ret_string.split(' '))};
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
						auto conn = std::make_shared<QMetaObject::Connection>();
						*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,coaches,n_connections]
																				(const QString &userid, const bool success) mutable {
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
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::sendFileToServer(const QString &filename, const QString &successMessage, const QString &subdir, const QString &targetUser,
									const bool removeLocalFile)
{
	if (!onlineCheckIn())
	{
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Online server unavailable. Tray again later"));
		return;
	}

	QFile *upload_file{appUtils()->openFile(filename, QIODeviceBase::ReadOnly)};
	if (upload_file)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId(QLatin1StringView{QString{
								"sendFileToServer"_L1 + std::move(appUtils()->getFileName(filename).toLatin1())}.toLatin1()})};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (removeLocalFile)
						QFile::remove(filename);
					else
						upload_file->close();

					if (ret_code == 0 && !successMessage.isEmpty())
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + successMessage);
					else
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
					delete upload_file;
				}
			});
			appOnlineServices()->sendFile(requestid, key, value, upload_file, subdir, targetUser);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
	else
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tr("Failed to open ") + filename);
}

int DBUserModel::downloadFileFromServer(const QString &filename, const QString &localFile, const QString &successMessage,
											const QString &subdir, const QString &targetUser)
{
	if (!onlineCheckIn())
	{
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Online server unavailable. Tray again later"));
		return -1;
	}

	QLatin1StringView v{filename.toLatin1().constData()};
	const int requestid{appUtils()->generateUniqueId(v)};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [=,this]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				switch (ret_code)
				{
					case 0: //file downloaded
					{
						QString destDir;
						if (localFile.isEmpty())
							destDir = std::move(appUtils()->localAppFilesDir());
						else
						{
							destDir = std::move(appUtils()->getFilePath(localFile));
							static_cast<void>(appUtils()->mkdir(destDir));
						}

						const QString &localFileName{destDir + filename};
						QFile *localFile{new QFile{localFileName, this}};
						if (!localFile->exists() || localFile->remove())
						{
							if (localFile->open(QIODeviceBase::WriteOnly))
							{
								localFile->write(contents);
								localFile->close();
							}
						}
						delete localFile;
						emit fileDownloaded(true, requestid, localFileName);
						if (!successMessage.isEmpty())
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + successMessage);
					}
					break;
					case 1: //online file and local file are the same
						emit fileDownloaded(true, requestid, localFile);
					break;
					default: //some error
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
						emit fileDownloaded(false, requestid, localFile);
				}
			}
		});
		appOnlineServices()->getFile(requestid, key, value, filename, subdir, targetUser, localFile);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(userId(0));
	return requestid;
}

void DBUserModel::removeFileFromServer(const QString &filename, const QString &subdir, const QString &targetUser)
{
	QLatin1StringView v{filename.toLatin1().constData()};
	const int requestid{appUtils()->generateUniqueId(v)};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value) {
		appOnlineServices()->removeFile(requestid, key, value, filename, subdir, targetUser);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(userId(0));
}

int DBUserModel::exportToFile(const uint user_idx, const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text);
		if (!out_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	const bool ret{appUtils()->writeDataToFile(out_file, userFileIdentifier, m_usersData)};
	out_file->close();
	return ret ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_EXPORT_FAILED;
}

int DBUserModel::exportToFormattedFile(const uint user_idx, const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = {appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
		if (!out_file)
			return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;
	}
	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	QList<std::function<QString(void)>> field_description{QList<std::function<QString(void)>>{} <<
											nullptr <<
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

	int ret{APPWINDOW_MSG_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					userFileIdentifier,
					m_usersData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_user_idx,
					QString{isCoach(user_idx) ? tr("Coach Information") : tr("Client Information") + "\n\n"_L1})
	)
		ret = APPWINDOW_MSG_EXPORT_OK;
	return ret;
}

int DBUserModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	int ret{appUtils()->readDataFromFile(in_file, m_usersData, USER_TOTAL_COLS, userFileIdentifier)};
	if (ret != APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE)
		ret = APPWINDOW_MSG_IMPORT_OK;
	in_file->close();
	return ret;
}

int DBUserModel::importFromFormattedFile(const QString &filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	int ret{appUtils()->readDataFromFormattedFile(in_file,
												m_usersData,
												USER_TOTAL_COLS,
												userFileIdentifier,
												[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	if (ret > 0)
		ret = APPWINDOW_MSG_IMPORT_OK;
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
	emit userModified(m_usersData.count() - 1);
	return true;
}

void DBUserModel::getPasswordFromUserInput(const int resultCode, const QString &password)
{
	if (resultCode == 0)
	{
		connect(appKeyChain(), &TPKeyChain::keyStored, this, [this] (const QString &key) {
			registerUserOnline();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		setPassword(password);
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
		m_usersData[0][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
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
		m_usersData[0][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
		emit userModified(0, USER_COL_APP_USE_MODE);
	}
}

bool DBUserModel::onlineCheckIn()
{
	if (!appOsInterface()->tpServerOK() && !appOsInterface()->internetConnectionCheckInPlace())
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn =  connect(appOsInterface(), &OSInterface::networkStatusChanged, this, [this,conn] () {
			if (appOsInterface()->tpServerOK())
			{
				disconnect(*conn);
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Connected to server"));
				onlineCheckIn();
			}
		});
		appOsInterface()->checkInternetConnection();
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tpNetworkTitle + record_separator + tr("Server unreachable"));
		return false;
	}
	else
	{
		if (!mb_userRegistered && !mb_onlineCheckInInProgress)
		{
			mb_onlineCheckInInProgress = true;
			registerUserOnline();
		}
		return mb_userRegistered == true;
	}
}

void DBUserModel::registerUserOnline()
{
	if (mainUserConfigured())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("registerUserOnline"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,key,value]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					mb_onlineCheckInInProgress = false;
					switch (ret_code)
					{
						case 0:
							mb_userRegistered = true;
							emit mainUserOnlineCheckInChanged();
						break;
						case 3:
							connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), this, SLOT(getPasswordFromUserInput(int,QString)));
						break;
						case 6: //User does not exist in the online database
						{
							auto conn2 = std::make_shared<QMetaObject::Connection>();
							*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid]
														(const int request_id, const int ret_code, const QString &ret_string) {
								if (request_id == requestid)
								{
									disconnect(*conn2);
									if (ret_code == 0)
									{
										mb_userRegistered = true;
										emit mainUserOnlineCheckInChanged();
									}
									appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("User information updated"));
								}
							});
							appOnlineServices()->registerUser(requestid, key, value);
						}
						break;
						default:
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
							mb_userRegistered = false;
						break;
					}
				}
			});
			appOnlineServices()->checkUser(requestid, key, value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(userId(0));
	}
}

inline QString DBUserModel::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}

QString DBUserModel::resume(const uint user_idx) const
{
	if (user_idx < m_usersData.count() && !userId(user_idx).isEmpty())
	{
		const QString &userid{userId(user_idx)};
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
	connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
		mb_coachRegistered = coaches_list.contains(userId(0));
		emit coachOnlineStatus(mb_coachRegistered == true);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	getOnlineCoachesList(true);
}

void DBUserModel::getUserOnlineProfile(const QString &netID, const QString &save_as_filename)
{
	const int request_id{downloadFileFromServer(userProfileFileNameName, save_as_filename, QString{}, QString{}, netID)};
	if (request_id != -1)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				emit userProfileAcquired(netID, success);
			}
		});
	}
}

void DBUserModel::sendProfileToServer()
{
	const QString &localProfile{localDir(0) + userProfileFileNameName};
	if (exportToFile(0, localProfile) == APPWINDOW_MSG_EXPORT_OK)
		sendFileToServer(localProfile, QString{}, QString{}, userId(0));
}

void DBUserModel::sendUserInfoToServer()
{
	const QString &localUserData{localDir(0) + userLocalDataFileName};
	if (exportToFile(0, localUserData) == APPWINDOW_MSG_EXPORT_OK)
		sendFileToServer(localUserData, tr("Online user information updated"), QString{}, userId(0), true);
}

void DBUserModel::downloadAvatarFromServer(const uint user_idx)
{
	QString avatar_file{std::move(avatar(user_idx, false))};
	if (avatar_file.startsWith("image://"_L1))
		avatar_file = std::move(localDir(user_idx));
	const int request_id{downloadFileFromServer("avatar", avatar_file, QString{}, QString{}, userId(user_idx))};
	if (request_id != -1)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (success)
					setAvatar(user_idx, localFileName, false, false);
			}
		});
	}
}

void DBUserModel::downloadResumeFromServer(const uint user_idx)
{
	const int request_id{downloadFileFromServer(userId(user_idx) + "_resume", resume(user_idx), QString{}, QString{}, userId(user_idx))};
	if (request_id != -1)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this] (const bool success, const uint requestid, const QString &localFileName) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (success)
					appOsInterface()->openURL(localFileName);
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
	const QFileInfoList &user_files{directory.entryInfoList(QStringList{} << "*.*"_L1, QDir::AllEntries|QDir::System|QDir::NoDotAndDotDot)};
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
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			startServerPolling();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	if (!m_mainTimer)
	{
		m_mainTimer = new QTimer{this};
		m_mainTimer->setInterval(POLLING_INTERVAL);
		m_mainTimer->callOnTimeout([this] () { pollServer(); });
		m_mainTimer->start();

		if (isCoach(0))
		{
			m_pendingClientRequests = new OnlineUserInfo{this};
			static_cast<void>(appUtils()->mkdir(m_dirForClientsRequests));
			static_cast<void>(appUtils()->mkdir(m_dirForCurrentClients));
		}
		if (isClient(0))
		{
			m_pendingCoachesResponses = new OnlineUserInfo{this};
			m_availableCoaches = new OnlineUserInfo{this};
			static_cast<void>(appUtils()->mkdir(m_dirForRequestedCoaches));
			static_cast<void>(appUtils()->mkdir(m_dirForCurrentCoaches));
		}
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
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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
		//checkMessages();
		//checkWorkouts();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(userId(0));
}

void DBUserModel::pollClientsRequests()
{
	const int requestid{appUtils()->generateUniqueId("pollClientsRequests"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				QStringList requests_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (m_pendingClientRequests->sanitize(requests_list, USER_COL_ID))
					emit pendingClientsRequestsChanged();

				//First pass
				for (qsizetype i{requests_list.count()-1}; i >= 0; --i)
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
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,requests_list,n_connections]
																				(const QString &userid, const bool success) mutable {
					if (requests_list.contains(userid))
					{
						if (--n_connections == 0)
							disconnect(*conn);
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
		const QString &client_dir{m_dirForClientsRequests + user_id + '/'};
		static_cast<void>(appUtils()->mkdir(client_dir));
		const QString &client_profile{client_dir + "profile.txt"_L1};
		if (m_pendingClientRequests->dataFromFileSource(client_profile, user_id))
		{
			m_pendingClientRequests->setIsCoach(m_pendingClientRequests->count()-1, false);
			emit pendingClientsRequestsChanged();
		}
	}
}

void DBUserModel::pollCoachesAnswers()
{
	const int requestid{appUtils()->generateUniqueId("pollCoachesAnswers"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
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
							appOnlineServices()->removeCoachAnwers(requestid, userId(0), m_password, coach_id);
							answers_list.remove(i);
						}
					}
					else //if (coach_id.endsWith("NAY"_L1))
						answers_list.remove(i);
				}

				//Second pass
				qsizetype n_connections{answers_list.count()};
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,answers_list,n_connections]
																			(const QString &userid, const bool success) mutable {
					const auto &it = std::find_if(answers_list.cbegin(), answers_list.cend(), [userid] (const auto coach) {
						return coach.startsWith(userid);
					});
					if (it != answers_list.cend())
					{
						if (--n_connections == 0)
							disconnect(*conn);
						if (success)
							addCoachAnswer(userid);
					}
				});
				for (qsizetype x{0}; x < answers_list.count(); ++x)
				{
					QString coach_id{std::move(answers_list.at(x))};
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
		const QString &coach_dir{m_dirForRequestedCoaches + user_id + '/'};
		static_cast<void>(appUtils()->mkdir(coach_dir));
		const QString &coach_profile{coach_dir + "profile.txt"_L1};
		if (m_pendingCoachesResponses->dataFromFileSource(coach_profile, user_id))
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
		const QString &coach_dir{m_onlineCoachesDir + user_id + '/'};
		static_cast<void>(appUtils()->mkdir(coach_dir));
		const QString &coach_profile{coach_dir + "profile.txt"_L1};
		if (m_availableCoaches->dataFromFileSource(coach_profile, user_id))
		{
			m_availableCoaches->setIsCoach(m_availableCoaches->count()-1, true);
			emit availableCoachesChanged();
		}
	}
}

void DBUserModel::pollCurrentClients()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentClients"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
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
							const QString oldUserName{_userName(i)};
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
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
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
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid,coach,coach_id]
							(const int request_id, const int ret_code, const QStringList &ret_list) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == 0)
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
		appOnlineServices()->listFiles(requestid, userId(0), m_password, mesosDir + userIdFromFieldValue(USER_COL_NAME, coach), userId(0));
	}
}

QString DBUserModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case USER_COL_SEX:
			return fieldValue == STR_ZERO ? std::move(tr("Male")) : std::move(tr("Female"));
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
			return fieldValue == tr("Male") ? STR_ZERO : STR_ONE;
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
