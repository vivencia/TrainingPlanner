#include "tpmessagesmanager.h"

#include "tpchat.h"
#include "tpmessage.h"
#include "tponlineservices.h"
#include "websocketserver.h"
#include "../dbusermodel.h"
#include "../qmlitemmanager.h"
#include "../tpfileops.h"
#include "../tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

TPMessagesManager *TPMessagesManager::_appMessagesManager{nullptr};

inline decltype(auto) chatID(const QString &userid)
{
	return fnv1a_hash(userid % "chat_msg"_L1);
}

enum RoleNames {
	createRole(tpMessage, 0)
};

TPMessagesManager::TPMessagesManager(QObject *parent)
	: QAbstractItemModel{parent}
	, m_rootMessage{std::make_unique<TPMessage>()}
{
	_appMessagesManager = this;
	REGISTER_QML_SINGLETON(TPMessagesManager, this);
	roleToString(tpMessage)
}

TPMessage *TPMessagesManager::topLevelMessage(const QString &user_id) const
{
	const QList<TPMessage*> &top_level_messages{m_rootMessage->children()};
	if (!top_level_messages.isEmpty()) {
		for (const auto message : top_level_messages) {
			if (message->userid() == user_id)
				return message;
		}
	}
	return nullptr;
}

TPMessage *TPMessagesManager::createTopLevelMessage(const QString &userid)
{
	TPMessage *top_level_msg{topLevelMessage(userid)};
	if (!top_level_msg) {
		const int user_idx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, userid)};
		top_level_msg = new TPMessage{m_rootMessage.get()};
		top_level_msg->setUserId(userid);
		top_level_msg->setType("topLevel"_L1);
		top_level_msg->setTitle(std::move(user_idx != -1 ? appUserModel()->userName(user_idx) : tr("Unknown contact")));
		top_level_msg->setIcon(std::move(user_idx != -1 ? appUserModel()->avatar(user_idx) : "unknown-user"));
		addMessage(top_level_msg);
		if (userid != tpsystem_userid) {
			top_level_msg->insertAction(std::move(tr("Send Message")), TPMessage::AT_BUTTON,
					[this,top_level_msg,userid] (const QVariant &data) -> QVariant {
						openNewMessageDialog(userid);
						return QVariant{};
					});
			top_level_msg->insertAction(std::move(tr("Clear")), TPMessage::AT_BUTTON,
					[this,userid,top_level_msg] (const QVariant &data) -> QVariant {
						top_level_msg->remove(top_level_msg->generalPurposeData().toBool()
										  ? QLatin1StringView{}
										  : TPUtils::chatmessage_prefix);
					return QVariant{};
				});
			top_level_msg->insertAction(tr("Include chat"), TPMessage::AT_CHECKBOX,
					[this,userid,top_level_msg] (const QVariant &data) -> QVariant {
						top_level_msg->setGeneralPurposeData(data.toBool());
						return QVariant{};
					});
		}
	}
	return top_level_msg;
}

TPMessage *TPMessagesManager::message(const TPMessage *const parent_message, const uint id) const
{
	return parent_message->findChild(id, TPMessage::FIELD_ID);
}

void TPMessagesManager::addMessage(TPMessage *msg)
{
	msg->parent()->insertChild(msg);
	const QModelIndex &parent_index{createIndex(msg->parent()->row(), 0, msg->parent())};
	beginInsertRows(parent_index, msg->row(), msg->row());
	insertRows(msg->row(), 1, parent_index);
	endInsertRows();
	connect(msg, &TPMessage::killMessage, this, [this] (TPMessage *message) { removeMessage(message); });
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (msg != nullptr) {
		auto remove = [this,msg] () -> void {
			beginRemoveRows(QModelIndex{}, msg->row(), msg->row());
			const QModelIndex &parent_index{createIndex(msg->parent()->row(), 0, msg->parent())};
			removeRows(msg->row(), 1, parent_index);
			msg->remove();
			endRemoveRows();
		};
		if (appUserModel()->canConnectToServer()) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,msg,conn,remove]
											(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == msg->id()) {
					disconnect(*conn);
					remove();
				}
			});
			appOnlineServices()->removeTPMessage(msg->id(), msg->encodedMessage());
		}
		else //TODO schedule online services to run when we have connection to the server
			remove();
	}
}

void TPMessagesManager::execAction(const QString &user_id, const int index, const int action_id, const QVariant &data)
{
	TPMessage *message{topLevelMessage(user_id)};
	if (message) {
		message = message->child(index);
		if (message)
			message->execAction(action_id, data);
	}
}

