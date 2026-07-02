#include "tpchat.h"

#include "tpchatdb.h"
#include "tponlineservices.h"
#include "websocketserver.h"
#include "../dbusermodel.h"
#include "../pageslistmodel.h"
#include "../tpfilepath.h"
#include "../tpfileops.h"
#include "../thread_manager.h"
#include "../tputils.h"

#include <QTimer>

#include <ranges>

using namespace QLiterals;

enum ConnectionType {
	CT_WS,
	Ct_SERVER,
};

enum ChatLoadedStatus {
	Unloaded,
	Waiting,
	Loaded
};

enum ChatWork {
	CW_SEND,
	CW_RECEIVED,
	CW_READ,
	CW_REMOVED,
	CW_EDITED,
	CW_N_WORKS
};

//The first two fields must match TPUtils::EF_HANDLER_ID and TPUtils::EF_SENDER respectively to be sorted out by
//WSServer::wsTextMessageReceived
enum WorkDataFields {
	WDF_HANDLE_PREFIX,
	WDF_SENDER,
	WDF_CHAT_WORK,
	WDF_MSGID,
	WDF_MSG_FIELD,
	WDF_WORK_VALUE
};

auto workDataFieldValue = [] (const QString &work_data, const WorkDataFields work_field) -> QString {
	return appUtils()->getCompositeValue(work_field, work_data, exercises_separator);
};

enum ChatRoleNames {
	createRole(msgId, TPChat::ID)
	createRole(msgSender, TPChat::SENDER)
	createRole(msgReceiver, TPChat::RECEIVER)
	createRole(msgSDate, TPChat::SDATE)
	createRole(msgSTime, TPChat::STIME)
	createRole(msgRDate, TPChat::RDATE)
	createRole(msgRTime, TPChat::RTIME)
	createRole(msgDeleted, TPChat::DELETED)
	createRole(msgSent, TPChat::SENT)
	createRole(msgReceived, TPChat::RECEIVED)
	createRole(msgRead, TPChat::READ)
	createRole(msgText, TPChat::TEXT)
	createRole(msgMedia, TPChat::MEDIA)
	createRole(ownMessage, TPChat::OWN_MESSAGE)
	createRole(mediaViewer, TPChat::MEDIA_VIEWER)
};

struct ChatMessage {
	uint id;
	QString sender, receiver;
	QDate sdate, rdate;
	QTime stime, rtime;
	bool deleted{false};
	bool sent{false};
	bool received{false};
	bool read{false};
	QString text, media;

	TPFileOps *media_viewer{nullptr};
	QString queued;
	bool own_message{false};
};

