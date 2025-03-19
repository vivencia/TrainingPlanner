#pragma once
#include "tplistmodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "online_services/onlineuserinfo.h"

#define USER_COL_ID 0
#define USER_COL_NAME 1
#define USER_COL_BIRTHDAY 2
#define USER_COL_SEX 3
#define USER_COL_PHONE 4
#define USER_COL_EMAIL 5
#define USER_COL_SOCIALMEDIA 6
#define USER_COL_USERROLE 7
#define USER_COL_COACHROLE 8
#define USER_COL_GOAL 9
#define USER_COL_APP_USE_MODE 10

#define USER_TOTAL_COLS USER_COL_APP_USE_MODE + 1

#define APP_USE_MODE_SINGLE_USER 1
#define APP_USE_MODE_SINGLE_COACH 2
#define APP_USE_MODE_SINGLE_USER_WITH_COACH 3
#define APP_USE_MODE_COACH_USER_WITH_COACH 4
#define APP_USE_MODE_PENDING_CLIENT 5

#define USER_COL_AVATAR 20 //not in database, but used on model and GUI operations

QT_FORWARD_DECLARE_CLASS(QTimer)

class DBUserModel : public TPListModel
{

Q_OBJECT

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
Q_PROPERTY(bool haveCoaches READ haveCoaches NOTIFY haveCoachesChanged FINAL)
Q_PROPERTY(bool haveClients READ haveClients NOTIFY haveClientsChanged FINAL)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured NOTIFY mainUserConfigurationFinished FINAL)

