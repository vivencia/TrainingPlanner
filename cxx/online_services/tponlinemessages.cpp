#include "tponlinemessages.h"

#include "../tpglobals.h"
#include "../tputils.h"

struct messageData
{
	int id;
	bool autoremove;
	QString label;
	QString icon;
	QStringList actions;
	QVariantList data;

	inline messageData(): id{-1}, autoremove{true} {}
};

enum RoleNames {
	idRole = Qt::UserRole,
	labelTextRole = Qt::UserRole + 1,
	iconRole = Qt::UserRole + 2,
	actionsRole = Qt::UserRole + 3,
	extraDataRole = Qt::UserRole + 4
};

static const QVariant emptyData{};

TPOnlineMessages::TPOnlineMessages(QObject *parent)
	: QAbstractListModel{parent}
{
	m_roleNames[idRole] = std::move("id");
	m_roleNames[labelTextRole] = std::move("labelText");
	m_roleNames[iconRole] = std::move("icon");
	m_roleNames[actionsRole] = std::move("actions");
	m_roleNames[extraDataRole] = std::move("extraData");
}

std::optional<int> TPOnlineMessages::addMessage(const QString &label, const QString &image, const bool auto_remove)
{
	beginInsertRows(QModelIndex{}, count(), count());
	messageData *msg{new messageData{}};
	QLatin1StringView v{std::move(label.toLatin1())};
	msg->id = appUtils()->generateUniqueId(v);
	msg->autoremove = auto_remove;
	msg->label = label;
	msg->icon = image;
	m_data.append(msg);
	endInsertRows();
	return msg->id;
}

void TPOnlineMessages::removeMessage(const int message_id)
{
	messageData *msg{findMessage(message_id)};
	removeMessage(msg);
}

void TPOnlineMessages::removeMessage(messageData *msg)
{
	if (msg != nullptr)
	{
		const qsizetype row{m_data.indexOf(msg)};
		if (row >= 0)
		{
			beginRemoveRows(QModelIndex{}, row, row);
			m_data.remove(row);
			delete msg;
			emit countChanged();
			endRemoveRows();
		}
	}
}

std::optional<int> TPOnlineMessages::insertAction(const int message_id, const QString& action_name, std::optional<bool> remove)
{
	messageData *msg{findMessage(message_id)};
	if (msg != nullptr)
	{
		msg->actions.append(action_name);
		if (remove && remove.has_value())
		{
			msg->actions.last().append(remove.value() ? record_separator : set_separator);
			msg->autoremove = remove.value();
		}
		return msg->actions.count() - 1;
	}
	return std::nullopt;
}

void TPOnlineMessages::execAction(const int message_id, const int action_id)
{
	messageData *msg{findMessage(message_id)};
	if (msg != nullptr)
	{
		if (action_id >= 0 && action_id < msg->actions.count())
		{
			const QChar &last_char{msg->actions.at(action_id).last(1).at(0)};
			if (last_char == record_separator)
				removeMessage(msg);
			else if (last_char != set_separator)
			{
				if (msg->autoremove)
					removeMessage(msg);
			}
			emit actionTriggered(message_id, action_id);
		}
	}
}

std::optional<int> TPOnlineMessages::insertData(const int message_id, const QVariant &data)
{
	messageData *msg{findMessage(message_id)};
	if (msg != nullptr)
	{
		msg->data.append(data);
		return msg->data.count() - 1;
	}
	return std::nullopt;
}

void TPOnlineMessages::removeData(const int message_id, const int data_id)
{
	messageData *msg{findMessage(message_id)};
	if (msg != nullptr)
	{
		if (data_id >= 0 && data_id < msg->data.count())
			msg->data.remove(data_id);
	}
}

const QVariant &TPOnlineMessages::getData(const int message_id, const int data_id) const
{
	messageData *msg{findMessage(message_id)};
	if (msg != nullptr)
	{
		if (data_id >= 0 && data_id < msg->data.count())
			return msg->data.at(data_id);
	}
	return emptyData;
}

QVariant TPOnlineMessages::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_data.count())
	{
		switch (role)
		{
			case idRole: return m_data.at(row)->id;
			case labelTextRole: return m_data.at(row)->label;
			case iconRole: return m_data.at(row)->icon;
			case actionsRole: return m_data.at(row)->actions;
			case extraDataRole: return m_data.at(row)->data;
		}
	}
	return emptyData;
}

inline messageData *TPOnlineMessages::findMessage(const int message_id) const
{
	const auto &it = std::find_if(m_data.cbegin(), m_data.cend(), [message_id] (const auto msg) {
		return msg->id == message_id;
	});
	return it != m_data.cend() ? *it : nullptr;
}
