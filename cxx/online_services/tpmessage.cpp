#include "tpmessage.h"

#include "tpmessagesmanager.h"
#include "../tpglobals.h"

static const QVariant emptyData{};

void TPMessage::plug()
{
	m_parent->addMessage(this);
}

int TPMessage::insertAction(const QString& actionLabel, const std::function<void(const QVariant &var)> &actionFunc, std::optional<bool> remove)
{
	m_actions.append(actionLabel);
	m_actionFuncs.append(actionFunc);
	if (remove != std::nullopt && remove.has_value())
	{
		m_actions.last().append(remove.value() ? record_separator : set_separator);
		setSticky(remove.value());
	}
	emit actionsChanged();
	return m_actions.count() - 1;
}

void TPMessage::removeAction(const int action_id)
{
	if (action_id >= 0 && action_id < m_actions.count())
	{
		m_actions.remove(action_id);
		m_actionFuncs.remove(action_id);
		emit actionsChanged();
	}
}

void TPMessage::execAction(const int action_id)
{
	if (action_id >= 0 && action_id < m_actions.count())
	{
		const QChar &lastChar{m_actions.at(action_id).last(1).at(0)};
		std::optional<bool> remove{std::nullopt};
		if (lastChar.toLatin1() > set_separator)
			remove = lastChar == record_separator;
		if (m_actionFuncs.at(action_id) != nullptr)
			m_actionFuncs.at(action_id);
		emit actionTriggered(action_id, remove);
	}
}

int TPMessage::insertData(const QVariant &data)
{
	m_data.append(data);
	return m_data.count() - 1;

}

void TPMessage::removeData(const int data_id)
{
	if (data_id >= 0 && data_id < m_data.count())
		m_data.remove(data_id);
}
