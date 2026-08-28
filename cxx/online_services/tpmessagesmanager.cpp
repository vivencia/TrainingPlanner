#include "tpmessagesmanager.h"

#include "tpchat.h"
#include "tpmessage.h"
#include "tponlineservices.h"
#include "websocketserver.h"
#include "../dbusermodel.h"
#include "../pageslistmodel.h"
#include "../qmlitemmanager.h"
#include "../return_codes.h"
#include "../tpfileops.h"
#include "../tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

enum MessageExtraData {
	MED_NEW_TPMESSAGES,
	MED_NEW_CHATMESSAGES,
	MED_CLEAR_CHAT,
};

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

void TPMessagesManager::newTextMessage(QString &&encoded_message)
{
	QString userid{std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_SENDER))};
	TPMessage *text_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPMessage::MT_TPMESSAGE)};
	if (!text_msg) {
		const QString &c_time{appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_CTIME)};
		text_msg = new TPMessage{topLevelUserMessage(userid)};
		text_msg->setId(fnv1a_hash(userid % c_time));
		text_msg->setUserId(std::move(userid));
		text_msg->setType(TPMessage::MT_TPMESSAGE);
		text_msg->setDateTime(std::move(appUtils()->dateTimeFromString(c_time)));
		text_msg->setExpiration(std::move(appUtils()->dateTimeFromString(
			appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_EXP_TIME))));
		text_msg->setFileName(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_REL_FILEPATH));
		text_msg->setTitle(std::move(text_msg->fileOps() ? tr("You have received a file") : tr("You have a message")));
		text_msg->setIcon(std::move("send-message"_L1));
		text_msg->setText(std::move(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_TEXT)));
		text_msg->setSticky(static_cast<TPBool>(false));
		text_msg->setEncodedMessage(std::forward<QString>(encoded_message));
		setTotalNewMessages(text_msg->parentMessage(), MED_NEW_TPMESSAGES, 1);
		text_msg->insertAction(tr("Dismiss"), TPMessage::AT_BUTTON, -1, [this,text_msg] (const QVariant &) -> QVariant {
			removeMessage(text_msg);
			return QVariant{};
		}, true);
		//killMessage is emitted either when the message expires or by the message's TPFileOps
		connect(text_msg, &TPMessage::killMessage, this, [this,text_msg] () { removeMessage(text_msg); });
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
	TPMessage *chat_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPMessage::MT_CHAT)};
	if (!chat_msg)
		createChatMessage(std::move(userid), false);
	openChatWindow(m_chatsList.value(userid)->chat);
}

void TPMessagesManager::openNewMessageDialog(const uint user_idx)
{
	if (!m_newTPMessageComponent) {
		m_newTPMessageComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "NewTPMessageDialog"_L1, QQmlComponent::Asynchronous};
		connect(m_newTPMessageComponent, &QQmlComponent::statusChanged, this, [this,user_idx] (QQmlComponent::Status status) {
			openNewMessageDialog(user_idx);
		});
	} else {
		if (!m_newTPMessageDialog) {
			switch (m_newTPMessageComponent->status()) {
			case QQmlComponent::Ready:
				m_newTPMessageComponent->disconnect();
				m_newTPMessageDialog = m_newTPMessageComponent->createWithInitialProperties(QVariantMap{
						{"selectedUsers", appUserModel()->userName(user_idx)}}, appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_newTPMessageDialog) {
					qCritical() << m_newTPMessageComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_newTPMessageDialog, QQmlEngine::CppOwnership);
				connect(m_newTPMessageDialog, SIGNAL(sendMessage(QStringList,QString,QString)), this, SLOT(sendTPMessage(QStringList,QString,QString)));
				openNewMessageDialog(user_idx);
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_newTPMessageComponent->errorString();
#endif
				return;
			}
		} else {
			appItemManager()->appPagesManager()->openPopup(m_newTPMessageDialog, appItemManager()->appHomePage(),
																									Qt::AlignBaseline);
		}
	}
}

void TPMessagesManager::showOnlineMessagesManagerDialog(const bool show)
{
	if (m_messagesManagerDialog) {
		appSettings()->setShowOnlineMessagesDialog(show);
		if (show)
			appPagesListModel()->raisePopup(m_messagesManagerDialog);
		else
			appPagesListModel()->hidePopup(m_messagesManagerDialog);
	}
}