TPChat::TPChat(const QString &otheruser_id, const bool check_unread_messages, QObject *parent)
	: QAbstractListModel{parent}, m_otherUserId{otheruser_id}, m_chatLoaded{Unloaded}
{
	roleToString(msgId)
	roleToString(msgSender)
	roleToString(msgReceiver)
	roleToString(msgSDate)
	roleToString(msgSTime)
	roleToString(msgRDate)
	roleToString(msgRTime)
	roleToString(msgDeleted)
	roleToString(msgSent)
	roleToString(msgReceived)
	roleToString(msgRead)
	roleToString(msgText)
	roleToString(msgMedia)
	roleToString(ownMessage)
	roleToString(mediaViewer)

	m_userIdx = appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, m_otherUserId);
	m_dbModelInterface = new DBModelInterfaceChat{this};
	m_db = new TPChatDB{this};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);

	m_workFuncs.insert(CW_SEND, [this] (const QString &work_data) -> void {
													incomingMessage(workDataFieldValue(work_data, WDF_MSG_FIELD)); });
	m_workFuncs.insert(CW_RECEIVED, [this] (const QString &work_data) -> void {
					setData(index(workDataFieldValue(work_data, WDF_MSGID).toInt()), true, msgReceivedRole); });
	m_workFuncs.insert(CW_READ, [this] (const QString &work_data) -> void {
					setData(index(workDataFieldValue(work_data, WDF_MSGID).toInt()), true, msgReadRole); });
	m_workFuncs.insert(CW_REMOVED, [this] (const QString &work_data) -> void {
					removeMessage(workDataFieldValue(work_data, WDF_MSGID).toInt(), false); });
	m_workFuncs.insert(CW_EDITED, [this] (const QString &work_data) -> void { editMessage(work_data); });

	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_idx, const uint field) {
		if (user_idx == m_userIdx) {
			switch (field) {
			case DBUserModel::USER_FIELD_NAME: emit interlocutorNameChanged(); break;
			case DBUserModel::USER_FIELD_AVATAR: emit avatarIconChanged(); break;
			}
		}
		else if (user_idx == 0 && field == USER_MODIFIED_REMOVED)
			m_userIdx = appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, m_otherUserId);
	});

	connect(appUserModel(), &DBUserModel::canConnectToServerChanged, this, [this] () {
		if (appUserModel()->canConnectToServer()) {
			for (const auto msg : std::as_const(m_messages)) {
				if (!msg->queued.isEmpty())
					unqueueMessage(msg);
			}
		}
	});

	if (check_unread_messages) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_db, &TPDatabaseTable::actionFinished, this, [this,conn]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2) {
			if (op == ThreadManager::CustomOperation) {
				disconnect(*conn);
				if (return_value1.toBool())
					setUnreadMessages(return_value2.toString());
			}
		});
		auto x = [this] () -> std::pair<QVariant,QVariant> { return m_db->getNumberOfUnreadMessages(); };
		m_db->setCustQueryFunction(x);
		appThreadManager()->runAction(m_db, ThreadManager::CustomOperation);
	}
}

const QString &TPChat::userId() const
{
	return appUserModel()->userId(0);
}

void TPChat::loadChat()
{
	if (m_sendMessageTimer)
		return;

	connect(m_db, &TPChatDB::chatLoaded, this, [this] (const bool success) {
		if (success) {
			m_nMedia = 0;
			beginInsertRows(QModelIndex{}, 0, m_dbModelInterface->modelData().count() - 1);
			for (const auto &str_message : std::as_const(m_dbModelInterface->modelData())) {
				ChatMessage *message = new ChatMessage;
				message->id = str_message.at(ID).toUInt();
				message->sender = str_message.at(SENDER);
				message->receiver = str_message.at(RECEIVER);
				message->sdate = std::move(appUtils()->dateFromString(str_message.at(SDATE), TPUtils::DF_ONLINE));
				message->stime = std::move(appUtils()->timeFromString(str_message.at(STIME), TPUtils::TF_ONLINE));
				message->rdate = std::move(appUtils()->dateFromString(str_message.at(RDATE), TPUtils::DF_ONLINE));
				message->rtime = std::move(appUtils()->timeFromString(str_message.at(RTIME), TPUtils::TF_ONLINE));
				message->deleted = str_message.at(DELETED).toUInt() == 1;
				message->sent = str_message.at(SENT).toUInt() == 1;
				message->received = str_message.at(RECEIVED).toUInt() == 1;
				message->read = str_message.at(READ).toUInt() == 1;
				message->text = str_message.at(TEXT);
				message->media = str_message.at(MEDIA);
				if (!message->media.isEmpty())
					createMediaViewer(message, false);
				message->own_message = message->sender == appUserModel()->userId(0);
				m_messages.append(message);
			};
			if (m_messageWorksQueued) {
				setChatLoadedStatus(Waiting);
				appOnlineServices()->recheckNewChatMessages();
			}
			else
				setChatLoadedStatus(Loaded);
			emit dataChanged(index(0, 0), index(count() - 1));
			emit countChanged();
			endInsertRows();
		}
		m_sendMessageTimer->start(1);
	}, Qt::SingleShotConnection);
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	m_sendMessageTimer = new QTimer{this};
	m_sendMessageTimer->setInterval(10000);
	m_sendMessageTimer->callOnTimeout([this] () {
		if (!m_dbModelInterface->modifiedIndices().isEmpty())
			appThreadManager()->runAction(m_db, ThreadManager::AlterRecords);
		m_sendMessageTimer->stop();
	});
}

