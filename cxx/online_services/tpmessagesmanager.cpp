#include "tpmessagesmanager.h"

#include "tpchat.h"
#include "tpmessage.h"
#include "tponlineservices.h"
#include "../dbusermodel.h"
#include "../qmlitemmanager.h"
#include "../tpfilepath.h"
#include "../tpfileops.h"
#include "../tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

#include <ranges>

TPMessagesManager *TPMessagesManager::_appMessagesManager{nullptr};

static inline QString userIdFromExchangeFile(const QString &filename)
{
	const auto subdir_len{TPMessagesManager::tpmessages_subdir.length()};
	const auto slash1_idx{filename.indexOf(TPMessagesManager::tpmessages_subdir) + subdir_len};
	if (slash1_idx >= subdir_len) {
		const auto slash2_idx{filename.indexOf('/', slash1_idx + subdir_len + 1)};
		return filename.sliced(slash1_idx, slash2_idx - slash1_idx - 1);
	}
	return QString{};
}

enum RoleNames
{
	createRole(msgId,				TPMESSAGE_FIELD_ID)
	createRole(msgTitle,			TPMESSAGE_FIELD_TITLE)
	createRole(msgText,				TPMESSAGE_FIELD_TEXT)
	createRole(msgIcon,				TPMESSAGE_FIELD_ICON)
	createRole(msgFileName,			TPMESSAGE_FIELD_FILE)
	createRole(msgExtraInfoText,	TPMESSAGE_FIELD_EXTRA_INFO)
	createRole(msgExtraInfoIcon,	TPMESSAGE_FIELD_EXTRA_ICON)
	createRole(msgDate,				TPMESSAGE_FIELD_DATE)
	createRole(msgTime,				TPMESSAGE_FIELD_TIME)
	createRole(msgActions,			TPMESSAGE_FIELD_ACTIONS)
	createRole(msgSticky,			TPMESSAGE_FIELD_STICKY)
	createRole(msgHasActions,		msgStickyRole + 1)
};

TPMessagesManager::TPMessagesManager(QObject *parent) : QAbstractListModel{parent}
{
	_appMessagesManager = this;
	REGISTER_QML_SINGLETON(TPMessagesManager, this);

	roleToString(msgId)
	roleToString(msgTitle)
	roleToString(msgText)
	roleToString(msgIcon)
	roleToString(msgFileName)
	roleToString(msgExtraInfoText)
	roleToString(msgExtraInfoIcon)
	roleToString(msgDate)
	roleToString(msgTime)
	roleToString(msgActions)
	roleToString(msgSticky)
	roleToString(msgHasActions)
}

void TPMessagesManager::readAllChats()
{
	QFileInfoList chat_dbs;
	appUtils()->scanDir(appUserModel()->userDir() + TPChat::chatsSubDir, chat_dbs, "*.db.sqlite"_L1);
	for (const auto &db_file : std::as_const(chat_dbs)) {
		if (!message(db_file.baseName().toLong()))
			static_cast<void>(createChatMessage(db_file.baseName(), true));
	}
}

TPMessage *TPMessagesManager::message(const qsizetype message_id) const
{
	const auto it{std::find_if(m_data.cbegin(), m_data.cend(), [message_id] (const auto msg) {
		return msg->id() == message_id;
	})};
	return it != m_data.cend() ? *it : nullptr;
}

void TPMessagesManager::addMessage(TPMessage *msg)
{
	beginInsertRows(QModelIndex{}, count(), count());
	const QLatin1StringView v{msg->text().toLatin1()};
	if (msg->id() == -1) //do not override an id set elsewhere
		msg->setId(appUtils()->generateUniqueId(v));
	m_data.append(msg);
	endInsertRows();
	emit countChanged();
	connect(msg, &TPMessage::dataChanged, this, [this] (const uint field) {
		const auto idx{m_data.indexOf(sender())};
		if (idx >= 0)
			emit dataChanged(index(idx, 0), index(idx, 0), QList<int>{static_cast<int>(Qt::UserRole + field)} );
	});
	connect(msg, &TPMessage::killMessage, this, [this] (TPMessage *message) {
		message->setSticky(false);
		removeMessage(message);
	});
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (msg != nullptr) {
		const qsizetype row{m_data.indexOf(msg)};
		if (row >= 0) {
			appOnlineServices()->removeFile(appUtils()->generateUniqueId(), msg->fileName()->fileName(),
																							msg->fileName()->subdirs());

			beginRemoveRows(QModelIndex{}, row, row);
			m_data.remove(row);
			if (!msg->sticky())
				delete msg;
			emit countChanged();
			emit dataChanged(QModelIndex{}, QModelIndex{}); //Needed because this is the only signal the QML side is getting, but I don't know why
			endRemoveRows();
		}
	}
}