void TPMessagesManager::startMessagesManager()
{
	if (!m_messagesManagerComponent) {
		m_messagesManagerComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "OnlineMessages"_L1, QQmlComponent::Asynchronous};
		connect(m_messagesManagerComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
			startMessagesManager();
		});
	} else {
		if (!m_messagesManagerDialog) {
			switch (m_messagesManagerComponent->status()) {
			case QQmlComponent::Ready:
				m_messagesManagerComponent->disconnect();
				m_messagesManagerDialog = m_messagesManagerComponent->create(appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				m_messagesManagerDialog->setProperty("objectName", std::move(QVariant{"onlineMessages"}));
				if (!m_messagesManagerDialog) {
					qCritical() << m_messagesManagerComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_messagesManagerDialog, QQmlEngine::CppOwnership);
				startMessagesManager();
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_messagesManagerComponent->errorString();
#endif
				return;
			}
		} else {
			appItemManager()->appPagesManager()->openPopup(m_messagesManagerDialog, appItemManager()->appHomePage(),
																									Qt::AlignBaseline);
		}
	}
}

void TPMessagesManager::sendTPMessage(const QStringList &users, const QString &message, const QString &filename)
{
	for (const auto &user : users) {
		const QString &encoded_message{appUtils()->makeEncodedMessage(
			TPUtils::tpmessage_prefix,
			appUserModel()->userId(0),
			user,
			appUtils()->formatDateTime(QDateTime::currentDateTime()),
			QString{},
			message,
			filename,
			QString{}
		)};
		sendTPMessage(user, encoded_message);
	}
}

TPMessage *TPMessagesManager::topLevelUserMessage(const QString &userid)
{
	TPMessage *top_level_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPMessage::MT_TOPLEVEL)};
	if (!top_level_msg) {
		const int useridx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, userid)};
		top_level_msg = new TPMessage{m_messagesModel->rootMessage()};
		top_level_msg->setUserId(userid);
		//top_level_msg->setObjectName("Top level for user " + userid);
		top_level_msg->setType(TPMessage::MT_TOPLEVEL);
		top_level_msg->setTitle(std::move(useridx != -1 ? appUserModel()->userName(useridx) : tr("Unknown contact")));
		top_level_msg->setIcon(std::move(useridx != -1 ? appUserModel()->avatar(useridx) : "unknown-user"));
		if (userid != tpsystem_userid) {
			connect(appUserModel(), &DBUserModel::userModified, this, [this,useridx,top_level_msg]
																(const uint user_idx, const uint field) {
				if (user_idx == useridx) {
					if (field == DBUserModel::USER_FIELD_AVATAR)
						top_level_msg->setIcon(appUserModel()->avatar(useridx));
					else if (field == DBUserModel::USER_FIELD_NAME)
						top_level_msg->setTitle(appUserModel()->userName(useridx));
				}
			});
			top_level_msg->insertAction(std::move(tr("New message")), TPMessage::AT_BUTTON, -1,
					[this,top_level_msg,useridx] (const QVariant &data) -> QVariant {
						openNewMessageDialog(useridx);
						return QVariant{};
					});
			top_level_msg->insertAction(std::move(tr("Open chat")), TPMessage::AT_BUTTON, -1,
					[this,top_level_msg,useridx] (const QVariant &data) -> QVariant {
						openChat(useridx);
						return QVariant{};
					});
			//Clear only *clears* the view, it does not empty a chat, nor removes messages from the server nor deletes files
			top_level_msg->insertAction(std::move(tr("Clear")), TPMessage::AT_BUTTON, -1,
					[this,userid,top_level_msg] (const QVariant &data) -> QVariant {
						removeChildrenMessages(top_level_msg, top_level_msg->generalPurposeData(MED_CLEAR_CHAT).toBool()
														? TPMessage::MT_TOPLEVEL : TPMessage::MT_TPMESSAGE);
						return QVariant{};
					});
			top_level_msg->insertAction(tr("Include chat"), TPMessage::AT_CHECKBOX, -1,
					[this,userid,top_level_msg] (const QVariant &data) -> QVariant {
						top_level_msg->setGeneralPurposeData(MED_CLEAR_CHAT, data.toBool());
						return QVariant{};
					}, true);
		}
		m_messagesModel->insertMessage(top_level_msg);
		emit messagesModelChanged();
	}
	return top_level_msg;
}

