#include "dbusermodel.h"

#include "dbexerciseslistmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusertable.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
#include "thread_manager.h"
#include "tpdatabasetable.h"
#include "tpfilepath.h"
#include "tpimage.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/onlineuserinfo.h"
#include "online_services/tpchat.h"
#include "online_services/tpmessagesmanager.h"
#include "online_services/tponlineservices.h"
#include "online_services/websocketserver.h"
#include "tpkeychain/tpkeychain.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QQmlComponent>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>

#ifndef Q_OS_ANDROID
#include "pageslistmodel.h"
#include <QThread>
#endif

#include <utility>

DBUserModel *DBUserModel::_appUserModel{nullptr};

constexpr QLatin1StringView local_user_data_file{"user.data"};
constexpr QLatin1StringView cmd_file_extension{".cmd"};
constexpr uint file_upload_max_size{8*1024*1024};

#ifndef QT_NO_DEBUG
#define POLLING_INTERVAL 60*1000 //When testing, poll more frequently
#else
#define POLLING_INTERVAL 2*60*1000
#endif

//A non-confirmed user both has userCategory set to UC_PENDING_CLIENT and
//appended to their name an additional string containing the not allowed char '!'
static inline QString userNameWithoutConfirmationWarning(const QString &userName)
{
	const qsizetype sep_idx{userName.indexOf('!')};
	return userName.left(sep_idx-1);
}

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel) : QObject{parent}
{
	_appUserModel = this;
	REGISTER_QML_SINGLETON(DBUserModel, this);

	mb_MainUserInfoChanged = false;
	m_network_msg_title = std::move(tr("TP Network"));

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		if (m_usersData.count() > 0)
			setPhoneBasedOnLocale();
		emit labelsChanged();
	});

	connect(this, &DBUserModel::cmdFileCreated, this, &DBUserModel::sendUnsentCmdFiles);
	connect(this, &DBUserModel::userModified, this, &DBUserModel::saveUserInfo);
	connect(this, &DBUserModel::mainUserConfigurationFinished, this, [this] () {
		appOsInterface()->initialCheck();
		if (appItemManager()->AppHomePage()) { //When -test cml is used, AppHomePage() will be nullptr
			appItemManager()->AppHomePage()->setProperty("loadClientMesos", mainUserConfigured() && isCoach(0));
			appItemManager()->AppHomePage()->setProperty("loadOwnMesos", mainUserConfigured() && isClient(0));
		}
	}, Qt::SingleShotConnection);
}

QString DBUserModel::userDir(const QString &userid) const
{
	return appSettings()->currentUserDir() % userid % '/';
}

void DBUserModel::initUserSession()
{
	if (!m_db) {
		m_dbModelInterface = new DBModelInterfaceUser;
		m_db = new DBUserTable{m_dbModelInterface};
		appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
		connect(m_db, &DBUserTable::userInfoAcquired, this, [this] (QStringList user_info, const bool all_info_acquired) {
			if (!all_info_acquired) {
				const qsizetype last_idx{m_usersData.count()};
				m_usersData.append(std::move(user_info));
				if (last_idx == 0)
					appOsInterface()->initialCheck();
			}
			else {
#ifndef Q_OS_ANDROID
				//Sync all the views(OnlineUserInfo) relying on DBUserModel with the new data
				emit userModified(0, USER_MODIFIED_SWITCHING);
#endif
				initUserSession();
			}
		});
		appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	}
	else {
		if (!mainUserConfigured())
			appItemManager()->showFirstTimeDialog();
		else {
			if (onlineAccount()) {
				appItemManager()->startMessagesManager();
				if (!appWSServer())
					new ChatWSServer{userId(0), this};
				if (!appMessagesManager())
					new TPMessagesManager{this};
				const bool server_up{appOnlineServices()->serverStatus() == TP_RET_CODE_SUCCESS};
				appWSServer()->setServerStatus(server_up);
				setCanConnectToServer(server_up);
				appMessagesManager()->readAllChats();
				connect(appOnlineServices(), &TPOnlineServices::onlineServicesReady, this, [this] () {
					if (!mainUserLoggedIn())
						onlineCheckIn();
				});
				connect(appOnlineServices(), &TPOnlineServices::serverStatusChanged, this, [this]
																	(const uint online_status, const QString &address) {
					appWSServer()->setServerStatus(online_status != TP_RET_CODE_SERVER_UNREACHABLE);
					setCanConnectToServer(online_status == TP_RET_CODE_SUCCESS);
					if (m_mainTimer) {
						if (!online_status && m_mainTimer->isActive())
							m_mainTimer->stop();
						else if (online_status && !m_mainTimer->isActive())
							m_mainTimer->start();
					}
				});

			}
			else
				appWSServer()->setServerStatus(false);
			appOnlineServices()->storeCredentials();

			#ifndef Q_OS_ANDROID
			DBMesocyclesModel *meso_model{m_mesoModels.value(userId(0))};
			if (!meso_model) {
				meso_model = new DBMesocyclesModel{this};
				new PagesListModel{this};
				m_mesoModels.insert(userId(0), meso_model);
			}
			#else
				m_mesoModel = new DBMesocyclesModel{this};
				new PagesListModel{this};
			#endif
			if (appItemManager()->AppHomePage())
				appItemManager()->AppHomePage()->setProperty("mesoModel", QVariant::fromValue(meso_model));
			appMainWindow()->setProperty("appPagesModel", QVariant::fromValue(appPagesListModel()));

			appExercisesList()->initExercisesList();
			if (appSettings()->appVersion() != TP_APP_VERSION) {
				//All the code to update the database goes in here
				//updateDB(new DBMesocyclesTable{nullptr});
				//appSettings()->saveAppVersion(TP_APP_VERSION);
			}

			emit mainUserConfigurationFinished();
			emit onlineUserChanged();
			emit userIdChanged();
		}
	}
	mb_userLoggedIn = std::nullopt;
}

void DBUserModel::setOnlineAccount(const bool online_user, const uint user_idx)
{
	if (user_idx == 0 && mainUserConfigured()) {
		if (onlineAccount(user_idx) && !online_user) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this,conn] (const uint8_t button) {
				disconnect(*conn);
				if (button == 1)
					unregisterUser();
			});
			QString message{tr("If you remove your online account you'll not be able to log onto it anymore from any device.")};
			if (isCoach(0))
				message = std::move(tr("You'll not have access to your online client(s) anymore."));
			if (isClient(0))
				message += std::move(tr("You'll not have access to your online coache(s) anymore."));
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
						{tr("Remove online account?"), message}, record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Yes"), tr("No"));
			setCoachPublicStatus(false);
		}
		else if (!onlineAccount(user_idx) && online_user)
			onlineCheckIn();
	}
	emit onlineUserChanged();
	m_usersData[0][USER_FIELD_ONLINEACCOUNT] = online_user ? '1' : '0';
	emit userModified(0, USER_FIELD_ONLINEACCOUNT);
}

