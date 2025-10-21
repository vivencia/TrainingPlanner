#include "tpchat.h"

#include "tpchatdb.h"
#include "tponlineservices.h"
#include "../dbinterface.h"
#include "../dbusermodel.h"
#include "../dbmesocalendarmanager.h"

using namespace Qt::Literals::StringLiterals;

enum ChatRoleNames {
	idRole			=		Qt::UserRole + MESSAGE_ID,
	senderRole		=		Qt::UserRole + MESSAGE_SENDER,
	receiverRole	=		Qt::UserRole + MESSAGE_RECEIVER,
	sDateRole		=		Qt::UserRole + MESSAGE_SDATE,
	sTimeRole		=		Qt::UserRole + MESSAGE_STIME,
	rDateRole		=		Qt::UserRole + MESSAGE_RDATE,
	rTimeRole		=		Qt::UserRole + MESSAGE_RTIME,
	deletedRole		=		Qt::UserRole + MESSAGE_DELETED,
	sentRole		=		Qt::UserRole + MESSAGE_SENT,
	receivedRole	=		Qt::UserRole + MESSAGE_RECEIVED,
	readRole		=		Qt::UserRole + MESSAGE_READ,
	textRole		=		Qt::UserRole + MESSAGE_TEXT,
	mediaRole		=		Qt::UserRole + MESSAGE_MEDIA,
};

struct ChatMessage {
	uint id;
	QString sender;
	QString receiver;
	QDate sdate, rdate;
	QTime stime, rtime;
	TPBool deleted;
	TPBool sent;
	TPBool received;
	TPBool read;
	QString text;
	QString media;
};

TPChat::TPChat(const QString &otheruser_id, QObject *parent)
	: QAbstractListModel{parent}, m_otherUserId{otheruser_id}
{
	m_roleNames[idRole]			=	std::move("msgId");
	m_roleNames[senderRole]		=	std::move("msgSender");
	m_roleNames[receiverRole]	=	std::move("msgReceiver");
	m_roleNames[sDateRole]		=	std::move("msgSentDate");
	m_roleNames[sTimeRole]		=	std::move("msgSentTime");
	m_roleNames[rDateRole]		=	std::move("msgReceivedDate");
	m_roleNames[rTimeRole]		=	std::move("msgReceivedTime");
	m_roleNames[deletedRole]	=	std::move("msgDeleted");
	m_roleNames[sentRole]		=	std::move("msgSent");
	m_roleNames[receivedRole]	=	std::move("msgReceived");
	m_roleNames[readRole]		=	std::move("msgRead");
	m_roleNames[textRole]		=	std::move("msgText");
	m_roleNames[mediaRole]		=	std::move("msgMedia");

	m_chatDB = new TPChatDB{appUserModel()->userId(0), m_otherUserId, this};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn] (const int _conn_id)
	{
		if (_conn_id == m_chatDB->uniqueId())
		{
			disconnect(*conn);
			QList<QStringList> whole_chat{std::move(m_chatDB->wholeChat())};
			for (auto &&str_message : whole_chat)
			{
				ChatMessage *message = new ChatMessage;
				message->id = str_message.at(MESSAGE_ID).toUInt();
				message->sender = std::move(str_message[MESSAGE_SENDER]);
				message->receiver = std::move(str_message[MESSAGE_RECEIVER]);
				message->sdate = std::move(appUtils()->getDateFromDateString(str_message.at(MESSAGE_SDATE), TPUtils::DF_ONLINE));
				message->stime = std::move(appUtils()->getTimeFromTimeString(str_message.at(MESSAGE_STIME), TPUtils::TF_ONLINE));
				message->rdate = std::move(appUtils()->getDateFromDateString(str_message.at(MESSAGE_RDATE), TPUtils::DF_ONLINE));
				message->rtime = std::move(appUtils()->getTimeFromTimeString(str_message.at(MESSAGE_RTIME), TPUtils::TF_ONLINE));
				message->deleted = str_message.at(MESSAGE_DELETED).toUInt() == 1;
				message->sent = str_message.at(MESSAGE_SENT).toUInt() == 1;
				message->received = str_message.at(MESSAGE_RECEIVED).toUInt() == 1;
				message->read = str_message.at(MESSAGE_READ).toUInt() == 1;
				message->text = std::move(str_message[MESSAGE_TEXT]);
				message->media = std::move(str_message[MESSAGE_MEDIA]);
				str_message.clear();
			};
			whole_chat.clear();
			beginInsertRows(QModelIndex{}, 0, count() - 1);
			emit countChanged();
			endInsertRows();
		}
	});
	m_chatDB->loadChat();
}

void TPChat::incomingMessage(ChatMessage *incoming)
{
	incoming->received = true;
	incoming->rdate = std::move(QDate::currentDate());
	incoming->rtime = std::move(QTime::currentTime());
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(incoming);
	emit countChanged();
	endInsertRows();
	saveChat(incoming);
	appOnlineServices()->chatMessageReceived(appUserModel()->userId(0), m_otherUserId, QString::number(incoming->id));
}

