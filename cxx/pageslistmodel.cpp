#include "pageslistmodel.h"

#include "qmlitemmanager.h"

#include <QQuickItem>
#include <QQuickWindow>

void PagesListModel::addMainMenuShortCut(const QString &label, QQuickItem *page, const std::function<void ()> &clean_up_func)
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
	if (label.isEmpty())
		return;

	for (const auto page_st : std::as_const(m_pagesData))
	{
		if (page_st->page == page)
			return;
	}

	beginInsertRows(QModelIndex{}, count(), count());
	pageInfo *pageinfo{new pageInfo};
	pageinfo->displayText = label;
	pageinfo->cleanUpFunc = clean_up_func;
	pageinfo->page = page;
	m_pagesData.append(pageinfo);
	emit countChanged();
	endInsertRows();
}

void PagesListModel::removeMainMenuShortCut(QQuickItem *page)
{	
	for (uint i{0}; i < m_pagesData.count(); ++i)
	{
		if (m_pagesData.at(i)->page == page)
		{
			removeMainMenuShortCut(i);
			return;
		}
	}
}

void PagesListModel::removeMainMenuShortCut(const uint index)
{
	if (index < m_pagesData.count())
	{
		QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, m_pagesData.at(index)->page));
		beginRemoveRows(QModelIndex{}, index, index);
		if (m_pagesData.at(index)->cleanUpFunc)
			m_pagesData.at(index)->cleanUpFunc();
		delete m_pagesData.at(index);
		m_pagesData.remove(index);
		emit countChanged();
		endRemoveRows();
	}
}

void PagesListModel::openMainMenuShortCut(const uint index) const
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_pagesData.at(index)->page));
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
