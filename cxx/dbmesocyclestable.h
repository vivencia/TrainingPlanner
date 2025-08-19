#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)

class DBMesocyclesTable final : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(DBMesocyclesModel *model);

	static QLatin1StringView tableName();
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getAllMesocycles();
	void saveMesocycle();

private:
	DBMesocyclesModel *m_model;
};
