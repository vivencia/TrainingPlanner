#pragma once

#include "../dbmodelinterface.h"
#include "../tpbool.h"

#include <QAbstractListModel>
#include <QQmlEngine>

enum ChatMessageFields {
	MESSAGE_ID,
	MESSAGE_SENDER,
	MESSAGE_RECEIVER,
	MESSAGE_SDATE,
	MESSAGE_STIME,
	MESSAGE_RDATE,
	MESSAGE_RTIME,
	MESSAGE_DELETED,
	MESSAGE_SENT,
	MESSAGE_RECEIVED,
	MESSAGE_READ,
	MESSAGE_TEXT,
	MESSAGE_MEDIA,
	MESSAGE_QUEUED,
	TP_CHAT_TOTAL_MESSAGE_FIELDS
};

constexpr QLatin1StringView messageWorkSend{".0msg"};
constexpr QLatin1StringView messageWorkReceived{".1received"};
constexpr QLatin1StringView messageWorkRead{".2read"};
constexpr QLatin1StringView messageWorkRemoved{".3removed"};
constexpr QLatin1StringView messageWorkEdited{".4edited"};

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
	inline uint unreadMessages() const { return m_unreadIds.count(); }
	inline bool hasUnreadMessages() const { return unreadMessages() > 0; }
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
	void processWebSocketTextMessage(const QString &message);
	void processWebSocketBinaryMessage(const QByteArray &data);
	Q_INVOKABLE void onChatWindowOpened();

signals:
	void countChanged();
	void interlocutorNameChanged();
	void avatarIconChanged();
	void unreadMessagesChanged();
	void initWSConnection(const QString &id, const QString &address);
	void messageReceived();
	void chatLoadedStatusChanged();

private:
	QString m_otherUserId;
	uint m_userIdx;
	QList<ChatMessage*> m_messages;
	QHash<int, QByteArray> m_roleNames;
	QObject *m_chatWindow;
	DBModelInterfaceChat *m_dbModelInterface;
	TPChatDB *m_db;
	QWebSocket *m_peerSocket;
	QTimer *m_sendMessageTimer;
	uint8_t m_chatLoaded;
	QHash<QString,std::function<void(const QString&)>> m_workFuncs;
	QStringList m_unreadIds;
	TPBool m_messageWorksQueued;

	void setChatLoadedStatus(uint8_t status);
	short connectionType() const;
	void unqueueMessage(ChatMessage *const message);
	void uploadAction(const uint field, ChatMessage *const message);
	void acknowledgeMessageWorked(const uint msgid, const QLatin1StringView &work);
	QString encodeMessageToUpload(const ChatMessage *const message) const;
	void encodeMessageToSave(const ChatMessage *const message);
	void updateFieldToSave(const uint msg_id, const int field, const QString &value) const;
	ChatMessage* decodeDownloadedMessage(const QString &encoded_message);
	void getNewMessagesNumber(const QString &encoded_messages);
	void setUnreadMessages(const QString &unread_ids, const bool add = true);
	QString chatsMediaSubDir(const bool fullpath) const;
	void getMediaPreviewFile(const ChatMessage *const message);
	void getImagePreviewFile(const ChatMessage *const message);

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
