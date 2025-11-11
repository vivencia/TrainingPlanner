#include "dbmesocyclestable.h"

#include "dbmesocyclesmodel.h"
#include "tputils.h"

#include <QThread>

DBMesocyclesTable::DBMesocyclesTable()
	: TPDatabaseTable{MESOCYCLES_TABLE_ID}
{
	setTableName(tableName());
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_meso_connection"_L1 + QString::number(m_uniqueID)};
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	m_sqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName("MesocyclesTable");
	#endif
}

QLatin1StringView DBMesocyclesTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
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
									");"_L1;
}

void DBMesocyclesTable::getAllMesocycles()
{
	if (execQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(tableName()), true, false))
	{
		if (m_workingQuery.first ())
		{
			do
			{
				QStringList meso_info{MESOCYCLES_TOTAL_COLS};
				for (uint i{MESOCYCLES_COL_ID}; i < MESOCYCLES_TOTAL_COLS; ++i)
					meso_info[i] = std::move(m_workingQuery.value(i).toString());
				static_cast<void>(appMesoModel()->newMesocycle(std::move(meso_info)));
			} while (m_workingQuery.next ());
		}
	}
}

void DBMesocyclesTable::saveMesocycle()
{
	const uint meso_idx{m_execArgs.at(0).toUInt()};
	if (execQuery("SELECT id FROM %1 WHERE id=%2;"_L1.arg(tableName(), appMesoModel()->id(meso_idx)), true, false))
	{
		bool update{false};
		if (m_workingQuery.first())
				update = m_workingQuery.value(0).toUInt() >= 0;

		if (update)
		{
			m_strQuery = std::move(u"UPDATE %1 SET meso_name=\'%2\', meso_start_date=%3, meso_end_date=%4, "
				"meso_note=\'%5\', meso_nweeks=%6, meso_split=\'%7\', "
				"splitA=\'%8\', splitB=\'%9\', splitC=\'%10\', splitD=\'%11\', splitE=\'%12\', splitF=\'%13\', "
				"meso_coach=%14, meso_client=%15, meso_program_file=\'%16\', meso_type=\'%17\', "
				"real_meso=%18 WHERE id=%19"_s
				.arg(tableName(), appMesoModel()->name(meso_idx), appMesoModel()->strStartDate(meso_idx), appMesoModel()->strEndDate(meso_idx),
					appMesoModel()->notes(meso_idx), appMesoModel()->nWeeks(meso_idx), appMesoModel()->split(meso_idx),
					appMesoModel()->splitA(meso_idx), appMesoModel()->splitB(meso_idx), appMesoModel()->splitC(meso_idx), appMesoModel()->splitD(meso_idx),
					appMesoModel()->splitE(meso_idx), appMesoModel()->splitF(meso_idx), appMesoModel()->coach(meso_idx), appMesoModel()->client(meso_idx),
					appMesoModel()->file(meso_idx), appMesoModel()->type(meso_idx), appMesoModel()->realMeso(meso_idx), appMesoModel()->id(meso_idx)));
		}
		else
		{
			m_strQuery = std::move(u"INSERT INTO %1 "
				"(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,"
				"splitA,splitB,splitC,splitD,splitE,splitF,meso_coach,meso_client,meso_program_file,meso_type,real_meso)"
				" VALUES(\'%2\', %3, %4, \'%5\', %6, \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\', \'%13\', "
				"%14, %15, \'%16\', \'%17\', %18)"_s
				.arg(tableName(), appMesoModel()->name(meso_idx), appMesoModel()->strStartDate(meso_idx), appMesoModel()->strEndDate(meso_idx),
					appMesoModel()->notes(meso_idx), appMesoModel()->nWeeks(meso_idx), appMesoModel()->split(meso_idx),
					appMesoModel()->splitA(meso_idx), appMesoModel()->splitB(meso_idx), appMesoModel()->splitC(meso_idx), appMesoModel()->splitD(meso_idx),
					appMesoModel()->splitE(meso_idx), appMesoModel()->splitF(meso_idx), appMesoModel()->coach(meso_idx), appMesoModel()->client(meso_idx),
					appMesoModel()->file(meso_idx), appMesoModel()->type(meso_idx), appMesoModel()->realMeso(meso_idx)));
		}
		if (execQuery(m_strQuery, false, false))
		{
			if (!update)
				appMesoModel()->setId(meso_idx, m_workingQuery.lastInsertId().toString());
			emit threadFinished(true);
			return;
		}
	}
	this->thread()->exit();
}
