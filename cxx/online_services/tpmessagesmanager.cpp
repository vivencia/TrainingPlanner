#include "tpmessagesmanager.h"

#include "tpmessage.h"
#include "../tputils.h"

TPMessagesManager *TPMessagesManager::_appMessagesManager{nullptr};

enum RoleNames {
	idRole = Qt::UserRole,
	labelTextRole = Qt::UserRole + 1,
	iconRole = Qt::UserRole + 2,
	actionsRole = Qt::UserRole + 3,
	extraDataRole = Qt::UserRole + 4
};

TPMessagesManager::TPMessagesManager(QObject *parent)
	: QAbstractListModel{parent}
{
	_appMessagesManager = this;
	m_roleNames[idRole] = std::move("id");
	m_roleNames[labelTextRole] = std::move("labelText");
	m_roleNames[iconRole] = std::move("icon");
	m_roleNames[actionsRole] = std::move("actions");
	m_roleNames[extraDataRole] = std::move("extraData");
}

TPMessage *TPMessagesManager::message(const int message_id) const
{
	const auto &it = std::find_if(m_data.cbegin(), m_data.cend(), [message_id] (const auto msg) {
		return msg->id() == message_id;
	});
	return it != m_data.cend() ? *it : nullptr;
}

std::optional<int> TPMessagesManager::addMessage(TPMessage *msg)
{
	if (msg && !msg->plugged())
	{
		beginInsertRows(QModelIndex{}, count(), count());
		const QLatin1StringView v{std::move(msg->_displayText().toLatin1())};
		if (msg->id() == -1) //do not override an id set elsewhere
			msg->setId(appUtils()->generateUniqueId(v));
		m_data.append(msg);
		endInsertRows();
		emit countChanged();
		msg->setPlugged(true);
		connect(msg, &TPMessage::actionTriggered, this, [this,msg] (const int action_id, const std::optional<bool> remove_message) {
			emit actionTriggered(msg->id(), action_id);
			if (remove_message.has_value())
			{
				if (remove_message.value())
					removeMessage(msg);
			}
			else
				if (!msg->sticky())
					removeMessage(msg);
		});
		return msg->id();
	}
	return std::nullopt;
}

void TPMessagesManager::removeMessage(TPMessage *msg)
{
	if (msg != nullptr)
	{
		const qsizetype row{m_data.indexOf(msg)};
		if (row >= 0)
		{
			beginRemoveRows(QModelIndex{}, row, row);
			m_data.remove(row);
			if (msg->autoDelete())
				delete msg;
			emit countChanged();
			endRemoveRows();
		}
	}
}

QVariant TPMessagesManager::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_data.count())
	{
		switch (role)
		{
			case idRole: return m_data.at(row)->id();
			case labelTextRole: return m_data.at(row)->displayText();
			case iconRole: return m_data.at(row)->iconSource();
			case actionsRole: return m_data.at(row)->actions();
		}
	}
	return QVariant{};
}
