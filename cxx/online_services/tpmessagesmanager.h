#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPMessage)

class TPMessagesManager : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged FINAL)

public:
	explicit TPMessagesManager(QObject *parent = nullptr);

	inline uint count() const { return m_data.count(); }

	TPMessage *message(const int message_id) const;
	Q_INVOKABLE inline TPMessage *messageEntry(const int index) const { return index >= 0 && index < m_data.count() ? m_data.at(index) : nullptr; }

	/**
	 * @brief Add a message to be displayed to the user based on online data received
	 * @return message_id
	 * @see removeMessage
	 */
	std::optional<int> addMessage(TPMessage *msg);
	inline void removeMessage(const int message_id) { removeMessage(message(message_id)); }
	Q_INVOKABLE void removeMessage(TPMessage *msg);

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

	static TPMessagesManager *_appMessagesManager;
	friend TPMessagesManager *appMessagesManager();
};

inline TPMessagesManager *appMessagesManager() { return TPMessagesManager::_appMessagesManager; }
