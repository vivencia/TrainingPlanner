#include "usersmanager.h"

#include "dbexerciseslistmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusertable.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
#include "return_codes.h"
#include "thread_manager.h"
#include "tpdatabasetable.h"
#include "tpfilepath.h"
#include "tpimage.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/tpchat.h"
#include "online_services/tpmessagesmanager.h"
#include "online_services/tponlineservices.h"
#include "online_services/websocketserver.h"
#include "tpkeychain/tpkeychain.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTimer>

#ifndef Q_OS_ANDROID
#include "pageslistmodel.h"
#include <QThread>
#endif

#include <utility>

UsersManager *UsersManager::_appUserModel{nullptr};

constexpr QLatin1StringView local_user_data_file{"user.data"};

#ifndef QT_NO_DEBUG
#define POLLING_INTERVAL 5*60*1000 //When testing, poll according to the current debugging needs(more or less frequently, that is)
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

UsersManager::UsersManager(QObject *parent, const bool bMainUserModel) : QObject{parent}
{
	_appUserModel = this;
	REGISTER_QML_SINGLETON(UsersManager, this);

	mb_MainUserInfoChanged = false;
	m_network_msg_title = std::move(tr("TP Network"));

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		if (m_usersData.count() > 0)
			setPhoneBasedOnLocale();
		emit labelsChanged();
	});

	connect(this, &UsersManager::cmdFileCreated, this, &UsersManager::sendUnsentCmdFiles);
	connect(this, &UsersManager::userModified, this, &UsersManager::saveUserInfo);
	connect(this, &UsersManager::mainUserConfigurationFinished, this, [this] () {
		appOsInterface()->initialCheck();
		if (appItemManager()->appHomePage()) { //When -test is used, appHomePage() will be nullptr
			appItemManager()->appHomePage()->setProperty("loadMesosFromCoaches", isClient(0));
			appItemManager()->appHomePage()->setProperty("loadMesosForSelf", isClient(0));
			appItemManager()->appHomePage()->setProperty("loadMesosForClients", isCoach(0));
		}
	});
	qDebug() << "UsersManager::UsersManager running on thread: " << thread()->isMainThread();
}

void UsersManager::initUserSession()
{
	if (!m_db) {
		m_dbModelInterface = new DBModelInterfaceUser;
		m_db = new DBUserTable{};
		appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
		connect(m_db, &DBUserTable::userInfoAcquired, this, [this] (QStringList user_info, const bool all_info_acquired) {
			if (!all_info_acquired) {
				const qsizetype last_idx{m_usersData.count()};
				m_usersData.append(std::move(user_info));
				if (last_idx == 0)
					appOsInterface()->initialCheck();
			} else {
#ifndef Q_OS_ANDROID
				//Sync all the views(UserInfoListModel) relying on UsersManager with the new data
				emit userModified(0, USER_MODIFIED_SWITCHING);
#endif
				initUserSession();
			}
		});
		appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	} else {
		if (!mainUserConfigured()) {
			appItemManager()->showFirstTimeDialog();
		} else {
			if (onlineAccount()) {
#ifdef ENABLE_TPMESSAGES_MANAGER
				if (!appMessagesManager()) {
					new TPMessagesManager{this};
					connect(appMessagesManager(), &TPMessagesManager::graphicalInterfaceReady, this, [this] () {
						appMessagesManager()->readAllChats();
					}, Qt::SingleShotConnection);
					appMessagesManager()->startMessagesManager();
				} else {
					appMessagesManager()->readAllChats();
				}
#endif
				if (!appWSServer())
					new WSServer{userId(0), this};
				appOnlineServices()->connectToServer();
				const bool server_up{appOnlineServices()->serverStatus() == TP_RET_CODE_SUCCESS};
				appWSServer()->setServerStatus(server_up);
				setCanConnectToServer(server_up);
				connect(appOnlineServices(), &TPOnlineServices::onlineServicesReady, this, [this] () {
					if (!mainUserLoggedIn())
						onlineCheckIn();
				});
				connect(appOnlineServices(), &TPOnlineServices::serverStatusChanged, this, [this]
														(const uint online_status, const QString &address) {
					appWSServer()->setServerStatus(online_status != TPSERVER_NOT_REACHABLE);
					setCanConnectToServer(online_status == TP_RET_CODE_SUCCESS);
					if (m_mainTimer) {
						if (!online_status && m_mainTimer->isActive())
							m_mainTimer->stop();
						else if (online_status && !m_mainTimer->isActive())
							m_mainTimer->start();
					}
				});

			} else {
				appWSServer()->setServerStatus(false);
			}
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
			if (appItemManager()->appHomePage())
				appItemManager()->appHomePage()->setProperty("mesoModel", QVariant::fromValue(meso_model));
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

QString UsersManager::mainUserDir() const
{
	return TPFilePath::localAppFilesDir() % appSettings()->currentUser() % '/';
}

void UsersManager::setOnlineAccount(const bool online_user, const uint user_idx)
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
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
			appUtils()->string_strings({tr("Remove online account?"), message}, record_separator)), Qt::AlignCenter
			, std::move("question_"_L1), 0, std::move(tr("Yes")), std::move(tr("No")));
			setCoachPublicStatus(false);
		} else if (!onlineAccount(user_idx) && online_user) {
			onlineCheckIn();
		}
	}
	emit onlineUserChanged();
	m_usersData[0][ONLINEACCOUNT] = online_user ? '1' : '0';
	emit userModified(0, ONLINEACCOUNT);
}