void TPMessagesManager::enableAction(const QString &user_id, const int index, const int action_id, const bool enable)
{
	TPMessage *message{topLevelMessage(user_id)};
	if (message) {
		message = message->child(index);
		if (message)
			message->setActionEnabled(action_id, enable);
	}
}

void TPMessagesManager::sendTPMessage(const QString &target_user, const QString &encoded_message, const int request_id)
{
	auto send_result = [this,target_user] (const int requestid, const bool sent) -> void {
		emit TPMessageSent(requestid, sent);
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(appUtils()->string_strings(
		{sent ? tr("Success!") : tr("Error!"), sent ? tr("Message sent to") : tr("Try again. Could not sent message to ")
		% appUserModel()->userNameFromId(target_user)}, record_separator)), Qt::AlignCenter, std::move(
		sent ? "set-completed"_L1 : "error"_L1));
	};
	if (appWSServer()->isConnectionOK(target_user, true)) {
		const bool sent{appWSServer()->sendTextMessage(encoded_message)};
		send_result(request_id, sent);
	} else {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appWSServer(), &WSServer::connectionAttemptResult, this, [=,this]
															(const bool established, const QString &userid) {
			if (userid == target_user) {
				disconnect(*conn);
				bool message_sent{false};
				if (established)
					message_sent = appWSServer()->sendTextMessage(encoded_message);
				else {
					if ((message_sent = appUserModel()->canConnectToServer())) {
						*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this,
							[this,request_id,conn,send_result] (const int requestid, const int ret_code, const QString &ret_string) {
							if (requestid == request_id) {
								disconnect(*conn);
								send_result(requestid, ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS);
							}
						});
						appOnlineServices()->sendTPMessage(request_id, encoded_message, target_user);
						return;
					}
				}
				send_result(request_id, message_sent);
			}
		});
	}
}

void TPMessagesManager::textMessageReceived(const QString &encoded_message)
{
	QString userid{std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_SENDER))};
	const QString &c_time{appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_CTIME)};
	TPMessage *top_level_msg{createTopLevelMessage(userid)};
	const auto msg_id{fnv1a_hash(userid % c_time)};
	if (!message(top_level_msg, msg_id)) {
		TPMessage *new_message{new TPMessage{top_level_msg}};
		new_message->setId(msg_id);
		new_message->setUserId(std::move(userid));
		new_message->setType(TPUtils::tpmessage_prefix);
		new_message->setDateTime(std::move(appUtils()->dateTimeFromString(c_time)));
		new_message->setExpiration(std::move(appUtils()->dateTimeFromString(
							appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_EXP_TIME))));
		new_message->setFileName(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_REL_FILEPATH));
		new_message->setTitle(std::move(new_message->fileOps() ? tr("You have received a file") : tr("You have a message")));
		new_message->setIcon(std::move("send-message"_L1));
		new_message->setText(std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_TEXT)));
		new_message->setSticky(false);
		new_message->insertAction(tr("Dismiss"), TPMessage::AT_BUTTON, [this,new_message] (const QVariant &) -> QVariant {
			emit new_message->killMessage(new_message);
			return QVariant{};
		});
		addMessage(new_message);
	}
}

void TPMessagesManager::openNewMessageDialog(const QString &userid)
{
	//TODO
}

void TPMessagesManager::readAllChats()
{
	QFileInfoList chat_dbs;
	appUtils()->scanDir(appUserModel()->mainUserDir(), chat_dbs, "*.db.sqlite"_L1, "chat"_L1, true);
	for (const auto &db_file : std::as_const(chat_dbs))
		createChatMessage(db_file.baseName(), true);
}

