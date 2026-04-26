#include "pageslistmodel.h"

#include "dbmesocyclesmodel.h"
#include "qmlitemmanager.h"
#include "dbusermodel.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <ranges>

#ifndef Q_OS_ANDROID
QHash<QString,PagesListModel*> PagesListModel::app_Pages_list_models{};
#else
PagesListModel *PagesListModel::app_Pages_list_model(nullptr);
#endif

enum RoleNames {
	createRole(displayText, 0)
	createRole(page, 1)
	createRole(buttonEnabled, 2)
};

PagesListModel::PagesListModel(QObject *parent) : QAbstractListModel{parent}
{
	#ifndef Q_OS_ANDROID
	app_Pages_list_models.insert(appSettings()->currentUser(), this);
	if (app_Pages_list_models.count() > 1) {
		for (const auto page_info : std::as_const(app_Pages_list_models)) {
			if (page_info->m_pagesData.count() > 0 && page_info->m_pagesData.first()->page) {
				insertHomePage(page_info->m_pagesData.first()->page);
				break;
			}
		}
	}
	else
		insertHomePage(appItemManager()->AppHomePage());
	m_backKey = Qt::Key_Left;
	#else
	app_Pages_list_model = this;
	m_backKey = Qt::Key_Back;
	insertHomePage(appItemManager()->AppHomePage());
	#endif
	roleToString(displayText)
	roleToString(page)
	roleToString(buttonEnabled)
}

#ifndef Q_OS_ANDROID
void PagesListModel::userSwitchingActions()
{
	QMetaObject::invokeMethod(appMainWindow(), "clearWindowsStack");
	for (const auto page_st : std::as_const(m_pagesData))
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page_st->page));
	appUserModel()->actualMesoModel()->setCurrentMesosView(appUserModel()->actualMesoModel()->isOwnMeso(m_pagesMesoIdx.last()));
	emit countChanged();
	setCurrentIndex(m_pagesData.count() - 1);
	emit dataChanged(index(0, 0), index(count() - 1, 0));
}
#endif

void PagesListModel::setCurrentIndex(const uint new_index)
{
	if (m_pagesIndex != new_index) {
		if (m_pagesData.at(m_pagesIndex)->page->objectName() == "settingsPage"_L1)
			emit appSettingsButtonEnabled(true);
		else if (m_pagesData.at(m_pagesIndex)->page->objectName() == "userPage"_L1)
			emit userSettingsButtonEnabled(true);
		emit dataChanged(QAbstractListModel::index(m_pagesIndex, 0), QAbstractListModel::index(m_pagesIndex, 0), { buttonEnabledRole });
		m_pagesIndex = new_index;
		emit dataChanged(QAbstractListModel::index(m_pagesIndex, 0), QAbstractListModel::index(m_pagesIndex, 0), { buttonEnabledRole });
		if (m_pagesData.at(m_pagesIndex)->page->objectName() == "settingsPage"_L1)
			emit appSettingsButtonEnabled(false);
		else if (m_pagesData.at(m_pagesIndex)->page->objectName() == "userPage"_L1)
			emit userSettingsButtonEnabled(false);
		emit currentIndexChanged();
	}
}

void PagesListModel::removeEventFilter()
{
	qApp->removeEventFilter(this);
}

void PagesListModel::reinstallEventFilter()
{
	qApp->installEventFilter(this);
}

void PagesListModel::insertHomePage(QQuickItem *page)
{
	pageInfo *pageinfo{new pageInfo};
	pageinfo->displayText = std::move(tr("Home"));
	pageinfo->page = page;
	m_pagesData.append(pageinfo);
	m_pagesMesoIdx.append(-1);
	#ifndef Q_OS_ANDROID
	if (app_Pages_list_models.count() > 1)
		qApp->removeEventFilter(app_Pages_list_models.constBegin().value());
	#endif
	qApp->installEventFilter(this);
}

void PagesListModel::openPage(QQuickItem *page, QString &&label, const std::function<void ()> &clean_up_func)
{
	int index{0};
	for (const auto page_st : std::as_const(m_pagesData)) {
		if (page_st->page == page) {
			openMainMenuShortCut(index);
			return;
		}
		++index;
	}
	beginInsertRows(QModelIndex{}, count(), count());
	pageInfo *pageinfo{new pageInfo};
	if (!label.isEmpty())
		pageinfo->displayText = std::move(label);
	if (clean_up_func)
		pageinfo->cleanUpFunc = clean_up_func;
	pageinfo->page = page;
	m_pagesData.append(pageinfo);
	m_pagesMesoIdx.append(appUserModel()->actualMesoModel()->currentWorkingMeso());
	endInsertRows();
	openQMLPage(m_pagesData.count() - 1);
	emit countChanged();
	setCurrentIndex(m_pagesData.count() - 1);
}

