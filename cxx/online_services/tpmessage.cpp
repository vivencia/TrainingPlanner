#include "tpmessage.h"

#include "tpmessagesmanager.h"
#include "../tputils.h"

QString TPMessage::date() const
{
	return appUtils()->formatDate(m_ctime.date(), TPUtils::DF_LOCALE);
}

QString TPMessage::time() const
{
	return appUtils()->formatTime(m_ctime.time(), TPUtils::TF_QML_DISPLAY_NO_SEC);
}

void TPMessage::plug()
{
	if (!m_plugged) {
		m_ctime = std::move(QDateTime::currentDateTime());
		m_plugged = true;
		appMessagesManager()->addMessage(this);
	}
}

int TPMessage::insertAction(const QString& actionLabel, const std::function<void(const QVariant &var)> &actionFunc, std::optional<bool> remove)
{
	m_actions.append(actionLabel);
	m_actionFuncs.append(actionFunc);
	if (remove != std::nullopt && remove.has_value()) {
		m_actions.last().append(remove.value() ? record_separator : set_separator);
		setSticky(!remove.value());
	}
	const qsizetype n_action{m_actions.count() - 1};
	emit dataChanged(TPMESSAGE_FIELD_ACTIONS);
	return n_action;
}

void TPMessage::removeAction(const int action_id)
{
	if (action_id >= 0 && action_id < m_actions.count()) {
		m_actions.remove(action_id);
		m_actionFuncs.remove(action_id);
	}
}

void TPMessage::execAction(const int action_id)
{
	if (action_id >= 0 && action_id < m_actions.count())
	{
		const QChar &lastChar{m_actions.at(action_id).last(1).at(0)};
		std::optional<bool> remove{std::nullopt};
		if (lastChar.toLatin1() < set_separator)
			remove = lastChar == record_separator;
		if (m_actionFuncs.at(action_id) != nullptr)
			m_actionFuncs.at(action_id)(action_id < m_data.count() ? m_data.at(action_id) : m_data);
		emit actionTriggered(action_id, remove);
	}
}

uint TPMessage::insertData(const QVariant &data, const int action_id)
{
	if (action_id < 0)
	{
		m_data.append(data);
		return m_data.count() - 1;
	}
	else if (action_id >= m_data.count())
	{
		for (auto i{m_data.count()}; i <= action_id ; ++i)
			m_data.append(QVariant{});
	}
	m_data[action_id] = data;
	return action_id;
}

void TPMessage::removeData(const int data_id)
{
	if (data_id >= 0 && data_id < m_data.count())
		m_data.remove(data_id);
}
