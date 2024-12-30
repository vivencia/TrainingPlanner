#include "dbusertable.h"
#include "dbusermodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlQuery>
#include <QTime>

#include <utility>

DBUserTable::DBUserTable(const QString& dbFilePath, DBUserModel* model)
	: TPDatabaseTable{nullptr}, m_model{model}
{
	m_tableName = std::move("user_table"_L1);
	m_tableID = USER_TABLE_ID;
	setObjectName(DBUserObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString& cnx_name("db_exercises_connection"_L1 + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString& dbname(dbFilePath + DBUserFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBUserTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString& strQuery{"CREATE TABLE IF NOT EXISTS user_table ("
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
									")"_L1
		};
		const bool ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBUserTable::getAllUsers()
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery{"SELECT * FROM user_table"_L1};
		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				do
				{
					QStringList user_info(USER_TOTAL_COLS);
					for (uint i(USER_COL_ID); i < USER_TOTAL_COLS; ++i)
						user_info[i] = std::move(query.value(static_cast<int>(i)).toString());
					m_model->appendList_fast(std::move(user_info));
				} while (query.next());
				m_model->setReady(true);
				ok = true;
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBUserTable::saveUser()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const uint row(m_execArgs.at(0).toUInt());
		bool bUpdate(false);
		QString strQuery;
		if (query.exec("SELECT id FROM user_table WHERE id=%1"_L1.arg(m_model->_userId(row))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery = std::move(u"UPDATE user_table SET name=\'%1\', birthday=%2, sex=\'%3\', phone=\'%4\', email=\'%5\', social=\'%6\', "
						"role=\'%7\', coach_role=\'%8\', goal=\'%9\', avatar=\'%10\', use_mode=%11, current_coach=%12, current_user=%13 WHERE id=%14"_s
				.arg(m_model->_userName(row), m_model->_birthDate(row), m_model->_sex(row), m_model->_phone(row), m_model->_email(row),
					m_model->_socialMedia(row), m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row), m_model->_avatar(row),
					m_model->_appUseMode(row), m_model->_currentCoach(row), m_model->_currentClient(), m_model->_userId(row)));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO user_table "
				"(name,birthday,sex,phone,email,social,role,coach_role,goal,avatar,use_mode,current_coach,current_user)"
				" VALUES(\'%1\', %2, \'%3\', \'%4\', \'%5\', \'%6\', \'%7\',\'%8\', \'%9\', \'%10\', %11, %12, %13)"_s
					.arg(m_model->_userName(row), m_model->_birthDate(row), m_model->_sex(row), m_model->_phone(row), m_model->_email(row),
					m_model->_socialMedia(row), m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row), m_model->_avatar(row),
					m_model->_appUseMode(row), m_model->_currentCoach(row), m_model->_currentClient()));
		}
		ok = query.exec(strQuery);
		if (ok && !bUpdate)
			m_model->setUserId(row, query.lastInsertId().toString());
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
