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
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)
Q_PROPERTY(bool dayIsEditable READ dayIsEditable WRITE setDayIsEditable NOTIFY dayIsEditableChanged FINAL)

public:
	explicit QmlTDayInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx, const QDate& date);
	~QmlTDayInterface();

	Q_INVOKABLE void getTrainingDayPage();
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan();
	Q_INVOKABLE void convertTDayToPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay);
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	Q_INVOKABLE void exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void importTrainingDay(const QString& filename = QString());

	void createExerciseObject();
	void removeExerciseObject(const uint exercise_idx);

	inline bool dayIsEditable() const { return m_bDayIsEditable; }
	inline void setDayIsEditable(const bool editable);
	inline bool dayIsFinished() const { return m_bDayIsFinished; }
	void setDayIsFinished(const bool bFinished);

	inline DBTrainingDayModel* tDayModel() { return m_tDayModel; }
	void displayMessage(const QString& title, const QString& message, const bool error = false, const uint msecs = 0) const;
	void askRemoveExercise(const uint exercise_idx) const;
	void askRemoveSet(const uint exercise_idx, const uint set_number) const;
	TPTimer* getTimer();
	void gotoNextExercise(const uint exercise_idx);
	void rollUpExercises() const;

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);
	void dayIsFinishedChanged();
	void dayIsEditableChanged();

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
	TPTimer* m_timer;
	bool m_bDayIsFinished, m_bDayIsEditable;

	void createTrainingDayPage();
	void createTrainingDayPage_part2();
	void updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void makeTDayPageHeaderLabel();
	void setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const model);
};

#endif // QMLTDAYINTERFACE_H
