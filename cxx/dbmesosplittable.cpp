#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <random>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, DBMesoSplitModel* model)
	: TPDatabaseTable(static_cast<TPListModel*>(model))
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	m_tableName = u"mesocycles_splits"_qs;
	m_tableID = MESOSPLIT_TABLE_ID;
	setObjectName(DBMesoSplitObjectName);
	m_UniqueID = QString::number(dist(gen)).remove(0, 2).toUInt();
	const QString cnx_name(u"db_mesosplit_connection-"_qs + QString::number(dist(gen)));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_qs, cnx_name );
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
		const QString strQuery(u"CREATE TABLE IF NOT EXISTS mesocycles_splits ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"splitA TEXT, "
										"splitB TEXT DEFAULT \"\", "
										"splitC TEXT DEFAULT \"\", "
										"splitD TEXT DEFAULT \"\", "
										"splitE TEXT DEFAULT \"\", "
										"splitF TEXT DEFAULT \"\", "
										"splitA_exercisesnames TEXT DEFAULT \"\", "
										"splitA_exercisesset_n TEXT DEFAULT \"\", "
										"splitA_exercisesset_notes TEXT DEFAULT \"\", "
										"splitA_exercisesset_types TEXT DEFAULT \"\", "
										"splitA_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitA_exercisesset_reps TEXT DEFAULT \"\", "
										"splitA_exercisesset_weight TEXT DEFAULT \"\", "
										"splitB_exercisesnames TEXT DEFAULT \"\", "
										"splitB_exercisesset_n TEXT DEFAULT \"\", "
										"splitB_exercisesset_notes TEXT DEFAULT \"\", "
										"splitB_exercisesset_types TEXT DEFAULT \"\", "
										"splitB_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitB_exercisesset_reps TEXT DEFAULT \"\", "
										"splitB_exercisesset_weight TEXT DEFAULT \"\", "
										"splitC_exercisesnames TEXT DEFAULT \"\", "
										"splitC_exercisesset_n TEXT DEFAULT \"\", "
										"splitC_exercisesset_notes TEXT DEFAULT \"\", "
										"splitC_exercisesset_types TEXT DEFAULT \"\", "
										"splitC_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitC_exercisesset_reps TEXT DEFAULT \"\", "
										"splitC_exercisesset_weight TEXT DEFAULT \"\", "
										"splitD_exercisesnames TEXT DEFAULT \"\", "
										"splitD_exercisesset_n TEXT DEFAULT \"\", "
										"splitD_exercisesset_notes TEXT DEFAULT \"\", "
										"splitD_exercisesset_types TEXT DEFAULT \"\", "
										"splitD_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitD_exercisesset_reps TEXT DEFAULT \"\", "
										"splitD_exercisesset_weight TEXT DEFAULT \"\", "
										"splitE_exercisesnames TEXT DEFAULT \"\", "
										"splitE_exercisesset_n TEXT DEFAULT \"\", "
										"splitE_exercisesset_notes TEXT DEFAULT \"\", "
										"splitE_exercisesset_types TEXT DEFAULT \"\", "
										"splitE_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitE_exercisesset_reps TEXT DEFAULT \"\", "
										"splitE_exercisesset_weight TEXT DEFAULT \"\", "
										"splitF_exercisesnames TEXT DEFAULT \"\", "
										"splitF_exercisesset_n TEXT DEFAULT \"\", "
										"splitF_exercisesset_notes TEXT DEFAULT \"\", "
										"splitF_exercisesset_types TEXT DEFAULT \"\", "
										"splitF_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitF_exercisesset_reps TEXT DEFAULT \"\", "
										"splitF_exercisesset_weight TEXT DEFAULT \"\")"_qs);

		m_result = query.exec(strQuery);
		if (!m_result)
		{
			MSG_OUT("DBMesoSplitTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoSplitTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery)
		}
		else
			MSG_OUT("DBMesoSplitTable createTable SUCCESS")
		mSqlLiteDB.close();
	}
}


