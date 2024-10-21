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

Q_PROPERTY(bool ownerIsCoach READ ownerIsCoach WRITE setOwnerIsCoach NOTIFY ownerIsCoachChanged FINAL)
Q_PROPERTY(bool hasCoach READ hasCoach WRITE setHasCoach NOTIFY hasCoachChanged FINAL)
Q_PROPERTY(bool realMeso READ realMeso WRITE setRealMeso NOTIFY realMesoChanged FINAL)
Q_PROPERTY(bool ownMeso READ ownMeso WRITE setOwnMeso NOTIFY ownMesoChanged FINAL)
Q_PROPERTY(bool isNewMeso READ isNewMeso NOTIFY isNewMesoChanged FINAL)
Q_PROPERTY(QString nameLabel READ nameLabel CONSTANT FINAL)
Q_PROPERTY(QString coachLabel READ coachLabel CONSTANT FINAL)
Q_PROPERTY(QString clientLabel READ clientLabel CONSTANT FINAL)
Q_PROPERTY(QString typeLabel READ typeLabel CONSTANT FINAL)
Q_PROPERTY(QString startDateLabel READ startDateLabel CONSTANT FINAL)
Q_PROPERTY(QString endDateLabel READ endDateLabel CONSTANT FINAL)
Q_PROPERTY(QString weeksLabel READ weeksLabel CONSTANT FINAL)
Q_PROPERTY(QString splitLabel READ splitLabel CONSTANT FINAL)
Q_PROPERTY(QString notesLabel READ notesLabel CONSTANT FINAL)
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
Q_PROPERTY(QString coach READ coach WRITE setCoach NOTIFY coachChanged FINAL)
Q_PROPERTY(QString client READ client WRITE setClient NOTIFY clientChanged FINAL)
Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged FINAL)
Q_PROPERTY(QString startDate READ startDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QString endDate READ endDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QString weeks READ weeks NOTIFY weeksChanged FINAL)
Q_PROPERTY(QString split READ split WRITE setSplit NOTIFY splitChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)
Q_PROPERTY(QString muscularGroupA READ muscularGroupA WRITE setMuscularGroupA NOTIFY muscularGroupAChanged FINAL)
Q_PROPERTY(QString muscularGroupB READ muscularGroupB WRITE setMuscularGroupB NOTIFY muscularGroupBChanged FINAL)
Q_PROPERTY(QString muscularGroupC READ muscularGroupC WRITE setMuscularGroupC NOTIFY muscularGroupCChanged FINAL)
Q_PROPERTY(QString muscularGroupD READ muscularGroupD WRITE setMuscularGroupD NOTIFY muscularGroupDChanged FINAL)
Q_PROPERTY(QString muscularGroupE READ muscularGroupE WRITE setMuscularGroupE NOTIFY muscularGroupEChanged FINAL)
Q_PROPERTY(QString muscularGroupF READ muscularGroupF WRITE setMuscularGroupF NOTIFY muscularGroupFChanged FINAL)
Q_PROPERTY(QString muscularGroupR READ muscularGroupR CONSTANT FINAL)
Q_PROPERTY(QDate minimumMesoStartDate READ minimumMesoStartDate WRITE setMinimumMesoStartDate NOTIFY minimumMesoStartDateChanged FINAL)
Q_PROPERTY(QDate maximumMesoEndDate READ maximumMesoEndDate WRITE setMaximumMesoEndDate NOTIFY maximumMesoEndDateChanged FINAL)
Q_PROPERTY(QDate calendarStartDate READ calendarStartDate WRITE setCalendarStartDate NOTIFY calendarStartDateChanged FINAL)

