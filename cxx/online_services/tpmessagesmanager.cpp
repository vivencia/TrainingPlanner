#include "tpmessagesmanager.h"

#include "tpchat.h"
#include "tpmessage.h"
#include "tponlineservices.h"
#include "../dbusermodel.h"
#include "../qmlitemmanager.h"
#include "../tputils.h"
#include "../tpkeychain/tpkeychain.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

#include <ranges>

TPMessagesManager *TPMessagesManager::_appMessagesManager{nullptr};

enum RoleNames
{
	idRole				= Qt::UserRole,
	labelTextRole		= Qt::UserRole + 1,
	iconRole			= Qt::UserRole + 2,
	dateRole			= Qt::UserRole + 3,
	timeRole			= Qt::UserRole + 4,
	extraInfoLabelRole	= Qt::UserRole + 5,
	extraInfoIconRole	= Qt::UserRole + 6,
	actionsRole			= Qt::UserRole + 7,
	stickyRole			= Qt::UserRole + 8,
	hasActionsRole		= Qt::UserRole + 9,
};

TPMessagesManager::TPMessagesManager(QObject *parent)
	: QAbstractListModel{parent}, m_chatWindowComponent{nullptr}
{
	_appMessagesManager = this;
	m_roleNames[idRole]				= std::move("id");
	m_roleNames[labelTextRole]		= std::move("labelText");
	m_roleNames[iconRole]			= std::move("msgicon");
	m_roleNames[dateRole]			= std::move("msgdate");
	m_roleNames[timeRole]			= std::move("msgtime");
	m_roleNames[extraInfoLabelRole]	= std::move("extraInfo");
	m_roleNames[extraInfoIconRole]	= std::move("extraInfoIcon");
	m_roleNames[actionsRole]		= std::move("actions");
	m_roleNames[stickyRole]			= std::move("sticky");
	m_roleNames[hasActionsRole]		= std::move("hasActions");
}

TPMessage *TPMessagesManager::message(const int message_id) const
{
	const auto it{std::find_if(m_data.cbegin(), m_data.cend(), [message_id] (const auto msg)
	{
		return msg->id() == message_id;
	})};
	return it != m_data.cend() ? *it : nullptr;
}

std::optional<int> TPMessagesManager::addMessage(TPMessage *msg)
{
	if (msg && !msg->plugged())
	{
		beginInsertRows(QModelIndex{}, count(), count());
		const QLatin1StringView v{std::move(msg->_displayText().toLatin1())};
		if (msg->id() == -1) //do not override an id set elsewhere
			msg->setId(appUtils()->generateUniqueId(v));
		m_data.append(msg);
		endInsertRows();
		emit countChanged();
		msg->setPlugged(true);
		connect(msg, &TPMessage::actionTriggered, this, [this,msg] (const int action_id, const std::optional<bool> remove_message) {
			if (remove_message.has_value())
			{
				if (remove_message.value())
					removeMessage(msg);
			}
			else
				if (!msg->sticky())
					removeMessage(msg);
		});
		return msg->id();
	}
	return std::nullopt;
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (msg != nullptr)
	{
		const qsizetype row{m_data.indexOf(msg)};
		if (row >= 0)
		{
			beginRemoveRows(QModelIndex{}, row, row);
			m_data.remove(row);
			if (msg->autoDelete())
				delete msg;
			emit countChanged();
			emit dataChanged(QModelIndex{}, QModelIndex{}); //Needed because this is the only signal the QML side is getting, but I don't know why
			endRemoveRows();
		}
	}
}

void TPMessagesManager::execAction(const int message_id, const uint action_id)
{
	message(message_id)->execAction(action_id);
}

void TPMessagesManager::itemClicked(const int message_id)
{
	TPMessage *msg{message(message_id)};
	if (msg)
	{
		const QString &userid{QString::number(message_id)};
		TPChat *chat{m_chatsList.value(userid)};
		if (chat)
		{
			openChatWindow(chat);
			msg->setExtraInfoLabel(QString{});
		}
	}
}

void TPMessagesManager::filterChatOptions(const QString &filter)
{

}