void UsersManager::createMainUser(const QString &userid, const QString &name)
{
	if (m_usersData.count() == 0) {
		m_usersData.insert(0, std::move(QStringList{} << (userid.isEmpty() ? std::move(generateUniqueUserId()) : userid) <<
			QString{} << std::move("0"_L1) << name << std::move("2429630"_L1) << std::move("2"_L1) << QString{} <<
			QString{} << QString{} << QString{} << QString{} << QString{} << std::move("0"_L1)));
		static_cast<void>(appUtils()->mkdir(appUserModel()->mainUserDir()));
		setPhoneBasedOnLocale();
		appUtils()->mkdir(appUserModel()->mainUserDir() % TPUtils::previewImagesSubDir);
		emit userModified(0, USER_MODIFIED_CREATED);
	}
}

void UsersManager::removeMainUser(const bool confirm)
{
	if (!m_usersData.isEmpty()) {
		if (!confirm) {
			m_usersData.removeFirst();
		} else {
			connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this] (const uint8_t button) {
				if (button == 0)
					m_usersData.removeFirst();
				emit mainUserRemoved(button == 0);
			}, Qt::SingleShotConnection);
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_WARNING, std::move(appUtils()->string_strings(
				{tr("Remove user?"), tr("All the data for %1 will be deleted").arg(userName(0))}, record_separator)),
				Qt::AlignCenter, std::move(QString{}), -1, std::move(tr("Yes")), std::move(tr("No")));
		}
	}
}

void UsersManager::removeUser(const int user_idx, const bool remove_local, const bool remove_online)
{
	if (user_idx >= 1 && user_idx < m_usersData.count()) {
		if (onlineAccount(user_idx)) {
			if (!remove_online)
				return;
		} else {
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

int UsersManager::userIdxFromFieldValue(const uint field, const QString &value, const bool exact_match) const
{
	int user_idx{0};
	if (exact_match) {
		for (const auto &user : m_usersData) {
			if (user.at(field) == value)
				return user_idx;
			++user_idx;
		}
		return -1;
	} else {
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

const QString &UsersManager::userIdFromFieldValue(const uint field, const QString &value) const
{
	const auto &user{std::find_if(m_usersData.cbegin(), m_usersData.cend(), [field,value] (const auto &user_info) {
		return user_info.at(field) == value;
	})};
	if (user != m_usersData.cend())
		return user->at(ID);
	return m_emptyString;
}

void UsersManager::showPasswordDialogForMainUser(const int mode, QQuickItem *parent_page)
{
	const int requestid{appUtils()->idFromString(userId(0) % "showPasswordDialog"_L1)};
	QString title;
	switch (mode) {
	case QmlItemManager::DM_GET_PASSWORD:
		title = std::move(tr("TP app password"));
		connect(appItemManager(), &QmlItemManager::passwordAcquired, this, &UsersManager::checkPassword, Qt::SingleShotConnection);
		break;
	case QmlItemManager::DM_NEW_PASSWORD:
		title = std::move(tr("New password"));
		connect(appItemManager(), &QmlItemManager::passwordCreated, this, &UsersManager::setNewPassword, Qt::SingleShotConnection);
		break;
	case QmlItemManager::DM_CHANGE_PASSWORD:
		title = std::move(tr("Change password"));
		connect(appItemManager(), &QmlItemManager::passwordChanged, this, &UsersManager::checkChangedPassword, Qt::SingleShotConnection);
		break;
	default:
		Q_UNREACHABLE();
	}
	appItemManager()->showPasswordDialog(requestid, parent_page, title, tr(""), static_cast<QmlItemManager::PASSWORD_DIALOG_MODE>(mode));
}

//Set requestid to -1 when this function is in the middle of a chain of calls and not the initiator of the chain
void UsersManager::checkPassword(const bool proceed, const int requestid, const QString &password)
{
	if (!proceed) {
		disconnect(appItemManager(), &QmlItemManager::passwordAcquired, this, nullptr);
	} else {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,requestid,conn,password]
													(const bool ok, const QString &key, const QString &value) {
			if (userId(0) == key) {
				if (requestid == -1) //other functions are connected to userPasswordOK
					disconnect(*conn);
				if (ok) {
					disconnect(appItemManager(), &QmlItemManager::passwordAcquired, this, nullptr);
					emit userPasswordOK(password == value);
				} else {
					emit userPasswordOK(false);
					if (requestid != -1) {
						//The first request in the function chain is to get the password, so
						//display password dialog again, until user desists or operation is successfull
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_WRONG_PASSWORD,
												std::move(tr("The provided password is not your TP App password")));
						showPasswordDialogForMainUser(QmlItemManager::DM_GET_PASSWORD);
					}
				}
			}
		});
		appKeyChain()->readKey(userId(0));
	}
}

void UsersManager::setNewPassword(const bool proceed, const int requestid, const QString &new_password)
{
	disconnect(appItemManager(), &QmlItemManager::passwordCreated, this, nullptr);
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn =  connect(appKeyChain(), &TPKeyChain::keyStored, this, [this,requestid,conn,new_password]
											(const bool ok, const QString &key, const QString &error_string) {
		if (userId(0) == key) {
			disconnect(*conn);
			emit userPasswordOK(ok);
			if (ok) {
				appOnlineServices()->storeCredentials();
				if (onlineAccount() && !mb_userLoggedIn) {
					loginUser();
					if (requestid != -1)
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_SUCCESS, std::move(
							appUtils()->string_strings({tr("Success!"), tr("New user password saved")}, record_separator)));
				}
			} else if (requestid != -1) {
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
					appUtils()->string_strings({tr("Error! Password not set"), error_string}, record_separator)));
			}
		}
	});
	appKeyChain()->writeKey(userId(0), new_password);
}

