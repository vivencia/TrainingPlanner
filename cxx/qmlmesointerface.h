#ifndef QMLMESOINTERFACE_H
#define QMLMESOINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class QmlTDayInterface;
class QmlMesoSplitInterface;
class QmlMesoCalendarInterface;
class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QMLMesoInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged FINAL)
Q_PROPERTY(bool ownerIsCoach READ ownerIsCoach WRITE setOwnerIsCoach NOTIFY ownerIsCoachChanged FINAL)
Q_PROPERTY(bool hasCoach READ hasCoach WRITE setHasCoach NOTIFY hasCoachChanged FINAL)
Q_PROPERTY(bool realMeso READ realMeso WRITE setRealMeso NOTIFY realMesoChanged FINAL)
Q_PROPERTY(bool ownMeso READ ownMeso WRITE setOwnMeso NOTIFY ownMesoChanged FINAL)
Q_PROPERTY(QString nameLabel READ nameLabel CONSTANT FINAL)
Q_PROPERTY(QString coachLabel READ coachLabel CONSTANT FINAL)
Q_PROPERTY(QString clientLabel READ clientLabel CONSTANT FINAL)
Q_PROPERTY(QString typeLabel READ typeLabel CONSTANT FINAL)
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
Q_PROPERTY(QString coach READ coach WRITE setCoach NOTIFY coachChanged FINAL)
Q_PROPERTY(QString client READ client WRITE setClient NOTIFY clientChanged FINAL)
Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(QString startDate READ startDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QString endDate READ endDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QDate minimumMesoStartDate READ minimumMesoStartDate WRITE setMinimumMesoStartDate NOTIFY minimumMesoStartDateChanged FINAL)
Q_PROPERTY(QDate maximumMesoEndDate READ maximumMesoEndDate WRITE setMaximumMesoEndDate NOTIFY maximumMesoEndDateChanged FINAL)
Q_PROPERTY(QDate calendarStartDate READ calendarStartDate WRITE setCalendarStartDate NOTIFY calendarStartDateChanged FINAL)

public:
	explicit QMLMesoInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx);
	~QMLMesoInterface();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	inline int mesoIdx() const { return m_mesoIdx; }
	void setMesoIdx(const int new_value);

	inline bool ownerIsCoach() const { return m_bOwnerIsCoach; }
	inline void setOwnerIsCoach(const bool new_value) { if (m_bOwnerIsCoach != new_value) { m_bOwnerIsCoach = new_value; emit ownerIsCoachChanged(); } };

	inline bool hasCoach() const { return m_bHasCoach; }
	inline void setHasCoach(const bool new_value) { if (m_bHasCoach != new_value) { m_bHasCoach = new_value; emit hasCoachChanged(); } };

	inline bool realMeso() const { return m_bRealMeso; }
	void setRealMeso(const bool new_value, const bool bFromQml = true);

	inline bool ownMeso() const { return m_bOwnMeso; }
	void setOwnMeso(const bool new_value, const bool bFromQml = true);

	QString nameLabel() const;
	QString coachLabel() const;
	QString clientLabel() const;
	QString typeLabel() const;

	inline QString name() const { return m_name; }
	void setName(const QString& new_value, const bool bFromQml = true);

	inline QString coach() const { return m_coach; }
	void setCoach(const QString& new_value, const bool bFromQml = true);

	inline QString client() const { return m_client; }
	void setClient(const QString& new_value, const bool bFromQml = true);

	inline QString type() const { return m_type; }
	void setType(const QString& new_value, const bool bFromQml = true);

	inline QString mesoStartDate() const { return m_startDate; }
	inline QDate minimumMesoStartDate() const { return m_minimumMesoStartDate; }
	void setMinimumMesoStartDate(const QDate& new_value, const bool bFromQml = true);

	inline QString mesoEndDate() const { return m_endDate; }
	inline QDate maximumMesoEndDate() const { return m_maximumMesoEndDate; }
	void setMaximumMesoEndDate(const QDate& new_value, const bool bFromQml = true);

	inline QDate calendarStartDate() const { return m_calendarStartDate; }
	void setCalendarStartDate(const QDate& new_value);
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday);

	void getMesocyclePage();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	void importMeso(const QString& filename = QString());

signals:
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void mesoIdxChanged();
	void muscularGroupIdChanged();
	void ownerIsCoachChanged();
	void hasCoachChanged();
	void realMesoChanged();
	void ownMesoChanged();
	void nameChanged();
	void coachChanged();
	void clientChanged();
	void typeChanged();
	void startDateChanged();
	void endDateChanged();
	void minimumMesoStartDateChanged();
	void maximumMesoEndDateChanged();
	void calendarStartDateChanged();
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_mesoPage;
	QVariantMap m_mesoProperties;
	uint m_muscularGroupId;

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	int m_mesoIdx;
	bool m_bOwnerIsCoach, m_bHasCoach, m_bRealMeso, m_bOwnMeso;
	QString m_name, m_coach, m_client, m_type, m_startDate, m_endDate;
	QDate m_minimumMesoStartDate, m_maximumMesoEndDate, m_calendarStartDate;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	QMap<QDate,QmlTDayInterface*> m_tDayPages;
	QmlMesoSplitInterface* m_exercisesPage;
	QmlMesoCalendarInterface* m_calendarPage;

	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();
	void setPropertiesBasedOnUseMode();

	friend class TPAppControl;
};

#endif // QMLMESOINTERFACE_H
