#pragma once

#include "dbmodelinterface.h"
#include "qml_singleton.h"
#include "tputils.h"
#include "online_services/onlineuserinfo.h"

#define USER_MODIFIED_CREATED 100
#define USER_MODIFIED_IMPORTED 101
#define USER_MODIFIED_REMOVED 102
#define USER_MODIFIED_ACCEPTED 103
#define USER_MODIFIED_SWITCHING 104

QT_FORWARD_DECLARE_CLASS(DBUserTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceUser)
QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(TPFilePath)
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
Q_PROPERTY(QString categoryLabel READ categoryLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString newUserLabel READ newUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString existingUserLabel READ existingUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString invalidEmailLabel READ invalidEmailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString invalidPasswordLabel READ invalidPasswordLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString checkEmailLabel READ checkEmailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString importUserLabel READ importUserLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(OnlineUserInfo *availableCoaches READ availableCoaches NOTIFY availableCoachesChanged FINAL)
Q_PROPERTY(OnlineUserInfo *allUsersList READ allUsersList NOTIFY allUsersListChanged FINAL)
Q_PROPERTY(bool onlineAccount READ onlineAccount WRITE setOnlineAccount NOTIFY onlineUserChanged FINAL)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured NOTIFY mainUserConfigurationFinished FINAL)
Q_PROPERTY(bool canConnectToServer READ canConnectToServer WRITE setCanConnectToServer NOTIFY canConnectToServerChanged FINAL)

#ifndef Q_OS_ANDROID
Q_PROPERTY(OnlineUserInfo *allUsers READ allUsers NOTIFY allUsersChanged FINAL)
#endif