void UsersManager::checkChangedPassword(const bool proceed, const int requestid, const QString &old_passwd, const QString &new_passwd)
{
	if (!proceed) {
		disconnect(appItemManager(), &QmlItemManager::passwordChanged, this, nullptr);
		return;
	}
	connect(this, &UsersManager::userPasswordOK, this, [=,this] (const bool password_ok) {
		if (!password_ok) {
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_WRONG_PASSWORD, std::move(tr("Unable to change "
															"password because the current password entered is wrong")));
			showPasswordDialogForMainUser(QmlItemManager::DM_CHANGE_PASSWORD); //display password dialog again, until user desists or operation is successfull
			return;
		}
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
											(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				disconnect(appItemManager(), &QmlItemManager::passwordChanged, this, nullptr); //password dialog connection
				if (ret_code == TP_RET_CODE_SUCCESS) {
					auto conn{std::make_shared<QMetaObject::Connection>()};
					*conn =  connect(appKeyChain(), &TPKeyChain::keyDeleted, this, [this,new_passwd,conn]
												(const bool ok, const QString &key, const QString &error_string) {
						if (userId(0) == key) {
							disconnect(*conn);
							if (ok) {
								setNewPassword(true, -1, new_passwd);
							} else {
								appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
								appUtils()->string_strings({tr("Error! Password not changed"), error_string}, record_separator)));
							}
						}
					});
					appKeyChain()->deleteKey(userId(0));
				} else {
					appItemManager()->displayMessageOnAppWindow(ret_code, std::move(QString{ret_string}));
					showPasswordDialogForMainUser(QmlItemManager::DM_CHANGE_PASSWORD); //display password dialog again, until user desists or operation is successfull
				}
			}
		});
		appOnlineServices()->changePassword(requestid, old_passwd, new_passwd);
	}, Qt::SingleShotConnection);
	checkPassword(true, -1, old_passwd);
}

void UsersManager::setPhone(const int user_idx, QString new_phone_prefix, const QString &new_phone)
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
	m_usersData[user_idx][PHONE] = std::move(new_phone_prefix % new_phone);
	emit userModified(user_idx, PHONE);
}

//Returns avatar.png if it exists or a defaultAvatar based on the user's sex. If the file exists check once a day
//if the local file is updated, i.e., download the user's new avatar file if the local file is older
//than the file sitting on the server. .avatar.query is updated only once a day
QString UsersManager::avatar(const int user_idx)
{
	QString local_avatar;
	if (user_idx >= 0 && user_idx < m_usersData.count()) [[likely]] {
		local_avatar = std::move(userDir(user_idx) % "avatar.png"_L1);
		if (user_idx > 0) {
			bool query_avatar_from_server{false};
			if (!QFile::exists(local_avatar)) [[unlikely]] {
				local_avatar = std::move(defaultAvatar(user_idx));
				query_avatar_from_server = true;
			} else {
				QFileInfo fi{userDir(user_idx) % ".avatar.query"_L1};
				if (fi.exists()) [[likely]] {
					const QDateTime &c_time{fi.lastModified()};
					query_avatar_from_server = c_time.daysTo(QDateTime::currentDateTime()) >= 1;
				} else {
					query_avatar_from_server = true;
				}
				if (query_avatar_from_server) {
					QFile *avatar_query{appUtils()->openFile(fi.filePath(), false, true, false, fi.exists(), true)};
					if (avatar_query) {
						avatar_query->write("1", 1);
						avatar_query->close();
						delete avatar_query;
					}
				}
			}
			if (query_avatar_from_server)
				downloadAvatarFromServer(user_idx);
		}
	}
	return local_avatar;
}

void UsersManager::setAvatar(const int user_idx, const QString &new_avatar, const bool saveToDisk, const bool upload)
{
	if (user_idx >= 0 && user_idx < m_usersData.count()) {
		if (saveToDisk && !new_avatar.isEmpty()) {
			TPImage img{nullptr};
			img.setSource(new_avatar);
			img.setHeight(256);
			img.setWidth(256);
			const QString &local_avatar{userDir(user_idx) % "avatar.png"_L1};
			img.saveToDisk(local_avatar);
		}
		emit userModified(user_idx, AVATAR);
		if (onlineAccount() && user_idx == 0 && upload)
			sendAvatarToServer();
	}
}