QString TPChat::interlocutorName() const
{
	return appUserModel()->userName(m_userIdx);
}

QString TPChat::avatarIcon() const
{
	return appUserModel()->avatar(m_userIdx);
}

//When called from ChatWindow: deletes the message locally and, if it is a sent message, insctruct the other party to have
//this message that they received removed. If it is a received message, keep the alteration local only
//When called from appMessagesManager(), message->sender will be equal to m_otherUserId, so it will be a received message:
//in this case, delete the message locally only
void TPChat::removeMessage(const uint msgid, const bool remove_for_interlocutor)
{
	if (msgid >= m_messages.count() || m_messages.at(msgid)->deleted)
		return;
	setData(index(msgid), "1"_L1, msgDeletedRole);
	setData(index(msgid), QString{}, msgTextRole);
	setData(index(msgid), QString{}, msgMediaRole);
	ChatMessage *message{m_messages.at(msgid)};
	if (message->media_viewer) {
		message->media_viewer->removeFile(true, true, false);
		delete message->media_viewer;
		message->media_viewer = nullptr;
	}
	if (message->own_message && remove_for_interlocutor)
		doChatWork(CW_REMOVED, m_messages.at(msgid));
}

void TPChat::editMessage(const QString &work_data)
{
	const uint msgid{workDataFieldValue(work_data, WDF_MSGID).toUInt()};
	if (msgid < m_messages.count()) {
		ChatMessage *message{m_messages.at(msgid)};
		if (message) {
			if (!message->deleted) {
				const uint field{workDataFieldValue(work_data, WDF_MSG_FIELD).toUInt()};
				if (field >= ID && field < QUEUED)
					setData(index(msgid), workDataFieldValue(work_data, WDF_WORK_VALUE).toUInt(), Qt::UserRole + field);
			}
		}
	}
}

void TPChat::markAllIncomingMessagesRead()
{
	for (const auto message : std::as_const(m_messages) | std::views::reverse) {
		if (message->read || message->own_message)
			break;
		setData(index(message->id), true, msgReadRole);
	}
}

void TPChat::createNewMessage(const QString &text, const bool attach_file)
{
	ChatMessage *message{new ChatMessage};
	message->id = m_messages.count();
	message->sender = appUserModel()->userId(0);
	message->receiver = m_otherUserId;
	message->sdate = std::move(QDate::currentDate());
	message->stime = std::move(QTime::currentTime());
	message->text = text;
	message->own_message = true;
	beginInsertRows(QModelIndex{}, count(), count());
	m_messages.append(message);
	emit countChanged();
	endInsertRows();
	if (attach_file)
		createMediaViewer(message, true);
	else
		sendMessage(message);
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
	if (!message->media.isEmpty())
		createMediaViewer(message, false);

	if (m_chatWindow) {
		beginInsertRows(QModelIndex{}, count(), count());
		if (appPagesListModel()->isPopupAboveAllOthers(m_chatWindow)) {
			if (m_chatWindow->property("canViewNewMessages").toBool())
				message->read = true;
		}
	}
	if (!message->read)
		setUnreadMessages(QString::number(message->id));
	m_messages.append(message);
	if (m_chatWindow) {
		emit countChanged();
		endInsertRows();
		emit messageReceived();
	}
	encodeMessageToSave(message);
	if (message->read)
		doChatWork(CW_READ, message);
	else
		doChatWork(CW_RECEIVED, message);
}

void TPChat::clearChat()
{
	void beginResetModel();
	m_db->clearTable();
	for (auto message : std::as_const(m_messages)) {
		if (message->media_viewer) {
			message->media_viewer->removeFile(true, true, false);
			delete message->media_viewer;
		}
		delete message;
	}
	m_messages.clear();
	appUtils()->rmDir(appUserModel()->mainUserDir() % chatSubDir());
	void endResetModel();
}

