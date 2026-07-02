#pragma once

#include "../tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(TPChat)

class TPChatDB final: public TPDatabaseTable
{

Q_OBJECT

public:
	explicit TPChatDB(TPChat *chat);

	QString subDir() const override final;
	QString dbFilePath() const override final;
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}
	bool loadChat(void *);
	std::pair<QVariant,QVariant> getNumberOfUnreadMessages();

signals:
	void chatLoaded(const bool success);

private:
	TPChat *m_chat;
};