void UsersManager::setUserCategory(const int user_idx, const int new_category, const bool add)
{
	uint category{userCategory(user_idx)};
	const bool has_category{(category & new_category) != 0};
	if (has_category && add || !has_category && !add)
		return;

	auto change_category = [this,user_idx] (const int final_category) {
		m_usersData[user_idx][CATEGORY] = std::move(QString::number(final_category));
		emit userModified(user_idx, CATEGORY);
		emit userCategoryChanged(user_idx);
	};

	if (!has_category && add)
		change_category(category | new_category);
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
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(appUtils()->string_strings(
				{tr("Revoke coach status?"), tr("All your clients will be removed and cannot be automatically retrieved")}
				, record_separator)), Qt::AlignCenter, std::move("question_"_L1), 0, std::move(tr("Revoke")), std::move(tr("No")));
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
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
				appUtils()->string_strings({tr("Revoke client status?"), tr("All your coaches will be removed and "
				"cannot be automatically retrieved")}, record_separator)), Qt::AlignCenter, std::move("question_"_L1)
				, 0, std::move(tr("Revoke")), std::move(tr("No")));
			return;
		}
		else
			change_category(category & ~new_category);
	}
}

#ifndef Q_OS_ANDROID
void UsersManager::getOnlineUsers()
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("getAllOnlineUsers"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,requestid]
												(const int request_id, const int ret_code, const QStringList &ret_list) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (!m_onlineUsers) {
					m_onlineUsers = new UserInfoListModel{this};
					m_onlineUsers->setSelectEntireRow(true);
				} else {
					m_onlineUsers->clear();
				}
				QList<QStringList> users_data;
				auto n_users{ret_list.count()};
				for (const auto &userid : std::as_const(ret_list)) {
					const int requestid2{static_cast<int>(userid.toLong())};
					auto conn2{std::make_shared<QMetaObject::Connection>()};
					*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
						[this,conn2,requestid2,users_data,&n_users] (const int request_id, const int ret_code, const QString &ret_string) mutable {
						if (request_id == requestid2) {
							disconnect(*conn2);
							if (ret_code == TP_RET_CODE_SUCCESS) {
								users_data.append(std::move(ret_string.split('\n')));
								if (--n_users == 0) {
									m_onlineUsers->setModelData(std::move(users_data));
									emit onlineUsersChanged();
								}
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

void UsersManager::getLocalUsers()
{
//TODO
}

void UsersManager::switchUser(UserInfoListModel *user_model)
{
	if (user_model->currentRow() >= 0) {
		QString userid{user_model->currentValue(ID)};
		connect(this, &UsersManager::userSwitchPhase1Finished, this, [this,userid] (const bool success) mutable {
			if (success)
				userSwitchingActions(false, std::move(userid));
		}, Qt::SingleShotConnection);
		switchToUser(userid, user_model->currentValue(NAME));
	}
}

void UsersManager::removeUser(UserInfoListModel *user_model)
{
	const QString &userid{user_model->currentValue(ID)};
	const QLatin1StringView seed{"remove" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,userid,conn,requestid,user_model]
													(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
				appUtils()->string_strings({tr("User removal"), user_model->currentValue(NAME)
				% ret_string}, record_separator)), Qt::AlignTop|Qt::AlignHCenter, std::move(
														ret_code == TP_RET_CODE_SUCCESS ? "set-completed" : "error"));
			if (ret_code == TP_RET_CODE_SUCCESS) {
				appUtils()->rmDir(userDir(userid));
				user_model->removeCurrent();
			}
		}
	});
	appOnlineServices()->removeUser(requestid, userid);
}

void UsersManager::userSwitchingActions(const bool create, QString &&userid)
{
	mb_userLoggedIn = false;
	appSettings()->importFromUserConfig(userid);
	if (create)
		createMainUser(appSettings()->currentUser(), tr("New user"));
	initUserSession();
	appPagesListModel()->userSwitchingActions();
}
#endif

bool UsersManager::mainUserConfigured() const
{
	bool ret{false};
	if (m_usersData.count() >= 1) {
		ret = (onlineAccount(0) && !email(0).isEmpty());
		ret &= (isCoach(0) == !m_usersData.at(0).at(COACHROLE).isEmpty());
		ret &= (isClient(0) == !m_usersData.at(0).at(GOAL).isEmpty());
	}
	return ret;
}

void UsersManager::acceptUser(const uint user_idx)
{
	if (isCoach(user_idx)) {
		addCoach(user_idx); //Integrate a pending coach into the available coaches list
		if (canConnectToServer())
			appOnlineServices()->acceptCoachAnswer(0, userId(user_idx));
	}
	else {
		setUserCategory(user_idx, UC_YET_AVAILABLE, false);
		if (canConnectToServer())
			appOnlineServices()->acceptClientRequest(0, userId(user_idx));
	}
	emit userModified(user_idx, USER_MODIFIED_ACCEPTED);
}

void UsersManager::checkExistingAccount(const QString &email, const QString &password)
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("checkExistingAccount"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,password]
												(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) {
					emit userImportFromServerStatus(true, false, tr("Attempting to import user data"));
					importUserDataFromServer(ret_string, password);
				}
				else {
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_USER_DOES_NOT_EXIST);
					emit userImportFromServerStatus(false, false, ret_string);
				}
			}
		});
		appOnlineServices()->checkUserAccount(requestid, "email="_L1 + email, password);
	}
}

