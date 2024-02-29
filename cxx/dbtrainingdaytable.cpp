#include "dbtrainingdaytable.h"
#include "dbtrainingdaymodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <random>

static const uint n_entries(9);

DBTrainingDayTable::DBTrainingDayTable(const QString& dbFilePath, QSettings* appSettings, DBTrainingDayModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	setObjectName( DBTrainingDayObjectName );
	const QString cnx_name( QStringLiteral("db_trainingday_connection-") + QString::number(dist(gen)) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBTrainingDayFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < n_entries; i++)
		m_data.append(QString());
}

void DBTrainingDayTable::createTable()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		query.prepare( QStringLiteral(
									"CREATE TABLE IF NOT EXISTS training_day_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"date INTEGER,"
										"day_number TEXT,"
										"split_letter TEXT,"
										"time_in TEXT,"
										"time_out TEXT,"
										"location TEXT,"
										"notes TEXT,"
										"exercises TEXT DEFAULT \"\","
										"setstypes TEXT DEFAULT \"\","
										"setsresttimes TEXT DEFAULT \"\","
										"setssubsets TEXT DEFAULT \"\","
										"setsreps TEXT DEFAULT \"\","
										"setsweights TEXT DEFAULT \"\","
										"setsnotes TEXT DEFAULT \"\")" ));
		m_result = query.exec();
		mSqlLiteDB.close();
	}
	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBTrainingDayTable createTable SUCCESS")
}

void DBTrainingDayTable::getTrainingDay()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT id,meso_id,date,day_number,split_letter,time_in,time_out,location,notes "
										"FROM training_day_table WHERE date=") + m_data[2] );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				uint i(0);
				for (i = 0; i < n_entries; ++i)
					split_info.append(query.value(static_cast<int>(i)).toString());
				m_model->appendList(split_info);
			}
		}
		mSqlLiteDB.close();
		m_result = true;
		m_model->setReady(true);
	}

	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable getTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable getTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBTrainingDayTable getTrainingDay SUCCESS")

	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::getTrainingDayExercises()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes "
										"FROM training_day_table WHERE date=") + m_data[2] );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				uint i(0);
				for (i = 0; i < n_entries-1; ++i)
					split_info.append(query.value(static_cast<int>(i)).toString());
				static_cast<DBTrainingDayModel*>(m_model)->fromDataBase(split_info);
			}
		}
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable getTrainingDayExercises Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable getTrainingDayExercises Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBTrainingDayTable getTrainingDayExercises SUCCESS")

	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::newTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"INSERT INTO training_day_table "
									"(meso_id,date,day_number,split_letter,time_in,time_out,location,notes)"
									" VALUES(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\')")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7), m_data.at(8)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBTrainingDayTable newTrainingDay SUCCESS")
			m_data[0] = query.lastInsertId().toString();
			if (m_model)
				m_model->appendList(m_data);
			m_opcode = OP_ADD;
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable newTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable newTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::updateTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"UPDATE training_day_table SET meso_id=%1, date=%2, day_number=\'%3\', "
									"split_letter=\'%4\', time_in=\'%5\', time_out=\'%6\', location=\'%7\', notes=\'%8\' WHERE id=%9")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7), m_data.at(8), m_data.at(0)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDay SUCCESS")
		if (m_model)
			m_model->updateList(m_data, m_model->currentRow());
	}
	else
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable updateTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::updateTrainingDayExercises()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		static_cast<DBTrainingDayModel*>(m_model)->getSaveInfo(m_data);

		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		query.prepare( QStringLiteral(
									"UPDATE training_day_table SET exercises=\'%1\', setstypes=\'%2\', setsresttimes=\'%3\', "
									"setssubsets=\'%4\', setsreps=\'%5\', setsweights=\'%6\', setsnotes=\'%7\' WHERE id=%8")
									.arg(m_data.at(0), m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), QString::number(m_execArgs.at(0).toUInt())) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDayExercises SUCCESS")
		if (m_model)
			m_model->updateList(m_data, m_model->currentRow());
	}
	else
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDayExercises Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable updateTrainingDayExercises Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::removeTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM training_day_table WHERE date=") + m_data.at(2) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		if (m_model)
			m_model->clear();
		MSG_OUT("DBTrainingDayTable removeTrainingDay SUCCESS")
	}
	else
	{
		MSG_OUT("DBTrainingDayTable removeTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable removeTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::deleteTrainingDayTable()
{
	QFile mDBFile(mSqlLiteDB.databaseName());
	m_result = mDBFile.remove();
	if (m_result)
	{
		if (m_model)
			m_model->clear();
		m_opcode = OP_DELETE_TABLE;
		MSG_OUT("DBTrainingDayTable deleteTrainingDayTable SUCCESS")
	}
	else
	{
		MSG_OUT("DBTrainingDayTable deleteTrainingDayTable error: Could not remove file " << mDBFile.fileName())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::setData(const QString& id, const QString& mesoId, const QString& date, const QString& trainingDayNumber,
						const QString& splitLetter, const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes)
{
	m_data[0] = id;
	m_data[1] = mesoId;
	m_data[2] = date;
	m_data[3] = trainingDayNumber;
	m_data[4] = splitLetter;
	m_data[5] = timeIn;
	m_data[6] = timeOut;
	m_data[7] = location;
	m_data[8] = notes;
}