void DBUserModel::createMainUser(const QString &userid, const QString &name)
{
	if (m_usersData.count() == 0) {
		m_usersData.insert(0, std::move(QStringList{} << (userid.isEmpty() ? std::move(generateUniqueUserId()) : userid) <<
			QString{} << std::move("0"_L1) << name << std::move("2429630"_L1) << std::move("2"_L1) << QString{} <<
			QString{} << QString{} << QString{} << QString{} << QString{} << std::move("0"_L1)));
		static_cast<void>(appUtils()->mkdir(userDir(0)));
		setPhoneBasedOnLocale();
		appUtils()->mkdir(appSettings()->currentUserDir() % TPUtils::previewImagesSubDir);
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
	if (user_idx >= 1 && user_idx < m_usersData.count()) {
		if (onlineAccount(user_idx)) {
			if (!remove_online)
				return;
		}
		else {
			if (!remove_local)
				return;
		}

		if (isCoach(user_idx))
			delCoach(user_idx);
		else
			delClient(user_idx);
		m_usersData.remove(user_idx);
		emit userModified(user_idx, USER_MODIFIED_REMOVED);
	}
}

void DBUserModel::scanUsersSubDirs(std::pair<QList<bool>,QFileInfoList> &results, const QString &subdir, const QString &match)
{
	QDir user_dir{userDir(0)};
	const QStringList &all_dirs{user_dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot)};
	auto n_results{results.second.count()};
	for (const auto &dir: all_dirs) {
		if (dir.at(0).isDigit()) {
			appUtils()->scanDir(userDir(0) % dir % '/' % subdir, results.second, match);
			for (; n_results < results.second.count(); ++n_results)
				results.first.append(isCoach(userIdxFromFieldValue(USER_FIELD_ID, dir)));
		}
	}
}

int DBUserModel::userIdxFromFieldValue(const uint field, const QString &value, const bool exact_match) const
{
	int user_idx{0};
	if (exact_match) {
		for (const auto &user : m_usersData) {
			if (user.at(field) == value)
				return user_idx;
			++user_idx;
		}
		return -1;
	}
	else {
		std::pair<double,int> greatest_similarity{0.0,-1};
		for (const auto &user : m_usersData) {
			const double similarity{appUtils()->similarityBetweenStrings(user.at(field), value)};
			if (greatest_similarity.first < similarity) {
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
		return user->at(USER_FIELD_ID);
	return m_emptyString;
}

void DBUserModel::requestPasswordFromUser(const int id, const QString &dialog_title, const QString &dialog_message)
{
	if (!m_passwordDialogComponent) {
		m_passwordDialogComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "PasswordDialog"_L1,
																							QQmlComponent::Asynchronous};
		connect(m_passwordDialogComponent, &QQmlComponent::statusChanged, this, [=,this] (QQmlComponent::Status status) {
			requestPasswordFromUser(id, dialog_title, dialog_message);
		});
	}
	else {
		if (!m_passwordDialog) {
			switch (m_passwordDialogComponent->status()) {
			case QQmlComponent::Ready:
				m_passwordDialogComponent->disconnect();
				m_passwordDialog = m_passwordDialogComponent->create(appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_passwordDialog) {
					qDebug() << m_passwordDialogComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_passwordDialog, QQmlEngine::CppOwnership);
				m_passwordDialog->setProperty("parent", std::move(QVariant::fromValue(appItemManager()->AppHomePage())));
				connect(m_passwordDialog, SIGNAL(passwordAcquired(bool,int,QString)), this, SIGNAL(passwordAcquired(bool,int,QString)));
				requestPasswordFromUser(id, dialog_title, dialog_message);
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_passwordDialogComponent->errorString();
#endif
				return;
			}
		}
		else {
			m_passwordDialog->setProperty("request_id", std::move(QVariant{id}));
			m_passwordDialog->setProperty("title", std::move(QVariant{dialog_title}));
			m_passwordDialog->setProperty("message", std::move(QVariant{dialog_message}));
			appPagesListModel()->openPopup(m_passwordDialog, appItemManager()->AppHomePage());
		}
	}
}

void DBUserModel::checkPassword(const QString &password)
{
	setPassword(password);
	if (onlineAccount())
		loginUser();
}

void DBUserModel::setPassword(const QString &password)
{
	appKeyChain()->writeKey(userId(0), password);
}

void DBUserModel::getPassword()
{
	if (!m_usersData.isEmpty()) {
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			emit userPasswordAvailable(value);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(userId(0));
	}
}

void DBUserModel::setPhone(const int user_idx, QString new_phone_prefix, const QString &new_phone)
{
	switch (new_phone_prefix.length()) {
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
	m_usersData[user_idx][USER_FIELD_PHONE] = std::move(new_phone_prefix % new_phone);
	emit userModified(user_idx, USER_FIELD_PHONE);
}

QString DBUserModel::avatar(const uint user_idx)
{
	QString avatar_file{std::move(findAvatar(userDir(user_idx)))};
	if (avatar_file.isEmpty())
		avatar_file = std::move(defaultAvatar(user_idx));
	if (onlineAccount() && user_idx > 0)
		downloadAvatarFromServer(user_idx);
	return avatar_file;
}

void DBUserModel::setAvatar(const int user_idx, const QString &new_avatar, const bool saveToDisk, const bool upload)
{
	if (saveToDisk) {
		TPImage img{nullptr};
		img.setSource(new_avatar);
		const QString &local_avatar{userDir(user_idx) % "avatar"_L1 % appUtils()->getFileExtension(img.source(), true, ".png"_L1)};
		static_cast<void>(QFile::remove(local_avatar));
		img.saveToDisk(local_avatar);
	}
	emit userModified(user_idx, USER_FIELD_AVATAR);
	if (onlineAccount() && user_idx == 0 && upload)
		sendAvatarToServer();
}

void DBUserModel::setUserCategory(const int user_idx, const int new_category, const bool add)
{
	uint category{userCategory(user_idx)};
	const bool has_category{(category & new_category) != 0};
	if (has_category && add || !has_category && !add)
		return;

	auto change_category = [this,user_idx] (const int final_category) {
		m_usersData[user_idx][USER_FIELD_USER_CATEGORY] = std::move(QString::number(final_category));
		emit userModified(user_idx, USER_FIELD_USER_CATEGORY);
		emit userCategoryChanged(user_idx);
	};

	if (!has_category && add)
		change_category(category |= new_category);
	else {
		if (new_category == UC_COACH && isCoach(0) && (category & UC_HAS_CLIENT)) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this,
													[this,conn,category,change_category] (const uint8_t button) mutable {
				disconnect(*conn);
				if (button == 1) {
					change_category(category &= ~UC_COACH);
					revokeCoachStatus();
				}
			});
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
				{tr("Revoke coach status?"), tr("All your clients will be removed and cannot be automatically retrieved")},
				record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Revoke"), tr("No"));
			return;
		}
		else if (new_category == UC_CLIENT && isClient(0) && (category & UC_HAS_COACH)) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this,
													[this,conn,category,change_category] (const uint8_t button) mutable {
				disconnect(*conn);
				if (button == 1) {
					change_category(category &= ~UC_CLIENT);
					revokeClientStatus();
				}
			});
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
				{tr("Revoke client status?"), tr("All your coaches will be removed and cannot be automatically retrieved")},
				record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Revoke"), tr("No"));
			return;
		}
		else
			change_category(category &= ~new_category);
	}
}

