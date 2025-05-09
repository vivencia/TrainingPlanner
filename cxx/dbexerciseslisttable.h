#ifndef DBEXERCISESTABLE_H
#define DBEXERCISESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

static const QLatin1StringView &DBExercisesFileName("ExercisesList.db.sqlite"_L1);

class DBExercisesModel;

class DBExercisesTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesTable(const QString &dbFilePath, DBExercisesModel *model = nullptr);

	void createTable() override final;
	void updateTable() override final;
	void getAllExercises();
	void updateExercisesList();
	void saveExercises();

signals:
	void updatedFromExercisesList();

private:
	DBExercisesModel *m_model;
	uint m_exercisesTableLastId;
	QStringList m_ExercisesList;

	void getExercisesList();
};

#endif // DBEXERCISESTABLE_H
