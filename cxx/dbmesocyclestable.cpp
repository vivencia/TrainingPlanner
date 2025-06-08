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
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS %1 ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_name TEXT,"
										"meso_start_date INTEGER,"
										"meso_end_date INTEGER,"
										"meso_note TEXT,"
										"meso_nweeks INTEGER,"
										"meso_split TEXT,"
										"splitA TEXT,"
										"splitB TEXT,"
										"splitC TEXT,"
										"splitD TEXT,"
										"splitE TEXT,"
										"splitF TEXT,"
										"meso_coach INTEGER,"
										"meso_client INTEGER,"
										"meso_program_file TEXT,"
										"meso_type TEXT,"
										"real_meso INTEGER"
									")"_L1.arg(m_tableName)
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
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"SELECT * FROM %1 ORDER BY ROWID"_L1.arg(m_tableName)};

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
		QSqlQuery query{std::move(getQuery())};
		const uint meso_idx{m_execArgs.at(0).toUInt()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec("SELECT id FROM %1 WHERE id=%2"_L1.arg(m_model->id(meso_idx), m_tableName)))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE %1 SET meso_name=\'%2\', meso_start_date=%3, meso_end_date=%4, "
								"meso_note=\'%5\', meso_nweeks=%6, meso_split=\'%7\', "
								"splitA=\'%8\', splitB=\'%9\', splitC=\'%10\', splitD=\'%11\', splitE=\'%11\', splitF=\'%12\', "
								"meso_coach=%13, meso_client=%14, meso_program_file=\'%15\', meso_type=\'%16\', "
								"real_meso=%17 WHERE id=%18"_s
								.arg(m_tableName, m_model->name(meso_idx), m_model->strStartDate(meso_idx), m_model->strEndDate(meso_idx),
									m_model->notes(meso_idx), m_model->nWeeks(meso_idx), m_model->split(meso_idx),
									m_model->splitA(meso_idx), m_model->splitB(meso_idx), m_model->splitC(meso_idx), m_model->splitD(meso_idx),
									m_model->splitE(meso_idx), m_model->splitF(meso_idx), m_model->coach(meso_idx), m_model->client(meso_idx),
									m_model->file(meso_idx), m_model->type(meso_idx), m_model->realMeso(meso_idx), m_model->id(meso_idx)));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO %1 "
						"(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,"
						"splitA,splitB,splitC,splitD,splitE,splitF,meso_coach,meso_client,meso_program_file,meso_type,real_meso)"
						" VALUES(\'%2\', %3, %4, \'%5\', %6, \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\',"
							"%13, %14, \'%15\', \'%16\', %17)"_s
							.arg(m_tableName, m_model->name(meso_idx), m_model->strStartDate(meso_idx), m_model->strEndDate(meso_idx),
								m_model->notes(meso_idx), m_model->nWeeks(meso_idx), m_model->split(meso_idx),
								m_model->splitA(meso_idx), m_model->splitB(meso_idx), m_model->splitC(meso_idx), m_model->splitD(meso_idx),
								m_model->splitE(meso_idx), m_model->splitF(meso_idx), m_model->coach(meso_idx), m_model->client(meso_idx),
								m_model->file(meso_idx), m_model->type(meso_idx), m_model->realMeso(meso_idx)));
		}
		ok = query.exec(strQuery);
		if (ok)
		{
			if (!bUpdate)
				m_model->setId(meso_idx, query.lastInsertId().toString());
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
