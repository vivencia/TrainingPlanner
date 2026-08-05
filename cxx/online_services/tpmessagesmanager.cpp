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

TPMessagesManager::TPMessagesManager(QObject *parent)
	: QObject{parent}, m_messagesModel{new TPMessagesModel}
{
	_appMessagesManager = this;
	REGISTER_QML_SINGLETON(TPMessagesManager, this);
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

void TPMessagesManager::newTextMessage(const QString &encoded_message)
{
	QString userid{std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_SENDER))};
	TPMessage *text_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPUtils::tpmessage_prefix)};
	if (!text_msg) {
		const QString &c_time{appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_CTIME)};
		text_msg = new TPMessage{topLevelUserMessage(userid)};
		text_msg->setId(fnv1a_hash(userid % c_time));
		text_msg->setUserId(std::move(userid));
		text_msg->setType(TPUtils::tpmessage_prefix);
		text_msg->setDateTime(std::move(appUtils()->dateTimeFromString(c_time)));
		text_msg->setExpiration(std::move(appUtils()->dateTimeFromString(
			appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_EXP_TIME))));
		text_msg->setFileName(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_REL_FILEPATH));
		text_msg->setTitle(std::move(text_msg->fileOps() ? tr("You have received a file") : tr("You have a message")));
		text_msg->setIcon(std::move("send-message"_L1));
		text_msg->setText(std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_TEXT)));
		text_msg->setSticky(false);
		text_msg->insertAction(tr("Dismiss"), TPMessage::AT_BUTTON, [this,text_msg] (const QVariant &) -> QVariant {
			removeMessage(text_msg);
			return QVariant{};
		});
		m_messagesModel->insertMessage(text_msg);
		emit messagesModelChanged();
	}
}

void TPMessagesManager::sendTPMessage(const QString &target_user, const QString &encoded_message, const int request_id)
{
	auto send_result = [this,target_user] (const int requestid, const bool sent) -> void {
		emit TPMessageSent(requestid, sent);
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(appUtils()->string_strings(
			{ sent ? tr("Success!") : tr("Error!")
			, sent ? tr("Message sent to") : tr("Try again. Could not sent message to ")
			% appUserModel()->userNameFromId(target_user)}, record_separator)), Qt::AlignCenter
			, std::move(sent ? "set-completed"_L1 : "error"_L1));
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
				if (established) {
					message_sent = appWSServer()->sendTextMessage(encoded_message);
				} else {
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

void TPMessagesManager::readAllChats()
{
	QFileInfoList chat_dbs;
	appUtils()->scanDir(appUserModel()->mainUserDir(), chat_dbs, "*.db.sqlite"_L1, "chat"_L1, true);
	for (const auto &db_file : std::as_const(chat_dbs))
		createChatMessage(db_file.baseName(), true);
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
	TPMessage *chat_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPUtils::chatmessage_prefix)};
	if (!chat_msg)
		createChatMessage(std::move(userid), false);
	openChatWindow(m_chatsList.value(userid)->chat);
}

void TPMessagesManager::openNewMessageDialog(const uint user_idx)
{
	//TODO
}

TPMessage *TPMessagesManager::topLevelUserMessage(const QString &userid)
{
	TPMessage *top_level_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, tp_toplevel_message)};
	if (!top_level_msg) {
		const int user_idx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, userid)};
		top_level_msg = new TPMessage{m_messagesModel->rootMessage()};
		top_level_msg->setUserId(userid);
		top_level_msg->setObjectName("Top level for user " + userid);
		top_level_msg->setType("topLevel"_L1);
		top_level_msg->setTitle(std::move(user_idx != -1 ? appUserModel()->userName(user_idx) : tr("Unknown contact")));
		top_level_msg->setIcon(std::move(user_idx != -1 ? appUserModel()->avatar(user_idx) : "unknown-user"));
		if (userid != tpsystem_userid) {
			top_level_msg->insertAction(std::move(tr("Send Message")), TPMessage::AT_BUTTON,
					[this,top_level_msg,user_idx] (const QVariant &data) -> QVariant {
						openNewMessageDialog(user_idx);
						return QVariant{};
					});
			top_level_msg->insertAction(std::move(tr("Clear")), TPMessage::AT_BUTTON,
					[this,userid,top_level_msg] (const QVariant &data) -> QVariant {
						removeChildrenMessages(top_level_msg, top_level_msg->generalPurposeData().toBool()
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
		m_messagesModel->insertMessage(top_level_msg);
		emit messagesModelChanged();
	}
	return top_level_msg;
}

void TPMessagesManager::receivedTPMessages(const QStringList &messages)
{
	for (const auto &message : messages) {
		if (message.startsWith(TPUtils::tpmessage_prefix))
			newTextMessage(message);
	}
}

/*	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender ids)
*/
void TPMessagesManager::parseNewChatMessages(const QString &encoded_messages)
{
	const QStringList &messages_list{encoded_messages.split(set_separator)};
	for (const auto &encoded_message : messages_list) {
		QString sender_id{std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_SENDER))};
		TPChat *chat_mngr{createChatMessage(std::move(sender_id), false)};
		chat_mngr->processChatMessage(encoded_message);
	}
}