#ifndef Q_OS_ANDROID
void DBUserModel::getAllOnlineUsers()
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("getAllOnlineUsers"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
												(const int request_id, const int ret_code, const QStringList &ret_list) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (!m_allUsers) {
					m_allUsers = new OnlineUserInfo{this};
					m_allUsers->setSelectEntireRow(true);
				}
				else
					m_allUsers->clear();
				for (const auto &userid : std::as_const(ret_list)) {
					const int requestid2{static_cast<int>(userid.toLong())};
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid2]
											(const int request_id, const int ret_code, const QString &ret_string) mutable {
						if (request_id == requestid2) {
							disconnect(*conn2);
							if (ret_code == TP_RET_CODE_SUCCESS) {
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
	if (m_allUsers->currentRow() >= 0) {
		QString userid{m_allUsers->allUsersData(USER_FIELD_ID).toString()};
		connect(this, &DBUserModel::userSwitchPhase1Finished, this, [this,userid] (const bool success) mutable {
			if (success)
				userSwitchingActions(false, std::move(userid));
		}, Qt::SingleShotConnection);
		switchToUser(userid, m_allUsers->allUsersData(USER_FIELD_NAME).toString());
	}
}

void DBUserModel::removeOtherUser()
{
	const QString &userid{m_allUsers->allUsersData(USER_FIELD_ID).toString()};
	const QLatin1StringView seed{"remove" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,userid,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
				appUtils()->string_strings({tr("User removal"), m_allUsers->allUsersData(USER_FIELD_NAME).toString() %
															ret_string}, record_separator), Qt::AlignTop|Qt::AlignHCenter,
																ret_code == TP_RET_CODE_SUCCESS ? "set-completed" : "error");
			if (ret_code == TP_RET_CODE_SUCCESS) {
				appUtils()->rmDir(userDir(userid));
				m_allUsers->removeUserInfo(m_allUsers->currentRow());
			}
		}
	});
	appOnlineServices()->removeUser(requestid, userid);
}

void DBUserModel::userSwitchingActions(const bool create, QString &&userid)
{
	mb_userLoggedIn = false;
	appSettings()->importFromUserConfig(userid);
	if (create)
		createMainUser(appSettings()->currentUser(), tr("New user"));
	initUserSession();

	appPagesListModel()->userSwitchingActions();
}
#endif

bool DBUserModel::mainUserConfigured() const
{
	bool ret{false};
	if (m_usersData.count() >= 1) {
		ret = (onlineAccount(0) && !email(0).isEmpty());
		ret &= (isCoach(0) == !m_usersData.at(0).at(USER_FIELD_COACHROLE).isEmpty());
		ret &= (isClient(0) == !m_usersData.at(0).at(USER_FIELD_GOAL).isEmpty());
	}
	return ret;
}

void DBUserModel::acceptUser(const uint user_idx)
{
	if (isCoach(user_idx)) {
		addCoach(user_idx); //Integrate a pending coach into the available coaches list
		if (canConnectToServer())
			appOnlineServices()->acceptCoachAnswer(0, userId(user_idx));
	}
	else {
		//Make user a pending client, i.e. now main user, the coach, is only awaiting confirmation from them that he can be their coach
		setUserCategory(user_idx, UC_PENDING_STATUS, true);
		setUserCategory(user_idx, UC_YET_AVAILABLE, false);
		if (canConnectToServer())
			appOnlineServices()->acceptClientRequest(0, userId(user_idx));
	}
	emit userModified(user_idx, USER_MODIFIED_ACCEPTED);
}

void DBUserModel::checkExistingAccount(const QString &email, const QString &password)
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("checkExistingAccount"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,password]
													(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) { //Password matches server's. Store it for the session
					m_onlineAccountId = ret_string;
					m_password = password;
				}
				emit userOnlineCheckResult(ret_code == TP_RET_CODE_SUCCESS);
			}
		});
		appOnlineServices()->checkUserAccount(requestid, "email="_L1 + email, password);
	}
}

void DBUserModel::changePassword(const QString &old_password, const QString &new_password)
{
	const int requestid{appUtils()->generateUniqueId("changePassword"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,new_password]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				appKeyChain()->deleteKey(userId(0));
				setPassword(new_password);
			}
			else
				appItemManager()->displayMessageOnAppWindow(ret_code, ret_string);
		}
	});
	appOnlineServices()->changePassword(requestid, old_password, new_password);
}

void DBUserModel::importFromOnlineServer()
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("importFromOnlineServer"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) {
					removeMainUser();
					if (importFromString(ret_string)) {
						mb_userLoggedIn = true;
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
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("setCoachPublicStatus"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) {
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS,
									appUtils()->string_strings({tr("Coach registration"), ret_string}, record_separator));
				}
				mb_coachRegistered = mb_coachPublic && (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS);
				emit coachOnlineStatus(mb_coachRegistered.value());
			}
		});
		appOnlineServices()->addOrRemoveCoach(requestid, mb_coachPublic);
	}
}

void DBUserModel::uploadResume(const QString &filename)
{
	TPFilePathPtr local_resumefile{TPFilePath::newTPFilePath()};
	local_resumefile->setFileName("resume"_L1 % appUtils()->getFileExtension(filename), true);
	local_resumefile->setBothUsers(userId());
	if (appUtils()->copyFile(appUtils()->getCorrectPath(filename), local_resumefile->toString()))
		sendFileToServer(local_resumefile, tr("Résumé uploaded successfully!"), true);
}

void DBUserModel::setMainUserConfigurationFinished()
{
	if (canConnectToServer()) {
		if (!mainUserLoggedIn())
			onlineCheckIn();
		else {
			if (mb_MainUserInfoChanged) {
				sendProfileToServer();
				sendUserDataToServerDatabase();
				mb_MainUserInfoChanged = false;
			}
		}
	}
	emit mainUserConfigurationFinished();
}

void DBUserModel::sendRequestToCoaches(OnlineUserInfo *users_list)
{
	for (auto i{0}; i < users_list->count(); ++i) {
		if (users_list->isSelected(i)) {
			const QString &coach_id{users_list->data(USER_FIELD_ID, i)};
			const int requestid{appUtils()->generateUniqueId(QLatin1StringView{QString{"sendRequestToCoaches"_L1 % coach_id}.toLatin1()})};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid) {
					disconnect(*conn);
					if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) {
						const int user_idx{users_list->currentUserIdx(i)};
						setIsConfirmed(user_idx, true);
						setIsAvailable(user_idx, false);
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS,
									appUtils()->string_strings({tr("Coach contacting"), tr("Online coach contacted ") %
																users_list->data(i, USER_FIELD_NAME)}, record_separator));
					}
					else
						appItemManager()->displayMessageOnAppWindow(ret_code, ret_string);
				}
			});
			appOnlineServices()->sendRequestToCoach(requestid, coach_id);
		}
	}
}

