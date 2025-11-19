#pragma once

#include "dbmodelinterface.h"
#include "tputils.h"
#include "online_services/onlineuserinfo.h"

//Everytime these indexes change, look in the QML files for Connections to onUserModified to see if the hardcoded field
//numbers in those functions are affected by the changes here
#define USER_COL_ID 0
#define USER_COL_INSERTTIME 1
#define USER_COL_ONLINEACCOUNT 2
#define USER_COL_NAME 3
#define USER_COL_BIRTHDAY 4
#define USER_COL_SEX 5
#define USER_COL_PHONE 6
#define USER_COL_EMAIL 7
#define USER_COL_SOCIALMEDIA 8
#define USER_COL_USERROLE 9
#define USER_COL_COACHROLE 10
#define USER_COL_GOAL 11
#define USER_COL_APP_USE_MODE 12
#define USER_TOTAL_COLS USER_COL_APP_USE_MODE + 1

#define APP_USE_MODE_SINGLE_USER 1
#define APP_USE_MODE_SINGLE_COACH 2
#define APP_USE_MODE_SINGLE_USER_WITH_COACH 3
#define APP_USE_MODE_COACH_USER_WITH_COACH 4
#define APP_USE_MODE_PENDING_CLIENT 5

#define USER_COL_AVATAR 20 //not in database, but used on model and GUI operations

#define USER_MODIFIED_CREATED 100
#define USER_MODIFIED_IMPORTED 101
#define USER_MODIFIED_REMOVED 102
#define USER_MODIFIED_ACCEPTED 103

QT_FORWARD_DECLARE_CLASS(DBUserTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceUser);
QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(QTimer)

class DBUserModel : public QObject
{

Q_OBJECT

Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged FINAL)
Q_PROPERTY(QString onlineAccountUserLabel READ onlineAccountUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString passwordLabel READ passwordLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString birthdayLabel READ birthdayLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString sexLabel READ sexLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString phoneLabel READ phoneLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString emailLabel READ emailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString socialMediaLabel READ socialMediaLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString userRoleLabel READ userRoleLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString coachRoleLabel READ coachRoleLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString goalLabel READ goalLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString avatarLabel READ avatarLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString appUseModelLabel READ appUseModelLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString newUserLabel READ newUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString existingUserLabel READ existingUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString invalidEmailLabel READ invalidEmailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString invalidPasswordLabel READ invalidPasswordLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString checkEmailLabel READ checkEmailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString importUserLabel READ importUserLabel NOTIFY labelsChanged FINAL)

Q_PROPERTY(OnlineUserInfo *availableCoaches READ availableCoaches NOTIFY availableCoachesChanged FINAL)
Q_PROPERTY(OnlineUserInfo *pendingCoachesResponses READ pendingCoachesResponses NOTIFY pendingCoachesResponsesChanged FINAL)
Q_PROPERTY(OnlineUserInfo *pendingClientsRequests READ pendingClientsRequests NOTIFY pendingClientsRequestsChanged FINAL)
Q_PROPERTY(OnlineUserInfo *currentCoaches READ currentCoaches NOTIFY currentCoachesChanged FINAL)
Q_PROPERTY(OnlineUserInfo *currentClients READ currentClients NOTIFY currentClientsChanged FINAL)
Q_PROPERTY(OnlineUserInfo *currentCoachesAndClients READ currentCoachesAndClients NOTIFY currentCoachesAndClientsChanged FINAL)
Q_PROPERTY(bool mainUserIsClient READ mainUserIsClient NOTIFY appUseModeChanged FINAL)
Q_PROPERTY(bool mainUserIsCoach READ mainUserIsCoach NOTIFY appUseModeChanged FINAL)
Q_PROPERTY(bool onlineAccount READ onlineAccount WRITE setOnlineAccount NOTIFY onlineUserChanged FINAL)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured NOTIFY mainUserConfigurationFinished FINAL)
Q_PROPERTY(bool canConnectToServer READ canConnectToServer NOTIFY canConnectToServerChanged FINAL)

