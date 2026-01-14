#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercisesList)

class DBExercisesListTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBExercisesListTable(DBModelInterfaceExercisesList *dbmodel_interface, const QString &list_version);

	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getAllExercises(void *);
	bool updateExercisesList();

signals:
	void updatedFromExercisesList();

private:
	QStringList m_exercisesList;
	bool m_updateList;
	bool getExercisesList();
};