QVariant TPChat::data(const ChatMessage *const message, const uint field, const bool format_output) const
{
	switch (field) {
	case ID: return message->id;
    case SENDER: return message->sender;
    case RECEIVER: return message->receiver;
	case SDATE: return format_output ? QVariant{appUtils()->formatDate(message->sdate)} : QVariant{message->sdate};
	case STIME: return format_output ? QVariant{appUtils()->formatTime(message->stime)} : QVariant{message->stime};
	case RDATE: return format_output ? QVariant{appUtils()->formatDate(message->rdate)} : QVariant{message->rdate};
	case RTIME: return format_output ? QVariant{appUtils()->formatTime(message->rtime)} : QVariant{message->rtime};
	case DELETED: return static_cast<bool>(message->deleted);
	case SENT: return static_cast<bool>(message->sent);
	case RECEIVED: return static_cast<bool>(message->received);
	case READ: return static_cast<bool>(message->read);
	case TEXT: return message->text;
	case MEDIA: return message->media;
	case OWN_MESSAGE: return static_cast<bool>(message->own_message);
	case MEDIA_VIEWER: return QVariant::fromValue(message->media_viewer);
	}
	return QVariant{};
}

bool TPChat::canUseWebSocket() const
{
	return isBitSet(checkConnectionOptions(), CT_WS);
}

bool TPChat::canUseServer() const
{
	return isBitSet(checkConnectionOptions(), Ct_SERVER);
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
	if (row >= 0 && row < m_messages.count()) {
		ChatMessage *const message{m_messages.at(row)};
		switch (role) {
		case msgRDateRole:
			message->rdate = std::move(value.toDate());
			break;
		case msgRTimeRole:
			message->rtime = std::move(value.toTime());
			break;
		case msgDeletedRole:
			message->deleted = value.toBool();
			break;
		case msgSentRole:
			if (message->sent == value.toBool())
				return false;
			message->sent = value.toBool();
			break;
		case msgReceivedRole:
			message->received = value.toBool();
			if (!message->own_message)
				doChatWork(CW_RECEIVED, message);
			break;
		case msgReadRole:
			if ((message->read = value.toBool()))
				setData(index, true, msgReceivedRole);
			if (message->own_message) {
				setUnreadMessages(QString::number(message->id), !message->read);
				doChatWork(CW_READ, message);
			}
			break;
		case msgTextRole:
			message->text = std::move(value.toString());
			if (message->own_message)
				doChatWork(CW_EDITED, message);
			break;
		case msgMediaRole:
			message->media = std::move(value.toString());
			if (message->own_message)
				doChatWork(CW_EDITED, message);
			break;
		default: return false;
		}
		emit dataChanged(index, index, QList<int>{role});
		const int field{role - Qt::UserRole};
		updateFieldToSave(message->id, field, data(message, field).toString());
		return true;
	}
	return false;
}

void TPChat::processChatMessage(const QString &encoded_message)
{
	if (m_chatLoaded == Unloaded) {
		connect(this, &TPChat::chatLoadedStatusChanged, this, [this,&encoded_message] () {
			if (m_chatLoaded == Loaded)
				processChatMessage(encoded_message);
		});
		loadChat();
		return;
	}
	const auto work{workDataFieldValue(encoded_message, WDF_WORK_VALUE).toUInt()};
	if (work >= CW_SEND && work <= CW_EDITED)
		m_workFuncs.at(work)(encoded_message);
}

void TPChat::onChatWindowOpened()
{
	markAllIncomingMessagesRead();
	appWSServer()->connectToPeer(this, TPUtils::MH_TPCHAT, m_otherUserId);
}

inline void TPChat::setChatLoadedStatus(uint8_t status)
{
	if (status != m_chatLoaded) {
		m_chatLoaded = status;
		emit chatLoadedStatusChanged();
	}
}

inline short TPChat::checkConnectionOptions() const
{
	short has_connection{0};
	if (!m_messageWorksQueued && appWSServer()->isConnectionOK(m_otherUserId))
		setBit(has_connection, CT_WS);
	if (appUserModel()->canConnectToServer())
		setBit(has_connection, Ct_SERVER);
	return has_connection;
}

