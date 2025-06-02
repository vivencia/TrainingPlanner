#include "dbworkoutsorsplitstable.h"

#include "dbexercisesmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

DBWorkoutsOrSplitsTable::DBWorkoutsOrSplitsTable(DBExercisesModel *model)
	: TPDatabaseTable{model->calendarDay() >= 0 ? WORKOUT_TABLE_ID : MESOSPLIT_TABLE_ID, nullptr}, m_model{model}
{
	commonConstructor();
}

void DBWorkoutsOrSplitsTable::commonConstructor()
{
	m_tableName = std::move(tableId() == WORKOUT_TABLE_ID ? "workouts_table"_L1 : "mesosplit_table"_L1);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_exercises_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName(tableId() == WORKOUT_TABLE_ID ? "WorkoutsTable"_L1 : "SplitTable"_L1);
	#endif
}

void DBWorkoutsOrSplitsTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS %1 ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"calendar_day INTEGER,"
										"split_letter TEXT,"
										"exercises TEXT, "
										"track_rest_time TEXT,"
										"auto_rest_time TEXT,"
										"setstypes TEXT,"
										"setsresttimes TEXT,"
										"setssubsets TEXT,"
										"setsreps TEXT,"
										"setsweights TEXT,"
										"setsnotes TEXT,"
										"setscompleted TEXT)"_L1.arg(m_tableName)
		};
		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBWorkoutsOrSplitsTable::getExercises()
{
	if (openDatabase(true))
	{
		bool b_ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{tableId() == WORKOUT_TABLE_ID ?
					"SELECT * FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(
											m_tableName, m_model->mesoId(), QString::number(m_model->calendarDay())) :
					"SELECT * FROM %1 WHERE meso_id=%2 AND split_letter=%3"_L1.arg(
											m_tableName, m_model->mesoId(), m_model->splitLetter())
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

void DBWorkoutsOrSplitsTable::saveExercises()
{
	if (openDatabase())
	{
		bool ok{false};
		const QStringList &workoutData{m_model->toDatabase()};
		QSqlQuery query{getQuery()};
		bool bUpdate{false};
		QString strQuery;

		if (query.exec(tableId() == WORKOUT_TABLE_ID ?
				"SELECT id FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(
												m_tableName, m_model->mesoId(), QString::number(m_model->calendarDay())) :
				"SELECT id FROM %2 WHERE meso_id=%2 AND split_letter=%3"_L1.arg(
												m_tableName, m_model->mesoId(), m_model->splitLetter()))
				)
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = std::move(u"UPDATE %1 SET calendar_day=%2, split_letter=\'%3\', exercises=\'%4\', setstypes=\'%5\', setsresttimes=\'%6\', "
							"setssubsets=\'%7\', setsreps=\'%8\', setsweights=\'%9\', setsnotes=\'%10\', setscompleted=\'%11\' WHERE id=%12"_s
								.arg(m_tableName, workoutData.at(EXERCISES_COL_CALENDARDAY), workoutData.at(EXERCISES_COL_SPLITLETTER),
									workoutData.at(EXERCISES_COL_EXERCISES), workoutData.at(EXERCISES_COL_SETTYPES),
									workoutData.at(EXERCISES_COL_RESTTIMES), workoutData.at(EXERCISES_COL_SUBSETS),
									workoutData.at(EXERCISES_COL_REPS), workoutData.at(EXERCISES_COL_WEIGHTS),
									workoutData.at(EXERCISES_COL_NOTES), workoutData.at(EXERCISES_COL_COMPLETED), m_model->id()));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO %1 "
						"(meso_id,calendar_day,split_letter,exercises,setstypes,setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
						" VALUES(%2, %3, %4, \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\')"_s
						.arg(m_tableName, workoutData.at(EXERCISES_COL_MESOID),
							workoutData.at(EXERCISES_COL_CALENDARDAY), workoutData.at(EXERCISES_COL_SPLITLETTER),
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

void DBWorkoutsOrSplitsTable::removeExercises()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		QString strQuery{tableId() == WORKOUT_TABLE_ID ?
				std::move("DELETE FROM %1 WHERE meso_id=%2"_L1.arg(m_tableName, m_model->mesoId())) :
				std::move("DELETE FROM %1 WHERE meso_id=%2"_L1.arg(m_tableName, m_model->mesoId()))
		};
		const bool remove_all{m_execArgs.at(0).toBool()};
		if (!remove_all) //Delete one specific entry, otherwise all entries for the meso will be deleted
		{
			strQuery += std::forward<QString>(tableId() == WORKOUT_TABLE_ID ?
			std::move(" AND calendar_day=%1"_L1.arg(QString::number(m_model->calendarDay()))) :
			std::move(" AND split_letter=%1"_L1.arg(m_model->splitLetter())));
		}

		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

bool DBWorkoutsOrSplitsTable::mesoHasAllSplitPlans(const QString &meso_id, const QString &split)
{
	bool ok{false};
	if (openDatabase(true))
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT setstypes FROM mesosplit_table WHERE meso_id=%1 AND split_letter=\'%2\'"_L1.arg(meso_id)};
		for (const auto split_letter : split)
		{
			if (split_letter.cell() >= 'A' && split_letter.cell() <= 'F')
			{
				if (query.exec(strQuery.arg(split_letter)))
				{
					if (query.first())
						query.value(0).toUInt(&ok);
					if (!ok)
						break;
				}
				else
					break;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	return ok;
}

bool DBWorkoutsOrSplitsTable::mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter)
{
	bool ok{false};
	if (openDatabase(true))
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT setstypes FROM mesosplit_table WHERE meso_id=%1 AND split_letter=\'%2\'"_L1.arg(meso_id, split_letter)};
		if (query.exec(strQuery))
		{
			if (query.first())
				query.value(0).toUInt(&ok);
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	return ok;
}

void DBWorkoutsOrSplitsTable::getPreviousWorkouts()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		QString strQuery{"SELECT calendar_day FROM %1 WHERE meso_id=%2 AND split_letter=\'%3\' "
							"AND calendar_day<%4 ORDER BY calendar_day DESC LIMIT 5"_L1.arg(
								m_tableName, m_model->mesoId(), m_model->splitLetter(), QString::number(m_model->calendarDay()))};
		const bool ok{query.exec(strQuery)};
		if (ok)
		{
			if (query.first())
			{
				m_model->clearPreviousWorkouts();
				do {
					m_model->appendPreviousWorkout(query.value(0).toUInt());
				} while (query.next());
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
/*void DBWorkoutsOrSplitsTable::workoutsInfoForTimePeriod()
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
}*/