void DBUserModel::getOnlineCoachesList(const bool get_list_only)
{
	if (canConnectToServer() && onlineAccount()) {
		const int requestid{appUtils()->generateUniqueId("getOnlineCoachesList"_L1)};
		auto conn {std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,get_list_only]
							(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) {
					QStringList coaches{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
					if (get_list_only) {
						emit coachesListReceived(coaches);
						return;
					}
					qsizetype n_connections{coaches.count()};
					auto conn{std::make_shared<QMetaObject::Connection>()};
					*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,coaches,n_connections]
																	(const QString &userid, const bool success) mutable {
						if (--n_connections == 0)
							disconnect(*conn);
						if (success)
							addAvailableCoach(userid);
					});
					for (const auto &coach_id : std::as_const(coaches))
						getUserOnlineProfile(coach_id);
				}
			}
		});
		appOnlineServices()->getOnlineCoachesList(requestid);
	}
}

void DBUserModel::sendFileToUser(const TPFilePathPtr &tp_filename, const QVariant &extra_info,
																const QString &success_message, const bool first_attempt)
{
	const bool use_ws{appWSServer()->isConnectionOK(tp_filename->targetUser())};
	if (!use_ws && first_attempt) {
		QTimer::singleShot(5000, this, [this,tp_filename,extra_info,success_message] () {
			sendFileToUser(tp_filename, extra_info, success_message, false);
		});
		appWSServer()->connectToPeer(this, ChatWSServer::WS_TPMESSAGESMANAGER, tp_filename->targetUser());
		return;
	}

	if (use_ws)
		appWSServer()->sendBinaryMessage(ChatWSServer::WS_TPMESSAGESMANAGER, tp_filename->ownerUser(),
								tp_filename->targetUser(), extra_info.toString(), tp_filename->toString());
	else {
		appUtils()->writeBinaryFile(tp_filename->toString(), tp_filename->toString(true), false);
		sendFileToServer(tp_filename, success_message, true);
	}
}

int DBUserModel::sendFileToServer(const TPFilePathPtr &tp_filename, const QString &successMessage, const bool removeLocalFile)
{
	if (!canConnectToServer()) {
		if (!successMessage.isEmpty())
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_SERVER_UNREACHABLE);
		return TP_RET_CODE_UPLOAD_FAILED;
	}
	else {
		if (!mainUserLoggedIn())
			return TP_RET_CODE_UPLOAD_FAILED;
	}

	QFileInfo fi{tp_filename->toString()};
	if (fi.size() > file_upload_max_size) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
			appUtils()->string_strings({ tr("Cannot upload file"), tr("Maximum file size allowed: 8MB")}, record_separator));
		return TP_RET_CODE_UPLOAD_FAILED;
	}

	const int requestid{tp_filename->generateUniqueId()};

	QFile *upload_file{appUtils()->openFile(tp_filename->toString(), true, false, false, false, false)};
	if (upload_file) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				upload_file->close();
				if (removeLocalFile)
					QFile::remove(upload_file->fileName());
				if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) {
					if (!successMessage.isEmpty())
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS,
									appUtils()->string_strings({m_network_msg_title, successMessage}, record_separator));
				}
				else
					appItemManager()->displayMessageOnAppWindow(ret_code, ret_string);
				delete upload_file;
				emit fileUploaded(ret_code == TP_RET_CODE_SUCCESS, requestid);
			}
		});
		appOnlineServices()->sendFile(requestid, upload_file, tp_filename->subdirs(), tp_filename->targetUser());
	}
	return requestid;
}

int DBUserModel::downloadFileFromServer(const std::shared_ptr<TPFilePath> &tp_filename, const QString &successMessage)
{
	if (!canConnectToServer()) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_SERVER_UNREACHABLE);
		return TP_RET_CODE_DOWNLOAD_FAILED;
	}
	else {
		if (!mainUserLoggedIn())
			return TP_RET_CODE_DOWNLOAD_FAILED;
	}
	if (appUtils()->fileRecentlyModified(tp_filename->toString(), 30))
		return TP_RET_CODE_NO_CHANGES_SUCCESS;

	const int requestid{tp_filename->generateUniqueId()};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [=,this]
						(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
		if (request_id == requestid) {
			disconnect(*conn);
			bool success{!contents.isEmpty()};
			static_cast<void>(appUtils()->mkdir(tp_filename->toString()));
			switch (ret_code) {
			case TP_RET_CODE_SUCCESS: //file downloaded
				if (success) {
					QFile *local_file{new QFile{tp_filename->toString(), this}};
					if (!local_file->exists() || local_file->remove()) {
						if (local_file->open(QIODeviceBase::WriteOnly)) {
							local_file->write(contents);
							local_file->close();
						}
					}
					delete local_file;
					if (!successMessage.isEmpty())
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS, appUtils()->string_strings(
																{m_network_msg_title, successMessage}, record_separator));
				}
				break;
			case TP_RET_CODE_NO_CHANGES_SUCCESS: //online file and local file are the same
				success = true;
				break;
			default: //some error
				success = false;
			}
			if (!success && !successMessage.isEmpty())
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, appUtils()->string_strings(
																				{filename + contents}, record_separator));
			emit fileDownloaded(success, requestid, tp_filename);
		}
	});
	appOnlineServices()->getFile(requestid, tp_filename->fileName(), tp_filename->subdirs(), tp_filename->targetUser(),
																									tp_filename->toString());
	return requestid;
}

void DBUserModel::removeFileFromServer(const std::shared_ptr<TPFilePath> &tp_filename)
{
	if (!mainUserLoggedIn())
		return;
	const int requestid{tp_filename->generateUniqueId()};
	appOnlineServices()->removeFile(requestid, tp_filename->fileName(), tp_filename->subdirs(), tp_filename->targetUser());
}

int DBUserModel::listFilesFromServer(const QString &subdir, const QString &targetUser, const QString &filter)
{
	QLatin1StringView v{QString{targetUser + subdir}.toLatin1().constData()};
	const int requestid{appUtils()->generateUniqueId(v)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
												(const int request_id, const int ret_code, const QStringList &ret_list) {
		if (request_id == requestid) {
			disconnect(*conn);
			emit filesListReceived(ret_code == TP_RET_CODE_SUCCESS, requestid, ret_list);
		}
	});
	appOnlineServices()->listFiles(requestid, true, false, filter, subdir, targetUser);
	return requestid;
}

void DBUserModel::sendCmdFileToServer(const QString &cmd_filename)
{
	TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(cmd_filename)};
	const int request_id{sendFileToServer(tp_filename, QString{}, true)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBUserModel::fileUploaded, this, [this,request_id,conn,tp_filename] (const bool success, const uint requestid) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (success)
				appOnlineServices()->executeCommands(requestid, tp_filename->subdirs(), mb_singleDevice.has_value() ?
																						mb_singleDevice.value() : false);
		}
	});
}