public:
	explicit inline QMLMesoInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
	: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_mesoComponent(nullptr), m_mesoIdx(meso_idx),
		m_exercisesPage(nullptr), m_calendarPage(nullptr) {}
	~QMLMesoInterface();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_value) { m_mesoIdx = new_value; }

	inline bool ownerIsCoach() const { return m_bOwnerIsCoach; }
	inline void setOwnerIsCoach(const bool new_value) { if (m_bOwnerIsCoach != new_value) { m_bOwnerIsCoach = new_value; emit ownerIsCoachChanged(); } };

	inline bool hasCoach() const { return m_bHasCoach; }
	inline void setHasCoach(const bool new_value) { if (m_bHasCoach != new_value) { m_bHasCoach = new_value; emit hasCoachChanged(); } };

	inline bool realMeso() const { return m_bRealMeso; }
	void setRealMeso(const bool new_value, const bool bFromQml = true);

	inline bool ownMeso() const { return m_bOwnMeso; }
	void setOwnMeso(const bool new_value, const bool bFromQml = true);

	bool isNewMeso() const;

	QString nameLabel() const;
	QString coachLabel() const;
	QString clientLabel() const;
	QString typeLabel() const;
	QString startDateLabel() const;
	QString endDateLabel() const;
	QString weeksLabel() const;
	QString splitLabel() const;
	QString notesLabel() const;

	inline QString name() const { return m_name; }
	void setName(const QString& new_value, const bool bFromQml = true);

	inline QString coach() const { return m_coach; }
	void setCoach(const QString& new_value, const bool bFromQml = true);

	inline QString client() const { return m_client; }
	void setClient(const QString& new_value, const bool bFromQml = true);

	inline QString type() const { return m_type; }
	void setType(const QString& new_value, const bool bFromQml = true);

	QString fileName() const;
	inline QString file() const { return m_file; }
	void setFile(const QString& new_value, const bool bFromQml = true);

	inline QString startDate() const { return m_startDate; }
	inline QDate minimumMesoStartDate() const { return m_minimumMesoStartDate; }
	void setMinimumMesoStartDate(const QDate& new_value, const bool bFromQml = true);

	inline QString endDate() const { return m_endDate; }
	inline QDate maximumMesoEndDate() const { return m_maximumMesoEndDate; }
	void setMaximumMesoEndDate(const QDate& new_value, const bool bFromQml = true);

	inline QString weeks() const { return m_weeks; }
	void setWeeks(const QString& new_value, const bool bFromQml = true);

	inline QString split() const { return m_split; }
	void setSplit(const QString& new_value, const bool bFromQml = true);

	inline QString notes() const { return m_notes; }
	void setNotes(const QString& new_value, const bool bFromQml = true);

	inline QString muscularGroupA() const { return m_muscularGroup.at(0); }
	void setMuscularGroupA(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupB() const { return m_muscularGroup.at(1); }
	void setMuscularGroupB(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupC() const { return m_muscularGroup.at(2); }
	void setMuscularGroupC(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupD() const { return m_muscularGroup.at(3); }
	void setMuscularGroupD(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupE() const { return m_muscularGroup.at(4); }
	void setMuscularGroupE(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupF() const { return m_muscularGroup.at(5); }
	void setMuscularGroupF(const QString& new_value, const bool bFromQml = true);
	inline QString muscularGroupR() const { return m_muscularGroup.at(6); }

	inline QDate calendarStartDate() const { return m_calendarStartDate; }
	void setCalendarStartDate(const QDate& new_value);
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday);
	Q_INVOKABLE void getCalendarPage();
	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getTrainingDayPage(const QDate& date);

	void getMesocyclePage();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	void importMeso(const QString& filename = QString());

signals:
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void ownerIsCoachChanged();
	void hasCoachChanged();
	void realMesoChanged();
	void ownMesoChanged();
	void isNewMesoChanged();
	void nameChanged();
	void coachChanged();
	void clientChanged();
	void typeChanged();
	void fileNameChanged();
	void fileChanged();
	void startDateChanged();
	void endDateChanged();
	void weeksChanged();
	void splitChanged();
	void notesChanged();
	void muscularGroupAChanged();
	void muscularGroupBChanged();
	void muscularGroupCChanged();
	void muscularGroupDChanged();
	void muscularGroupEChanged();
	void muscularGroupFChanged();
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
	QString m_name, m_coach, m_client, m_type, m_file, m_startDate, m_endDate, m_weeks, m_split, m_notes;
	QDate m_minimumMesoStartDate, m_maximumMesoEndDate, m_calendarStartDate;
	QStringList m_muscularGroup;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	QMap<QDate,QmlTDayInterface*> m_tDayPages;
	QmlMesoSplitInterface* m_exercisesPage;
	QmlMesoCalendarInterface* m_calendarPage;

	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();
	void setPropertiesBasedOnUseMode();
	void updateMuscularGroupFromOutside(const uint splitIndex);
};

#endif // QMLMESOINTERFACE_H
