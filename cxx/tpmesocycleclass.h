#ifndef TPMESOCYCLECLASS_H
#define TPMESOCYCLECLASS_H

#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "tptimer.h"

#include <QObject>
#include <QMap>
#include <QDate>

class DBMesocyclesModel;
class DBExercisesModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

static const uint mesoPageCreateId(175);
static const uint calPageCreateId(35);
static const uint tDayPageCreateId(70);
static const uint tDayExerciseCreateId(105);
static const uint tDaySetCreateId(140);
static const uint menuShortCutCreatedId(200);
static const uint exercisesPlannerCreateId(235);

class TPMesocycleClass : public QObject
{

Q_OBJECT

public:
	TPMesocycleClass(const int meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent = nullptr);
	~TPMesocycleClass();

	inline int mesoId() const { return m_mesoId; }
	void setMesoId(const int new_mesoid);
	inline uint mesoIdx() const { return m_mesoIdx; }
	void changeMesoIdxFromPagesAndModels(const uint new_mesoidx);

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();

	inline void setMesocycleModel(DBMesocyclesModel* model) { m_MesocyclesModel = model; }
	inline QQuickItem* getMesoPage() { return m_mesoPage; }
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	Q_INVOKABLE void createPlannerPage();
	void createPlannerPage_part2();
	inline QQuickItem* getExercisesPlannerPage() const { return m_plannerPage; }

	void createMesoSplitPage();
	void createMesoSplitPage_part2();

	inline DBMesoSplitModel* getSplitModel(const QChar& splitLetter)
	{
		if (!m_splitModels.contains(splitLetter))
			m_splitModels.insert(splitLetter, new DBMesoSplitModel(this, true, m_mesoIdx));
		return m_splitModels.value(splitLetter);
	}
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	void swapPlans(const QString& splitLetter1, const QString& splitLetter2);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	uint createMesoCalendarPage();
	void createMesoCalendarPage_part2();

	inline QQuickItem* getCalendarPage() const { return m_calPage; }
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	uint createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();
	Q_INVOKABLE void resetWorkout();

	inline DBTrainingDayModel* gettDayModel(const QDate& date)
	{
		if (!m_tDayModels.contains(date))
			m_tDayModels.insert(date, m_CurrenttDayModel = new DBTrainingDayModel(this, m_mesoIdx));
		else
			m_CurrenttDayModel = m_tDayModels.value(date);
		return m_CurrenttDayModel;
	}
	inline QQuickItem* gettDayPage(const QDate& date) const { return m_tDayPages.value(date); }
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }
	inline QQuickItem* currenttDayPage() const { return m_CurrenttDayPage; }
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	inline bool setsLoaded(const uint exercise_idx) const { return m_currentExercises->setCount(exercise_idx) > 0; }
	void updateOpenTDayPagesWithNewCalendarInfo(const DBTrainingDayModel* tDayModel);

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE uint createExerciseObject(DBExercisesModel* exercisesModel);
	void createExerciseObject_part2(const int object_idx = -1);
	void createExercisesObjects();

	Q_INVOKABLE inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_currentExercises->exerciseEntry(exercise_idx); }
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	void clearExercises();
	Q_INVOKABLE void moveExercise(const uint exercise_idx, const uint new_idx);
	Q_INVOKABLE void rollUpExercises() const;
	Q_INVOKABLE void manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet,
									 const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0, const bool bNewSet = false);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
									  const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text, const bool bChangeModel = true);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	Q_INVOKABLE QQuickItem* nextSetObject(const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	void enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed);
	void enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime, const uint bAutoRestTime, const uint except_set_number = 0);
	void findSetMode(const uint exercise_idx, const uint set_number);
	void findCurrentSet(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void changeSetMode(const uint exercise_idx, const uint set_number);
	void startRestTimer(const uint exercise_idx, const uint set_number);
	void stopRestTimer(const uint exercise_idx, const uint set_number);
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

public slots:
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id);
	void requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset);
	void showRemoveExerciseMessage(int exercise_idx);
	void showRemoveSetMessage(int set_number, int exercise_idx);
	void exerciseCompleted(int exercise_idx);

signals:
	void pageReady(QQuickItem* item, const uint id);
	void itemReady(QQuickItem* item, const uint id);

private:
	int m_mesoId;
	uint m_mesoIdx;
	QQmlApplicationEngine* m_QMlEngine;
	QQuickItem* m_appStackView;

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	DBMesocyclesModel* m_MesocyclesModel;
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_mesoPage;
	QVariantMap m_mesoProperties;
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	QQmlComponent* m_plannerComponent;
	QVariantMap m_plannerProperties;
	QQuickItem* m_plannerPage;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	QString m_createdSplits;
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_lastUsedCalMesoID;
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	struct tDayExercises {
		struct exerciseObject {
			QQuickItem* m_exerciseEntry;
			QList<QQuickItem*> m_setObjects;
			TPTimer* m_setTimer;

			exerciseObject() : m_setTimer(nullptr) {}
		};
		QList<exerciseObject*> exerciseObjects;

		inline QQuickItem* exerciseEntry_const(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_exerciseEntry; }
		inline QQuickItem* exerciseEntry(const uint exercise_idx) { return exerciseObjects[exercise_idx]->m_exerciseEntry; }
		inline QQuickItem* setObject_const(const uint exercise_idx, const uint set_number) const { return exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number); }
		inline QQuickItem* setObject(const uint exercise_idx, const uint set_number) const { return exerciseObjects.at(exercise_idx)->m_setObjects[set_number]; }
		inline TPTimer* setTimer(const uint exercise_idx)
		{
			if (!exerciseObjects.at(exercise_idx)->m_setTimer)
				exerciseObjects[exercise_idx]->m_setTimer = new TPTimer();
			return exerciseObjects.at(exercise_idx)->m_setTimer;
		}

		inline uint setCount(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_setObjects.count(); }
		inline uint exercisesCount() const { return exerciseObjects.count(); }

		void appendExerciseEntry(QQuickItem* new_exerciseItem);
		void removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow = false);
		void removeSet(const uint exercise_idx, const uint set_number);

		inline void insertSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject)
		{
			exerciseObjects.at(exercise_idx)->m_setObjects.insert(set_number, new_setObject);
		}
		inline void appendSet(const uint exercise_idx, QQuickItem* new_setObject)
		{
			exerciseObjects.at(exercise_idx)->m_setObjects.append(new_setObject);
		}

		inline void clear(const bool bDeleteNow = false)
		{
			for (int i(exerciseObjects.count() - 1); i >= 0 ; --i)
				removeExerciseEntry(i, bDeleteNow);
		}

		~tDayExercises()
		{
			clear(true);
		}
	};

	QMap<QDate,DBTrainingDayModel*> m_tDayModels;
	QMap<QDate,QQuickItem*> m_tDayPages;
	QMap<QDate,tDayExercises*> m_tDayExercisesList;
	tDayExercises* m_currentExercises;
	QVariantMap m_tDayProperties;
	QQmlComponent* m_tDayComponent;
	DBTrainingDayModel* m_CurrenttDayModel;
	QQuickItem* m_CurrenttDayPage;

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	QVariantMap m_tDayExerciseEntryProperties;
	QQmlComponent* m_tDayExercisesComponent;
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	uint m_expectedSetNumber;
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
};
#endif // TPMESOCYCLECLASS_H
