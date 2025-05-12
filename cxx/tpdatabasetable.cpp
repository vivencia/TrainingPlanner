#include "tpdatabasetable.h"

#include "tpglobals.h"

#include <QFile>

using namespace Qt::Literals::StringLiterals;

void TPDatabaseTable::removeEntry(const bool bUseMesoId)
{
	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery("DELETE FROM "_L1 + m_tableName + (bUseMesoId ? " WHERE meso_id="_L1 : " WHERE id="_L1) + m_execArgs.at(0).toString());
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeTemporaries(const bool bUseMesoId)
{
	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery("DELETE FROM "_L1 + m_tableName + (bUseMesoId ? " WHERE meso_id<0"_L1 : " WHERE id<0"_L1));
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
		QSqlQuery query{getQuery()};
		const QString &strQuery{std::move("DELETE FROM "_L1 + m_tableName)};
		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeDBFile()
{
	const bool ok{QFile::remove(mSqlLiteDB.databaseName())};
	if (ok)
		createTable();
	setQueryResult(ok, "", SOURCE_LOCATION);
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
