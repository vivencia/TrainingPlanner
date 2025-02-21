#include "pageslistmodel.h"

#include "qmlitemmanager.h"

#include <QQuickItem>
#include <QQuickWindow>

PagesListModel::~PagesListModel()
{
	for (uint i(0); i < m_modeldata.count(); ++i)
		delete m_modeldata.at(i);
}

void PagesListModel::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
	if (label.isEmpty())
		return;

	for (uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i)->page == page)
			return;
	}

	beginInsertRows(QModelIndex{}, count(), count());
	pageInfo* pageinfo{new pageInfo};
	pageinfo->displayText = label;
	pageinfo->page = page;
	m_modeldata.append(pageinfo);
	emit countChanged();
	endInsertRows();
}

void PagesListModel::removeMainMenuShortCut(QQuickItem* page)
{
	for (uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i)->page == page)
		{
			QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, page));
			beginRemoveRows(QModelIndex{}, i, i);
			delete m_modeldata.at(i);
			m_modeldata.remove(i);
			emit countChanged();
			endRemoveRows();
			return;
		}
	}
}

void PagesListModel::removeMainMenuShortCut(const uint index)
{
	if (index < m_modeldata.count())
	{
		QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, m_modeldata.at(index)->page));
		beginRemoveRows(QModelIndex{}, index, index);
		delete m_modeldata.at(index);
		m_modeldata.remove(index);
		emit countChanged();
		endRemoveRows();
	}
}

void PagesListModel::openMainMenuShortCut(const uint index) const
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_modeldata.at(index)->page));
}

QVariant PagesListModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if(row >= 0 && row < m_modeldata.count())
	{
		if (role == displayTextRole)
			return m_modeldata.at(row)->displayText;
		else
			return QVariant::fromValue(m_modeldata.at(row)->page);
	}
	return QVariant();
}
