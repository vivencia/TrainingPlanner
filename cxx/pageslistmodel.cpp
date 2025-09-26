#include "pageslistmodel.h"

#include "qmlitemmanager.h"

#include <QQuickItem>
#include <QQuickWindow>

#ifndef Q_OS_ANDROID
QHash<QString,PagesListModel*> PagesListModel::app_Pages_list_models{};
#else
PagesListModel *PagesListModel::app_Pages_list_model(nullptr);
#endif

PagesListModel::PagesListModel(QObject *parent)
	: QAbstractListModel{parent}, m_pagesIndex{0}
{
	#ifndef Q_OS_ANDROID
	app_Pages_list_models.insert(appSettings()->currentUser(), this);
	if (app_Pages_list_models.count() > 1)
	{
		for (const auto page_info : std::as_const(app_Pages_list_models))
		{
			if (page_info->m_pagesData.count() > 0 && page_info->m_pagesData.first()->page)
			{
				insertHomePage(page_info->m_pagesData.first()->page);
				break;
			}
		}
	}
	m_backKey = Qt::Key_Left;
	#else
	app_Pages_list_model = this;
	m_backKey = Qt::Key_Back;
	#endif
	m_roleNames[displayTextRole] = std::move("displayText");
	m_roleNames[pageRole] = std::move("page");
}

void PagesListModel::userSwitchingActions()
{
	QMetaObject::invokeMethod(appMainWindow(), "clearWindowsStack");
	for (const auto page_st : std::as_const(m_pagesData))
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page_st->page), Q_ARG(bool, false));
	emit countChanged();
	emit currentIndexChanged();
	emit dataChanged(index(0, 0), index(count() - 1, 0));
}

void PagesListModel::insertHomePage(QQuickItem *page)
{
	pageInfo *pageinfo{new pageInfo};
	pageinfo->displayText = std::move(tr("Home"));
	pageinfo->page = page;
	m_pagesData.append(pageinfo);
	#ifndef Q_OS_ANDROID
	if (app_Pages_list_models.count() > 1)
		qApp->removeEventFilter(app_Pages_list_models.constBegin().value());
	#endif
	qApp->installEventFilter(this);
}

void PagesListModel::openPage(const QString &label, QQuickItem *page, const std::function<void ()> &clean_up_func)
{
	int index{0};
	for (const auto page_st : std::as_const(m_pagesData))
	{
		if (page_st->page == page)
		{
			openMainMenuShortCut(index);
			return;
		}
		++index;
	}
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page), Q_ARG(bool, true));

	beginInsertRows(QModelIndex{}, count(), count());
	pageInfo *pageinfo{new pageInfo};
	pageinfo->displayText = label;
	pageinfo->cleanUpFunc = clean_up_func;
	pageinfo->page = page;
	m_pagesData.append(pageinfo);
	endInsertRows();
	emit countChanged();
	setCurrentIndex(m_pagesData.count() - 1);
}

void PagesListModel::closePage(QQuickItem *page)
{
	for (uint i{0}; i < m_pagesData.count(); ++i)
	{
		if (m_pagesData.at(i)->page == page)
		{
			closePage(i);
			return;
		}
	}
}

void PagesListModel::closePage(const uint index)
{
	if (index > 0 && index < m_pagesData.count())
	{
		QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, m_pagesData.at(index)->page));
		beginRemoveRows(QModelIndex{}, index, index);
		if (m_pagesData.at(index)->cleanUpFunc)
			m_pagesData.at(index)->cleanUpFunc();
		delete m_pagesData.at(index);
		m_pagesData.remove(index);
		endRemoveRows();
		emit countChanged();
		if (m_pagesIndex >= index && m_pagesIndex > 0)
			setCurrentIndex(--m_pagesIndex);
	}
}

void PagesListModel::openMainMenuShortCut(const uint index, const bool change_order)
{
	if (index > 0)
	{
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_pagesData.at(index)->page),
					Q_ARG(bool, true));
		if (change_order)
		{
			setCurrentIndex(m_pagesData.count() - 1);
			if (index != currentIndex())
			{
				beginMoveRows(QModelIndex{}, index, index, QModelIndex{}, m_pagesIndex + 1);
				m_pagesData.move(index, m_pagesIndex);
				endMoveRows();
			}
			return;
		}
	}
	else
		QMetaObject::invokeMethod(appMainWindow(), "goHome");
	setCurrentIndex(index);
}

QVariant PagesListModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if(row >= 0 && row < m_pagesData.count())
	{
		switch (role)
		{
			case displayTextRole:
				return m_pagesData.at(row)->displayText;
			case pageRole:
				return QVariant::fromValue(m_pagesData.at(row)->page);
		}
	}
	return QVariant{};
}

bool PagesListModel::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *key_event{static_cast<QKeyEvent*>(event)};
		if (key_event->key() == m_backKey)
		{
			if (appMainWindow()->property("n_dialogs_open").toInt() > 0)
				QMetaObject::invokeMethod(appMainWindow(), "closeDialog");
			else {
				if (currentIndex() != 0)
					prevPage();
				else
					QMetaObject::invokeMethod(appMainWindow(), "showExitPopUp");
			}
			return true; // Return true to stop the event from propagating
		}
	}
	return QObject::eventFilter(obj, event);
}
