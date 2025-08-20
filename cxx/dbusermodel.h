#pragma once

#include "tpglobals.h"
#include "tputils.h"
#include "online_services/onlineuserinfo.h"

#define USER_COL_ID 0
#define USER_COL_NETUSER 1
#define USER_COL_NAME 2
#define USER_COL_BIRTHDAY 3
#define USER_COL_SEX 4
#define USER_COL_PHONE 5
#define USER_COL_EMAIL 6
#define USER_COL_SOCIALMEDIA 7
#define USER_COL_USERROLE 8
#define USER_COL_COACHROLE 9
#define USER_COL_GOAL 10
#define USER_COL_APP_USE_MODE 11
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

QT_FORWARD_DECLARE_CLASS(QTimer)

class DBUserModel : public QObject
{

Q_OBJECT

Q_PROPERTY(QString netUserLabel READ netUserLabel NOTIFY labelsChanged FINAL)
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

Q_PROPERTY(OnlineUserInfo* availableCoaches READ availableCoaches NOTIFY availableCoachesChanged FINAL)
Q_PROPERTY(OnlineUserInfo* pendingCoachesResponses READ pendingCoachesResponses NOTIFY pendingCoachesResponsesChanged FINAL)
Q_PROPERTY(OnlineUserInfo* pendingClientsRequests READ pendingClientsRequests NOTIFY pendingClientsRequestsChanged FINAL)
Q_PROPERTY(QStringList coachesNames READ coachesNames NOTIFY coachesNamesChanged FINAL)
Q_PROPERTY(QStringList clientsNames READ clientsNames NOTIFY clientsNamesChanged FINAL)
Q_PROPERTY(bool onlineUser READ onlineUser WRITE setOnlineUser NOTIFY onlineUserChanged FINAL)
Q_PROPERTY(bool haveCoaches READ haveCoaches NOTIFY haveCoachesChanged FINAL)
Q_PROPERTY(bool haveClients READ haveClients NOTIFY haveClientsChanged FINAL)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured NOTIFY mainUserConfigurationFinished FINAL)

public:
	explicit DBUserModel(QObject *parent = nullptr, const bool bMainUserModel = true);

	inline QString idLabel() const { return "Id: "_L1; }
	inline QString netUserLabel() const { return tr("Register online: "); }
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

	inline uint userCount() const { return m_usersData.count(); }

	inline const QString &_onlineUser(const uint user_idx) const { return user_idx < m_usersData.count() ? m_usersData.at(user_idx).at(USER_COL_NETUSER) : m_onlineUserId; }
	inline bool onlineUser(const uint user_idx = 0) const { return _onlineUser(user_idx) == '1'; }
	void setOnlineUser(const bool online_user, const uint user_idx = 0);

	void addUser(QStringList &&user_info);
	Q_INVOKABLE void createMainUser();
	void removeMainUser();
	Q_INVOKABLE void removeUser(const int user_idx, const bool remove_local = true, const bool remove_online = true);

	const int getuser_idxByCoachName(const QString &coachname) const;
	Q_INVOKABLE inline bool isCoach(const uint user_idx) const
	{
		const uint app_use_mode{appUseMode(user_idx)};
		return app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH;
	}
	Q_INVOKABLE inline bool isClient(const uint user_idx) const
	{
		return appUseMode(user_idx) != APP_USE_MODE_SINGLE_COACH;
	}

	Q_INVOKABLE inline int findUserByName(const QString &username) const { return userIdxFromFieldValue(USER_COL_NAME, username); }
	Q_INVOKABLE inline QString userNameFromId(const QString &userid) const { return userName(userIdxFromFieldValue(USER_COL_ID, userid)); }
	int userIdxFromFieldValue(const uint field, const QString &value) const;
	const QString &userIdFromFieldValue(const uint field, const QString &value) const;
	inline const QString localDir(const QString &userid) const { return localDir(userIdxFromFieldValue(USER_COL_ID, userid)); }
	const QString localDir(const int user_idx) const;

	// m_onlineCoachesDir: just a referenceable QString that will not have any meaningful impact elsehwere
	inline const QString &userId(const int user_idx) const { return user_idx < m_usersData.count() ? m_usersData.at(user_idx).at(USER_COL_ID) : m_onlineCoachesDir; }
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

	Q_INVOKABLE inline QString phone(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _phone(user_idx) : m_emptyString; }
	inline const QString &_phone(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_COL_PHONE); }
	Q_INVOKABLE inline void setPhone(const int user_idx, const QString &new_phone)
	{
		m_usersData[user_idx][USER_COL_PHONE] = new_phone;
		emit userModified(user_idx, USER_COL_PHONE);
	}

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
	inline QStringList coachesNames() const { return m_coachesNames; }
	Q_INVOKABLE inline int coachuser_idx(const QString &coach_name) const { return m_coachesNames.indexOf(coach_name); }
	inline bool haveCoaches() const { return m_coachesNames.count() > 0; }
	void addCoach(const uint user_idx);
	void delCoach(const uint user_idx);
	const QString currentCoachName(const uint user_idx) const;
	void checkCoachesReponses();

	inline OnlineUserInfo *pendingClientsRequests() const { return m_pendingClientRequests; }
	inline QStringList clientsNames() const { return m_clientsNames; }
	Q_INVOKABLE inline int clientuser_idx(const QString &userid) const { return m_clientsNames.indexOf(userName(userIdxFromFieldValue(USER_COL_ID, userid))); }
	inline const QString &defaultClient() const { return m_clientsNames.count() > 0 ? userIdFromFieldValue(USER_COL_NAME, m_clientsNames.last()) : m_emptyString; }
	inline bool haveClients() const { return m_clientsNames.count() > 0; }
	void addClient(const uint user_idx);
	void delClient(const uint user_idx);
	void changeClient(const uint user_idx, const QString &oldname);

	inline bool canConnectToServer() const { return mb_canConnectToServer; }
	Q_INVOKABLE int getTemporaryUserInfo(OnlineUserInfo *tempUser, const uint userInfouser_idx);
	bool mainUserConfigured() const;

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
	Q_INVOKABLE inline void viewResume(OnlineUserInfo *tempUser, const uint userInfouser_idx)
	{
		viewResume(getTemporaryUserInfo(tempUser, userInfouser_idx));
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
	int downloadFileFromServer(const QString &filename, const QString &localFile = QString{}, const QString &successMessage = QString{},
							   const QString &subdir = QString{}, const QString &targetUser = QString{});
	void removeFileFromServer(const QString &filename, const QString &subdir = QString{}, const QString &targetUser = QString{});

	int exportToFile(const uint user_idx, const QString &filename, QFile *out_file = nullptr) const;
	int exportToFormattedFile(const uint user_idx, const QString &filename, QFile *out_file = nullptr) const;
	int importFromFile(const QString &filename, QFile *in_file = nullptr);
	int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	bool importFromString(const QString &user_data);
	int newUserFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);

