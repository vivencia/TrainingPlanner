#include "dbworkouttable.h"

#include "dbexercisesmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

DBWorkoutsTable::DBWorkoutsTable(DBWorkoutModel *model)
	: TPDatabaseTable{model->calendarDay() >= 0 ? WORKOUT_TABLE_ID : MESOSPLIT_TABLE_ID, nullptr}, m_model{model}
{
	m_tableName = std::move(tableId() == WORKOUT_TABLE_ID ? "workouts_table"_L1 : "mesosplit_table"_L1);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_exercises_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath());
	#ifndef QT_NO_DEBUG
	setObjectName(tableId() == WORKOUT_TABLE_ID ? "WorkoutsTable"_L1 : "SplitTable"_L1);
	#endif
}

void DBWorkoutsTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS %1 ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"calendar_day INTEGER,"
										"split_letter TEXT, "
										"exercises TEXT DEFAULT \"\","
										"setstypes TEXT DEFAULT \"\","
										"setsresttimes TEXT DEFAULT \"\","
										"setssubsets TEXT DEFAULT \"\","
										"setsreps TEXT DEFAULT \"\","
										"setsweights TEXT DEFAULT \"\","
										"setsnotes TEXT DEFAULT \"\","
										"setscompleted TEXT DEFAULT \"\")"_L1.arg(m_tableName)
		};
		const bool ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBWorkoutsTable::updateTable()
{
	//doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBWorkoutsTable::getExercises()
{
	if (openDatabase(true))
	{
		bool b_ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{tableId() == WORKOUT_TABLE_ID ?
					"SELECT * FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->calendarDay()) :
					"SELECT * FROM %1 WHERE meso_id=%2 AND split_letter=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->splitLetter())
		};
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

void DBWorkoutsTable::saveExercises()
{
	if (openDatabase())
	{
		bool ok{false};
		const QStringList &workoutData{m_model->toDatabase()};
		QSqlQuery query{getQuery()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec(tableId() == WORKOUT_TABLE_ID ?
				"SELECT id FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->calendarDay()) :
				"SELECT id FROM %2 WHERE meso_id=%2 AND split_letter=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->splitLetter()))
				)
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE %1 SET exercises=\'%2\', setstypes=\'%3\', setsresttimes=\'%4\', "
							"setssubsets=\'%5\', setsreps=\'%6\', setsweights=\'%7\', setsnotes=\'%8\', setscompleted=\'%9\' WHERE id=%10"_s
								.arg(m_tableName ,workoutData.at(EXERCISES_COL_EXERCISES), workoutData.at(EXERCISES_COL_SETTYPES),
									workoutData.at(EXERCISES_COL_RESTTIMES), workoutData.at(EXERCISES_COL_SUBSETS),
									workoutData.at(EXERCISES_COL_REPS), workoutData.at(EXERCISES_COL_WEIGHTS),
									workoutData.at(EXERCISES_COL_NOTES), workoutData.at(EXERCISES_COL_COMPLETED), m_model->id()));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO %1 "
						"(meso_id,%2,exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
						" VALUES(%3, %4, \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\')"_s
						.arg(m_tableName, tableId() == WORKOUT_TABLE_ID ? "calendar_day"_L1 : "split_letter"_L1,
							m_model->mesoId(),
							tableId() == WORKOUT_TABLE_ID ? QString::number(m_model->calendarDay()) : QString{m_model->splitLetter()},
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

void DBWorkoutsTable::removeExercises()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{tableId() == WORKOUT_TABLE_ID ?
			"DELETE FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->calendarDay()) :
			"DELETE FROM %1 WHERE meso_id=%2 AND split_letter=%3"_L1.arg(m_tableName, m_model->mesoId(), m_model->splitLetter())
		};
		const bool ok{query.exec(strQuery)};
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
