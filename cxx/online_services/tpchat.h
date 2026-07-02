#pragma once

#include "../dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceChat)
QT_FORWARD_DECLARE_STRUCT(ChatMessage)
QT_FORWARD_DECLARE_CLASS(TPChatDB)
QT_FORWARD_DECLARE_CLASS(QTimer)

class TPChat : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(ChatModel)
QML_UNCREATABLE("")

Q_PROPERTY(uint count READ count NOTIFY countChanged FINAL)
Q_PROPERTY(QString interlocutorName READ interlocutorName NOTIFY interlocutorNameChanged FINAL)
Q_PROPERTY(QString avatarIcon READ avatarIcon NOTIFY avatarIconChanged FINAL)
Q_PROPERTY(bool hasUnreadMessages READ hasUnreadMessages WRITE setHasUnreadMessages NOTIFY unreadMessagesChanged FINAL)

public:
	enum ChatMessageFields {
		ID,
		SENDER,
		RECEIVER,
		SDATE,
		STIME,
		RDATE,
		RTIME,
		DELETED,
		SENT,
		RECEIVED,
		READ,
		TEXT,
		MEDIA,
		QUEUED,
		TPCHAT_N_FIELDS,
		OWN_MESSAGE,
		MEDIA_VIEWER,
	};

	explicit TPChat(const QString &otheruser_id, const bool check_unread_messages, QObject *parent = nullptr);
	inline QString chatSubDir() const { return m_otherUserId % Qt::StringLiterals::operator""_L1("/chat/", 6); }

	const QString &userId() const;
	inline const QString &otherUserId() const { return m_otherUserId; }
	void loadChat();
	inline void setChatWindow(QObject *chat_window) { m_chatWindow = chat_window; }
	inline QObject *chatWindow() const { return m_chatWindow; }
	Q_INVOKABLE inline uint count() const { return m_messages.count(); }

	QString interlocutorName() const;
	QString avatarIcon() const;
	inline uint userIdx() const { return m_userIdx; }

	Q_INVOKABLE void removeMessage(const uint msgid, const bool remove_for_interlocutor);
	void editMessage(const QString &work_data);
	inline uint unreadMessages() const { return m_unreadIds.count(); }
	inline bool hasUnreadMessages() const { return unreadMessages() > 0; }
	inline void setHasUnreadMessages(const bool has_unread) { if (!has_unread) markAllIncomingMessagesRead(); }
	Q_INVOKABLE void markAllIncomingMessagesRead();
	Q_INVOKABLE void createNewMessage(const QString &text, const bool attach_file);
	void incomingMessage(const QString &encoded_message);
	void clearChat();
	QVariant data(const ChatMessage *const message, const uint field, const bool format_output = false) const;
	Q_INVOKABLE inline int nMediaMessages() const { return m_nMedia; }

	bool canUseWebSocket() const;
	bool canUseServer() const;

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

public slots:
	void processChatMessage(const QString &encoded_message);
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
	uint m_userIdx, m_nMedia;
	QList<ChatMessage*> m_messages;
	QList<std::function<void(const QString&)>> m_workFuncs;
	QHash<int, QByteArray> m_roleNames;
	QObject *m_chatWindow{nullptr};
	DBModelInterfaceChat *m_dbModelInterface{nullptr};
	TPChatDB *m_db{nullptr};
	QTimer *m_sendMessageTimer{nullptr};
	uint8_t m_chatLoaded;
	QStringList m_unreadIds;
	bool m_messageWorksQueued{false};

	void setChatLoadedStatus(uint8_t status);
	short checkConnectionOptions() const;
	void unqueueMessage(ChatMessage *const message);
	void doChatWork(const uint work, ChatMessage *const message, const int field = -1);
	QString encodeMessageToUpload(const ChatMessage *const message) const;
	QString encodeWorkMessage(const ChatMessage *const message, const int work, const int field = -1) const;
	void encodeMessageToSave(const ChatMessage *const message);
	void updateFieldToSave(const uint msg_id, const int field, const QString &value) const;
	ChatMessage* decodeDownloadedMessage(const QString &encoded_message);
	void getNewMessagesNumber(const QString &encoded_messages);
	void setUnreadMessages(const QString &unread_ids, const bool add = true);
	void createMediaViewer(ChatMessage *message, const bool add_file);
	void sendMessage(ChatMessage *message);

	friend class TPMessagesManager;
	friend class TPChatDB;
	Q_DISABLE_COPY(TPChat)
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