#ifndef Q_OS_ANDROID
Q_PROPERTY(OnlineUserInfo *allUsers READ allUsers NOTIFY allUsersChanged FINAL)
#endif

public:
	explicit DBUserModel(QObject *parent = nullptr, const bool bMainUserModel = true);

	inline QString userDir(const int user_idx = 0) const { return userDir(userId(user_idx)); }
	QString userDir(const QString &userid) const;
	QString profileFileName(const QString &userid) const;
	QString profileFilePath(const QString &userid) const;

	inline QString idLabel() const { return "Id: "_L1; }
	inline QString onlineAccountUserLabel() const { return tr("Create online account: "); }
	inline QString nameLabel() const { return tr("Name: "); }
	inline QString birthdayLabel() const { return tr("Birthday: "); }
	inline QString sexLabel() const { return tr("Sex: "); }
	inline QString phoneLabel() const { return tr("Phone: "); }
	inline QString emailLabel() const { return "e-mail: "_L1; }
	inline QString socialMediaLabel() const { return tr("Social Media: "); }
	inline QString userRoleLabel() const { return tr("Your are: "); }
	inline QString coachRoleLabel() const { return tr("Training Job: "); }
	inline QString goalLabel() const { return tr("Goal: "); }
	inline QString avatarLabel() const { return "Avatar: "_L1; }
	inline QString appUseModelLabel() const { return tr("App use mode: "); }
	inline QString passwordLabel() const { return tr("Password:"); }
	inline QString newUserLabel() const
	{
		return !m_usersData.isEmpty() && !_userName(0).isEmpty() ? tr("Continue Setup") : tr("Create a new user");
	}
	inline QString existingUserLabel() const { return tr("User already registered"); }
	inline QString invalidEmailLabel() const { return tr("Invalid email address"); }
	inline QString invalidPasswordLabel() const { return tr("Password must have 6 characters or more"); }
	inline QString checkEmailLabel() const { return tr("Check"); }
	inline QString importUserLabel() const { return tr("Import"); }

	void showFirstTimeUseDialog();

	inline uint userCount() const { return m_usersData.count(); }
	inline const QString &_onlineAccount(const uint user_idx) const
	{
		return user_idx < m_usersData.count() ? m_usersData.at(user_idx).at(USER_COL_ONLINEACCOUNT) : STR_ZERO;
	}
	inline bool onlineAccount(const uint user_idx = 0) const { return _onlineAccount(user_idx).at(0) == '1'; }
	void setOnlineAccount(const bool online_user, const uint user_idx = 0);

	void addUser(QStringList &&user_info);
	Q_INVOKABLE void createMainUser(const QString &userid = QString{}, const QString &name = QString{});
	void removeMainUser();
	Q_INVOKABLE void removeUser(const int user_idx, const bool remove_local = true, const bool remove_online = true);

	Q_INVOKABLE inline bool isCoach(const uint user_idx) const
	{
		const uint app_use_mode{appUseMode(user_idx)};
		return app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH;
	}
	inline bool mainUserIsCoach() const { return isCoach(0); }
	Q_INVOKABLE inline bool isClient(const uint user_idx) const
	{
		return appUseMode(user_idx) != APP_USE_MODE_SINGLE_COACH;
	}
	inline bool mainUserIsClient() const { return isClient(0); }

	Q_INVOKABLE inline int findUserByName(const QString &username, const bool exact_match = true) const
	{
		return userIdxFromFieldValue(USER_COL_NAME, username, exact_match);
	}
	Q_INVOKABLE inline QString userNameFromId(const QString &userid) const { return userName(userIdxFromFieldValue(USER_COL_ID, userid)); }
	int userIdxFromFieldValue(const uint field, const QString &value, const bool exact_match = true) const;
	const QString &userIdFromFieldValue(const uint field, const QString &value) const;

	inline QString userId(const int user_idx = 0) const { return user_idx < m_usersData.count() ? m_usersData.at(user_idx).at(USER_COL_ID) : QString{}; }
	inline void setUserId(const uint user_idx, const QString &new_id) { m_usersData[user_idx][USER_COL_ID] = new_id; }

	Q_INVOKABLE inline QString userName(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _userName(user_idx) : QString{}; }
	inline const QString &_userName(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_NAME); }
	Q_INVOKABLE inline void setUserName(const int user_idx, const QString &new_name)
	{
		if (new_name != _userName(user_idx))
		{
			m_usersData[user_idx][USER_COL_NAME] = new_name;
			emit userModified(user_idx, USER_COL_NAME);
		}
	}

	Q_INVOKABLE void setPassword(const QString &passwd);
	Q_INVOKABLE void getPassword();

	Q_INVOKABLE inline QDate birthDate(const int user_idx) const
	{
		return user_idx >= 0 && user_idx < m_usersData.count() ? QDate::fromJulianDay(_birthDate(user_idx).toLongLong()) : QDate::currentDate();
	}
	Q_INVOKABLE inline int birthYear(const int user_idx) const { return birthDate(user_idx).year(); }
	Q_INVOKABLE inline QString birthDateFancy(const int user_idx) const
	{
		return appUtils()->formatDate(birthDate(user_idx));
	}
	inline const QString &_birthDate(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_BIRTHDAY); }
	Q_INVOKABLE inline void setBirthDate(const uint user_idx, const QDate& new_date)
	{
		if (new_date != birthDate(user_idx))
		{
			m_usersData[user_idx][USER_COL_BIRTHDAY] = std::move(QString::number(new_date.toJulianDay()));
			emit userModified(user_idx, USER_COL_BIRTHDAY);
		}
	}

	Q_INVOKABLE inline uint sex(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _sex(user_idx).toUInt() : 2; }
	inline const QString &_sex(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_SEX); }
	Q_INVOKABLE void setSex(const int user_idx, const bool male)
	{
		m_usersData[user_idx][USER_COL_SEX] = male ? '0' : '1';
		setAvatar(user_idx, (male ? "image://tpimageprovider/m0"_L1 : "image://tpimageprovider/f1"_L1));
		emit userModified(user_idx, USER_COL_SEX);
	}

	Q_INVOKABLE inline QString phoneCountryPrefix(const uint user_idx) const
	{
		return user_idx < m_usersData.count() ? getPhonePart(_phone(user_idx), true) : QString{};
	}
	Q_INVOKABLE inline QString phoneNumber(uint user_idx) const
	{
		return user_idx < m_usersData.count() ? getPhonePart(_phone(user_idx), false) : QString{};
	}

	inline const QString &_phone(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_PHONE); }
	Q_INVOKABLE void setPhone(const int user_idx, QString new_phone_prefix, const QString &new_phone);

	Q_INVOKABLE inline QString email(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _email(user_idx) : m_emptyString; }
	inline const QString &_email(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_EMAIL); }
	Q_INVOKABLE inline void setEmail(const int user_idx, const QString &new_email)
	{
		m_usersData[user_idx][USER_COL_EMAIL] = new_email;
		emit userModified(user_idx, USER_COL_EMAIL);
	}

	Q_INVOKABLE inline QString socialMedia(const int user_idx, const int index) const
	{
		return user_idx >= 0 && user_idx < m_usersData.count() ?
			appUtils()->getCompositeValue(index, _socialMedia(user_idx), record_separator) :
			QString{};
	}
	inline const QString &_socialMedia(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_SOCIALMEDIA); }
	Q_INVOKABLE inline void setSocialMedia(const int user_idx, const uint index, const QString &new_social)
	{
		appUtils()->setCompositeValue(index, new_social, m_usersData[user_idx][USER_COL_SOCIALMEDIA], record_separator);
		emit userModified(user_idx, USER_COL_SOCIALMEDIA);
	}

	Q_INVOKABLE inline QString userRole(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _userRole(user_idx) : m_emptyString; }
	inline const QString &_userRole(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_USERROLE); }
	Q_INVOKABLE inline void setUserRole(const int user_idx, const QString &new_role)
	{
		m_usersData[user_idx][USER_COL_USERROLE] = new_role;
		emit userModified(user_idx, USER_COL_USERROLE);
	}

	Q_INVOKABLE inline QString coachRole(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _coachRole(user_idx) : m_emptyString; }
	inline const QString &_coachRole(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_COACHROLE); }
	Q_INVOKABLE inline void setCoachRole(const int user_idx, const QString &new_role)
	{
		m_usersData[user_idx][USER_COL_COACHROLE] = new_role;
		emit userModified(user_idx, USER_COL_COACHROLE);
	}

	Q_INVOKABLE inline QString goal(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _goal(user_idx) : QString{}; }
	inline const QString &_goal(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_GOAL); }
	Q_INVOKABLE inline void setGoal(const int user_idx, const QString &new_goal)
	{
		m_usersData[user_idx][USER_COL_GOAL] = new_goal;
		emit userModified(user_idx, USER_COL_GOAL);
	}

	Q_INVOKABLE inline QString avatarFromId(const QString &userid) { return avatar(userIdxFromFieldValue(USER_COL_ID, userid)); }
	Q_INVOKABLE QString avatar(const uint user_idx, const bool checkServer = true);
	Q_INVOKABLE void setAvatar(const int user_idx, const QString &new_avatar, const bool saveToDisk = true, const bool upload = true);

	Q_INVOKABLE inline uint appUseMode(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _appUseMode(user_idx).toUInt() : 0; }
	inline const QString &_appUseMode(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_APP_USE_MODE); }
	Q_INVOKABLE void setAppUseMode(const int user_idx, const int new_use_opt);

	inline OnlineUserInfo *availableCoaches() const { return m_availableCoaches; }
	inline OnlineUserInfo *pendingCoachesResponses() const { return m_pendingCoachesResponses; }
	inline OnlineUserInfo *currentCoaches() const { return m_currentCoaches; }
	void addCoach(const uint user_idx, const bool emit_signal = true);
	void delCoach(const uint user_idx);
	const QString currentCoachName(const uint user_idx) const;
	void checkCoachesReponses();

	inline OnlineUserInfo *pendingClientsRequests() const { return m_pendingClientRequests; }
	inline OnlineUserInfo *currentClients() const { return m_currentClients; }
	inline const QString &defaultClient() const { return m_currentClients ? m_currentClients->data(0, USER_COL_NAME) : m_emptyString; }
	void addClient(const uint user_idx, const bool emit_signal = true);
	void delClient(const uint user_idx);
	void changeClient(const uint user_idx, const QString &oldname);

	inline OnlineUserInfo *currentCoachesAndClients() const { return m_currentCoachesAndClients; }

