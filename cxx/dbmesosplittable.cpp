#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <random>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	m_tableName = u"mesocycles_splits"_qs;
	setObjectName(DBMesoSplitObjectName);
	m_UniqueID = QString::number(dist(gen)).remove(0, 2).toUInt();
	const QString cnx_name(QStringLiteral("db_mesosplit_connection-") + QString::number(dist(gen)));
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBMesoSplitFileName );
	mSqlLiteDB.setDatabaseName( dbname );
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
										"splitA_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitA_exercisesset_reps TEXT DEFAULT \"\", "
										"splitA_exercisesset_weight TEXT DEFAULT \"\", "
										"splitA_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitA_exercisesset_notes TEXT DEFAULT \"\", "
										"splitB_exercisesnames TEXT DEFAULT \"\", "
										"splitB_exercisesset_types TEXT DEFAULT \"\", "
										"splitB_exercisesset_n TEXT DEFAULT \"\", "
										"splitB_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitB_exercisesset_reps TEXT DEFAULT \"\", "
										"splitB_exercisesset_weight TEXT DEFAULT \"\", "
										"splitB_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitB_exercisesset_notes TEXT DEFAULT \"\", "
										"splitC_exercisesnames TEXT DEFAULT \"\", "
										"splitC_exercisesset_types TEXT DEFAULT \"\", "
										"splitC_exercisesset_n TEXT DEFAULT \"\", "
										"splitC_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitC_exercisesset_reps TEXT DEFAULT \"\", "
										"splitC_exercisesset_weight TEXT DEFAULT \"\", "
										"splitC_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitC_exercisesset_notes TEXT DEFAULT \"\", "
										"splitD_exercisesnames TEXT DEFAULT \"\", "
										"splitD_exercisesset_types TEXT DEFAULT \"\", "
										"splitD_exercisesset_n TEXT DEFAULT \"\", "
										"splitD_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitD_exercisesset_reps TEXT DEFAULT \"\", "
										"splitD_exercisesset_weight TEXT DEFAULT \"\", "
										"splitD_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitD_exercisesset_notes TEXT DEFAULT \"\", "
										"splitE_exercisesnames TEXT DEFAULT \"\", "
										"splitE_exercisesset_types TEXT DEFAULT \"\", "
										"splitE_exercisesset_n TEXT DEFAULT \"\", "
										"splitE_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitE_exercisesset_reps TEXT DEFAULT \"\", "
										"splitE_exercisesset_weight TEXT DEFAULT \"\", "
										"splitE_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitE_exercisesset_notes TEXT DEFAULT \"\", "
										"splitF_exercisesnames TEXT DEFAULT \"\", "
										"splitF_exercisesset_types TEXT DEFAULT \"\", "
										"splitF_exercisesset_n TEXT DEFAULT \"\", "
										"splitF_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitF_exercisesset_reps TEXT DEFAULT \"\", "
										"splitF_exercisesset_weight TEXT DEFAULT \"\", "
										"splitF_exercisesset_dropset TEXT DEFAULT \"\", "
										"splitF_exercisesset_notes TEXT DEFAULT \"\")" ));
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
				uint i(0);
				for (i = 0; i < 8; ++i)
					split_info.append(query.value(static_cast<int>(i)).toString());
				m_model->appendList(split_info);
				m_result = true;
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable getMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable getMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable getMesoSplit SUCCESS")

	doneFunc(static_cast<TPDatabaseTable*>(this));
}


