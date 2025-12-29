#pragma once

#include "../dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

constexpr uint MESSAGE_ID				{0};
constexpr uint MESSAGE_SENDER			{1};
constexpr uint MESSAGE_RECEIVER			{2};
constexpr uint MESSAGE_SDATE			{3};
constexpr uint MESSAGE_STIME			{4};
constexpr uint MESSAGE_RDATE			{5};
constexpr uint MESSAGE_RTIME			{6};
constexpr uint MESSAGE_DELETED			{7};
constexpr uint MESSAGE_SENT				{8};
constexpr uint MESSAGE_RECEIVED			{9};
constexpr uint MESSAGE_READ				{10};
constexpr uint MESSAGE_TEXT				{11};
constexpr uint MESSAGE_MEDIA			{12};
constexpr uint MESSAGE_QUEUED			{13};
constexpr uint TP_CHAT_MESSAGE_FIELDS	{MESSAGE_QUEUED + 1};

constexpr QLatin1StringView messageWorkSend {".msg"};
constexpr QLatin1StringView messageWorkReceived {".received"};
constexpr QLatin1StringView messageWorkRead {".read"};
constexpr QLatin1StringView messageWorkRemoved {".removed"};
constexpr QLatin1StringView messageWorkEdited {".edited"};

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceChat)
QT_FORWARD_DECLARE_STRUCT(ChatMessage)
QT_FORWARD_DECLARE_CLASS(TPChatDB)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class TPChat : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged FINAL)
Q_PROPERTY(QString interlocutorName READ interlocutorName NOTIFY interlocutorNameChanged FINAL)
Q_PROPERTY(QString avatarIcon READ avatarIcon NOTIFY avatarIconChanged FINAL)
Q_PROPERTY(bool hasUnreadMessages READ hasUnreadMessages WRITE setHasUnreadMessages NOTIFY unreadMessagesChanged FINAL)

public:
	static constexpr QLatin1StringView chatsSubDir{"chats/"};

	explicit TPChat(const QString &otheruser_id, const bool check_unread_messages, QObject *parent = nullptr);

	void setWSPeer(QWebSocket *peer);

	void loadChat();
	inline void setChatWindow(QObject *chat_window) { m_chatWindow = chat_window; }
	inline QObject *chatWindow() const { return m_chatWindow; }
	Q_INVOKABLE inline uint count() const { return m_messages.count(); }
	inline const QString &otherUserId() const { return m_otherUserId; }

	QString interlocutorName() const;
	QString avatarIcon() const;
	inline uint userIdx() const { return m_userIdx; }

	void processTPServerMessage(const QString &work, const QString &messages);
	Q_INVOKABLE void removeMessage(const uint msgid, const bool remove_for_interlocutor);
	void editMessage(const QString &encoded_data);
	inline uint unreadMessages() const { return m_unreadMessages; }
	void setUnreadMessages(const int n_unread);
	inline bool hasUnreadMessages() const { return m_unreadMessages > 0; }
	inline void setHasUnreadMessages(const bool has_unread) { if (!has_unread) markAllIncomingMessagesRead(); }
	Q_INVOKABLE void markAllIncomingMessagesRead();
	Q_INVOKABLE void createNewMessage(const QString &text, const QString &media = QString{});
	void incomingMessage(const QString &encoded_message);
	void clearChat();
	QVariant data(const ChatMessage *const message, const uint field, const bool format_output = false) const;

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

public slots:
	void processWebSocketMessage(const QString &message);
	Q_INVOKABLE void onChatWindowOpened();

signals:
	void countChanged();
	void interlocutorNameChanged();
	void avatarIconChanged();
	void unreadMessagesChanged();
	void initWSConnection(const QString &id, const QString &address);
	void messageReceived();

private:
	QString m_otherUserId;
	uint m_userIdx, m_unreadMessages;
	QList<ChatMessage*> m_messages;
	QHash<int, QByteArray> m_roleNames;
	QObject *m_chatWindow;
	DBModelInterfaceChat *m_dbModelInterface;
	TPChatDB *m_db;
	QWebSocket *m_peerSocket;
	QTimer *m_sendMessageTimer;
	bool m_chatLoaded;
	QHash<QString,std::function<void(const QString&)>> m_workFuncs;

	short connectionType() const;
	void unqueueMessage(ChatMessage *const message);
	void uploadAction(const uint field, ChatMessage *const message);
	void acknowledgeMessageWorked(const uint msgid, const QLatin1StringView &work);
	QString encodeMessageToUpload(const ChatMessage *const message) const;
	void encodeMessageToSave(const ChatMessage *const message);
	void updateFieldToSave(const uint msg_id, const int field, const QString &value) const;
	ChatMessage* decodeDownloadedMessage(const QString &encoded_message);

	friend class TPMessagesManager;
};

class DBModelInterfaceChat : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceChat(TPChat *model) : DBModelInterface{model} {}
	inline const QList<QStringList> &modelData() const { return m_modelData; }
	inline QList<QStringList> &modelData() { return m_modelData; }

private:
	QList<QStringList> m_modelData;
};