public:

	enum userFields {
		USER_FIELD_ID,
		USER_FIELD_INSERTTIME,
		USER_FIELD_ONLINEACCOUNT,
		USER_FIELD_NAME,
		USER_FIELD_BIRTHDAY,
		USER_FIELD_SEX,
		USER_FIELD_PHONE,
		USER_FIELD_EMAIL,
		USER_FIELD_SOCIALMEDIA,
		USER_FIELD_USERROLE,
		USER_FIELD_COACHROLE,
		USER_FIELD_GOAL,
		USER_FIELD_USER_CATEGORY,
		USER_N_FIELS,
		USER_FIELD_AVATAR,
	};
	Q_ENUM(userFields)

	enum st_userCategory {
		UC_CLIENT			=	1U << 0,
		UC_COACH			=	1U << 1,
		UC_HAS_COACH		=	1U << 2,
		UC_HAS_CLIENT		=	1U << 3,
		UC_YET_AVAILABLE	=	1U << 4,
		UC_PENDING_STATUS	=	1U << 5,
	};
	Q_ENUM(st_userCategory)

	static constexpr QLatin1StringView binary_files_subdir{"exchange_files/" };

	explicit DBUserModel(QObject *parent = nullptr, const bool bMainUserModel = true);

	inline QString userDir(const int user_idx = 0) const { return userDir(userId(user_idx)); }
	QString userDir(const QString &userid) const;

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
	inline QString categoryLabel() const { return tr("User category: "); }
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

	void initUserSession();

	inline uint userCount() const { return m_usersData.count(); }
	inline const QString _onlineAccount(const uint user_idx) const
	{
		return user_idx < m_usersData.count() ? m_usersData.at(user_idx).at(USER_FIELD_ONLINEACCOUNT) : "0"_L1;
	}
	inline bool onlineAccount(const uint user_idx = 0) const { return _onlineAccount(user_idx).at(0) == '1'; }
	void setOnlineAccount(const bool online_user, const uint user_idx = 0);

	Q_INVOKABLE void createMainUser(const QString &userid = QString{}, const QString &name = QString{});
	void removeMainUser();
	Q_INVOKABLE void removeUser(const int user_idx, const bool remove_local = true, const bool remove_online = true);

	Q_INVOKABLE inline bool isCoach(const uint user_idx) const { return userCategory(user_idx) & UC_COACH; }
	Q_INVOKABLE inline void setIsCoach(const uint user_idx, const bool coach) { setUserCategory(user_idx, UC_COACH, coach); }
	Q_INVOKABLE inline bool isClient(const uint user_idx) const { return userCategory(user_idx) & UC_CLIENT; }
	Q_INVOKABLE inline void setIsClient(const uint user_idx, const bool client) { setUserCategory(user_idx, UC_CLIENT, client); }
	inline bool isConfirmed(const uint user_idx) const { return (userCategory(user_idx) & UC_PENDING_STATUS) == 0; }
	inline void setIsConfirmed(const uint user_idx, const bool confirmed) { setUserCategory(user_idx, UC_PENDING_STATUS, !confirmed); }
	inline bool isAvailable(const uint user_idx) const { return (userCategory(user_idx) & UC_YET_AVAILABLE) == UC_YET_AVAILABLE; }
	inline void setIsAvailable(const uint user_idx, const bool available) { setUserCategory(user_idx, UC_YET_AVAILABLE, available); }

	//the first() method of results indicates whether the file belongs to a main user's coach(or not)
	void scanUsersSubDirs(std::pair<QList<bool>, QFileInfoList> &results, const QString &subdir = QString{}, const QString &match = QString{});
	Q_INVOKABLE inline int findUserById(const QString &userid, const bool exact_match = true) const
	{
		return userIdxFromFieldValue(USER_FIELD_NAME, userid, exact_match);
	}
	Q_INVOKABLE inline QString userNameFromId(const QString &userid) const { return userName(userIdxFromFieldValue(USER_FIELD_ID, userid)); }
	int userIdxFromFieldValue(const uint field, const QString &value, const bool exact_match = true) const;
	const QString &userIdFromFieldValue(const uint field, const QString &value) const;

	inline const QString &userId(const int user_idx = 0) const { return m_usersData.at(user_idx).at(USER_FIELD_ID); }
	Q_INVOKABLE inline QString userId_QML(const int row) const { return row >= 0 && row < m_usersData.count() ? userId(row) : QString{}; }
	inline void setUserId(const uint user_idx, const QString &new_id) { m_usersData[user_idx][USER_FIELD_ID] = new_id; }

	Q_INVOKABLE inline QString userName(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _userName(user_idx) : QString{}; }
	inline const QString &_userName(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_NAME); }
	Q_INVOKABLE inline void setUserName(const int user_idx, const QString &new_name)
	{
		if (new_name != _userName(user_idx)) {
			m_usersData[user_idx][USER_FIELD_NAME] = new_name;
			emit userModified(user_idx, USER_FIELD_NAME);
		}
	}

	Q_INVOKABLE void requestPasswordFromUser(const int id, const QString &dialog_title, const QString &dialog_message);
	Q_INVOKABLE void checkPassword(const QString &password);
	Q_INVOKABLE void setPassword(const QString &passwd);
	Q_INVOKABLE void getPassword();

	Q_INVOKABLE inline QDate birthDate(const int user_idx) const
	{
		return user_idx >= 0 && user_idx < m_usersData.count() ? QDate::fromJulianDay(_birthDate(user_idx).toLongLong()) : QDate::currentDate();
	}
	Q_INVOKABLE inline int birthYear(const int user_idx) const { return birthDate(user_idx).year(); }
	Q_INVOKABLE inline QString birthDateFancy(const int user_idx) const { return appUtils()->formatDate(birthDate(user_idx)); }
	inline const QString &_birthDate(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_BIRTHDAY); }
	Q_INVOKABLE inline void setBirthDate(const uint user_idx, const QDate& new_date)
	{
		if (new_date != birthDate(user_idx)) {
			m_usersData[user_idx][USER_FIELD_BIRTHDAY] = std::move(QString::number(new_date.toJulianDay()));
			emit userModified(user_idx, USER_FIELD_BIRTHDAY);
		}
	}

	Q_INVOKABLE inline uint sex(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _sex(user_idx).toUInt() : 2; }
	inline const QString &_sex(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_SEX); }
	Q_INVOKABLE void setSex(const int user_idx, const bool male)
	{
		m_usersData[user_idx][USER_FIELD_SEX] = male ? '0' : '1';
		setAvatar(user_idx, (male ? "image://tpimageprovider/m0"_L1 : "image://tpimageprovider/f1"_L1));
		emit userModified(user_idx, USER_FIELD_SEX);
	}

	Q_INVOKABLE inline QString phoneCountryPrefix(const uint user_idx) const
	{
		return user_idx < m_usersData.count() ? getPhonePart(_phone(user_idx), true) : QString{};
	}
	Q_INVOKABLE inline QString phoneNumber(uint user_idx) const
	{
		return user_idx < m_usersData.count() ? getPhonePart(_phone(user_idx), false) : QString{};
	}

	inline const QString &_phone(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_PHONE); }
	Q_INVOKABLE void setPhone(const int user_idx, QString new_phone_prefix, const QString &new_phone);
	inline void setPhone(const uint user_idx, QString &&new_phone)
	{
		m_usersData[user_idx][USER_FIELD_PHONE] = std::forward<QString>(new_phone);
		emit userModified(user_idx, USER_FIELD_PHONE);
	}

	Q_INVOKABLE inline QString email(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _email(user_idx) : m_emptyString; }
	inline const QString &_email(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_EMAIL); }
	Q_INVOKABLE inline void setEmail(const int user_idx, const QString &new_email)
	{
		m_usersData[user_idx][USER_FIELD_EMAIL] = new_email;
		emit userModified(user_idx, USER_FIELD_EMAIL);
	}

	Q_INVOKABLE inline QString socialMedia(const int user_idx, const int index) const
	{
		return user_idx >= 0 && user_idx < m_usersData.count() ?
			appUtils()->getCompositeValue(index, _socialMedia(user_idx), record_separator) :
			QString{};
	}
	inline const QString &_socialMedia(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_SOCIALMEDIA); }
	Q_INVOKABLE inline void setSocialMedia(const int user_idx, const uint index, const QString &new_social)
	{
		appUtils()->setCompositeValue(index, new_social, m_usersData[user_idx][USER_FIELD_SOCIALMEDIA], record_separator);
		emit userModified(user_idx, USER_FIELD_SOCIALMEDIA);
	}
	inline void setSocialMedia(const int user_idx, QString &&new_social)
	{
		m_usersData[user_idx][USER_FIELD_SOCIALMEDIA] = std::forward<QString>(new_social);
		emit userModified(user_idx, USER_FIELD_SOCIALMEDIA);
	}

	Q_INVOKABLE inline QString userRole(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _userRole(user_idx) : m_emptyString; }
	inline const QString &_userRole(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_USERROLE); }
	Q_INVOKABLE inline void setUserRole(const int user_idx, const QString &new_role)
	{
		m_usersData[user_idx][USER_FIELD_USERROLE] = new_role;
		emit userModified(user_idx, USER_FIELD_USERROLE);
	}

	Q_INVOKABLE inline QString coachRole(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _coachRole(user_idx) : m_emptyString; }
	inline const QString &_coachRole(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_COACHROLE); }
	Q_INVOKABLE inline void setCoachRole(const int user_idx, const QString &new_role)
	{
		m_usersData[user_idx][USER_FIELD_COACHROLE] = new_role;
		emit userModified(user_idx, USER_FIELD_COACHROLE);
	}

	Q_INVOKABLE inline QString goal(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _goal(user_idx) : QString{}; }
	inline const QString &_goal(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_GOAL); }
	Q_INVOKABLE inline void setGoal(const int user_idx, const QString &new_goal)
	{
		m_usersData[user_idx][USER_FIELD_GOAL] = new_goal;
		emit userModified(user_idx, USER_FIELD_GOAL);
	}

	Q_INVOKABLE inline QString avatarFromId(const QString &userid) { return avatar(userIdxFromFieldValue(USER_FIELD_ID, userid)); }
	Q_INVOKABLE QString avatar(const uint user_idx);
	Q_INVOKABLE void setAvatar(const int user_idx, const QString &new_avatar, const bool saveToDisk = true, const bool upload = true);

	Q_INVOKABLE inline uint userCategory(const int user_idx) const { return user_idx >= 0 && user_idx < m_usersData.count() ? _userCategory(user_idx).toUInt() : 0; }
	inline const QString &_userCategory(const uint user_idx) const { return m_usersData.at(user_idx).at(USER_FIELD_USER_CATEGORY); }
	void setUserCategory(const int user_idx, const int new_category, const bool add);

	inline OnlineUserInfo *allUsersList() const { return m_allUsersInfo; }
	const QString currentCoachName(const uint user_idx) const;
	void checkCoachesReponses();

