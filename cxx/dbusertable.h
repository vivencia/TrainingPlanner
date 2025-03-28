#pragma once

#include "tpdatabasetable.h"

#include <QObject>

class DBUserModel;

static const QLatin1StringView &DBUserFileName{"Users.db.sqlite"_L1};

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(const QString &dbFilePath, DBUserModel *model = nullptr);

	virtual void createTable() override final;
	virtual void updateTable() override final {}
	void getAllUsers();
	void saveUser();
	void removeUser();

private:
	DBUserModel *m_model;
};