void TPMessagesManager::receivedTPMessages(const QStringList &messages)
{
	for (auto message : messages) {
		if (message.startsWith(TPUtils::tpmessage_prefix))
			newTextMessage(std::move(message));
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
	TPMessage *chat_msg{m_messagesModel->findMessage(TPMessage::FIELD_USERID, userid, TPMessage::MT_CHAT)};
	if (!chat_msg) {
		TPMessage *chat_message{new TPMessage{topLevelUserMessage(userid)}};
		chat_message->setId(chatID(userid));
		chat_message->setUserId(std::forward<QString>(userid));
		chat_message->setType(TPMessage::MT_CHAT);
		chat_message->setDateTime(std::move(QDateTime::currentDateTime()));
		chat_message->setTitle(std::move(tr("Chat")));
		chat_message->setIcon(std::move("chat_"_L1));
		chat_message->setSticky(static_cast<TPBool>(true));
		chat_message->insertAction(tr("Open chat"), TPMessage::AT_BUTTON, -1,
																	[this,chat_message] (const QVariant &) -> QVariant {
			openChatWindow(m_chatsList.value(chat_message->userid())->chat);
			return QVariant{};
		});
		chat_message->insertAction(tr("Clear chat"), TPMessage::AT_BUTTON, -1,
																	[this,chat_message] (const QVariant &) -> QVariant {
			m_chatsList.value(chat_message->userid())->chat->clearChat();
			delete m_chatsList.value(chat_message->userid())->dialog;
			delete m_chatsList.value(chat_message->userid())->chat;
			m_chatsList.remove(chat_message->userid());
			removeMessage(chat_message);
			return QVariant{};
		}, true);

		TPChat *new_chat{new TPChat{userid, check_unread_messages, this}};
		connect(new_chat, &TPChat::unreadMessagesChanged, this, [this,chat_message,new_chat] () {
			setTotalNewMessages(chat_message->parentMessage(), MED_NEW_CHATMESSAGES, new_chat->unreadMessages());
			if (new_chat->unreadMessages() > 0) {
				chat_message->setExtraImage(std::move("new-messages"_L1));
				chat_message->setExtraInfo(std::move(QString::number(new_chat->unreadMessages())));
			} else {
				chat_message->setExtraImage(std::move(QString{}));
				chat_message->setExtraInfo(QString{});
			}
		});
		st_Chat chat_data{new_chat, nullptr};
		static_cast<void>(m_chatsList.emplace(userid, &chat_data));
		m_messagesModel->insertMessage(chat_message, 0);
		return new_chat;
	} else {
		return chatManager(userid);
	}
}

void TPMessagesManager::removeChildrenMessages(TPMessage *msg, const int exclude_type)
{
	if (appUserModel()->canConnectToServer()) {
		int _type{TPMessage::MT_TPMESSAGE|TPMessage::MT_CHAT};
		unSetBit(_type, exclude_type);
		const QList<TPMessage*> &messages{m_messagesModel->findMessages(TPMessage::FIELD_USERID,
																				msg->userid(), _type)};
		for (const auto message : std::as_const(messages)) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == message->id()) {
					disconnect(*conn);
					if (ret_code == TP_RET_CODE_SUCCESS) {
						setTotalNewMessages(message->parentMessage(), MED_NEW_TPMESSAGES, -1);
						m_messagesModel->removeMessage(message);
					}
				}
			});
			appOnlineServices()->removeTPMessage(message->id(), message->encodedMessage());
		}
	} //TODO schedule online services to run when we have connection to the server
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (msg->type() == TPMessage::MT_TPMESSAGE) {
		if (appUserModel()->canConnectToServer()) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,msg,conn]
												(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == msg->id()) {
					disconnect(*conn);
					setTotalNewMessages(msg->parentMessage(), MED_NEW_TPMESSAGES, -1);
					m_messagesModel->removeMessage(msg);
				}
			});
			appOnlineServices()->removeTPMessage(msg->id(), msg->encodedMessage());
		} //TODO schedule online services to run when we have connection to the server
	} else {
		setTotalNewMessages(msg->parentMessage(), MED_NEW_TPMESSAGES, -1);
		m_messagesModel->removeMessage(msg);
	}
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

void TPMessagesManager::setTotalNewMessages(TPMessage *top_level_msg, const int key, const int new_messages)
{
	top_level_msg->setGeneralPurposeData(key, top_level_msg->generalPurposeData(key).toInt() + new_messages);
	const int total{top_level_msg->generalPurposeData(MED_NEW_TPMESSAGES).toInt() +
												top_level_msg->generalPurposeData(MED_NEW_CHATMESSAGES).toInt()};
	if (total > 0) {
		top_level_msg->setExtraInfo(std::move(QString::number(total)));
		top_level_msg->setExtraImage(std::move("new-messages"_L1));
	} else {
		top_level_msg->setExtraInfo(std::move(QString{}));
		top_level_msg->setExtraImage(std::move(QString{}));
	}
}
