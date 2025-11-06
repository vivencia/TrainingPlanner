#include "tpchat.h"

#include "tpchatdb.h"
#include "tponlineservices.h"
#include "../dbinterface.h"
#include "../dbusermodel.h"
#include "../dbmesocalendarmanager.h" //for TPBool
#include "../tpkeychain/tpkeychain.h"

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

	TPBool queued;
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

	m_userIdx = appUserModel()->userIdxFromFieldValue(USER_COL_ID, m_otherUserId);
	m_chatDB = new TPChatDB{appUserModel()->userId(0), m_otherUserId};
	static_cast<void>(m_chatDB->createTable());

	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_idx, const uint field)
	{
		if (user_idx == m_userIdx)
		{
			switch (field)
			{
				case USER_COL_NAME: emit interlocutorNameChanged(); break;
				case USER_COL_AVATAR: emit avatarIconChanged(); break;
			}
		}
		else if (user_idx == 0 && field == USER_MODIFIED_REMOVED)
			m_userIdx = appUserModel()->userIdxFromFieldValue(USER_COL_ID, m_otherUserId);
	});

	connect(appUserModel(), &DBUserModel::canConnectToServerChanged, this, [this] ()
	{
		if (appUserModel()->canConnectToServer() && QFile::exists(tempMessagesFile()))
		{
			appUserModel()->sendFileToServer(tempMessagesFile(), nullptr, QString{}, "chats/", appUserModel()->userId(0), true);
			for (const auto msg : std::as_const(m_messages))
			{
				if (msg->queued)
				{
					msg->queued = false;
					changeSentProperty(msg, true);
				}
			}
		}
	});

	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn] (const int _conn_id)
	{
		if (_conn_id == m_chatDB->uniqueId())
		{
			disconnect(*conn);
			QList<QStringList> whole_chat{std::move(m_chatDB->wholeChat())};
			beginInsertRows(QModelIndex{}, 0, whole_chat.count() - 1);
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
				m_messages.append(message);
			};
			emit dataChanged(index(0, 0), index(count() - 1));
			emit countChanged();
			endInsertRows();
		}
	});
	appDBInterface()->createThread(m_chatDB, [this] () { return m_chatDB->loadChat(); });
}

TPChat::~TPChat()
{
	delete m_chatDB;
}

QString TPChat::interlocutorName() const
{
	return appUserModel()->userName(m_userIdx);
}

