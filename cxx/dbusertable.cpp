#include "dbusertable.h"

#include "dbusermodel.h"
#include "tputils.h"

#include <QThread>

DBUserTable::DBUserTable()
	: TPDatabaseTable{USERS_TABLE_ID}
{
	setTableName(tableName());
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name("db_users_connection"_L1 + QString::number(m_uniqueID));
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	m_sqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NOT_DEBUG
	setObjectName("UsersTable");
	#endif
}

QLatin1StringView DBUserTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
										"userid INTEGER PRIMARY KEY,"
										"inserttime INTEGER,"
										"onlineaccount INTEGER,"
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
									");"_L1;
}

void DBUserTable::getAllUsers()
{
	if (execQuery("SELECT * FROM users_table ORDER BY inserttime ASC;"_L1, true, false))
	{
		if (m_workingQuery.first ())
		{
			do
			{
				QStringList user_info{USER_TOTAL_COLS};
				for (uint i{USER_COL_ID}; i < USER_TOTAL_COLS; ++i)
					user_info[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				appUserModel()->addUser(std::move(user_info));
			} while (m_workingQuery.next());
		}
	}
}

void DBUserTable::saveUser()
{
	bool ok{false};
	const uint row{m_execArgs.at(0).toUInt()};

	if (execQuery("SELECT userid FROM %1 WHERE userid=%2;"_L1.arg(tableName(), appUserModel()->userId(row)), true, false))
	{
		bool update{false};
		if (m_workingQuery.first())
			update = m_workingQuery.value(0).toUInt() >= 0;

		if (update)
		{
			m_strQuery = std::move(u"UPDATE %1 SET onlineaccount=%2, name=\'%3\', birthday=%4, sex=%5, "
								 "phone=\'%6\', email=\'%7\', social=\'%8\', role=\'%9\', coach_role=\'%10\', "
								 "goal=\'%11\', use_mode=%12 WHERE userid=%13;"_s
				.arg(tableName(), appUserModel()->_onlineAccount(row), appUserModel()->_userName(row), appUserModel()->_birthDate(row),
					appUserModel()->_sex(row), appUserModel()->_phone(row), appUserModel()->_email(row), appUserModel()->_socialMedia(row),
					appUserModel()->_userRole(row), appUserModel()->_coachRole(row), appUserModel()->_goal(row), appUserModel()->_appUseMode(row),
					appUserModel()->userId(row)));
		}
		else
		{
			m_strQuery = std::move(u"INSERT INTO %1 "
				"(userid,inserttime,onlineaccount,name,birthday,sex,phone,email,social,role,coach_role,goal,use_mode) "
				"VALUES(%2, %3, %4, \'%5\', %6, %7, \'%8\', \'%9\', \'%10\', \'%11\',\'%12\', \'%13\', %14);"_s
					.arg(tableName(), appUserModel()->userId(row), QString::number(QDateTime::currentMSecsSinceEpoch()),
					appUserModel()->_onlineAccount(row), appUserModel()->_userName(row), appUserModel()->_birthDate(row), appUserModel()->_sex(row),
					appUserModel()->_phone(row), appUserModel()->_email(row), appUserModel()->_socialMedia(row), appUserModel()->_userRole(row),
					appUserModel()->_coachRole(row), appUserModel()->_goal(row), appUserModel()->_appUseMode(row)));
		}
		ok = execQuery(m_strQuery, false);
	}
	emit threadFinished(ok);
}


void DBUserTable::removeUser()
{
	const QString &userid{appUserModel()->userId(m_execArgs.at(0).toUInt())};
	m_strQuery = std::move("DELETE FROM %1 WHERE userid=%2;"_L1.arg(tableName(), userid));
	const bool ok{execQuery(m_strQuery, false)};
	emit threadFinished(ok);
}
