#include "dbmesocalendartable.h"

#include "dbmesocalendarmanager.h"
#include "tputils.h"

#include <QSqlError>

DBMesoCalendarTable::DBMesoCalendarTable(DBMesoCalendarManager *model)
	: TPDatabaseTable{MESOCALENDAR_TABLE_ID}, m_model{model}
{
	setTableName(tableName());
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_mesocal_connection-"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_QDEBUG
	setObjectName("MesoCalendarTable");
	#endif
}

QLatin1StringView DBMesoCalendarTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
										"meso_id INTEGER,"
										"date TEXT,"
										"data TEXT);"_L1;
}

void DBMesoCalendarTable::getMesoCalendar()
{
	bool ok{false};
	const uint meso_idx{m_execArgs.at(0).toUInt()};
	const QString &meso_id{m_execArgs.at(1).toString()};
	if (execQuery("SELECT date,data FROM %1 WHERE meso_id=%2;"_L1.arg(tableName(), meso_id), true, false))
	{
		if (m_workingQuery.first())
		{
			uint calendar_day{0};
			QList<stDayInfo*> &meso_calendar{m_model->mesoCalendar(meso_idx)};
			do
			{
				stDayInfo *day_info{new stDayInfo{}};
				day_info->date = std::move(m_workingQuery.value(0).toString());
				day_info->data = std::move(m_workingQuery.value(1).toString());
				meso_calendar[calendar_day] = day_info;
				if (++calendar_day >= meso_calendar.count()) //error in database
					break;
			} while (m_workingQuery.next());
			if (!m_model->mesoCalendar(meso_idx).isEmpty())
			{
				m_model->setDBDataReady(meso_idx, true,
							appUtils()->calculateNumberOfMonths(meso_calendar.first()->date, meso_calendar.last()->date));
				ok = true;
			}
		}
	}
	if (!ok)
		m_model->createCalendar(meso_idx);
}

void DBMesoCalendarTable::saveMesoCalendar()
{
	const uint meso_idx{m_execArgs.at(0).toUInt()};
	if (m_model && m_model->hasCalendarInfo(meso_idx))
	{
		const QString &meso_id{m_model->mesoId(meso_idx, 0)};
		if (execQuery("SELECT meso_id FROM %1 WHERE meso_id=%2;"_L1.arg(tableName(), meso_id), true, false))
		{
			TPBool update;
			if (m_workingQuery.first())
				update = m_workingQuery.value(0).toString() == meso_id;

			if (mSqlLiteDB.transaction())
			{
				if (!update)
				{
					const QString &queryCommand{"INSERT INTO "_L1 + tableName() + u"(meso_id, date, data) VALUES "_s};
					const QString &queryValuesTemplate{ "(%1, \'%2\', \'%3\'),"_L1 };
					QString dbdata{std::move(std::accumulate(m_model->mesoCalendar(meso_idx).cbegin(),
												 m_model->mesoCalendar(meso_idx).cend(),
												 QString{},
												 [this,meso_id,queryValuesTemplate] (const QString &data, stDayInfo *day_info) {
						return data + queryValuesTemplate.arg(meso_id, day_info->date, day_info->data);
					}))};
					dbdata[dbdata.length()-1] = ';';
					m_strQuery = std::move(queryCommand + dbdata);
				}
				else
				{
					const QString &queryCommand{"UPDATE "_L1 + tableName() + "SET data=\'%1\' WHERE meso_id="_L1 +
															meso_id + " AND date=\'%2\';"_L1};
					m_strQuery = std::move(std::accumulate(m_model->mesoCalendar(meso_idx).cbegin(),
												 m_model->mesoCalendar(meso_idx).cend(),
												 QString{},
												 [this,meso_id,queryCommand] (const QString &data, stDayInfo *day_info) {
						if (day_info->modified)
						{
							day_info->modified = false;
							return data + queryCommand.arg(day_info->data, day_info->date);
						}
						else
							return data + QString{};
					}));
				}
				if (execQuery(m_strQuery, false, false))
				{
					if (mSqlLiteDB.commit())
						emit queryExecuted(true, true);
					#ifndef QT_NO_DEBUG
					else
					{
						qDebug() << "****** ERROR ******";
						qDebug() << "DBMesoCalendarTable::saveMesoCalendar() -> transaction not commited"_L1;
						qDebug() << mSqlLiteDB.lastError();
						qDebug();
					}
					#endif
				}
			}
		}
	}
}

bool DBMesoCalendarTable::mesoCalendarSavedInDB(const QString &meso_id)
{
	if (execQuery("SELECT meso_id FROM %1 WHERE meso_id=%2;"_L1.arg(tableName(), meso_id), true, false))
	{
		if (m_workingQuery.first())
			return m_workingQuery.value(0).toString() == meso_id;
	}
	return false;
}

//TODO
void DBMesoCalendarTable::workoutDayInfoForEntireMeso()
{
	clearWorkoutsInfoList();
	if (openDatabase(true))
	{
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"SELECT * FROM %1 WHERE meso_id=%2"_L1.arg(tableName(), m_execArgs.at(0).toString())};
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do {
					st_workoutDayInfo *workout_info{new st_workoutDayInfo};
					/*workout_info->trainingDay = std::move(query.value(MESOCALENDAR_COL_TRAINING_DAY).toString());
					if (workout_info->trainingDay == '0')
						workout_info->trainingDay = std::move(tr("Rest day"));
					workout_info->splitLetter = std::move(query.value(MESOCALENDAR_COL_SPLITLETTER).toString());
					workout_info->completed = std::move(query.value(MESOCALENDAR_COL_TRAININGCOMPLETE).toString() == "1"_L1);
					workout_info->date = std::move(QDate(query.value(MESOCALENDAR_COL_YEAR).toUInt(), query.value(MESOCALENDAR_COL_MONTH).toUInt(),
														query.value(MESOCALENDAR_COL_DAY).toUInt()));*/
					m_workoutsInfoList.append(workout_info);
				} while (query.next());
				m_workoutsInfoList.at(0)->meso_id = m_execArgs.at(0).toUInt();
			}
		}
	}
}

//TODO
void DBMesoCalendarTable::completedDaysForSplitWithinTimePeriod()
{
	m_completedWorkoutDates.clear();
	if (openDatabase(true))
	{
		const QChar &splitLetter{m_execArgs.at(0).toChar()};
		const QDate &startDate{m_execArgs.at(1).toDate()};
		const QDate &endDate{m_execArgs.at(2).toDate()};
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"SELECT training_complete,year,month,day FROM mesocycles_calendar_table WHERE training_split=%1 AND "
			"year>=%2 AND year<=%3 AND month>=%4 AND month<=%5 AND year>=%6 AND year<=%7"_L1.arg(
				QString(splitLetter), QString::number(startDate.year()), QString::number(endDate.year()), QString::number(startDate.month()),
				QString::number(endDate.month()), QString::number(startDate.day()), QString::number(endDate.day()))};

		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do
				{
					if (query.value(0).toInt() == 1)
					{
						QDate date{query.value(1).toInt(), query.value(2).toInt(), query.value(3).toInt()};
						m_completedWorkoutDates.append(std::move(date));
					}
				} while (query.next());
			}
		}
	}
}
