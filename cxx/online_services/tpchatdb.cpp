#include "tpchatdb.h"

#include "tpchat.h"
#include "../tputils.h"

#include <QThread>

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
	return appSettings()->userDir(m_userId) + "chats/"_L1;
}

QLatin1StringView TPChatDB::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
										"msgid INTEGER PRIMARY KEY,"
										"sender INTEGER,"
										"receiver INTEGER,"
										"sdate INTEGER,"
										"stime INTEGER,"
										"rdate INTEGER,"
										"rtime INTEGER,"
										"deleted INTEGER,"
										"sent INTEGER,"
										"received INTEGER,"
										"read INTEGER,"
										"text TEXT,"
										"media TEXT"
									");"_L1;
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

void TPChatDB::saveChat(const bool update, const QStringList &message_info)
{
	if (update)
	{
		m_strQuery = std::move(u"UPDATE %1 SET sender=%2, receiver=%3, sdate=%4, stime=%5, rdate=%6, rtime=%7, deleted=%8, "
							   "sent=%9, received=%10, read=%11, text=\'%12\', media=\'%13\', WHERE msgid=%14;"_s
				.arg(tableName(), message_info.at(MESSAGE_SENDER), message_info.at(MESSAGE_RECEIVER),
					message_info.at(MESSAGE_SDATE), message_info.at(MESSAGE_STIME), message_info.at(MESSAGE_RDATE),
					message_info.at(MESSAGE_RTIME), message_info.at(MESSAGE_DELETED), message_info.at(MESSAGE_SENT),
					message_info.at(MESSAGE_RECEIVED), message_info.at(MESSAGE_READ), message_info.at(MESSAGE_TEXT),
					message_info.at(MESSAGE_MEDIA), message_info.at(MESSAGE_ID)));
	}
	else
	{
		m_strQuery = std::move(u"INSERT INTO %1 "
				"(msgid,sender,receiver,sdate,stime,rdate,rtime,deleted,sent,received,read,text,media) "
				"VALUES(%2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, \'%13\', \'%14\');"_s
			.arg(tableName(), message_info.at(MESSAGE_ID), message_info.at(MESSAGE_SENDER), message_info.at(MESSAGE_RECEIVER),
				message_info.at(MESSAGE_SDATE), message_info.at(MESSAGE_STIME), message_info.at(MESSAGE_RDATE),
				message_info.at(MESSAGE_RTIME), message_info.at(MESSAGE_DELETED), message_info.at(MESSAGE_SENT),
				message_info.at(MESSAGE_RECEIVED), message_info.at(MESSAGE_READ), message_info.at(MESSAGE_TEXT),
				message_info.at(MESSAGE_MEDIA)));
	}
	static_cast<void>(execQuery(m_strQuery, false));
	moveToThread(originalThread());
	emit threadFinished();
}
