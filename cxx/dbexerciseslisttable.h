#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBExercisesListModel)

class DBExercisesListTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesListTable();

	inline static QLatin1StringView tableName() { return "exercises_table"_L1; }
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getAllExercises();
	void updateExercisesList();
	void saveExercises();

signals:
	void updatedFromExercisesList();

private:
	uint m_exercisesTableLastId;
	QStringList m_ExercisesList;

	void getExercisesList();
};

