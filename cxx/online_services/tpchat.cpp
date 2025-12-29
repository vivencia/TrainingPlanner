#include "tpchat.h"

#include "tpchatdb.h"
#include "tponlineservices.h"
#include "websocketserver.h"
#include "../thread_manager.h"
#include "../dbusermodel.h"
#include "../pageslistmodel.h"
#include "../tpbool.h"

#include <QTimer>
#include <QWebSocket>

#include <ranges>

constexpr uint MESSAGE_OWNMESSAGE{MESSAGE_MEDIA + 1};

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
	ownMessageRole	=		mediaRole + 1,
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
	QString queued;

	TPBool own_message;
};

TPChat::TPChat(const QString &otheruser_id, const bool check_unread_messages, QObject *parent)
	: QAbstractListModel{parent}, m_otherUserId{otheruser_id}, m_unreadMessages{0}, m_chatWindow{nullptr},  m_chatLoaded{false},
									m_sendMessageTimer{nullptr}, m_peerSocket{nullptr}
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
	m_roleNames[ownMessageRole]	=	std::move("ownMessage");

	m_userIdx = appUserModel()->userIdxFromFieldValue(USER_COL_ID, m_otherUserId);
	m_dbModelInterface = new DBModelInterfaceChat{this};
	m_db = new TPChatDB{appUserModel()->userId(0), m_otherUserId, m_dbModelInterface};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);

	m_workFuncs.insert(messageWorkSend, [this] (const QString &data) -> void { incomingMessage(data); });
	m_workFuncs.insert(messageWorkReceived, [this] (const QString &data) -> void { setData(index(data.toUInt()), true, receivedRole); });
	m_workFuncs.insert(messageWorkRead, [this] (const QString &data) -> void { setData(index(data.toUInt()), true, readRole); });
	m_workFuncs.insert(messageWorkRemoved, [this] (const QString &data) -> void { removeMessage(data.toUInt(), false); });
	m_workFuncs.insert(messageWorkEdited, [this] (const QString &data) -> void { editMessage(data); });

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
		if (appUserModel()->canConnectToServer())
		{
			for (const auto msg : std::as_const(m_messages))
			{
				if (!msg->queued.isEmpty())
					unqueueMessage(msg);
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

	if (appWSServer()->peerSocket(m_otherUserId)) //If peer had initiated the connection before, become their client
		setWSPeer(appWSServer()->peerSocket(m_otherUserId));

	connect(appWSServer(), &ChatWSServer::wsConnectionToClientPeerConcluded, this, [this]
																(const bool success, const QString &userid, QWebSocket *peer)
	{
		if (userid == m_otherUserId)
		{
			if (success)
				setWSPeer(peer);
		}
	});
}

void TPChat::setWSPeer(QWebSocket *peer)
{
	if (peer != m_peerSocket)
	{
		m_peerSocket = peer;
		connect(m_peerSocket, SIGNAL(textMessageReceived(const QString &)), this, SLOT(processWebSocketMessage(const QString &)));
		connect(m_peerSocket, &QWebSocket::disconnected, this, [this] () {
			m_peerSocket->disconnect();
			m_peerSocket->deleteLater();
			m_peerSocket = nullptr;
		});
	}
}

void TPChat::loadChat()
{
	if (m_sendMessageTimer)
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
				message->own_message = message->sender == appUserModel()->userId(0);
				m_messages.append(message);
				if (!message->read && !message->own_message)
					++unread_messages;
			};
			emit dataChanged(index(0, 0), index(count() - 1));
			emit countChanged();
			if (unread_messages > 0)
				setUnreadMessages(unread_messages);
			endInsertRows();
			m_chatLoaded = true;
		}
		m_sendMessageTimer->start(0);
	}, Qt::SingleShotConnection);
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

void TPChat::processTPServerMessage(const QString &work, const QString &messages)
{
	if (!m_chatLoaded)
		return;
	uint msg_idx{0};
	do {
		const QString &message{appUtils()->getCompositeValue(msg_idx, messages, set_separator)};
		if (message.isEmpty())
			break;
		if (m_workFuncs.contains(work))
			m_workFuncs.value(work)(message);
	} while (++msg_idx);
}

