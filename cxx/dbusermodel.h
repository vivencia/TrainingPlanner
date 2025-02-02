#ifndef DBUSERMODEL_H
#define DBUSERMODEL_H

#include "tplistmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#define USER_COL_NET_NAME 0
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
#define USER_COL_AVATAR 10
#define USER_COL_APP_USE_MODE 11
#define USER_COL_CURRENT_COACH 12
#define USER_COL_CURRENT_CLIENT 13

#define USER_TOTAL_COLS USER_COL_CURRENT_CLIENT + 1

#define APP_USE_MODE_CLIENTS 0
#define APP_USE_MODE_SINGLE_USER 1
#define APP_USE_MODE_SINGLE_COACH 2
#define APP_USE_MODE_SINGLE_USER_WITH_COACH 3
#define APP_USE_MODE_COACH_USER_WITH_COACH 4

class DBUserModel : public TPListModel
{

Q_OBJECT

Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString birthdayLabel READ birthdayLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString sexLabel READ sexLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString phoneLabel READ phoneLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString emailLabel READ emailLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString socialMediaLabel READ socialMediaLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString userRoleLabel READ userRoleLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString coachRoleLabel READ coachRoleLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString goalLabel READ goalLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString avatarLabel READ avatarLabel NOTIFY labelsChanged FINAL)

