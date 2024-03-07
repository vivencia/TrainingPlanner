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

class TPMesocycleClass : public QObject
{

Q_OBJECT

public:
	TPMesocycleClass(const int meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent = nullptr);
	~TPMesocycleClass();

	inline int mesoId() const { return m_MesoId; }
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
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	uint createMesoCalendarPage();
	void createMesoCalendarPage_part2();

	inline DBMesoCalendarModel* getCalendarModel()
	{
		if (m_mesosCalendarModel == nullptr)
			m_mesosCalendarModel = new DBMesoCalendarModel(this);
		return m_mesosCalendarModel;
	}

	inline QQuickItem* getCalendarPage() const { return m_calPage; }
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	uint createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();

	inline DBTrainingDayModel* gettDayModel(const QDate& date)
	{
		if (!m_tDayModels.contains(date))
			m_tDayModels.insert(date, new DBTrainingDayModel(this));
		return m_tDayModels.value(date);
	}
	inline QQuickItem* gettDayPage(const QDate& date) const { return m_tDayPages.value(date); }
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }
	inline QQuickItem* currenttDayPage() const { return m_CurrenttDayPage; }
	inline void setCurrenttDay(const QDate& date) { m_CurrenttDayModel = m_tDayModels.value(date); m_CurrenttDayPage = m_tDayPages.value(date); }

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	uint createExerciseObject(const QString& exerciseName);
	void createExerciseObject_part2(const int object_idx = -1);
	void createExercisesObjects();

	inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_tDayExercises.at(exercise_idx); }
	void removeExercise(const uint exercise_idx);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx);
	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0);

	inline QQuickItem* getSetObject(const uint set_number, const uint exercise_idx) const
	{
		return set_number < m_setObjects.count() ? m_setObjects.value(exercise_idx).at(set_number) : nullptr;
	}
	void removeSet(const uint set_number, const uint exercise_idx);
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

public slots:
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, const QVariant& visible);

signals:
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
	QMap<QDate,DBTrainingDayModel*> m_tDayModels;
	QMap<QDate,QQuickItem*> m_tDayPages;
	QQmlComponent* m_tDayComponent;
	QVariantMap m_tDayProperties;
	DBTrainingDayModel* m_CurrenttDayModel;
	QQuickItem* m_CurrenttDayPage;
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	QVariantMap m_tDayExerciseEntryProperties;
	QList<QQuickItem*> m_tDayExercises;
	QQmlComponent* m_tDayExercisesComponent;
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	QQmlComponent* m_setComponents[6];
	QMap<uint,QList<QQuickItem*>> m_setObjects;
	QList<uint> m_setCounter;
	QVariantMap m_setObjectProperties;
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

};
#endif // TPMESOCYCLECLASS_H