#ifndef Q_OS_ANDROID
	inline DBMesocyclesModel *actualMesoModel() const { return m_mesoModels.value(userId(0)); }
	Q_INVOKABLE void getAllOnlineUsers();
	Q_INVOKABLE void switchUser();
	Q_INVOKABLE inline void createNewUser() { userSwitchingActions(true, std::move(generateUniqueUserId())); }
	Q_INVOKABLE void removeOtherUser();
	void userSwitchingActions(const bool create, QString &&userid);
	inline OnlineUserInfo *allUsers() const { return m_allUsers; }
#else
	inline DBMesocyclesModel *actualMesoModel() const { return m_mesoModel; }
#endif

	bool mainUserConfigured() const;
	inline bool canConnectToServer() const { return mb_canConnectToServer; }
	inline void setCanConnectToServer(const bool can_connect) {
		if (can_connect != mb_canConnectToServer) {
			mb_canConnectToServer = can_connect;
			emit canConnectToServerChanged();
		}
	}

	Q_INVOKABLE void acceptUser(const uint user_idx);
	Q_INVOKABLE inline void rejectUser(const uint user_idx) { removeUser(user_idx); }

	Q_INVOKABLE void checkExistingAccount(const QString &email, const QString &password);
	Q_INVOKABLE void changePassword(const QString &old_password, const QString &new_password);
	Q_INVOKABLE void importFromOnlineServer();
	Q_INVOKABLE inline bool mainUserLoggedIn() const { return mb_userLoggedIn.has_value() && mb_userLoggedIn.value(); }
	Q_INVOKABLE void setCoachPublicStatus(const bool bPublic);
	Q_INVOKABLE inline void viewResume(const uint user_idx) { downloadResumeFromServer(user_idx); }
	Q_INVOKABLE void uploadResume(const QString &filename);
	Q_INVOKABLE void setMainUserConfigurationFinished();
	Q_INVOKABLE inline bool isCoachRegistered()
	{
		if (mb_coachRegistered.has_value() && mb_coachRegistered.value())
			return (mb_coachPublic = true);
		return false;
	}
	Q_INVOKABLE void sendRequestToCoaches(OnlineUserInfo *users_list);
	Q_INVOKABLE void getOnlineCoachesList(const bool get_list_only = false);

	void sendFileToUser(const std::shared_ptr<TPFilePath> &tp_filename, const QVariant &extra_info = QVariant{},
										const QString &success_message = QString{}, const bool first_attempt = true);
	int sendFileToServer(const std::shared_ptr<TPFilePath> &tp_filename, const QString &successMessage = QString{},
																					const bool removeLocalFile = false);
	int downloadFileFromServer(const std::shared_ptr<TPFilePath> &tp_filename, const QString &successMessage = QString{});
	void removeFileFromServer(const std::shared_ptr<TPFilePath>& tp_filename);
	int listFilesFromServer(const QString &subdir, const QString &targetUser, const QString &filter = QString{});
	void sendCmdFileToServer(const QString &cmd_filename);
	void downloadCmdFilesFromServer(const QString &subdir);

	int exportToFile(const uint user_idx, const std::shared_ptr<TPFilePath> &tp_filename, const bool write_header) const;
	int exportToFormattedFile(const uint user_idx, const std::shared_ptr<TPFilePath> &tp_filename) const;
	int importFromFile(const std::shared_ptr<TPFilePath> &tp_filename);
	int importFromFormattedFile(const std::shared_ptr<TPFilePath> &tp_filename);
	bool importFromString(const QString &user_data);
	int newUserFromFile(const std::shared_ptr<TPFilePath> &tp_filename,
										const std::optional<bool> &file_formatted = std::nullopt, uint category = 0);

