#include "dbusertable.h"
#include "dbusermodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBUserTable::DBUserTable(const QString& dbFilePath, DBUserModel* model)
	: TPDatabaseTable{model}
{
	m_tableName = u"user_table"_qs;
	m_tableID = USER_TABLE_ID;
	setObjectName(DBUserObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString& cnx_name(QStringLiteral("db_exercises_connection") + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), cnx_name);
	const QString& dbname(dbFilePath + DBUserFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBUserTable::createTable()
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
		const QString& strQuery(u"CREATE TABLE IF NOT EXISTS user_table ("
										"id INTEGER PRIMARY KEY,"
										"name TEXT,"
										"birthday INTEGER,"
										"sex TEXT,"
										"phone TEXT,"
										"email TEXT,"
										"social TEXT,"
										"role TEXT,"
										"coach_role TEXT,"
										"goal TEXT,"
										"avatar TEXT,"
										"use_mode INTEGER DEFAULT 1,"
										"current_coach INTEGER, "
										"current_user INTEGER"
									")"_qs);
		m_result = query.exec(strQuery);
		if (!m_result)
		{
			MSG_OUT("DBUserTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery);
		}
		else
			MSG_OUT("DBUserTable createTable SUCCESS")
		mSqlLiteDB.close();
	}
}

void DBUserTable::getAllUsers()
{
	mSqlLiteDB.setConnectOptions(u"QSQLITE_OPEN_READONLY"_qs);
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
		query.setForwardOnly(true);

		if (query.exec(u"SELECT * FROM user_table"_qs))
		{
			if (query.first ())
			{
				QStringList user_info(USER_TOTAL_COLS);
				do
				{
					for (uint i(USER_COL_ID); i < USER_TOTAL_COLS; ++i)
						user_info[i] = query.value(static_cast<int>(i)).toString();
					m_model->appendList(user_info);
				} while (query.next ());
				m_result = m_model->count() > 0;
			}
		}
		m_model->setReady(m_result);
		if (!m_result)
		{
			MSG_OUT("DBUserTable getAllUsers Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable getAllUsers Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBUserTable getAllUsers SUCCESS")
		mSqlLiteDB.close();
	}
}

void DBUserTable::saveUser()
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

		const uint row(m_execArgs.at(0).toUInt());
		bool bUpdate(false);
		QString strQuery;
		if (query.exec(u"SELECT id FROM user_table WHERE id=%1"_qs.arg(m_model->getFast(row, USER_COL_ID))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery = u"UPDATE user_table SET name=\'%1\', birthday=%2, sex=\'%3\', phone=\'%4\', email=\'%5\', social=\'%6\', "
						"role=\'%7\', coach_role=\'%8\', goal=\'%9\', avatar=\'%10\', use_mode=%11, current_coach=%12, current_user=%13 WHERE id=%14"_qs
				.arg(m_model->getFast(row, USER_COL_NAME), m_model->getFast(row, USER_COL_BIRTHDAY), m_model->getFast(row, USER_COL_SEX),
					m_model->getFast(row, USER_COL_PHONE), m_model->getFast(row, USER_COL_EMAIL), m_model->getFast(row, USER_COL_SOCIALMEDIA),
					m_model->getFast(row, USER_COL_USERROLE), m_model->getFast(row, USER_COL_COACHROLE), m_model->getFast(row, USER_COL_GOAL),
					m_model->getFast(row, USER_COL_AVATAR), m_model->getFast(row, USER_COL_APP_USE_MODE), m_model->getFast(row, USER_COL_CURRENT_COACH),
					m_model->getFast(row, USER_COL_CURRENT_USER), m_model->getFast(row, USER_COL_ID));
		}
		else
		{
			strQuery = u"INSERT INTO user_table "
				"(name,birthday,sex,phone,email,social,role,coach_role,goal,avatar,use_mode,current_coach,current_user)"
				" VALUES(\'%1\', %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\',\'%8\', \'%9\', \'%10\', %11, %12, %13)"_qs
					.arg(m_model->getFast(row, USER_COL_NAME), m_model->getFast(row, USER_COL_BIRTHDAY),
					m_model->getFast(row, USER_COL_SEX), m_model->getFast(row, USER_COL_PHONE), m_model->getFast(row, USER_COL_EMAIL),
					m_model->getFast(row, USER_COL_SOCIALMEDIA), m_model->getFast(row, USER_COL_USERROLE), m_model->getFast(row, USER_COL_COACHROLE),
					m_model->getFast(row, USER_COL_GOAL), m_model->getFast(row, USER_COL_AVATAR), m_model->getFast(row, USER_COL_APP_USE_MODE),
					m_model->getFast(row, USER_COL_CURRENT_COACH), m_model->getFast(row, USER_COL_CURRENT_USER));
		}
		m_result = query.exec(strQuery);
		if (m_result)
		{
			m_model->setModified(false);
			if (!bUpdate)
				m_model->setFast(row, USER_COL_ID, query.lastInsertId().toString());
			MSG_OUT("DBUserTable saveUser SUCCESS");
		}
		else
		{
			MSG_OUT("DBUserTable saveUser Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBUserTable saveUser Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery);
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBUserTable saveUser Could not open Database")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
