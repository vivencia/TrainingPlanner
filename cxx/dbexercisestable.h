#ifndef DBEXERCISESTABLE_H
#define DBEXERCISESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

static const QString DBExercisesFileName(QStringLiteral("ExercisesList.db.sqlite"));

class DBExercisesModel;

class DBExercisesTable : public TPDatabaseTable
{

public:
	explicit DBExercisesTable(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllExercises();
	void updateExercisesList();
	void updateFromModel();
	void saveExercise();

	//Call before starting a thread
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