public slots:	
	void saveUserInfo(const uint user_idx, const uint field);
	void sendUnsentCmdFiles(const QString &dir);

signals:
	void userModified(const uint user_idx, const uint field);
	void labelsChanged();
	void passwordAcquired(const bool proceed, const int request_id, const QString &passwd);
	void userCategoryChanged(const uint user_idx);
	void onlineUserChanged();

	void coachesListReceived(const QStringList &coaches_list);
	void clientsListReceived(const QStringList &clients_list);
	void availableCoachesChanged();
	void allUsersListChanged();
	void userOnlineCheckResult(const bool registered);
	void userOnlineImportFinished(const bool result);
	void allUserFilesDownloaded(const bool success);
	void mainUserConfigurationFinished();
	void canConnectToServerChanged();
	void userLoggedIn(const bool first_checkin = false);
	void userLoggedOut();
	void coachOnlineStatus(bool registered);
	void userProfileAcquired(const QString &userid, const bool success);
	void userPasswordAvailable(const QString &password);
	void fileDownloaded(const bool success, const uint requestid, const std::shared_ptr<TPFilePath> &tp_filepath);
	void fileUploaded(const bool success, const uint requestid);
	void filesListReceived(const bool success, const uint requestid, const QStringList& files_list);
	void onlineDevicesListReceived();
	void cmdFileCreated(const QString &dir);
	//Only used in desktop for development purposes, but must be here so that the QML parser does not complain
	//when it finds onUserIdChanged() in a Connections{} block
	void userIdChanged();