TPChat *TPMessagesManager::createChatMessage(QString &&userid, const bool check_unread_messages)
{
	TPMessage *chat_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPUtils::chatmessage_prefix)};
	if (!chat_msg) {
		TPMessage *chat_message{new TPMessage{topLevelUserMessage(userid)}};
		chat_message->setId(chatID(userid));
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
			removeMessage(chat_message);
			return QVariant{};
		});

		TPChat *new_chat{new TPChat{userid, check_unread_messages, this}};
		connect(new_chat, &TPChat::interlocutorNameChanged, this, [this,chat_message,new_chat] () {
			chat_message->setTitle(std::move(new_chat->interlocutorName()));
		});
		connect(new_chat, &TPChat::avatarIconChanged, this, [this,chat_message,new_chat] () {
			chat_message->setIcon(std::move(new_chat->avatarIcon()));
		});
		connect(new_chat, &TPChat::unreadMessagesChanged, this, [this,chat_message,new_chat] () {
			chat_message->setExtraInfo(std::move(QString::number(new_chat->unreadMessages())));
		});
		st_Chat chat_data{new_chat, nullptr};
		m_chatsList.emplace(userid, &chat_data);
		m_messagesModel->insertMessage(chat_message);
		return new_chat;
	} else {
		return chatManager(userid);
	}
}

void TPMessagesManager::removeChildrenMessages(TPMessage *msg, const QLatin1StringView &exclude_type)
{
	if (appUserModel()->canConnectToServer()) {
		const QList<TPMessage*> &messages{m_messagesModel->findMessages(TPMessage::FIELD_USERID, msg->userid(),
			exclude_type == TPUtils::tpmessage_prefix ? TPUtils::chatmessage_prefix : TPUtils::tpmessage_prefix)};
		for (const auto message : std::as_const(messages)) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == message->id()) {
					disconnect(*conn);
					if (ret_code == TP_RET_CODE_SUCCESS)
						msg->removeChild(message);
				}
			});
			appOnlineServices()->removeTPMessage(message->id(), message->encodedMessage());
		}
	} //TODO schedule online services to run when we have connection to the server
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (appUserModel()->canConnectToServer()) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,msg,conn]
									(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == msg->id()) {
				disconnect(*conn);
				msg->parentMessage()->removeChild(msg);
			}
		});
		appOnlineServices()->removeTPMessage(msg->id(), msg->encodedMessage());
	} //TODO schedule online services to run when we have connection to the server
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

		if (last_sent == -1 || last_received == -1) {
			break;
		} else if (last_sent != 0 && last_received != 0) {
			msecs = last_sent - last_received;
			if (msecs < 0)
				msecs *= -1;
			if (msecs < 15*60*1000) {
				if (msecs <= 5*60*1000) {
					if (msecs <= 60*1000)
						msecs = 1000; //Last message exchange was within the last minute. Check again after 1 second
					else
						msecs = 5000; //Last message exchange was between 1 and 5 minutes ago. Check again after 5 seconsd
				} else {
					msecs = 8000; //Last message exchange was less then 15 minutes ago. Check again after 8 seconds
				}
			}
			//Last message exchange was more then 15 minutes ago. Check again after 20 seconds(the default value)
			break;
		}
	}
	return msecs;
}
