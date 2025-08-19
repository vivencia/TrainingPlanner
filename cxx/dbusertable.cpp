#include "dbusertable.h"

#include "dbusermodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlQuery>
#include <QTime>

#include <utility>

DBUserTable::DBUserTable(DBUserModel *model)
	: TPDatabaseTable{USERS_TABLE_ID}, m_model{model}
{
	setTableName(tableName());
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name("db_exercises_connection"_L1 + QString::number(m_UniqueID));
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NOT_DEBUG
	setObjectName("UsersTable");
	#endif
}

QLatin1StringView DBUserTable::tableName()
{
	return "users_table"_L1;
}

QLatin1StringView DBUserTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
										"userid INTEGER PRIMARY KEY,"
										"onlineuser INTEGER,"
										"name TEXT,"
										"birthday INTEGER,"
										"sex INTEGER,"
										"phone TEXT,"
										"email TEXT,"
										"social TEXT,"
										"role TEXT,"
										"coach_role TEXT,"
										"goal TEXT,"
										"use_mode INTEGER"
									")"_L1;
}

void DBUserTable::getAllUsers()
{
	if (openDatabase(true))
	{
		bool ok{false};
		QSqlQuery query{std::move(getQuery())};
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
					m_model->addUser(std::move(user_info));
				} while (query.next());
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
		QSqlQuery query{std::move(getQuery())};
		const uint row{m_execArgs.at(0).toUInt()};
		bool bUpdate{false};
		QString strQuery;
		if (query.exec("SELECT userid FROM %1 WHERE userid=%2"_L1.arg(tableName(), m_model->userId(row))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery = std::move(u"UPDATE %1 SET onlineuser=%2, name=\'%3\', birthday=%4, sex=%5, "
								 "phone=\'%6\', email=\'%7\', social=\'%8\', role=\'%9\', coach_role=\'%10\', "
								 "goal=\'%11\', use_mode=%12 WHERE userid=%13"_s
				.arg(tableName(), m_model->_onlineUser(row), m_model->_userName(row), m_model->_birthDate(row),
					m_model->_sex(row), m_model->_phone(row), m_model->_email(row), m_model->_socialMedia(row),
					m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row), m_model->_appUseMode(row),
					m_model->userId(row)));
		}
		else
		{
			strQuery = std::move(u"INSERT INTO %1 "
				"(userid,onlineuser,name,birthday,sex,phone,email,social,role,coach_role,goal,use_mode)"
				" VALUES(%2, %3, \'%4\', %5, %6, \'%7\', \'%8\', \'%9\', \'%10\',\'%11\', \'%12\', %13)"_s
					.arg(tableName(), m_model->userId(row), m_model->_onlineUser(row), m_model->_userName(row),
					m_model->_birthDate(row), m_model->_sex(row), m_model->_phone(row), m_model->_email(row),
					m_model->_socialMedia(row), m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row),
					m_model->_appUseMode(row)));
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
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"DELETE FROM %1 WHERE userid=%2"_L1.arg(tableName(), m_execArgs.at(0).toString())};
		ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
