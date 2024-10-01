#ifndef DBUSERMODEL_H
#define DBUSERMODEL_H

#include "tplistmodel.h"
#include "tpglobals.h"
#include "tputils.h"

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
#define USER_COL_CURRENT_USER 13

#define USER_TOTAL_COLS USER_COL_CURRENT_USER + 1

#define APP_USE_MODE_CLIENTS 0
#define APP_USE_MODE_SINGLE_USER 1
#define APP_USE_MODE_SINGLE_COACH 2
#define APP_USE_MODE_SINGLE_USER_WITH_COACH 3
#define APP_USE_MODE_COACH_USER_WITH_COACH 4

class DBUserModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBUserModel(QObject* parent = nullptr);
	inline DBUserModel(const DBUserModel& other)
		: TPListModel{other.parent()}, mb_empty(other.mb_empty), m_searchRow(other.m_searchRow)
		{ copy(other); }
	inline ~DBUserModel() {}

	Q_INVOKABLE int addUser(const bool bCoach);
	Q_INVOKABLE uint removeUser(const int row, const bool bCoach);

	Q_INVOKABLE int findFirstUser(const bool bCoach = false);
	Q_INVOKABLE int findNextUser(const bool bCoach = false);
	Q_INVOKABLE int findPrevUser(const bool bCoach = false);
	Q_INVOKABLE int findLastUser(const bool bCoach = false);
	Q_INVOKABLE QString getCurrentUserName(const bool bCoach) const;
	const int getRowByCoachName(const QString& coachname) const;
	Q_INVOKABLE const int getRowByName(const QString& username) const;

	Q_INVOKABLE QStringList getCoaches() const;
	Q_INVOKABLE QStringList getClients() const;

	inline int userId(const uint row) const { return _userId(row).toInt(); }
	inline const QString& _userId(const int row) const { return m_modeldata.at(row).at(USER_COL_ID); }
	inline void setUserId(const uint row, const QString& new_id) { m_modeldata[row][USER_COL_ID] = new_id; }

	Q_INVOKABLE inline QString userName(const int row) const { return row < m_modeldata.count() ? _userName(row) : QString(); }
	inline const QString& _userName(const uint row) const { return m_modeldata.at(row).at(USER_COL_NAME); }
	Q_INVOKABLE inline void setUserName(const int row, const QString& new_name)
	{
		m_modeldata[row][USER_COL_NAME] = new_name;
		emit userModified(row);
		if (m_modeldata.count() > 1 && m_modeldata.at(row).at(USER_COL_ID) == STR_MINUS_ONE)
			emit userAdded(row);
	}

	Q_INVOKABLE inline QDate birthDate(const int row) const
	{
		return row < m_modeldata.count() ? QDate::fromJulianDay(_birthDate(row).toLongLong()) : QDate::currentDate();
	}
	Q_INVOKABLE inline QString birthDateFancy(const int row) const
	{
		return appUtils()->formatDate(birthDate(row));
	}
	inline const QString& _birthDate(const uint row) const { return m_modeldata.at(row).at(USER_COL_BIRTHDAY); }
	Q_INVOKABLE inline void setBirthDate(const uint row, const QDate& new_date)
	{
		m_modeldata[row][USER_COL_BIRTHDAY] = QString::number(new_date.toJulianDay());
		emit userModified(row);
	}

	Q_INVOKABLE inline uint sex(const int row) const { return row < m_modeldata.count() ? _sex(row).toUInt() : 2; }
	inline const QString& _sex(const uint row) const { return m_modeldata.at(row).at(USER_COL_SEX); }
	Q_INVOKABLE void setSex(const int row, const uint new_sex)
	{
		m_modeldata[row][USER_COL_SEX] = QString::number(new_sex);
		emit userModified(row);
	}

	Q_INVOKABLE inline QString phone(const int row) const { return row < m_modeldata.count() ? _phone(row) : QString(); }
	inline const QString& _phone(const uint row) const { return m_modeldata.at(row).at(USER_COL_PHONE); }
	Q_INVOKABLE void setPhone(const int row, const QString& new_phone)
	{
		m_modeldata[row][USER_COL_PHONE] = new_phone;
		emit userModified(row);
	}

	Q_INVOKABLE inline QString email(const int row) const { return row < m_modeldata.count() ? _email(row) : QString(); }
	inline const QString& _email(const uint row) const { return m_modeldata.at(row).at(USER_COL_EMAIL); }
	Q_INVOKABLE void setEmail(const int row, const QString& new_email)
	{
		m_modeldata[row][USER_COL_EMAIL] = new_email;
		emit userModified(row);
	}

	Q_INVOKABLE inline QString socialMedia(const int row, const int index) const
	{
		return row < m_modeldata.count() ?
		appUtils()->getCompositeValue(index, _socialMedia(row), record_separator) :
		QString();
	}
	inline const QString& _socialMedia(const uint row) const { return m_modeldata.at(row).at(USER_COL_SOCIALMEDIA); }
	Q_INVOKABLE void setSocialMedia(const int row, const uint index, const QString& new_social)
	{
		appUtils()->setCompositeValue(index, new_social, m_modeldata[row][USER_COL_SOCIALMEDIA], record_separator);
		emit userModified(row);
	}

	Q_INVOKABLE inline QString userRole(const int row) const { return row < m_modeldata.count() ? _userRole(row) : QString(); }
	inline const QString& _userRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_USERROLE); }
	Q_INVOKABLE void setUserRole(const int row, const QString& new_role)
	{
		m_modeldata[row][USER_COL_USERROLE] = new_role;
		emit userModified(row);
	}

	Q_INVOKABLE inline QString coachRole(const int row) const { return row < m_modeldata.count() ? _coachRole(row) : QString(); }
	inline const QString& _coachRole(const uint row) const { return m_modeldata.at(row).at(USER_COL_COACHROLE); }
	Q_INVOKABLE void setCoachRole(const int row, const QString& new_role)
	{
		m_modeldata[row][USER_COL_COACHROLE] = new_role;
		emit userModified(row);
	}

	Q_INVOKABLE inline QString goal(const int row) const { return row < m_modeldata.count() ? _goal(row) : QString(); }
	inline const QString& _goal(const uint row) const { return m_modeldata.at(row).at(USER_COL_GOAL); }
	Q_INVOKABLE void setGoal(const int row, const QString& new_goal)
	{
		m_modeldata[row][USER_COL_GOAL] = new_goal;
		emit userModified(row);
	}

	Q_INVOKABLE inline QString avatar(const int row) const { return row < m_modeldata.count() ? _avatar(row) : QString(); }
	inline const QString& _avatar(const uint row) const { return m_modeldata.at(row).at(USER_COL_AVATAR); }
	Q_INVOKABLE void setAvatar(const int row, const QString& new_avatar)
	{
		m_modeldata[row][USER_COL_AVATAR] = new_avatar;
		emit userModified(row);
	}

	Q_INVOKABLE inline int appUseMode(const int row) const { return row < m_modeldata.count() ? _appUseMode(row).toUInt() : 0; }
	inline const QString& _appUseMode(const uint row) const { return m_modeldata.at(row).at(USER_COL_APP_USE_MODE); }
	Q_INVOKABLE void setAppUseMode(const int row, const int new_use_opt)
	{
		m_modeldata[row][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
		emit appUseModeChanged(row);
		emit userModified(row);
	}

	Q_INVOKABLE inline int currentCoach(const int row) const { return row < m_modeldata.count() ? _currentCoach(row).toUInt() : -1; }
	inline const QString& _currentCoach(const uint row) const { return m_modeldata.at(row).at(USER_COL_CURRENT_COACH); }
	Q_INVOKABLE void setCurrentCoach(const int row, const int new_current_coach)
	{
		m_modeldata[row][USER_COL_CURRENT_COACH] = QString::number(new_current_coach);
		emit userModified(row);
	}

	Q_INVOKABLE inline int currentUser(const int row) const { return row < m_modeldata.count() ? _currentUser(row).toUInt() : -1; }
	inline const QString& _currentUser(const uint row) const { return m_modeldata.at(row).at(USER_COL_CURRENT_USER); }
	Q_INVOKABLE void setCurrentUser(const int row, const int new_current_user)
	{
		m_modeldata[row][USER_COL_CURRENT_USER] = QString::number(new_current_user);
		emit userModified(row);
	}

	Q_INVOKABLE inline bool isEmpty() const { return mb_empty; }
	void setIsEmpty(const bool empty) { mb_empty = empty; }

	virtual int importFromFile(const QString& filename) override;
	virtual bool updateFromModel(const TPListModel* const) override;

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
	QString formatFieldToExport(const uint field, const QString& fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString& fieldValue) const;

signals:
	void userModified(const uint row);
	void appUseModeChanged(const int row);
	void userAdded(const uint row);

private:
	bool mb_empty;
	int m_searchRow;

	static DBUserModel* app_user_model;
	friend DBUserModel* appUserModel();
};
Q_DECLARE_METATYPE(DBUserModel*)

inline DBUserModel* appUserModel() { return DBUserModel::app_user_model; }

#endif // DBUSERMODEL_H
