#pragma once

#include <QDate>
#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBCalendarModel)
QT_FORWARD_DECLARE_CLASS(QmlExerciseEntry)
QT_FORWARD_DECLARE_CLASS(QmlSetEntry)
QT_FORWARD_DECLARE_CLASS(TPTimer)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

Q_DECLARE_OPAQUE_POINTER(QmlExerciseEntry*)

#ifndef QMLSETENTRY_H
Q_DECLARE_OPAQUE_POINTER(QmlSetEntry*)
#endif

class QmlWorkoutInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QmlExerciseEntry *workingExercise READ workingExercise WRITE setWorkingExercise NOTIFY workingExerciseChanged FINAL)
Q_PROPERTY(QmlSetEntry *workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged FINAL)
Q_PROPERTY(uint timerHour READ timerHour WRITE setTimerHour NOTIFY timerHourChanged FINAL)
Q_PROPERTY(uint timerMinute READ timerMinute WRITE setTimerMinute NOTIFY timerMinuteChanged FINAL)
Q_PROPERTY(uint timerSecond READ timerSecond WRITE setTimerSecond NOTIFY timerSecondChanged FINAL)
Q_PROPERTY(QChar splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged FINAL)
Q_PROPERTY(QString timeIn READ timeIn WRITE setTimeIn NOTIFY timeInChanged FINAL)
Q_PROPERTY(QString timeOut READ timeOut WRITE setTimeOut NOTIFY timeOutChanged FINAL)
Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)
Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged FINAL)
Q_PROPERTY(QString muscularGroup READ muscularGroup NOTIFY muscularGroupChanged FINAL)
Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged FINAL)
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)
Q_PROPERTY(bool workoutIsEditable READ workoutIsEditable WRITE setWorkoutIsEditable NOTIFY workoutIsEditableChanged FINAL)
Q_PROPERTY(bool canImportFromPreviousWorkout READ canImportFromPreviousWorkout WRITE setCanImportFromPreviousWorkout NOTIFY canImportFromPreviousWorkoutChanged FINAL)
Q_PROPERTY(bool canImportFromSplitPlan READ canImportFromSplitPlan WRITE setCanImportFromSplitPlan NOTIFY canImportFromSplitPlanChanged FINAL)
Q_PROPERTY(bool mainDateIsToday READ mainDateIsToday WRITE setMainDateIsToday NOTIFY mainDateIsTodayChanged FINAL)
Q_PROPERTY(bool needActivation READ needActivation WRITE setNeedActivation NOTIFY needActivationChanged FINAL)
Q_PROPERTY(bool timerActive READ timerActive WRITE setTimerActive NOTIFY timerActiveChanged FINAL)
Q_PROPERTY(bool hasExercises READ hasExercises NOTIFY hasExercisesChanged FINAL)
Q_PROPERTY(QStringList previousWorkoutsList READ previousWorkoutsList NOTIFY previousWorkoutsListChanged FINAL)

