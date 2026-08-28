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

	explicit TPMessagesManager(QObject *parent = nullptr);
	TPMessagesModel *messagesModel() const { return m_messagesModel; }
	inline QObject *messagesManagerDialog() const { return m_messagesManagerDialog; }

	void startMessagesPolling(const QString &userid);
	void newTextMessage(QString &&encoded_message);
	void sendTPMessage(const QString &target_user, const QString &encoded_message, const int request_id = -1);
	void readAllChats();
	void openChatWindow(TPChat *chat_manager);
	inline TPChat *chatManager(const QString &userid) const { return m_chatsList.value(userid)->chat; }
	Q_INVOKABLE void openChat(const uint user_idx);
	Q_INVOKABLE void openNewMessageDialog(const uint user_idx);
	Q_INVOKABLE void showOnlineMessagesManagerDialog(const bool show);
	void startMessagesManager();

public slots:
	void sendTPMessage(const QStringList &users, const QString &message, const QString &filename);

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
	QQmlComponent *m_chatWindowComponent{nullptr}, *m_messagesManagerComponent{nullptr}, *m_newTPMessageComponent{nullptr};
	QObject *m_messagesManagerDialog{nullptr}, *m_newTPMessageDialog{nullptr};
	QVariantMap m_chatWindowProperties;
	TPMessagesModel *m_messagesModel{nullptr};

	TPMessage *topLevelUserMessage(const QString &userid);
	void receivedTPMessages(const QStringList &messages);
	void createGeneralMessagesPopup();
	void parseNewChatMessages(const QString &encoded_messages);
	TPChat *createChatMessage(QString &&userid, const bool check_unread_messages);
	void removeChildrenMessages(TPMessage *msg, const int exclude_type);
	void removeMessage(TPMessage *msg);
	int newMessagesCheckingInterval() const;
	void setTotalNewMessages(TPMessage *top_level_msg, const int key, const int new_messages);

	static TPMessagesManager *_appMessagesManager;
	friend TPMessagesManager *appMessagesManager();
};

DECLARE_QML_NAMED_SINGLETON(TPMessagesManager, AppMessages)

inline TPMessagesManager *appMessagesManager() { return TPMessagesManager::_appMessagesManager; }
