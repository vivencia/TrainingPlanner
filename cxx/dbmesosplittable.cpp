#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBMesoSplitObjectName );
	const QString cnx_name( QStringLiteral("db_worker_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBMesoSplitFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < 8; i++)
		m_data.append(QString());
}

void DBMesoSplitTable::createTable()
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
									"CREATE TABLE IF NOT EXISTS mesocycles_splits ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"splitA TEXT,"
										"splitB TEXT,"
										"splitC TEXT,"
										"splitD TEXT,"
										"splitE TEXT,"
										"splitF TEXT,"
										"splitA_exercisesnames TEXT,"
										"splitA_exercisesset_types TEXT,"
										"splitA_exercisesset_n TEXT,"
										"splitA_exercisesset_reps TEXT,"
										"splitA_exercisesset_weight TEXT,"
										"splitB_exercisesnames TEXT,"
										"splitB_exercisesset_types TEXT,"
										"splitB_exercisesset_n TEXT,"
										"splitB_exercisesset_reps TEXT,"
										"splitB_exercisesset_weight TEXT,"
										"splitC_exercisesnames TEXT,"
										"splitC_exercisesset_types TEXT,"
										"splitC_exercisesset_n TEXT,"
										"splitC_exercisesset_reps TEXT,"
										"splitC_exercisesset_weight TEXT,"
										"splitD_exercisesnames TEXT,"
										"splitD_exercisesset_types TEXT,"
										"splitD_exercisesset_n TEXT,"
										"splitD_exercisesset_reps TEXT,"
										"splitD_exercisesset_weight TEXT,"
										"splitE_exercisesnames TEXT,"
										"splitE_exercisesset_types TEXT,"
										"splitE_exercisesset_n TEXT,"
										"splitE_exercisesset_reps TEXT,"
										"splitE_exercisesset_weight TEXT,"
										"splitF_exercisesnames TEXT,"
										"splitF_exercisesset_types TEXT,"
										"splitF_exercisesset_n TEXT,"
										"splitF_exercisesset_reps TEXT,"
										"splitF_exercisesset_weight TEXT"
									")"
								)
		);
		m_result = query.exec();
		mSqlLiteDB.close();
	}
	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable createTable SUCCESS")
}


void DBMesoSplitTable::getMesoSplit()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM mesocycles_splits") );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				const uint n_entries(8);
				uint i(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						split_info.append(query.value(static_cast<int>(i)).toString());
					m_model->appendList(split_info);
					split_info.clear();
				} while ( query.next () );
			}
		}
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable getMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable getMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable getMesoSplit SUCCESS")

	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}


void DBMesoSplitTable::newMesoSplit()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"INSERT INTO mesocycles_splits "
									"(meso_id, splitA, splitB, splitC, splitD, splitE, splitF)"
									" VALUES(\'%1\', \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\')")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesoSplitTable newMesoSplit SUCCESS")
			m_data[0] = query.lastInsertId().toString();
			if (m_model)
				m_model->appendList(m_data);
			m_opcode = OP_ADD;
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable newMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable newMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::updateMesoSplit()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"UPDATE mesocycles_splits SET splitA=\'%1\', splitB=\'%2\', "
									"splitC=\'%3\', splitD=\'%4\', splitE=\'%5\', splitF=\'%6\' WHERE meso_id=%1")
									.arg(m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
										m_data.at(6), m_data.at(7), m_data.at(1)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplit SUCCESS")
		if (m_model)
			m_model->updateList(m_data, m_model->currentRow());
	}
	else
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable updateMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::removeMesoSplit()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM mesocycles_splits WHERE meso_id=") + m_data.at(1) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		if (m_model)
			m_model->removeFromList(m_model->currentRow());
		MSG_OUT("DBMesoSplitTable removeMesoSplit SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoSplitTable removeMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable removeMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::setData(const QString& mesoId, const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	m_data[0] = "";
	m_data[1] = mesoId;
	m_data[2] = splitA;
	m_data[3] = splitB;
	m_data[4] = splitC;
	m_data[5] = splitD;
	m_data[6] = splitE;
	m_data[7] = splitF;
}
