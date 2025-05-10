#include "dbworkouttable.h"
#include "dbworkoutmodel.h"
#include "tputils.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlQuery>

DBWorkoutsTable::DBWorkoutsTable(const QString &dbFilePath, DBWorkoutModel *model)
	: TPDatabaseTable{nullptr}, m_model(model)
{
	m_tableName = std::move("workouts_table"_L1);
	m_tableID = WORKOUT_TABLE_ID;
	setObjectName(DBTrainingDayObjectName);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name("db_trainingday_connection"_L1 + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString &dbname(dbFilePath + DBTrainingDayFileName);
	mSqlLiteDB.setDatabaseName( dbname );
}

void DBWorkoutsTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS workouts_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"calendar_day INTEGER,"
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

void DBWorkoutsTable::updateTable()
{
	//doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBWorkoutsTable::getWorkout()
{
	if (openDatabase(true))
	{
		bool b_ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT * FROM workouts_table WHERE meso_id=%1 AND calendar_day=%2"_L1.arg(
									m_model->mesoId(), m_model->calendarDay())};
		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				QStringList workout_info{WORKOUT_TOTALCOLS};
				for (uint i{EXERCISES_COL_ID}; i < WORKOUT_TOTALCOLS; ++i)
					workout_info[i] = std::move(query.value(i).toString());
				b_ok = m_model->fromDataBase(workout_info);
			}	
		}
		setQueryResult(b_ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBWorkoutsTable::saveWorkout()
{
	if (openDatabase())
	{
		bool ok{false};
		const QStringList &workoutData{m_model->toDatabase()};
		QSqlQuery query{getQuery()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec("SELECT id FROM workouts_table WHERE meso_id=%1 AND calendar_day=%2"_L1.arg(
											m_model->mesoId(), m_model->calendarDay())))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE workouts_table SET exercises=\'%1\', setstypes=\'%2\', setsresttimes=\'%3\', "
							"setssubsets=\'%4\', setsreps=\'%5\', setsweights=\'%6\', setsnotes=\'%7\', setscompleted=\'%8\' WHERE id=%9"_s
								.arg(workoutData.at(EXERCISES_COL_EXERCISES), workoutData.at(EXERCISES_COL_SETTYPES),
									workoutData.at(EXERCISES_COL_RESTTIMES), workoutData.at(EXERCISES_COL_SUBSETS),
									workoutData.at(EXERCISES_COL_REPS), workoutData.at(EXERCISES_COL_WEIGHTS),
									workoutData.at(EXERCISES_COL_NOTES), workoutData.at(EXERCISES_COL_COMPLETED), m_model->id()));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO workouts_table "
							"(meso_id,calendar_day,exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
							" VALUES(%1, %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\')"_s
								.arg(m_model->mesoId(), QString::number(m_model->calendarDay()),
									workoutData.at(EXERCISES_COL_EXERCISES), workoutData.at(EXERCISES_COL_SETTYPES),
									workoutData.at(EXERCISES_COL_RESTTIMES), workoutData.at(EXERCISES_COL_SUBSETS),
									workoutData.at(EXERCISES_COL_REPS), workoutData.at(EXERCISES_COL_WEIGHTS),
									workoutData.at(EXERCISES_COL_NOTES), workoutData.at(EXERCISES_COL_COMPLETED)));
		}
		ok = query.exec(strQuery);
		if (ok && !bUpdate)
			m_model->setId(query.lastInsertId().toString());
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBWorkoutsTable::removeWorkout()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"DELETE FROM workouts_table WHERE meso_id=%1 AND calendar_day=%2"_L1.arg(
						m_model->mesoId(), m_model->calendarDay())};
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

void DBWorkoutsTable::workoutsInfoForTimePeriod()
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
		const QString &strQuery{"SELECT exercises,setsreps,setsweights FROM workouts_table WHERE date IN(%1)"_L1.arg(datesList)};

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
