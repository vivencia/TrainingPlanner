#include "dbtrainingdaytable.h"
#include "dbtrainingdaymodel.h"
#include "tputils.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlQuery>

DBTrainingDayTable::DBTrainingDayTable(const QString &dbFilePath, DBTrainingDayModel *model)
	: TPDatabaseTable{nullptr}, m_model(model)
{
	m_tableName = std::move("training_day_table"_L1);
	m_tableID = TRAININGDAY_TABLE_ID;
	setObjectName(DBTrainingDayObjectName);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name("db_trainingday_connection"_L1 + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString &dbname(dbFilePath + DBTrainingDayFileName);
	mSqlLiteDB.setDatabaseName( dbname );
}

void DBTrainingDayTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS training_day_table ("
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
										"setscompleted TEXT DEFAULT \"\")"_L1
		};
		const bool ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBTrainingDayTable::updateTable()
{
	/*m_result = false;
	QList<QStringList> oldTableInfo;
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		query.exec(u"PRAGMA page_size = 4096"_s);
		query.exec(u"PRAGMA cache_size = 16384"_s);
		query.exec(u"PRAGMA temp_store = MEMORY"_s);
		query.exec(u"PRAGMA journal_mode = OFF"_s);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_s);
		query.exec(u"PRAGMA synchronous = 0"_s);
		query.setForwardOnly(true);

		if (query.exec(u"SELECT * FROM training_day_table"_s))
		{
			if (query.first ())
			{
				uint nsets(0);
				QString completedStr;
				QStringList dayInfo;
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
				if (openDatabase())
				{
					if (mSqlLiteDB.transaction())
					{
						const QString &queryStart( QStringLiteral("INSERT INTO training_day_table "
									"(meso_id,date,day_number,split_letter,time_in,time_out,location,notes,"
									"exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
									" VALUES ") );
						const QString &queryValuesTemplate("(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\', \'%13\', \'%14\', \'%15\', \'%16\'),");
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
					if (!m_result)
					{
						MSG_OUT("DBTrainingDayTable updateDatabase Database error:  " << mSqlLiteDB.lastError().databaseText())
						MSG_OUT("DBTrainingDayTable updateDatabase Driver error:  " << mSqlLiteDB.lastError().driverText())
					}
					else
						MSG_OUT("DBTrainingDayTable updateDatabase SUCCESS")
					mSqlLiteDB.close();
				}
			}
		}
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));*/
}

void DBTrainingDayTable::getTrainingDay()
{
	if (openDatabase(true))
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT id,meso_id,date,day_number,split_letter,time_in,time_out,location,notes "
										"FROM training_day_table WHERE date="_L1 + m_execArgs.at(0).toString()};
		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				QStringList &split_info(m_model->getRow(0));
				for (uint i(TDAY_COL_ID); i < TDAY_TOTAL_COLS; ++i)
					split_info[i] = std::move(query.value(static_cast<int>(i)).toString());
				m_model->setReady(true);
				SUCCESS_MESSAGE_WITH_STATEMENT(DEFINE_SOURCE_LOCATION PRINT_SOURCE_LOCATION)
				mSqlLiteDB.close();
				getTrainingDayExercises();
				return;
			}	
		}
		setQueryResult(false, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::getTrainingDayExercises(const bool bClearSomeFieldsForReUse)
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted "
						"FROM training_day_table WHERE date=%1 AND meso_id=%2"_L1.arg(m_execArgs.at(0).toString(), m_execArgs.at(1).toString())};

		if (query.exec(strQuery))
		{
			if (query.first())
			{
				QStringList workout_info(TDAY_EXERCISES_TOTALCOLS);
				for (uint i(TDAY_EXERCISES_COL_NAMES); i < TDAY_EXERCISES_TOTALCOLS; ++i)
					workout_info[i] = std::move(query.value(static_cast<int>(i)).toString());
				m_model->fromDataBase(workout_info, bClearSomeFieldsForReUse);
				m_model->setReady(true);
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

inline QString DBTrainingDayTable::formatDate(const uint julianDay) const
{
	const QDate &date{QDate::fromJulianDay(julianDay)};
	return appUtils()->appLocale()->toString(date, "ddd d/M/yyyy"_L1);
}

void DBTrainingDayTable::getPreviousTrainingDaysInfo()
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString &mainDate{m_execArgs.at(2).toString()};
		m_model->appendRow();
		m_model->setDateStr(mainDate); //QmlItemManager needs this to know if the received model is the one it's looking for

		QString strQuery{std::move("SELECT exercises,date FROM training_day_table "
							"WHERE meso_id=%1 AND split_letter=\'%2\' AND date<%3 ORDER BY date DESC LIMIT 10"_L1
								.arg(m_execArgs.at(0).toString(), m_execArgs.at(1).toString(), mainDate))};
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				QStringList dates;
				do {
				if (!query.value(0).toString().isEmpty())
					dates.append(std::move(formatDate(query.value(1).toUInt())));
				} while (query.next());
				if (!dates.isEmpty())
					m_model->appendList_fast(std::move(dates));
				ok = true;
			}
			query.finish();
		}

		if (ok)
		{
			strQuery = std::move("SELECT location FROM training_day_table WHERE meso_id=%1 AND date<%3 ORDER BY date DESC LIMIT 5"_L1
				.arg(m_execArgs.at(0).toString(), mainDate));
			if (query.exec(strQuery))
			{
				if (query.first())
				{
					do {
						const QString &lastLocation{query.value(0).toString()};
						if (!lastLocation.isEmpty())
						{
							m_model->setLocation(lastLocation, false);
							break;
						}
					} while (query.next());
					m_model->setReady(true);
				}
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::saveTrainingDay()
{
	if (openDatabase())
	{
		bool ok{false};
		const QStringList &tDayInfoList{m_model->getSaveInfo()};
		QSqlQuery query{getQuery()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec("SELECT id FROM training_day_table WHERE date=%1"_L1.arg(m_model->dateStr())))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE training_day_table SET date=%1, day_number=\'%2\', "
							"split_letter=\'%3\', time_in=\'%4\', time_out=\'%5\', location=\'%6\', notes=\'%7\', "_s
								.arg(m_model->dateStr(), m_model->trainingDay(), m_model->splitLetter(),
									m_model->timeIn(), m_model->timeOut(), m_model->location(), m_model->dayNotes())) +
						std::move(u"exercises=\'%1\', setstypes=\'%2\', setsresttimes=\'%3\', "
							"setssubsets=\'%4\', setsreps=\'%5\', setsweights=\'%6\', setsnotes=\'%7\', setscompleted=\'%8\' WHERE id=%9"_s
								.arg(tDayInfoList.at(TDAY_EXERCISES_COL_NAMES), tDayInfoList.at(TDAY_EXERCISES_COL_TYPES),
									tDayInfoList.at(TDAY_EXERCISES_COL_RESTTIMES), tDayInfoList.at(TDAY_EXERCISES_COL_SUBSETS),
									tDayInfoList.at(TDAY_EXERCISES_COL_REPS), tDayInfoList.at(TDAY_EXERCISES_COL_WEIGHTS),
									tDayInfoList.at(TDAY_EXERCISES_COL_NOTES), tDayInfoList.at(TDAY_EXERCISES_COL_COMPLETED), m_model->idStr()));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO training_day_table "
							"(meso_id,date,day_number,split_letter,time_in,time_out,location,notes,"
							"exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
							" VALUES(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\',"
							"\'%9\', \'%10\', \'%11\', \'%12\', \'%13\', \'%14\', \'%15\', \'%16\')"_s
								.arg(m_model->mesoIdStr(), m_model->dateStr(), m_model->trainingDay(), m_model->splitLetter(),
									m_model->timeIn(), m_model->timeOut(), m_model->location(), m_model->dayNotes(),
									tDayInfoList.at(TDAY_EXERCISES_COL_NAMES), tDayInfoList.at(TDAY_EXERCISES_COL_TYPES),
									tDayInfoList.at(TDAY_EXERCISES_COL_RESTTIMES), tDayInfoList.at(TDAY_EXERCISES_COL_SUBSETS),
									tDayInfoList.at(TDAY_EXERCISES_COL_REPS), tDayInfoList.at(TDAY_EXERCISES_COL_WEIGHTS),
									tDayInfoList.at(TDAY_EXERCISES_COL_NOTES), tDayInfoList.at(TDAY_EXERCISES_COL_COMPLETED)));
		}
		ok = query.exec(strQuery);
		if (ok && !bUpdate)
			m_model->setId(query.lastInsertId().toString());
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
		if (m_model->importMode())
			delete m_model;
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::removeTrainingDay()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"DELETE FROM training_day_table WHERE date=%1 AND meso_id=%2"_L1.arg(
							m_model->dateStr(), m_model->mesoIdStr())};
		const bool ok{query.exec(strQuery)};
		if (ok)
		{
			m_model->clearFast();
			m_model->setReady(false);
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBTrainingDayTable::workoutsInfoForTimePeriod()
{
	m_workoutsInfo.clear();
	if (openDatabase(true))
	{
		const QStringList &exercises{m_execArgs.at(0).toStringList()};
		const QList<QDate> &dates{m_execArgs.at(1).value<QList<QDate>>()};

		QString datesList{'('};
		for (uint i(0); i < dates.count(); ++i)
			datesList += QString::number(dates.at(i).toJulianDay()) + ',';
		datesList.chop(1);
		datesList.append(')');

		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT exercises,setsreps,setsweights FROM training_day_table WHERE date IN(%1)"_L1.arg(datesList)};

		bool ok(false);
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do
				{
					const QString &exercise{query.value(0).toString()};
					if (exercises.contains(exercise))
					{
						QList<QStringList> workout;
						workout.append(std::move(query.value(1).toString().split(exercises_separator, Qt::SkipEmptyParts)));
						workout.append(std::move(query.value(2).toString().split(exercises_separator, Qt::SkipEmptyParts)));
						workout.append(std::move(QStringList(workout.at(0).count())));
						for (uint i(0); i < workout.at(2).count(); ++i)
						{
							const uint nsets(workout.at(0).at(i).count(record_separator));
							workout[2][i] = std::move(QString::number(nsets))+exercises_separator;
						}
					}
				} while (query.next());
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
