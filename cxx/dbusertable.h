#ifndef DBUSERTABLE_H
#define DBUSERTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBUserModel;

static const QString DBUserFileName(u"User.db.sqlite"_s);

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(const QString& dbFilePath, DBUserModel* model = nullptr);

	virtual void createTable();
	virtual void updateTable() {}
	void getAllUsers();
	void saveUser();

private:
	DBUserModel* m_model;
};

#endif // DBUSERTABLE_H
