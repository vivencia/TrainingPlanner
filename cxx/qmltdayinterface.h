#ifndef QMLTDAYINTERFACE_H
#define QMLTDAYINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class DBTrainingDayModel;
class QmlExerciseInterface;

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
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE void clearExercises();
	Q_INVOKABLE void moveExercise(const uint exercise_idx, const uint new_idx);

	DBTrainingDayModel* gettDayModel(const QDate& date);
	inline DBTrainingDayModel* tDayModel() { return m_tDayModel; }

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
	QList<QmlExerciseInterface*> m_exerciseObjects;
	uint m_mesoIdx;
	QDate m_Date;

	/*struct tDayExercises {
		struct exerciseObject {
			QQuickItem* m_exerciseEntry;
			QList<QQuickItem*> m_setObjects;
			QString nSets, nReps, nWeight, restTime;
			uint newSetType;
			TPTimer* m_setTimer;

			exerciseObject() : m_setTimer(nullptr), newSetType(SET_TYPE_REGULAR) {}
		};
		QList<exerciseObject*> exerciseObjects;

		inline TPTimer* setTimer(const uint exercise_idx)
		{
			if (!exerciseObjects.at(exercise_idx)->m_setTimer)
				exerciseObjects[exercise_idx]->m_setTimer = new TPTimer();
			return exerciseObjects.at(exercise_idx)->m_setTimer;
		}

		friend class QmlItemManager;
	};*/

	void createTrainingDayPage();
	void createTrainingDayPage_part2();
	void updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void makeTDayPageHeaderLabel();
	void setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const model);
	void rollUpExercises() const;
};

#endif // QMLTDAYINTERFACE_H
