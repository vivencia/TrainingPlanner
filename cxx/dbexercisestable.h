#ifndef DBEXERCISESTABLE_H
#define DBEXERCISESTABLE_H

#include "tpdatabasetable.h"
#include "dbexercisesmodel.h"

#include <QObject>
#include <QSettings>

static const QString DBExercisesFileName ( QStringLiteral("ExercisesList.db.sqlite") );
static const QString DBExercisesObjectName ( QStringLiteral("Exercises") );
static const uint EXERCISES_TABLE_ID = 0x0001;

class DBExercisesModel;

class dbExercisesTable : public TPDatabaseTable
{

public:
	explicit dbExercisesTable(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model = nullptr);

	void createTable();
	void getAllExercises();
	void updateExercisesList();
	void newExercise();
	void updateExercise();
	void removeExercise();

	//Call before starting a thread that execs newExercise() and updateExercise()
	void setData(const QString& id, const QString& mainName = QString(), const QString& subName = QString(),
						const QString& muscularGroup = QString(), const QString& nSets = QString(),
						const QString& nReps = QString(), const QString& nWeight = QString(),
						const QString& uWeight = QString(), const QString& mediaPath = QString());

private:
	static uint m_exercisesTableLastId;
	QStringList m_ExercisesList;

	void removePreviousListEntriesFromDB();
	void getExercisesList();
};

#endif // DBEXERCISESTABLE_H