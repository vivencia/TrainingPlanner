#ifndef DBUSERMODEL_H
#define DBUSERMODEL_H

#include "tplistmodel.h"

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
	explicit DBUserModel(QObject *parent = nullptr);

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

	Q_INVOKABLE inline int userId(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_ID).toInt() : -1; }
	Q_INVOKABLE inline QString userName(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_NAME) : QString(); }
	Q_INVOKABLE void setUserName(const int row, const QString& new_name);
	Q_INVOKABLE inline QDate birthDate(const int row) const
	{
		return row >= 0 ?
			QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(USER_COL_BIRTHDAY)).toLongLong()) :
			QDate::currentDate();
	}
	Q_INVOKABLE void setBirthDate(const int row, const QDate& new_date);
	Q_INVOKABLE inline uint sex(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_SEX).toUInt() : 2; }
	Q_INVOKABLE void setSex(const int row, const uint new_sex);
	Q_INVOKABLE inline QString phone(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_PHONE) : QString(); }
	Q_INVOKABLE void setPhone(const int row, const QString& new_phone);
	Q_INVOKABLE inline QString email(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_EMAIL) : QString(); }
	Q_INVOKABLE void setEmail(const int row, const QString& new_email);
	Q_INVOKABLE inline QString socialMedia(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_SOCIALMEDIA) : QString(); }
	Q_INVOKABLE void setSocialMedia(const int row, const QString& new_social);
	Q_INVOKABLE inline QString userRole(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_USERROLE) : QString(); }
	Q_INVOKABLE void setUserRole(const int row, const QString& new_role);
	Q_INVOKABLE inline QString coachRole(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_COACHROLE) : QString(); }
	Q_INVOKABLE void setCoachRole(const int row, const QString& new_role);
	Q_INVOKABLE inline QString goal(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_GOAL) : QString(); }
	Q_INVOKABLE void setGoal(const int row, const QString& new_goal);
	Q_INVOKABLE inline QString avatar(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_AVATAR) : QString(); }
	Q_INVOKABLE void setAvatar(const int row, const QString& new_avatar);
	Q_INVOKABLE inline int appUseMode(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_APP_USE_MODE).toUInt() : 0; }
	Q_INVOKABLE void setAppUseMode(const int row, const int new_use_opt);
	Q_INVOKABLE inline int currentCoach(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_CURRENT_COACH).toUInt() : -1; }
	Q_INVOKABLE void setCurrentCoach(const int row, const int new_current_coach);
	Q_INVOKABLE inline int currentUser(const int row) const { return row >= 0 ? m_modeldata.at(row).at(USER_COL_CURRENT_USER).toUInt() : -1; }
	Q_INVOKABLE void setCurrentUser(const int row, const int new_current_user);

	Q_INVOKABLE inline bool isEmpty() const { return mb_empty; }
	void setIsEmpty(const bool empty) { mb_empty = empty; }

	virtual bool importFromFile(const QString& filename) override;
	virtual inline bool isFieldFormatSpecial (const uint field) const override
	{
		switch (field)
		{
			default: return false;
			case USER_COL_BIRTHDAY:
			case USER_COL_AVATAR:
			case USER_COL_SEX:
				return true;
		}
	}
	virtual QString formatFieldToExport(const uint field, const QString& fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString& fieldValue) const;
	virtual bool updateFromModel(const TPListModel* model) override;


signals:
	void appUseModeChanged(const int row);
	void userAdded(const uint row);

private:
	bool mb_empty;
	int m_searchRow;
};
#endif // DBUSERMODEL_H
