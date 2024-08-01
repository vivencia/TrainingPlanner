#include "dbusertable.h"
#include "dbusermodel.h"
#include "runcommands.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTime>

DBUserTable::DBUserTable(const QString& dbFilePath, QSettings* appSettings, DBUserModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	m_tableName = u"user_table"_qs;
	m_tableID = USER_TABLE_ID;
	setObjectName(DBUserObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString cnx_name(QStringLiteral("db_exercises_connection") + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), cnx_name);
	const QString dbname(dbFilePath + DBUserFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBUserTable::createTable()
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
		query.prepare( QStringLiteral(
									"CREATE TABLE IF NOT EXISTS user_table ("
										"id INTEGER PRIMARY KEY,"
										"name TEXT,"
										"birthday INTEGER,"
										"sex TEXT,"
										"contact TEXT,"
										"role TEXT,"
										"goal TEXT,"
										"coach TEXT"
									")"
								)
		);
		m_result = query.exec();
		if (!m_result)
		{
			MSG_OUT("DBUserTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBUserTable createTable SUCCESS")
		mSqlLiteDB.close();
	}
}

void DBUserTable::getAllUsers()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM user_table") );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList user_info;
				uint i(0);

				do
				{
					for (i = USER_COL_ID; i <= USER_COL_COACH; ++i)
						user_info.append(query.value(static_cast<int>(i)).toString());
					m_model->appendList(user_info);
					user_info.clear();
				} while ( query.next () );
				m_result = m_model->count() > 0;
			}
		}
		m_model->setReady(m_result);
		if (!m_result)
		{
			MSG_OUT("DBUserTable getAllExercises Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable getAllExercises Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBUserTable getAllExercises SUCCESS")
		mSqlLiteDB.close();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBUserTable::saveUser()
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

		const uint row(m_execArgs.at(0).toUInt());
		bool bUpdate(false);
		QString strQuery;
		if (query.exec(QStringLiteral("SELECT id FROM user_table WHERE id=%1").arg(m_model->getFast(row, USER_COL_ID))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery =  QStringLiteral(
				"UPDATE user_table SET name=\'%1\', birthdaye=%2, sex=\'%3\', contact=\'%4\', role=\'%5\', goal=\'%6\', coach=\'%7\' WHERE id=%8")
				.arg(m_model->getFast(row, USER_COL_NAME), m_model->getFast(row, USER_COL_BIRTHDAY), m_model->getFast(row, USER_COL_CONTACT),
					m_model->getFast(row, USER_COL_ROLE), m_model->getFast(row, USER_COL_GOAL), m_model->getFast(row, USER_COL_COACH),
					m_model->getFast(row, USER_COL_ID));
		}
		else
		{
			strQuery = QStringLiteral(
				"INSERT INTO user_table"
				"(id,name,birthday,sex,contact,role,goal,coach)"
				" VALUES(%1, \'%2\', %3, \'%4\', \'%5\', \'%6\', \'%7\', \'%8\')")
					.arg(m_model->getFast(row, USER_COL_ID), m_model->getFast(row, USER_COL_NAME), m_model->getFast(row, USER_COL_BIRTHDAY),
						m_model->getFast(row, USER_COL_CONTACT), m_model->getFast(row, USER_COL_ROLE), m_model->getFast(row, USER_COL_GOAL),
						m_model->getFast(row, USER_COL_COACH));
		}
		m_result = query.exec(strQuery);
		if (m_result)
		{
			m_model->setModified(false);
			if (!bUpdate)
				m_model->setFast(row, USER_COL_ID, query.lastInsertId().toString());
			MSG_OUT("DBUserTable saveExercise SUCCESS");
			MSG_OUT(strQuery);
		}
		else
		{
			MSG_OUT("DBUserTable saveExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable saveExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT("--ERROR--");
			MSG_OUT(strQuery);
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBUserTable saveExercise Could not open Database")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