#ifndef Q_OS_ANDROID
	void allUsersChanged();
	void userSwitchPhase1Finished(const bool success);
#endif

private:
	QList<QStringList> m_usersData;
	int n_devices{0};
	QString m_onlineAccountId, m_password, m_defaultAvatar, m_emptyString, m_network_msg_title;
	std::optional<bool> mb_singleDevice, mb_userLoggedIn, mb_coachRegistered;
	OnlineUserInfo *m_allUsersInfo{nullptr};
	bool mb_canConnectToServer{false}, mb_coachPublic{false}, mb_MainUserInfoChanged{false};
	QTimer *m_mainTimer{nullptr};

	DBUserTable *m_db{nullptr};
	DBModelInterfaceUser *m_dbModelInterface{nullptr};
	QObject* m_passwordDialog{nullptr};
	QQmlComponent *m_passwordDialogComponent{nullptr};

#ifndef Q_OS_ANDROID
	QHash<QString,DBMesocyclesModel*> m_mesoModels;
	OnlineUserInfo *m_allUsers{nullptr};
#else
	DBMesocDBMesocyclesModel *m_mesoModel{nullptr};
#endif

	QString getPhonePart(const QString &str_phone, const bool prefix) const;
	void setPhoneBasedOnLocale();
	QString generateUniqueUserId() const;
	void onlineCheckIn();
	void loginUser();
	void onlineCheckinActions();
	void getOnlineDevicesList();
	void switchToUser(const QString &new_userid, const QString &test_username = QString{});
	void downloadAllUserFiles(const QString &userid);
	void checkIfCoachRegisteredOnline();
	void getUserOnlineProfile(const QString &userid);
	void sendProfileToServer();
	void sendUserDataToServerDatabase();
	void sendAvatarToServer();
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
	void addAvailableClient(const QString &user_id);
	void pollCoachesAnswers();
	void addAvailableCoach(const QString &user_id);
	void pollCurrentClients();
	void pollCurrentCoaches();
	void addIntoCoachesAndClients(OnlineUserInfo* other_userinfo, const uint row);
	void revokeCoachStatus();
	void revokeClientStatus();
	void unregisterUser();
	void addCoach(const uint user_idx, const bool notify = true);
	void delCoach(const uint user_idx);
	void addClient(const uint user_idx, const bool  notify = true);
	void delClient(const uint user_idx);
	QString formatFieldToExport(const uint field, const QString &fieldValue) const;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;

	static DBUserModel *_appUserModel;
	friend DBUserModel *appUserModel();
	friend class OnlineUserInfo;
	friend class DBModelInterfaceUser;
};

DECLARE_QML_NAMED_SINGLETON(DBUserModel, AppUserModel)

inline DBUserModel *appUserModel() { return DBUserModel::_appUserModel; }

class DBModelInterfaceUser : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceUser() : DBModelInterface{appUserModel()} {}
	inline const QList<QStringList> &modelData() const { return appUserModel()->m_usersData; }
	inline QList<QStringList> &modelData() { return appUserModel()->m_usersData; }
};