public:
	explicit QmlWorkoutInterface(QObject *parent, const uint meso_idx, const QDate &date);
	inline ~QmlWorkoutInterface() { cleanUp(); }
	void cleanUp();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	inline QmlExerciseEntry *workingExercise() const { return m_workingExercise; }
	void setWorkingExercise(QmlExerciseEntry *new_workingexercise);
	inline QmlSetEntry *workingSet() const { return m_workingSet; }
	void setWorkingSet(QmlSetEntry *new_workingset);

	inline uint timerHour() const { return m_hour; }
	inline void setTimerHour(const uint new_value) { m_hour = new_value; emit timerHourChanged(); }

	inline uint timerMinute() const { return m_min; }
	inline void setTimerMinute(const uint new_value) { m_min = new_value; emit timerMinuteChanged(); }

	inline uint timerSecond() const { return m_sec; }
	inline void setTimerSecond(const uint new_value) { m_sec = new_value; emit timerSecondChanged(); }

	QChar splitLetter() const;
	void setSplitLetter(const QChar &new_splitletter, const bool clear_exercises = false);

	QString timeIn() const;
	void setTimeIn(const QString &new_timein);

	QString timeOut() const;
	void setTimeOut(const QString &new_timeout);

	QString location() const;
	void setLocation(const QString &new_location);
	Q_INVOKABLE QString lastWorkOutLocation() const;

	QString notes() const;
	void setNotes(const QString &new_notes);

	bool dayIsFinished() const;
	void setDayIsFinished(const bool finished);

	inline QString headerText() const { return m_headerText; }
	void setHeaderText(const QString &new_header = QString{});
	Q_INVOKABLE QString muscularGroup() const;

	inline bool editMode() const { return m_editMode; }
	void setEditMode(const bool edit_mode);

	inline bool workoutIsEditable() const { return m_workoutIsEditable; }
	void setWorkoutIsEditable(const bool editable);

	inline bool canImportFromSplitPlan() const { return m_importFromSplitPlan; }
	inline void setCanImportFromSplitPlan(const bool can_import) { m_importFromSplitPlan = can_import; emit canImportFromSplitPlanChanged(); }

	inline bool canImportFromPreviousWorkout() const { return m_importFromPrevWorkout; }
	inline void setCanImportFromPreviousWorkout(const bool can_import) { m_importFromPrevWorkout = can_import; emit canImportFromPreviousWorkoutChanged(); }

	inline bool mainDateIsToday() const { return m_bMainDateIsToday; }
	void setMainDateIsToday(const bool is_today);

	inline bool needActivation() const { return m_bNeedActivation; }
	inline void setNeedActivation(const bool new_value) { if (m_bNeedActivation != new_value) { m_bNeedActivation = new_value; emit needActivationChanged(); } }

	inline bool timerActive() const { return m_bTimerActive; }
	inline void setTimerActive(const bool new_value) { m_bTimerActive = new_value; emit timerActiveChanged(); }

	bool hasExercises() const;

	inline QStringList previousWorkoutsList() const { return m_prevWorkouts; }
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void setMesoIdx(const uint new_meso_idx);
	void getWorkoutPage();

	Q_INVOKABLE void loadExercisesFromDate(const QString &strDate);
	Q_INVOKABLE void getExercisesFromSplitPlan();
	Q_INVOKABLE void exportWorkoutToSplitPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void exportWorkout(const bool bShare);
	Q_INVOKABLE void importWorkout(const QString &filename = QString());
	Q_INVOKABLE void prepareWorkOutTimer(const QString &strStartTime = QString(), const QString &strEndTime = QString());
	Q_INVOKABLE void startWorkout();
	Q_INVOKABLE void stopWorkout();
	Q_INVOKABLE void addExercise(const bool show_exercises_list_page = true);
	Q_INVOKABLE void clearExercises(const bool bShowIntentDialog = true);
	Q_INVOKABLE void removeExercise(const uint exercise_number);

	void removeExerciseObject(const uint exercise_idx, const bool bAsk);
	void moveExercise(const uint exercise_number, const uint new_exercisenumber);

	inline DBExercisesModel *workoutModel() const { return m_workoutModel; }
	inline QQuickItem *workoutPage() const { return m_workoutPage; }

	void simpleExercisesList(const bool show, const bool multi_sel = false);
	void displayMessage(const QString &title, const QString &message, const bool error = false, const uint msecs = 0) const;
	void askRemoveExercise(const uint exercise_number);
	void gotoNextExercise(const uint exercise_number);
	void rollUpExercise(const uint exercise_number) const;
	void rollUpExercises() const;

	TPTimer *restTimer();

public slots:
	void newExerciseFromExercisesList();
	void changeExerciseFromExercisesList(int exercise_number = -1);
	void silenceTimeWarning();
	void hideSimpleExercisesList();
	void verifyWorkoutOptions();

signals:
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void workingExerciseChanged();
	void workingSetChanged();
	void timerHourChanged();
	void timerMinuteChanged();
	void timerSecondChanged();
	void splitLetterChanged();
	void timeInChanged();
	void timeOutChanged();
	void locationChanged();
	void lastWorkOutLocationChanged();
	void notesChanged();
	void headerTextChanged();
	void muscularGroupChanged();
	void editModeChanged();
	void dayIsFinishedChanged();
	void workoutIsEditableChanged();
	void canImportFromPreviousWorkoutChanged();
	void canImportFromSplitPlanChanged();
	void mainDateIsTodayChanged();
	void needActivationChanged();
	void timerActiveChanged();
	void hasExercisesChanged();
	void previousWorkoutsListChanged();
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void displayMessageOnAppWindow(const int message_id, const QString &filename = QString{});
	void requestMesoSplitModel(const QChar &splitletter);

private:
	QQmlComponent *m_workoutComponent, *m_exercisesComponent;
	DBExercisesModel *m_workoutModel;
	DBCalendarModel  *m_calendarModel;
	QQuickItem *m_workoutPage, *m_exercisesLayout;
	QVariantMap m_workoutProperties, m_exercisesProperties;
	QList<QmlExerciseEntry*> m_exercisesList;
	uint m_mesoIdx, m_calendarDay;
	TPTimer *m_workoutTimer, *m_restTimer;
	int m_nExercisesToCreate;
	QStringList m_prevWorkouts;

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	QmlExerciseEntry *m_workingExercise;
	QmlSetEntry *m_workingSet;
	uint m_hour, m_min, m_sec;
	QString m_headerText;
	bool m_editMode, m_workoutIsEditable, m_importFromPrevWorkout, m_importFromSplitPlan, m_bMainDateIsToday, m_bNeedActivation,
			m_bTimerActive;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void createWorkoutPage();
	void createWorkoutPage_part2();
	void createExerciseObject(const uint exercise_number);
	void createExerciseObject_part2(const uint exercise_number);
	void loadExercises();
	void calculateWorkoutTime();
};
