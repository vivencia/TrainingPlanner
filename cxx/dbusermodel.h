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
#define USER_COL_ROLE 7
#define USER_COL_GOAL 8
#define USER_COL_AVATAR 9
#define USER_COL_COACH 10

#define USER_TOTAL_COLS USER_COL_COACH + 1

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
Q_PROPERTY(QString role READ role WRITE setRole NOTIFY roleChanged FINAL)
Q_PROPERTY(QString goal READ goal WRITE setGoal NOTIFY goalChanged FINAL)
Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged FINAL)
Q_PROPERTY(QString coach READ coach WRITE setCoach NOTIFY coachChanged FINAL)

public:
	explicit DBUserModel(QObject *parent = nullptr);

	int loadUserInfo(const QString& name);
	inline void setCurrentViewedUser(const uint user_row) { m_userRow = user_row; }
	inline uint currentViewedUser() const { return m_userRow; }

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
	inline QString role() const { return m_modeldata.at(m_userRow).at(USER_COL_ROLE); }
	inline void setRole(const QString& new_role) { m_modeldata[m_userRow][USER_COL_ROLE] = new_role; emit roleChanged(); setModified(true); }
	inline QString goal() const { return m_modeldata.at(m_userRow).at(USER_COL_GOAL); }
	inline void setGoal(const QString& new_goal) { m_modeldata[m_userRow][USER_COL_GOAL] = new_goal; emit goalChanged(); setModified(true); }
	inline QString avatar() const { return m_modeldata.at(m_userRow).at(USER_COL_AVATAR); }
	inline void setAvatar(const QString& new_avatar) { m_modeldata[m_userRow][USER_COL_AVATAR] = new_avatar; emit avatarChanged(); setModified(true); }
	inline QString coach() const { return m_modeldata.at(m_userRow).at(USER_COL_COACH); }
	inline void setCoach(const QString& new_coach) { m_modeldata[m_userRow][USER_COL_COACH] = new_coach; emit coachChanged(); setModified(true); }

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
	void roleChanged();
	void goalChanged();
	void avatarChanged();
	void coachChanged();

private:
	bool mb_empty;
	uint m_userRow;
};
#endif // DBUSERMODEL_H
