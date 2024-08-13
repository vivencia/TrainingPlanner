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
#define APP_USE_MODE_COACH_USER_WITH_COACHES 4

class DBUserModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged FINAL)
Q_PROPERTY(QDate birthDate READ birthDate WRITE setBirthDate NOTIFY birthDateChanged FINAL)
Q_PROPERTY(QString sex READ sex WRITE setSex NOTIFY sexChanged FINAL)
Q_PROPERTY(QString phone READ phone WRITE setPhone NOTIFY phoneChanged FINAL)
Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged FINAL)
Q_PROPERTY(QString socialMedia READ socialMedia WRITE setSocialMedia NOTIFY socialMediaChanged FINAL)
Q_PROPERTY(QString userRole READ userRole WRITE setUserRole NOTIFY userRoleChanged FINAL)
Q_PROPERTY(QString coachRole READ coachRole WRITE setCoachRole NOTIFY coachRoleChanged FINAL)
Q_PROPERTY(QString goal READ goal WRITE setGoal NOTIFY goalChanged FINAL)
Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged FINAL)
Q_PROPERTY(int appUseMode READ appUseMode WRITE setAppUseMode NOTIFY appUseModeChanged FINAL)
Q_PROPERTY(int currentCoach READ currentCoach WRITE setCurrentCoach NOTIFY currentCoachChanged FINAL)
Q_PROPERTY(int currentUser READ currentUser WRITE setCurrentUser NOTIFY currentUserChanged FINAL)

public:
	explicit DBUserModel(QObject *parent = nullptr);

	Q_INVOKABLE void addUser(const bool coach);
	Q_INVOKABLE void removeUser(const int row);

	int loadUserInfo(const QString& name);
	Q_INVOKABLE int findFirstUser(const bool bCoach = false);
	Q_INVOKABLE int findNextUser(const bool bCoach = false);
	Q_INVOKABLE int findPrevUser(const bool bCoach = false);
	Q_INVOKABLE int findLastUser(const bool bCoach = false);
	inline void setCurrentViewedUser(const uint user_row) { m_userRow = user_row; }
	Q_INVOKABLE inline uint currentViewedUser() const { return m_userRow; }

	inline uint mainUserAppUseMode() const { return m_modeldata.at(0).at(USER_COL_APP_USE_MODE).toInt(); }

	inline uint userId(const uint row) const { return m_modeldata.at(row).at(USER_COL_ID).toUInt(); }
	inline QString userName() const { return m_modeldata.at(m_userRow).at(USER_COL_NAME); }
	inline void setUserName(const QString& new_name) { m_modeldata[m_userRow][USER_COL_NAME] = new_name; emit userNameChanged(); setModified(true);}
	inline QDate birthDate() const { return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(m_userRow).at(USER_COL_BIRTHDAY)).toLongLong()); }
	inline void setBirthDate(const QDate& new_date) { m_modeldata[m_userRow][USER_COL_BIRTHDAY] = QString::number(new_date.toJulianDay()); emit birthDateChanged(); setModified(true); }
	inline QString sex() const { return m_modeldata.at(m_userRow).at(USER_COL_SEX); }
	inline void setSex(const QString& new_sex) { m_modeldata[m_userRow][USER_COL_SEX] = new_sex; emit sexChanged(); setModified(true); }
	inline QString phone() const { return m_modeldata.at(m_userRow).at(USER_COL_PHONE); }
	inline void setPhone(const QString& new_phone) { m_modeldata[m_userRow][USER_COL_PHONE] = new_phone; emit phoneChanged(); setModified(true); }
	inline QString email() const { return m_modeldata.at(m_userRow).at(USER_COL_EMAIL); }
	inline void setEmail(const QString& new_email) { m_modeldata[m_userRow][USER_COL_EMAIL] = new_email; emit emailChanged(); setModified(true); }
	inline QString socialMedia() const { return m_modeldata.at(m_userRow).at(USER_COL_SOCIALMEDIA); }
	inline void setSocialMedia(const QString& new_social) { m_modeldata[m_userRow][USER_COL_SOCIALMEDIA] = new_social; emit socialMediaChanged(); setModified(true); }
	inline QString userRole() const { return m_modeldata.at(m_userRow).at(USER_COL_USERROLE); }
	inline void setUserRole(const QString& new_role) { m_modeldata[m_userRow][USER_COL_USERROLE] = new_role; emit userRoleChanged(); setModified(true); }
	inline QString coachRole() const { return m_modeldata.at(m_userRow).at(USER_COL_COACHROLE); }
	inline void setCoachRole(const QString& new_role) { m_modeldata[m_userRow][USER_COL_COACHROLE] = new_role; emit coachRoleChanged(); setModified(true); }
	inline QString goal() const { return m_modeldata.at(m_userRow).at(USER_COL_GOAL); }
	inline void setGoal(const QString& new_goal) { m_modeldata[m_userRow][USER_COL_GOAL] = new_goal; emit goalChanged(); setModified(true); }
	inline QString avatar() const { return m_modeldata.at(m_userRow).at(USER_COL_AVATAR); }
	inline void setAvatar(const QString& new_avatar) { m_modeldata[m_userRow][USER_COL_AVATAR] = new_avatar; emit avatarChanged(); setModified(true); }
	inline int appUseMode() const { return m_modeldata.at(m_userRow).at(USER_COL_APP_USE_MODE).toInt(); }
	inline void setAppUseMode(const int new_use_opt) { m_modeldata[m_userRow][USER_COL_APP_USE_MODE] = QString::number(new_use_opt); emit appUseModeChanged(); setModified(true); }
	inline int currentCoach() const { return m_modeldata.at(m_userRow).at(USER_COL_CURRENT_COACH).toInt(); }
	inline void setCurrentCoach(const int new_current_coach) { m_modeldata[m_userRow][USER_COL_CURRENT_COACH] = QString::number(new_current_coach); emit currentCoachChanged(); setModified(true); }
	inline int currentUser() const { return m_modeldata.at(m_userRow).at(USER_COL_CURRENT_USER).toInt(); }
	inline void setCurrentUser(const int new_current_user) { m_modeldata[m_userRow][USER_COL_CURRENT_USER] = QString::number(new_current_user); emit currentUserChanged(); setModified(true); }

	Q_INVOKABLE inline bool isEmpty() const { return mb_empty; }
	inline void setIsEmpty(const bool empty) { mb_empty = empty; }

	virtual bool updateFromModel(const TPListModel* model) override;
	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual bool importFromFancyText(QFile* inFile, QString& inData) override;

signals:
	void userNameChanged();
	void birthDateChanged();
	void sexChanged();
	void phoneChanged();
	void emailChanged();
	void socialMediaChanged();
	void userRoleChanged();
	void coachRoleChanged();
	void goalChanged();
	void avatarChanged();
	void appUseModeChanged();
	void currentCoachChanged();
	void currentUserChanged();

private:
	bool mb_empty;
	uint m_userRow;
	int m_searchRow;
};
#endif // DBUSERMODEL_H