void TPChat::newMessage(const QString &text, const QString &media)
{
	ChatMessage *message = new ChatMessage;
	message->id = m_messages.count();
	message->sender = appUserModel()->userId(0);
	message->receiver = m_otherUserId;
	message->sdate = std::move(QDate::currentDate());
	message->stime = std::move(QTime::currentTime());
	message->text = text;
	message->media = media;
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(message);
	emit countChanged();
	endInsertRows();
	saveChat(message);
	appOnlineServices()->sendMessage(appUserModel()->userId(0), m_otherUserId, std::move(encodeMessageToUpload(message)));
}

void TPChat::updateMessage(ChatMessage *incoming)
{
	delete m_messages.at(incoming->id);
	m_messages[incoming->id] = incoming;
	saveChat(incoming);
}

void TPChat::updateMessage(const uint msgid, const QString &text, const QString &media)
{
	ChatMessage *message{m_messages.at(msgid)};
	message->text = text;
	message->media = media;
	emit dataChanged(index(msgid, 0), index(msgid, 0));
	saveChat(message);
	appOnlineServices()->sendMessage(appUserModel()->userId(0), m_otherUserId, std::move(encodeMessageToUpload(message)));
}

void TPChat::removeMessage(const uint msgid)
{
	ChatMessage *message{m_messages.at(msgid)};
	message->text.clear();
	message->media.clear();
	message->deleted = true;
	emit dataChanged(index(msgid, 0), index(msgid, 0));
	saveChat(message);
}

QString TPChat::encodeMessageToUpload(ChatMessage* message)
{
	return appUtils()->string_strings({
					QString::number(message->id),
					message->sender,
					message->receiver,
					appUtils()->formatDate(message->sdate, TPUtils::DF_ONLINE),
					appUtils()->formatTime(message->stime, TPUtils::TF_ONLINE),
					QString{},
					QString{},
					"0"_L1,
					"1"_L1,
					"0"_L1,
					"0"_L1,
					message->text,
					message->media
	}, record_separator);
}

void TPChat::saveChat(ChatMessage *message)
{
	const bool update{message->deleted ? true : message->id <= m_chatDB->mostRecentMessageId()};
	appDBInterface()->createThread(m_chatDB, [this,message,update] () {
		m_chatDB->saveChat(update, {
					QString::number(message->id),
					message->sender,
					message->receiver,
					appUtils()->formatDate(message->sdate, TPUtils::DF_ONLINE),
					appUtils()->formatTime(message->stime, TPUtils::TF_ONLINE),
					appUtils()->formatDate(message->rdate, TPUtils::DF_ONLINE),
					appUtils()->formatTime(message->rtime, TPUtils::TF_ONLINE),
					message->deleted ? "1"_L1 : "0"_L1,
					message->sent ? "1"_L1 : "0"_L1,
					message->received ? "1"_L1 : "0"_L1,
					message->read ? "1"_L1 : "0"_L1,
					message->text,
					message->media
		});
	});
}

QVariant TPChat::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_messages.count())
	{
		switch (role)
		{
			case idRole: return m_messages.at(row)->id;
			case senderRole: return m_messages.at(row)->sender;
			case receiverRole: return m_messages.at(row)->receiver;
			case sDateRole: return appUtils()->formatDate(m_messages.at(row)->sdate);
			case sTimeRole: return appUtils()->formatTime(m_messages.at(row)->stime);
			case rDateRole: return appUtils()->formatDate(m_messages.at(row)->rdate);
			case rTimeRole: return appUtils()->formatTime(m_messages.at(row)->rtime);
			case deletedRole: return static_cast<bool>(m_messages.at(row)->deleted);
			case sentRole: return static_cast<bool>(m_messages.at(row)->sent);
			case receivedRole: return static_cast<bool>(m_messages.at(row)->received);
			case readRole: return static_cast<bool>(m_messages.at(row)->read);
			case textRole: return m_messages.at(row)->text;
			case mediaRole: return m_messages.at(row)->media;
		}
	}
	return QVariant{};
}

bool TPChat::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_messages.count())
	{
		switch (role)
		{
			case idRole: m_messages.at(row)->id = value.toUInt(); break;
			case senderRole: m_messages.at(row)->sender = std::move(value.toString()); break;
			case receiverRole: m_messages.at(row)->receiver = std::move(value.toString()); break;
			case sDateRole: m_messages.at(row)->sdate = std::move(value.toDate()); break;
			case sTimeRole: m_messages.at(row)->stime = std::move(value.toTime()); break;
			case rDateRole: m_messages.at(row)->rdate = std::move(value.toDate()); break;
			case rTimeRole: m_messages.at(row)->stime = std::move(value.toTime()); break;
			case deletedRole: m_messages.at(row)->deleted = value.toBool(); break;
			case sentRole: m_messages.at(row)->sent = value.toBool(); break;
			case receivedRole: m_messages.at(row)->received = value.toBool(); break;
			case readRole: m_messages.at(row)->read = value.toBool(); break;
			case textRole: m_messages.at(row)->text = std::move(value.toString()); break;
			case mediaRole: m_messages.at(row)->media = std::move(value.toString()); break;
		}
		emit dataChanged(index, index, QList<int>{} << role);
		return true;
	}
	return false;
}
