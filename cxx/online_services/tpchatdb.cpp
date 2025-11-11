#include "tpchatdb.h"

#include "tpchat.h"
#include "../dbusermodel.h"
#include "../tputils.h"

#include <QThread>

constexpr QLatin1StringView fieldNames[13][2] {
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

TPChatDB::TPChatDB(const QString &user_id, const QString &otheruser_id, QObject *parent)
	: TPDatabaseTable{CHAT_TABLE_ID, parent}, m_userId{user_id}, m_otherUserId{otheruser_id}
{
	setTableName(tableName());
	m_deleteAfterFinished = false;
	const QLatin1StringView seed{otheruser_id.toLatin1()};
	m_uniqueID = appUtils()->generateUniqueId(seed);
	const QString &cnx_name(QString::number(m_uniqueID));
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	m_sqlLiteDB.setDatabaseName(databaseDir() + databaseFileName());
	#ifndef QT_NOT_DEBUG
	setObjectName("ChatTable");
	#endif
}

QString TPChatDB::databaseDir() const
{
	return appUserModel()->userDir(m_userId) + TPChat::chatsSubDir;
}

QLatin1StringView TPChatDB::createTableQuery()
{
	QString str{std::move("CREATE TABLE IF NOT EXISTS %1 ("_L1)};
	for (uint i{0}; i < TP_CHAT_MESSAGE_FIELDS; ++i)
		str += std::move(fieldNames[i][0] + ' ' + fieldNames[i][1]) + ',';
	str.chop(1);
	str += std::move(");");
	return QLatin1StringView{str.toLatin1().constData(), str.length()};
}

bool TPChatDB::createTable()
{
	if (!QFile::exists(databaseFileName()))
	{
		if (appUtils()->mkdir(databaseDir()))
			return execQuery(createTableQuery().arg(tableName()), false);
	}
	return true;
}

QString TPChatDB::dbFilePath(const uint, const bool path_only)
{
	return appUserModel()->userDir() + TPChat::chatsSubDir + (path_only ? QString{} : databaseFileName());
}

void TPChatDB::loadChat()
{
	if (execQuery("SELECT * FROM "_L1 + tableName(), true, false))
	{
		if (m_workingQuery.first ())
		{
			do
			{
				QStringList message_info{TP_CHAT_MESSAGE_FIELDS};
				for (uint i{0}; i < TP_CHAT_MESSAGE_FIELDS; ++i)
					message_info[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				m_wholeChat.append(std::move(message_info));
			} while (m_workingQuery.next());
		}
	}
	moveToThread(originalThread());
	emit threadFinished();
}

void TPChatDB::insertMessage(const QStringList &message_info)
{
	m_strQuery = std::move(u"INSERT INTO %1 "
			"(msgid,sender,receiver,sdate,stime,rdate,rtime,deleted,sent,received,read,text,media) "
			"VALUES(%2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, \'%13\', \'%14\');"_s
		.arg(tableName(), message_info.at(MESSAGE_ID), message_info.at(MESSAGE_SENDER), message_info.at(MESSAGE_RECEIVER),
			message_info.at(MESSAGE_SDATE), message_info.at(MESSAGE_STIME), message_info.at(MESSAGE_RDATE),
			message_info.at(MESSAGE_RTIME), message_info.at(MESSAGE_DELETED), message_info.at(MESSAGE_SENT),
			message_info.at(MESSAGE_RECEIVED), message_info.at(MESSAGE_READ), message_info.at(MESSAGE_TEXT),
			message_info.at(MESSAGE_MEDIA)));

	static_cast<void>(execQuery(m_strQuery, false));
	moveToThread(originalThread());
	emit threadFinished();
}

void TPChatDB::updateField(const QString &msg_id, const uint field, const QString &new_value)
{
	m_strQuery = std::move("UPDATE "_L1 + tableName() + u"SET %1=%2 WHERE msgid=%3;"_s.arg(
		fieldNames[field][0], field < MESSAGE_TEXT ? new_value : '\'' + new_value + '\'', msg_id));
	execQuery(m_strQuery, false);
	moveToThread(originalThread());
	emit threadFinished(true);
}

void TPChatDB::updateFields(const QStringList &msg_ids, QList<uint> fields, const QStringList &new_values)
{
	if (m_sqlLiteDB.transaction())
	{
		const QString &query_cmd{"UPDATE "_L1 + tableName() + u"SET %1=%2 WHERE msgid=%3;"_s};
		for (uint i{0}; i < msg_ids.count(); ++i)
		{
			m_strQuery = std::move(query_cmd.arg(fieldNames[fields.at(i)][0],
					fields.at(i) < MESSAGE_TEXT ? new_values.at(i) : '\'' + new_values.at(i) + '\'',
					msg_ids.at(i)));
		}
		if (execQuery(m_strQuery, false, false))
			static_cast<void>(m_sqlLiteDB.commit());
	}
	moveToThread(originalThread());
	emit threadFinished(true);
}