void TPChat::unqueueMessage(ChatMessage* const message)
{
	const QString &msgid{QString::number(message->id)};
	for (uint i{0}; i <= MEDIA; ++i) {
		const QString &field_value{appUtils()->getCompositeValue(i, message->queued, record_separator)};
		if (field_value == "1"_L1)
			doChatWork(i, message);
	}
	message->queued.clear();
}

void TPChat::doChatWork(const uint work, ChatMessage *const message, const int field)
{
	const bool use_ws{canUseWebSocket()};
	if (use_ws || canUseServer()) {
		const QString &msgid{QString::number(message->id)};
		const QLatin1StringView seed{QString{message->text % QString::number(work)}.toLatin1()};
		const int requestid{appUtils()->generateUniqueId(seed)};
		switch (work) {
		case CW_SEND:
			if (message->own_message) {
				setData(index(message->id), true, msgSentRole);
				const QString &encoded_message{encodeWorkMessage(message, CW_SEND, TPCHAT_N_FIELDS)};
				if (!message->media.isEmpty())
					message->media_viewer->sendFileTo(TPUtils::MH_TPCHAT, {m_otherUserId}, encoded_message);
				else {
					if (use_ws)
						appWSServer()->sendTextMessage(encoded_message);
					else
						appOnlineServices()->sendChatMessage(requestid, m_otherUserId, encoded_message);
				}
			}
			break;
		case CW_RECEIVED:
		case CW_READ:
		case CW_REMOVED:
		case CW_EDITED:
			if (use_ws)
				appWSServer()->sendTextMessage(encodeWorkMessage(message, work, field));
			else
				appOnlineServices()->sendChatMessage(requestid, m_otherUserId, encodeWorkMessage(message, work, field));
		default: break;
		}
	}
	else
		appUtils()->setCompositeValue(work, "1"_L1, message->queued, record_separator);
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
					message->media_viewer ? message->media_viewer->fileName() : QString{},
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
				"0"_L1,
				"1"_L1,
				"0"_L1,
				"0"_L1,
				message->text,
				message->media_viewer ? message->media_viewer->fileName() : QString{}
	}, record_separator);
}

QString TPChat::encodeWorkMessage(const ChatMessage *const message, const int work, const int field) const
{
	QString encoded_work{std::move(TPUtils::chatmessage_prefix % exercises_separator % message->sender
											   % exercises_separator % QString::number(work) % exercises_separator)};
	switch (work) {
	case CW_SEND:
		encoded_work += std::move(encodeMessageToUpload(message) % exercises_separator);
		break;
	case CW_RECEIVED:
	case CW_READ:
	case CW_REMOVED:
		encoded_work += std::move(QString::number(message->id) % exercises_separator);
		break;
	case CW_EDITED:
		encoded_work += std::move(appUtils()->string_strings({QString::number(message->id), QString::number(field),
															  data(message, field).toString() }, exercises_separator ));
		break;
	default: Q_UNREACHABLE();
	}
	return encoded_work;
}

