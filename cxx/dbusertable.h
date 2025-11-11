#pragma once

#include "tpdatabasetable.h"

#include <QObject>

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable();

	inline static QLatin1StringView tableName() { return "users_table"_L1; }
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getAllUsers();
	void saveUser();
	void removeUser();
};

