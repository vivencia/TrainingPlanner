#include "dbmesocylestable.h"
#include "dbmesocyclesmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>

DBMesocyclesTable::DBMesocyclesTable(const QString& dbFilePath, QSettings* appSettings, DBMesocyclesModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBMesocyclesObjectName );
	const QString cnx_name( QStringLiteral("db_worker_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBMesocyclesFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < 8; i++)
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

				const uint n_entries(8);
				uint i(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						meso_info.append(query.value(static_cast<int>(i)).toString());
					meso_info.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
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
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::getMesoInfo()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	const uint meso_id (m_execArgs.at(0).toUInt());
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM mesocycles_table WHERE id=") + QString::number(meso_id) );

		if (query.exec())
		{
			if (query.first ())
			{
				m_data.clear();
				const uint n_entries(7);
				for (uint i(0); i < n_entries; ++i)
					m_data.append(query.value(static_cast<int>(i)).toString());
				m_data.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
				m_result = true;
				m_opcode = OP_READ;
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  SUCCESS")
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::getPreviousMesoId()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	const uint current_meso_id (m_execArgs.at(0).toUInt());
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM mesocycles_table WHERE id<") + QString::number(current_meso_id));
		if (query.exec())
		{
			if (query.last ())
			{
				m_data.clear();
				const uint n_entries(8);
				for (uint i(0); i < n_entries; ++i)
					m_data.append(query.value(static_cast<int>(i)).toString());
				m_data.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
				m_opcode = OP_READ;
				m_result = true;
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getPreviousMesoId current_meso_id " << current_meso_id << "  Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getPreviousMesoId current_meso_id " << current_meso_id << "  Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getPreviousMesoId current_meso_id " << current_meso_id << "  SUCCESS")
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::getNextMesoStartDate()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	const uint meso_id (m_execArgs.at(0).toUInt());
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT id,meso_start_date FROM mesocycles_table WHERE id>=") + QString::number(meso_id));
		if (query.exec())
		{
			if (query.last ())
			{
				m_data.clear();
				m_data.append("");
				m_data.append("");

				//This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
				if (query.value(0).toUInt() == meso_id)
				{
					QDate futureDate(QDate::currentDate().addMonths(6));
					m_data.append(QString::number(futureDate.toJulianDay()));
				}
				else
					m_data.append(query.value(1).toString());
				m_opcode = OP_READ;
				m_result = true;

			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getNextMesoStartDate current_meso_id " << meso_id << "  Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getNextMesoStartDate current_meso_id " << meso_id << "  Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getNextMesoStartDate current_meso_id " << meso_id << "  SUCCESS")
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::getLastMesoEndDate()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT meso_end_date,MAX(id) FROM mesocycles_table"));
		if (query.exec())
		{
			if (query.first ())
			{
				m_data.clear();
				m_data.append("");
				m_data.append("");
				m_data.append("");
				m_data.append(query.value(0).toString());
				m_opcode = OP_READ;
				m_result = true;
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getLastMesoEndDate Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getLastMesoEndDate Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getLastMesoEndDate SUCCESS")
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
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
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesocyclesTable newMesocycle SUCCESS")
			m_data[0] = query.lastInsertId().toString();
			m_data.append(m_data.at(3) != QStringLiteral("0") ? QStringLiteral("1") : QStringLiteral("0"));
			if (m_model)
				m_model->appendList(m_data);
			m_opcode = OP_ADD;
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable newMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable newMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
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
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7), m_data.at(0)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBMesocyclesTable updateMesocycle SUCCESS")
		if (m_model)
			m_model->updateList(m_data, m_model->currentRow());
	}
	else
	{
		MSG_OUT("DBMesocyclesTable updateMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable updateMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::removeMesocycle()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM mesocycles_table WHERE id=") + m_data.at(0) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		if (m_model)
			m_model->removeFromList(m_model->currentRow());
		MSG_OUT("DBMesocyclesTable removeMesocycle SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesocyclesTable removeMesocycle Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable removeMesocycle Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesocyclesTable::setData(const QString& id, const QString& mesoName, const QString& mesoStartDate,
						const QString& mesoEndDate, const QString& mesoNote, const QString& mesoWeeks,
						const QString& mesoSplit, const QString& mesoDrugs)
{
	m_data[0] = id;
	m_data[1] = mesoName;
	m_data[2] = mesoStartDate;
	m_data[3] = mesoEndDate;
	m_data[4] = mesoNote;
	m_data[5] = mesoWeeks;
	m_data[6] = mesoSplit;
	m_data[7] = mesoDrugs;
}
