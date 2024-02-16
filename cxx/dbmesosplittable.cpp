#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QFile>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBMesoSplitObjectName );
	const QString cnx_name( QStringLiteral("db_mesosplit_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
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
		const QString mesoId(m_execArgs.at(0).toString());

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT id,meso_id,splitA,splitB,splitC,splitD,splitE,splitF FROM mesocycles_splits WHERE meso_id=") + mesoId );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				const uint n_entries(8);
				uint i(0);
				for (i = 0; i < n_entries; ++i)
					split_info.append(query.value(static_cast<int>(i)).toString());
				m_model->appendList(split_info);
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
									"splitC=\'%3\', splitD=\'%4\', splitE=\'%5\', splitF=\'%6\' WHERE meso_id=%7")
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

void DBMesoSplitTable::deleteMesoSplitTable()
{
	QFile mDBFile(mSqlLiteDB.databaseName());
	m_result = mDBFile.remove();
	if (m_result)
	{
		if (m_model)
			m_model->clear();
		m_opcode = OP_DELETE_TABLE;
		MSG_OUT("DBMesoSplitTable deleteMesoSplitTable SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoSplitTable deleteMesoSplitTable error: Could not remove file " << mDBFile.fileName())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::getCompleteMesoSplit()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QString mesoId(m_execArgs.at(0).toString());
		const QLatin1Char splitLetter(static_cast<char>(m_execArgs.at(1).toInt()));

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT id, meso_id, split%1, split%1_exercisesnames, split%1_exercisesset_types, split%1_exercisesset_n,"
				"split%1_exercisesset_reps, split%1_exercisesset_weight FROM mesocycles_splits WHERE meso_id=%2").arg(splitLetter, mesoId) );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				for (uint meso(0); meso < m_model->count(); ++ meso)
				{
					if (m_model->getRow(meso).at(1) == mesoId )
					{
						split_info = m_model->getRow(meso);
						break;
					}
				}

				uint n_entries(8);
				uint i(0);
				if (split_info.isEmpty())
				{
					split_info.reserve(38);
					for (i = 0; i < n_entries; ++i)
						split_info.append(query.value(static_cast<int>(i)).toString());
				}
				else
				{
					for (i = 0; i < n_entries; ++i)
						split_info[i] = query.value(static_cast<int>(i)).toString();
				}

				uint next_i(0);
				switch (splitLetter)
				{
					case 'A': next_i = 8; break;
					case 'B': next_i = 13; break;
					case 'C': next_i = 18; break;
					case 'D': next_i = 23; break;
					case 'E': next_i = 28; break;
					case 'F': next_i = 33; break;
				}
				n_entries = next_i + 5;

				if (split_info.count() == 8)
				{
					for (; i < 38; ++i)
						split_info.append(QString());
				}

				for (i = next_i; i < n_entries; ++i)
					split_info[i] = query.value(static_cast<int>(i)).toString();
			}
		}
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit SUCCESS")

	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::updateMesoSplitComplete()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("UPDATE mesocycles_splits SET split%1=\'%2\', split%1_exercisesnames=\'%3\',""
									 "split%1_exercisesset_types=\'%4\', split%1_exercisesset_n=\'%5\',"
									"split%1_exercisesset_reps=\'%6\', split%1_exercisesset_weight=\'%7\' WHERE meso_id=%8")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5),
									m_data.at(6), m_data.at(7), m_data.at(0)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete SUCCESS")
		if (m_model)
		{
			uint group_pos(0), start(0);
			switch (m_data.at(1).toLatin1().constData()[0])
			{
				case 'A': group_pos = 2; start = 8; break;
				case 'B': group_pos = 3; start = 13; break;
				case 'C': group_pos = 4; start = 18; break;
				case 'D': group_pos = 5; start = 23; break;
				case 'E': group_pos = 6; start = 28; break;
				case 'F': group_pos = 7; start = 33; break;
			}
			QStringList split_info(m_model->getRow(m_model->currentRow()));
			split_info[group_pos] = m_data[2];
			split_info[start] = m_data[3];
			split_info[start+1] = m_data[4];
			split_info[start+2] = m_data[5];
			split_info[start+3] = m_data[6];
			split_info[start+4] = m_data[7];
		}
	}
	else
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Driver error:  " << mSqlLiteDB.lastError().driverText())
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

void DBMesoSplitTable::setDataComplete(const QString& mesoId, QLatin1Char splitLetter, const QString& splitGroup, const QString& exercises,
							const QString& types, const QString& nsets, const QString& nreps, const QString& nweights)
{
	m_data[0] = mesoId;
	m_data[1] = splitLetter;
	m_data[2] = splitGroup;
	m_data[3] = exercises;
	m_data[4] = types;
	m_data[5] = nsets;
	m_data[6] = nreps;
	m_data[7] = nweights;
}
