#ifndef QMLTDAYINTERFACE_H
#define QMLTDAYINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class DBTrainingDayModel;
class QmlExerciseInterface;
class TPTimer;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlTDayInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QString timeIn READ timeIn WRITE setTimeInFinished NOTIFY timeInChanged FINAL)
Q_PROPERTY(QString timeOut READ timeOut WRITE setTimeOutFinished NOTIFY timeOutChanged FINAL)
Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged FINAL)
Q_PROPERTY(QString lastWorkOutLocation READ lastWorkOutLocation WRITE setLastWorkOutLocation NOTIFY lastWorkOutLocationChanged FINAL)
Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged FINAL)
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)
Q_PROPERTY(bool dayIsEditable READ dayIsEditable WRITE setDayIsEditable NOTIFY dayIsEditableChanged FINAL)
Q_PROPERTY(bool hasPreviousTDays READ hasPreviousTDays WRITE setHasPreviousTDays NOTIFY hasPreviousTDaysChanged FINAL)
Q_PROPERTY(bool hasMesoPlan READ hasMesoPlan WRITE setHasMesoPlan NOTIFY hasMesoPlanChanged FINAL)
Q_PROPERTY(bool mainDateIsToday READ mainDateIsToday WRITE setMainDateIsToday NOTIFY mainDateIsToday FINAL)
Q_PROPERTY(bool needActivation READ needActivation WRITE setNeedActivation NOTIFY needActivationChanged FINAL)
Q_PROPERTY(QStringList previousTDays READ previousTDays WRITE setPreviousTDays NOTIFY previousTDaysChanged FINAL)

public:
	explicit QmlTDayInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx, const QDate& date);
	~QmlTDayInterface();

	Q_INVOKABLE void getTrainingDayPage();
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan();
	Q_INVOKABLE void convertTDayToPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay);
	Q_INVOKABLE void exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void importTrainingDay(const QString& filename = QString());
	Q_INVOKABLE void prepareWorkOutTimer(const QString& strStartTime = QString(), const QString& strEndTime = QString());

	void createExerciseObject();
	void removeExerciseObject(const uint exercise_idx, const bool bAsk);

	inline QString timeIn() const { return m_timeIn; }
	void setTimeIn(const QString& new_value);

	inline QString timeOut() const { return m_timeOut; }
	void setTimeOut(const QString& new_value);

	inline QString headerText() const { return m_headerText; }
	void setHeaderText(const QString& new_value);

	inline QString lastWorkoutLocation() const { return m_lastWorkOutLocation; }
	void setLastWorkoutLocation(const QString& new_value);

	inline bool editMode() const { return m_bEditMode; }
	inline void setEditMod(const bool new_value) { if (m_bEditMode != new_value) { m_bEditMode = new_value; emit editModeChanged(); } }

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

	inline DBTrainingDayModel* tDayModel() const { return m_tDayModel; }
	void displayMessage(const QString& title, const QString& message, const bool error = false, const uint msecs = 0) const;
	void askRemoveExercise(const uint exercise_idx) const;
	void askRemoveSet(const uint exercise_idx, const uint set_number) const;
	TPTimer* getTimer();
	void gotoNextExercise(const uint exercise_idx);
	void rollUpExercises() const;
	void showSimpleExercisesList(const uint exercise_idx, const bool bMultiSel);
	void hideSimpleExercisesList();

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);
	void timeInChanged();
	void timeOutChanged();
	void headerTextChanged();
	void lastWorkOutLocationChanged();
	void editModeChanged();
	void dayIsFinishedChanged();
	void dayIsEditableChanged();
	void hasPreviousTDaysChanged();
	void hasMesoPlanChanged();
	void mainDateIsTodayChanged();
	void needActivationChanged();
	void previousTDaysChanged();

public slots:
	void removeExercise(const uint exercise_idx);
	void removeSetFromExercise(const uint exercise_idx, const uint set_number);

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
	TPTimer* m_workoutTimer;

	QString m_timeIn, m_timeOut, m_headerText, m_lastWorkOutLocation;
	bool m_bEditMode, m_bDayIsFinished, m_bDayIsEditable, m_bHasPreviousTDays, m_bHasMesoPlan, m_bMainDateIsToday, m_bNeedActivation;
	QStringList m_previousTDays;

	void createTrainingDayPage();
	void createTrainingDayPage_part2();
	void updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void makeTDayPageHeaderLabel();
	void setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const model);
	void calculateWorkoutTime();
};

#endif // QMLTDAYINTERFACE_H
