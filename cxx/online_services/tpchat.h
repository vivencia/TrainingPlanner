#pragma once

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
constexpr uint TP_CHAT_MESSAGE_FIELDS	{MESSAGE_MEDIA+1};

QT_FORWARD_DECLARE_CLASS(TPChatDB)
QT_FORWARD_DECLARE_STRUCT(ChatMessage)

class TPChat : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged FINAL)
Q_PROPERTY(QString interlocutorName READ interlocutorName NOTIFY interlocutorNameChanged FINAL)
Q_PROPERTY(QString avatarIcon READ avatarIcon NOTIFY avatarIconChanged FINAL)

public:
	static constexpr QLatin1StringView chatsSubDir{"chats/"};

	explicit TPChat(const QString &otheruser_id, QObject *parent = nullptr);
	~TPChat();

	void loadChat();
	inline void setChatWindow(QObject *chat_window) { m_chatWindow = chat_window; }
	inline QObject *chatWindow() const { return m_chatWindow; }
	Q_INVOKABLE inline uint count() const { return m_messages.count(); }
	inline const QString &otherUserId() const { return m_otherUserId; }

	QString interlocutorName() const;
	QString avatarIcon() const;
	inline uint userIdx() const { return m_userIdx; }

	void setSentMessageReceived(const uint msgid);
	void setSentMessageRead(const uint msgid);
	Q_INVOKABLE void removeMessage(const uint msgid);
	inline uint unreadMessages() const { return m_unreadMessages; }
	void setUnreadMessages(const int n_unread);
	void markAllIncomingMessagesRead();
	Q_INVOKABLE void newMessage(const QString &text, const QString &media = QString{});
	void incomingMessage(const QString &encoded_message);
	void clearChat();

	QVariant data(ChatMessage* message, const uint field) const;

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();
	void interlocutorNameChanged();
	void avatarIconChanged();
	void unreadMessagesChanged(const uint n_unread_messages);

private:
	QString m_otherUserId;
	uint m_userIdx, m_unreadMessages;
	QList<ChatMessage*> m_messages;
	QHash<int, QByteArray> m_roleNames;
	TPChatDB *m_chatDB;
	QObject *m_chatWindow;

	QString tempMessagesFile() const;
	QString encodeMessageToUpload(ChatMessage* message) const;
	ChatMessage* decodeDownloadedMessage(const QString &encoded_message);
	void saveNewMessageIntoDB(ChatMessage *message);
	void saveMessageToFile(ChatMessage *message) const;

	friend class TPMessagesManager;
};

