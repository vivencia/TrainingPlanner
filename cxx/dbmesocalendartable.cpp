#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBMesoCalendarTable::DBMesoCalendarTable(const QString& dbFilePath, DBMesoCalendarModel* model)
	: TPDatabaseTable{}, m_model(model)
{
	m_tableName = u"mesocycles_calendar_table"_s;
	m_tableID = MESOCALENDAR_TABLE_ID;
	setObjectName(DBMesoCalendarObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString& cnx_name(u"db_mesocal_connection-"_s + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_s, cnx_name);
	const QString& dbname(dbFilePath + DBMesoCalendarFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesoCalendarTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"CREATE TABLE IF NOT EXISTS mesocycles_calendar_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"training_day INTEGER,"
										"training_split TEXT,"
										"training_complete INTEGER,"
										"year INTEGER,"
										"month INTEGER,"
										"day INTEGER"
									")"_s);
		const bool ok = query.exec(strQuery);
		setResult(ok, nullptr, strQuery, SOURCE_LOCATION);
	}
}

void DBMesoCalendarTable::updateTable()
{
	/*m_result = false;
	QStringList oldTableInfo;
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		QString dayInfo;
		query.setForwardOnly(true);
		if (query.exec(u"SELECT * FROM mesocycles_calendar_table"_s))
		{
			if (query.first ())
			{
				do {
					for (uint i(0); i < 7; ++i)
					{
						dayInfo = query.value(MESOCALENDAR_COL_MESOID).toString() + ',' + query.value(MESOCALENDAR_COL_TRAINING_DAY).toString() +
							u",\'"_s +	query.value(MESOCALENDAR_COL_SPLITLETTER).toString() + QStringLiteral("\',1,") +
							query.value(MESOCALENDAR_COL_YEAR-1).toString() + ',' + query.value(MESOCALENDAR_COL_MONTH-1).toString() + ',' +
							query.value(MESOCALENDAR_COL_DAY-1).toString();
					}
					oldTableInfo.append(dayInfo);
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
					query.exec(u"PRAGMA page_size = 4096"_s);
					query.exec(u"PRAGMA cache_size = 16384"_s);
					query.exec(u"PRAGMA temp_store = MEMORY"_s);
					query.exec(u"PRAGMA journal_mode = OFF"_s);
					query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_s);
					query.exec(u"PRAGMA synchronous = 0"_s);
					if (mSqlLiteDB.transaction())
					{
						const QString& queryStart(u"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES "_s);
						QString queryValues;

						for (uint i(0), n(0); i < oldTableInfo.count(); ++i, ++n)
						{
							queryValues += '(' + oldTableInfo.at(i) + QStringLiteral("),");
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
						MSG_OUT("DBMesoCalendarTable updateDatabase Database error:  " << mSqlLiteDB.lastError().databaseText())
						MSG_OUT("DBMesoCalendarTable updateDatabase Driver error:  " << mSqlLiteDB.lastError().driverText())
					}
					else
						MSG_OUT("DBMesoCalendarTable updateDatabase SUCCESS")
					mSqlLiteDB.close();
				}
			}
		}
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));*/
}

