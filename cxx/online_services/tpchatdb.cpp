#include "tpchatdb.h"

#include "tpchat.h"
#include "../dbusermodel.h"

constexpr int n_fields{TP_CHAT_MESSAGE_FIELDS};
constexpr QLatin1StringView table_name{ "chat_table"_L1 };
constexpr QLatin1StringView field_names[TP_CHAT_MESSAGE_FIELDS][2] {
	{"msgid"_L1,	"INTEGER PRIMARY KEY"_L1},
	{"sender"_L1,	"INTEGER"_L1},
	{"receiver"_L1,	"INTEGER"_L1},
	{"sdate"_L1,	"INTEGER"_L1},
	{"stime"_L1,	"INTEGER"_L1},
	{"rdate"_L1,	"INTEGER"_L1},
	{"rtime"_L1,	"INTEGER"_L1},
	{"deleted"_L1,	"INTEGER"_L1},
	{"sent"_L1,		"INTEGER"_L1},
	{"received"_L1,	"INTEGER"_L1},
	{"read"_L1,		"INTEGER"_L1},
	{"text"_L1,		"TEXT"_L1},
	{"media"_L1,	"TEXT"_L1},
};

TPChatDB::TPChatDB(const QString &user_id, const QString &otheruser_id, DBModelInterfaceChat *dbmodel_interface)
	: TPDatabaseTable{CHAT_TABLE_ID, dbmodel_interface}, m_userId{user_id}, m_otherUserId{otheruser_id}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NOT_DEBUG
	setObjectName("ChatTable");
	#endif
	setReadAllRecordsFunc([this] () { return loadChat(); });
}

QString TPChatDB::dbFilePath() const
{
	return appUserModel()->userDir(m_userId) + TPChat::chatsSubDir;
}

QString TPChatDB::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move(m_otherUserId + dbfile_extension)};
	return fullpath ? dbFilePath() + filename : filename;
}

bool TPChatDB::loadChat()
{
	bool success{false};
	if (execQuery("SELECT * FROM "_L1 + table_name, true, false))
	{
		if (m_workingQuery.first ())
		{

			do
			{
				QStringList message_info{TP_CHAT_MESSAGE_FIELDS};
				for (uint i{0}; i < TP_CHAT_MESSAGE_FIELDS; ++i)
					message_info[i] = std::move(m_workingQuery.value(i).toString());
				m_dbModelInterface->modelData().append(std::move(message_info));
			} while (m_workingQuery.next());
			success = true;
		}
	}
	emit chatLoaded(success);
	return success;
}
