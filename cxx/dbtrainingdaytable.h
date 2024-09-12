#ifndef DBTRAININGDAYTABLE_H
#define DBTRAININGDAYTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBTrainingDayModel;

static const QString DBTrainingDayFileName(u"TrainingDay.db.sqlite"_qs);

class DBTrainingDayTable : public TPDatabaseTable
{

public:
	explicit DBTrainingDayTable(const QString& dbFilePath, DBTrainingDayModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase();
	void getTrainingDay();
	void getTrainingDayExercises(const bool bClearSomeFieldsForReUse = false);
	void getPreviousTrainingDaysInfo();
	void saveTrainingDay();
	void removeTrainingDay();

private:
	QString formatDate(const uint julianDay) const;
};

#endif // DBTRAININGDAYTABLE_H