public:
	explicit DBUserModel(QObject *parent = nullptr, const bool bMainUserModel = true);
	void updateColumnNames();

	inline QString nameLabel() const { return mColumnNames.at(USER_COL_NAME); }
	inline QString birthdayLabel() const { return mColumnNames.at(USER_COL_BIRTHDAY); }
	inline QString sexLabel() const { return mColumnNames.at(USER_COL_SEX); }
	inline QString phoneLabel() const { return mColumnNames.at(USER_COL_PHONE); }
	inline QString emailLabel() const { return mColumnNames.at(USER_COL_EMAIL); }
	inline QString socialMediaLabel() const { return mColumnNames.at(USER_COL_SOCIALMEDIA); }
	inline QString userRoleLabel() const { return mColumnNames.at(USER_COL_USERROLE); }
	inline QString coachRoleLabel() const { return mColumnNames.at(USER_COL_COACHROLE); }
	inline QString goalLabel() const { return mColumnNames.at(USER_COL_GOAL); }
	inline QString avatarLabel() const { return std::move("Avatar: "_L1); }
	QString passwordLabel() const;
	QString newUserLabel() const;
	QString existingUserLabel() const;
	QString invalidEmailLabel() const;
	QString invalidPasswordLabel() const;
	QString checkEmailLabel() const;
	QString importUserLabel() const;

	void addUser(QStringList &&user_info);
	Q_INVOKABLE void createMainUser();
	Q_INVOKABLE void removeMainUser();
	Q_INVOKABLE void removeUser(const int row);

	const int getRowByCoachName(const QString &coachname) const;
	inline bool isCoach(const uint row) const
	{
		const uint app_use_mode{appUseMode(row)};
		return app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH;
	}
	inline bool isClient(const uint row) const
	{
		return appUseMode(row) != APP_USE_MODE_SINGLE_COACH;
	}

	Q_INVOKABLE int findUserByName(const QString &userName) const;
	Q_INVOKABLE int findUserById(const QString &userId) const;

	inline const QString &userId(const int row) const { return m_modeldata.at(row).at(USER_COL_ID); }
	inline void setUserId(const uint row, const QString &new_id) { m_modeldata[row][USER_COL_ID] = new_id; }

	Q_INVOKABLE inline QString userName(const int row) const { return row >= 0 && row < m_modeldata.count() ? _userName(row) : QString{}; }
	inline const QString &_userName(const uint row) const { return m_modeldata.at(row).at(USER_COL_NAME); }
	Q_INVOKABLE inline void setUserName(const int row, const QString &new_name)
	{
		if (new_name != _userName(row))
		{
			m_modeldata[row][USER_COL_NAME] = new_name;
			emit userModified(row, USER_COL_NAME);
		}
	}

	Q_INVOKABLE void setPassword(const QString& passwd);
	Q_INVOKABLE void getPassword();

	Q_INVOKABLE inline QDate birthDate(const int row) const
	{
		return row >= 0 && row < m_modeldata.count() ? QDate::fromJulianDay(_birthDate(row).toLongLong()) : QDate::currentDate();
	}
	Q_INVOKABLE inline int birthYear(const int row) const { return birthDate(row).year(); }
	Q_INVOKABLE inline QString birthDateFancy(const int row) const
	{
		return appUtils()->formatDate(birthDate(row));
	}
	inline const QString &_birthDate(const uint row) const { return m_modeldata.at(row).at(USER_COL_BIRTHDAY); }
	Q_INVOKABLE inline void setBirthDate(const uint row, const QDate& new_date)
	{
		if (new_date != birthDate(row))
		{
			m_modeldata[row][USER_COL_BIRTHDAY] = QString::number(new_date.toJulianDay());
			emit userModified(row, USER_COL_BIRTHDAY);
		}
	}

	Q_INVOKABLE inline uint sex(const int row) const { return row >= 0 && row < m_modeldata.count() ? _sex(row).toUInt() : 2; }
	inline const QString &_sex(const uint row) const { return m_modeldata.at(row).at(USER_COL_SEX); }
	Q_INVOKABLE void setSex(const int row, const uint new_sex)
	{
		m_modeldata[row][USER_COL_SEX] = QString::number(new_sex);
		emit userModified(row, USER_COL_SEX);
	}

	Q_INVOKABLE inline QString phone(const int row) const { return row >= 0 && row < m_modeldata.count() ? _phone(row) : QString{}; }
	inline const QString &_phone(const uint row) const { return m_modeldata.at(row).at(USER_COL_PHONE); }
	Q_INVOKABLE inline void setPhone(const int row, const QString &new_phone)
	{
		m_modeldata[row][USER_COL_PHONE] = new_phone;
		emit userModified(row, USER_COL_PHONE);
	}

	Q_INVOKABLE inline QString email(const int row) const { return row >= 0 && row < m_modeldata.count() ? _email(row) : QString{}; }
	inline const QString &_email(const uint row) const { return m_modeldata.at(row).at(USER_COL_EMAIL); }
	Q_INVOKABLE inline void setEmail(const int row, const QString &new_email)
	{
		m_modeldata[row][USER_COL_EMAIL] = new_email;
		emit userModified(row, USER_COL_EMAIL);
	}

	Q_INVOKABLE inline QString socialMedia(const int row, const int index) const
	{
		return row >= 0 && row < m_modeldata.count() ?
		appUtils()->getCompositeValue(index, _socialMedia(row), record_separator) :
		QString{};
	}
	inline const QString &_socialMedia(const uint row) const { return m_modeldata.at(row).at(USER_COL_SOCIALMEDIA); }
	Q_INVOKABLE inline void setSocialMedia(const int row, const uint index, const QString &new_social)
	{
		appUtils()->setCompositeValue(index, new_social, m_modeldata[row][USER_COL_SOCIALMEDIA], record_separator);
		emit userModified(row, USER_COL_SOCIALMEDIA);
	}

	Q_INVOKABLE inline QString userRole(const int row) const { return row >= 0 && row < m_modeldata.count() ? _userRole(row) : QString{}; }
	inline const QString &_userRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_USERROLE); }
	Q_INVOKABLE inline void setUserRole(const int row, const QString &new_role)
	{
		m_modeldata[row][USER_COL_USERROLE] = new_role;
		emit userModified(row, USER_COL_USERROLE);
	}

	Q_INVOKABLE inline QString coachRole(const int row) const { return row >= 0 && row < m_modeldata.count() ? _coachRole(row) : QString{}; }
	inline const QString &_coachRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_COACHROLE); }
	Q_INVOKABLE inline void setCoachRole(const int row, const QString &new_role)
	{
		m_modeldata[row][USER_COL_COACHROLE] = new_role;
		emit userModified(row, USER_COL_COACHROLE);
	}

	Q_INVOKABLE inline QString goal(const int row) const { return row >= 0 && row < m_modeldata.count() ? _goal(row) : QString{}; }
	inline const QString &_goal(const uint row) const { return m_modeldata.at(row).at(USER_COL_GOAL); }
	Q_INVOKABLE inline void setGoal(const int row, const QString &new_goal)
	{
		m_modeldata[row][USER_COL_GOAL] = new_goal;
		emit userModified(row, USER_COL_GOAL);
	}

	Q_INVOKABLE QString avatar(const int row) const;
	Q_INVOKABLE void setAvatar(const int row, const QString &new_avatar, const bool saveToDisk = true, const bool upload = true);
	Q_INVOKABLE inline QString defaultAvatar(const uint row) const
	{
		return sex(row) == 0 ? "image://tpimageprovider/m0"_L1 : "image://tpimageprovider/f1"_L1;
	}

	Q_INVOKABLE inline uint appUseMode(const int row) const { return row >= 0 && row < m_modeldata.count() ? _appUseMode(row).toUInt() : 0; }
	inline const QString &_appUseMode(const uint row) const { return m_modeldata.at(row).at(USER_COL_APP_USE_MODE); }
	Q_INVOKABLE void setAppUseMode(const int row, const int new_use_opt);

	inline OnlineUserInfo *availableCoaches() const { return m_availableCoaches; }
	inline OnlineUserInfo *pendingCoachesResponses() const { return m_pendingCoachesResponses; }
	inline QStringList coachesNames() const { return m_coachesNames; }
	Q_INVOKABLE inline int coachRow(const QString &coach_name) const { return m_coachesNames.indexOf(coach_name); }
	inline const QString defaultCoach() const { return m_coachesNames.count() > 0 ? m_coachesNames.at(0) : QString{}; }
	inline bool haveCoaches() const { return m_coachesNames.count() > 0; }
	void addCoach(const uint row);
	void delCoach(const uint row);
	const QString currentCoachName(const uint row) const;
	void checkCoachesReponses();

	inline OnlineUserInfo *pendingClientsRequests() const { return m_pendingClientRequests; }
	inline QStringList clientsNames() const { return m_clientsNames; }
	Q_INVOKABLE inline int clientRow(const QString &client_name) const { return m_clientsNames.indexOf(client_name); }
	inline const QString defaultClient() const { return m_clientsNames.count() > 0 ? m_clientsNames.at(0) : QString{}; }
	inline bool haveClients() const { return m_clientsNames.count() > 0; }
	void addClient(const uint row);
	void delClient(const uint row);
	void changeClient(const uint row, const QString &oldname);

	Q_INVOKABLE int getTemporaryUserInfo(OnlineUserInfo *tempUser, const uint userInfoRow);
	bool mainUserConfigured() const;

	Q_INVOKABLE void acceptUser(OnlineUserInfo *userInfo, const int userInfoRow);
	Q_INVOKABLE void rejectUser(OnlineUserInfo *userInfo, const int userInfoRow);
	Q_INVOKABLE void removeCoach(const uint row);
	Q_INVOKABLE void removeClient(const uint row);

	Q_INVOKABLE inline void cancelPendingOnlineRequests()
	{
		disconnect(this, &DBUserModel::mainUserOnlineCheckInChanged, nullptr, nullptr);
	}
	Q_INVOKABLE void checkUserOnline(const QString &email, const QString &password);
	Q_INVOKABLE void importFromOnlineServer();
	Q_INVOKABLE inline bool mainUserRegistered() const { return mb_userRegistered.has_value() && mb_userRegistered == true; }
	Q_INVOKABLE void setCoachPublicStatus(const bool bPublic);
	Q_INVOKABLE inline void viewResume(const uint row)
	{
		downloadResumeFromServer(row);
	}
	Q_INVOKABLE inline void viewResume(OnlineUserInfo *tempUser, const uint userInfoRow)
	{
		viewResume(getTemporaryUserInfo(tempUser, userInfoRow));
	}
	Q_INVOKABLE void uploadResume(const QString &resumeFileName);
	Q_INVOKABLE void setMainUserConfigurationFinished();
	Q_INVOKABLE inline bool isCoachRegistered() { return mb_coachRegistered ? mb_coachRegistered == true : false; }
	Q_INVOKABLE void sendRequestToCoaches();
	Q_INVOKABLE void getOnlineCoachesList(const bool get_list_only = false);

	inline int importFromFile(const QString &filename) override { return _importFromFile(filename, m_modeldata); }
	bool updateFromModel(TPListModel*) override;
	bool importFromString(const QString &user_data);

	inline bool isFieldFormatSpecial (const uint field) const override
	{
		switch (field)
		{
			default: return false;
			case USER_COL_BIRTHDAY:
			case USER_COL_SEX:
			case USER_COL_SOCIALMEDIA:
			case USER_COL_APP_USE_MODE:
				return true;
		}
	}
	QString formatFieldToExport(const uint field, const QString &fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;

public slots:
	void getPasswordFromUserInput(const int resultCode, const QString &password);
	void slot_removeNoLongerAvailableUser(const int row, bool remove);
	void slot_revokeCoachStatus(int new_use_opt, bool revoke);
	void slot_revokeClientStatus(int new_use_opt, bool revoke);

signals:
	void userModified(const uint row, const uint field = 100); //100 all fields
	void userRemoved(const uint row);
	void labelsChanged();
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

private:
	int m_tempRow;
	QString m_appDataPath, m_onlineUserId, m_password, m_defaultAvatar;
	std::optional<bool> mb_userRegistered, mb_coachRegistered;
	OnlineUserInfo *m_availableCoaches, *m_pendingClientRequests, *m_pendingCoachesResponses, *m_tempRowUserInfo;
	QStringList m_coachesNames, m_clientsNames;
	bool mb_onlineCheckInInProgress;
	QTimer *m_mainTimer;

	bool onlineCheckIn();
	void registerUserOnline();
	QString generateUniqueUserId() const;
	QString resume(const uint row) const;
	void checkIfCoachRegisteredOnline();
	void getUserOnlineProfile(const QString &netName, const QString &save_as_filename);
	void sendProfileToServer();
	void sendUserInfoToServer();
	void sendAvatarToServer();
	void downloadAvatarFromServer(const uint row);
	void downloadResumeFromServer(const uint row);
	void moveTempUserFilesToFinalUserDir(const QString &destDir, OnlineUserInfo *userInfo, const int userInfoRow) const;
	void clearTempUserFiles(OnlineUserInfo *userInfo, const int userInfoRow) const;
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
	int _importFromFile(const QString &filename, QList<QStringList> &targetModel);

	QString m_localProfileFile, m_onlineCoachesDir, m_dirForRequestedCoaches, m_dirForClientsRequests,
						m_dirForCurrentClients, m_dirForCurrentCoaches;
	static DBUserModel *_appUserModel;
	friend DBUserModel *appUserModel();
	friend class OnlineUserInfo;
};

inline DBUserModel *appUserModel() { return DBUserModel::_appUserModel; }
