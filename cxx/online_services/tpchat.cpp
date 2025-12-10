#include "tpchat.h"

#include "tpchatdb.h"
#include "tponlineservices.h"
#include "../thread_manager.h"
#include "../dbusermodel.h"
#include "../tpbool.h"
#include "../tpkeychain/tpkeychain.h"

#include <QTimer>

#include <ranges>

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

TPChat::TPChat(const QString &otheruser_id, const bool check_unread_messages, QObject *parent)
	: QAbstractListModel{parent}, m_otherUserId{otheruser_id}, m_unreadMessages{0}, m_chatWindow{nullptr}, m_chatLoaded{false}
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
	m_dbModelInterface = new DBModelInterfaceChat{this};
	m_db = new TPChatDB{appUserModel()->userId(0), m_otherUserId, m_dbModelInterface};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);

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
			appUserModel()->sendFileToServer(tempMessagesFile(), nullptr, QString{}, m_db->subDir(), appUserModel()->userId(0), true);
			for (const auto msg : std::as_const(m_messages))
			{
				if (msg->queued)
				{
					msg->queued = false;
					setData(index(msg->id), true, sentRole);
				}
			}
		}
	});

	if (check_unread_messages)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_db, &TPDatabaseTable::actionFinished, this, [this,conn]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2)
		{
			if (op == ThreadManager::CustomOperation)
			{
				disconnect(*conn);
				if (return_value1.toBool())
					setUnreadMessages(return_value2.toUInt());
			}
		});
		auto x = [this] () -> std::pair<QVariant,QVariant> { return m_db->getNumberOfUnreadMessages(); };
		m_db->setCustQueryFunction(x);
		appThreadManager()->runAction(m_db, ThreadManager::CustomOperation);
	}
}

void TPChat::loadChat()
{
	if (m_chatLoaded)
		return;

	connect(m_db, &TPChatDB::chatLoaded, this, [this] (const bool success)
	{
		if (success)
		{
			beginInsertRows(QModelIndex{}, 0, m_dbModelInterface->modelData().count() - 1);
			uint unread_messages{0};
			for (const auto &str_message : std::as_const(m_dbModelInterface->modelData()))
			{
				ChatMessage *message = new ChatMessage;
				message->id = str_message.at(MESSAGE_ID).toUInt();
				message->sender = str_message.at(MESSAGE_SENDER);
				message->receiver = str_message.at(MESSAGE_RECEIVER);
				message->sdate = std::move(appUtils()->getDateFromDateString(str_message.at(MESSAGE_SDATE), TPUtils::DF_ONLINE));
				message->stime = std::move(appUtils()->getTimeFromTimeString(str_message.at(MESSAGE_STIME), TPUtils::TF_ONLINE));
				message->rdate = std::move(appUtils()->getDateFromDateString(str_message.at(MESSAGE_RDATE), TPUtils::DF_ONLINE));
				message->rtime = std::move(appUtils()->getTimeFromTimeString(str_message.at(MESSAGE_RTIME), TPUtils::TF_ONLINE));
				message->deleted = str_message.at(MESSAGE_DELETED).toUInt() == 1;
				message->sent = str_message.at(MESSAGE_SENT).toUInt() == 1;
				message->received = str_message.at(MESSAGE_RECEIVED).toUInt() == 1;
				message->read = str_message.at(MESSAGE_READ).toUInt() == 1;
				message->text = str_message.at(MESSAGE_TEXT);
				message->media = str_message.at(MESSAGE_MEDIA);
				m_messages.append(message);
				if (!message->read && (message->sender != appUserModel()->userId(0)))
					++unread_messages;
			};
			emit dataChanged(index(0, 0), index(count() - 1));
			emit countChanged();
			if (unread_messages > 0)
				setUnreadMessages(unread_messages);
			endInsertRows();
		}
		m_chatLoaded = true;
		m_sendMessageTimer->start(0);
	});
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	m_sendMessageTimer = new QTimer{this};
	m_sendMessageTimer->setInterval(10000);
	m_sendMessageTimer->callOnTimeout([this] () {
		if (!m_dbModelInterface->modifiedIndices().isEmpty())
			appThreadManager()->runAction(m_db, ThreadManager::alterRecords);
		m_sendMessageTimer->stop();
	});
}

QString TPChat::interlocutorName() const
{
	return appUserModel()->userName(m_userIdx);
}

QString TPChat::avatarIcon() const
{
	return appUserModel()->avatar(m_userIdx, false);
}

void TPChat::setSentMessageReceived(const uint msgid, const bool notify_server)
{
	if (m_chatLoaded)
	{
		setData(index(msgid), true, receivedRole);
		if (notify_server)
			acknowledgeAcknowledgement(msgid, messageWorkReceived);
	}
}

void TPChat::setSentMessageRead(const uint msgid, const bool notify_server)
{
	if (m_chatLoaded)
	{
		setData(index(msgid), true, readRole);
		if (notify_server)
			acknowledgeAcknowledgement(msgid, messageWorkRead);
	}
}

