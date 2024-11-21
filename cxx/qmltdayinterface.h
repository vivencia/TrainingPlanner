#ifndef QMLTDAYINTERFACE_H
#define QMLTDAYINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class DBTrainingDayModel;
class DBMesoSplitModel;
class QmlExerciseInterface;
class QmlExerciseEntry;
class TPTimer;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlTDayInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(uint timerHour READ timerHour WRITE setTimerHour NOTIFY timerHourChanged FINAL)
Q_PROPERTY(uint timerMinute READ timerMinute WRITE setTimerMinute NOTIFY timerMinuteChanged FINAL)
Q_PROPERTY(uint timerSecond READ timerSecond WRITE setTimerSecond NOTIFY timerSecondChanged FINAL)
Q_PROPERTY(QString splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged FINAL)
Q_PROPERTY(QString timeIn READ timeIn WRITE setTimeIn NOTIFY timeInChanged FINAL)
Q_PROPERTY(QString timeOut READ timeOut WRITE setTimeOut NOTIFY timeOutChanged FINAL)
Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged FINAL)
Q_PROPERTY(QString lastWorkOutLocation READ lastWorkOutLocation WRITE setLastWorkOutLocation NOTIFY lastWorkOutLocationChanged FINAL)
Q_PROPERTY(QString dayNotes READ dayNotes WRITE setDayNotes NOTIFY dayNotesChanged FINAL)
Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged FINAL)
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)
Q_PROPERTY(bool dayIsEditable READ dayIsEditable WRITE setDayIsEditable NOTIFY dayIsEditableChanged FINAL)
Q_PROPERTY(bool hasPreviousTDays READ hasPreviousTDays WRITE setHasPreviousTDays NOTIFY hasPreviousTDaysChanged FINAL)
Q_PROPERTY(bool hasMesoPlan READ hasMesoPlan WRITE setHasMesoPlan NOTIFY hasMesoPlanChanged FINAL)
Q_PROPERTY(bool mainDateIsToday READ mainDateIsToday WRITE setMainDateIsToday NOTIFY mainDateIsTodayChanged FINAL)
Q_PROPERTY(bool needActivation READ needActivation WRITE setNeedActivation NOTIFY needActivationChanged FINAL)
Q_PROPERTY(bool timerActive READ timerActive WRITE setTimerActive NOTIFY timerActiveChanged FINAL)
Q_PROPERTY(bool hasExercises READ hasExercises WRITE setHasExercises NOTIFY hasExercisesChanged FINAL)
Q_PROPERTY(QStringList previousTDays READ previousTDays WRITE setPreviousTDays NOTIFY previousTDaysChanged FINAL)

