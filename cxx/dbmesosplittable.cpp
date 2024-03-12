#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <random>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model)), mb_emitNow(true)
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	setObjectName( DBMesoSplitObjectName );
	const QString cnx_name( QStringLiteral("db_mesosplit_connection-") + QString::number(dist(gen)) );
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
										"splitA TEXT, "
										"splitB TEXT DEFAULT \"\", "
										"splitC TEXT DEFAULT \"\", "
										"splitD TEXT DEFAULT \"\", "
										"splitE TEXT DEFAULT \"\", "
										"splitF TEXT DEFAULT \"\", "
										"splitA_exercisesnames TEXT DEFAULT \"\", "
										"splitA_exercisesset_types TEXT DEFAULT \"\", "
										"splitA_exercisesset_n TEXT DEFAULT \"\", "
										"splitA_exercisesset_reps TEXT DEFAULT \"\", "
										"splitA_exercisesset_weight TEXT DEFAULT \"\", "
										"splitB_exercisesnames TEXT DEFAULT \"\", "
										"splitB_exercisesset_types TEXT DEFAULT \"\", "
										"splitB_exercisesset_n TEXT DEFAULT \"\", "
										"splitB_exercisesset_reps TEXT DEFAULT \"\", "
										"splitB_exercisesset_weight TEXT DEFAULT \"\", "
										"splitC_exercisesnames TEXT DEFAULT \"\", "
										"splitC_exercisesset_types TEXT DEFAULT \"\", "
										"splitC_exercisesset_n TEXT DEFAULT \"\", "
										"splitC_exercisesset_reps TEXT DEFAULT \"\", "
										"splitC_exercisesset_weight TEXT DEFAULT \"\", "
										"splitD_exercisesnames TEXT DEFAULT \"\", "
										"splitD_exercisesset_types TEXT DEFAULT \"\", "
										"splitD_exercisesset_n TEXT DEFAULT \"\", "
										"splitD_exercisesset_reps TEXT DEFAULT \"\", "
										"splitD_exercisesset_weight TEXT DEFAULT \"\", "
										"splitE_exercisesnames TEXT DEFAULT \"\", "
										"splitE_exercisesset_types TEXT DEFAULT \"\", "
										"splitE_exercisesset_n TEXT DEFAULT \"\", "
										"splitE_exercisesset_reps TEXT DEFAULT \"\", "
										"splitE_exercisesset_weight TEXT DEFAULT \"\", "
										"splitF_exercisesnames TEXT DEFAULT \"\", "
										"splitF_exercisesset_types TEXT DEFAULT \"\", "
										"splitF_exercisesset_n TEXT DEFAULT \"\", "
										"splitF_exercisesset_reps TEXT DEFAULT \"\", "
										"splitF_exercisesset_weight TEXT DEFAULT \"\")" ));
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
		const QString mesoId(QString::number(m_execArgs.at(0).toUInt()));
		const QChar splitLetter(m_execArgs.at(1).toChar());

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT split%1_exercisesnames, split%1_exercisesset_types, split%1_exercisesset_n,"
				"split%1_exercisesset_reps, split%1_exercisesset_weight FROM mesocycles_splits WHERE meso_id=%2").arg(splitLetter).arg(mesoId) );

		if (query.exec())
		{
			if (query.first ())
			{
				const QStringList exercises(query.value(0).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setstypes(query.value(1).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsnumber(query.value(2).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsreps(query.value(3).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsweight(query.value(4).toString().split(record_separator, Qt::SkipEmptyParts));

				QStringList split_info;
				for(uint i(0); i < exercises.count(); ++i)
				{
					split_info.append(exercises.at(i));
					split_info.append(setstypes.at(i));
					split_info.append(setsnumber.at(i));
					split_info.append(setsreps.at(i));
					split_info.append(setsweight.at(i));
					m_model->appendList(split_info);
					split_info.clear();
				}
			}
		}
		mSqlLiteDB.close();
		m_result = true;
		m_model->setReady(true);
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit SUCCESS")

	if (mb_emitNow)
	{
		resultFunc(static_cast<TPDatabaseTable*>(this));
		doneFunc(static_cast<TPDatabaseTable*>(this));
	}
}

void DBMesoSplitTable::updateMesoSplitComplete()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QString mesoId(QString::number(m_execArgs.at(0).toUInt()));
		const QString splitLetter(m_execArgs.at(1).toString());

		QString exercises;
		QString setstypes;
		QString setsnumber;
		QString setsreps;
		QString setsweight;
		for(uint i(0); i < m_model->count(); ++i)
		{
			exercises += m_model->getFast(i, 0) + record_separator;
			setstypes += m_model->getFast(i, 1) + record_separator;
			setsnumber += m_model->getFast(i, 2) + record_separator;
			setsreps += m_model->getFast(i, 3) + record_separator;
			setsweight += m_model->getFast(i, 4) + record_separator;
		}

		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("UPDATE mesocycles_splits SET split%1_exercisesnames=\'%2\',"
									 "split%1_exercisesset_types=\'%3\', split%1_exercisesset_n=\'%4\',"
									"split%1_exercisesset_reps=\'%5\', split%1_exercisesset_weight=\'%6\' WHERE meso_id=%7")
									.arg(splitLetter, exercises, setstypes, setsnumber, setsreps, setsweight, mesoId) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		static_cast<DBMesoSplitModel*>(m_model)->setSplitModified(false);
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

bool DBMesoSplitTable::mesoHasPlan(const QString& mesoId, const QString& splitLetter)
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		query.prepare( QStringLiteral("SELECT split%1_exercisesnames FROM mesocycles_splits WHERE meso_id=%2")
									.arg(splitLetter, mesoId) );
		m_result = query.exec();
		if (m_result)
		{
			m_result = query.first();
			if (m_result)
				m_result = query.value(0).toString().length() > 0;
		}
		mSqlLiteDB.close();
	}
	return m_result;
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