void PagesListModel::closePage(QQuickItem *page)
{
	for (uint i{0}; i < m_pagesData.count(); ++i) {
		if (m_pagesData.at(i)->page == page) {
			closePage(i);
			return;
		}
	}
}

void PagesListModel::closePage(const uint index)
{
	if (index > 0 && index < m_pagesData.count()) {
		QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, m_pagesData.at(index)->page));
		beginRemoveRows(QModelIndex{}, index, index);
		if (m_pagesData.at(index)->cleanUpFunc)
			m_pagesData.at(index)->cleanUpFunc();
		delete m_pagesData.at(index);
		m_pagesData.remove(index);
		m_pagesMesoIdx.remove(index);
		endRemoveRows();
		emit countChanged();
		if (m_pagesIndex >= index && m_pagesIndex > 0)
			setCurrentIndex(--m_pagesIndex);
	}
}

void PagesListModel::openMainMenuShortCut(const uint index, const bool change_order)
{
	openQMLPage(index);
	if (change_order) {
		setCurrentIndex(m_pagesData.count() - 1);
		if (index != currentIndex()) {
			beginMoveRows(QModelIndex{}, index, index, QModelIndex{}, m_pagesIndex + 1);
			m_pagesData.move(index, m_pagesIndex);
			endMoveRows();
		}
		return;
	}
	setCurrentIndex(index);
}

void PagesListModel::changeLabel(QQuickItem *page, QString &&new_label)
{
	for (uint i{0}; i < m_pagesData.count(); ++i) {
		if (m_pagesData.at(i)->page == page) {
			m_pagesData.at(i)->displayText = std::move(new_label);
			return;
		}
	}
}

void PagesListModel::openMainMenu()
{
	QMetaObject::invokeMethod(appMainWindow(), "openMainMenu");
}

void PagesListModel::openPopup(QObject *popup, QQuickItem *parentPage, const int position, QQuickItem *widget)
{
	const QVariant &keepAbove{popup->property("keepAbove")};
	if (keepAbove.isValid() && keepAbove.toBool()) {
		pageInfo *page_info{getPageInfo(parentPage)};
		if (page_info) {
			const QList<QObject*>::const_iterator _popup{std::find_if(page_info->tpPopups.cbegin(), page_info->tpPopups.cend(),
																								[popup] (const QObject *tppopup) {
				return tppopup == popup;
			})};
			if (_popup == page_info->tpPopups.cend()) {
				popup->setProperty("z", std::move(QVariant{page_info->tpPopups.count()}));
				page_info->tpPopups.append(popup);
				connect(popup, SIGNAL(popupClosed(QObject*)), this, SLOT(popupClosed(QObject*)));
			}
		}
	}
	popup->setProperty("parentPage", std::move(QVariant::fromValue(parentPage)));
	popup->setProperty("show_position", std::move(QVariant{position}));
	popup->setProperty("open_in_window", std::move(QVariant{widget == nullptr}));
	popup->setProperty("reference_widget", std::move(QVariant::fromValue(widget)));
	QMetaObject::invokeMethod(popup, "tpOpen");
}

void PagesListModel::raisePopup(QObject* popup)
{
	pageInfo *page_info{getPageInfo(popup)};
	if (page_info) {
		if (popup != page_info->tpPopups.constLast()) {
			changePopupStackOrder(popup, page_info);
			popup->setProperty("z", page_info->tpPopups.count() - 1);
			const auto z_order{page_info->tpPopups.indexOf(popup)};
			page_info->tpPopups.move(z_order, page_info->tpPopups.count() - 1);
			popup->setProperty("visible", true);
			QMetaObject::invokeMethod(popup, "forceActiveFocus");
		}
	}
}

void PagesListModel::hidePopup(QObject *popup)
{
	pageInfo *page_info{getPageInfo(popup)};
	if (page_info) {
		changePopupStackOrder(popup, page_info);
		popup->setProperty("visible", false);
	}
}

