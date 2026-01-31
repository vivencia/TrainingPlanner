#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercisesList)

class DBExercisesListTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesListTable(DBModelInterfaceExercisesList *dbmodel_interface);

	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}
	bool getAllExercises(void *);

private:
	QStringList m_exercisesList;
	bool exercisesListUpdated();

	bool readExercisesFromList();
};