//When called from ChatWindow: deletes the message locally and, if it is a sent message, insctruct the other party to have
//this message that they received removed. If it is a received message, keep the alteration local only
//When called from appMessagesManager(), message->sender will be equal to m_otherUserId, so it will be a received message:
//in this case, delete the message locally only
void TPChat::removeMessage(const uint msgid, const bool remove_for_interlocutor)
{
	if (msgid >= m_messages.count() || m_messages.at(msgid)->deleted)
		return;
	if (remove_for_interlocutor)
		setData(index(msgid), "1"_L1, deletedRole);
	else //Remove locally only. Do not call setData() because it will, in turn, call uploadAction()
	{
		m_messages.at(msgid)->deleted = true;
		emit dataChanged(index(msgid), index(msgid), QList<int>{1, deletedRole});
		updateFieldToSave(msgid, MESSAGE_DELETED, "1"_L1);
	}
	setData(index(msgid), QString{}, textRole);
	setData(index(msgid), QString{}, mediaRole);
}

void TPChat::editMessage(const QString &encoded_data)
{
	bool ok{false};
	const uint msgid{appUtils()->getCompositeValue(0, encoded_data, set_separator).toUInt(&ok)};
	if (ok)
	{
		ChatMessage *message{m_messages.at(msgid)};
		if (message)
		{
			if (!message->deleted)
			{
				const uint field{appUtils()->getCompositeValue(1, encoded_data, set_separator).toUInt() + Qt::UserRole};
				if (field >= textRole && field <= mediaRole)
					setData(index(msgid), appUtils()->getCompositeValue(2, encoded_data, set_separator), field);
			}
		}
	}
}

void TPChat::setUnreadMessages(const int n_unread)
{
	switch (n_unread)
	{
		case -1: if (m_unreadMessages > 0) --m_unreadMessages; break;
		case -2: ++m_unreadMessages; break;
		default: m_unreadMessages = n_unread; break;
	}
	emit unreadMessagesChanged();
}

void TPChat::markAllIncomingMessagesRead()
{
	for (const auto message : std::as_const(m_messages) | std::views::reverse)
	{
		if (message->read || message->own_message)
			break;
		setData(index(message->id), true, readRole);
	}
}

void TPChat::createNewMessage(const QString &text, const QString &media)
{
	ChatMessage *message{new ChatMessage};
	message->id = m_messages.count();
	message->sender = appUserModel()->userId(0);
	message->receiver = m_otherUserId;
	message->sdate = std::move(QDate::currentDate());
	message->stime = std::move(QTime::currentTime());
	message->text = text;
	message->text.replace('\'', '"');
	message->media = media;
	message->own_message = true;
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(message);
	emit countChanged();
	endInsertRows();
	encodeMessageToSave(message);
	uploadAction(MESSAGE_ID, message);
}

void TPChat::incomingMessage(const QString &encoded_message)
{
	ChatMessage *message{decodeDownloadedMessage(encoded_message)};
	if (!message)
		return;
	message->rdate = std::move(QDate::currentDate());
	message->rtime = std::move(QTime::currentTime());
	message->own_message = false;
	message->received = true;

	if (m_chatWindow)
	{
		beginInsertRows(QModelIndex{}, count(), count());
		if (appPagesListModel()->isPopupAboveAllOthers(m_chatWindow))
		{
			if (m_chatWindow->property("canViewNewMessages").toBool())
				message->read = true;
		}
	}
	if (!message->read)
		setUnreadMessages(-2);
	m_messages.append(message);
	if (m_chatWindow)
	{
		emit countChanged();
		endInsertRows();
		emit messageReceived();
	}
	encodeMessageToSave(message);
	uploadAction(MESSAGE_RECEIVED, message);
	if (message->read)
		uploadAction(MESSAGE_READ, message);
}

void TPChat::clearChat()
{
	void beginResetModel();
	m_db->clearTable();
	qDeleteAll(m_messages);
	void endResetModel();
}

QVariant TPChat::data(const ChatMessage *const message, const uint field, const bool format_output) const
{
	switch (field)
	{
		case MESSAGE_ID: return message->id;
		case MESSAGE_SENDER: return message->sender;
		case MESSAGE_RECEIVER: return message->receiver;
		case MESSAGE_SDATE: return format_output ? QVariant{appUtils()->formatDate(message->sdate)} : QVariant{message->sdate};
		case MESSAGE_STIME: return format_output ? QVariant{appUtils()->formatTime(message->stime)} : QVariant{message->stime};
		case MESSAGE_RDATE: return format_output ? QVariant{appUtils()->formatDate(message->rdate)} : QVariant{message->rdate};
		case MESSAGE_RTIME: return format_output ? QVariant{appUtils()->formatTime(message->rtime)} : QVariant{message->rtime};
		case MESSAGE_DELETED: return static_cast<bool>(message->deleted);
		case MESSAGE_SENT: return static_cast<bool>(message->sent);
		case MESSAGE_RECEIVED: return static_cast<bool>(message->received);
		case MESSAGE_READ: return static_cast<bool>(message->read);
		case MESSAGE_TEXT: return message->text;
		case MESSAGE_MEDIA: return message->media;
		case MESSAGE_OWNMESSAGE: return static_cast<bool>(message->own_message);
	}
	return QVariant{};
}