TPChat *TPMessagesManager::createChatMessage(QString &&userid, const bool check_unread_messages)
{
	TPMessage *top_level_msg{createTopLevelMessage(userid)};
	const auto msg_id{chatID(userid)};
	if (!message(top_level_msg, msg_id)) {
		TPMessage *chat_message{new TPMessage{top_level_msg}};
		chat_message->setId(msg_id);
		chat_message->setUserId(std::forward<QString>(userid));
		chat_message->setType(TPUtils::chatmessage_prefix);
		chat_message->setDateTime(std::move(QDateTime::currentDateTime()));
		chat_message->setTitle(std::move(tr("Chat")));
		chat_message->setIcon(std::move("chat_"_L1));
		chat_message->setSticky(true);
		chat_message->setExtraImage(std::move("new-messages"_L1));
		chat_message->insertAction(tr("Open chat"), TPMessage::AT_BUTTON, [this,chat_message] (const QVariant &) -> QVariant {
			openChatWindow(m_chatsList.value(chat_message->userid())->chat);
			return QVariant{};
		});
		chat_message->insertAction(tr("Clear chat"), TPMessage::AT_BUTTON, [this,chat_message] (const QVariant &) -> QVariant {
			m_chatsList.value(chat_message->userid())->chat->clearChat();
			delete m_chatsList.value(chat_message->userid())->dialog;
			delete m_chatsList.value(chat_message->userid())->chat;
			m_chatsList.remove(chat_message->userid());
			emit chat_message->killMessage(chat_message);
			return QVariant{};
		});

		TPChat *new_chat{new TPChat{userid, check_unread_messages, this}};
		connect(new_chat, &TPChat::interlocutorNameChanged, this, [this,chat_message,new_chat] () {
			chat_message->setTitle(static_cast<const QString&>(new_chat->interlocutorName()));
		});
		connect(new_chat, &TPChat::avatarIconChanged, this, [this,chat_message,new_chat] () {
			chat_message->setIcon(static_cast<const QString&>(new_chat->avatarIcon()));
		});
		connect(new_chat, &TPChat::unreadMessagesChanged, this, [this,chat_message,new_chat] () {
			chat_message->setExtraInfo(static_cast<const QString&>(QString::number(new_chat->unreadMessages())));
		});
		st_Chat chat_data{new_chat, nullptr};
		m_chatsList.insert(userid, &chat_data);
		addMessage(chat_message);
		return new_chat;
	}
	return nullptr;

}

void TPMessagesManager::openChatWindow(TPChat *chat_manager)
{
	QObject *chat_dialog{m_chatsList.value(chat_manager->otherUserId())->dialog};
	if (!chat_dialog) {
		if (!m_chatWindowComponent) {
			m_chatWindowComponent = new QQmlComponent{appQmlEngine(), "TpQml.User"_L1, "ChatWindow"_L1, QQmlComponent::Asynchronous};
			connect(m_chatWindowComponent, &QQmlComponent::statusChanged, this, [this,chat_manager] (QQmlComponent::Status status) {
				openChatWindow(chat_manager);
			});
		}
		switch (m_chatWindowComponent->status()) {
		case QQmlComponent::Ready: {
			m_chatWindowComponent->disconnect();
			chat_manager->loadChat();
			m_chatWindowProperties["chatManager"_L1] = std::move(QVariant::fromValue(chat_manager));
			QObject *chat_dialog{m_chatWindowComponent->createWithInitialProperties(
												m_chatWindowProperties, appQmlEngine()->rootContext())};
#ifndef QT_NO_DEBUG
			if (!chat_dialog) {
				qCritical() << m_chatWindowComponent->errorString();
				return;
			}
#endif
			appQmlEngine()->setObjectOwnership(chat_dialog, QQmlEngine::CppOwnership);
			chat_manager->setChatWindow(chat_dialog);
			m_chatsList.value(chat_manager->otherUserId())->dialog = chat_dialog;
			openChatWindow(chat_manager);
			break;
		}
		case QQmlComponent::Loading:
			break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_chatWindowComponent->errorString();
			#endif
			break;
		}
	}
	else
		appPagesListModel()->openPopup(chat_dialog, appItemManager()->appHomePage());
}

void TPMessagesManager::openChat(const uint user_idx)
{
	QString userid{appUserModel()->userId(user_idx)};
	TPMessage *top_level_message{topLevelMessage(userid)};
	if (!message(top_level_message, chatID(userid)))
		createChatMessage(std::move(userid), false);
	openChatWindow(m_chatsList.value(userid)->chat);
}

void TPMessagesManager::startMessagesPolling(const QString &userid)
{
	connect(appUserModel(), &DBUserModel::canConnectToServerChanged, this, [this] () {
		if (!appUserModel()->canConnectToServer())
			m_checkMessagesTimer->stop();
		else
			m_checkMessagesTimer->start();
	});

	m_checkMessagesTimer = new QTimer{this};
	const QLatin1StringView seed{QString{userid + "check_chat_messages"_L1}.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,requestid]
									(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			if (ret_code == TP_RET_CODE_SUCCESS)
				parseNewChatMessages(ret_string);
		}
	});
	const QLatin1StringView seed2{QString{userid + "check_tp_messages"_L1}.toLatin1()};
	const int requestid2{appUtils()->generateUniqueId(seed2)};
	connect(appOnlineServices(), &TPOnlineServices::networkListReceived, this, [this,requestid2]
								(const int request_id, const int ret_code, const QStringList &ret_list) {
		if (request_id == requestid2) {
			if (ret_code == TP_RET_CODE_SUCCESS)
				receivedTPMessages(ret_list);
		}
	});
	m_checkMessagesTimer->callOnTimeout([this,requestid,requestid2] () {
		appOnlineServices()->checkChatMessages(requestid);
		appOnlineServices()->checkTPMessages(requestid2);
		m_checkMessagesTimer->setInterval(newMessagesCheckingInterval());
	});
	m_checkMessagesTimer->start();
}