#ifndef Q_OS_ANDROID
	Q_INVOKABLE void getAllOnlineUsers();
	Q_INVOKABLE void switchUser();
	Q_INVOKABLE void createNewUser();
	Q_INVOKABLE void removeOtherUser();
	void userSwitchingActions(const bool create, QString &&userid);
	inline OnlineUserInfo *allUsers() const { return m_allUsers; }
#endif

	int getTemporaryUserInfo(OnlineUserInfo *tempUser, const uint userInfouser_idx);
	bool mainUserConfigured() const;
	inline bool canConnectToServer() const { return mb_canConnectToServer; }

	Q_INVOKABLE void acceptUser(OnlineUserInfo *userInfo, const int userInfouser_idx);
	Q_INVOKABLE void rejectUser(OnlineUserInfo *userInfo, const int userInfouser_idx);

	Q_INVOKABLE inline void cancelPendingOnlineRequests()
	{
		disconnect(this, &DBUserModel::mainUserOnlineCheckInChanged, nullptr, nullptr);
	}
	Q_INVOKABLE void checkUserOnline(const QString &email, const QString &password);
	Q_INVOKABLE void changePassword(const QString &old_password, const QString &new_password);
	Q_INVOKABLE void importFromOnlineServer();
	Q_INVOKABLE inline bool mainUserRegistered() const { return mb_userRegistered.has_value() && mb_userRegistered == true; }
	Q_INVOKABLE void setCoachPublicStatus(const bool bPublic);
	Q_INVOKABLE inline void viewResume(const uint user_idx)
	{
		downloadResumeFromServer(user_idx);
	}
	Q_INVOKABLE inline void viewResume(OnlineUserInfo *tempUser, const uint userinfo_user_idx)
	{
		viewResume(getTemporaryUserInfo(tempUser, userinfo_user_idx));
	}
	Q_INVOKABLE void uploadResume(const QString &resumeFileName);
	Q_INVOKABLE void setMainUserConfigurationFinished();
	Q_INVOKABLE inline bool isCoachRegistered()
	{
		if (mb_coachRegistered.has_value() && mb_coachRegistered.value())
			return (mb_coachPublic = true);
		return false;
	}
	Q_INVOKABLE void sendRequestToCoaches();
	Q_INVOKABLE void getOnlineCoachesList(const bool get_list_only = false);

	int sendFileToServer(const QString &filename, QFile *upload_file = nullptr, const QString &successMessage = QString{},
			const QString &subdir = QString{}, const QString &targetUser = QString{}, const bool removeLocalFile = false);
	int downloadFileFromServer(const QString &filename, const QString &local_filename = QString{}, const QString &successMessage = QString{},
							   const QString &subdir = QString{}, const QString &targetUser = QString{});
	void removeFileFromServer(const QString &filename, const QString &subdir = QString{}, const QString &targetUser = QString{});
	int listFilesFromServer(const QString &subdir, const QString &targetUser, const QString &filter = QString{});
	void sendCmdFileToServer(const QString &cmd_filename);
	void downloadCmdFilesFromServer(const QString &subdir);

	int exportToFile(const uint user_idx, const QString &filename, const bool write_header, QFile *out_file = nullptr) const;
	int exportToFormattedFile(const uint user_idx, const QString &filename, QFile *out_file = nullptr) const;
	int importFromFile(const QString &filename, QFile *in_file = nullptr);
	int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	bool importFromString(const QString &user_data);
	int newUserFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);

