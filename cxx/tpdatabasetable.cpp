#include "tpdatabasetable.h"

#include "tpglobals.h"

#include "dbexerciseslisttable.h"
#include "dbmesocalendartable.h"
#include "dbmesocyclestable.h"
#include "dbusertable.h"
#include "dbworkoutsorsplitstable.h"

#include <QFile>

using namespace Qt::Literals::StringLiterals;

TPDatabaseTable *TPDatabaseTable::createDBTable(const uint table_id, const bool auto_delete)
{
	TPDatabaseTable *db_table{nullptr};
	switch (table_id)
	{
		case EXERCISES_TABLE_ID: db_table = new DBExercisesTable{nullptr}; break;
		case MESOCYCLES_TABLE_ID: db_table = new DBMesocyclesTable{nullptr}; break;
		case MESOSPLIT_TABLE_ID: db_table = new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}; break;
		case MESOCALENDAR_TABLE_ID: db_table = new DBMesoCalendarTable{nullptr}; break;
		case WORKOUT_TABLE_ID: db_table = new DBWorkoutsOrSplitsTable{WORKOUT_TABLE_ID}; break;
		case USERS_TABLE_ID: db_table = new DBUserTable{nullptr}; break;
	}
	if (db_table && auto_delete)
		db_table->deleteLater();
	return db_table;
}

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