//When called from ChatWindow: deletes the message locally and, if it is a sent message, insctruct the other party to have
//this message that they received removed. If it is a received message, keep the alteration local only
//When called from appMessagesManager(), message->sender will be equal to m_otherUserId, so it will be a received message:
//in this case, delete the message locally only
void TPChat::removeMessage(const uint msgid, const bool remove_for_interlocutor, const bool from_qml)
{
	if (!m_chatLoaded)
		return;

	ChatMessage *message{m_messages.at(msgid)};
	message->text.clear();
	message->media.clear();
	message->deleted = true;
	emit dataChanged(index(msgid, 0), index(msgid, 0));
	updateFieldToSave(msgid, MESSAGE_DELETED, "1"_L1);

	if (remove_for_interlocutor && message->sender == appUserModel()->userId())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [=,this] (const QString &key, const QString &value)
		{
			const int requestid{appUtils()->generateRandomNumber(0, 5000)};
			appOnlineServices()->chatMessageAcknowledgement(requestid, key, value, m_otherUserId,
													QString::number(message->id) + set_separator, messageWorkRemoved);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(appUserModel()->userId(0));
	}
	else if (!remove_for_interlocutor && !from_qml && message->sender == m_otherUserId)
		acknowledgeAcknowledgement(msgid, messageWorkRemoved);
}

void TPChat::setUnreadMessages(const int n_unread)
{
	switch (n_unread)
	{
		case -1: --m_unreadMessages; break;
		case -2: ++m_unreadMessages; break;
		default: m_unreadMessages = n_unread; break;
	}
	emit unreadMessagesChanged(m_unreadMessages);
}

void TPChat::markAllIncomingMessagesRead()
{
	QStringList msg_ids;
	for (const auto message : std::as_const(m_messages) | std::views::reverse)
	{
		if (message->read || message->sender == appUserModel()->userId(0))
			break;
		msg_ids.append(std::move(QString::number(message->id)));
		setSentMessageRead(message->id, true);
	}
	if (!msg_ids.isEmpty())
	{
		setUnreadMessages(0);
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,msg_ids] (const QString &key, const QString &value)
		{
			const int requestid{appUtils()->generateRandomNumber(0, 5000)};
			appOnlineServices()->chatMessageAcknowledgement(requestid, key, value, m_otherUserId,
												msg_ids.join(set_separator) + set_separator, messageWorkRead);
		}, Qt::SingleShotConnection);
		appKeyChain()->readKey(appUserModel()->userId(0));
	}
}

void TPChat::createNewMessage(const QString &text, const QString &media)
{
	beginInsertRows(QModelIndex{}, count(), count());
	ChatMessage *message{new ChatMessage};
	message->id = m_messages.count();
	message->sender = appUserModel()->userId(0);
	message->receiver = m_otherUserId;
	message->sdate = std::move(QDate::currentDate());
	message->stime = std::move(QTime::currentTime());
	message->text = text;
	message->media = media;
	m_messages.append(message);
	emit countChanged();
	endInsertRows();

	encodeMessageToSave(message);

	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		if (appUserModel()->canConnectToServer())
		{
			setData(index(message->id), true, sentRole);
			appOnlineServices()->sendMessage(requestid, key, value, m_otherUserId, std::move(encodeMessageToUpload(message)));
		}
		else
		{
			message->queued = true;
			saveMessageToFile(message);
		}
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::incomingMessage(const QString &encoded_message)
{
	ChatMessage *message{decodeDownloadedMessage(encoded_message)};
	if (!message)
		return;
	message->received = true;
	message->rdate = std::move(QDate::currentDate());
	message->rtime = std::move(QTime::currentTime());
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(message);
	setUnreadMessages(-1);
	emit countChanged();
	endInsertRows();

	encodeMessageToSave(message);
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,message] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		appOnlineServices()->chatMessageAcknowledgement(requestid, key, value, m_otherUserId,
										QString::number(message->id) + set_separator, messageWorkReceived);
		if (m_chatWindow && m_chatWindow->property("activeFocus").toBool())
		{
			appOnlineServices()->chatMessageAcknowledgement(requestid, key, value, m_otherUserId,
											QString::number(message->id) + set_separator, messageWorkRead);
			setUnreadMessages(0);
		}
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPChat::clearChat()
{
	void beginResetModel();
	m_db->clearTable();
	qDeleteAll(m_messages);
	void endResetModel();
}

void TPChat::acknowledgeAcknowledgement(const uint msgid, const QLatin1StringView &work)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,msgid,work] (const QString &key, const QString &value)
	{
		const int requestid{appUtils()->generateRandomNumber(0, 5000)};
		appOnlineServices()->chatMessageAcknowledgmentAcknowledged(requestid, key, value, m_otherUserId, QString::number(msgid), work);
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
}

