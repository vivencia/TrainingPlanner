#ifndef DBUSERTABLE_H
#define DBUSERTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBUserModel;

static const QString DBUserFileName(u"User.db.sqlite"_qs);

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(const QString& dbFilePath, DBUserModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllUsers();
	void saveUser();
};

#endif // DBUSERTABLE_H
