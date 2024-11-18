#ifndef DBTRAININGDAYTABLE_H
#define DBTRAININGDAYTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBTrainingDayModel;

static const QString DBTrainingDayFileName(u"TrainingDay.db.sqlite"_s);

class DBTrainingDayTable final : public TPDatabaseTable
{

public:
	explicit DBTrainingDayTable(const QString& dbFilePath, DBTrainingDayModel* model = nullptr);

	void createTable() override;
	void updateTable() override;
	void getTrainingDay();
	void getTrainingDayExercises(const bool bClearSomeFieldsForReUse = false);
	void getPreviousTrainingDaysInfo();
	void saveTrainingDay();
	void removeTrainingDay();

	inline DBTrainingDayModel* model() const { return m_model; }

	//Functions for TPStatistics
	void workoutInfoForTimePeriod();
	inline const QList<QStringList>& workoutInfo() const { return m_workoutInfo; }

private:
	DBTrainingDayModel* m_model;
	QList<QStringList> m_workoutInfo;
	inline QString formatDate(const uint julianDay) const;
};

#endif // DBTRAININGDAYTABLE_H
