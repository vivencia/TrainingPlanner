#include "dbusertable.h"

#include "dbusermodel.h"
#include "tputils.h"

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
				m_model->addUser(std::move(user_info));
			} while (m_workingQuery.next());
		}
	}
}

void DBUserTable::saveUser()
{
	const uint row{m_execArgs.at(0).toUInt()};
	if (execQuery("SELECT userid FROM %1 WHERE userid=%2;"_L1.arg(tableName(), m_model->userId(row)), true, false))
	{
		bool update{false};
		if (m_workingQuery.first())
			update = m_workingQuery.value(0).toUInt() >= 0;

		if (update)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			m_strQuery = std::move(u"UPDATE %1 SET onlineaccount=%2, name=\'%3\', birthday=%4, sex=%5, "
								 "phone=\'%6\', email=\'%7\', social=\'%8\', role=\'%9\', coach_role=\'%10\', "
								 "goal=\'%11\', use_mode=%12 WHERE userid=%13;"_s
				.arg(tableName(), m_model->_onlineAccount(row), m_model->_userName(row), m_model->_birthDate(row),
					m_model->_sex(row), m_model->_phone(row), m_model->_email(row), m_model->_socialMedia(row),
					m_model->_userRole(row), m_model->_coachRole(row), m_model->_goal(row), m_model->_appUseMode(row),
					m_model->userId(row)));
		}
		else
		{
			m_strQuery = std::move(u"INSERT INTO %1 "
				"(userid,inserttime,onlineaccount,name,birthday,sex,phone,email,social,role,coach_role,goal,use_mode) "
				"VALUES(%2, %3, %4, \'%5\', %6, %7, \'%8\', \'%9\', \'%10\', \'%11\',\'%12\', \'%13\', %14);"_s
					.arg(tableName(), m_model->userId(row), QString::number(QDateTime::currentMSecsSinceEpoch()),
					m_model->_onlineAccount(row), m_model->_userName(row), m_model->_birthDate(row), m_model->_sex(row),
					m_model->_phone(row), m_model->_email(row), m_model->_socialMedia(row), m_model->_userRole(row),
					m_model->_coachRole(row), m_model->_goal(row), m_model->_appUseMode(row)));
		}
		const bool success{execQuery(m_strQuery, false)};
		emit queryExecuted(success, true);
	}
}


void DBUserTable::removeUser()
{
	m_strQuery = std::move("DELETE FROM %1 WHERE userid=%2;"_L1.arg(tableName(), m_execArgs.at(0).toString()));
	const bool success{execQuery(m_strQuery, false)};
	emit queryExecuted(success, true);
}
