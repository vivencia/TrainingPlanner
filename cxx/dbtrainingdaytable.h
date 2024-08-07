#ifndef DBTRAININGDAYTABLE_H
#define DBTRAININGDAYTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBTrainingDayModel;

static const QString DBTrainingDayFileName ( QStringLiteral("TrainingDay.db.sqlite") );

class DBTrainingDayTable : public TPDatabaseTable
{

public:
	explicit DBTrainingDayTable(const QString& dbFilePath, QSettings* appSettings, DBTrainingDayModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase();
	void getTrainingDay();
	void getTrainingDayExercises(const bool bClearSomeFieldsForReUse = false);
	void getPreviousTrainingDays();
	void saveTrainingDay();
	void removeTrainingDay();

private:
	QString formatDate(const uint julianDay) const;
};

#endif // DBTRAININGDAYTABLE_H
