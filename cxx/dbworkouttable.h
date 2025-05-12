#ifndef DBWORKOUTTABLE_H
#define DBWORKOUTTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)

class DBWorkoutsTable final : public TPDatabaseTable
{

public:
	explicit DBWorkoutsTable(DBExercisesModel* model);

	void createTable() override final;
	void updateTable() override final;
	void getExercises();
	void saveExercises();
	void removeExercises();

	inline DBExercisesModel* model() const { return m_model; }

	//Functions for TPStatistics
	void workoutsInfoForTimePeriod();
	inline const QList<QList<QStringList>>& workoutsInfo() const { return m_workoutsInfo; }

private:
	DBExercisesModel* m_model;
	QList<QList<QStringList>> m_workoutsInfo;
};

#endif // DBWORKOUTTABLE_H