void TPMessagesManager::execAction(const int message_index, const uint action_id)
{
	if (message_index >= 0 && message_index < m_data.count()) {
		TPMessage *msg{m_data.at(message_index)};
		msg->execAction(action_id);
	}
}

void TPMessagesManager::binaryFileReceived(const QByteArray &data, const QString &meta_info)
{
	const QString &filename{appUtils()->binaryFileMetaInfoFieldValue(meta_info, TPUtils::BFIF_FILEPATH)};
	auto tp_filename{TPFilePath::newTPFilePath(filename, appUserModel()->userId(0),
											   appUtils()->binaryFileMetaInfoFieldValue(meta_info, TPUtils::BFIF_SENDERID))};
	const int id{tp_filename->generateUniqueId()};

	if (message(id) == nullptr) {
		TPMessage *new_message{new TPMessage{}};
		new_message->setId(id);
		new_message->setTitle(std::move(appUserModel()->userNameFromId(tp_filename->targetUser()) % tr(" has sent you a file: ")));
		new_message->setIconSource(std::move(appUserModel()->avatarFromId(tp_filename->targetUser())));
		new_message->setFileName(*tp_filename);
		new_message->insertData(meta_info);
		new_message->setSticky(false);
		new_message->plug();
	}
}

void TPMessagesManager::textMesssageReceived(const QString &msg, const TPFilePath &filename)
{
	QString msg__{msg};
	const int id{appUtils()->idFromString(msg__.length() <= 30 ? msg__ : msg__.sliced(5, 25))};
	if (message(id) == nullptr) {
		TPMessage *new_message{new TPMessage{}};
		new_message->setId(id);
		new_message->setTitle(std::move(tr("Message from ") % appUserModel()->userNameFromId(filename.targetUser())));
		new_message->setIconSource(std::move("send-message"_L1));
		new_message->setFileName(filename);
		new_message->setText(std::move(msg__));
		new_message->setSticky(false);
		new_message->insertAction(tr("Delete"), [this,new_message] (const QVariant &) { new_message->unplug(); });
		new_message->plug();
	}
}

TPMessage *TPMessagesManager::createChatMessage(const QString &userid, const bool check_unread_messages)
{
	const int user_idx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, userid)};
	QString user_name{std::move(user_idx != -1 ? appUserModel()->userName(user_idx) : tr("Unknown contact"))};
	QString user_icon{std::move(user_idx != -1 ? appUserModel()->avatar(user_idx) : "unknown-user")};
	return createChatMessage(userid, std::move(user_name), std::move(user_icon), check_unread_messages);
}

TPMessage *TPMessagesManager::createChatMessage(const QString &userid, QString &&user_name, QString &&icon_source,
																						const bool check_unread_messages)
{
	TPMessage *chat_message{new TPMessage{}};
	chat_message->setTitle(std::forward<QString>(user_name));
	chat_message->setIconSource(std::forward<QString>(icon_source));
	chat_message->setSticky(true);
	chat_message->setId(userid.toLong());
	chat_message->setExtraInfoImage("new-messages");
	chat_message->insertAction(tr("Chat"), [this,userid] (const QVariant &) { openChatWindow(m_chatsList.value(userid)); });
	chat_message->insertAction(tr("Delete"), [this,userid,chat_message] (const QVariant &) {
		m_chatsList.value(userid)->clearChat();
		m_chatsList.remove(userid);
		removeChatWindow(userid);
		chat_message->unplug();
	});
	chat_message->plug();

	TPChat *new_chat{new TPChat{userid, check_unread_messages, this}};
	m_chatsList.insert(userid, new_chat);
	connect(new_chat, &TPChat::interlocutorNameChanged, this, [this,chat_message,new_chat] () {
		chat_message->setText(std::move(new_chat->interlocutorName()));
	});
	connect(new_chat, &TPChat::avatarIconChanged, this, [this,chat_message,new_chat] () {
		chat_message->setIconSource(std::move(new_chat->avatarIcon()));
	});
	connect(new_chat, &TPChat::unreadMessagesChanged, this, [this,chat_message,new_chat] () {
		chat_message->setExtraInfoText(std::move(QString::number(new_chat->unreadMessages())));
	});

	return chat_message;
}