QString TPChat::avatarIcon() const
{
	return appUserModel()->avatar(m_userIdx, false);
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
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		if (appUserModel()->canConnectToServer())
		{
			message->sent = true;

			appOnlineServices()->sendMessage(requestid, key, value, m_otherUserId, std::move(encodeMessageToUpload(message)));
		}
		else
		{
			message->queued = true;
			saveMessageToFile(message);
		}
		saveChat(message);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::incomingMessage(const QString &encoded_message)
{
	ChatMessage *message{decodeDownloadedMessage(encoded_message)};
	message->received = true;
	message->rdate = std::move(QDate::currentDate());
	message->rtime = std::move(QTime::currentTime());
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(message);
	emit countChanged();
	endInsertRows();
	saveChat(message);
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		appOnlineServices()->chatMessageReceived(requestid, key, value, m_otherUserId, QString::number(message->id));
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::messageRead(const uint msgid)
{
	ChatMessage *message{m_messages.at(msgid)};
	message->read = true;
	saveChat(message, false);
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		appOnlineServices()->chatMessageRead(requestid, key, value, m_otherUserId, QString::number(message->id));
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::removeMessage(const uint msgid)
{
	ChatMessage *message{m_messages.at(msgid)};
	message->text.clear();
	message->media.clear();
	message->deleted = true;
	emit dataChanged(index(msgid, 0), index(msgid, 0));
	saveChat(message, false);
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		appOnlineServices()->removeMessage(requestid, key, value, m_otherUserId, QString::number(message->id));
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::clearChat()
{
	void beginResetModel();
	m_chatDB->clearTable();
	qDeleteAll(m_messages);
	void endResetModel();
}

inline void TPChat::changeSentProperty(ChatMessage *message, const bool sent)
{
	message->sent = sent;
	emit dataChanged(index(message->id), index(message->id), QList<int>{} << sentRole);
}

inline QString TPChat::tempMessagesFile() const
{
	return m_chatDB->databaseDir() + m_otherUserId + ".msg"_L1;
}

/*
	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender ids)
*/
QString TPChat::encodeMessageToUpload(ChatMessage* message) const
{
	return appUtils()->string_strings({
					QString::number(message->id),
					message->sender,
					message->receiver,
					appUtils()->formatDate(message->sdate, TPUtils::DF_ONLINE),
					appUtils()->formatTime(message->stime, TPUtils::TF_ONLINE),
					QString{},
					QString{},
					STR_ZERO,
					STR_ONE,
					STR_ZERO,
					STR_ZERO,
					message->text,
					message->media
	}, record_separator);
}

ChatMessage* TPChat::decodeDownloadedMessage(const QString &encoded_message)
{
	ChatMessage *new_message{new ChatMessage};
	new_message->id = appUtils()->getCompositeValue(MESSAGE_ID, encoded_message, record_separator).toUInt();
	new_message->sender = std::move(appUtils()->getCompositeValue(MESSAGE_SENDER, encoded_message, record_separator));
	new_message->receiver = std::move(appUtils()->getCompositeValue(MESSAGE_RECEIVER, encoded_message, record_separator));
	new_message->sdate = std::move(appUtils()->getDateFromDateString(
							appUtils()->getCompositeValue(MESSAGE_SDATE, encoded_message, record_separator), TPUtils::DF_ONLINE));
	new_message->rdate = std::move(appUtils()->getDateFromDateString(
							appUtils()->getCompositeValue(MESSAGE_RDATE, encoded_message, record_separator), TPUtils::DF_ONLINE));
	new_message->stime = std::move(appUtils()->getTimeFromTimeString(
							appUtils()->getCompositeValue(MESSAGE_STIME, encoded_message, record_separator), TPUtils::TF_ONLINE));
	new_message->rtime = std::move(appUtils()->getTimeFromTimeString(
							appUtils()->getCompositeValue(MESSAGE_RTIME, encoded_message, record_separator), TPUtils::TF_ONLINE));
	new_message->deleted = appUtils()->getCompositeValue(MESSAGE_DELETED, encoded_message, record_separator) == STR_ONE;
	new_message->sent = appUtils()->getCompositeValue(MESSAGE_SENT, encoded_message, record_separator) == STR_ONE;
	new_message->received = appUtils()->getCompositeValue(MESSAGE_RECEIVED, encoded_message, record_separator) == STR_ONE;
	new_message->read = appUtils()->getCompositeValue(MESSAGE_READ, encoded_message, record_separator) == STR_ONE;
	new_message->text = std::move(appUtils()->getCompositeValue(MESSAGE_TEXT, encoded_message, record_separator));
	new_message->media = std::move(appUtils()->getCompositeValue(MESSAGE_MEDIA, encoded_message, record_separator));
	return new_message;
}

void TPChat::saveChat(ChatMessage *message, const bool insert)
{
	appDBInterface()->createThread(m_chatDB, [this,message,insert] () {
		m_chatDB->saveChat(!insert, {
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

void TPChat::saveMessageToFile(ChatMessage *message) const
{
	QFile* msg_file{appUtils()->openFile(tempMessagesFile(), false, true, true)};
	if (msg_file)
	{
		msg_file->write(encodeMessageToUpload(message).toUtf8().constData());
		msg_file->write(QByteArray{1, set_separator.toLatin1()});
		msg_file->close();
		delete msg_file;
	}
}

QVariant TPChat::data(ChatMessage *message, const uint field) const
{
	switch (field)
	{
		case MESSAGE_ID: return message->id;
		case MESSAGE_SENDER: return message->sender;
		case MESSAGE_RECEIVER: return message->receiver;
		case MESSAGE_SDATE: return message->sdate;
		case MESSAGE_STIME: return message->stime;
		case MESSAGE_RDATE: return message->rdate;
		case MESSAGE_RTIME: return message->rtime;
		case MESSAGE_DELETED: return static_cast<bool>(message->deleted);
		case MESSAGE_SENT: return static_cast<bool>(message->sent);
		case MESSAGE_RECEIVED: return static_cast<bool>(message->received);
		case MESSAGE_READ: return static_cast<bool>(message->read);
		case MESSAGE_TEXT: return message->text;
		case MESSAGE_MEDIA: return message->media;
	}
	return QVariant{};
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
