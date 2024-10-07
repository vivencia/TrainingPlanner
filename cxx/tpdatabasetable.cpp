#include "tpdatabasetable.h"

#include <QFile>

void TPDatabaseTable::removeEntry()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"DELETE FROM "_qs + m_tableName + u" WHERE id="_qs + m_execArgs.at(0).toString());
		ok = query.exec(strQuery);
		setResult(ok, nullptr, strQuery, {std::source_location::current()})
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
		const QString& strQuery(u" DROP TABLE "_qs + m_tableName);
		ok = query.exec(strQuery);
		setResult(ok, nullptr, strQuery, {std::source_location::current()})
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeDBFile()
{
	const bool ok = QFile::remove(mSqlLiteDB.databaseName());
	if (ok)
		createTable();
	setResult(ok, nullptr, "", {std::source_location::current()});
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