ChatMessage* TPChat::decodeDownloadedMessage(const QString &encoded_message)
{
	uint id{appUtils()->getCompositeValue(ID, encoded_message, record_separator).toUInt()};
	const auto &itr{std::find_if(m_messages.cbegin(), m_messages.cend(), [id] (const ChatMessage *message) {
		return message->id == id;
	})};
	if (itr != m_messages.cend())
		return nullptr;

	ChatMessage *new_message{new ChatMessage};
	new_message->id = id;
	new_message->sender = std::move(appUtils()->getCompositeValue(SENDER, encoded_message, record_separator));
	new_message->receiver = std::move(appUtils()->getCompositeValue(RECEIVER, encoded_message, record_separator));
	new_message->sdate = std::move(appUtils()->dateFromString(
							appUtils()->getCompositeValue(SDATE, encoded_message, record_separator), TPUtils::DF_ONLINE));
	new_message->rdate = std::move(appUtils()->dateFromString(
							appUtils()->getCompositeValue(RDATE, encoded_message, record_separator), TPUtils::DF_ONLINE));
	new_message->stime = std::move(appUtils()->timeFromString(
							appUtils()->getCompositeValue(STIME, encoded_message, record_separator), TPUtils::TF_ONLINE));
	new_message->rtime = std::move(appUtils()->timeFromString(
							appUtils()->getCompositeValue(RTIME, encoded_message, record_separator), TPUtils::TF_ONLINE));
	new_message->deleted = appUtils()->getCompositeValue(DELETED, encoded_message, record_separator) == "1"_L1;
	new_message->sent = appUtils()->getCompositeValue(SENT, encoded_message, record_separator) == "1"_L1;
	new_message->received = appUtils()->getCompositeValue(RECEIVED, encoded_message, record_separator) == "1"_L1;
	new_message->read = appUtils()->getCompositeValue(READ, encoded_message, record_separator) == "1"_L1;
	new_message->text = std::move(appUtils()->getCompositeValue(TEXT, encoded_message, record_separator));
	new_message->media = std::move(appUtils()->getCompositeValue(MEDIA, encoded_message, record_separator));
	return new_message;
}

void TPChat::getNewMessagesNumber(const QString &encoded_messages)
{
	uint msg_idx{0};
	QString unread_ids;
	do {
		const QString &encoded_message{appUtils()->getCompositeValue(msg_idx, encoded_messages, set_separator)};
		if (encoded_message.isEmpty())
			break;
		unread_ids.append(appUtils()->getCompositeValue(ID, encoded_message, record_separator) % set_separator);
	} while (++msg_idx);
	setUnreadMessages(unread_ids);
}

void TPChat::setUnreadMessages(const QString &unread_ids, const bool add)
{
	uint idx{0};
	const auto n_unread_ids{m_unreadIds.count()};
	if (unread_ids.contains(set_separator)) {
		do {
			QString msg_id{std::move(appUtils()->getCompositeValue(idx, unread_ids, set_separator))};
			if (msg_id.isEmpty())
				break;
			const bool contains{m_unreadIds.contains(msg_id)};
			if (add && !contains)
				m_unreadIds.append(std::move(msg_id));
			else if (!add && contains)
				m_unreadIds.removeOne(unread_ids);
		} while (++idx);
	}
	else {
		const bool contains{m_unreadIds.contains(unread_ids)};
		if (add && !contains)
			m_unreadIds.append(unread_ids);
		else if (!add && contains)
			m_unreadIds.removeOne(unread_ids);
	}
	if (n_unread_ids != m_unreadIds.count())
		emit unreadMessagesChanged();
}

void TPChat::createMediaViewer(ChatMessage *message, const bool add_file)
{
	++m_nMedia;
	TPFileOps *media_viewer{new TPFileOps{}};
	media_viewer->setUseControls(true);
	media_viewer->setCanAddFile(true);
	media_viewer->setSuggestedFileNameGenerator([this] (const QString &selected_filename) -> TPFilePathPtr {
		return TPFilePath::newTPFilePath(selected_filename, userId(), m_otherUserId, {chatSubDir(), "media/"_L1});
	});
	connect(media_viewer, &TPFileOps::fileAdded, this, [this,message] (const QString &filepath) {
		if (message->media.isEmpty())
			sendMessage(message);
		else
			setData(index(message->id), filepath, msgMediaRole);
	});
	message->media_viewer = media_viewer;
	emit dataChanged(index(message->id), index(message->id), QList<int>{mediaViewerRole});
	if (add_file)
		media_viewer->doFileOperation(TPFileOps::OT_AddFile);
	else
		media_viewer->setFileName(message->media);
}

inline void TPChat::sendMessage(ChatMessage *message)
{
	encodeMessageToSave(message);
	doChatWork(CW_SEND, message);
	QMetaObject::invokeMethod(m_chatWindow, "postSendingActions");
}
