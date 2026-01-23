#pragma once

#include <QDate>
#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBCalendarModel)
QT_FORWARD_DECLARE_CLASS(TPTimer)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

using DBWorkoutModel = DBExercisesModel;

class QmlWorkoutInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(uint timerHour READ timerHour WRITE setTimerHour NOTIFY timerHourChanged FINAL)
Q_PROPERTY(uint timerMinute READ timerMinute WRITE setTimerMinute NOTIFY timerMinuteChanged FINAL)
Q_PROPERTY(uint timerSecond READ timerSecond WRITE setTimerSecond NOTIFY timerSecondChanged FINAL)
Q_PROPERTY(QString headerText READ headerText NOTIFY headerTextChanged FINAL)
Q_PROPERTY(QString headerText_2 READ headerText_2 NOTIFY headerTextChanged FINAL)
Q_PROPERTY(QString sessionLabel READ sessionLabel NOTIFY haveNewWorkoutOptionsChanged FINAL)
Q_PROPERTY(bool haveExercises READ haveExercises NOTIFY haveExercisesChanged FINAL)
Q_PROPERTY(bool canImportFromSplitPlan READ canImportFromSplitPlan WRITE setCanImportFromSplitPlan NOTIFY haveNewWorkoutOptionsChanged FINAL)
Q_PROPERTY(bool canImportFromPreviousWorkout READ canImportFromPreviousWorkout WRITE setCanImportFromPreviousWorkout NOTIFY haveNewWorkoutOptionsChanged FINAL)
Q_PROPERTY(bool workoutInProgress READ workoutInProgress WRITE setWorkoutInProgress NOTIFY workoutStatusChanged FINAL)
Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY workoutStatusChanged FINAL)
Q_PROPERTY(bool workoutFinished READ workoutFinished WRITE setWorkoutFinished NOTIFY workoutStatusChanged FINAL)
Q_PROPERTY(bool workoutIsEditable READ workoutIsEditable WRITE setWorkoutIsEditable NOTIFY workoutStatusChanged FINAL)
Q_PROPERTY(bool todaysWorkout READ todaysWorkout WRITE setTodaysWorkout NOTIFY workoutStatusChanged FINAL)

