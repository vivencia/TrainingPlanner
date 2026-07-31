#pragma once

#include "qml_singleton.h"

#include <QAbstractItemModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPFilePath)
QT_FORWARD_DECLARE_CLASS(TPMessage)
QT_FORWARD_DECLARE_CLASS(QTimer)

class TPMessagesManager : public QAbstractItemModel
{

Q_OBJECT
QML_UNCREATABLE("Created only once via c++")

public:
	Q_DISABLE_COPY_MOVE(TPMessagesManager)
	static constexpr QLatin1StringView tpmessages_subdir{"exchange_files/"};
	static constexpr QLatin1StringView tpsystem_userid{"TPApp"};

	explicit TPMessagesManager(QObject *parent = nullptr);

	TPMessage *topLevelMessage(const QString &user_id) const;
	TPMessage *createTopLevelMessage(const QString &userid);
	TPMessage *message(const TPMessage *const parent_message, const uint id) const;
	void addMessage(TPMessage *msg);
	void removeMessage(TPMessage *msg);

	Q_INVOKABLE void execAction(const QString &user_id, const int index, const int action_id, const QVariant &data);
	Q_INVOKABLE void enableAction(const QString &user_id, const int index, const int action_id, const bool enable);
	void sendTPMessage(const QString &target_user, const QString &encoded_message, const int request_id = -1);
	void textMessageReceived(const QString &encoded_message);
	void openNewMessageDialog(const QString &userid);

	void readAllChats();
	TPChat *createChatMessage(QString &&userid, const bool check_unread_messages);
	void openChatWindow(TPChat *chat_manager);
	inline TPChat *chatManager(const QString &userid) const { return m_chatsList.value(userid)->chat; }
	Q_INVOKABLE void openChat(const uint user_idx);
	void startMessagesPolling(const QString &userid);

	QVariant data(const QModelIndex &index, int role) const override;
	inline bool setData(const QModelIndex &index, const QVariant &value, int role) override final { return false; }
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = {}) const override;
	inline int columnCount(const QModelIndex &parent = {}) const override { return 0; }
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void TPMessageSent(const int requestid, const bool success);

private:
	struct st_Chat {
		TPChat *chat{nullptr};
		QObject *dialog{nullptr};
	};
	QHash<QString,st_Chat*> m_chatsList;
	QHash<int, QByteArray> m_roleNames;
	std::unique_ptr<TPMessage> m_rootMessage;
	QTimer *m_checkMessagesTimer{nullptr};
	QQmlComponent *m_chatWindowComponent{nullptr};
	QVariantMap m_chatWindowProperties;

	int newMessagesCheckingInterval() const;
	void clearTopLevelMessage(TPMessage *tlm, const bool clear_chat);
	void receivedTPMessages(const QStringList &messages);
	void binaryFileReceived(const QString &filename, QString &&text_message);
	void parseNewChatMessages(const QString &encoded_messages);

	static TPMessagesManager *_appMessagesManager;
	friend TPMessagesManager *appMessagesManager();
};

DECLARE_QML_NAMED_SINGLETON(TPMessagesManager, AppMessages)

inline TPMessagesManager *appMessagesManager() { return TPMessagesManager::_appMessagesManager; }