void UsersManager::importUserDataFromServer(const QString &userid, const QString &password)
{
	if (canConnectToServer()) {
		const int requestid{appUtils()->generateUniqueId("importFromOnlineServer"_L1)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS) {
					removeMainUser(false);
					if (importFromString(ret_string)) {
						mb_userLoggedIn = true;
						setNewPassword(true, -1, password);
						switchToUser(userid);
					}
				}
				emit userImportFromServerStatus(true, ret_code == TP_RET_CODE_SUCCESS, ret_string);
			}
		});
		appOnlineServices()->getOnlineUserData(requestid, userid);
	}
}

void UsersManager::setCoachPublicStatus(const bool bPublic)
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
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS, std::move(
								appUtils()->string_strings({tr("Coach registration"), ret_string}, record_separator)));
				}
				mb_coachRegistered = mb_coachPublic && (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS);
				emit coachOnlineStatus(mb_coachRegistered.value());
			}
		});
		appOnlineServices()->addOrRemoveCoach(requestid, mb_coachPublic);
	}
}

QString UsersManager::resume(const uint user_idx) const
{
	TPFilePath tp_filename{};
	tp_filename.setOwnerUser(userId(0));
	tp_filename.setTargetUser(userId(user_idx));
	const QDir &localFilesDir{tp_filename.filePath()};
	const QFileInfoList &files{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
	for (const auto &it: files) {
		if (it.fileName().startsWith("resume."_L1)) {
			tp_filename.setFileName(it.filePath(), true);
			break;
		}
	}
	return tp_filename.toString();
}


void UsersManager::setMainUserConfigurationFinished()
{
	if (canConnectToServer()) {
		if (!mainUserLoggedIn()) {
			onlineCheckIn();
		} else {
			if (mb_MainUserInfoChanged) {
				sendProfileToServer();
				sendUserDataToServerDatabase();
				mb_MainUserInfoChanged = false;
			}
		}
	}
	emit mainUserConfigurationFinished();
}

void UsersManager::sendRequestToCoaches(UserInfoListModel *users_list)
{
	for (auto i{0}; i < users_list->count(); ++i) {
		if (users_list->isSelected(i)) {
			const QString &coach_id{users_list->dataValue(i, ID)};
			const int requestid{appUtils()->generateUniqueId(QLatin1StringView{QString{"sendRequestToCoaches"_L1 % coach_id}.toLatin1()})};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid) {
					disconnect(*conn);
					if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) {
						const int user_idx{users_list->realRow(i)};
						setIsConfirmed(user_idx, true);
						setIsAvailable(user_idx, false);
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS, std::move(
							appUtils()->string_strings({tr("Coach contacting"), tr("Online coach contacted ")
							% users_list->dataValue(i, NAME)}, record_separator)));
					} else {
						appItemManager()->displayMessageOnAppWindow(ret_code, std::move(QString{ret_string}));
					}
				}
			});
			appOnlineServices()->sendRequestToCoach(requestid, coach_id);
		}
	}
}