TPMessage *TPMessagesManager::createChatMessage(const QString &userid, QString &&display_text, QString &&icon_source)
{
	TPMessage *chat_message{new TPMessage{std::move(display_text), std::move(icon_source), this}};
	chat_message->setId(userid.toLong());
	chat_message->setExtraInfoImage("new-messages");
	chat_message->insertAction(tr("Chat"), [this,userid] (const QVariant &) {
		openChatWindow(m_chatsList.value(userid));
	});
	chat_message->insertAction(tr("Delete"), [this,userid] (const QVariant &) {
		m_chatsList.value(userid)->clearChat();
		m_chatsList.remove(userid);
		removeChatWindow(userid);
	});
	chat_message->plug();
	TPChat *new_chat{new TPChat{userid, this}};
	m_chatsList.insert(userid, new_chat);
	return chat_message;
}

void TPMessagesManager::openChatWindow(TPChat *chat_manager)
{
	//QMetaObject::invokeMethod(appMainWindow(), "showChatWindow", Q_ARG(TPChat*, chat_manager));
	//return;
	if (!m_chatWindowList.value(chat_manager->otherUserId()))
	{
		if (!m_chatWindowComponent)
		{
			m_chatWindowProperties.insert("parentPage"_L1, QVariant::fromValue(appItemManager()->appHomePage()));
			m_chatWindowComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/User/ChatWindow.qml"_L1}, QQmlComponent::Asynchronous};
		}
		switch (m_chatWindowComponent->status())
		{
			case QQmlComponent::Ready:
				createChatWindow_part2(chat_manager);
			break;
			case QQmlComponent::Loading:
				connect(m_chatWindowComponent, &QQmlComponent::statusChanged, this, [this,chat_manager] (QQmlComponent::Status status) {
					openChatWindow(chat_manager);
				}, Qt::SingleShotConnection);
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
		QMetaObject::invokeMethod(m_chatWindowList.value(chat_manager->otherUserId()), "open");
}

void TPMessagesManager::openChat(const QString &username)
{
	const QString &userid{appUserModel()->userIdFromFieldValue(USER_COL_NAME, username)};
	const int i_userid{userid.toInt()};

	if (!message(i_userid))
		createChatMessage(userid, std::move(QString{username}), std::move(appUserModel()->avatar(i_userid)));
	openChatWindow(m_chatsList.value(userid));
}

QVariant TPMessagesManager::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_data.count())
	{
		switch (role)
		{
			case idRole: return m_data.at(row)->id();
			case labelTextRole: return m_data.at(row)->displayText();
			case iconRole: return m_data.at(row)->iconSource();
			case dateRole: return m_data.at(row)->date();
			case timeRole: return m_data.at(row)->time();
			case extraInfoLabelRole: return m_data.at(row)->extraInfoLabel();
			case extraInfoIconRole: return m_data.at(row)->extraInfoImage();
			case actionsRole: return m_data.at(row)->actions();
			case stickyRole: return m_data.at(row)->sticky();
			case hasActionsRole: return m_data.at(row)->hasActions();
		}
	}
	return QVariant{};
}

void TPMessagesManager::startChatMessagesPolling()
{
	m_newChatMessagesTimer = new QTimer{this};
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value)
	{
		const QLatin1StringView seed{QString{key + "check_messages"_L1}.toLatin1()};
		const int requestid{appUtils()->generateUniqueId(seed)};
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,requestid]
								(const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				if (ret_code == 0)
					parseNewChatMessages(ret_string);
				#ifndef QT_NO_DEBUG
				else
					qDebug() << ret_string;
				#endif
			}
		});
		m_newChatMessagesTimer->callOnTimeout( [this,requestid,key,value] () {
			appOnlineServices()->checkMessages(requestid, key, value);
			m_newChatMessagesTimer->setInterval(newMessagesCheckingInterval());
		});
	}, Qt::SingleShotConnection);
	appKeyChain()->readKey(appUserModel()->userId(0));
	m_newChatMessagesTimer->start();
}