inline QString TPChat::tempMessagesFile() const
{
	return m_db->dbFileName() + m_otherUserId + messageFileExtension;
}

//record_separator(oct 036, dec 30) separates the message fields
//set_separator (oct 037, dec 31) separates messages of the same sender
//exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and
//the odd numbers are the sender ids)
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

void TPChat::encodeMessageToSave(ChatMessage* message)
{
	const uint modified_row{static_cast<uint>(m_dbModelInterface->modelData().count())};
	m_dbModelInterface->modelData().append(std::move(QStringList{
					std::move(QString::number(message->id)),
					message->sender,
					message->receiver,
					std::move(appUtils()->formatDate(message->sdate, TPUtils::DF_ONLINE)),
					std::move(appUtils()->formatTime(message->stime, TPUtils::TF_ONLINE)),
					std::move(appUtils()->formatDate(message->rdate, TPUtils::DF_ONLINE)),
					std::move(appUtils()->formatTime(message->rtime, TPUtils::TF_ONLINE)),
					std::move(message->deleted ? "1"_L1 : "0"_L1),
					std::move(message->sent ? "1"_L1 : "0"_L1),
					std::move(message->received ? "1"_L1 : "0"_L1),
					std::move(message->read ? "1"_L1 : "0"_L1),
					message->text,
					message->media
	}));
	updateFieldToSave(modified_row, -1, QString{});
}

void TPChat::updateFieldToSave(const uint msg_id, const int field, const QString &value) const
{
	if (field >= 0)
		m_dbModelInterface->modelData()[msg_id][field] = value;
	m_dbModelInterface->setModified(msg_id, field);
	if (m_chatLoaded)
		m_sendMessageTimer->start();
}

ChatMessage* TPChat::decodeDownloadedMessage(const QString &encoded_message)
{
	uint id{appUtils()->getCompositeValue(MESSAGE_ID, encoded_message, record_separator).toUInt()};
	for (const auto msg : std::as_const(m_messages))
	{
		if (msg->id == id)
			return nullptr;
	}

	ChatMessage *new_message{new ChatMessage};
	new_message->id = id;
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
			case idRole:
			{
				bool ok{false};
				const int id{value.toInt(&ok)};
				if (!ok || m_messages.at(row)->id == id)
					return false;
				m_messages.at(row)->id = id;
			}
			break;
			case senderRole:
			{
				QString sender{std::move(value.toString())};
				if (sender == m_messages.at(row)->sender)
					return false;
				m_messages.at(row)->sender = std::move(sender);
			}
			break;
			case receiverRole:
			{
				QString receiver{std::move(value.toString())};
				if (receiver == m_messages.at(row)->receiver)
					return false;
				m_messages.at(row)->receiver = std::move(receiver);
			}
			break;
			case sDateRole:
			{
				QDate date{std::move(value.toDate())};
				if (date == m_messages.at(row)->sdate)
					return false;
				m_messages.at(row)->sdate = std::move(date);
			}
			break;
			case sTimeRole:
			{
				QTime time{std::move(value.toTime())};
				if (time == m_messages.at(row)->stime)
					return false;
				m_messages.at(row)->stime = std::move(time);
			}
			break;
			case rDateRole:
			{
				QDate date{std::move(value.toDate())};
				if (date == m_messages.at(row)->rdate)
					return false;
				m_messages.at(row)->rdate = std::move(date);
			}
			break;
			case rTimeRole:
			{
				QTime time{std::move(value.toTime())};
				if (time == m_messages.at(row)->rtime)
					return false;
				m_messages.at(row)->rtime = std::move(time);
			}
			break;
			case deletedRole:
			{
				bool val{value.toBool()};
				if (val == m_messages.at(row)->deleted)
					return false;
				m_messages.at(row)->deleted = val;
			}
			break;
			case sentRole:
			{
				bool val{value.toBool()};
				if (val == m_messages.at(row)->sent)
					return false;
				m_messages.at(row)->sent = val;
			}
			break;
			case receivedRole:
			{
				bool val{value.toBool()};
				if (val == m_messages.at(row)->received)
					return false;
				m_messages.at(row)->received = val;
			}
			break;
			case readRole:
			{
				bool val{value.toBool()};
				if (val == m_messages.at(row)->read)
					return false;
				m_messages.at(row)->read = val;
			}
			break;
			case textRole:
			{
				QString text{std::move(value.toString())};
				if (text == m_messages.at(row)->text)
					return false;
				m_messages.at(row)->text = std::move(text);
			}
			break;
			case mediaRole:
			{
				QString media{std::move(value.toString())};
				if (media == m_messages.at(row)->media)
					return false;
				m_messages.at(row)->media = std::move(media);
			}
			break;
		}
		emit dataChanged(index, index, QList<int>{1, role});
		updateFieldToSave(m_messages.at(row)->id, role - Qt::UserRole, data(index, role).toString());
		return true;
	}
	return false;
}