void UsersManager::getOnlineCoachesList(const bool get_list_only)
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
					for (const auto &user_info : std::as_const(m_usersData)) {
						const auto idx{coaches.indexOf(user_info.at(ID))};
						if (idx >= 0)
							coaches.removeAt(idx);
					}
					qsizetype n_connections{coaches.count()};
					auto conn{std::make_shared<QMetaObject::Connection>()};
					*conn = connect(this, &UsersManager::userProfileAcquired, this, [this,conn,coaches,n_connections]
																(const QString &userid, const int ret_code) mutable {
						if (--n_connections == 0)
							disconnect(*conn);
						if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
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

int UsersManager::exportToFile(const uint user_idx, const TPFilePath &tp_filename, const bool write_header) const
{
	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	const auto ret{appUtils()->writeDataToFile(tp_filename.toString(), write_header ? appUtils()->userFileIdentifier : QString{}, m_usersData)};
	return ret;
}

int UsersManager::exportToFormattedFile(const uint user_idx, const TPFilePath &tp_filename) const
{
	const QList<uint> &export_user_idx{QList<uint>{} << user_idx};
	const QList<std::function<QString(void)>> &field_description{QList<std::function<QString(void)>>{} <<
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

	const auto ret{appUtils()->writeDataToFormattedFile(tp_filename.toString(),
					appUtils()->userFileIdentifier,
					m_usersData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_user_idx,
					QString{isCoach(user_idx) ? tr("Coach Information") : tr("Client Information") % "\n\n"_L1})
	};
	return ret;
}

int UsersManager::importFromFile(const TPFilePath &tp_filename)
{
	const auto ret{appUtils()->readDataFromFile(tp_filename.toString(), m_usersData, USER_N_FIELDS,
																						appUtils()->userFileIdentifier)};
	return ret;
}

int UsersManager::importFromFormattedFile(const TPFilePath &tp_filename)
{
	const auto ret{appUtils()->readDataFromFormattedFile(
							tp_filename.toString(),
							m_usersData,
							USER_N_FIELDS,
							appUtils()->userFileIdentifier,
							[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	return ret;
}

bool UsersManager::importFromString(const QString &user_data)
{
	QStringList modeldata{std::move(user_data.split('\n'))};
	if (modeldata.count() < USER_N_FIELDS)
		return false;
	if (modeldata.count() > USER_N_FIELDS)
		modeldata.resize(USER_N_FIELDS); //remove the password field and anything else that does not belong
	m_usersData.append(std::move(modeldata));
	emit userModified(m_usersData.count() - 1, USER_MODIFIED_IMPORTED);
	return true;
}

int UsersManager::newUserFromFile(const TPFilePath &tp_filename, const std::optional<bool> &file_formatted, uint category)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value()) {
		if (file_formatted.value())
			import_result = importFromFormattedFile(tp_filename);
		else
			import_result = importFromFile(tp_filename);
	} else {
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
	} else {
		m_usersData[user_idx][CATEGORY] = std::move(QString::number(category));
	}
	emit userModified(user_idx, CATEGORY);
	return TP_RET_CODE_IMPORT_OK;
}

void UsersManager::saveUserInfo(const uint user_idx, const uint field)
{
	if (field < USER_N_FIELDS) {
		if (user_idx == 0) {
			mb_MainUserInfoChanged = true;
			if (field == CATEGORY)
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
			m_usersData[user_idx][INSERTTIME] = std::move(generateUniqueUserId());
			m_dbModelInterface->setModified(user_idx, field);
			appThreadManager()->runAction(m_db, ThreadManager::InsertRecords);
			break;
		case USER_MODIFIED_REMOVED:
			m_dbModelInterface->setRemovalInfo(user_idx, QList<uint>{1, ID});
			appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);
			break;
		}
	}
}

void UsersManager::sendUnsentCmdFiles(const QString &dir)
{
	QFileInfoList cmd_files;
	appUtils()->scanDir(dir, cmd_files, '*' % TPDatabaseTable::cmd_file_extension);
	for (const auto &cmd_file : std::as_const(cmd_files))
		appOnlineServices()->sendCmdFileToServer(cmd_file.absoluteFilePath());
}

QString UsersManager::getPhonePart(const QString &str_phone, const bool prefix) const
{
	if (str_phone.length() > 0) {
		const qsizetype idx{str_phone.indexOf('(')};
		if (prefix) {
			return idx >= 0 ? str_phone.left(idx) : str_phone;
		} else {
			if (idx >= 0)
				return str_phone.sliced(idx, str_phone.length() - idx);
		}
	}
	return QString{};
}

void UsersManager::setPhoneBasedOnLocale()
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

inline QString UsersManager::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}

void UsersManager::onlineCheckIn()
{
	if (mainUserConfigured() && onlineAccount()) {
		connect(this, &UsersManager::userLoggedIn, this, [this] (const bool first_checkin) {
			if (first_checkin) {
				sendUserDataToServerDatabase();
				sendProfileToServer();
				sendAvatarToServer();
			}
			if (!isCoachRegistered() && mb_coachPublic)
				setCoachPublicStatus(mb_coachPublic);
			startServerPolling();
		}, Qt::SingleShotConnection);
		loginUser();
	}
}

void UsersManager::loginUser()
{
	const int requestid{appUtils()->generateUniqueId("loginUser"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
												(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			switch (ret_code) {
			case TP_RET_CODE_SUCCESS:
			case TP_RET_CODE_NO_CHANGES_SUCCESS:
				mb_userLoggedIn = true;
				emit userLoggedIn();
				break;
			case TP_RET_CODE_WRONG_PASSWORD:
				showPasswordDialogForMainUser(QmlItemManager::DM_GET_PASSWORD);
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(appUtils()->string_strings(
					{tr("Login failed"), tr("Please, type in your TraininPlanner user password")}, record_separator)));
				break;
			case TP_RET_CODE_USER_DOES_NOT_EXIST: { //User does not exist in the online database
				auto conn2{std::make_shared<QMetaObject::Connection>()};
				*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
						[this,conn2,requestid] (const int request_id, const int ret_code, const QString &ret_string) {
					if (request_id == requestid) {
						disconnect(*conn2);
						if (ret_code == TP_RET_CODE_SUCCESS) {
							mb_userLoggedIn = true;
							emit userLoggedIn(true);
						}
						appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
							appUtils()->string_strings({m_network_msg_title, ret_string}, record_separator))
							, Qt::AlignTop|Qt::AlignHCenter, std::move(ret_code == TP_RET_CODE_CUSTOM_SUCCESS
							? "set_separator"_L1 : "error"_L1));
						}
					});
					appOnlineServices()->registerUser(requestid);
				}
				break;
			default:
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_UNKNOWN_ERROR, std::move(QString{ret_string}));
				mb_userLoggedIn = false;
				break;
			}
		}
	});
	appOnlineServices()->userLogin(requestid);
}

void UsersManager::switchToUser(const QString &new_userid, const QString &test_username)
{
	QTimer *download_timeout{new QTimer{this}};
	connect(this, &UsersManager::allUserFilesDownloaded, this, [=,this] (const bool success) {
		delete download_timeout;
		if (!success) {
			#ifndef Q_OS_ANDROID
			if (!test_username.isEmpty()) {
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
					appUtils()->string_strings({ tr("User switching error"), tr("Could not download files for user ")
					% test_username}, record_separator)));
			} else
			#endif
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
					appUtils()->string_strings({tr("User switching error"), tr("Could not download files for user ")
					% new_userid}, record_separator)));
		} else {
			if (test_username.isEmpty()) {
				appSettings()->importFromUserConfig(new_userid);
				initUserSession();
			}
		}
		#ifndef Q_OS_ANDROID
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

