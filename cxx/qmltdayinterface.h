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

public:
	explicit QmlTDayInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx, const QDate& date);
	~QmlTDayInterface();

	Q_INVOKABLE void getTrainingDayPage();
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan();
	Q_INVOKABLE void convertTDayToPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void setDayIsFinished(const bool bFinished);
	Q_INVOKABLE void adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay);
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	Q_INVOKABLE void exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void importTrainingDay(const QString& filename = QString());

	inline DBTrainingDayModel* tDayModel() { return m_tDayModel; }
	void displayMessage(const QString& title, const QString& message, const bool error = false, const uint msecs = 0);
	TPTimer* getTimer();

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

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

	void createTrainingDayPage();
	void createTrainingDayPage_part2();
	void updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void makeTDayPageHeaderLabel();
	void setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const model);
	void rollUpExercises() const;
};

#endif // QMLTDAYINTERFACE_H
