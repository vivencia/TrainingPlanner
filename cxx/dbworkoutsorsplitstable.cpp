#include "dbworkoutsorsplitstable.h"

#include "dbexercisesmodel.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>

using namespace Qt::Literals::StringLiterals;

DBWorkoutsOrSplitsTable::DBWorkoutsOrSplitsTable(DBExercisesModel *model)
	: TPDatabaseTable{model->calendarDay() >= 0 ? WORKOUT_TABLE_ID : MESOSPLIT_TABLE_ID, nullptr}, m_model{model}
{
	commonConstructor();
}

QLatin1StringView DBWorkoutsOrSplitsTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
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
										"setscompleted TEXT);"_L1;
}

void DBWorkoutsOrSplitsTable::commonConstructor()
{
	setTableName(tableName(tableId()));
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{tableName(tableId()) + "_connection"_L1 + QString::number(m_uniqueID)};
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	m_sqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName(tableId() == WORKOUT_TABLE_ID ? "WorkoutsTable"_L1 : "SplitTable"_L1);
	#endif
}

void DBWorkoutsOrSplitsTable::getExercises()
{
	const QString &str_query{tableId() == WORKOUT_TABLE_ID ?
					"SELECT * FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(
											tableName(tableId()), m_model->mesoId(), QString::number(m_model->calendarDay())) :
					"SELECT * FROM %1 WHERE meso_id=%2 AND split_letter=\'%3\'"_L1.arg(
											tableName(tableId()), m_model->mesoId(), m_model->splitLetter())
	};
	if (execQuery(str_query, true, false))
	{
		if (m_workingQuery.first ())
		{
			QStringList workout_info{WORKOUT_TOTALCOLS};
			for (uint i{EXERCISES_COL_ID}; i < WORKOUT_TOTALCOLS; ++i)
				workout_info[i] = std::move(m_workingQuery.value(i).toString());
			static_cast<void>(m_model->fromDataBase(workout_info));
		}
	}
}

void DBWorkoutsOrSplitsTable::saveExercises()
{
	const QStringList &modelData{m_model->toDatabase()};
	bool update{false};
	const QString &str_query{tableId() == WORKOUT_TABLE_ID ?
			"SELECT id FROM %1 WHERE meso_id=%2 AND calendar_day=%3"_L1.arg(
											tableName(tableId()), m_model->mesoId(), QString::number(m_model->calendarDay())) :
			"SELECT id FROM %1 WHERE meso_id=%2 AND split_letter=\'%3\'"_L1.arg(
											tableName(tableId()), m_model->mesoId(), m_model->splitLetter())
	};

	if (execQuery(str_query, true, false))
	{
		if (m_workingQuery.first())
			update = m_workingQuery.value(0).toUInt() >= 0;

		if (update)
		{
			m_strQuery = std::move(
					u"UPDATE %1 SET split_letter=\'%2\', exercises=\'%3\', track_rest_time=\'%4\', "
					"auto_rest_time=\'%5\', setstypes=\'%6\', setsresttimes=\'%7\', setssubsets=\'%8\', "
					"setsreps=\'%9\', setsweights=\'%10\', setsnotes=\'%11\', setscompleted=\'%12\' WHERE id=%13"_s
						.arg(tableName(tableId()), modelData.at(EXERCISES_COL_SPLITLETTER),
							modelData.at(EXERCISES_COL_EXERCISES), modelData.at(EXERCISES_COL_TRACKRESTTIMES),
							modelData.at(EXERCISES_COL_AUTORESTTIMES), modelData.at(EXERCISES_COL_SETTYPES),
							modelData.at(EXERCISES_COL_RESTTIMES), modelData.at(EXERCISES_COL_SUBSETS),
							modelData.at(EXERCISES_COL_REPS), modelData.at(EXERCISES_COL_WEIGHTS),
							modelData.at(EXERCISES_COL_NOTES), modelData.at(EXERCISES_COL_COMPLETED), m_model->id()));
		}
		else
		{
			m_strQuery = std::move(u"INSERT INTO %1 "
					"(meso_id,calendar_day,split_letter,exercises,track_rest_time,auto_rest_time,setstypes,"
					"setsresttimes,setssubsets,setsreps,setsweights,setsnotes,setscompleted)"
					" VALUES(%2, %3, \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\', \'%11\', \'%12\', \'%13\', \'%14\')"_s
						.arg(tableName(tableId()), modelData.at(EXERCISES_COL_MESOID),
							modelData.at(EXERCISES_COL_CALENDARDAY), modelData.at(EXERCISES_COL_SPLITLETTER),
							modelData.at(EXERCISES_COL_EXERCISES), modelData.at(EXERCISES_COL_TRACKRESTTIMES),
							modelData.at(EXERCISES_COL_AUTORESTTIMES), modelData.at(EXERCISES_COL_SETTYPES),
							modelData.at(EXERCISES_COL_RESTTIMES), modelData.at(EXERCISES_COL_SUBSETS),
							modelData.at(EXERCISES_COL_REPS), modelData.at(EXERCISES_COL_WEIGHTS),
							modelData.at(EXERCISES_COL_NOTES), modelData.at(EXERCISES_COL_COMPLETED)));
		}
		if (execQuery(m_strQuery, false, false))
		{
			if (!update)
				m_model->setId(m_workingQuery.lastInsertId().toString());
			emit queryExecuted(true, true);
		}
	}
}

