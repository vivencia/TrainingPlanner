#ifndef DBWORKOUTTABLE_H
#define DBWORKOUTTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBWorkoutModel;

static const QLatin1StringView &DBTrainingDayFileName("TrainingDay.db.sqlite"_L1);

class DBWorkoutsTable final : public TPDatabaseTable
{

public:
	explicit DBWorkoutsTable(const QString& dbFilePath, DBWorkoutModel* model = nullptr);

	void createTable() override final;
	void updateTable() override final;
	void getTrainingDay();
	void getTrainingDayExercises(const bool bClearSomeFieldsForReUse = false);
	void getPreviousTrainingDaysInfo();
	void saveTrainingDay();
	void removeWorkout();

	inline DBWorkoutModel* model() const { return m_model; }

	//Functions for TPStatistics
	void workoutsInfoForTimePeriod();
	inline const QList<QList<QStringList>>& workoutsInfo() const { return m_workoutsInfo; }

private:
	DBWorkoutModel* m_model;
	QList<QList<QStringList>> m_workoutsInfo;
	inline QString formatDate(const uint julianDay) const;
};

#endif // DBWORKOUTTABLE_H
