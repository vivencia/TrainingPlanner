#pragma once

#include "../tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceChat)

class TPChatDB final: public TPDatabaseTable
{

Q_OBJECT

public:
	explicit TPChatDB(const QString &user_id, const QString &otheruser_id, DBModelInterfaceChat *dbmodel_interface);

	QString subDir() const override final;
	QString dbFilePath() const override final;
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}
	bool loadChat();
	std::pair<QVariant,QVariant> getNumberOfUnreadMessages();

signals:
	void chatLoaded(const bool success);

private:
	QString m_userId, m_otherUserId;
};

