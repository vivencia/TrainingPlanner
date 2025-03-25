#ifndef DBTRAININGDAYTABLE_H
#define DBTRAININGDAYTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBTrainingDayModel;

static const QLatin1StringView &DBTrainingDayFileName("TrainingDay.db.sqlite"_L1);

class DBTrainingDayTable final : public TPDatabaseTable
{

public:
	explicit DBTrainingDayTable(const QString& dbFilePath, DBTrainingDayModel* model = nullptr);

	void createTable() override final;
	void updateTable() override final;
	void getTrainingDay();
	void getTrainingDayExercises(const bool bClearSomeFieldsForReUse = false);
	void getPreviousTrainingDaysInfo();
	void saveTrainingDay();
	void removeTrainingDay();

	inline DBTrainingDayModel* model() const { return m_model; }

	//Functions for TPStatistics
	void workoutsInfoForTimePeriod();
	inline const QList<QList<QStringList>>& workoutsInfo() const { return m_workoutsInfo; }

private:
	DBTrainingDayModel* m_model;
	QList<QList<QStringList>> m_workoutsInfo;
	inline QString formatDate(const uint julianDay) const;
};

#endif // DBTRAININGDAYTABLE_H
