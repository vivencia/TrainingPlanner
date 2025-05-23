#include "dbmesocyclestable.h"

#include "dbmesocyclesmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>
#include <QTime>

DBMesocyclesTable::DBMesocyclesTable(DBMesocyclesModel *model)
	: TPDatabaseTable{MESOCYCLES_TABLE_ID}, m_model{model}
{
	m_tableName = std::move("mesocycles_table"_L1);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_meso_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName("MesocyclesTable");
	#endif
}

void DBMesocyclesTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS mesocycles_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_name TEXT,"
										"meso_start_date INTEGER,"
										"meso_end_date INTEGER,"
										"meso_note TEXT,"
										"meso_nweeks INTEGER,"
										"meso_split TEXT,"
										"meso_coach INTEGER,"
										"meso_client INTEGER,"
										"meso_program_file TEXT,"
										"meso_type TEXT,"
										"real_meso INTEGER"
									")"_L1
		};
		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBMesocyclesTable::getAllMesocycles()
{
	if (openDatabase(true))
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT * FROM mesocycles_table ORDER BY ROWID"_L1};

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
							"meso_note=\'%4\', meso_nweeks=%5, meso_split=\'%6\', meso_coach=%7, meso_client=%8, "
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
							" VALUES(\'%1\', %2, %3, \'%4\', %5, \'%6\', %7, %8, \'%9\', \'%10\', %11)"_s
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
