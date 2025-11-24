#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceMesocycle)

class DBMesocyclesTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBMesocyclesTable(DBModelInterfaceMesocycle *dbmodel_interface);

	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getAllMesocycles();
};
