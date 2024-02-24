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
	void newTrainingDay();
	void updateTrainingDay();
	void updateTrainingDayExercises();
	void removeTrainingDay();
	void deleteTrainingDayTable();

	//Call before starting a thread
	void setData(const QString& id, const QString& mesoId = QString(), const QString& date = QString(), const QString& trainingDayNumber = QString(),
				const QString& splitLetter = QString(), const QString& timeIn = QString(), const QString& timeOut = QString(),
				const QString& location = QString(), const QString& notes = QString());
	void setExercisesData(const QString& id, const QString& exercises_names, const QString& sets_types, const QString& rest_times, const QString& sub_sets,
							const QString& reps, const QString& weights);
};

#endif // DBTRAININGDAYTABLE_H
