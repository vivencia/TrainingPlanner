#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPMessage)
QT_FORWARD_DECLARE_CLASS(QTimer)

class TPMessagesManager : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged FINAL)

public:
	explicit TPMessagesManager(QObject *parent = nullptr);
	~TPMessagesManager();

	inline uint count() const { return m_data.count(); }
	void readAllChats();

	TPMessage *message(const qsizetype message_id) const;
	/**
	 * @brief Add a message to be displayed to the user based on online data received
	 * @return message_id
	 * @see removeMessage
	 */
	void addMessage(TPMessage *msg);
	Q_INVOKABLE inline void removeMessage(const qsizetype message_id) { removeMessage(message(message_id)); }
	void removeMessage(TPMessage *msg);
	Q_INVOKABLE void execAction(const int message_index, const uint action_id);
	Q_INVOKABLE void itemClicked(const qsizetype message_id);

	TPMessage *createChatMessage(const QString &userid, const bool check_unread_messages);
	/**
	 * @brief Creates a chat entry in the messages window. Therefore, the message created will be added to the messages list
	 * @param display_text should reflect the user name
	 * @param icon_source should use the user's avatar
	 * @return Returns the newly created message
	 */
	TPMessage *createChatMessage(const QString &userid, QString &&display_text, QString &&icon_source, const bool check_unread_messages);
	void openChatWindow(TPChat *chat_manager);
	inline TPChat *chatManager(const QString &userid) const { return m_chatsList.value(userid); }
	Q_INVOKABLE void openChat(const QString &username);
	void startChatMessagesPolling(const QString &userid);

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();

private:
	QList<TPMessage*> m_data;
	QHash<int, QByteArray> m_roleNames;
	QHash<QString,TPChat*> m_chatsList;
	QHash<QString,QObject*> m_chatWindowList;
	QTimer *m_newChatMessagesTimer;
	QQmlComponent *m_chatWindowComponent;
	QVariantMap m_chatWindowProperties;

	int newMessagesCheckingInterval() const;
	void parseNewChatMessages(const QString &encoded_messages);
	void parseNewMessage(const QString &sender_id, const QString &sender_messages);
	void parseReceivedActionMessage(const QString &sender_id, const QString &action_message);
	void parseReadActionMessage(const QString &sender_id, const QString &action_message);
	void parseRemovedActionMessage(const QString &sender_id, const QString &action_message);
	void createChatWindow_part2(TPChat *chat_manager);
	void removeChatWindow(const QString &other_userid);

	static TPMessagesManager *_appMessagesManager;
	friend TPMessagesManager *appMessagesManager();
};

inline TPMessagesManager *appMessagesManager() { return TPMessagesManager::_appMessagesManager; }