public:
	explicit DBUserModel(QObject *parent = nullptr, const bool bMainUserModel = true);

	inline QString nameLabel() const { return mColumnNames.at(USER_COL_NAME); }
	inline QString birthdayLabel() const { return mColumnNames.at(USER_COL_BIRTHDAY); }
	inline QString sexLabel() const { return mColumnNames.at(USER_COL_SEX); }
	inline QString phoneLabel() const { return mColumnNames.at(USER_COL_PHONE); }
	inline QString emailLabel() const { return mColumnNames.at(USER_COL_EMAIL); }
	inline QString socialMediaLabel() const { return mColumnNames.at(USER_COL_SOCIALMEDIA); }
	inline QString userRoleLabel() const { return mColumnNames.at(USER_COL_USERROLE); }
	inline QString coachRoleLabel() const { return mColumnNames.at(USER_COL_COACHROLE); }
	inline QString goalLabel() const { return mColumnNames.at(USER_COL_GOAL); }
	inline QString avatarLabel() const { return mColumnNames.at(USER_COL_AVATAR); }

	Q_INVOKABLE int addUser(const bool bCoach);
	Q_INVOKABLE uint removeUser(const int row, const bool bCoach);

	Q_INVOKABLE int findFirstUser(const bool bCoach = false);
	Q_INVOKABLE int findNextUser(const bool bCoach = false);
	Q_INVOKABLE int findPrevUser(const bool bCoach = false);
	Q_INVOKABLE int findLastUser(const bool bCoach = false);
	const int getRowByCoachName(const QString &coachname) const;
	inline bool isCoach(const uint row) const
	{
		const uint app_use_mode{appUseMode(row)};
		return app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH;
	}
	inline bool isUser(const uint row) const
	{
		return !isCoach(row);
	}

	Q_INVOKABLE QStringList getCoaches() const;
	Q_INVOKABLE QStringList getClients() const;

	inline int userId(const uint row) const { return _userId(row).toInt(); }
	inline const QString &_userId(const int row) const { return m_modeldata.at(row).at(USER_COL_ID); }
	inline void setUserId(const uint row, const QString &new_id) { m_modeldata[row][USER_COL_ID] = new_id; }

	Q_INVOKABLE uint userRow(const QString &userName) const;
	Q_INVOKABLE inline QString userName(const int row) const { return row >= 0 && row < m_modeldata.count() ? _userName(row) : QString(); }
	inline const QString &_userName(const uint row) const { return m_modeldata.at(row).at(USER_COL_NAME); }
	Q_INVOKABLE void setUserName(const int row, const QString &new_name, const int prev_use_mode = -1, const QDate &prev_birthdate = QDate{},
								 int ret_code = -1000, const QString &networkReply = QString{});

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
			const QDate prev_date{std::move(birthDate(row))};
			m_modeldata[row][USER_COL_BIRTHDAY] = QString::number(new_date.toJulianDay());
			emit userModified(row, USER_COL_BIRTHDAY);
			if (!_userName(row).isEmpty())
				setUserName(row, userName(row), -1, prev_date);
		}
	}

	Q_INVOKABLE inline uint sex(const int row) const { return row >= 0 && row < m_modeldata.count() ? _sex(row).toUInt() : 2; }
	inline const QString &_sex(const uint row) const { return m_modeldata.at(row).at(USER_COL_SEX); }
	Q_INVOKABLE void setSex(const int row, const uint new_sex)
	{
		m_modeldata[row][USER_COL_SEX] = QString::number(new_sex);
		emit userModified(row, USER_COL_SEX);
		setAvatar(row, new_sex == 0 ? std::move("image://tpimageprovider/m5"_L1) : std::move("image://tpimageprovider/f1"_L1));
	}

	Q_INVOKABLE inline QString phone(const int row) const { return row >= 0 && row < m_modeldata.count() ? _phone(row) : QString(); }
	inline const QString &_phone(const uint row) const { return m_modeldata.at(row).at(USER_COL_PHONE); }
	Q_INVOKABLE inline void setPhone(const int row, const QString &new_phone)
	{
		m_modeldata[row][USER_COL_PHONE] = new_phone;
		emit userModified(row, USER_COL_PHONE);
	}

	Q_INVOKABLE inline QString email(const int row) const { return row >= 0 && row < m_modeldata.count() ? _email(row) : QString(); }
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
		QString();
	}
	inline const QString &_socialMedia(const uint row) const { return m_modeldata.at(row).at(USER_COL_SOCIALMEDIA); }
	Q_INVOKABLE inline void setSocialMedia(const int row, const uint index, const QString &new_social)
	{
		appUtils()->setCompositeValue(index, new_social, m_modeldata[row][USER_COL_SOCIALMEDIA], record_separator);
		emit userModified(row, USER_COL_SOCIALMEDIA);
	}

	Q_INVOKABLE inline QString userRole(const int row) const { return row >= 0 && row < m_modeldata.count() ? _userRole(row) : QString(); }
	inline const QString &_userRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_USERROLE); }
	Q_INVOKABLE inline void setUserRole(const int row, const QString &new_role)
	{
		m_modeldata[row][USER_COL_USERROLE] = new_role;
		emit userModified(row, USER_COL_USERROLE);
	}

	Q_INVOKABLE inline QString coachRole(const int row) const { return row >= 0 && row < m_modeldata.count() ? _coachRole(row) : QString(); }
	inline const QString &_coachRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_COACHROLE); }
	Q_INVOKABLE inline void setCoachRole(const int row, const QString &new_role)
	{
		m_modeldata[row][USER_COL_COACHROLE] = new_role;
		emit userModified(row, USER_COL_COACHROLE);
	}

	Q_INVOKABLE inline QString goal(const int row) const { return row >= 0 && row < m_modeldata.count() ? _goal(row) : QString(); }
	inline const QString &_goal(const uint row) const { return m_modeldata.at(row).at(USER_COL_GOAL); }
	Q_INVOKABLE inline void setGoal(const int row, const QString &new_goal)
	{
		m_modeldata[row][USER_COL_GOAL] = new_goal;
		emit userModified(row, USER_COL_GOAL);
	}

	Q_INVOKABLE inline QString avatar(const int row) const { return row >= 0 && row < m_modeldata.count() ? _avatar(row) : QString(); }
	inline const QString &_avatar(const uint row) const { return m_modeldata.at(row).at(USER_COL_AVATAR); }
	Q_INVOKABLE void setAvatar(const int row, const QString &new_avatar);

	Q_INVOKABLE inline uint appUseMode(const int row) const { return row >= 0 && row < m_modeldata.count() ? _appUseMode(row).toUInt() : 0; }
	inline const QString &_appUseMode(const uint row) const { return m_modeldata.at(row).at(USER_COL_APP_USE_MODE); }
	Q_INVOKABLE inline void setAppUseMode(const int row, const int new_use_opt)
	{
		const uint prev_use_mode{appUseMode(row)};
		if (new_use_opt != prev_use_mode)
		{
			setCoachPublicStatus(row, false);
			m_modeldata[row][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
			emit userModified(row, USER_COL_APP_USE_MODE);
			setUserName(row, userName(row), prev_use_mode);
		}
	}

	Q_INVOKABLE inline int currentCoach(const int row) const { return row >= 0 && row < m_modeldata.count() ? _currentCoach(row).toUInt() : -1; }
	inline const QString &_currentCoach(const uint row) const { return m_modeldata.at(row).at(USER_COL_CURRENT_COACH); }
	inline const QString currentCoachName(const uint row) const
	{
		return currentCoach(row) >= 0 ? m_modeldata.at(row).at(USER_COL_NAME) : tr("(Select coach ...)");
	}

	Q_INVOKABLE inline void setCurrentCoach(const int row, const int new_current_coach)
	{
		m_modeldata[row][USER_COL_CURRENT_COACH] = QString::number(new_current_coach);
		emit userModified(row, USER_COL_CURRENT_COACH);
	}

	Q_INVOKABLE inline int currentClient() const { return _currentClient().toUInt(); }
	inline const QString &_currentClient() const { return m_modeldata.at(0).at(USER_COL_CURRENT_CLIENT); }

	Q_INVOKABLE inline void setCurrentClient(const int row, const int new_current_user)
	{
		m_modeldata[row][USER_COL_CURRENT_CLIENT] = QString::number(new_current_user);
		emit userModified(row, USER_COL_CURRENT_CLIENT);
	}

	Q_INVOKABLE void setCoachPublicStatus(const uint row, const bool bPublic);
	Q_INVOKABLE void isCoachAlreadyRegisteredOnline(const uint row);
	Q_INVOKABLE void uploadResume(const uint row, const QString &resumeFileName);
	Q_INVOKABLE void downloadResume(const uint coach_index);
	Q_INVOKABLE void mainUserConfigurationFinished();
	Q_INVOKABLE inline bool isCoachRegistered() { return mb_coachRegistered ? mb_coachRegistered == true : false; }
	Q_INVOKABLE void sendRequestToCoaches(const QList<bool> &selectedCoaches);

	void getOnlineCoachesList();
	void getUserOnlineProfile(const QString& netName, uint n_max_profiles = 1);

	inline QString networkUserName(const uint row)
	{
		if (row == 0)
		{
			if (m_networkName.isEmpty())
				m_networkName = std::move(getNetworkUserName(_userName(0), appUseMode(0), birthDate(0)));
			return m_networkName;
		}
		return getNetworkUserName(_userName(row), appUseMode(row), birthDate(row));
	}
	inline QString networkUserPassword(const uint row) const
	{
		return makeUserPassword(_userName(row));
	}

	inline int importFromFile(const QString &filename) override { return _importFromFile(filename, m_modeldata); }
	bool updateFromModel(TPListModel*) override;

	inline bool isFieldFormatSpecial (const uint field) const override
	{
		switch (field)
		{
			default: return false;
			case USER_COL_BIRTHDAY:
			case USER_COL_SEX:
			case USER_COL_SOCIALMEDIA:
			case USER_COL_AVATAR:
				return true;
		}
	}
	QString formatFieldToExport(const uint field, const QString &fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;

signals:
	void userModified(const uint row, const uint field);
	void userAddedOrRemoved(const uint row, const bool bAdded);
	void mainUserConfigurationFinishedSignal();
	void labelsChanged();
	void userNameOK(int row, bool b_ok);
	void coachOnlineStatus(bool registered);
	void coachesListReceived(const QStringList& coaches_list);
	void userProfileAcquired();

private:
	bool mb_empty;
	int m_searchRow;
	QString m_networkName;
	std::optional<bool> mb_coachRegistered;
	QList<QStringList> m_onlineUserInfo;

	QString getNetworkUserName(const QString &userName, const uint app_use_mode, const QDate &birthdate) const;
	QString makeUserPassword(const QString &userName) const;
	void _setUserName(const uint row, const QString &new_name);
	int _importFromFile(const QString &filename, QList<QStringList>& targetModel);
	static DBUserModel* _appUserModel;
	friend DBUserModel* appUserModel();
};

inline DBUserModel* appUserModel() { return DBUserModel::_appUserModel; }

#endif // DBUSERMODEL_H