void DBWorkoutsOrSplitsTable::removeExercises()
{
	const QString meso_id{m_execArgs.at(0).toString()};
	m_strQuery = std::move(tableId() == WORKOUT_TABLE_ID ?
			std::move("DELETE FROM %1 WHERE meso_id=%2"_L1.arg(tableName(tableId()), meso_id)) :
			std::move("DELETE FROM %1 WHERE meso_id=%2"_L1.arg(tableName(tableId()), meso_id)));

	const bool remove_all{m_execArgs.at(1).toBool()};
	if (!remove_all) //Delete one specific entry, otherwise all entries for the meso will be deleted
	{
		m_strQuery += std::forward<QString>(tableId() == WORKOUT_TABLE_ID ?
		std::move(" AND calendar_day=%1;"_L1.arg(QString::number(m_model->calendarDay()))) :
		std::move(" AND split_letter=%1;"_L1.arg(m_model->splitLetter())));
	}
	else
		m_strQuery.append(';');
	const bool success{execQuery(m_strQuery, false)};
	emit queryExecuted(success, true);
}

bool DBWorkoutsOrSplitsTable::mesoHasAllSplitPlans(const QString &meso_id, const QString &split)
{
	bool ok{false};
	const QString &str_query{"SELECT setstypes FROM mesosplit_table WHERE meso_id="_L1 + meso_id + " AND split_letter=\'%1\';"_L1};
	for (const auto split_letter : split)
	{
		if (split_letter.cell() >= 'A' && split_letter.cell() <= 'F')
		{
			if (execQuery(str_query.arg(split_letter), true, false))
			{
				if (m_workingQuery.first())
				{
					static_cast<void>(m_workingQuery.value(0).toUInt(&ok));
					if (!ok)
						break;
				}
				else
					break;
			}
		}
	}
	return ok;
}

bool DBWorkoutsOrSplitsTable::mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter)
{
	const QString &str_query{"SELECT setstypes FROM mesosplit_table WHERE meso_id=%1 AND split_letter=\'%2\';"_L1.arg(meso_id, split_letter)};
	bool ok{false};
	if (execQuery(str_query, true, false))
	{
		if (m_workingQuery.first())
			m_workingQuery.value(0).toUInt(&ok);
	}
	return ok;
}

void DBWorkoutsOrSplitsTable::getPreviousWorkouts()
{
	QString m_strQuery{"SELECT calendar_day FROM %1 WHERE meso_id=%2 AND split_letter=\'%3\' "
							"AND calendar_day<%4 ORDER BY calendar_day DESC LIMIT 5;"_L1.arg(
								tableName(tableId()), m_model->mesoId(), m_model->splitLetter(), QString::number(m_model->calendarDay()))};
	if (execQuery(m_strQuery, true, false))
	{
		if (m_workingQuery.first())
		{
			m_model->clearPreviousWorkouts();
			do {
				m_model->appendPreviousWorkout(m_workingQuery.value(0).toUInt());
			} while (m_workingQuery.next());
		}
	}
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

		QSqlQuery query{std::move(getQuery())};
		const QString &m_strQuery{"SELECT exercises,setsreps,setsweights FROM workouts_table WHERE date IN(%1)"_L1.arg(datesList)};

		bool ok(false);
		if (m_workingQuery.exec(m_strQuery))
		{
			if (m_workingQuery.first())
			{
				do
				{
					const QString &exercise{m_workingQuery.value(0).toString()};
					if (exercises.contains(exercise))
					{
						QList<QStringList> workout;
						workout.append(std::move(m_workingQuery.value(1).toString().split(exercises_separator, Qt::SkipEmptyParts)));
						workout.append(std::move(m_workingQuery.value(2).toString().split(exercises_separator, Qt::SkipEmptyParts)));
						workout.append(std::move(QStringList(workout.at(0).count())));
						for (uint i(0); i < workout.at(2).count(); ++i)
						{
							const uint nsets(workout.at(0).at(i).count(record_separator));
							workout[2][i] = std::move(QString::number(nsets))+exercises_separator;
						}
					}
				} while (m_workingQuery.next());
				ok = true;
			}
		}
		setQueryResult(ok, m_strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}*/