int TPMessagesManager::newMessagesCheckingInterval() const
{
	int msecs{20000};
	int last_sent{0}, last_received{0};
	for (const auto chat : m_chatsList)
	{
		if (last_sent == -1 || last_received == -1)
			break;
		else if (last_sent != 0 && last_received != 0)
		{
			msecs = last_sent - last_received;
			if (msecs < 0)
				msecs *= -1;
			if (msecs < 15*60*1000)
			{
				if (msecs <= 5*60*1000)
				{
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
		for (const auto message : std::as_const(chat->m_messages) | std::views::reverse)
		{
			if (last_sent == 0 && chat->data(message, MESSAGE_SENT).toBool())
			{
				if (chat->data(message, MESSAGE_SENDER).toString() == appUserModel()->userId(0))
				{
					const QDate &sent_date{chat->data(message, MESSAGE_SDATE).toDate()};
					if (sent_date != QDate::currentDate())
					{
						last_sent = -1;
						continue;
					}
					const QTime &sent_time{chat->data(message, MESSAGE_SDATE).toTime()};
					last_sent = QTime::currentTime().msecsSinceStartOfDay() - sent_time.msecsSinceStartOfDay();
				}
			}
			if (last_received == 0 && chat->data(message, MESSAGE_RECEIVED).toBool())
			{
				if (chat->data(message, MESSAGE_RECEIVER).toString() != appUserModel()->userId(0))
				{
					const QDate &received_date{chat->data(message, MESSAGE_RDATE).toDate()};
					if (received_date != QDate::currentDate())
					{
						last_received = -1;
						continue;
					}
					const QTime &received_time{chat->data(message, MESSAGE_RTIME).toTime()};
					last_received = QTime::currentTime().msecsSinceStartOfDay() - received_time.msecsSinceStartOfDay();
				}
			}
		}
	}
	return msecs;
}

/*
	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender ids)
*/

void TPMessagesManager::parseNewChatMessages(const QString &encoded_messages)
{
	uint sender_idx{0};
	do {
		QString sender_id{std::move(appUtils()->getCompositeValue(sender_idx, encoded_messages, exercises_separator))};
		if (sender_id.isEmpty())
			break;
		QString sender_messages{std::move(appUtils()->getCompositeValue(++sender_idx, encoded_messages, exercises_separator))};
		if (sender_messages.isEmpty())
			break;
		const uint i_sender_id{sender_id.toUInt()};
		TPMessage *chat_message{message(i_sender_id)};
		if (!chat_message)
		{
			const int user_idx{appUserModel()->userIdxFromFieldValue(USER_COL_ID, sender_id)};
			QString user_name{user_idx != -1 ? appUserModel()->userName(user_idx) : std::move(tr("Unknown contact"))};
			QString user_icon{user_idx != -1 ? appUserModel()->avatar(user_idx) : std::move("unknown-user")};
			chat_message = createChatMessage(sender_id, std::move(user_name), std::move(user_icon));
		}
		TPChat *chat_mngr{chatManager(sender_id)};
		uint msg_idx{0};
		do {
			const QString &encoded_message{appUtils()->getCompositeValue(msg_idx, sender_messages, set_separator)};
			if (encoded_message.isEmpty())
				break;
			chat_mngr->incomingMessage(encoded_message);
		} while (++msg_idx);
		chat_message->setExtraInfoLabel(QString::number(msg_idx));
	} while (++sender_idx);
}

void TPMessagesManager::createChatWindow_part2(TPChat *chat_manager)
{
	m_chatWindowProperties.insert("chatManager"_L1, QVariant::fromValue(chat_manager));
	QObject *chat_window{m_chatWindowComponent->createWithInitialProperties(m_chatWindowProperties, appQmlEngine()->rootContext())};
	appQmlEngine()->setObjectOwnership(chat_window, QQmlEngine::CppOwnership);
	chat_window->setProperty("parent", QVariant::fromValue(appItemManager()->appHomePage()));
	QMetaObject::invokeMethod(chat_window, "open");
	m_chatWindowList.insert(chat_manager->otherUserId(), chat_window);
}

void TPMessagesManager::removeChatWindow(const QString &other_userid)
{
	QObject *chat_window{m_chatWindowList.value(other_userid)};
	if (chat_window)
	{
		delete chat_window;
		m_chatWindowList.remove(other_userid);
	}
}
