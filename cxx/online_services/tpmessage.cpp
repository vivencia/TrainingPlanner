#include "tpmessage.h"

#include "tpmessagesmanager.h"
#include "../tputils.h"

#include <QTimer>

#include <ranges>

TPMessage::~TPMessage()
{
	if (m_fileOps)
		delete m_fileOps;
}

TPMessage *TPMessage::findChild(const QVariant &value, const TPMessageFields field) const
{
	if (!m_children.isEmpty()) {
		for (const auto child : std::as_const(m_children)) {
			bool match{false};
			switch (field) {
			case FIELD_ID: match = value == child->m_id; break;
			case FIELD_ROW: match = value == child->m_row; break;
			case FIELD_USERID: match = value == child->m_userid; break;
			case FIELD_TYPE: match = value == child->m_type; break;
			case FIELD_TITLE: match = value == child->m_title.value(); break;
			case FIELD_TEXT: match = value == child->m_text; break;
			case FIELD_ICON: match = value == child->m_icon.value(); break;
			case FIELD_DATETIME: match = value == child->m_dateTime; break;
			case FIELD_FILE: child->m_fileOps ? match = value.value<TPFileOps*>() == m_fileOps : false; break;
			case FIELD_EXTRA_INFO: match = value == child->m_extraInfo.value(); break;
			case FIELD_EXTRA_ICON: match = value == child->m_extraImage; break;
			case FIELD_EXPIRATION: match = value == child->m_expirationTime; break;
			default: break;
			}
			if (match)
				return child;
		}
	}
	return nullptr;
}

void TPMessage::insertChild(TPMessage* child, int row)
{
	auto itr{std::find_if(m_children.cbegin(), m_children.constEnd(), [child] (const auto _child) {
		return child == _child;
	})};
	if (itr == m_children.cend()) {
		if (row >= 0 && row <= m_children.count()) {
			static_cast<void>(m_children.insert(row, child));
		} else {
			row = m_children.count();
			m_children.append(child);
		}
		child->m_parentMessage = this;
		child->m_depth = m_depth + 1;
		child->m_row = row;
	}
}

void TPMessage::removeChild(TPMessage *child)
{
	int i{0};
	bool removed{false};
	for (TPMessage *message : std::as_const(m_children)) {
		if (message == child) {
			message->remove();
			message->deleteLater();
			removed = true;
			continue;
		} else {
			if (removed)
				message->setRow(i);
		}
		++i;
	}
}

void TPMessage::remove(const QLatin1StringView &exclude_type, const bool remove_self_if_no_children)
{
	auto row{m_children.count() - 1};
	for (TPMessage *message : std::as_const(m_children) | std::views::reverse) {
		if (message->m_type != exclude_type) {
			message->remove(exclude_type, remove_self_if_no_children);
			message->deleteLater();
			m_children.remove(row);
		}
		--row;
	}
	if (remove_self_if_no_children && m_children.count() == 0) {
		m_sticky = false;
		emit killMessage(this);
	}
}

void TPMessage::setFileName(const QString &filename)
{
	m_fileOps = new TPFileOps{};
	m_fileOps->setUseControls(true);
	m_fileOps->setCanDownloadOrGenerate(true);
	m_fileOps->setFileName(filename);
	m_fileOps->attemptToCreateOrGetFile();
	connect(m_fileOps, &TPFileOps::fileRemovalRequested, this, [this] () {
		emit killMessage(this);
	});
}

void TPMessage::setExpiration(QDateTime &&date_time)
{
	if (date_time == m_expirationTime) {
		return;
	} else if (date_time.date() != QDate::currentDate()) {
		return;
	} else {
		auto killTimer = [this] () -> void {
			if (m_timer) {
				m_timer->stop();
				delete m_timer;
			}
			m_expirationTime = std::move(QDateTime{});
		};
		if (!date_time.isValid()) {
			killTimer();
			return;
		}
		auto expiration_time{QTime::currentTime().msecsTo(date_time.time())};
		if (expiration_time > 0) {
			if (!m_timer) {
				m_timer = new QTimer{this};
				m_timer->setSingleShot(true);
				m_timer->callOnTimeout([this] () { emit killMessage(this); } );
			} else {
				m_timer->stop();
			}
			m_timer->setInterval(expiration_time);
			m_timer->start();
			m_expirationTime = std::forward<QDateTime>(date_time);
		} else {
			killTimer();
		}
	}
}

QString TPMessage::dateTime() const
{
	return appUtils()->formatDateTime(m_dateTime,
		static_cast<int>(TPUtils::DF_LOCALE)|static_cast<int>(TPUtils::TF_QML_DISPLAY_NO_SEC), QLatin1Char{' '});
}

int TPMessage::insertAction(QString &&label, const ActionType type, const std::function<QVariant(const QVariant &)> &func)
{
	st_Action new_action;
	new_action.label = std::forward<QString>(label);
	new_action.type = type;
	new_action.func = func;
	m_actions.append(std::move(new_action));
	return m_actions.count() - 1;
}
