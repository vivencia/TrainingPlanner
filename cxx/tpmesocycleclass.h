#ifndef TPMESOCYCLECLASS_H
#define TPMESOCYCLECLASS_H

#include "dbmesosplitmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaymodel.h"

#include <QObject>
#include <QMap>
#include <QDate>

class DBMesocyclesModel;
class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

static const uint mesoPageCreateId(175);
static const uint calPageCreateId(35);
static const uint tDayPageCreateId(70);
static const uint tDayExerciseCreateId(105);
static const uint tDaySetCreateId(140);

class TPMesocycleClass : public QObject
{

Q_OBJECT

public:
	TPMesocycleClass(const int meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent = nullptr);
	~TPMesocycleClass();

	inline int mesoId() const { return m_MesoId; }
	inline void setMesoId(const int new_mesoid) { m_MesoId = new_mesoid; }
	inline uint mesoIdx() const { return m_MesoIdx; }

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();

	inline void setMesocycleModel(DBMesocyclesModel* model) { m_MesocyclesModel = model; }
	inline QQuickItem* getMesoPage() const { return m_MesoPage; }
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	void createMesoSplitPage();
	void createMesoSplitPage_part2();

	inline DBMesoSplitModel* getSplitModel(const QChar& splitLetter)
	{
		if (!m_splitModels.contains(splitLetter))
			m_splitModels.insert(splitLetter, new DBMesoSplitModel(this));
		return m_splitModels.value(splitLetter);
	}
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	void pushSplitPage(const QChar& splitLetter) const;
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	uint createMesoCalendarPage();
	void createMesoCalendarPage_part2();

	inline QQuickItem* getCalendarPage() const { return m_calPage; }
	inline void setMesoCalendarModel(DBMesoCalendarModel* model) { m_mesosCalendarModel = model; }
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	uint createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();
	void convertMesoPlanToTDayExercises(DBMesoSplitModel* splitModel);

	inline DBTrainingDayModel* gettDayModel(const QDate& date)
	{
		if (!m_tDayModels.contains(date))
			m_tDayModels.insert(date, new DBTrainingDayModel(this));
		m_CurrenttDayModel = m_tDayModels.value(date);
		return m_CurrenttDayModel;
	}
	inline QQuickItem* gettDayPage(const QDate& date) const { return m_tDayPages.value(date); }
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }
	inline QQuickItem* currenttDayPage() const { return m_CurrenttDayPage; }
	inline void setCurrenttDay(const QDate& date) { m_CurrenttDayModel = m_tDayModels.value(date); m_CurrenttDayPage = m_tDayPages.value(date); }
	inline bool setsLoaded(const uint exercise_idx) const { return m_currentExercises->setCount(exercise_idx) > 0; }

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE uint createExerciseObject(const QString& exerciseName, const QString& nSets, const QString& nReps, const QString& nWeight);
	void createExerciseObject_part2(const int object_idx = -1);
	void createExercisesObjects();

	Q_INVOKABLE inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_currentExercises->exerciseEntry(exercise_idx); }
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE inline void clearExercises() { m_currentExercises->clear(); m_CurrenttDayModel->clearExercises(); }
	void moveExercise(const uint exercise_idx, const uint new_idx);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx,
							const QString& nReps = QString(), const QString& nWeight = QString());
	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
							const QString& nReps = QString(), const QString& nWeight = QString());
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text);
	void moveSets(const uint exercise_idx, const uint new_idx);
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

public slots:
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id);
	void requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type);

signals:
	void pageReady(QQuickItem* item, const uint id);
	void itemReady(QQuickItem* item, const uint id);

private:
	int m_MesoId;
	uint m_MesoIdx;
	QQmlApplicationEngine* m_QMlEngine;

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	DBMesocyclesModel* m_MesocyclesModel;
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_MesoPage;
	QVariantMap m_mesoProperties;
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	QString m_createdSplits;
	QQuickItem* m_qmlSplitObjectParent;
	QQuickItem* m_qmlSplitObjectContainer;
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	DBMesoCalendarModel* m_mesosCalendarModel;
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
		};
		QList<exerciseObject*> exerciseObjects;

		inline QQuickItem* exerciseEntry_const(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_exerciseEntry; }
		inline QQuickItem* exerciseEntry(const uint exercise_idx) { return exerciseObjects[exercise_idx]->m_exerciseEntry; }
		inline QQuickItem* setObject_const(const uint exercise_idx, const uint set_number) const { return exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number); }
		inline QQuickItem* setObject(const uint exercise_idx, const uint set_number) const { return exerciseObjects[exercise_idx]->m_setObjects[set_number]; }

		inline uint setCount(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_setObjects.count(); }
		inline uint exercisesCount() const { return exerciseObjects.count(); }

		void appendExerciseEntry(QQuickItem* new_exerciseItem);
		void removeExerciseEntry(const uint exercise_idx);
		void removeSet(const uint exercise_idx, const uint set_number);

		inline void appendSet(const uint exercise_idx, QQuickItem* new_setObject)
		{
			exerciseObjects.at(exercise_idx)->m_setObjects.append(new_setObject);
		}

		inline void clear()
		{
			for (uint i(0); i < exerciseObjects.count(); ++i)
				removeExerciseEntry(i);
			exerciseObjects.clear();
		}

		~tDayExercises()
		{
			clear();
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
	QQmlComponent* m_setComponents[7];
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

};
#endif // TPMESOCYCLECLASS_H
