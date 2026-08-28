#include "tpmessage.h"

#include "tpmessagesmanager.h"
#include "../qmlitemmanager.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QTimer>

QQmlComponent *TPMessage::_actionsLayoutComponent{nullptr};
QQmlComponent *TPMessage::_fileViewerComponent{nullptr};

TPMessage::~TPMessage()
{
	if (m_fileOps)
		delete m_fileOps;
	if (!m_actions.isEmpty()) {
		m_actions.clear();
		if (m_actionsLayout) {
			delete _actionsLayoutComponent;
			delete m_actionsLayout;
		}
	}
	if (m_fileViewer) {
		delete _fileViewerComponent;
		delete m_fileViewer;
	}
}

TPMessage *TPMessage::findChild(const QVariant &value, const TPMessageFields field) const
{
	if (m_children.size() > 0) {
		for (const auto child : std::as_const(m_children)) {
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
				return child;
		}
	}
	return nullptr;
}

void TPMessage::insertChild(TPMessage *child, const uint row)
{
	if (!isChild(child)) {
		if (row == childCount())
			m_children.append(child);
		else
			m_children.insert(row, child);
		emit childCountChanged();
	}
}

void TPMessage::removeChild(TPMessage *child)
{
	if (isChild(child)) {
		child->removeAllChildren();
		m_children.remove(child->row());
		child->deleteLater();
	}
}

void TPMessage::removeAllChildren()
{
	for(auto child : std::as_const(m_children))
		removeChild(child);
	emit childCountChanged();
}

int TPMessage::row() const
{
	if (m_parentMessage == nullptr)
		return 0;
	const auto it{std::find_if(m_parentMessage->m_children.cbegin(), m_parentMessage->m_children.cend(),
																						[this] (TPMessage *message) {
		return message == this;
	})};
	if (it != m_parentMessage->m_children.cend())
		return std::distance(m_parentMessage->m_children.cbegin(), it);
	Q_ASSERT(false); // should not happen
	return -1;
}