public slots:
	void saveUserInfo(const uint user_idx, const uint field);
	void getPasswordFromUserInput(const int resultCode, const QString &password);
	void slot_unregisterUser(const bool unregister);
	void slot_removeNoLongerAvailableUser(const int user_idx, bool remove);
	void slot_revokeCoachStatus(int new_use_opt, bool revoke);
	void slot_revokeClientStatus(int new_use_opt, bool revoke);

signals:
	void userModified(const uint user_idx, const uint field);
	void labelsChanged();
	void appUseModeChanged();
	void onlineUserChanged();
	void pendingCoachesResponsesChanged();
	void pendingClientsRequestsChanged();
	void currentCoachesChanged();
	void currentClientsChanged();
	void currentCoachesAndClientsChanged();
	void coachesListReceived(const QStringList &coaches_list);
	void clientsListReceived(const QStringList &clients_list);
	void availableCoachesChanged();
	void userOnlineCheckResult(const bool registered);
	void userOnlineImportFinished(const bool result);
	void allUserFilesDownloaded(const bool success);
	void mainUserConfigurationFinished();
	void canConnectToServerChanged();
	void mainUserOnlineCheckInChanged(const bool first_checkin = false);
	void coachOnlineStatus(bool registered);
	void userProfileAcquired(const QString &userid, const bool success);
	void userPasswordAvailable(const QString &password);
	void fileDownloaded(const bool success, const uint requestid, const QString &local_file_name);
	void fileUploaded(const bool success, const uint requestid);
	void filesListReceived(const bool success, const uint requestid, const QStringList& files_list);
	void onlineDevicesListReceived();
	void lastOnlineCmdRetrieved(const uint requestid, const QString &last_cmd);
	//Only used in desktop for development purposes, but must be here so that the QML parser does not complain
	//when it finds onUserIdChanged() in a Connections{} block
	void userIdChanged();