void TPMessagesManager::openChatWindow(TPChat *chat_manager)
{
	QObject *chat_window{m_chatWindowList.value(chat_manager->otherUserId())};
	if (!chat_window) {
		if (!m_chatWindowComponent) {
			m_chatWindowComponent = new QQmlComponent{appQmlEngine(), "TpQml.User"_L1, "ChatWindow"_L1, QQmlComponent::Asynchronous};
			connect(m_chatWindowComponent, &QQmlComponent::statusChanged, this, [this,chat_manager] (QQmlComponent::Status status) {
				openChatWindow(chat_manager);
			});
		}
		switch (m_chatWindowComponent->status()) {
		case QQmlComponent::Ready:
			m_chatWindowComponent->disconnect();
			chat_manager->loadChat();
			createChatWindow_part2(chat_manager);
			openChatWindow(chat_manager);
			break;
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
		appPagesListModel()->openPopup(chat_window, appItemManager()->AppHomePage());
}

void TPMessagesManager::openChat(const uint user_idx)
{
	const QString &userid{appUserModel()->userId(user_idx)};
	const qsizetype i_userid{userid.toLong()};

	if (!message(i_userid))
		createChatMessage(userid, std::move(QString{appUserModel()->userName(user_idx)}),
															std::move(appUserModel()->avatarFromId(userid)), false);
	openChatWindow(m_chatsList.value(userid));
}

void TPMessagesManager::sendFileChatMessage(const TPFilePath &filename, const QString &message)
{
	const qsizetype i_userid{filename.targetUser().toLong()};
	openChat(appUserModel()->findUserById(filename.targetUser()));
	chatManager(filename.targetUser())->createNewMessage(message, filename.toString());
}

void TPMessagesManager::startChatMessagesPolling(const QString &userid)
{
	scanLocalMessages();
	connect(appUserModel(), &DBUserModel::canConnectToServerChanged, this, [this] () {
		if (!appUserModel()->canConnectToServer())
			m_newChatMessagesTimer->stop();
		else
			m_newChatMessagesTimer->start();
	});

	m_newChatMessagesTimer = new QTimer{this};
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
	m_newChatMessagesTimer->callOnTimeout( [this,requestid] () {
		appOnlineServices()->checkMessages(requestid);
		appOnlineServices()->checkTPMessages(requestid);
		m_newChatMessagesTimer->setInterval(newMessagesCheckingInterval());
	});
	m_newChatMessagesTimer->start();
}

QVariant TPMessagesManager::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_data.count()) {
		switch (role) {
		case msgIdRole:				return m_data.at(row)->id();
		case msgTitleRole:			return m_data.at(row)->title();
		case msgTextRole:			return m_data.at(row)->text();
		case msgIconRole:			return m_data.at(row)->iconSource();
		case msgFileNameRole:		return m_data.at(row)->fileName()->toString();
		case msgExtraInfoTextRole:	return m_data.at(row)->extraInfoText();
		case msgExtraInfoIconRole:	return m_data.at(row)->extraInfoImage();
		case msgDateRole:			return m_data.at(row)->date();
		case msgTimeRole:			return m_data.at(row)->time();
		case msgActionsRole:		return m_data.at(row)->actions();
		case msgStickyRole:			return m_data.at(row)->sticky();
		case msgHasActionsRole:		return m_data.at(row)->hasActions();
		}
	}
	return QVariant{};
}

