#include "dbmanager.h"

#include <QSqlQuery>

DbManager::DbManager(const QString& dbPath)
	: QObject (nullptr)
{
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE");
	mSqlLiteDB.setDatabaseName(dbPath);
}

bool DbManager::updateExercisesList(const QStringList &exercisesList)
{
	int ret (mSqlLiteDB.open());
	if (ret) {

	}
	else {
		qDebug() << "SQLite database could not be opened:" << mSqlLiteDB.databaseName();
	}
	return ret;
}