void DBUserModel::downloadCmdFilesFromServer(const QString &subdir)
{
	if (canConnectToServer()) {
		const int request_id{listFilesFromServer(subdir, userId(0), cmd_file_extension)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &DBUserModel::filesListReceived, this, [this,conn,request_id,subdir]
												(const bool success, const int requestid, const QStringList &files_list) {
			if (requestid == request_id) {
				disconnect(*conn);
				if (!success || files_list.isEmpty())
					return;

				TPFilePath tp_filename;
				tp_filename.setBothUsers(userId());
				tp_filename.setSubdirs(subdir, true);
				for (const auto &file : std::as_const(files_list)) {
					tp_filename.setFileName(file, true);
					const int requestid2{downloadFileFromServer(TPFilePath::newTPFilePath(tp_filename))};
					auto parseCmd = [this] (const QString &cmd_file) {
						m_db->parseCmdFile(cmd_file);
						QFile::remove(cmd_file);
					};
					if (requestid2 == TP_RET_CODE_DOWNLOAD_FAILED)
						continue;
					else if (requestid2 == TP_RET_CODE_NO_CHANGES_SUCCESS) {
						parseCmd(tp_filename.toString());
						continue;
					}
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(this, &DBUserModel::fileDownloaded, this, [this,conn2,requestid2,parseCmd]
												(const bool success, const uint requestid, const QString &local_file_name) {
						if (requestid == requestid2) {
							disconnect(*conn2);
							if (success)
								parseCmd(local_file_name);
						}
					});
				}
			}
		});
	}
}

int DBUserModel::exportToFile(const uint user_idx, const TPFilePathPtr &tp_filename, const bool write_header) const
{
	QFile *out_file{appUtils()->openFile(tp_filename->toString(), false, true, false, true)};
	if (!out_file)
		return TP_RET_CODE_OPEN_WRITE_FAILED;

	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	const bool ret{appUtils()->writeDataToFile(out_file, write_header ? appUtils()->userFileIdentifier : QString{}, m_usersData)};
	out_file->close();
	return ret ? TP_RET_CODE_EXPORT_OK : TP_RET_CODE_EXPORT_FAILED;
}

int DBUserModel::exportToFormattedFile(const uint user_idx, const TPFilePathPtr &tp_filename) const
{
	QFile *out_file{appUtils()->openFile(tp_filename->toString(), false, true, false, true)};
	if (!out_file)
		return TP_RET_CODE_OPEN_CREATE_FAILED;
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
					QString{isCoach(user_idx) ? tr("Coach Information") : tr("Client Information") % "\n\n"_L1})
	)
		ret = TP_RET_CODE_EXPORT_OK;
	return ret;
}

int DBUserModel::importFromFile(const TPFilePathPtr &tp_filename)
{
	QFile *in_file{appUtils()->openFile(tp_filename->toString())};
	if (!in_file)
		return TP_RET_CODE_OPEN_READ_FAILED;

	int ret{appUtils()->readDataFromFile(in_file, m_usersData, USER_N_FIELS, appUtils()->userFileIdentifier)};
	if (ret != TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
		ret = TP_RET_CODE_IMPORT_OK;
	in_file->close();
	return ret;
}

int DBUserModel::importFromFormattedFile(const std::shared_ptr<TPFilePath> &tp_filename)
{
	QFile *in_file{appUtils()->openFile(tp_filename->toString())};
	if (!in_file)
		return TP_RET_CODE_OPEN_READ_FAILED;

	int ret{appUtils()->readDataFromFormattedFile(
							in_file,
							m_usersData,
							USER_N_FIELS,
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
	if (modeldata.count() < USER_N_FIELS)
		return false;
	if (modeldata.count() > USER_N_FIELS)
		modeldata.resize(USER_N_FIELS); //remove the password field and anything else that does not belong
	m_usersData.append(std::move(modeldata));
	emit userModified(m_usersData.count() - 1, USER_MODIFIED_IMPORTED);
	return true;
}

int DBUserModel::newUserFromFile(const TPFilePathPtr &tp_filename, const std::optional<bool> &file_formatted, uint category)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value()) {
		if (file_formatted.value())
			import_result = importFromFormattedFile(tp_filename);
		else
			import_result = importFromFile(tp_filename);
	}
	else {
		import_result = importFromFile(tp_filename);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(tp_filename);
	}
	if (import_result != TP_RET_CODE_IMPORT_OK)
		return import_result;

	auto user_idx{m_usersData.count() - 1};
	if (category == 0) {
		if (isCoach(user_idx))
			setIsClient(user_idx, false);
		setIsConfirmed(user_idx, false);
	}
	else
		m_usersData[user_idx][USER_FIELD_USER_CATEGORY] = std::move(QString::number(category));
	emit userModified(user_idx, USER_FIELD_USER_CATEGORY);
	return TP_RET_CODE_IMPORT_OK;
}

void DBUserModel::saveUserInfo(const uint user_idx, const uint field)
{
	if (field < USER_N_FIELS) {
		if (user_idx == 0) {
			mb_MainUserInfoChanged = true;
			if (field == USER_FIELD_USER_CATEGORY)
				emit userCategoryChanged(user_idx);
		}
		m_dbModelInterface->setModified(user_idx, field);
		appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField);
	}
	else {
		switch (field) {
		case USER_MODIFIED_CREATED:
		case USER_MODIFIED_IMPORTED:
		case USER_MODIFIED_ACCEPTED:
			m_usersData[user_idx][USER_FIELD_INSERTTIME] = std::move(generateUniqueUserId());
			m_dbModelInterface->setModified(user_idx, field);
			appThreadManager()->runAction(m_db, ThreadManager::InsertRecords);
			break;
		case USER_MODIFIED_REMOVED:
			m_dbModelInterface->setRemovalInfo(user_idx, QList<uint>{1, USER_FIELD_ID});
			appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);
			break;
		}
	}
}

void DBUserModel::sendUnsentCmdFiles(const QString &dir)
{
	QFileInfoList cmd_files;
	appUtils()->scanDir(dir, cmd_files, '*' % cmd_file_extension);
	for (const auto &cmd_file : std::as_const(cmd_files))
		sendCmdFileToServer(cmd_file.absoluteFilePath());
}

