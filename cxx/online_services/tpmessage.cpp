#include "tpmessage.h"

#include "tpmessagesmanager.h"
#include "../tputils.h"

#include <QTimer>

auto find_itr_const = [] (const TPMessage *const parent, const TPMessage *const child) {
	return std::find_if(parent->children().cbegin(), parent->children().cend(), [child] (const auto &_child) {
		return child == _child.get();
	});
};
auto find_itr = [] (TPMessage *parent, TPMessage *child) {
	return std::find_if(parent->children().begin(), parent->children().end(), [child] (const auto &_child) {
		return child == _child.get();
	});
};

TPMessage::~TPMessage()
{
	if (m_fileOps)
		delete m_fileOps;
}

TPMessage *TPMessage::findChild(const QVariant &value, const TPMessageFields field) const
{
	if (m_children.size() > 0) {
		for (const auto &child : std::as_const(m_children)) {
			bool match{false};
			switch (field) {
			case FIELD_ID: match = value == child->m_id; break;
			case FIELD_ROW: match = value == child->row(); break;
			case FIELD_USERID: match = value == child->m_userid; break;
			case FIELD_TYPE: match = value == child->m_type; break;
			case FIELD_TITLE: match = value == child->m_title.value(); break;
			case FIELD_TEXT: match = value == child->m_text.value(); break;
			case FIELD_ICON: match = value == child->m_icon.value(); break;
			case FIELD_DATETIME: match = value == child->m_dateTime.value(); break;
			case FIELD_FILE: child->m_fileOps ? match = value.value<TPFileOps*>() == m_fileOps : false; break;
			case FIELD_EXTRA_INFO: match = value == child->m_extraInfo.value(); break;
			case FIELD_EXTRA_ICON: match = value == child->m_extraImage.value(); break;
			case FIELD_EXPIRATION: match = value == child->m_expirationTime; break;
			default: break;
			}
			if (match)
				return child.get();
		}
	}
	return nullptr;
}

void TPMessage::insertChild(TPMessage *child, const uint row)
{
	if (!isChild(child)) {
		if (row == childCount()) {
			m_children.push_back(std::move(std::unique_ptr<TPMessage>{child}));
		} else {
			auto itr{find_itr(this, m_children.at(row).get())};
			static_cast<void>(m_children.emplace(itr, std::move(std::unique_ptr<TPMessage>{child})));
		}
	}
}

void TPMessage::removeChild(TPMessage *child)
{
	if (isChild(child)) {
		child->removeAllChildren();
		m_children.erase(std::remove(m_children.begin(), m_children.end(),
															*find_itr_const(this, child)), m_children.end());
	}
}

void TPMessage::removeAllChildren()
{
	for(auto &child : std::as_const(m_children)) {
		child->removeAllChildren();
		m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
	}
}

int TPMessage::row() const
{
	if (m_parentMessage == nullptr)
		return 0;
	const auto it{std::find_if(m_parentMessage->m_children.cbegin(), m_parentMessage->m_children.cend(),
														[this](const std::unique_ptr<TPMessage> &message) {
		return message.get() == this;
	})};
	if (it != m_parentMessage->m_children.cend())
		return std::distance(m_parentMessage->m_children.cbegin(), it);
	Q_ASSERT(false); // should not happen
	return -1;
}

void TPMessage::setFileName(const QString &filename)
{
	m_fileOps = new TPFileOps;
	m_fileOps->setUseControls(true);
	m_fileOps->setCanDownloadOrGenerate(true);
	m_fileOps->setFileName(filename);
	m_fileOps->attemptToCreateOrGetFile();
	connect(m_fileOps, &TPFileOps::fileRemovalRequested, this, [this] () { emit killMessage(); });
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
				m_timer->callOnTimeout([this] () { emit killMessage(); } );
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

void TPMessage::setDateTime(const QDateTime &ctime)
{
	m_dateTime = std::move(appUtils()->formatDateTime(ctime,
		static_cast<int>(TPUtils::DF_LOCALE)|static_cast<int>(TPUtils::TF_QML_DISPLAY_NO_SEC), QLatin1Char{' '}));
}

int TPMessage::insertAction(QString &&label, const ActionType type, const std::function<QVariant(const QVariant &)> &func)
{
	st_Action new_action;
	new_action.label = std::forward<QString>(label);
	new_action.type = type;
	new_action.func = func;
	m_actions.append(std::move(new_action));
	emit actionsChanged();
	return m_actions.count() - 1;
}

inline bool TPMessage::isChild(TPMessage *msg) const
{
	auto itr{std::find_if(m_children.cbegin(), m_children.cend(), [msg] (const auto &child) {
		return msg == child.get();
	})};
	return itr != m_children.cend();
}
