#include "dbusertable.h"

#include "dbusermodel.h"

constexpr int n_fields{USER_TOTAL_COLS};
constexpr QLatin1StringView table_name{ "users_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"userid"_L1,			"INTEGER PRIMARY KEY"_L1},
	{"inserttime"_L1,		"INTEGER"_L1},
	{"onlineaccount"_L1,	"INTEGER"_L1},
	{"name"_L1,				"TEXT"_L1},
	{"birthday"_L1,			"INTEGER"_L1},
	{"sex"_L1,				"INTEGER"_L1},
	{"phone"_L1,			"TEXT"_L1},
	{"email"_L1,			"TEXT"_L1},
	{"social"_L1,			"TEXT"_L1},
	{"role"_L1,				"TEXT"_L1},
	{"coach_role"_L1,		"TEXT"_L1},
	{"goal"_L1,				"TEXT"_L1},
	{"use_mode"_L1,			"INTEGER"_L1},
};

DBUserTable::DBUserTable(DBModelInterfaceUser *dbmodel_interface)
	: TPDatabaseTable{USERS_TABLE_ID, dbmodel_interface}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NOT_DEBUG
	setObjectName("UsersTable");
	#endif
	setReadAllRecordsFunc([this] () { return getAllUsers(); });
}

QString DBUserTable::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move("Users"_L1 + dbfile_extension)};
	return fullpath ? dbFilePath() + filename : filename;
}

bool DBUserTable::getAllUsers()
{
	bool success{false};
	if (execReadOnlyQuery("SELECT * FROM users_table ORDER BY inserttime ASC;"_L1))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList user_info{USER_TOTAL_COLS};
				for (uint i{USER_COL_ID}; i < USER_TOTAL_COLS; ++i)
					user_info[i] = std::move(m_workingQuery.value(i).toString());
				emit userInfoAcquired(user_info);
			} while (m_workingQuery.next());
			emit userInfoAcquired(QStringList{}, true);
			success = true;
		}
	}
	return success;
}