QString DBUserModel::getPhonePart(const QString &str_phone, const bool prefix) const
{
	if (str_phone.length() > 0) {
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
	if (phoneCountryPrefix(0).length() <= 0) {
		QString phone_country_prefix;
		switch (appSettings()->userLocaleIdx()) {
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
	if (mainUserConfigured() && onlineAccount()) {
		connect(this, &DBUserModel::userLoggedIn, this, [this] (const bool first_checkin) {
			if (first_checkin) {
				sendUserDataToServerDatabase();
				sendProfileToServer();
				sendAvatarToServer();
			}
			if (m_onlineAccountId.isEmpty()) //When importing an user from the server there is no need to perform any action online
				onlineCheckinActions();
			if (!isCoachRegistered() && mb_coachPublic)
				setCoachPublicStatus(mb_coachPublic);
			startServerPolling();
		}, Qt::SingleShotConnection);
		loginUser();
	}
}

void DBUserModel::loginUser()
{
	const int requestid{appUtils()->generateUniqueId("loginUser"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
															(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			switch (ret_code) {
			case TP_RET_CODE_SUCCESS:
				mb_userLoggedIn = true;
				emit userLoggedIn();
				break;
			case TP_RET_CODE_WRONG_PASSWORD:
				*conn = connect(this, &DBUserModel::passwordAcquired, this, [this,conn,requestid]
																(const bool proceed, const int request_id, const QString &passwd) {
					if (request_id == requestid) {
						disconnect(*conn);
						if (proceed)
							checkPassword(passwd);
					}
				});
				requestPasswordFromUser(requestid, tr("Login failed"), tr("Please, type in your TraininPlanner user password"));
				break;
			case TP_RET_CODE_USER_DOES_NOT_EXIST: //User does not exist in the online database
				{
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
									[this,conn2,requestid] (const int request_id, const int ret_code, const QString &ret_string) {
					if (request_id == requestid) {
						disconnect(*conn2);
						if (ret_code == TP_RET_CODE_SUCCESS) {
							mb_userLoggedIn = true;
							emit userLoggedIn(true);
						}
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
											{m_network_msg_title, ret_string}, record_separator), Qt::AlignTop|Qt::AlignHCenter,
																ret_code == TP_RET_CODE_CUSTOM_SUCCESS ? "set_separator" : "error");
						}
					});
					appOnlineServices()->registerUser(requestid);
				}
				break;
			default:
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_UNKNOWN_ERROR, ret_string);
				mb_userLoggedIn = false;
				break;
			}
		}
	});
	appOnlineServices()->userLogin(requestid);
}

void DBUserModel::onlineCheckinActions()
{
	if (!mb_singleDevice.has_value())
	{
		connect(this, &DBUserModel::onlineDevicesListReceived, this, [this] ()
		{
			if (!mb_singleDevice.value())
				downloadCmdFilesFromServer(m_db->subDir());
			sendUnsentCmdFiles(userDir() + m_db->subDir());
		}, Qt::SingleShotConnection);
		getOnlineDevicesList();
	}
}

void DBUserModel::getOnlineDevicesList()
{
	const int requestid{appUtils()->generateUniqueId("getOnlineDevicesList"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
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
				appOnlineServices()->addDevice(requestid, appOsInterface()->deviceID());
				++n_devices;
			}
			emit onlineDevicesListReceived();
		}
	});
	appOnlineServices()->getDevicesList(requestid);
}

void DBUserModel::switchToUser(const QString &new_userid, const QString &test_username)
{
	QTimer *download_timeout{new QTimer{this}};
	connect(this, &DBUserModel::allUserFilesDownloaded, this, [=,this] (const bool success) {
		delete download_timeout;
		if (!success) {
			#ifndef Q_OS_ANDROID
			if (!test_username.isEmpty()) {
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
										appUtils()->string_strings({ tr("User switching error"),
											tr("Could not download files for user ") % test_username}, record_separator));
			}
			else
			#endif
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, appUtils()->string_strings({
				tr("User switching error"), tr("Could not download files for user ") +  m_onlineAccountId}, record_separator));
		}
		else {
			if (test_username.isEmpty()) {
				appSettings()->importFromUserConfig(new_userid);
				initUserSession();
			}
		}
		if (test_username.isEmpty())
			emit userOnlineImportFinished(success);
		#ifndef Q_OS_ANDROID
		else
			emit userSwitchPhase1Finished(success);
		#endif

	}, Qt::SingleShotConnection);
	if (canConnectToServer()) {
		download_timeout->callOnTimeout([this] () { emit allUserFilesDownloaded(false); });
		download_timeout->start(60*1000);
		downloadAllUserFiles(new_userid);
	}
	#ifndef Q_OS_ANDROID
	else if (!test_username.isEmpty()) // maybe all the files have been previously downloaded, maybe not. This is testing, go for it
		emit userSwitchPhase1Finished(true);
	#endif
}

void DBUserModel::downloadAllUserFiles(const QString &userid)
{
	static int total_dirs{0};
	static int total_files{0};
	appUtils()->mkdir(userDir(userid));
	total_dirs = total_files = 0;
	const int requestid{static_cast<int>(userid.toLong())};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,userid,requestid]
												(const int request_id, const int ret_code, const QStringList &ret_list) {
		if (request_id == requestid) {
			disconnect(*conn);
			total_dirs = ret_list.count();
			for (const auto &dir : std::as_const(ret_list)) {
				if (ret_code != TP_RET_CODE_SUCCESS)
					return;
				QString dest_dir{std::move(userDir(userid))};
				if (dir != '.') {
					dest_dir += dir % '/';
					appUtils()->mkdir(dest_dir);
				}
				const QLatin1StringView seed{dir.toLatin1().constData()};
				const int requestid2{listFilesFromServer(dir, userid)};
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(this, &DBUserModel::filesListReceived, this, [this,conn2,requestid2,userid,dest_dir]
												(const bool success, const int requestid, const QStringList &files_list) {
					if (requestid == requestid2) {
						if (--total_dirs <= 0)
							disconnect(*conn2);
						if (!success)
							return;
						if (files_list.isEmpty()) { //All files up to date, no need to download them
							if (total_files == 0 && total_dirs == 0)
								emit allUserFilesDownloaded(true);
							return;
						}
						total_files += files_list.count();
						TPFilePath tp_filename;
						tp_filename.setOwnerUser(userId());
						tp_filename.setTargetUser(userid);
						tp_filename.setSubdirs(dest_dir, true);
						for (const auto &file : std::as_const(files_list)) {
							tp_filename.setFileName(file, true);
							const int requestid3{downloadFileFromServer(TPFilePath::newTPFilePath(tp_filename))};
							if (requestid3 == TP_RET_CODE_DOWNLOAD_FAILED)
								continue;
							else if (requestid3 == TP_RET_CODE_NO_CHANGES_SUCCESS) {
								if (--total_files <= 0)
									emit allUserFilesDownloaded(true);
								continue;
							}
							auto conn3{std::make_shared<QMetaObject::Connection>()};
							*conn3 = connect(this, &DBUserModel::fileDownloaded, this, [this,conn3,requestid3]
												(const bool success, const uint requestid, const QString &local_file_name) mutable {
								if (requestid == requestid3) {
									disconnect(*conn3);
									if (--total_files <= 0)
										emit allUserFilesDownloaded(true);
								}
							});
						}
					}
				});
			}
		}
	});
	appOnlineServices()->listDirs(requestid, QString{}, QString{}, userid, true);
}

//Only applicable to the main user that is a coach
void DBUserModel::checkIfCoachRegisteredOnline()
{
	connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
		mb_coachRegistered = coaches_list.contains(userId(0));
		emit coachOnlineStatus(mb_coachRegistered == true);
	}, Qt::SingleShotConnection);
	getOnlineCoachesList(true);
}

