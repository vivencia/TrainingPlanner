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
		QSqlQuery query{std::move(getQuery())};
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
		QSqlQuery query{std::move(getQuery())};
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
		QSqlQuery query{std::move(getQuery())};
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

bool TPDatabaseTable::openDatabase(const bool bReadOnly)
{
	mSqlLiteDB.setConnectOptions(bReadOnly ? "QSQLITE_OPEN_READONLY"_L1 : "QSQLITE_BUSY_TIMEOUT=0"_L1);
	const bool ok{mSqlLiteDB.open()};
	#ifndef QT_NO_DEBUG
	if (!ok)
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("Could not open Database file: "_L1, mSqlLiteDB.databaseName())
	}
	#endif
	return ok;
}

QSqlQuery TPDatabaseTable::getQuery() const
{
	QSqlQuery query{mSqlLiteDB};
	if (!mSqlLiteDB.connectOptions().isEmpty())
		query.setForwardOnly(true);
	static_cast<void>(query.exec("PRAGMA page_size = 4096"_L1));
	static_cast<void>(query.exec("PRAGMA cache_size = 16384"_L1));
	static_cast<void>(query.exec("PRAGMA temp_store = MEMORY"_L1));
	static_cast<void>(query.exec("PRAGMA journal_mode = OFF"_L1));
	static_cast<void>(query.exec("PRAGMA locking_mode = EXCLUSIVE"_L1));
	static_cast<void>(query.exec("PRAGMA synchronous = 0"_L1));
	return query;
}

#ifndef QT_NO_DEBUG
void TPDatabaseTable::_setQueryResult(const bool bResultOK, const std::source_location &location, const QString &message)
{
	mb_result = bResultOK;
	if (!message.isEmpty())
	{
		if (bResultOK)
			SUCCESS_MESSAGE_WITH_STATEMENT(PRINT_SOURCE_LOCATION)
		else
			ERROR_MESSAGE(message, "")
	}
	if (mSqlLiteDB.connectOptions().isEmpty()) //optimize after modifying the database
	{
		QSqlQuery query{mSqlLiteDB};
		static_cast<void>(query.exec("VACUUM"_L1));
		static_cast<void>(query.exec("PRAGMA optimize"_L1));
	}
	mSqlLiteDB.close();
}
#else
void TPDatabaseTable::_setQueryResult(const bool bResultOK)
{
	mb_result = bResultOK;
	if (mSqlLiteDB.connectOptions().isEmpty()) //optimize after modifying the database
	{
		QSqlQuery query{mSqlLiteDB};
		static_cast<void>(query.exec("VACUUM"_L1));
		static_cast<void>(query.exec("PRAGMA optimize"_L1));
	}
	mSqlLiteDB.close();
}
#endif
