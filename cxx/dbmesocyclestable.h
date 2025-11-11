#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)

class DBMesocyclesTable final : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable();

	inline static QLatin1StringView tableName() { return "mesocycles_table"_L1; }
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getAllMesocycles();
	void saveMesocycle();
};