void DBMesoCalendarTable::getMesoCalendar()
{
	if (openDatabase(true))
	{
		bool ok(false);
		const QString& mesoId(m_execArgs.at(0).toString());
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT * FROM mesocycles_calendar_table WHERE meso_id="_s + mesoId);

		if (query.exec(strQuery))
		{
			if (query.first())
			{
				// 0: id | 1: meso_id | 2: training day number | 3: split letter | 4: training_complete | 5: year | 6: month | 7: day
				QStringList mesocal_info;
				int month(-1);
				QString strMonth, strYear;
				uint day(1);

				do
				{
					const int dbmonth = query.value(MESOCALENDAR_COL_MONTH).toInt();
					if (dbmonth != month)
					{
						strMonth = QString::number(dbmonth);
						strYear = std::move(query.value(MESOCALENDAR_COL_YEAR).toString());
						if (!mesocal_info.isEmpty())
						{
							m_model->appendListMove(mesocal_info);
							mesocal_info.clear();
							day = 1;
						}
						else
						{
							const uint firstDayOfMeso(query.value(MESOCALENDAR_COL_DAY).toUInt());
							if (firstDayOfMeso > 1)
							{
								//Fill the model with info that reflects that these month days are not part of the meso
								for( ; day < firstDayOfMeso; ++day)
									mesocal_info.append(std::move(u"-1,-1,-1,N,-1,"_s + strYear + ',' + strMonth));
							}
						}
					}
					month = dbmonth;
					mesocal_info.append(query.value(MESOCALENDAR_COL_ID).toString() + ',' + mesoId + ',' + query.value(MESOCALENDAR_COL_TRAINING_DAY).toString() +
						',' + query.value(MESOCALENDAR_COL_SPLITLETTER).toString() + ',' + query.value(MESOCALENDAR_COL_TRAININGCOMPLETE).toString() +
						',' + strYear + ',' + strMonth);
					day++;
				} while (query.next ());
				if (!mesocal_info.isEmpty()) //The days of the last month
				{
					const uint lastDayOfMonth(QDate(strYear.toInt(), strMonth.toInt(), ++day).daysInMonth());
					//Fill the model with info that reflects that these month days are not part of the meso
					for( ; day <= lastDayOfMonth; ++day)
						mesocal_info.append(std::move(u"-1,-1,-1,N,-1,"_s + strYear + ',' + strMonth));
					m_model->appendListMove(mesocal_info);
				}
			}
			else
			{
				m_model->createModel();
				saveMesoCalendar();
			}
			ok = true;
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::saveMesoCalendar()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};

		const QString& queryStart(u"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES "_s);
		QString queryValues;
		QStringList day_info;
		uint n(0);

		static_cast<void>(mSqlLiteDB.transaction());
		for (uint i(0), x(0); i < m_model->count(); ++i)
		{
			for (x = 0; x < m_model->getRow_const(i).count(); ++x, ++n)
			{
				day_info = m_model->getDayInfo(i, x).split(',');
				if (day_info.at(MESOCALENDAR_COL_MESOID) != STR_MINUS_ONE)
				{
					queryValues += '(' + day_info.at(MESOCALENDAR_COL_MESOID) + ',' + day_info.at(MESOCALENDAR_COL_TRAINING_DAY) +
						u",\'"_s + day_info.at(MESOCALENDAR_COL_SPLITLETTER) + u"\',"_s +
								day_info.at(MESOCALENDAR_COL_TRAININGCOMPLETE) + ',' + day_info.at(MESOCALENDAR_COL_YEAR) + ',' +
								day_info.at(MESOCALENDAR_COL_MONTH) + ',' + QString::number(x+1) + u"),"_s;
				}
			}
			if (n >= 150)
			{
				queryValues.chop(1);
				ok = query.exec(queryStart + queryValues);
				queryValues.clear();
				n = 0;
			}
		}
		if (!queryValues.isEmpty())
		{
			queryValues.chop(1);
			ok = query.exec(queryStart + queryValues);
		}
		static_cast<void>(mSqlLiteDB.commit());
		setResult(ok, m_model, queryStart + queryValues, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateMesoCalendarEntry()
{
	if (openDatabase())
	{
		bool ok(false);
		const QDate& date(m_execArgs.at(0).toDate());
		QSqlQuery query{getQuery()};

		QString strQuery(u"SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_s.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day())));
		if (query.exec(strQuery))
		{
			query.first();
			const QString& strId(query.value(0).toString());
			query.finish();
			const QString& strTrainingDay(m_execArgs.at(1).toString());
			const QString& strSplit(m_execArgs.at(2).toString());

			strQuery = u"UPDATE mesocycles_calendar_table SET training_day=%1, training_split=\'%2\' WHERE id=%3"_s
											.arg(strTrainingDay, strSplit, strId);
			ok = query.exec(strQuery);
			if (ok)
				m_model->updateDay(date, strTrainingDay, strSplit);
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateDayIsFinished()
{
	if (openDatabase())
	{
		bool ok(false);
		const QDate& date(m_execArgs.at(0).toDate());
		QSqlQuery query{getQuery()};
		QString strQuery;

		strQuery = u"SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_s.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day()));
		if (query.exec(strQuery))
		{
			query.first();
			const QString& strId(query.value(0).toString());
			query.finish();
			strQuery = u"UPDATE mesocycles_calendar_table SET training_complete=%1 WHERE id=%2"_s
									.arg(m_execArgs.at(1).toBool() ? STR_ONE : STR_ZERO, strId);
			ok = query.exec(strQuery);
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::dayInfo(const QDate& date, QStringList& dayInfoList)
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT meso_id,training_day,training_split,training_complete "
								"FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_s.arg(
								QString::number(date.year()),QString::number(date.month()), QString::number(date.day())));
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				dayInfoList.append(query.value(0).toString());
				dayInfoList.append(query.value(1).toString());
				dayInfoList.append(query.value(2).toString());
				dayInfoList.append(query.value(3).toString());
				ok = true;
			}
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
}

void DBMesoCalendarTable::changeMesoCalendar()
{
	m_model->changeModel(m_execArgs.at(0).toBool(), m_execArgs.at(1).toBool(), m_execArgs.at(2).toDate());
	removeMesoCalendar();
	saveMesoCalendar();
}

void DBMesoCalendarTable::updateMesoCalendar()
{
	m_model->updateModel(m_execArgs.at(0).toDate(), m_execArgs.at(1).toString());
	removeMesoCalendar();
	saveMesoCalendar();
}

void DBMesoCalendarTable::removeMesoCalendar()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"DELETE FROM mesocycles_calendar_table WHERE meso_id="_s + m_model->getMesoId());
		ok = query.exec(strQuery);
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}	
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
