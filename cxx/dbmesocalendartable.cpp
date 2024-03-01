#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QFile>

DBMesoCalendarTable::DBMesoCalendarTable(const QString& dbFilePath, QSettings* appSettings, DBMesoCalendarModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBMesoCalendarObjectName );
	const QString cnx_name( QStringLiteral("db_mesocal_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBMesoCalendarFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < 5; i++)
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
				// 0: id | 1: meso_id | 2: training day number | 3: split letter | 4: year | 5: month | 6: day
				QStringList mesocal_info;
				int month(-1), dbmonth(-1);
				QString strMonth, strYear;
				uint day(1);

				do
				{
					dbmonth = query.value(5).toInt();
					if (dbmonth != month)
					{
						strMonth = QString::number(dbmonth);
						strYear = query.value(4).toString();
						if (!mesocal_info.isEmpty())
						{
							m_model->appendList(mesocal_info);
							mesocal_info.clear();
							day = 1;
						}
						else
						{
							const uint firstDayOfMeso(query.value(6).toUInt());
							if (firstDayOfMeso > 1)
							{
								//Fill the model with info that reflects that these month days are not part of the meso
								for( ; day < firstDayOfMeso; ++day)
									mesocal_info.append( QStringLiteral("-1,-1,-1,'N',") + strYear + ',' + strMonth);
							}
						}
					}
					month = dbmonth;
					mesocal_info.append(query.value(0).toString() + ',' + mesoId + ',' + query.value(2).toString() + ',' +
										query.value(3).toString() + ',' + strYear + ',' + strMonth);
					day++;
				} while (query.next ());
				if (!mesocal_info.isEmpty()) //The days of the last month
				{
					const uint lastDayOfMonth( QDate(strYear.toInt(), strMonth.toInt(), ++day).daysInMonth() );
					//Fill the model with info that reflects that these month days are not part of the meso
					for( ; day <= lastDayOfMonth; ++day)
						mesocal_info.append( QStringLiteral("-1, -1, -1, 'N',") + strYear + ',' + strMonth);
					m_model->appendList(mesocal_info);
				}
			}
		}
		m_model->setReady(true);
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
	resultFunc(static_cast<TPDatabaseTable*>(this));
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
									"(meso_id, training_day, training_split, year, month, day) VALUES ") );
			QString queryValues;
			QStringList day_info;
			uint n(0);

			for (uint i(0), x(0); i < m_model->count(); ++i)
			{
				for (x = 0; x < m_model->getRow_const(i).count(); ++x, ++n)
				{
					day_info = m_model->get(i, x).split(',');
					if (day_info.at(1) != QStringLiteral("-1"))
					{
						queryValues += '(' + day_info.at(1) + ',' + day_info.at(2) + QStringLiteral(",\'") + day_info.at(3) + QStringLiteral("\',") +
									day_info.at(4) + ',' + day_info.at(5) + ',' + QString::number(x+1) + QStringLiteral("),");
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

		if (m_result)
		{
			MSG_OUT("DBMesoCalendarTable createMesoCalendar SUCCESS")
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable createMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable createMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::newMesoCalendarEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"INSERT INTO mesocycles_calendar_table "
									"(meso_id, cal_date, training_day, training_split)"
									" VALUES(%1, %2, %3, \'%4\')")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4)) );
		m_result = query.exec();
		if (m_result)
		{
			MSG_OUT("DBMesoCalendarTable newMesoCalendarEntry SUCCESS")
			m_data[0] = query.lastInsertId().toString();
			if (m_model)
				m_model->appendList(m_data);
			m_opcode = OP_ADD;
		}
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBMesoCalendarTable newMesoCalendarEntry Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable newMesoCalendarEntry Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::updateMesoCalendarEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"UPDATE mesocycles_calendar_table SET meso_id=%1, cal_date=%2, training_day=%3, "
									"training_split=\'%4\' WHERE id=%5")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(0)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry SUCCESS")
		if (m_model)
			m_model->updateList(m_data, m_model->currentRow());
	}
	else
	{
		MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable updateMesoCalendarEntry Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::removeMesoCalendar()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM mesocycles_calendar_table WHERE id=") + m_data.at(0) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		if (m_model)
			m_model->removeFromList(m_model->currentRow());
		MSG_OUT("DBExercisesTable removeMesoCalendar SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoCalendarTable removeMesoCalendar Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBMesoCalendarTable removeMesoCalendar Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::deleteMesoCalendarTable()
{
	QFile mDBFile(mSqlLiteDB.databaseName());
	m_result = mDBFile.remove();
	if (m_result)
	{
		if (m_model)
			m_model->clear();
		m_opcode = OP_DELETE_TABLE;
		MSG_OUT("DBMesoCalendarTable deleteMesoCalendarTable SUCCESS")
	}
	else
	{
		MSG_OUT("DBMesoCalendarTable deleteMesoCalendarTable error: Could not remove file " << mDBFile.fileName())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoCalendarTable::setData(const QString& id, const QString& mesoId, const QString& calDate, const QString& calNDay, const QString& calSplit)
{
	m_data[0] = id;
	m_data[1] = mesoId;
	m_data[2] = calDate;
	m_data[3] = calNDay;
	m_data[4] = calSplit;
}
