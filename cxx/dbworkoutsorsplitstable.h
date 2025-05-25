#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)

class DBWorkoutsOrSplitsTable final : public TPDatabaseTable
{

public:
	explicit DBWorkoutsOrSplitsTable(DBExercisesModel *model);
	inline explicit DBWorkoutsOrSplitsTable(const short table_id) : TPDatabaseTable{table_id, nullptr}, m_model{nullptr} { commonConstructor(); }

	void createTable() override final;
	virtual void updateTable() override final {}

	void getExercises();
	void saveExercises();
	void removeExercises();
	bool mesoHasAllSplitPlans(const QString &meso_id, const QString &split);
	bool mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter);
	void getPreviousWorkouts();

	inline DBExercisesModel *model() const { return m_model; }
	inline void setModel(DBExercisesModel *model) { m_model = model; }

	//Functions for TPStatistics
	//void workoutsInfoForTimePeriod();
	//inline const QList<QList<QStringList>>& workoutsInfo() const { return m_workoutsInfo; }

private:
	DBExercisesModel *m_model;
	//QList<QList<QStringList>> m_workoutsInfo;

	void commonConstructor();
};