void DBMesoSplitTable::newMesoSplit()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		const uint row(m_execArgs.at(0).toUInt());
		query.prepare( QStringLiteral( "INSERT INTO mesocycles_splits "
										"(meso_id, splitA, splitB, splitC, splitD, splitE, splitF)"
										" VALUES(\'%1\', \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\')")
								.arg(m_model->getFast(row, 1), m_model->getFast(row, 2), m_model->getFast(row, 3),
									m_model->getFast(row, 4), m_model->getFast(row, 5), m_model->getFast(row, 6), m_model->getFast(row, 7)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesoSplitTable newMesoSplit SUCCESS")
			m_model->setFast(row, 0, query.lastInsertId().toString());
			m_model->setModified(false);
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable newMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable newMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::updateMesoSplit()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		const uint row(m_execArgs.at(0).toUInt());
		query.prepare( QStringLiteral(
									"UPDATE mesocycles_splits SET splitA=\'%1\', splitB=\'%2\', "
									"splitC=\'%3\', splitD=\'%4\', splitE=\'%5\', splitF=\'%6\' WHERE meso_id=%7")
									.arg(m_model->getFast(row, 2), m_model->getFast(row, 3), m_model->getFast(row, 4), m_model->getFast(row, 5),
										m_model->getFast(row, 6), m_model->getFast(row, 7), m_model->getFast(row, 1)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
		MSG_OUT("DBMesoSplitTable updateMesoSplit SUCCESS")
	else
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable updateMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::getCompleteMesoSplit(const bool bEmitSignal)
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QString mesoId(m_execArgs.at(0).toString());
		const QChar splitLetter(m_execArgs.at(1).toChar());

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		query.prepare(QStringLiteral("SELECT split%1_exercisesnames, split%1_exercisesset_types, split%1_exercisesset_n, "
						"split%1_exercisesset_subsets, split%1_exercisesset_reps, split%1_exercisesset_weight, "
						"split%1_exercisesset_dropset, split%1_exercisesset_notes, split%1 "
						"FROM mesocycles_splits WHERE meso_id=%2").arg(splitLetter).arg(mesoId));

		if (query.exec())
		{
			if (query.first ())
			{
				m_model->clear(); //The model might have been used before, but we want a clean slate now
				static_cast<DBMesoSplitModel*>(m_model)->setMuscularGroup(query.value(8).toString());
				static_cast<DBMesoSplitModel*>(m_model)->setSplitLetter(splitLetter);

				const QStringList exercises(query.value(0).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setstypes(query.value(1).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsnumber(query.value(2).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setssubsets(query.value(3).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsreps(query.value(4).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsweight(query.value(5).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsdropset(query.value(6).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsnotes(query.value(7).toString().split(record_separator, Qt::SkipEmptyParts));

				QStringList split_info;
				for(uint i(0); i < setsnumber.count(); ++i)
				{
					split_info.append(exercises.at(i));
					split_info.append(setstypes.at(i));
					split_info.append(setsnumber.at(i));
					split_info.append(setssubsets.at(i));
					split_info.append(setsreps.at(i));
					split_info.append(setsweight.at(i));
					split_info.append(setsdropset.at(i));
					split_info.append(setsnotes.at(i));
					m_model->appendList(split_info);
					split_info.clear();
				}
			}
		}
		mSqlLiteDB.close();
		m_result = true;
		m_model->setReady(true);
		m_model->setModified(false);
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoSplitTable getCompleteMesoSplit SUCCESS")

	if (bEmitSignal)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::updateMesoSplitComplete()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		DBMesoSplitModel* model(static_cast<DBMesoSplitModel*>(m_model));

		QString exercises;
		QString setstypes;
		QString setsnumber;
		QString setssubsets;
		QString setsreps;
		QString setsweight;
		QString setsdropset;
		QString setsnotes;
		for(uint i(0); i < model->count(); ++i)
		{
			exercises += model->getFast(i, MESOSPLIT_COL_EXERCISENAME) + record_separator;
			setstypes += model->getFast(i, MESOSPLIT_COL_SETTYPE) + record_separator;
			setsnumber += model->getFast(i, MESOSPLIT_COL_SETSNUMBER) + record_separator;
			setssubsets += model->getFast(i, MESOSPLIT_COL_SUBSETSNUMBER) + record_separator;
			setsreps += model->getFast(i, MESOSPLIT_COL_REPSNUMBER) + record_separator;
			setsweight += model->getFast(i, MESOSPLIT_COL_WEIGHT) + record_separator;
			setsdropset += model->getFast(i, MESOSPLIT_COL_DROPSET) + record_separator;
			setsnotes += model->getFast(i, MESOSPLIT_COL_NOTES) + record_separator;
		}

		const QString mesoId(m_execArgs.at(0).toString());
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		//Cannot run this query asynchronously because updateMesoSplitComplete() is itself called asynchronously for possibly multiple times in a row
		//I can manage the threads TP creates, but I have no control over what the qsqlite driver will do. Multiple queries will yield multiple
		//calls to write to the database and this was causing some of the queries to fail
		//query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		//query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		const QString strQuery(QStringLiteral("UPDATE mesocycles_splits SET split%1_exercisesnames=\'%2\', "
								"split%1_exercisesset_types=\'%3\', split%1_exercisesset_n=\'%4\', "
								"split%1_exercisesset_subsets=\'%5\', split%1_exercisesset_reps=\'%6\', "
								"split%1_exercisesset_weight=\'%7\', split%1_exercisesset_dropset=\'%8\', "
								"split%1_exercisesset_notes=\'%9\', split%1=\'%10\' WHERE meso_id=%11")
								.arg(model->splitLetter(), exercises, setstypes, setsnumber, setssubsets, setsreps,
										setsweight, setsdropset, setsnotes, model->muscularGroup(), mesoId));
		query.prepare( strQuery );
		m_result = query.exec();
		if (!m_result)
		{
			qDebug() << "Error";
			qDebug() << strQuery;
		}
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		m_model->setModified(false);
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable updateMesoSplitComplete Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
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

void DBMesoSplitTable::convertTDayExercisesToMesoPlan(DBTrainingDayModel* tDayModel)
{
	static_cast<DBMesoSplitModel*>(m_model)->convertFromTDayModel(tDayModel);
	updateMesoSplitComplete();
}
