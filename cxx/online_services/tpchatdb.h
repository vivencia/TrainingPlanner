#pragma once

#include "../tpdatabasetable.h"

#include <QObject>

constexpr uint CHAT_TABLE_ID{0x0007};

class TPChatDB : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit TPChatDB(const QString &user_id, const QString &otheruser_id, QObject *parent = nullptr);

	QList<QStringList> &wholeChat() & { return m_wholeChat; }
	QString databaseFileName() { return m_otherUserId + ".db.sqlite"_L1; }
	QString databaseDir() const;
	inline static QLatin1StringView tableName() { return "chat_table"_L1; }
	static QLatin1StringView createTableQuery();
	[[maybe_unused]] bool createTable() override final;

	void updateTable() override final {}
	void loadChat();
	void saveChat(const bool update, const QStringList &message_info);
	void updateFields(const QStringList &msg_ids, QList<uint> fields, const QStringList &new_values);

private:
	QString m_userId, m_otherUserId;
	QList<QStringList> m_wholeChat;
};

