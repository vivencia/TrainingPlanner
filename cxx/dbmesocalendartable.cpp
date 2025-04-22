#include "dbmesocalendartable.h"

#include "dbmesocalendarmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBMesoCalendarTable::DBMesoCalendarTable(const QString &dbFilePath, DBMesoCalendarModel *model)
	: TPDatabaseTable{}, m_model{model}
{
	m_tableName = std::move("mesocycles_calendar_table"_L1);
	m_tableID = MESOCALENDAR_TABLE_ID;
	setObjectName(DBMesoCalendarObjectName);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_mesocal_connection-"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString &dbname{dbFilePath + DBMesoCalendarFileName};
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesoCalendarTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS mesocycles_calendar_table ("
										"meso_id INTEGER,"
										"date TEXT,"
										"data TEXT)"_L1};
		const bool ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBMesoCalendarTable::updateTable()
{
	//doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::getMesoCalendar()
{
	if (openDatabase(true))
	{
		bool ok(false);
		const QString &mesoId(m_execArgs.at(0).toString());
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT * FROM mesocycles_calendar_table WHERE meso_id="_L1 + mesoId};

		if (query.exec(strQuery))
		{
			if (query.first())
			{
				// 0: id | 1: meso_id | 2: training day number | 3: split letter | 4: training_complete | 5: year | 6: month | 7: day
				QStringList mesocal_info;
				QString strMonth, strYear;
				int month(-1);
				do
				{
					const int dbmonth = query.value(MESOCALENDAR_COL_MONTH).toInt();
					if (dbmonth != month)
					{
						month = dbmonth;
						strMonth = std::move(QString::number(dbmonth));
						strYear = std::move(query.value(MESOCALENDAR_COL_YEAR).toString());
						if (!mesocal_info.isEmpty())
						{
							m_model->appendList_fast(std::move(mesocal_info));
							mesocal_info.clear();
						}
					}
					mesocal_info.append(std::move(query.value(MESOCALENDAR_COL_ID).toString() + ',' + mesoId + ',' + query.value(MESOCALENDAR_COL_TRAINING_DAY).toString() +
						',' + query.value(MESOCALENDAR_COL_SPLITLETTER).toString() + ',' + query.value(MESOCALENDAR_COL_TRAININGCOMPLETE).toString() +
						',' + strYear + ',' + strMonth));
				} while (query.next ());
				if (!mesocal_info.isEmpty())
					m_model->appendList(std::move(mesocal_info));
			}
			else
			{
				m_model->createModel();
				saveMesoCalendar();
			}
			ok = true;
			m_model->setReady(true);
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::saveMesoCalendar()
{
	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const uint meso_idx{m_execArgs.at(0).toUInt()};
		QString meso_id{std::move(m_model->mesoId(meso_idx, 0).value())};
		bool bUpdate{false};

		if (query.exec("SELECT meso_id FROM mesocycles_calendar_table WHERE meso_id=%1"_L1.arg(meso_id)))
		{
			if (query.first())
				bUpdate = query.value(0).toString() == meso_id;
			query.finish();
		}


		if (mSqlLiteDB.transaction())
		{
			if (!bUpdate)
			{

				const QString &queryCommand{u"INSERT INTO mesocycles_calendar_table (meso_id, date, data) VALUES "_s};
				const QString &queryValuesTemplate{ "(%1, \'%2\', \'%3\'),"_L1 };
				QString dbdata{std::move(std::accumulate(m_model->modeldata(meso_idx).cbegin(),
												 m_model->modeldata(meso_idx).cend(),
												 QString{},
												 [this,meso_id,queryValuesTemplate] (QString data, const QString &day_info) {
					return data + queryValuesTemplate.arg(meso_id,
									appUtils()->getCompositeValue(MESOCALENDAR_COL_DATE, day_info, record_separator), day_info);
				}))};
				dbdata.chop(1);
				ok = query.exec(queryCommand + dbdata);
			}
			else
			{
				const QString &queryCommand{u"UPDATE mesocycles_calendar_table SET data=\'%1\' WHERE meso_id="_s + meso_id + " AND date=\'%2\'; "_L1};
				QString dbdata{std::move(std::accumulate(m_model->modified(meso_idx).cbegin(),
												 m_model->modified(meso_idx).cend(),
												 QString{},
												 [this,meso_id,queryCommand] (QString data, const TPBool is_modified) {

					return is_modified ? data + queryCommand.arg(day_info,
									appUtils()->getCompositeValue(MESOCALENDAR_COL_DATE, day_info, record_separator));
				}))};

			}
		}
		if (ok)
			ok = mSqlLiteDB.commit();
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateMesoCalendarEntry()
{
	if (openDatabase())
	{
		bool ok(false);
		const QDate &date{m_execArgs.at(0).toDate()};
		QSqlQuery query{getQuery()};
		QString strQuery{"SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_L1.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day()))};

		if (query.exec(strQuery))
		{
			query.first();
			const QString &strId(query.value(0).toString());
			query.finish();
			const QString &strTrainingDay(m_execArgs.at(1).toString());
			const QString &strSplit(m_execArgs.at(2).toString());

			strQuery = std::move("UPDATE mesocycles_calendar_table SET training_day=%1, training_split=\'%2\' WHERE id=%3"_L1
											.arg(strTrainingDay, strSplit, strId));
			ok = query.exec(strQuery);
			if (ok)
				m_model->updateDay(date, strTrainingDay, strSplit);
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateDayIsFinished()
{
	if (openDatabase())
	{
		bool ok(false);
		const QDate &date(m_execArgs.at(0).toDate());
		QSqlQuery query{getQuery()};
		QString strQuery;

		strQuery = std::move("SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_L1.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day())));
		if (query.exec(strQuery))
		{
			query.first();
			const QString &strId(query.value(0).toString());
			query.finish();
			strQuery = std::move("UPDATE mesocycles_calendar_table SET training_complete=%1 WHERE id=%2"_L1
									.arg(m_execArgs.at(1).toBool() ? STR_ONE : STR_ZERO, strId));
			ok = query.exec(strQuery);
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::dayInfo(const QDate &date, QStringList &dayInfoList)
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT meso_id,training_day,training_split,training_complete "
								"FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_L1.arg(
								QString::number(date.year()),QString::number(date.month()), QString::number(date.day()))};
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				dayInfoList.append(std::move(query.value(0).toString()));
				dayInfoList.append(std::move(query.value(1).toString()));
				dayInfoList.append(std::move(query.value(2).toString()));
				dayInfoList.append(std::move(query.value(3).toString()));
				dayInfoList.append(std::move(QString::number(date.toJulianDay())));
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBMesoCalendarTable::changeMesoCalendar()
{
	removeEntry();
	m_model->changeModel(m_execArgs.at(1).toBool(), m_execArgs.at(2).toBool(), m_execArgs.at(3).toDate());
	m_model->setReady(true);
	saveMesoCalendar();
}

void DBMesoCalendarTable::updateMesoCalendar()
{
	removeEntry();
	m_model->updateModel(m_execArgs.at(1).toDate(), m_execArgs.at(2).toString());
	m_model->setReady(true);
	saveMesoCalendar();
}

void DBMesoCalendarTable::workoutDayInfoForEntireMeso()
{
	clearWorkoutsInfoList();
	if (openDatabase(true))
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT  *FROM mesocycles_calendar_table WHERE meso_id=%1"_L1.arg(m_execArgs.at(0).toString())};
		bool ok(false);
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do {
					st_workoutDayInfo *workout_info{new st_workoutDayInfo};
					workout_info->trainingDay = std::move(query.value(MESOCALENDAR_COL_TRAINING_DAY).toString());
					if (workout_info->trainingDay == '0')
						workout_info->trainingDay = std::move(tr("Rest day"));
					workout_info->splitLetter = std::move(query.value(MESOCALENDAR_COL_SPLITLETTER).toString());
					workout_info->completed = std::move(query.value(MESOCALENDAR_COL_TRAININGCOMPLETE).toString() == "1"_L1);
					workout_info->date = std::move(QDate(query.value(MESOCALENDAR_COL_YEAR).toUInt(), query.value(MESOCALENDAR_COL_MONTH).toUInt(),
														query.value(MESOCALENDAR_COL_DAY).toUInt()));
					m_workoutsInfoList.append(workout_info);
				} while (query.next());
				m_workoutsInfoList.at(0)->meso_id = m_execArgs.at(0).toUInt();
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::completedDaysForSplitWithinTimePeriod()
{
	m_completedWorkoutDates.clear();
	if (openDatabase(true))
	{
		const QChar &splitLetter{m_execArgs.at(0).toChar()};
		const QDate &startDate{m_execArgs.at(1).toDate()};
		const QDate &endDate{m_execArgs.at(2).toDate()};
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT training_complete,year,month,day FROM mesocycles_calendar_table WHERE training_split=%1 AND "
			"year>=%2 AND year<=%3 AND month>=%4 AND month<=%5 AND year>=%6 AND year<=%7"_L1.arg(
				QString(splitLetter), QString::number(startDate.year()), QString::number(endDate.year()), QString::number(startDate.month()),
				QString::number(endDate.month()), QString::number(startDate.day()), QString::number(endDate.day()))};

		bool ok(false);
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
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