void DBUserModel::getUserOnlineProfile(const QString &userid)
{
	TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(userid % TPUtils::TP_FILE_EXTENSION, userId(), userid)};
	const int request_id{downloadFileFromServer(tp_filename)};
	if (request_id == TP_RET_CODE_DOWNLOAD_FAILED)
		return;
	else if (request_id == TP_RET_CODE_NO_CHANGES_SUCCESS) {
		emit userProfileAcquired(userid, true);
		return;
	}
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this]
							(const bool success, const uint requestid, const std::shared_ptr<TPFilePath> &tp_filepath) {
		if (request_id == requestid) {
			disconnect(*conn);
			emit userProfileAcquired(userid, success);
		}
	});
}

void DBUserModel::sendProfileToServer()
{
	auto tp_filename{TPFilePath::newTPFilePath(userId() % TPUtils::TP_FILE_EXTENSION, userId(), userId())};
	if (exportToFile(0, tp_filename, true) == TP_RET_CODE_EXPORT_OK)
		static_cast<void>(sendFileToServer(tp_filename));
}

void DBUserModel::sendUserDataToServerDatabase()
{
	auto tp_filename{TPFilePath::newTPFilePath(local_user_data_file, userId(), userId())};
	if (exportToFile(0, tp_filename, false) == TP_RET_CODE_EXPORT_OK)
		static_cast<void>(sendFileToServer(tp_filename, tr("Online user information updated"), true));
}

void DBUserModel::sendAvatarToServer()
{
	sendFileToServer(TPFilePath::newTPFilePath(avatar(0)));
}

QString DBUserModel::findAvatar(const QString &base_dir) const
{
	const QDir &localFilesDir{base_dir};
	const QFileInfoList &images{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
	const auto &it = std::find_if(images.cbegin(), images.cend(), [] (const auto &image_fi) {
		return image_fi.fileName().contains("avatar."_L1);
	});
	return it != images.cend() ? it->filePath() : QString{};
}

QString DBUserModel::findResume(const QString &base_dir) const
{
	const QDir &localFilesDir{base_dir};
	const QFileInfoList &files{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
	for (const auto &it: files) {
		if (it.fileName().startsWith("resume."_L1))
			return it.filePath();
	}
	return QString{};
}

void DBUserModel::downloadAvatarFromServer(const uint user_idx)
{
	QString avatar_file{std::move(findAvatar(userDir(user_idx)))};
	auto tp_filename{TPFilePath::newTPFilePath(avatar_file)};
	tp_filename->setUseFileExtension(false);
	const int request_id{downloadFileFromServer(tp_filename)};
	if (request_id == TP_RET_CODE_DOWNLOAD_FAILED)
		return;
	else if (request_id == TP_RET_CODE_NO_CHANGES_SUCCESS) {
		setAvatar(user_idx, avatar_file, true, false);
		return;
	}
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this]
								(const bool success, const uint requestid, const std::shared_ptr<TPFilePath> &tp_filepath) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (success) {
				static_cast<void>(QFile::remove(avatar_file));
				setAvatar(user_idx, tp_filepath->toString(), true, false);
			}
		}
	});
}

void DBUserModel::downloadResumeFromServer(const uint user_idx)
{
	const QString &resume_file{findResume(userDir(user_idx))};
	auto tp_filename{TPFilePath::newTPFilePath(resume_file)};
	tp_filename->setUseFileExtension(false);
	const int request_id{downloadFileFromServer(tp_filename)};
	if (request_id == TP_RET_CODE_DOWNLOAD_FAILED)
		return;
	else if (request_id == TP_RET_CODE_NO_CHANGES_SUCCESS) {
		openResume(resume_file);
		return;
	}
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBUserModel::fileDownloaded, this, [=,this]
								(const bool success, const uint requestid, const std::shared_ptr<TPFilePath> &tp_filepath) {
		if (request_id == requestid) {
			disconnect(*conn);
			tp_filename->setUseFileExtension(true);
			if (tp_filepath->toString() != resume_file)
				static_cast<void>(QFile::remove(resume_file));
			openResume(success ? tp_filepath->toString() : QString{});
		}
	});
}

void DBUserModel::openResume(const QString &filename) const
{
	if (!filename.isEmpty())
		appOsInterface()->openURL(filename);
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR,
				appUtils()->string_strings({tr("Error"), tr("No resumé file provided by the coach")}, record_separator));
}

void DBUserModel::startServerPolling()
{
	if (!m_mainTimer) {
		m_mainTimer = new QTimer{this};
		m_mainTimer->setInterval(POLLING_INTERVAL);
		m_mainTimer->callOnTimeout([this] () { pollServer(); });
		m_mainTimer->start();
		pollServer();
		appMessagesManager()->startChatMessagesPolling(userId(0));
		//checkWorkouts();
	}
	else {
		if (!m_mainTimer->isActive())
			m_mainTimer->start();
	}
}

void DBUserModel::pollServer()
{
	if (isCoach(0)) {
		if (!mb_coachRegistered) {
			//poll immediatelly after receiving confirmation the man user is  a registerd coach
			connect(this, &DBUserModel::coachOnlineStatus, this, [this] (bool registered) {
				if (registered) {
					pollClientsRequests();
					pollCurrentClients();
				}
			}, Qt::SingleShotConnection);
			checkIfCoachRegisteredOnline();
		}
		else {
			if (mb_coachRegistered == true) {
				pollClientsRequests();
				pollCurrentClients();
			}
		}
	}
	if (isClient(0)) {
		pollCoachesAnswers();
		pollCurrentCoaches();
	}
}

void DBUserModel::pollClientsRequests()
{
	const int requestid{appUtils()->generateUniqueId("pollClientsRequests"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
																(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				QStringList requests_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				qsizetype n_connections{requests_list.count()};
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn2,requests_list,n_connections]
																		(const QString &userid, const bool success) mutable {
					if (requests_list.contains(userid)) {
						if (--n_connections == TP_RET_CODE_SUCCESS)
							disconnect(*conn2);
						if (success)
							addAvailableClient(userid); //User asked main user to be their coach. User is now available as a potential client
					}
				});
				for (const auto &clientid : std::as_const(requests_list))
					getUserOnlineProfile(clientid);
			}
		}
	});
	appOnlineServices()->checkClientsRequests(requestid);
}

void DBUserModel::addAvailableClient(const QString &user_id)
{
	if (findUserById(user_id) == -1) {
		TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(user_id % TPUtils::TP_FILE_EXTENSION, userId(), user_id)};
		static_cast<void>(newUserFromFile(tp_filename, false, UC_CLIENT & UC_YET_AVAILABLE));
	}
}

