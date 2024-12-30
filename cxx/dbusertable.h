#ifndef DBUSERTABLE_H
#define DBUSERTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBUserModel;

static const QString& DBUserFileName("User.db.sqlite"_L1);

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(const QString& dbFilePath, DBUserModel* model = nullptr);

	virtual void createTable() override final;
	virtual void updateTable() override final {}
	void getAllUsers();
	void saveUser();

private:
	DBUserModel* m_model;
};

#endif // DBUSERTABLE_H
