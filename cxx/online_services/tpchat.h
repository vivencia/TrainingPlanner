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

public:
	explicit TPChat(const QString &otheruser_id, QObject *parent = nullptr);

	Q_INVOKABLE inline uint count() const { return m_messages.count(); }
	inline const QString &otherUserId() const { return m_otherUserId; }

	Q_INVOKABLE void newMessage(const QString &text, const QString &media = QString{});
	void incomingMessage(const QString &encoded_message);
	Q_INVOKABLE void messageRead(const uint msgid);
	Q_INVOKABLE void removeMessage(const uint msgid);
	void clearChat();

	QVariant data(ChatMessage* message, const uint field) const;

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();

private:
	QString m_otherUserId;
	QList<ChatMessage*> m_messages;
	QHash<int, QByteArray> m_roleNames;
	TPChatDB *m_chatDB;

	QString encodeMessageToUpload(ChatMessage* message);
	ChatMessage* decodeDownloadedMessage(const QString &encoded_message);
	void saveChat(ChatMessage *message);

	friend class TPMessagesManager;
};

