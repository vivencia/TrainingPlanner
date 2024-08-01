#ifndef DBUSERTABLE_H
#define DBUSERTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBUserModel;

static const QString DBUserFileName(QStringLiteral("User.db.sqlite"));

class DBUserTable : public TPDatabaseTable
{

public:
	explicit DBUserTable(const QString& dbFilePath, QSettings* appSettings, DBUserModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllUsers();
	void saveUser();

private:
};

#endif // DBUSERTABLE_H
