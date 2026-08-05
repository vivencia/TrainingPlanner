#pragma once

#include "tpmessagesmodel.h"
#include "../qml_singleton.h"

#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPFilePath)
QT_FORWARD_DECLARE_CLASS(TPMessage)
QT_FORWARD_DECLARE_CLASS(QTimer)

class TPMessagesManager : public QObject
{

Q_OBJECT
QML_UNCREATABLE("Created only once via c++")

Q_PROPERTY(TPMessagesModel* messagesModel READ messagesModel NOTIFY messagesModelChanged FINAL)

public:
	Q_DISABLE_COPY_MOVE(TPMessagesManager)
	static constexpr QLatin1StringView tpmessages_subdir{"exchange_files/"};
	static constexpr QLatin1StringView tpsystem_userid{"TPApp"};
	static constexpr QLatin1StringView tp_toplevel_message{"topLevel"};

	explicit TPMessagesManager(QObject *parent = nullptr);
	TPMessagesModel *messagesModel() const { return m_messagesModel; }

	void startMessagesPolling(const QString &userid);
	void newTextMessage(const QString &encoded_message);
	void sendTPMessage(const QString &target_user, const QString &encoded_message, const int request_id = -1);
	void readAllChats();
	void openChatWindow(TPChat *chat_manager);
	inline TPChat *chatManager(const QString &userid) const { return m_chatsList.value(userid)->chat; }
	Q_INVOKABLE void openChat(const uint user_idx);
	Q_INVOKABLE void openNewMessageDialog(const uint user_idx);

signals:
	void messagesModelChanged();
	void TPMessageSent(const int requestid, const bool success);

private:
	struct st_Chat {
		TPChat *chat{nullptr};
		QObject *dialog{nullptr};
	};
	QHash<QString,st_Chat*> m_chatsList;

	QTimer *m_checkMessagesTimer{nullptr};
	QQmlComponent *m_chatWindowComponent{nullptr};
	QVariantMap m_chatWindowProperties;
	TPMessagesModel *m_messagesModel{nullptr};

	TPMessage *topLevelUserMessage(const QString &userid);
	void receivedTPMessages(const QStringList &messages);
	void parseNewChatMessages(const QString &encoded_messages);
	TPChat *createChatMessage(QString &&userid, const bool check_unread_messages);
	void removeChildrenMessages(TPMessage *msg, const QLatin1StringView &exclude_type);
	void removeMessage(TPMessage *msg);
	int newMessagesCheckingInterval() const;

	static TPMessagesManager *_appMessagesManager;
	friend TPMessagesManager *appMessagesManager();
};

DECLARE_QML_NAMED_SINGLETON(TPMessagesManager, AppMessages)

inline TPMessagesManager *appMessagesManager() { return TPMessagesManager::_appMessagesManager; }
