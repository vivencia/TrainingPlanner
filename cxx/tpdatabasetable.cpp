#include "tpdatabasetable.h"
#include "tpglobals.h"

#include <QFile>

void TPDatabaseTable::removeEntry()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery("DELETE FROM "_L1 + m_tableName + " WHERE id="_L1 + m_execArgs.at(0).toString());
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::clearTable()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery{" DROP TABLE "_L1 + m_tableName};
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeDBFile()
{
	const bool ok = QFile::remove(mSqlLiteDB.databaseName());
	if (ok)
		createTable();
	setQueryResult(ok, "", SOURCE_LOCATION);
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
