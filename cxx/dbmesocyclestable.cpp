#include "dbmesocyclestable.h"
#include "dbmesocyclesmodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlQuery>
#include <QTime>

DBMesocyclesTable::DBMesocyclesTable(const QString& dbFilePath, DBMesocyclesModel* model)
	: TPDatabaseTable{}, m_model{model}
{
	m_tableName = std::move("mesocycles_table"_L1);
	m_tableID = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString& cnx_name{"db_meso_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString& dbname{dbFilePath + DBMesocyclesFileName};
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesocyclesTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString& strQuery{"CREATE TABLE IF NOT EXISTS mesocycles_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_name TEXT,"
										"meso_start_date INTEGER,"
										"meso_end_date INTEGER,"
										"meso_note TEXT,"
										"meso_nweeks INTEGER,"
										"meso_split TEXT,"
										"meso_coach TEXT,"
										"meso_client TEXT,"
										"meso_program_file TEXT,"
										"meso_type TEXT,"
										"real_meso INTEGER"
									")"_L1
		};
		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBMesocyclesTable::updateTable()
{
	/*m_result = false;
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		query.exec(u"PRAGMA page_size = 4096"_s);
		query.exec(u"PRAGMA cache_size = 16384"_s);
		query.exec(u"PRAGMA temp_store = MEMORY"_s);
		query.exec(u"PRAGMA journal_mode = OFF"_s);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_s);
		query.exec(u"PRAGMA synchronous = 0"_s);
		query.setForwardOnly(true);

		QStringList oldTableInfo;
		if (query.exec(u"SELECT * FROM mesocycles_table"_s))
		{
			if (query.first ())
			{
				do {
					oldTableInfo << query.value(MESOCYCLES_COL_ID).toString() << query.value(MESOCYCLES_COL_NAME).toString() <<
							query.value(MESOCYCLES_COL_STARTDATE).toString() << query.value(MESOCYCLES_COL_ENDDATE).toString() <<
							query.value(MESOCYCLES_COL_NOTE).toString() << query.value(MESOCYCLES_COL_WEEKS).toString() <<
							query.value(MESOCYCLES_COL_SPLIT).toString() << QString() << QString() << QString() << QString() <<
							(query.value(MESOCYCLES_COL_ENDDATE).toUInt() != 0 ? STR_ONE : STR_ZERO);
				} while (query.next());
			}
			m_result = oldTableInfo.count() > 0;
		}
		mSqlLiteDB.close();
		clearTable();
		if (m_result)
		{
			createTable();
			if (m_result)
			{
				if (openDatabase())
				{
					query.exec(u"PRAGMA page_size = 4096"_s);
					query.exec(u"PRAGMA cache_size = 16384"_s);
					query.exec(u"PRAGMA temp_store = MEMORY"_s);
					query.exec(u"PRAGMA journal_mode = OFF"_s);
					query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_s);
					query.exec(u"PRAGMA synchronous = 0"_s);

					QString strQuery;
					for (uint i(0); i <= oldTableInfo.count() - MESOCYCLES_TOTAL_COLS; i += MESOCYCLES_TOTAL_COLS)
					{
						strQuery = QStringLiteral(
							"INSERT INTO mesocycles_table "
							"(id,meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,"
							"meso_coach,meso_client,meso_program_file,meso_type,real_meso)"
							" VALUES(%1, \'%2\', %3, %4, \'%5\', %6, \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', %12")
							.arg(oldTableInfo.at(i+MESOCYCLES_COL_ID), oldTableInfo.at(i+MESOCYCLES_COL_NAME), oldTableInfo.at(i+MESOCYCLES_COL_STARTDATE),
								oldTableInfo.at(i+MESOCYCLES_COL_ENDDATE), oldTableInfo.at(i+MESOCYCLES_COL_NOTE), oldTableInfo.at(i+MESOCYCLES_COL_WEEKS),
								oldTableInfo.at(i+MESOCYCLES_COL_SPLIT), oldTableInfo.at(i+MESOCYCLES_COL_COACH), oldTableInfo.at(i+MESOCYCLES_COL_CLIENT),
								oldTableInfo.at(i+MESOCYCLES_COL_FILE),	oldTableInfo.at(i+MESOCYCLES_COL_TYPE), oldTableInfo.at(i+MESOCYCLES_COL_REALMESO));
						m_result = query.exec(strQuery);
					}
					if (!m_result)
					{
						MSG_OUT("DBMesocyclesTable updateDatabase Database error:  " << mSqlLiteDB.lastError().databaseText())
						MSG_OUT("DBMesocyclesTable updateDatabase Driver error:  " << mSqlLiteDB.lastError().driverText())
					}
					else
						MSG_OUT("DBMesoCalendarTable updateDatabase SUCCESS")
					mSqlLiteDB.close();
				}
			}
		}
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));*/
}

void DBMesocyclesTable::getAllMesocycles()
{
	if (openDatabase(true))
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString& strQuery{"SELECT * FROM mesocycles_table"_L1};

		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				do
				{
					QStringList meso_info{MESOCYCLES_TOTAL_COLS};
					for (uint i{MESOCYCLES_COL_ID}; i < MESOCYCLES_TOTAL_COLS; ++i)
						meso_info[i] = std::move(query.value(i).toString());
					static_cast<void>(m_model->newMesocycle(std::move(meso_info)));
				} while (query.next ());
				m_model->setReady(ok = true);
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBMesocyclesTable::saveMesocycle()
{
	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const uint row{m_execArgs.at(0).toUInt()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec("SELECT id FROM mesocycles_table WHERE id=%1"_L1.arg(m_model->id(row))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE mesocycles_table SET meso_name=\'%1\', meso_start_date=%2, meso_end_date=%3, "
							"meso_note=\'%4\', meso_nweeks=%5, meso_split=\'%6\', meso_coach=\'%7\', meso_client=\'%8\', "
							"meso_program_file=\'%9\', meso_type=\'%10\', real_meso=\'%11\' WHERE id=%12"_s
								.arg(m_model->name(row), m_model->strStartDate(row), m_model->strEndDate(row), m_model->notes(row),
									m_model->nWeeks(row), m_model->split(row), m_model->coach(row), m_model->client(row),
									m_model->file(row), m_model->type(row), m_model->realMeso(row), m_model->id(row)));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO mesocycles_table "
							"(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,"
							"meso_coach,meso_client,meso_program_file,meso_type,real_meso)"
							" VALUES(\'%1\', %2, %3, \'%4\', %5, \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', %11)"_s
								.arg(m_model->name(row), m_model->strStartDate(row), m_model->strEndDate(row), m_model->notes(row),
									m_model->nWeeks(row), m_model->split(row), m_model->coach(row), m_model->client(row),
									m_model->file(row), m_model->type(row), m_model->realMeso(row)));
		}
		ok = query.exec(strQuery);
		if (ok)
		{
			if (!bUpdate)
				m_model->setId(row, query.lastInsertId().toString());
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