public:
	explicit inline QmlTDayInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx, const QDate& date)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_tDayPage(nullptr), m_mesoIdx(meso_idx), m_Date(date),
			m_exerciseManager(nullptr), m_workoutTimer(nullptr), m_restTimer(nullptr), m_SimpleExercisesListRequesterExerciseComp(0) {}
	~QmlTDayInterface();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	inline uint timerHour() const { return m_hour; }
	inline void setTimerHour(const uint new_value) { m_hour = new_value; emit timerHourChanged(); }

	inline uint timerMinute() const { return m_min; }
	inline void setTimerMinute(const uint new_value) { m_min = new_value; emit timerMinuteChanged(); }

	inline uint timerSecond() const { return m_sec; }
	inline void setTimerSecond(const uint new_value) { m_sec = new_value; emit timerSecondChanged(); }

	inline const QChar _splitLetter() const { return m_splitLetter.at(0); }
	inline QString splitLetter() const { return m_splitLetter; }
	void setSplitLetter(const QString& new_value, const bool bFromQml = true, const bool bDontConfirm = false);

	inline QString timeIn() const { return m_timeIn; }
	void setTimeIn(const QString& new_value);

	inline QString timeOut() const { return m_timeOut; }
	void setTimeOut(const QString& new_value);

	inline QString headerText() const { return m_headerText; }
	void setHeaderText(const QString& = QString());

	inline QString lastWorkOutLocation() const { return m_lastWorkOutLocation; }
	void setLastWorkOutLocation(const QString& new_value);

	inline QString dayNotes() const { return m_dayNotes; }
	void setDayNotes(const QString& new_value);

	inline bool editMode() const { return m_bEditMode; }
	void setEditMode(const bool new_value);

	inline bool dayIsFinished() const { return m_bDayIsFinished; }
	void setDayIsFinished(const bool new_value);

	inline bool dayIsEditable() const { return m_bDayIsEditable; }
	void setDayIsEditable(const bool new_value);

	inline bool hasMesoPlan() const { return m_bHasMesoPlan; }
	inline void setHasMesoPlan(const bool new_value) { if (m_bHasMesoPlan != new_value) { m_bHasMesoPlan = new_value; emit hasMesoPlanChanged(); } }

	inline bool hasPreviousTDays() const { return m_bHasPreviousTDays; }
	inline void setHasPreviousTDays(const bool new_value) { if (m_bHasPreviousTDays != new_value) { m_bHasPreviousTDays = new_value; emit hasPreviousTDaysChanged(); } }

	inline bool mainDateIsToday() const { return m_bMainDateIsToday; }
	void setMainDateIsToday(const bool new_value);

	inline bool needActivation() const { return m_bNeedActivation; }
	inline void setNeedActivation(const bool new_value) { if (m_bNeedActivation != new_value) { m_bNeedActivation = new_value; emit needActivationChanged(); } }

	inline bool timerActive() const { return m_bTimerActive; }
	inline void setTimerActive(const bool new_value) { m_bTimerActive = new_value; emit timerActiveChanged(); }

	inline bool hasExercises() const { return m_bHasExercises; }
	inline void setHasExercises(const bool new_value) { if (m_bHasExercises != new_value) { m_bHasExercises = new_value; emit hasExercisesChanged(); } }

	inline QStringList previousTDays() const { return m_previousTDays; }
	inline void setPreviousTDays(const QStringList& other) { m_previousTDays = other; emit previousTDaysChanged(); }
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void setMesoIdx(const uint new_meso_idx);
	void getTrainingDayPage();

	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan(DBMesoSplitModel* const splitModel = nullptr);
	Q_INVOKABLE void convertTDayToPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void changeSplit(const QString& newSplitLetter, const bool bClearExercises = false);
	Q_INVOKABLE void adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay);
	Q_INVOKABLE void exportTrainingDay(const bool bShare);
	Q_INVOKABLE void importTrainingDay(const QString& filename = QString());
	Q_INVOKABLE void prepareWorkOutTimer(const QString& strStartTime = QString(), const QString& strEndTime = QString());
	Q_INVOKABLE void startWorkout();
	Q_INVOKABLE void stopWorkout();
	Q_INVOKABLE void removeExercise(const uint exercise_idx);
	Q_INVOKABLE void removeSetFromExercise(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void createExerciseObject();
	void removeExerciseObject(const uint exercise_idx, const bool bAsk);

	inline DBTrainingDayModel* tDayModel() const { return m_tDayModel; }
	inline QQuickItem* tDayPage() const { return m_tDayPage; }

	void simpleExercisesList(const uint exercise_idx, const bool show, const bool multi_sel, const uint comp_exercise);
	void displayMessage(const QString& title, const QString& message, const bool error = false, const uint msecs = 0) const;
	void askRemoveExercise(const uint exercise_idx);
	void askRemoveSet(const uint exercise_idx, const uint set_number);
	void gotoNextExercise(const uint exercise_idx);
	void rollUpExercises() const;

	TPTimer* restTimer();

signals:
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void timerHourChanged();
	void timerMinuteChanged();
	void timerSecondChanged();
	void splitLetterChanged();
	void timeInChanged();
	void timeOutChanged();
	void headerTextChanged();
	void lastWorkOutLocationChanged();
	void dayNotesChanged();
	void editModeChanged();
	void dayIsFinishedChanged();
	void dayIsEditableChanged();
	void hasPreviousTDaysChanged();
	void hasMesoPlanChanged();
	void mainDateIsTodayChanged();
	void needActivationChanged();
	void timerActiveChanged();
	void hasExercisesChanged();
	void previousTDaysChanged();
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);
	void requestMesoSplitModel(const QChar& splitletter);
	void convertTDayToSplitPlan(const DBTrainingDayModel* const tDayModel);

public slots:
	void silenceTimeWarning();
	void exerciseSelected(QmlExerciseEntry* exerciseEntry = nullptr);
	void hideSimpleExercisesList();

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_tDayComponent;
	DBTrainingDayModel* m_tDayModel;
	QQuickItem* m_tDayPage;
	QVariantMap m_tDayProperties;
	QmlExerciseInterface* m_exerciseManager;
	uint m_mesoIdx;
	QDate m_Date;
	TPTimer* m_workoutTimer, *m_restTimer;
	int m_SimpleExercisesListRequesterExerciseIdx, m_SimpleExercisesListRequesterExerciseComp;

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	uint m_hour, m_min, m_sec;
	QString m_splitLetter, m_timeIn, m_timeOut, m_headerText, m_lastWorkOutLocation, m_dayNotes;
	bool m_bEditMode, m_bDayIsFinished, m_bDayIsEditable, m_bHasPreviousTDays, m_bHasMesoPlan, m_bMainDateIsToday, m_bNeedActivation,
			m_bTimerActive, m_bHasExercises;
	QStringList m_previousTDays;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void createTrainingDayPage();
	void createTrainingDayPage_part2();
	void loadExercises();
	void updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void calculateWorkoutTime();
	void setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const model);
};

#endif // QMLTDAYINTERFACE_H