public slots:
	void getPasswordFromUserInput(const int resultCode, const QString &password);
	void slot_unregisterUser(const bool unregister);
	void slot_removeNoLongerAvailableUser(const int user_idx, bool remove);
	void slot_revokeCoachStatus(int new_use_opt, bool revoke);
	void slot_revokeClientStatus(int new_use_opt, bool revoke);

signals:
	void userModified(const uint user_idx, const uint field);
	void labelsChanged();
	void onlineUserChanged();
	void haveCoachesChanged();
	void haveClientsChanged();
	void coachesNamesChanged();
	void clientsNamesChanged();
	void coachesListReceived(const QStringList &coaches_list);
	void clientsListReceived(const QStringList &clients_list);
	void availableCoachesChanged();
	void pendingCoachesResponsesChanged();
	void pendingClientsRequestsChanged();
	void userOnlineCheckResult(const bool registered);
	void userOnlineImportFinished(const bool result);
	void mainUserConfigurationFinished();
	void mainUserOnlineCheckInChanged();
	void coachOnlineStatus(bool registered);
	void userProfileAcquired(const QString &userid, const bool success);
	void userPasswordAvailable(const QString &password);
	void fileDownloaded(const bool success, const uint requestid, const QString &localFileName);
	void fileUploaded(const bool success, const uint requestid);
	void onlineDevicesListReceived(const bool success);
	void lastOnlineCmdRetrieved(const uint requestid, const QString &last_cmd);

private:
	QList<QStringList> m_usersData, m_tempUserData;
	int m_tempRow, n_devices;
	QString m_onlineUserId, m_password, m_defaultAvatar, m_emptyString, m_onlineCoachesDir,
		m_dirForRequestedCoaches, m_dirForClientsRequests, m_dirForCurrentClients, m_dirForCurrentCoaches;
	std::optional<bool> mb_singleDevice, mb_userRegistered, mb_coachRegistered;
	OnlineUserInfo *m_availableCoaches, *m_pendingClientRequests, *m_pendingCoachesResponses, *m_tempUserInfo;
	QStringList m_coachesNames, m_clientsNames;
	bool mb_canConnectToServer, mb_coachPublic, mb_MainUserInfoChanged;
	QTimer *m_mainTimer;

	QString generateUniqueUserId() const;
	void onlineCheckIn();
	void registerUserOnline();
	void onlineCheckinActions();
	void getOnlineDevicesList();
	void createOnlineDatabases();
	void syncDatabases(const QStringList &online_db_files);
	void lastOnlineCmd(const uint requestid, const QString &subdir = QString{});
	QString resume(const uint user_idx) const;
	void checkIfCoachRegisteredOnline();
	void getUserOnlineProfile(const QString &netName, const QString &save_as_filename);
	void sendProfileToServer();
	void sendUserInfoToServer();
	inline void sendAvatarToServer() { sendFileToServer(avatar(0), nullptr, QString{}, QString{}, userId(0)); }
	inline QString defaultAvatar(const uint user_idx) const
	{
		return sex(user_idx) == 0 ? "image://tpimageprovider/m0"_L1 : "image://tpimageprovider/f1"_L1;
	}
	void downloadAvatarFromServer(const uint user_idx);
	void downloadResumeFromServer(const uint user_idx);
	void copyTempUserFilesToFinalUserDir(const QString &destDir, OnlineUserInfo *userInfo, const int userInfouser_idx) const;
	void clearTempUserFiles(OnlineUserInfo *userInfo, const int userInfouser_idx) const;
	void clearUserDir(const QString &dir) const;
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
	QString formatFieldToExport(const uint field, const QString &fieldValue) const;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;
	inline QList<QStringList> &tempUserData() { return m_tempUserData; }

	static DBUserModel *_appUserModel;
	friend DBUserModel *appUserModel();
	friend class OnlineUserInfo;
};

inline DBUserModel *appUserModel() { return DBUserModel::_appUserModel; }
