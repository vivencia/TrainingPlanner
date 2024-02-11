#include "dbmesocylestable.h"

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
									"CREATE TABLE IF NOT EXISTS mesocycles ("
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
		query.prepare( QStringLiteral("SELECT * FROM mesocycles") );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList meso_info;

				const uint n_entries(7);
				uint i(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						meso_info.append(query.value(static_cast<int>(i)).toString());
					meso_info.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
					m_model->appendList(meso_info);
					meso_info.clear();
				} while ( query.next () );
				m_opcode = OP_READ;
				m_result = true;
				resultFunc(static_cast<TPDatabaseTable*>(this));
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getAllMesocycles Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getAllMesocycles Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getAllMesocycles SUCCESS")
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
		query.prepare( QStringLiteral("SELECT * FROM mesocycles WHERE meso_id=") + QString::number(meso_id) );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList meso_info;
				const uint n_entries(7);
				for (uint i(0); i < n_entries; ++i)
					meso_info.append(query.value(static_cast<int>(i)).toString());
				meso_info.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
				m_model->appendList(meso_info);
				m_opcode = OP_READ;
				m_result = true;
				mSqlLiteDB.close();
				resultFunc(static_cast<TPDatabaseTable*>(this));
			}
		}
	}

	if (!m_result)
	{
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesocyclesTable getMesoInfo meso_id " << meso_id << "  SUCCESS")
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
		query.prepare( QStringLiteral("SELECT * FROM mesocycles WHERE meso_id<") + QString::number(current_meso_id));
		if (query.exec())
		{
			if (query.last ())
			{
				QStringList meso_info;
				//Get all the info, even if the caller will not need them. Sometimes it will be used, other not, but I want only one function for this command
				const uint n_entries(7);
				for (uint i(0); i < n_entries; ++i)
					meso_info.append(query.value(static_cast<int>(i)).toString());
				meso_info.append(query.value(3).toInt() != 0 ? QStringLiteral("1") : QStringLiteral("0")); //realMeso? 3 = mesoEndDate
				m_model->appendList(meso_info);
				m_opcode = OP_READ;
				m_result = true;
				resultFunc(static_cast<TPDatabaseTable*>(this));
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
		query.prepare( QStringLiteral("SELECT meso_id,meso_start_date FROM mesocycles WHERE meso_id>=") + QString::number(meso_id));
		if (query.exec())
		{
			if (query.last ())
			{
				QStringList meso_info;
				meso_info.append("");
				meso_info.append("");

				//This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
				if (query.value(0).toUInt() == meso_id)
				{
					QDate futureDate(QDate::currentDate().addMonths(6));
					meso_info.append(QString::number(futureDate.toStdSysDays()));
				}
				else
					meso_info.append(query.value(1).toString());

				m_model->appendList(meso_info);
				m_opcode = OP_READ;
				m_result = true;
				resultFunc(static_cast<TPDatabaseTable*>(this));
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
		query.prepare( QStringLiteral("SELECT meso_end_date,MAX(meso_id) FROM mesocycles"));
		if (query.exec())
		{
			if (query.first ())
			{
				QStringList meso_info;
				meso_info.append("");
				meso_info.append("");
				meso_info.append("");
				meso_info.append(query.value(0).toString());
				m_model->appendList(meso_info);
				m_opcode = OP_READ;
				m_result = true;
				resultFunc(static_cast<TPDatabaseTable*>(this));
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
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
