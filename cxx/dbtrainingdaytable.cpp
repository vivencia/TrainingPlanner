#include "dbtrainingdaytable.h"
#include "dbtrainingdaymodel.h"
#include "runcommands.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <random>

DBTrainingDayTable::DBTrainingDayTable(const QString& dbFilePath, QSettings* appSettings, DBTrainingDayModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	m_tableName = u"training_day_table"_qs;
	setObjectName(DBTrainingDayObjectName);
	const QString cnx_name( QStringLiteral("db_trainingday_connection-") + QString::number(dist(gen)) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBTrainingDayFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(TDAY_EXERCISES_COL_NAMES); i <= TDAY_EXERCISES_COL_COMPLETED; i++)
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
										"setsnotes TEXT DEFAULT \"\","
										"setscompleted TEXT DEFAULT \"\")" ));
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

void DBTrainingDayTable::updateDatabase()
{
	m_result = false;
	QList<QStringList> oldTableInfo;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		QStringList dayInfo;
		query.setForwardOnly(true);
		if (query.exec( QStringLiteral("SELECT * FROM training_day_table")))
		{
			if (query.first ())
			{
				uint nsets(0);
				QString completedStr;
				do {
					dayInfo.append(query.value(TDAY_COL_ID).toString());
					dayInfo.append(query.value(TDAY_COL_MESOID).toString());
					dayInfo.append(query.value(TDAY_COL_DATE).toString());
					dayInfo.append(query.value(TDAY_COL_TRAININGDAYNUMBER).toString());
					dayInfo.append(query.value(TDAY_COL_SPLITLETTER).toString());
					dayInfo.append(query.value(TDAY_COL_TIMEIN).toString());
					dayInfo.append(query.value(TDAY_COL_TIMEOUT).toString());
					dayInfo.append(query.value(TDAY_COL_LOCATION).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_NAMES+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_TYPES+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_RESTTIMES+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_SUBSETS+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_REPS+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_WEIGHTS+1).toString());
					dayInfo.append(query.value(TDAY_COL_NOTES+TDAY_EXERCISES_COL_NOTES+1).toString());
					completedStr = dayInfo.at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_NOTES+1);
					completedStr.replace(' ', '1'); //COMPLETED set to true by default
					dayInfo.append(completedStr);
					oldTableInfo.append(dayInfo);
					dayInfo.clear();
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
					if (mSqlLiteDB.transaction())
					{
						const QString queryStart( QStringLiteral("INSERT INTO training_day_table "
									"(meso_id,date,day_number,split_letter,time_in,time_out,location,notes,"
									"exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
									" VALUES ") );
						const QString queryValuesTemplate("(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\', \'%13\', \'%14\', \'%15\', \'%16\'),");
						QString queryValues;

						for (uint i(0), n(0); i < oldTableInfo.count(); ++i, ++n)
						{
							queryValues += queryValuesTemplate.arg(oldTableInfo.at(i).at(TDAY_COL_MESOID), oldTableInfo.at(i).at(TDAY_COL_DATE), oldTableInfo.at(i).at(TDAY_COL_TRAININGDAYNUMBER),
									oldTableInfo.at(i).at(TDAY_COL_SPLITLETTER), oldTableInfo.at(i).at(TDAY_COL_TIMEIN), oldTableInfo.at(i).at(TDAY_COL_TIMEOUT),
									oldTableInfo.at(i).at(TDAY_COL_LOCATION), oldTableInfo.at(i).at(TDAY_COL_NOTES),
									oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_NAMES+1), oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_TYPES+1),
									oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_RESTTIMES+1), oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_SUBSETS+1),
									oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_REPS+1), oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_WEIGHTS+1),
									oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_NOTES+1), oldTableInfo.at(i).at(TDAY_COL_NOTES+TDAY_EXERCISES_COL_COMPLETED+1));
							if (n == 50)
							{
								queryValues.chop(1);
								query.exec(queryStart + queryValues);
								queryValues.clear();
								n = 0;
							}
						}
						if (!queryValues.isEmpty())
						{
							queryValues.chop(1);
							query.exec(queryStart + queryValues);
						}
						m_result = mSqlLiteDB.commit();
					}
					mSqlLiteDB.close();
				}
			}
		}
	}
	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable updateDatabase Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable updateDatabase Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBTrainingDayTable updateDatabase SUCCESS")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::getTrainingDay()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		query.prepare( QStringLiteral("SELECT id,meso_id,date,day_number,split_letter,time_in,time_out,location,notes "
										"FROM training_day_table WHERE date=") + m_execArgs.at(0).toString() );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList split_info;
				uint i(0);
				for (i = TDAY_COL_ID; i <= TDAY_COL_NOTES; ++i)
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

	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::getTrainingDayExercises(const bool bClearSomeFieldsForReUse)
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		const QString queryCmd(QStringLiteral("SELECT exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted "
						"FROM training_day_table WHERE date=%1 AND meso_id=%2").arg(m_execArgs.at(1).toString(), m_execArgs.at(0).toString()));
		query.prepare( queryCmd );

		if (query.exec())
		{
			if (query.first())
			{
				QStringList workout_info;
				uint i(0);
				for (i = TDAY_EXERCISES_COL_NAMES; i <= TDAY_EXERCISES_COL_COMPLETED; ++i)
					workout_info.append(query.value(static_cast<int>(i)).toString());
				static_cast<DBTrainingDayModel*>(m_model)->fromDataBase(workout_info, bClearSomeFieldsForReUse);
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

	doneFunc(static_cast<TPDatabaseTable*>(this));
}

QString DBTrainingDayTable::formatDate(const uint julianDay) const
{
	const QDate date(QDate::fromJulianDay(julianDay));
	return runCmd()->appLocale()->toString(date, QStringLiteral("ddd d/M/yyyy"));
}

void DBTrainingDayTable::getPreviousTrainingDays()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("SELECT exercises,date FROM training_day_table WHERE meso_id=%1 AND split_letter=\'%2\' AND date<%3 LIMIT 10")
							.arg(m_execArgs.at(0).toString(), m_execArgs.at(1).toString(), m_execArgs.at(2).toString()));

		if (query.exec())
		{
			if (query.last())
			{
				QStringList dates;
				do {
				if (!query.value(0).toString().isEmpty())
					dates.append(formatDate(query.value(1).toUInt()));
				} while (query.previous());
				static_cast<DBTrainingDayModel*>(m_model)->appendList(dates);
			}
		}
		m_opcode = OP_READ;
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable getPreviousTrainingDays Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable getPreviousTrainingDays Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBTrainingDayTable getPreviousTrainingDays SUCCESS")

	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::newTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		DBTrainingDayModel* model(static_cast<DBTrainingDayModel*>(m_model));
		model->getSaveInfo(m_data);

		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		query.prepare( QStringLiteral(
									"INSERT INTO training_day_table "
									"(meso_id,date,day_number,split_letter,time_in,time_out,location,notes,"
									"exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
									" VALUES(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\',"
									"\'%9\', \'%10\', \'%11\', \'%12\', \'%13\', \'%14\', \'%15\', \'%16\')")
									.arg(model->mesoIdStr(), model->dateStr(), model->trainingDay(), model->splitLetter(),
											model->timeIn(), model->timeOut(), model->location(), model->dayNotes(),
											m_data.at(TDAY_EXERCISES_COL_NAMES), m_data.at(TDAY_EXERCISES_COL_TYPES),
											m_data.at(TDAY_EXERCISES_COL_RESTTIMES), m_data.at(TDAY_EXERCISES_COL_SUBSETS),
											m_data.at(TDAY_EXERCISES_COL_REPS), m_data.at(TDAY_EXERCISES_COL_WEIGHTS),
											m_data.at(TDAY_EXERCISES_COL_NOTES), m_data.at(TDAY_EXERCISES_COL_COMPLETED)) );

		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBTrainingDayTable newTrainingDay SUCCESS")
			model->setId(query.lastInsertId().toString());
			m_model->setModified(false);
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBTrainingDayTable newTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable newTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::updateTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		DBTrainingDayModel* model(static_cast<DBTrainingDayModel*>(m_model));
		model->getSaveInfo(m_data);

		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		query.prepare( QStringLiteral(
									"UPDATE training_day_table SET date=%1, day_number=\'%2\', "
									"split_letter=\'%3\', time_in=\'%4\', time_out=\'%5\', location=\'%6\', notes=\'%7\', ")
									.arg(model->dateStr(), model->trainingDay(), model->splitLetter(),
									model->timeIn(), model->timeOut(), model->location(), model->dayNotes()) +
									QStringLiteral("exercises=\'%1\', setstypes=\'%2\', setsresttimes=\'%3\', "
									"setssubsets=\'%4\', setsreps=\'%5\', setsweights=\'%6\', setsnotes=\'%7\', setscompleted=\'%8\' WHERE id=%9")
									.arg(m_data.at(TDAY_EXERCISES_COL_NAMES), m_data.at(TDAY_EXERCISES_COL_TYPES),
											m_data.at(TDAY_EXERCISES_COL_RESTTIMES), m_data.at(TDAY_EXERCISES_COL_SUBSETS),
											m_data.at(TDAY_EXERCISES_COL_REPS), m_data.at(TDAY_EXERCISES_COL_WEIGHTS),
											m_data.at(TDAY_EXERCISES_COL_NOTES), m_data.at(TDAY_EXERCISES_COL_COMPLETED), model->idStr()) );

		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDay SUCCESS")
		m_model->setModified(false);
	}
	else
	{
		MSG_OUT("DBTrainingDayTable updateTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable updateTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::removeTrainingDay()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM training_day_table WHERE date=%1 AND meso_id=%2").arg(
			static_cast<DBTrainingDayModel*>(m_model)->dateStr(), static_cast<DBTrainingDayModel*>(m_model)->mesoIdStr()) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		m_model->clear();
		MSG_OUT("DBTrainingDayTable removeTrainingDay SUCCESS")
	}
	else
	{
		MSG_OUT("DBTrainingDayTable removeTrainingDay Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBTrainingDayTable removeTrainingDay Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
