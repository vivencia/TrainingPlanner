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
	setObjectName(DBMesocyclesObjectName);
	const QString cnx_name( QStringLiteral("db_meso_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBMesocyclesFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_COL_REALMESO; i++)
		m_data.append(QString());
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
										"meso_drugs TEXT"
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
					for (i = MESOCYCLES_COL_ID; i < MESOCYCLES_COL_REALMESO; ++i)
						meso_info.append(query.value(static_cast<int>(i)).toString());
					meso_info.append(query.value(MESOCYCLES_COL_ENDDATE).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //MESOCYCLES_COL_REALMESO
					m_model->appendList(meso_info);
					meso_info.clear();
				} while ( query.next () );
			}
		}
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getAllMesocycles Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getAllMesocycles Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getAllMesocycles SUCCESS")
}

void DBMesocyclesTable::newMesocycle()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"INSERT INTO mesocycles_table "
									"(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,meso_drugs)"
									" VALUES(\'%1\', \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\')")
									.arg(m_data.at(MESOCYCLES_COL_NAME), m_data.at(MESOCYCLES_COL_STARTDATE), m_data.at(MESOCYCLES_COL_ENDDATE),
										m_data.at(MESOCYCLES_COL_NOTE), m_data.at(MESOCYCLES_COL_WEEKS), m_data.at(MESOCYCLES_COL_SPLIT),
										m_data.at(MESOCYCLES_COL_DRUGS)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesocyclesTable newMesocycle SUCCESS")
			m_data[MESOCYCLES_COL_ID] = query.lastInsertId().toString();
			m_data.append(m_data.at(MESOCYCLES_COL_ENDDATE) != QStringLiteral("0") ? QStringLiteral("1") : QStringLiteral("0"));
			m_model->updateList(m_data, m_model->currentRow());
			m_model->setModified(false);
			m_opcode = OP_ADD;
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable newMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable newMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::updateMesocycle()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"UPDATE mesocycles_table SET meso_name=\'%1\', meso_start_date=\'%2\', meso_end_date=\'%3\', "
									"meso_note=\'%4\', meso_nweeks=\'%5\', meso_split=\'%6\', meso_drugs=\'%7\' WHERE id=%8")
									.arg(m_data.at(MESOCYCLES_COL_NAME), m_data.at(MESOCYCLES_COL_STARTDATE), m_data.at(MESOCYCLES_COL_ENDDATE),
										m_data.at(MESOCYCLES_COL_NOTE), m_data.at(MESOCYCLES_COL_WEEKS), m_data.at(MESOCYCLES_COL_SPLIT),
										m_data.at(MESOCYCLES_COL_DRUGS), m_data.at(MESOCYCLES_COL_ID)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBMesocyclesTable updateMesocycle SUCCESS")
		m_model->setModified(false);
	}
	else
	{
		MSG_OUT("DBMesocyclesTable updateMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable updateMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::setData(const QString& id, const QString& mesoName, const QString& mesoStartDate,
						const QString& mesoEndDate, const QString& mesoNote, const QString& mesoWeeks,
						const QString& mesoSplit, const QString& mesoDrugs)
{
	m_data[MESOCYCLES_COL_ID] = id;
	m_data[MESOCYCLES_COL_NAME] = mesoName;
	m_data[MESOCYCLES_COL_STARTDATE] = mesoStartDate;
	m_data[MESOCYCLES_COL_ENDDATE] = mesoEndDate;
	m_data[MESOCYCLES_COL_NOTE] = mesoNote;
	m_data[MESOCYCLES_COL_WEEKS] = mesoWeeks;
	m_data[MESOCYCLES_COL_SPLIT] = mesoSplit;
	m_data[MESOCYCLES_COL_DRUGS] = mesoDrugs;
}