void DBMesoSplitTable::getAllMesoSplits()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		const QString strQuery(u"SELECT id,meso_id,splitA,splitB,splitC,splitD,splitE,splitF FROM mesocycles_splits"_qs);

		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				uint meso_idx(0);
				do
				{
					if (meso_idx < m_model->count())
					{
						for (uint i(0); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
							m_model->setFast(meso_idx, i, query.value(i).toString());
					}
					++meso_idx;
				} while (query.next ());
				m_result = true;
			}
		}
		if (!m_result)
		{
			MSG_OUT("DBMesoSplitTable getAllMesoSplits Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoSplitTable getAllMesoSplits Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBMesoSplitTable getAllMesoSplits SUCCESS")
		MSG_OUT(strQuery);
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBMesoSplitTable getAllMesoSplits ERROR: Could not open database")
}


void DBMesoSplitTable::saveMesoSplit()
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

		query.prepare( QStringLiteral("SELECT id FROM mesocycles_splits WHERE meso_id=%1").arg(m_model->getFast(row, 1)) );
		if (query.exec())
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (!bUpdate)
		{
			query.prepare( QStringLiteral("INSERT INTO mesocycles_splits "
								"(meso_id, splitA, splitB, splitC, splitD, splitE, splitF)"
								" VALUES(\'%1\', \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\')")
									.arg(m_model->getFast(row, 1), m_model->getFast(row, 2), m_model->getFast(row, 3),
										m_model->getFast(row, 4), m_model->getFast(row, 5), m_model->getFast(row, 6), m_model->getFast(row, 7)) );
		}
		else
		{
			query.prepare( QStringLiteral(
									"UPDATE mesocycles_splits SET splitA=\'%1\', splitB=\'%2\', "
									"splitC=\'%3\', splitD=\'%4\', splitE=\'%5\', splitF=\'%6\' WHERE meso_id=%7")
									.arg(m_model->getFast(row, 2), m_model->getFast(row, 3), m_model->getFast(row, 4), m_model->getFast(row, 5),
										m_model->getFast(row, 6), m_model->getFast(row, 7), m_model->getFast(row, 1)) );
		}
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesoSplitTable saveMesoSplit SUCCESS")
			m_model->setFast(row, 0, query.lastInsertId().toString());
			m_model->setModified(false);
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoSplitTable saveMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoSplitTable saveMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
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
		static_cast<DBMesoSplitModel*>(m_model)->setSplitLetter(splitLetter); //set the main property right away

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		const QString strQuery(QStringLiteral("SELECT split%1_exercisesnames, split%1_exercisesset_n, split%1_exercisesset_notes, "
						"split%1_exercisesset_types, split%1_exercisesset_subsets, split%1_exercisesset_reps, "
						"split%1_exercisesset_weight, split%1 FROM mesocycles_splits WHERE meso_id=%2").arg(splitLetter).arg(mesoId));

		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				m_model->clear(); //The model might have been used before, but we want a clean slate now
				static_cast<DBMesoSplitModel*>(m_model)->setMuscularGroup(query.value(7).toString());

				const QStringList exercises(query.value(MESOSPLIT_COL_EXERCISENAME).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsnumber(query.value(MESOSPLIT_COL_SETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsnotes(query.value(MESOSPLIT_COL_NOTES).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setstypes(query.value(MESOSPLIT_COL_SETTYPE).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setssubsets(query.value(MESOSPLIT_COL_SUBSETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsreps(query.value(MESOSPLIT_COL_REPSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList setsweight(query.value(MESOSPLIT_COL_WEIGHT).toString().split(record_separator, Qt::SkipEmptyParts));

				QStringList split_info;
				for(uint i(0); i < exercises.count(); ++i)
				{
					split_info.append(exercises.at(i));
					split_info.append(setsnumber.at(i));
					split_info.append(setsnotes.at(i));
					split_info.append(setstypes.at(i));
					split_info.append(setssubsets.at(i));
					split_info.append(setsreps.at(i));
					split_info.append(setsweight.at(i));
					split_info.append(u"0"_qs); //MESOSPLIT_COL_WORKINGSET
					m_model->appendList(split_info);
					split_info.clear();
				}
				m_result = true;
				m_model->setReady(true);
				m_model->setModified(false);
			}
		}

		if (m_result)
		{
			MSG_OUT("DBMesoSplitTable getCompleteMesoSplit SUCCESS")
			MSG_OUT(strQuery)
		}
		else
		{
			MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoSplitTable getCompleteMesoSplit Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT("--ERROR--")
			MSG_OUT(strQuery)
		}
		mSqlLiteDB.close();
	}
	if (bEmitSignal)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::saveMesoSplitComplete()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		DBMesoSplitModel* model(static_cast<DBMesoSplitModel*>(m_model));

		QString exercises;
		QString setsnumber;
		QString setsnotes;
		QString setstypes;
		QString setssubsets;
		QString setsreps;
		QString setsweight;
		for(uint i(0); i < model->count(); ++i)
		{
			exercises += model->getFast(i, MESOSPLIT_COL_EXERCISENAME) + record_separator;
			setsnumber += model->getFast(i, MESOSPLIT_COL_SETSNUMBER) + record_separator;
			setsnotes += model->getFast(i, MESOSPLIT_COL_NOTES) + record_separator;
			setstypes += model->getFast(i, MESOSPLIT_COL_SETTYPE) + record_separator;
			setssubsets += model->getFast(i, MESOSPLIT_COL_SUBSETSNUMBER) + record_separator;
			setsreps += model->getFast(i, MESOSPLIT_COL_REPSNUMBER) + record_separator;
			setsweight += model->getFast(i, MESOSPLIT_COL_WEIGHT) + record_separator;
		}

		const QString mesoId(m_execArgs.at(0).toString());
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		bool bUpdate(false);
		QString strQuery;

		if (query.exec(QStringLiteral("SELECT id FROM mesocycles_splits WHERE meso_id=%1").arg(mesoId)))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = QStringLiteral("UPDATE mesocycles_splits SET split%1_exercisesnames=\'%2\', "
								"split%1_exercisesset_n=\'%3\', split%1_exercisesset_notes=\'%4\', "
								"split%1_exercisesset_types=\'%5\', split%1_exercisesset_subsets=\'%6\', "
								"split%1_exercisesset_reps=\'%7\', split%1_exercisesset_weight=\'%8\', "
								"split%1=\'%9\' WHERE meso_id=%10")
								.arg(model->splitLetter(), exercises, setsnumber, setsnotes, setstypes, setssubsets,
										setsreps, setsweight, model->muscularGroup(), mesoId);
		}
		else
		{
			strQuery = QStringLiteral("INSERT INTO mesocycles_splits "
								"(meso_id, split%1_exercisesnames, split%1_exercisesset_n, split%1_exercisesset_notes, "
								"split%1_exercisesset_types, split%1_exercisesset_subsets, split%1_exercisesset_reps, "
								"split%1_exercisesset_weight, split%1)"
								" VALUES(\'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\')")
								.arg(model->splitLetter(), mesoId, exercises, setsnumber, setsnotes, setstypes, setssubsets,
										setsreps, setsweight, model->muscularGroup());
		}
		m_result = query.exec(strQuery);
		if (m_result)
		{
			model->setModified(false);
			MSG_OUT("DBMesoSplitTable saveMesoSplitComplete SUCCESS")
			MSG_OUT(strQuery)
		}
		else
		{
			MSG_OUT("DBMesoSplitTable saveMesoSplitComplete Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoSplitTable saveMesoSplitComplete Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT("--ERROR--")
			MSG_OUT(strQuery)
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBMesoSplitTable saveMesoSplitComplete Could not open Database")
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

void DBMesoSplitTable::convertTDayExercisesToMesoPlan(const DBTrainingDayModel* const tDayModel)
{
	static_cast<DBMesoSplitModel*>(m_model)->convertFromTDayModel(tDayModel);
	saveMesoSplitComplete();
}
