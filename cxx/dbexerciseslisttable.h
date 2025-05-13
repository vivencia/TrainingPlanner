#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBExercisesListModel)

class DBExercisesTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesTable(DBExercisesListModel *model);

	void createTable() override final;
	virtual void updateTable() override final {}

	void getAllExercises();
	void updateExercisesList();
	void saveExercises();

signals:
	void updatedFromExercisesList();

private:
	DBExercisesListModel *m_model;
	uint m_exercisesTableLastId;
	QStringList m_ExercisesList;

	void getExercisesList();
};

