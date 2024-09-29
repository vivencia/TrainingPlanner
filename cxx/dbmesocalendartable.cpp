#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBMesoCalendarTable::DBMesoCalendarTable(const QString& dbFilePath, DBMesoCalendarModel* model)
	: TPDatabaseTable(static_cast<TPListModel*>(model))
{
	m_tableName = u"mesocycles_calendar_table"_qs;
	m_tableID = MESOCALENDAR_TABLE_ID;
	setObjectName(DBMesoCalendarObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString cnx_name(u"db_mesocal_connection-"_qs + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_qs, cnx_name);
	const QString dbname(dbFilePath + DBMesoCalendarFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesoCalendarTable::createTable()
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
		m_result = query.exec( u"CREATE TABLE IF NOT EXISTS mesocycles_calendar_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"training_day INTEGER,"
										"training_split TEXT,"
										"training_complete INTEGER,"
										"year INTEGER,"
										"month INTEGER,"
										"day INTEGER"
									")"_qs);
		if (!m_result)
		{
			MSG_OUT("DBMesoCalendarTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBMesoCalendarTable createTable SUCCESS")
		mSqlLiteDB.close();
	}
}

void DBMesoCalendarTable::updateDatabase()
{
	m_result = false;
	QStringList oldTableInfo;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		QString dayInfo;
		query.setForwardOnly(true);
		if (query.exec(u"SELECT * FROM mesocycles_calendar_table"_qs))
		{
			if (query.first ())
			{
				do {
					for (uint i(0); i < 7; ++i)
					{
						dayInfo = query.value(MESOCALENDAR_COL_MESOID).toString() + ',' + query.value(MESOCALENDAR_COL_TRAINING_DAY).toString() +
							u",\'"_qs +	query.value(MESOCALENDAR_COL_SPLITLETTER).toString() + QStringLiteral("\',1,") +
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
						const QString queryStart(u"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES "_qs);
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
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::getMesoCalendar()
{
	mSqlLiteDB.setConnectOptions(u"QSQLITE_OPEN_READONLY"_qs);
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QString& mesoId(m_execArgs.at(0).toString());

		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		query.setForwardOnly(true);
		if (query.exec(u"SELECT * FROM mesocycles_calendar_table WHERE meso_id="_qs + mesoId))
		{
			if (query.first())
			{
				// 0: id | 1: meso_id | 2: training day number | 3: split letter | 4: training_complete | 5: year | 6: month | 7: day
				QStringList mesocal_info;
				int month(-1), dbmonth(-1);
				QString strMonth, strYear;
				uint day(1);

				do
				{
					dbmonth = query.value(MESOCALENDAR_COL_MONTH).toInt();
					if (dbmonth != month)
					{
						strMonth = QString::number(dbmonth);
						strYear = query.value(MESOCALENDAR_COL_YEAR).toString();
						if (!mesocal_info.isEmpty())
						{
							m_model->appendList(mesocal_info);
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
									mesocal_info.append(u"-1,-1,-1,N,-1,"_qs + strYear + ',' + strMonth);
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
					const uint lastDayOfMonth( QDate(strYear.toInt(), strMonth.toInt(), ++day).daysInMonth() );
					//Fill the model with info that reflects that these month days are not part of the meso
					for( ; day <= lastDayOfMonth; ++day)
						mesocal_info.append(u"-1,-1,-1,N,-1,"_qs + strYear + ',' + strMonth);
					m_model->appendList(mesocal_info);
				}
			}
			else
			{
				DBMesoCalendarModel* mesoCalendarModel = static_cast<DBMesoCalendarModel*>(m_model);
				mesoCalendarModel->createModel();
				saveMesoCalendar();
			}
			m_model->setReady(true);
			m_result = true;
		}
		if (!m_result)
		{
			MSG_OUT("DBMesoCalendarTable getAllMesoCalendars Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable getAllMesoCalendars Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBMesoCalendarTable getAllMesoCalendars SUCCESS")
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBMesoCalendarTable getAllMesoCalendars Database error:  Could not open Database")

	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::saveMesoCalendar()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		if (mSqlLiteDB.transaction())
		{
			const QString& queryStart(u"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES "_qs);
			QString queryValues;
			QStringList day_info;
			uint n(0);

			for (uint i(0), x(0); i < m_model->count(); ++i)
			{
				for (x = 0; x < m_model->getRow_const(i).count(); ++x, ++n)
				{
					day_info = m_model->get(i, x).split(',');
					if (day_info.at(MESOCALENDAR_COL_MESOID) != STR_MINUS_ONE)
					{
						queryValues += '(' + day_info.at(MESOCALENDAR_COL_MESOID) + ',' + day_info.at(MESOCALENDAR_COL_TRAINING_DAY) +
							u",\'"_qs + day_info.at(MESOCALENDAR_COL_SPLITLETTER) + u"\',"_qs +
									day_info.at(MESOCALENDAR_COL_TRAININGCOMPLETE) + ',' + day_info.at(MESOCALENDAR_COL_YEAR) + ',' +
									day_info.at(MESOCALENDAR_COL_MONTH) + ',' + QString::number(x+1) + u"),"_qs;
					}
				}
				if (n >= 50)
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
			MSG_OUT("DBMesoCalendarTable saveMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable saveMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBMesoCalendarTable saveMesoCalendar SUCCESS")
		mSqlLiteDB.close();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateMesoCalendarEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QDate& date(m_execArgs.at(0).toDate());

		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		QString strQuery(u"SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_qs.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day())));
		if (query.exec(strQuery))
		{
			query.first();
			const QString& strId(query.value(0).toString());
			query.finish();
			const QString& strTrainingDay(m_execArgs.at(1).toString());
			const QString& strSplit(m_execArgs.at(2).toString());
			const QString& strDayCompleted(m_execArgs.at(3).toString());

			strQuery = u"UPDATE mesocycles_calendar_table SET training_day=%1, "
										"training_split=\'%2\', training_complete=%3 WHERE id=%4"_qs
											.arg(strTrainingDay, strSplit, strDayCompleted, strId);
			m_result = query.exec(strQuery);
			if (m_result)
				static_cast<DBMesoCalendarModel*>(m_model)->updateDay(date, strTrainingDay, strSplit, strDayCompleted);
		}
		if (m_result)
			MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry SUCCESS")
		else
		{
			MSG_OUT("DBMesoCalendarTable saveMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable saveMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery);
		}
		mSqlLiteDB.close();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateDayIsFinished()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QDate& date(m_execArgs.at(0).toDate());
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		QString strQuery;
		strQuery = u"SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_qs.arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day()));
		if (query.exec(strQuery))
		{
			query.first();
			const QString strId(query.value(0).toString());
			query.finish();
			strQuery = u"UPDATE mesocycles_calendar_table SET training_complete=%1 WHERE id=%2"_qs
									.arg(m_execArgs.at(1).toBool() ? u"1"_qs : u"0"_qs, strId);
			m_result = query.exec(strQuery);
		}
		if (m_result)
			MSG_OUT("DBMesoCalendarTable::updateDayIsFinished SUCCESS")
		else
		{
			MSG_OUT("DBMesoCalendarTable::updateDayIsFinished Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable::updateDayIsFinished Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery)
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBMesoCalendarTable::updateDayIsFinished(): could not open database");

	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::dayInfo(const QDate& date, QStringList& dayInfoList)
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		const QString& strQuery(u"SELECT meso_id,training_day,training_split,training_complete "
								"FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3"_qs.arg(
								QString::number(date.year()),QString::number(date.month()), QString::number(date.day())));
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				dayInfoList.append(query.value(0).toString());
				dayInfoList.append(query.value(1).toString());
				dayInfoList.append(query.value(2).toString());
				dayInfoList.append(query.value(3).toString());
			}
			MSG_OUT("DBMesoCalendarTable::isDayFinished SUCCESS")
		}
		else
		{
			MSG_OUT(strQuery)
			MSG_OUT("DBMesoCalendarTable::isDayFinished Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable::isDayFinished Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		mSqlLiteDB.close();
	}
}

void DBMesoCalendarTable::changeMesoCalendar()
{
	static_cast<DBMesoCalendarModel*>(m_model)->changeModel(m_execArgs.at(0).toBool(), m_execArgs.at(1).toBool(), m_execArgs.at(2).toDate());
	removeMesoCalendar();
	saveMesoCalendar();
}

void DBMesoCalendarTable::updateMesoCalendar()
{
	static_cast<DBMesoCalendarModel*>(m_model)->updateModel(m_execArgs.at(0).toDate(), m_execArgs.at(1).toString());
	removeMesoCalendar();
	saveMesoCalendar();
}

void DBMesoCalendarTable::removeMesoCalendar()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		m_result = query.exec(u"DELETE FROM mesocycles_calendar_table WHERE meso_id="_qs + static_cast<DBMesoCalendarModel*>(m_model)->getMesoId());
		if (m_result)
			MSG_OUT("DBExercisesTable removeMesoCalendar SUCCESS")
		else
		{
			MSG_OUT("DBMesoCalendarTable removeMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBMesoCalendarTable removeMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		mSqlLiteDB.close();
	}	
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
