#include "dbmesocylestable.h"
#include "dbmesocyclesmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QFile>

DBMesocyclesTable::DBMesocyclesTable(const QString& dbFilePath, QSettings* appSettings, DBMesocyclesModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	m_tableName = u"mesocycles_table"_qs;
	m_tableID = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString cnx_name(QStringLiteral("db_meso_connection") + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), cnx_name);
	const QString dbname(dbFilePath + DBMesocyclesFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesocyclesTable::createTable()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"CREATE TABLE IF NOT EXISTS mesocycles_table ("
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
									")"
								)
		);
		m_result = query.exec();
		mSqlLiteDB.close();
	}
	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable createTable SUCCESS")
}

void DBMesocyclesTable::updateDatabase()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		QStringList oldTableInfo;
		query.setForwardOnly(true);
		if (query.exec( QStringLiteral("SELECT * FROM mesocycles_table")))
		{
			if (query.first ())
			{
				do {
					oldTableInfo << query.value(MESOCYCLES_COL_ID).toString() << query.value(MESOCYCLES_COL_NAME).toString() <<
							query.value(MESOCYCLES_COL_STARTDATE).toString() << query.value(MESOCYCLES_COL_ENDDATE).toString() <<
							query.value(MESOCYCLES_COL_NOTE).toString() << query.value(MESOCYCLES_COL_WEEKS).toString() <<
							query.value(MESOCYCLES_COL_SPLIT).toString() << QString() << QString() << QString() << QString() <<
							(query.value(MESOCYCLES_COL_ENDDATE).toUInt() != 0 ? u"1"_qs : u"0"_qs);
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
				if (mSqlLiteDB.open())
				{
					query.exec(QStringLiteral("PRAGMA page_size = 4096"));
					query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
					query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
					query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
					query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
					query.exec(QStringLiteral("PRAGMA synchronous = 0"));
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
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::getAllMesocycles()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM mesocycles_table") );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList meso_info;
				uint i(0);
				do
				{
					for (i = MESOCYCLES_COL_ID; i < MESOCYCLES_TOTAL_COLS; ++i)
						meso_info.append(query.value(static_cast<int>(i)).toString());
					meso_info.append(query.value(MESOCYCLES_COL_ENDDATE).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //MESOCYCLES_COL_REALMESO
					m_model->appendList(meso_info);
					meso_info.clear();
				} while ( query.next () );
				m_result = true;
			}
		}
		if (!m_result)
		{
			MSG_OUT("DBMesocyclesTable getAllMesocycles Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesocyclesTable getAllMesocycles Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBMesocyclesTable getAllMesocycles SUCCESS")
		mSqlLiteDB.close();	
	}
}

void DBMesocyclesTable::saveMesocycle()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		const uint row(m_execArgs.at(0).toUInt());
		bool bUpdate(false);
		QString strQuery;
		if (query.exec(QStringLiteral("SELECT id FROM mesocycles_table WHERE id=%1").arg(m_model->getFast(row, MESOCYCLES_COL_ID))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		m_model->setFast(row, MESOCYCLES_COL_REALMESO,
			m_model->getFast(row, MESOCYCLES_COL_ENDDATE) != QStringLiteral("0") ? QStringLiteral("1") : QStringLiteral("0"));

		if (bUpdate)
		{
			strQuery = QStringLiteral(
							"UPDATE mesocycles_table SET meso_name=\'%1\', meso_start_date=%2, meso_end_date=%3, "
							"meso_note=\'%4\', meso_nweeks=%5, meso_split=\'%6\', meso_coach=\'%7\', meso_client=\'%8\', "
							"meso_program_file=\'%9\', meso_type=\'%10\', real_meso=\'%11\' WHERE id=%12")
								.arg(m_model->getFast(row, MESOCYCLES_COL_NAME), m_model->getFast(row, MESOCYCLES_COL_STARTDATE), m_model->getFast(row, MESOCYCLES_COL_ENDDATE),
									m_model->getFast(row, MESOCYCLES_COL_NOTE), m_model->getFast(row, MESOCYCLES_COL_WEEKS), m_model->getFast(row, MESOCYCLES_COL_SPLIT),
									m_model->getFast(row, MESOCYCLES_COL_COACH), m_model->getFast(row, MESOCYCLES_COL_CLIENT),
									m_model->getFast(row, MESOCYCLES_COL_FILE), m_model->getFast(row, MESOCYCLES_COL_TYPE),
									m_model->getFast(row, MESOCYCLES_COL_REALMESO), m_model->getFast(row, MESOCYCLES_COL_ID));
		}
		else
		{
			strQuery = QStringLiteral(
							"INSERT INTO mesocycles_table "
							"(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,"
							"meso_coach,meso_client,meso_program_file,meso_type,real_meso)"
							" VALUES(\'%1\', %2, %3, \'%4\', %5, \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', %11)")
								.arg(m_model->getFast(row, MESOCYCLES_COL_NAME), m_model->getFast(row, MESOCYCLES_COL_STARTDATE),
									m_model->getFast(row, MESOCYCLES_COL_ENDDATE), m_model->getFast(row, MESOCYCLES_COL_NOTE),
									m_model->getFast(row, MESOCYCLES_COL_WEEKS), m_model->getFast(row, MESOCYCLES_COL_SPLIT),
									m_model->getFast(row, MESOCYCLES_COL_COACH), m_model->getFast(row, MESOCYCLES_COL_CLIENT),
									m_model->getFast(row, MESOCYCLES_COL_FILE), m_model->getFast(row, MESOCYCLES_COL_TYPE),
									m_model->getFast(row, MESOCYCLES_COL_REALMESO));
		}
		m_result = query.exec(strQuery);
		if (m_result)
		{
			m_model->setModified(false);
			if (!bUpdate)
			{
				m_model->setFast(row, MESOCYCLES_COL_ID, query.lastInsertId().toString());
				m_opcode = OP_ADD;
			}
			MSG_OUT("DBMesocyclesTable saveMesocycle SUCCESS")
			MSG_OUT(strQuery);
		}
		else
		{
			MSG_OUT("DBMesocyclesTable saveMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesocyclesTable saveMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT("--ERROR--");
			MSG_OUT(strQuery);
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBMesocyclesTable saveMesocycle Could not open Database")

	doneFunc(static_cast<TPDatabaseTable*>(this));
}