bool PagesListModel::isPopupAboveAllOthers(QObject* popup) const
{
	pageInfo *page_info{getPageInfo(popup)};
	if (page_info)
		return page_info->tpPopups.indexOf(popup) == page_info->tpPopups.count() - 1;
	return false;
}

QVariant PagesListModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_pagesData.count()) {
		switch (role) {
		case displayTextRole:	return m_pagesData.at(row)->displayText;
		case pageRole:			return QVariant::fromValue(m_pagesData.at(row)->page);
		case buttonEnabledRole:	return m_pagesIndex != row;
		}
	}
	return QVariant{};
}

void PagesListModel::popupClosed(QObject* popup)
{
	const QVariant &keepAbove{popup->property("keepAbove")};
	if (keepAbove.isValid() && keepAbove.toBool()) {
		pageInfo *page_info{getPageInfo(popup)};
		if (page_info) {
			page_info->tpPopups.removeOne(popup);
			changePopupStackOrder(popup, page_info);
		}
	}
}

bool PagesListModel::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *key_event{static_cast<QKeyEvent*>(event)};
		if (key_event->key() == m_backKey) {
			pageInfo *page_info{m_pagesData.constLast()};
			if (!page_info->tpPopups.isEmpty()) {
				QObject *popup{page_info->tpPopups.last()};
				if (popup->property("modal").toBool() || popup->property("keepAbove").toBool())
					QMetaObject::invokeMethod(popup, "backKeyPressed");
				else
					QMetaObject::invokeMethod(popup, "tpClose");
			}
			else {
				if (currentIndex() != 0)
					prevPage();
				else {
					auto conn{std::make_shared<QMetaObject::Connection>()};
					*conn = connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this,conn]
																									(const uint8_t button_idx) {
						disconnect(*conn);
						if (button_idx == 1)
							qApp->exit();
					});
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings( {tr("Exit"),
					tr("Are you sure you want to leave?")}, record_separator), Qt::AlignCenter, "question_"_L1, 0, tr("Yes"), tr("No"));
				}
			}
			return true; // Return true to stop the event from propagating
		}
		else
			return false;
	}
	return QObject::eventFilter(obj, event);
}

inline PagesListModel::pageInfo *PagesListModel::getPageInfo(QQuickItem *qml_page) const
{
	auto page_info{std::find_if(m_pagesData.cbegin(), m_pagesData.cend(), [qml_page] (const pageInfo *pageinfo) {
		return pageinfo->page == qml_page;
	})};
	return page_info != m_pagesData.cend() ? *page_info : nullptr;
}

inline PagesListModel::pageInfo *PagesListModel::getPageInfo(QObject *tp_popup) const
{
	const QVariant &parentPageVar{tp_popup->property("parentPage")};
	if (parentPageVar.isValid()) {
		QQuickItem *parentPage{parentPageVar.value<QQuickItem*>()};
		if (parentPage)
			return getPageInfo(parentPage);
	}
	return nullptr;
}

void PagesListModel::openQMLPage(const uint index)
{
	deActivateQmlPage(currentIndex());
	QQuickItem *page{m_pagesData.at(index)->page};
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
	activateQmlPage(index);
	//if (index > 0)
	//	appUserModel()->actualMesoModel()->setCurrentMesosView(appUserModel()->actualMesoModel()->isOwnMeso(m_pagesMesoIdx.at(index)));
}

void PagesListModel::activateQmlPage(const uint index)
{
	pageInfo *page_info{m_pagesData.at(index)};
	QMetaObject::invokeMethod(page_info->page, "pageActivated");
	for (const auto popup: std::as_const(page_info->tpPopups))
		QMetaObject::invokeMethod(popup, "tpOpen");
}

void PagesListModel::deActivateQmlPage(const uint index)
{
	pageInfo *page_info{m_pagesData.at(index)};
	for (const auto popup: std::as_const(page_info->tpPopups))
		QMetaObject::invokeMethod(popup, "tpClose");
	QMetaObject::invokeMethod(m_pagesData.at(index)->page, "pageDeActivated");
}

void PagesListModel::changePopupStackOrder(QObject *popup, pageInfo *page_info)
{
	const auto z_order{page_info->tpPopups.indexOf(popup)};
	if (z_order >= 0 && z_order < page_info->tpPopups.count() - 1) {
		int i{-1};
		for (const auto pop_up : std::as_const(page_info->tpPopups) | std::views::drop(z_order + 1))
			pop_up->setProperty("z", z_order + ++i);
	}
}