void DBUserModel::pollCoachesAnswers()
{
	const int requestid{appUtils()->generateUniqueId("pollCoachesAnswers"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) {
				QStringList answers_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				for (QString coach_id : std::as_const(answers_list)) {
					const int user_idx{userIdxFromFieldValue(USER_FIELD_ID, coach_id)};
					if (user_idx != -1) {
						const bool add_coach{coach_id.endsWith("AOK"_L1)};
						coach_id.chop(3);
						if (add_coach) {
							addCoach(user_idx);
							appOnlineServices()->acceptCoachAnswer(requestid, coach_id);
						}
						else
							delCoach(user_idx);
						appOnlineServices()->removeCoachAnwers(requestid, coach_id);
					}
				}
			}
		}
	});
	appOnlineServices()->checkCoachesAnswers(requestid);
}

void DBUserModel::addAvailableCoach(const QString &user_id)
{
	if (findUserById(user_id) == -1) {
		TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(user_id % TPUtils::TP_FILE_EXTENSION, userId(), user_id)};
		if (newUserFromFile(tp_filename, false, UC_YET_AVAILABLE) == TP_RET_CODE_IMPORT_OK)
			emit availableCoachesChanged();
	}
}

void DBUserModel::pollCurrentClients()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentClients"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				const QStringList &clients_list{ret_string.split(' ', Qt::SkipEmptyParts)};
				for (qsizetype i{m_usersData.count()-1}; i >= 1 ; --i) {
					if (!isClient(i)) continue;
					if (clients_list.contains(userId(i))) {
						if (!isConfirmed(i))
							addClient(i); //Client accepted main user as coach. Remove the pending status
						continue;
					}
					*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this,conn,i]
																									(const uint8_t button) {
							disconnect(*conn);
							if (button == 1)
								removeUser(i);
					});
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
						{userId(i) + tr(" - unavailable"), tr("The user is no longer available as your client. If you need to know "
						"more about this, contact them to find out the reason. Remove the user from your list of clients?")},
						record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Revoke"), tr("No"));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentClients(requestid);
}

void DBUserModel::pollCurrentCoaches()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentCoaches"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
															(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				const QStringList &coaches_list{ret_string.split(' ', Qt::SkipEmptyParts)};
				for (auto i{m_usersData.count() - 1}; i >= 1 ; --i) {
					if (!isCoach(i)) continue;
					if (coaches_list.contains(userId(i))) continue;
					*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this,conn,i]
									(const uint8_t button) {
										disconnect(*conn);
										if (button == 1)
											removeUser(i);
									});
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
						{userId(i) + tr(" - unavailable"), tr("The user is no longer available as your coach. If you need to know "
						 "more about this, contact them to find out the reason. Remove the user from your list of coaches?")},
						record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Revoke"), tr("No"));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentCoaches(requestid);
}

void DBUserModel::revokeCoachStatus()
{
	for (auto i{m_usersData.count() - 1}; i >= 1; --i)
		if (isClient(i))
			removeUser(i);
}

void DBUserModel::revokeClientStatus()
{
	for (qsizetype i{m_usersData.count() - 1}; i >= 1; --i)
		if (isCoach(i))
			removeUser(i);
}

void DBUserModel::unregisterUser()
{
	const int requestid{appUtils()->generateUniqueId("unregisterUserOnline"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
															(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			auto conn2{std::make_shared<QMetaObject::Connection>()};
			*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid]
															(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid) {
					disconnect(*conn2);
					if (ret_code == TP_RET_CODE_SUCCESS) {
						mb_userLoggedIn = false;
						emit userLoggedOut();
					}
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
						appUtils()->string_strings({m_network_msg_title, ret_code == TP_RET_CODE_SUCCESS ?
									tr("Online account removed") : tr("Failed to remove online account")}, record_separator),
												Qt::AlignTop|Qt::AlignHCenter, ret_code == TP_RET_CODE_SUCCESS ? "set-completed" : "error");
				}
			});
			appOnlineServices()->removeUser(requestid, userId(0));
		}
	});
}

void DBUserModel::addCoach(const uint user_idx, const bool notify)
{
	setUserCategory(user_idx, UC_PENDING_STATUS, false);
	setUserCategory(0, UC_HAS_COACH, true);
	if (notify) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
			appUtils()->string_strings({tr("New coach!"), tr("Now that ") % userName(user_idx) %
			tr(" is your coach, you can send them messages using the Star Button on the Home screen") }, record_separator),
			Qt::AlignTop|Qt::AlignHCenter, avatar(user_idx), 10000);
	}
}

void DBUserModel::delCoach(const uint user_idx)
{
	if (isConfirmed(user_idx)) {
		bool has_other_coaches{false};
		for (auto i {1}; i < m_usersData.count(); ++i) {
			if (isCoach(i) && isConfirmed(i)) {
				has_other_coaches = true;
				break;
			}
		}
		if (!has_other_coaches)
			setUserCategory(0, UC_HAS_COACH, false);
	}
	else
		appOnlineServices()->rejectCoachAnswer(0, userId(user_idx));
	appUtils()->rmDir(userDir(user_idx));
	appOnlineServices()->removeCoachFromClient(0, userId(user_idx));
}

void DBUserModel::addClient(const uint user_idx, const bool notify)
{
	setUserCategory(user_idx, UC_PENDING_STATUS, false);
	setUserCategory(0, UC_HAS_CLIENT, true);
	if (notify) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
			appUtils()->string_strings({tr("New client!"), tr("Now that ") % userName(user_idx) %
			tr(" is your client, you can send them messages using the Star Button on the Home screen") }, record_separator),
			Qt::AlignTop|Qt::AlignHCenter, avatar(user_idx), 10000);
	}
}

void DBUserModel::delClient(const uint user_idx)
{
	if (isConfirmed(user_idx)) {
		bool has_other_clients{false};
		for (auto i {1}; i < m_usersData.count(); ++i) {
			if (isClient(i) && isConfirmed(i)) {
				has_other_clients = true;
				break;
			}
		}
		if (!has_other_clients)
			setUserCategory(0, UC_HAS_CLIENT, false);
	}
	else
		appOnlineServices()->rejectClientRequest(0, userId(user_idx));
	appUtils()->rmDir(userDir(user_idx));
	appOnlineServices()->removeClientFromCoach(0, userId(user_idx));
}

QString DBUserModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field) {
	case USER_FIELD_BIRTHDAY:
		return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
	case USER_FIELD_SEX:
		return fieldValue == '0' ? std::move(tr("Male")) : std::move(tr("Female"));
	case USER_FIELD_SOCIALMEDIA:
		{
		QString strSocial{fieldValue};
		return strSocial.replace(record_separator, fancy_record_separator1);
		}
	case USER_FIELD_USER_CATEGORY:
		switch (fieldValue.at(0).toLatin1()) {
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
	switch (field) {
	case USER_FIELD_BIRTHDAY:
		return QString::number(appUtils()->dateFromString(fieldValue).toJulianDay());
	case USER_FIELD_SEX:
		return fieldValue == tr("Male") ? "0"_L1 : "1"_L1;
	case USER_FIELD_SOCIALMEDIA:
		{
		QString strSocial{fieldValue};
		return strSocial.replace(fancy_record_separator1, record_separator);
		}
	case USER_FIELD_USER_CATEGORY:
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
