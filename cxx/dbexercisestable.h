#ifndef DBEXERCISESTABLE_H
#define DBEXERCISESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

static const QString DBExercisesFileName(u"ExercisesList.db.sqlite"_qs);

class DBExercisesModel;

class DBExercisesTable : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesTable(const QString& dbFilePath, DBExercisesModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllExercises();
	void updateExercisesList();
	void saveExercises();

signals:
	void updatedFromExercisesList();

private:
	DBExercisesModel* m_model;
	uint m_exercisesTableLastId;
	QStringList m_ExercisesList;

	void removePreviousListEntriesFromDB();
	void getExercisesList();
};

#endif // DBEXERCISESTABLE_H
