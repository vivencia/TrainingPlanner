#ifndef DBTRAININGDAYTABLE_H
#define DBTRAININGDAYTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBTrainingDayModel;

static const QString DBTrainingDayFileName ( QStringLiteral("TrainingDay.db.sqlite") );
static const QString DBTrainingDayObjectName ( QStringLiteral("TrainingDay") );
static const uint TRAININGDAY_TABLE_ID = 0x0005;

class DBTrainingDayTable : public TPDatabaseTable
{

public:
	explicit DBTrainingDayTable(const QString& dbFilePath, QSettings* appSettings, DBTrainingDayModel* model = nullptr);

	virtual void createTable();
	void getTrainingDay();
	void getTrainingDayExercises();
	void getPreviousTrainingDays();
	void newTrainingDay();
	void updateTrainingDay();
	void updateTrainingDayExercises();
	void removeTrainingDay();
	void deleteTrainingDayTable();

private:
	QString formatDate(const uint julianDay) const;
};

#endif // DBTRAININGDAYTABLE_H
