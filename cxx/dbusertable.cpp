#include "dbusertable.h"

#include "dbusermodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>
#include <QTime>

#include <utility>

DBUserTable::DBUserTable(const QString &dbFilePath, DBUserModel *model)
	: TPDatabaseTable{nullptr}, m_model{model}
{
	m_tableName = std::move("users_table"_L1);
	m_tableID = USERS_TABLE_ID;
	setObjectName(DBUserObjectName);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name("db_exercises_connection"_L1 + QString::number(m_UniqueID));
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	const QString &dbname(dbFilePath + DBUserFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBUserTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS users_table ("
										"userid INTEGER,"
										"name TEXT,"
										"birthday INTEGER,"
										"sex TEXT,"
										"phone TEXT,"
										"email TEXT,"
										"social TEXT,"
										"role TEXT,"
										"coach_role TEXT,"
										"goal TEXT,"
										"use_mode INTEGER,"
										"coaches TEXT, "
										"clients TEXT"
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
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT * FROM users_table ORDER BY ROWID"_L1};
		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				do
				{
					QStringList user_info{USER_TOTAL_COLS};
					for (uint i{USER_COL_ID}; i < USER_TOTAL_COLS; ++i)
						user_info[i] = std::move(query.value(static_cast<int>(i)).toString());
					m_model->addUser_fast(std::move(user_info));
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
		bool ok{false};
		QSqlQuery query{getQuery()};
		const uint row{m_execArgs.at(0).toUInt()};
		bool bUpdate{false};
		QString strQuery;
		if (query.exec("SELECT userid FROM users_table WHERE userid=%1"_L1.arg(m_model->_userId(row))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery = std::move(u"UPDATE users_table SET name=\'%1\', birthday=%2, sex=\'%3\', phone=\'%4\', email=\'%5\', social=\'%6\', "
						"role=\'%7\', coach_role=\'%8\', goal=\'%9\', use_mode=%10, coaches=\'%11\', clients=\'%12\' WHERE userid=%13"_s
				.arg(m_model->_userName(row), m_model->_birthDate(row), m_model->_sex(row), m_model->_phone(row), m_model->_email(row),
					m_model->_socialMedia(row), m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row), m_model->_appUseMode(row),
					m_model->coaches(row), m_model->clients(row), m_model->_userId(row)));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO users_table "
				"(userid,name,birthday,sex,phone,email,social,role,coach_role,goal,use_mode,coaches,clients)"
				" VALUES(%1, \'%2\', %3, \'%4\', \'%5\', \'%6\', \'%7\', \'%8\',\'%9\', \'%10\', %11, \'%12\', \'%13\')"_s
					.arg(m_model->_userId(row), m_model->_userName(row), m_model->_birthDate(row), m_model->_sex(row),
					m_model->_phone(row), m_model->_email(row), m_model->_socialMedia(row), m_model->_userRole(row), m_model->_coachRole(row),
					m_model->_goal(row), m_model->_appUseMode(row), m_model->coaches(row), m_model->clients(row)));
		}
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}


void DBUserTable::removeUser()
{
	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery("DELETE FROM users_table WHERE userid="_L1 + m_execArgs.at(0).toString());
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