void UsersManager::downloadAllUserFiles(const QString &userid)
{
	static int total_dirs{0};
	static int total_files{0};
	appUtils()->mkdir(userDir(userid));
	total_dirs = total_files = 0;
	auto res{appOnlineServices()->listFilesOrDirs(true, true, true, userid, QString{}, QString{}, true)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,conn,userid,res]
										(const int request_id, const int ret_code, const QStringList &ret_list) mutable {
		if (res.first == request_id) {
			disconnect(*conn);
			if (ret_code != TP_RET_CODE_SUCCESS)
				return;

			TPFilePath tp_filename;
			int total_files{0};
			for (const auto &filename : std::as_const(ret_list)) {
				tp_filename = filename;
				if (tp_filename.fileName().isEmpty()) { //directory
					if (!appUtils()->mkdir(tp_filename.filePath()))
						qDebug() << "Failed to create dir "_L1 << tp_filename.filePath();
				} else { //file
					res = appOnlineServices()->downloadFileFromServer(tp_filename);
					if (res.first) {
						++total_files;
						auto conn2{std::make_shared<QMetaObject::Connection>()};
						*conn2 = connect(appOnlineServices(), &TPOnlineServices::fileDownloaded, this, [this,conn2,res,&total_files]
								(const int ret_code, const uint requestid, const TPFilePath &local_file_name) mutable {
							if (res.second == requestid) {
								disconnect(*conn2);
								if (--total_files <= 0)
									emit allUserFilesDownloaded(true);
							}
						});
					}
				}
			}
		}
	});
}

//Only applicable to the main user that is a coach
void UsersManager::checkIfCoachRegisteredOnline()
{
	connect(this, &UsersManager::coachesListReceived, this, [this] (const QStringList &coaches_list) {
		mb_coachRegistered = coaches_list.contains(userId(0));
		emit coachOnlineStatus(mb_coachRegistered == true);
	}, Qt::SingleShotConnection);
	getOnlineCoachesList(true);
}

void UsersManager::getUserOnlineProfile(const QString &userid)
{
	TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(userid % TPUtils::TP_FILE_EXTENSION, userId(), userid)};
	const auto res{appOnlineServices()->downloadFileFromServer(*tp_filename)};
	if (res.first) {
		if (res.second == TP_RET_CODE_NO_CHANGES_SUCCESS) {
			emit userProfileAcquired(userid, TP_RET_CODE_NO_CHANGES_SUCCESS);
		} else {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::fileDownloaded, this, [=,this]
											(const int ret_code, const uint requestid, const TPFilePath &tp_filepath) {
				if (res.second == requestid) {
					disconnect(*conn);
					emit userProfileAcquired(userid, ret_code);
				}
			});
		}
	}
}

void UsersManager::sendProfileToServer()
{
	TPFilePath tp_filename{userId() % TPUtils::TP_FILE_EXTENSION, userId(), userId(), {}};
	if (exportToFile(0, tp_filename, true) == TP_RET_CODE_EXPORT_OK)
		static_cast<void>(appOnlineServices()->sendFileToServer(tp_filename));
}

void UsersManager::sendUserDataToServerDatabase()
{
	TPFilePath tp_filename{local_user_data_file, userId(), userId(), {}};
	if (exportToFile(0, tp_filename, false) == TP_RET_CODE_EXPORT_OK)
		static_cast<void>(appOnlineServices()->sendFileToServer(tp_filename, true));
}

void UsersManager::sendAvatarToServer()
{
	static_cast<void>(appOnlineServices()->sendFileToServer(*TPFilePath::newTPFilePath(avatar(0))));
}

//user_idx must always be > 0 and < total users
void UsersManager::downloadAvatarFromServer(const uint user_idx)
{
	auto tp_filename{TPFilePath::newTPFilePath("avatar.png"_L1, userId(0), userId(user_idx))};
	const auto res{appOnlineServices()->downloadFileFromServer(*tp_filename)};
	if (res.first) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::fileDownloaded, this, [=,this]
										(const int ret_code, const uint requestid, const TPFilePath &tp_filepath) {
			if (res.second == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
					setAvatar(user_idx, tp_filepath.toString(), ret_code != TP_RET_CODE_NO_CHANGES_SUCCESS, false);
			}
		});
	}
}

void UsersManager::startServerPolling()
{
	if (!m_mainTimer) {
		m_mainTimer = new QTimer{this};
		m_mainTimer->setInterval(POLLING_INTERVAL);
		m_mainTimer->callOnTimeout([this] () { pollServer(); });
		m_mainTimer->start();
		pollServer();
#ifdef ENABLE_TPMESSAGES_MANAGER
		appMessagesManager()->startMessagesPolling(userId(0));
#endif
		//checkWorkouts();
	}
	else {
		if (!m_mainTimer->isActive())
			m_mainTimer->start();
	}
}

void UsersManager::pollServer()
{
	if (isCoach(0)) {
		if (!mb_coachRegistered) {
			//poll immediatelly after receiving confirmation the man user is  a registerd coach
			connect(this, &UsersManager::coachOnlineStatus, this, [this] (bool registered) {
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

void UsersManager::pollClientsRequests()
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
				*conn2 = connect(this, &UsersManager::userProfileAcquired, this, [this,conn2,requests_list,n_connections]
														(const QString &userid, const int ret_code) mutable {
					if (requests_list.contains(userid)) {
						if (--n_connections == TP_RET_CODE_SUCCESS)
							disconnect(*conn2);
						if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
							addAvailableClient(userid);//User asked main user to be their coach. User is now available as a potential client
					}
				});
				for (const auto &clientid : std::as_const(requests_list))
					getUserOnlineProfile(clientid);
			}
		}
	});
	appOnlineServices()->checkClientsRequests(requestid);
}