void TPMessage::setFileName(const QString &filename)
{
	if (!filename.isEmpty()) {
		m_fileOps = new TPFileOps;
		m_fileOps->setUseControls(true);
		m_fileOps->setCanDownloadOrGenerate(true);
		m_fileOps->setFileName(filename);
		if (!m_fileViewer)
			createFileViewer();
		m_fileOps->attemptToCreateOrGetFile();
		connect(m_fileOps, &TPFileOps::fileRemovalRequested, this, [this] () { emit killMessage(); });
		connect(m_fileOps, &TPFileOps::tpFileImported, this, [this] (const bool success) {
			if (success)
				emit killMessage();
		});
	}
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

int TPMessage::insertAction(QString &&label, const ActionType type, int index,
									const std::function<QVariant(const QVariant &)> &func, const bool setup_actions)
{
	st_Action new_action;
	new_action.label = std::forward<QString>(label);
	new_action.type = type;
	new_action.func = func;
	if (index == -1)
		m_actions.append(std::move(new_action));
	else
		m_actions.insert(index, std::move(new_action));
	if (setup_actions) {
		if (!m_actionsLayout)
			createActionsLayout();
		else
			setupActionsLayout(index == -1, false, index >= 0);
	}
	emit actionCountChanged();
	return m_actions.count() - 1;
}

void TPMessage::popupSizeChanged(const qreal w_ratio, const qreal h_ratio)
{
	for (auto &action : std::as_const(m_actions)) {
		auto width{action.qml_item->property("width").toReal()};
		action.qml_item->setProperty("width", width * w_ratio);
	}
}

inline bool TPMessage::isChild(TPMessage *msg) const
{
	auto itr{std::find_if(m_children.cbegin(), m_children.cend(), [msg] (auto child) {
		return msg == child;
	})};
	return itr != m_children.cend();
}

void TPMessage::createActionsLayout()
{
	if (!_actionsLayoutComponent) {
		_actionsLayoutComponent = new QQmlComponent{appQmlEngine(), "TpQml.Widgets"_L1, "TPLayout"_L1, QQmlComponent::PreferSynchronous};
		if (!_actionsLayoutComponent || _actionsLayoutComponent->isError()) {
			qDebug() << _actionsLayoutComponent->errorString();
			return;
		}
	}
	m_actionsLayout = qobject_cast<QQuickItem*>(_actionsLayoutComponent->create(appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_actionsLayout) {
		qCritical() << _actionsLayoutComponent->errorString();
		return;
	}
#endif
	m_actionsLayout->setParentItem(m_actionsLayoutParent);
	connect(m_actionsLayout, SIGNAL(execAction(int,QVariant)), this, SLOT(execAction(int,QVariant)));
	appQmlEngine()->setObjectOwnership(m_actionsLayout, QQmlEngine::CppOwnership);
	m_actionsLayout->setWidth(appSettings()->getCustomValue(appMessagesManager()->messagesManagerDialog()->objectName()
		% ".size"_L1, appMessagesManager()->messagesManagerDialog()->property("normal_size").toSize()).toSize().width());
	setupActionsLayout(false, false, false);
	connect(appMessagesManager()->messagesManagerDialog(), SIGNAL(popupSizeChanged(qreal,qreal)), this,
																				SLOT(popupSizeChanged(qreal,qreal)));
}

void TPMessage::setupActionsLayout(const bool append, const bool remove_last, const bool reset)
{
	if (append) {
		createActionItem(m_actions.count() - 1);
	} else if (remove_last) {
		QVariantList row_width_list{m_actionsLayout->property("_row_width").toList()};
		qreal last_row_width{row_width_list.last().toReal()};
		last_row_width -= m_actions.constLast().qml_item->width();
		row_width_list.replace(row_width_list.count() - 1, std::move(last_row_width));
		delete m_actions.last().qml_item;
		m_actions.last().qml_item = nullptr;
		const auto new_last{m_actions.count() - 2};
		QMetaObject::invokeMethod(m_actionsLayout, "reLayoutLastRow", Q_ARG(QQuickItem*, m_actions.at(new_last).qml_item),
			last_row_width > m_actions.at(new_last).qml_item->width() ? m_actions.at(new_last-1).qml_item : nullptr);
	} else {
		if (reset) {
			m_actionsLayout->setHeight(0.0);
			m_actionsLayout->setProperty("_row_width", std::move(QVariantList{1, 0.0}));
			for (const auto &action : std::as_const(m_actions))
				delete action.qml_item;
		}
		for (int i{0}; i < m_actions.size(); ++i)
			createActionItem(i);
	}
	if (m_actionsLayoutParent)
		m_actionsLayoutParent->setHeight(m_actionsLayout->height());
	setMessageComponentHeight(MC_ACTIONS, m_actionsLayout->height());
}

void TPMessage::createActionItem(const uint action_index)
{
	st_Action *action{&m_actions[action_index]};
	QMetaObject::invokeMethod(m_actionsLayout, "createItem",
							  Q_RETURN_ARG(QQuickItem*, action->qml_item),
							  Q_ARG(int, action->type), Q_ARG(QString, action->label), Q_ARG(int, action_index));
	QMetaObject::invokeMethod(m_actionsLayout, "placeItem", Q_ARG(QQuickItem*, action->qml_item),
							  Q_ARG(QQuickItem*, action_index > 0 ? m_actions.at(action_index - 1).qml_item : nullptr),
							  Q_ARG(int, action_index), Q_ARG(int, m_actions.count()));
}

void TPMessage::createFileViewer()
{
	if (!_fileViewerComponent) {
		_fileViewerComponent = new QQmlComponent{appQmlEngine(), "TpQml.Widgets"_L1, "TPFileViewer"_L1, QQmlComponent::PreferSynchronous};
		if (!_fileViewerComponent || _fileViewerComponent->isError()) {
			qDebug() << _fileViewerComponent->errorString();
			return;
		};
	}
	m_fileViewerProperties["fileOps"_L1] = std::move(QVariant::fromValue(m_fileOps));
	m_fileViewer =static_cast<QQuickItem*>(_fileViewerComponent->createWithInitialProperties(
																m_fileViewerProperties, appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_fileViewer) {
		qCritical() << _fileViewerComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(m_fileViewer, QQmlEngine::CppOwnership);
	m_fileViewer->setParentItem(m_fileViewerParent);
	const qreal viewer_height{m_fileViewer->property("minimumHeight").toReal()};
	if (m_fileViewerParent) {
		m_fileViewerParent->setHeight(viewer_height);
		QMetaObject::invokeMethod(m_fileViewer, "anchorToParent");
	}
	setMessageComponentHeight(MC_FILEOPS, viewer_height + appSettings()->itemLargeHeight());
}