QVariant TPChat::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_messages.count())
		return data(m_messages.at(row), role - Qt::UserRole, true);
	return QVariant{};
}

bool TPChat::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_messages.count())
	{
		ChatMessage *const message{m_messages.at(row)};
		const bool respond{!isBitSet(connectionType(), 0)}; //when not using WebSockets, a response to the server is needed

		switch (role)
		{
			case rDateRole:
				message->rdate = std::move(value.toDate());
			break;
			case rTimeRole:
				message->rtime = std::move(value.toTime());
			break;
			case deletedRole:
				if (message->deleted == value.toBool())
					return false;
				message->deleted = value.toBool();
				uploadAction(MESSAGE_DELETED, message);
			break;
			case sentRole:
				if (message->sent == value.toBool())
					return false;
				message->sent = value.toBool();
			break;
			case receivedRole:
				if (message->received == value.toBool())
					return false;
				message->received = value.toBool();
				if (respond)
					uploadAction(MESSAGE_RECEIVED, message);
			break;
			case readRole:
				if (message->read == value.toBool())
					return false;
				message->read = value.toBool();
				setUnreadMessages(message->read ? -1 : -2);
				if (respond || !message->own_message)
					uploadAction(MESSAGE_READ, message);
			break;
			case textRole:
				message->text = std::move(value.toString());
				if (respond && !message->deleted)
					uploadAction(MESSAGE_TEXT, message);
			break;
			case mediaRole:
				message->media = std::move(value.toString());
				if (respond && !message->deleted)
					uploadAction(MESSAGE_MEDIA, message);
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>{1, role});
		const int field{role - Qt::UserRole};
		updateFieldToSave(message->id, field, data(message, field).toString());
		return true;
	}
	return false;
}

void TPChat::processWebSocketMessage(const QString &message)
{
	bool ok{false};
	static_cast<void>(appUtils()->getCompositeValue(0, message, exercises_separator).toInt(&ok));
	if (ok)
	{
		if (!m_chatLoaded)
		{
			connect(this, &TPChat::countChanged, this, [this,message] {
				processWebSocketMessage(message);
			}, Qt::SingleShotConnection);
			loadChat();
			return;
		}
		const QString &action{appUtils()->getCompositeValue(1, message, exercises_separator)};
		const QString &value{appUtils()->getCompositeValue(2, message, exercises_separator)};
		m_workFuncs.value(action)(value);
	}
}

void TPChat::onChatWindowOpened()
{
	if (!m_peerSocket)
		//Attempt to initiate the connection by querying the server for the peer's(interlocutor's) address
		appWSServer()->connectToPeer(m_otherUserId);
	else
	{
		if (!m_peerSocket->isValid())
		{
			m_peerSocket->disconnect();
			m_peerSocket->deleteLater();
			m_peerSocket = nullptr;
		}
	}
	markAllIncomingMessagesRead();
}

inline short TPChat::connectionType() const
{
	short has_connection{0};
	if (m_peerSocket && m_peerSocket->isValid())
		setBit(has_connection, 0);
	if (appUserModel()->canConnectToServer())
		setBit(has_connection, 1);
	return has_connection;
}

void TPChat::unqueueMessage(ChatMessage* const message)
{
	const QString &msgid{QString::number(message->id)};
	for (uint i{0}; i <= MESSAGE_MEDIA; ++i)
	{
		const QString &field_value{appUtils()->getCompositeValue(i, message->queued, record_separator)};
		if (field_value == "1"_L1)
			uploadAction(i, message);
	}
	message->queued.clear();
}