void UsersManager::addAvailableClient(const QString &user_id)
{
	if (findUserById(user_id) == -1) {
		TPFilePath tp_filename{user_id % TPUtils::TP_FILE_EXTENSION, userId(), user_id, {}};
		static_cast<void>(newUserFromFile(tp_filename, false, UC_CLIENT & UC_YET_AVAILABLE));
	}
}

void UsersManager::pollCoachesAnswers()
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
					const int user_idx{userIdxFromFieldValue(ID, coach_id)};
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

void UsersManager::addAvailableCoach(const QString &user_id)
{
	if (findUserById(user_id) == -1) {
		TPFilePath tp_filename{user_id % TPUtils::TP_FILE_EXTENSION, userId(), user_id, {}};
		if (newUserFromFile(tp_filename, false, UC_YET_AVAILABLE) == TP_RET_CODE_IMPORT_OK)
			emit availableCoachesChanged();
	}
}

void UsersManager::pollCurrentClients()
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
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
						appUtils()->string_strings( {userId(i) % tr(" - unavailable"), tr("The user is no longer "
						"available as your client. If you need to know more about this, contact them to find out the "
						"reason. Remove the user from your list of clients?")}, record_separator)), Qt::AlignCenter
						, std::move("question_"_L1), 0, std::move(tr("Revoke")), std::move(tr("No")));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentClients(requestid);
}

void UsersManager::pollCurrentCoaches()
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
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
						appUtils()->string_strings({userId(i) + tr(" - unavailable")
						, tr("The user is no longer available as your coach. If you need to know more about this, "
						"contact them to find out the reason. Remove the user from your list of coaches?")}
						, record_separator)), Qt::AlignCenter, std::move("question_"_L1), 0, std::move(tr("Revoke"))
						, std::move(tr("No")));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentCoaches(requestid);
}

void UsersManager::revokeCoachStatus()
{
	for (auto i{m_usersData.count() - 1}; i >= 1; --i)
		if (isClient(i))
			removeUser(i);
}

void UsersManager::revokeClientStatus()
{
	for (qsizetype i{m_usersData.count() - 1}; i >= 1; --i)
		if (isCoach(i))
			removeUser(i);
}

void UsersManager::unregisterUser()
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
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
						appUtils()->string_strings({m_network_msg_title, ret_code == TP_RET_CODE_SUCCESS
							? tr("Online account removed") : tr("Failed to remove online account")}, record_separator))
							, Qt::AlignTop|Qt::AlignHCenter, std::move(ret_code == TP_RET_CODE_SUCCESS
							? "set-completed"_L1 : "error"_L1));
				}
			});
			appOnlineServices()->removeUser(requestid, userId(0));
		}
	});
}

void UsersManager::addCoach(const uint user_idx, const bool notify)
{
	setUserCategory(user_idx, UC_CONFIRMED, true);
	setUserCategory(0, UC_HAS_COACH, true);
	if (notify) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
			std::move(appUtils()->string_strings({tr("New coach!"), tr("Now that ") % userName(user_idx) %
			tr(" is your coach, you can send them messages using the Star Button on the Home screen") }, record_separator)),
			Qt::AlignTop|Qt::AlignHCenter, avatar(user_idx), 10000);
	}
}

void UsersManager::delCoach(const uint user_idx)
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

void UsersManager::addClient(const uint user_idx, const bool notify)
{
	setUserCategory(user_idx, UC_CONFIRMED, true);
	setUserCategory(0, UC_HAS_CLIENT, true);
	if (notify) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
			appUtils()->string_strings({tr("New client!"), tr("Now that ") % userName(user_idx) %
			tr(" is your client, you can send them messages using the Star Button on the Home screen")}, record_separator)),
			Qt::AlignTop|Qt::AlignHCenter, avatar(user_idx), 10000);
	}
}

void UsersManager::delClient(const uint user_idx)
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

QString UsersManager::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field) {
	case BIRTHDAY:
		return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
	case SEX:
		return fieldValue == '0' ? std::move(tr("Male")) : std::move(tr("Female"));
	case SOCIALMEDIA:
		{
		QString strSocial{fieldValue};
		return strSocial.replace(record_separator, fancy_record_separator1);
		}
	case CATEGORY:
		switch (fieldValue.at(0).toLatin1()) {
		case '1': return tr("User");
		case '2': return tr("Coach");
		case '3': return tr("Client");
		default: return tr("Coach and Client");
		}
	default: return QString{};
	}
}

QString UsersManager::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field) {
	case BIRTHDAY:
		return QString::number(appUtils()->dateFromString(fieldValue).toJulianDay());
	case SEX:
		return fieldValue == tr("Male") ? "0"_L1 : "1"_L1;
	case SOCIALMEDIA: {
		QString strSocial{fieldValue};
		return strSocial.replace(fancy_record_separator1, record_separator);
	}
	case CATEGORY:
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