QVariant TPMessagesManager::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != tpMessageRole)
		return QVariant{};
	const TPMessage *message{static_cast<const TPMessage*>(index.internalPointer())};
	return QVariant::fromValue(message);
}

Qt::ItemFlags TPMessagesManager::flags(const QModelIndex &index) const
{
	return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QModelIndex TPMessagesManager::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex{};
	TPMessage *parent_message{parent.isValid()
							   ? static_cast<TPMessage*>(parent.internalPointer())
							   : m_rootMessage.get()};
	if (auto *childItem{parent_message->child(row)})
		return createIndex(row, column, childItem);
	return QModelIndex{};
}

QModelIndex TPMessagesManager::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex{};
	auto *childItem{static_cast<TPMessage*>(index.internalPointer())};
	TPMessage *parent_message{childItem->parent()};
	return parent_message != m_rootMessage.get()
										? createIndex(parent_message->row(), 0, parent_message)
										: QModelIndex{};
}

int TPMessagesManager::rowCount(const QModelIndex &parent) const
{
	const TPMessage *parent_message{parent.isValid()
									 ? static_cast<const TPMessage*>(parent.internalPointer())
									 : m_rootMessage.get()};
	return parent_message->childCount();
}

int TPMessagesManager::newMessagesCheckingInterval() const
{
	int msecs{20000};
	int last_sent{0}, last_received{0};
	for (const auto chat : m_chatsList) {
		for (const auto message : std::as_const(chat->chat->m_messages) | std::views::reverse) {
			if (last_sent == 0 && chat->chat->data(message, TPChat::SENT).toBool()) {
				if (chat->chat->data(message, TPChat::SENDER).toString() == appUserModel()->userId(0)) {
					const QDate &sent_date{chat->chat->data(message, TPChat::SDATE).toDate()};
					if (sent_date != QDate::currentDate()) {
						last_sent = -1;
						continue;
					}
					const QTime &sent_time{chat->chat->data(message, TPChat::SDATE).toTime()};
					last_sent = QTime::currentTime().msecsSinceStartOfDay() - sent_time.msecsSinceStartOfDay();
				}
			}
			if (last_received == 0 && chat->chat->data(message, TPChat::RECEIVED).toBool()) {
				if (chat->chat->data(message, TPChat::RECEIVER).toString() != appUserModel()->userId(0)) {
					const QDate &received_date{chat->chat->data(message, TPChat::RDATE).toDate()};
					if (received_date != QDate::currentDate()) {
						last_received = -1;
						continue;
					}
					const QTime &received_time{chat->chat->data(message, TPChat::RTIME).toTime()};
					last_received = QTime::currentTime().msecsSinceStartOfDay() - received_time.msecsSinceStartOfDay();
				}
			}
		}

		if (last_sent == -1 || last_received == -1)
			break;
		else if (last_sent != 0 && last_received != 0) {
			msecs = last_sent - last_received;
			if (msecs < 0)
				msecs *= -1;
			if (msecs < 15*60*1000) {
				if (msecs <= 5*60*1000) {
					if (msecs <= 60*1000)
						msecs = 1000; //Last message exchange was within the last minute. Check again after 1 second
					else
						msecs = 5000; //Last message exchange was between 1 and 5 minutes ago. Check again after 5 seconsd
				}
				else
					msecs = 8000; //Last message exchange was less then 15 minutes ago. Check again after 8 seconds
			}
			//Last message exchange was more then 15 minutes ago. Check again after 20 seconds(the default value)
			break;
		}
	}
	return msecs;
}

void TPMessagesManager::receivedTPMessages(const QStringList &messages)
{
	for (const auto &message : messages) {
		if (message.startsWith(TPUtils::tpmessage_prefix))
			textMessageReceived(message);
	}
}

/*	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender ids)
*/
void TPMessagesManager::parseNewChatMessages(const QString &encoded_messages)
{
	const QStringList &messages_list{encoded_messages.split(set_separator)};
	if (messages_list.isEmpty())
		return;
	for (const auto &encoded_message : messages_list) {
		QString sender_id{std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_SENDER))};
		TPMessage *top_level_message{topLevelMessage(sender_id)};
		TPChat *chat_mngr{nullptr};
		if (!message(top_level_message, chatID(sender_id)))
			chat_mngr = createChatMessage(std::move(sender_id), false);
		else
			chat_mngr = chatManager(sender_id);
		chat_mngr->processChatMessage(encoded_message);
	}
}
