#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBUserModel)

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(DBUserModel *model);

	static QLatin1StringView tableName();
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getAllUsers();
	void saveUser();
	void removeUser();

private:
	DBUserModel *m_model;
};

