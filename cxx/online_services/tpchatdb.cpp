#include "tpchatdb.h"

#include "tpchat.h"
#include "../dbusermodel.h"

constexpr int n_fields{TPChat::TPCHAT_N_FIELDS};
constexpr QLatin1StringView table_name{ "chat_table"_L1 };
constexpr QLatin1StringView field_names[TPChat::TPCHAT_N_FIELDS][2] {
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
	{"queued"_L1,	"TEXT"_L1},
};

TPChatDB::TPChatDB(TPChat *chat)
	: TPDatabaseTable{chat->userId().last(5).toUInt(), chat->m_dbModelInterface}
	, m_chat{chat}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NOT_DEBUG
	setObjectName("ChatTable");
	#endif
	setReadAllRecordsFunc<void>([this] (void *param) { return loadChat(param); });
}

QString TPChatDB::subDir() const
{
	return m_chat->chatSubDir();
}

QString TPChatDB::dbFilePath() const
{
	return appUserModel()->mainUserDir() % subDir();
}

QString TPChatDB::dbFileName(const bool fullpath) const
{
	const QString &filename{"chat"_L1 % dbfile_extension};
	return fullpath ? dbFilePath() % filename : filename;
}

bool TPChatDB::loadChat(void *)
{
	bool success{false};
	if (execReadOnlyQuery("SELECT * FROM "_L1 + table_name)) {
		if (m_workingQuery.first ()) {
			do {
				QStringList message_info{TPChat::TPCHAT_N_FIELDS};
				for (uint i{0}; i < TPChat::TPCHAT_N_FIELDS; ++i)
					message_info[i] = std::move(m_workingQuery.value(i).toString());
				m_dbModelInterface->modelData().append(std::move(message_info));
			} while (m_workingQuery.next());
			success = true;
		}
	}
	emit chatLoaded(success);
	return success;
}

std::pair<QVariant,QVariant> TPChatDB::getNumberOfUnreadMessages()
{
	bool success{false};
	QString unread_ids;
	m_strQuery = std::move("SELECT %1 FROM %2 WHERE %3=%4 AND %5=\'%6\';"_L1.arg(
			field_names[TPChat::ID][0], table_name,
			field_names[TPChat::READ][0], "0"_L1,
			field_names[TPChat::RECEIVER][0], m_chat->userId()));
	if (execReadOnlyQuery(m_strQuery)) {
		if (m_workingQuery.first()) {
			success = true;
			do {
				unread_ids.append(m_workingQuery.value(0).toString() % set_separator);
			} while (m_workingQuery.next());
		}
	}
	return std::pair<QVariant,QVariant>{success, unread_ids};
}