void TPChat::uploadAction(const uint field, ChatMessage *const message)
{
	const short has_connection{connectionType()};
	if (has_connection != 0)
	{
		const QString &msgid{QString::number(message->id)};
		const QLatin1StringView seed{QString{message->text % QString::number(field)}.toLatin1()};
		const int requestid{appUtils()->generateUniqueId(seed)};
		switch (field)
		{
			case MESSAGE_ID:
				if (message->own_message)
				{
					setData(index(message->id), true, sentRole);
					if (isBitSet(has_connection, 0))
						m_peerSocket->sendTextMessage(appUtils()->string_strings(
							{QString::number(requestid), messageWorkSend, encodeMessageToUpload(message)}, exercises_separator));
					else
						appOnlineServices()->sendMessage(requestid, m_otherUserId, std::move(encodeMessageToUpload(message)));
				}
			break;
			case MESSAGE_DELETED:
				if (isBitSet(has_connection, 0))
					m_peerSocket->sendTextMessage(appUtils()->string_strings(
												{QString::number(requestid), messageWorkRemoved, msgid}, exercises_separator));
				else
				{
					if (message->own_message)
					{
						//Add message->id to the m_otherUserId/chats/this_user.removed file
						appOnlineServices()->chatMessageWork(requestid, m_otherUserId, msgid, messageWorkRemoved);
					}
					else
						//Remove message->id from the m_otherUserId/chats/this_user.removed file
						appOnlineServices()->chatMessageWorkAcknowledged(requestid, m_otherUserId, msgid, messageWorkRemoved);
				}
			break;
			case MESSAGE_RECEIVED:
				if (isBitSet(has_connection, 0))
					m_peerSocket->sendTextMessage(appUtils()->string_strings(
											{QString::number(requestid), messageWorkReceived, msgid}, exercises_separator));
				else
				{
					if (!message->own_message)
					{
						//Remove message from the this_user/chats/m_otherUserId.msg file
						appOnlineServices()->chatMessageWork(requestid, m_otherUserId, msgid, messageWorkSend);
						//Add message->id to the m_otherUserId/chats/this_user.received file
						appOnlineServices()->chatMessageWork(requestid, m_otherUserId, msgid, messageWorkReceived);
					}
					else
						//Remove message->id from the m_otherUserId/chats/this_user.received file
						appOnlineServices()->chatMessageWorkAcknowledged(requestid, m_otherUserId, msgid, messageWorkReceived);
				}
			break;
			case MESSAGE_READ:
				if (isBitSet(has_connection, 0))
					m_peerSocket->sendTextMessage(appUtils()->string_strings(
													{QString::number(requestid), messageWorkRead, msgid}, exercises_separator));
				else
				{
					if (!message->own_message)
						//Add message->id to the m_otherUserId/chats/this_user.read file
						appOnlineServices()->chatMessageWork(requestid, m_otherUserId, msgid, messageWorkRead);
					else
						//Remove message->id from the m_otherUserId/chats/this_user.read file
						appOnlineServices()->chatMessageWorkAcknowledged(requestid, m_otherUserId, msgid, messageWorkRead);
				}
			break;
			case MESSAGE_TEXT:
			case MESSAGE_MEDIA:
				if (isBitSet(has_connection, 0))
					m_peerSocket->sendTextMessage(appUtils()->string_strings(
												{QString::number(requestid), messageWorkEdited, msgid}, exercises_separator));
				else
				{
					if (!message->own_message)
						appOnlineServices()->chatMessageWork(requestid, m_otherUserId, appUtils()->string_strings(
							{msgid, QString::number(field - Qt::UserRole), data(index(msgid.toUInt()), field).toString()},
							record_separator), messageWorkEdited);
					else
						appOnlineServices()->chatMessageWorkAcknowledged(requestid, m_otherUserId, msgid, messageWorkEdited);
				}
			default: break;
		}
	}
	else
		appUtils()->setCompositeValue(field, "1"_L1, message->queued, record_separator);
}

void TPChat::acknowledgeMessageWorked(const uint msgid, const QLatin1StringView &work)
{
	const int requestid{appUtils()->generateRandomNumber(0, 5000)};
	appOnlineServices()->chatMessageWorkAcknowledged(requestid, m_otherUserId, QString::number(msgid), work);
}

void TPChat::encodeMessageToSave(const ChatMessage* const message)
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
					message->media,
					message->queued
	}));
	updateFieldToSave(modified_row, -1, QString{});
}

void TPChat::updateFieldToSave(const uint msg_id, const int field, const QString &value) const
{
	if (field >= 0)
		m_dbModelInterface->modelData()[msg_id][field] = value;
	m_dbModelInterface->setModified(msg_id, field);
	m_sendMessageTimer->start();
}

//record_separator(oct 036, dec 30) separates the message fields
//set_separator (oct 037, dec 31) separates messages of the same sender
//exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and
//the odd numbers are the sender ids)
QString TPChat::encodeMessageToUpload(const ChatMessage* const message) const
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
	uint id{appUtils()->getCompositeValue(MESSAGE_ID, encoded_message, record_separator).toUInt()};
	const auto &itr{std::find_if(m_messages.cbegin(), m_messages.cend(), [id] (const ChatMessage *message) {
		return message->id == id;
	})};
	if (itr != m_messages.cend())
		return nullptr;

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
