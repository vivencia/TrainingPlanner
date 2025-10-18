#pragma once

#include "../tpdatabasetable.h"

#include <QObject>

constexpr uint CHAT_TABLE_ID{0XF0};

class TPChatDB : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit TPChatDB(const QString &user_id, const QString &otheruser_id, QObject *parent);

	inline uint mostRecentMessageId() const { return m_mostRecentMsgId; }
	QList<QStringList> &wholeChat() & { return m_wholeChat; }
	QString databaseFileName() { return m_otherUserId + ".db.sqlite"_L1; }
	QString databaseDir() const;
	inline static QLatin1StringView tableName() { return "chat_table"_L1; }
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void loadChat();
	void saveChat(bool update, const QStringList &message_info);

private:
	uint m_mostRecentMsgId;
	QString m_userId, m_otherUserId;
	QList<QStringList> m_wholeChat;
};