#ifndef Q_OS_ANDROID
	void allUsersChanged();
	void userSwitchPhase1Finished(const bool success);
#endif

private:
	QList<QStringList> m_usersData, m_tempUserData;
	int m_tempRow, n_devices;
	QString m_onlineAccountId, m_password, m_defaultAvatar, m_emptyString;
	std::optional<bool> mb_singleDevice, mb_userRegistered, mb_coachRegistered;
	OnlineUserInfo *m_availableCoaches, *m_pendingClientRequests, *m_pendingCoachesResponses,
						*m_tempUserInfo, *m_currentCoaches, *m_currentClients, *m_currentCoachesAndClients;
	bool mb_canConnectToServer, mb_coachPublic, mb_MainUserInfoChanged;
	QTimer *m_mainTimer;

	DBUserTable *m_db;
	DBModelInterfaceUser *m_dbModelInterface;

#ifndef Q_OS_ANDROID
	OnlineUserInfo *m_allUsers;
	QHash<QString,DBMesocyclesModel*> m_mesoModels;
#endif

	void initUserSession();
	QString getPhonePart(const QString &str_phone, const bool prefix) const;
	void setPhoneBasedOnLocale();
	QString generateUniqueUserId() const;
	void onlineCheckIn();
	void registerUserOnline();
	void onlineCheckinActions();
	void getOnlineDevicesList();
	void switchToUser(const QString &new_userid, const bool user_switching_for_testing = false);
	void downloadAllUserFiles(const QString &userid);
	void lastLocalCmd(const QString &subdir);
	void sendUnsentCmdFiles(const QString &subdir);
	QString resume(const uint user_idx) const;
	void checkIfCoachRegisteredOnline();
	void getUserOnlineProfile(const QString &netName, const QString &save_as_filename);
	void sendProfileToServer();
	void sendUserDataToServerDatabase();
	inline void sendAvatarToServer() { sendFileToServer(avatar(0), nullptr, QString{}, QString{}, userId(0)); }
	inline QString defaultAvatar(const uint user_idx) const
	{
		return sex(user_idx) == 0 ? "image://tpimageprovider/m0"_L1 : "image://tpimageprovider/f1"_L1;
	}
	QString findAvatar(const QString &base_dir) const;
	QString findResume(const QString &base_dir) const;
	void downloadAvatarFromServer(const uint user_idx);
	void downloadResumeFromServer(const uint user_idx);
	void openResume(const QString &filename) const;
	void startServerPolling();
	void pollServer();
	void pollClientsRequests();
	void addPendingClient(const QString &user_id);
	void pollCoachesAnswers();
	void addCoachAnswer(const QString &user_id);
	void addAvailableCoach(const QString &user_id);
	void pollCurrentClients();
	void pollCurrentCoaches();
	void checkNewMesos();
	void addIntoCoachesAndClients(OnlineUserInfo* other_userinfo, const uint row);
	QString formatFieldToExport(const uint field, const QString &fieldValue) const;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;
	inline QList<QStringList> &tempUserData() { return m_tempUserData; }

	static DBUserModel *_appUserModel;
	friend DBUserModel *appUserModel();
	friend class OnlineUserInfo;
	friend class DBModelInterfaceUser;
};

inline DBUserModel *appUserModel() { return DBUserModel::_appUserModel; }

class DBModelInterfaceUser : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceUser() : DBModelInterface{appUserModel()} {}
	inline const QList<QStringList> &modelData() const { return appUserModel()->m_usersData; }
	inline QList<QStringList> &modelData() { return appUserModel()->m_usersData; }
};
