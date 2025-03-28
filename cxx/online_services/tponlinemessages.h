#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_STRUCT(messageData)
QT_FORWARD_DECLARE_CLASS(TPImage)

class TPOnlineMessages : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:
	explicit TPOnlineMessages(QObject *parent = nullptr);

	inline uint count() const { return m_data.count(); }

	/**
	 * @brief Add a message to be displayed to the user based on online data received
	 * @param label What will be displayed to the user
	 * @param image An icon to the message
	 * @param auto_remove Whether to remove the message after some action is triggered
	 * @return message_id
	 * @see removeMessage
	 */
	std::optional<int> addMessage(const QString &label, const QString &image, const bool auto_remove = true);
	inline void removeMessage(const int message_id);
	void removeMessage(messageData *msg);

	/**
	 * @param message_id
	 * @param action_name
	 * @param remove if Specified, overrides autoremove
	 * @return action_id
	 */
	std::optional<int> insertAction(const int message_id, const QString& action_name, std::optional<bool> remove = std::nullopt);
	Q_INVOKABLE void execAction(const int message_id, const int action_id);

	/**
	 * @brief Used to store information across classes
	 * @param message_id
	 * @param data Anything that QVariant can handle
	 * @return data_id
	 */
	std::optional<int> insertData(const int message_id, const QVariant &data);
	void removeData(const int message_id, const int data_id);
	const QVariant &getData(const int message_id, const int data_id) const;

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void actionTriggered(const int message_id, const int action_id);

private:
	QList<messageData*> m_data;
	QHash<int, QByteArray> m_roleNames;

	inline messageData *findMessage(const int message_id) const;
};