int TPMessagesManager::newMessagesCheckingInterval() const
{
	int msecs{20000};
	int last_sent{0}, last_received{0};
	for (const auto chat : m_chatsList) {
		for (const auto message : std::as_const(chat->m_messages) | std::views::reverse) {
			if (last_sent == 0 && chat->data(message, MESSAGE_SENT).toBool()) {
				if (chat->data(message, MESSAGE_SENDER).toString() == appUserModel()->userId(0)) {
					const QDate &sent_date{chat->data(message, MESSAGE_SDATE).toDate()};
					if (sent_date != QDate::currentDate()) {
						last_sent = -1;
						continue;
					}
					const QTime &sent_time{chat->data(message, MESSAGE_SDATE).toTime()};
					last_sent = QTime::currentTime().msecsSinceStartOfDay() - sent_time.msecsSinceStartOfDay();
				}
			}
			if (last_received == 0 && chat->data(message, MESSAGE_RECEIVED).toBool()) {
				if (chat->data(message, MESSAGE_RECEIVER).toString() != appUserModel()->userId(0)) {
					const QDate &received_date{chat->data(message, MESSAGE_RDATE).toDate()};
					if (received_date != QDate::currentDate()) {
						last_received = -1;
						continue;
					}
					const QTime &received_time{chat->data(message, MESSAGE_RTIME).toTime()};
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

void TPMessagesManager::scanLocalMessages()
{
	TPFilePath tp_filepath{QString{}, appUserModel()->userId(0), QString{}, {tpmessages_subdir}};
	QFileInfoList files;
	appUtils()->scanDir(tp_filepath.toString(), files, QString{}, true);
	for (const auto &file : std::as_const(files)) {
		tp_filepath.setTargetUser(userIdFromExchangeFile(file.filePath()));
		tp_filepath.setFileName(file.fileName(), true);
		if (appUtils()->getFileExtension(file.fileName(), true) == tptextmessage_extension)
			parseTextMessage(tp_filepath);
		else {
			QByteArray data{appUtils()->readBinaryFile(tp_filepath.toString())};
			const QString &data_meta_info{appUtils()->getBinaryFileMetaInfo(data)};
			binaryFileReceived(data, data_meta_info);
		}
	}
}

void TPMessagesManager::parseTextMessage(const TPFilePath &filename)
{
	QFile *exchange_file{appUtils()->openFile(filename.toString())};
	if (exchange_file) {
		textMesssageReceived(exchange_file->readAll(), filename);
		exchange_file->close();
		delete exchange_file;
	}
}

void TPMessagesManager::receivedTPMessages(const QStringList &files)
{
	TPFilePath local_file;
	local_file.setOwnerUser(appUserModel()->userId(0));
	for (const auto &file : files) {
		local_file.setSubDirsPlusFilename(file);
		if (!QFile::exists(local_file.toString())) {
			auto file_ops{std::make_shared<TPFileOps>()};
			file_ops->setCanDownloadOrGenerate(true);
			file_ops->setFileName(local_file.toString());
			connect(&(*file_ops), &TPFileOps::fileAcquired, this, [this,file_ops] (const int ret_code) {
				if (ret_code == TP_RET_CODE_SUCCESS)
					file_ops->removeFile(true, false, true);
			});
			file_ops->doFileOperation(TPFileOps::OT_Download);
		}
	}
}

/*	record_separator(oct 036, dec 30) separates the message fields
	set_separator (oct 037, dec 31) separates messages of the same sender
	exercises_separator (oct 034 dec 28) separates the senders (the even number are the messages content and the odd numbers are the sender ids)
*/
void TPMessagesManager::parseNewChatMessages(const QString &encoded_messages)
{
	uint sender_idx{0};
	do {
		QString sender_id{std::move(appUtils()->getCompositeValue(sender_idx + 1, encoded_messages, exercises_separator))};
		if (sender_id.isEmpty())
			break;
		QString sender_messages{std::move(appUtils()->getCompositeValue(sender_idx, encoded_messages, exercises_separator))};
		if (sender_messages.isEmpty())
			continue;

		const QStringList &sender_parts{sender_id.split('.')};
		bool ok{false};
		const auto i_sender_id{sender_parts.at(0).toLong(&ok)};
		if (ok) {
			TPMessage *chat_message{message(i_sender_id)};
			if (!chat_message && sender_id.endsWith(messageWorkSend))
				static_cast<void>(createChatMessage(sender_id, true));
			TPChat *chat_mngr{chatManager(sender_parts.at(0))};
			chat_mngr->processTPServerMessage('.' % sender_parts.at(1), sender_messages);
		}
	} while (sender_idx += 2);
}

void TPMessagesManager::createChatWindow_part2(TPChat *chat_manager)
{
	m_chatWindowProperties["chatManager"_L1] = std::move(QVariant::fromValue(chat_manager));
	QObject *chat_window{m_chatWindowComponent->createWithInitialProperties(m_chatWindowProperties, appQmlEngine()->rootContext())};
#ifndef QT_NO_DEBUG
	if (!chat_window) {
		qDebug() << m_chatWindowComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(chat_window, QQmlEngine::CppOwnership);
	chat_window->setProperty("parent", QVariant::fromValue(appItemManager()->AppHomePage()));
	chat_manager->setChatWindow(chat_window);
	m_chatWindowList.insert(chat_manager->otherUserId(), chat_window);
}

void TPMessagesManager::removeChatWindow(const QString &other_userid)
{
	QObject *chat_window{m_chatWindowList.value(other_userid)};
	if (chat_window) {
		delete chat_window;
		m_chatWindowList.remove(other_userid);
	}
}