public:
	explicit QmlWorkoutInterface(QObject *parent, DBMesocyclesModel *meso_model, const uint meso_idx, const QDate &date);
	inline ~QmlWorkoutInterface() { cleanUp(); }
	void cleanUp();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	inline uint timerHour() const { return m_hour; }
	inline void setTimerHour(const uint new_value) { m_hour = new_value; emit timerHourChanged(); }

	inline uint timerMinute() const { return m_min; }
	inline void setTimerMinute(const uint new_value) { m_min = new_value; emit timerMinuteChanged(); }

	inline uint timerSecond() const { return m_sec; }
	inline void setTimerSecond(const uint new_value) { m_sec = new_value; emit timerSecondChanged(); }

	Q_INVOKABLE void changeSplitLetter(const QString &new_splitletter);	

	Q_INVOKABLE QString timeIn() const;
	Q_INVOKABLE void setTimeIn(const QString &new_timein);

	Q_INVOKABLE QString timeOut() const;
	Q_INVOKABLE void setTimeOut(const QString &new_timeout);

	Q_INVOKABLE QString location();
	Q_INVOKABLE void setLocation(const QString &new_location);

	Q_INVOKABLE QString notes() const;
	Q_INVOKABLE void setNotes(const QString &new_notes);

	QString headerText() const { return m_headerText; }
	QString headerText_2() const { return m_headerText_2; }
	void setHeaderText();
	QString sessionLabel() const;

	bool haveExercises() const;

	Q_INVOKABLE inline bool haveNewWorkoutOptions() const { return canImportFromSplitPlan() || canImportFromPreviousWorkout(); }
	inline bool canImportFromSplitPlan() const { return m_importFromSplitPlan; }
	inline void setCanImportFromSplitPlan(const bool can_import) { m_importFromSplitPlan = can_import; emit haveNewWorkoutOptionsChanged(); }
	inline bool canImportFromPreviousWorkout() const { return m_importFromPrevWorkout; }
	inline void setCanImportFromPreviousWorkout(const bool can_import) { m_importFromPrevWorkout = can_import; emit haveNewWorkoutOptionsChanged(); }

	bool workoutFinished() const;
	void setWorkoutFinished(const bool finished);
	bool workoutInProgress() const;
	void setWorkoutInProgress(const bool in_progress);
	bool editMode() const;
	void setEditMode(const bool edit_mode);
	bool workoutIsEditable() const;
	void setWorkoutIsEditable(const bool editable);
	bool todaysWorkout() const;
	void setTodaysWorkout(const bool is_today);

	Q_INVOKABLE void setWorkingSetMode();
	Q_INVOKABLE QStringList previousWorkoutsList_text() const;
	Q_INVOKABLE QList<uint> previousWorkoutsList_value() const;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	void setMesoIdx(const uint new_meso_idx);
	void getWorkoutPage();

	Q_INVOKABLE void loadExercisesFromCalendarDay(const uint calendar_day);
	Q_INVOKABLE void getExercisesFromSplitPlan();
	Q_INVOKABLE void exportWorkoutToSplitPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void exportWorkout(const bool bShare);
	Q_INVOKABLE void importWorkout(const QString &filename = QString());
	Q_INVOKABLE void prepareWorkOutTimer(const QString &strStartTime = QString(), const QString &strEndTime = QString());
	Q_INVOKABLE void startWorkout();
	Q_INVOKABLE void stopWorkout();
	Q_INVOKABLE void addExercise();
	Q_INVOKABLE void simpleExercisesList(const bool show);
	Q_INVOKABLE void clearExercises(const bool bShowIntentDialog = true);
	Q_INVOKABLE void removeExercise(const int exercise_number = -1);
	Q_INVOKABLE bool canChangeSetMode(const uint exercise_number, const uint exercise_idx, const uint set_number) const;

	inline DBWorkoutModel *workoutModel() const { return m_workoutModel; }
	inline QQuickItem *workoutPage() const { return m_workoutPage; }

	void askRemoveExercise(const uint exercise_number);
	void gotoNextExercise();
	void rollUpExercise(const uint exercise_number) const;
	void rollUpExercises() const;

public slots:
	void silenceTimeWarning();

signals:
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void haveNewWorkoutOptionsChanged();
	void timerHourChanged();
	void timerMinuteChanged();
	void timerSecondChanged();
	void timeInChanged();
	void timeOutChanged();
	void headerTextChanged();
	void muscularGroupChanged();
	void workoutStatusChanged();
	void editModeChanged();
	void haveExercisesChanged();
	void updateRestTime(const int exercise_number, const QString &rest_time);

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	void displayMessageOnAppWindow(const int message_id, const QString &filename = QString{});
	void requestMesoSplitModel(const QChar &splitletter);

private:
	DBMesocyclesModel *m_mesoModel;
	DBWorkoutModel *m_workoutModel;
	DBCalendarModel *m_calendarModel;
	QQmlComponent *m_workoutComponent, *m_exercisesComponent;
	QVariantMap m_workoutProperties, m_exercisesProperties;
	QQuickItem *m_workoutPage, *m_exercisesItem;
	uint m_mesoIdx, m_calendarDay;
	TPTimer *m_workoutTimer, *m_restTimer;
	QTime m_lastSetCompleted;
	int m_nExercisesToCreate;
	QHash<uint,QString> m_prevWorkouts;
	QDate m_date;
	uint8_t m_workoutStatus;
	uint m_hour, m_min, m_sec;
	bool m_importFromSplitPlan, m_importFromPrevWorkout;
	QString m_headerText, m_headerText_2;

	bool checkWorkoutStatus(uint8_t flag) const;
	bool changeWorkoutStatus(uint8_t flag, const bool set, const bool emit_signal = true);
	void createWorkoutPage();
	void createWorkoutPage_part2();
	void calculateWorkoutTime();
	void startRestTimer(const uint exercise_number, const QString &rest_time);
	void verifyWorkoutOptions();
	QString workoutCompletedMessage(const bool completed) const;
};
