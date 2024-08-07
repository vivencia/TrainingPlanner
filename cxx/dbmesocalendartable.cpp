#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QFile>

DBMesoCalendarTable::DBMesoCalendarTable(const QString& dbFilePath, QSettings* appSettings, DBMesoCalendarModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	m_tableName = u"mesocycles_calendar_table"_qs;
	m_tableID = MESOCALENDAR_TABLE_ID;
	setObjectName(DBMesoCalendarObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString cnx_name(QStringLiteral("db_mesocal_connection-") + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), cnx_name);
	const QString dbname(dbFilePath + DBMesoCalendarFileName);
	mSqlLiteDB.setDatabaseName(dbname);
	m_data.reserve(3);
	for(uint i(0); i < 3; i++)
		m_data.append(QString());
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
		m_result = query.exec( QStringLiteral(
									"CREATE TABLE IF NOT EXISTS mesocycles_calendar_table ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"training_day INTEGER,"
										"training_split TEXT,"
										"training_complete INTEGER,"
										"year INTEGER,"
										"month INTEGER,"
										"day INTEGER"
									")")
								);
		mSqlLiteDB.close();
	}
	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoCalendarTable createTable SUCCESS")
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
		if (query.exec( QStringLiteral("SELECT * FROM mesocycles_calendar_table")))
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
						const QString queryStart( QStringLiteral(
									"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES ") );
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
					mSqlLiteDB.close();
				}
			}
		}
	}
	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable updateDatabase Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable updateDatabase Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoCalendarTable updateDatabase SUCCESS")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::getMesoCalendar()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QString mesoId(m_execArgs.at(0).toString());

		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM mesocycles_calendar_table WHERE meso_id=") + mesoId );

		if (query.exec())
		{
			if (query.first ())
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
									mesocal_info.append(QStringLiteral("-1,-1,-1,N,-1,") + strYear + ',' + strMonth);
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
						mesocal_info.append(QStringLiteral("-1,-1,-1,N,-1,") + strYear + ',' + strMonth);
					m_model->appendList(mesocal_info);
				}
			}
		}
		m_model->setReady(true);
		m_opcode = OP_READ;
		mSqlLiteDB.close();
		m_result = true;
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable getAllMesoCalendars Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable getAllMesoCalendars Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoCalendarTable getAllMesoCalendars SUCCESS")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::createMesoCalendar()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));
		if (mSqlLiteDB.transaction())
		{
			const QString queryStart(QStringLiteral(
									"INSERT INTO mesocycles_calendar_table "
									"(meso_id, training_day, training_split, training_complete, year, month, day) VALUES ") );
			QString queryValues;
			QStringList day_info;
			uint n(0);

			for (uint i(0), x(0); i < m_model->count(); ++i)
			{
				for (x = 0; x < m_model->getRow_const(i).count(); ++x, ++n)
				{
					day_info = m_model->get(i, x).split(',');
					if (day_info.at(MESOCALENDAR_COL_MESOID) != QStringLiteral("-1"))
					{
						queryValues += '(' + day_info.at(MESOCALENDAR_COL_MESOID) + ',' + day_info.at(MESOCALENDAR_COL_TRAINING_DAY) +
							QStringLiteral(",\'") + day_info.at(MESOCALENDAR_COL_SPLITLETTER) + QStringLiteral("\',") +
									day_info.at(MESOCALENDAR_COL_TRAININGCOMPLETE) + ',' + day_info.at(MESOCALENDAR_COL_YEAR) + ',' +
									day_info.at(MESOCALENDAR_COL_MONTH) + ',' + QString::number(x+1) + QStringLiteral("),");
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
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable createMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable createMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoCalendarTable createMesoCalendar SUCCESS")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateMesoCalendarEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QDate date(m_execArgs.at(0).toDate());
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3").arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day())) );
		if (query.exec())
		{
			query.first();
			const QString strId(query.value(0).toString());
			query.finish();
			query.prepare( QStringLiteral(
									"UPDATE mesocycles_calendar_table SET training_day=%1, "
									"training_split=\'%2\', training_complete=%3 WHERE id=%4")
									.arg(m_data.at(0), m_data.at(1), m_data.at(2), strId) );
			m_result = query.exec();
			if (m_result)
			{
				static_cast<DBMesoCalendarModel*>(m_model)->updateDay(date, m_data.at(0), m_data.at(1), m_data.at(2));
				MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry SUCCESS")
			}
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateDayIsFinished()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		const QDate date(m_execArgs.at(0).toDate());
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("SELECT id FROM mesocycles_calendar_table WHERE year=%1 AND month=%2 AND day=%3").arg(
				QString::number(date.year()),QString::number(date.month()), QString::number(date.day())) );
		if (query.exec())
		{
			query.first();
			const QString strId(query.value(0).toString());
			query.finish();
			query.prepare( QStringLiteral(
									"UPDATE mesocycles_calendar_table SET training_complete=%1 WHERE id=%2")
									.arg(m_execArgs.at(1).toBool() ? u"1"_qs : u"0"_qs, strId) );
			m_result = query.exec();
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable updateDayIsFinished Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable updateDayIsFinished Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBMesoCalendarTable updateDayIsFinished SUCCESS")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::changeMesoCalendar()
{
	static_cast<DBMesoCalendarModel*>(m_model)->changeModel(m_execArgs.at(0).toUInt(), m_execArgs.at(1).toDate(),
			m_execArgs.at(2).toDate(), m_execArgs.at(3).toString(), m_execArgs.at(4).toBool(), m_execArgs.at(5).toBool());
	removeMesoCalendar();
	createMesoCalendar();
}

void DBMesoCalendarTable::updateMesoCalendar()
{
	static_cast<DBMesoCalendarModel*>(m_model)->updateModel(m_execArgs.at(1).toString(), m_execArgs.at(2).toDate(),
										m_execArgs.at(3).toString());
	removeMesoCalendar();
	createMesoCalendar();
}

void DBMesoCalendarTable::removeMesoCalendar()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM mesocycles_calendar_table WHERE meso_id=") + m_execArgs.at(0).toString() );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
		MSG_OUT("DBExercisesTable removeMesoCalendar SUCCESS")
	else
	{
		MSG_OUT("DBMesoCalendarTable removeMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable removeMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::setData(const QString& calNDay, const QString& calSplit, const QString& dayIsFinished)
{
	m_data[0] = calNDay;
	m_data[1] = calSplit;
	m_data[2] = dayIsFinished;
}
