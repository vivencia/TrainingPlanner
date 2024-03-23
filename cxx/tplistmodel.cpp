#include "tplistmodel.h"

#include <QRegularExpression>

void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 )
{
	using std::swap;
	swap (model1.m_modeldata, model2.m_modeldata);
	swap (model1.m_roleNames, model2.m_roleNames);
	swap (model1.m_indexProxy, model2.m_indexProxy);
}

void TPListModel::copy ( const TPListModel& src_item )
{
	m_modeldata = src_item.m_modeldata;
	m_roleNames = src_item.m_roleNames;
	m_indexProxy = src_item.m_indexProxy;
}

TPListModel::~TPListModel ()
{
	m_modeldata.clear();
	m_roleNames.clear();
	m_indexProxy.clear();
}

void TPListModel::updateList (const QStringList& list, const int row)
{
	const uint actual_row(m_indexProxy.at(row));
	m_modeldata.replace(actual_row, list);
	emit dataChanged(index(row, 0), index(row, list.count()-1));
}

void TPListModel::removeFromList (const int row)
{
	if (row < count())
	{
		beginRemoveRows(QModelIndex(), row, row);
		m_modeldata.remove(row);
		m_indexProxy.remove(row);
		for( uint i (row); i < m_modeldata.count(); ++i )
			m_indexProxy[i] = i-1;
		emit countChanged();
		endRemoveRows();
	}
}

void TPListModel::appendList(const QStringList& list)
{
	beginInsertRows(QModelIndex(), count(), count());
	m_modeldata.append(list);
	m_indexProxy.append(m_modeldata.count() - 1);
	emit countChanged();
	endInsertRows();
}

void TPListModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, count()-1);
	m_modeldata.clear();
	m_indexProxy.clear();
	setReady(false);
	setModified(true);
	emit countChanged();
	endRemoveRows();
}

void TPListModel::setCurrentRow(const int row)
{
	if (row >= -1 && row < m_indexProxy.count())
	{
		m_currentRow = row;
		emit currentRowChanged();
	}
}

void TPListModel::moveRow(const uint from, const uint to)
{
	if (from < count() && to < count())
	{
		const QModelIndex sourceParent(index(from < to ? from : to, 0));
		const QModelIndex destinationParent(index(to > from ? to : from, 0));
		const QStringList tempList(m_modeldata.at(from));
		if (to > from)
		{
			for(uint i(from); i < to; ++i)
				m_modeldata[i] = m_modeldata.at(i+1);
		}
		else
		{
			for(uint i(from); i > to; --i)
				m_modeldata[i] = m_modeldata.at(i-1);
		}
		m_modeldata[to] = tempList;
		QList<int> roles;
		for (uint role(0); role < m_roleNames.count(); ++role)
			roles.append(Qt::UserRole+role);
		emit dataChanged(sourceParent, destinationParent, roles);
		setModified(true);
	}
}

void TPListModel::setFilter(const QString &filter)
{
	if ( filter.length() >=3 )
	{
		QList<QStringList>::const_iterator lst_itr ( m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend ( m_modeldata.constEnd());
		uint idx(0);
		bool bFound(false), bFirst(true);

		const QRegularExpression regex(filter, QRegularExpression::CaseInsensitiveOption);
		for ( ; lst_itr != lst_itrend; ++lst_itr, ++idx )
		{
			bFound = regex.match(static_cast<QStringList>(*lst_itr).at(filterSearch_Field1)).hasMatch();
			if (!bFound)
				bFound = regex.match(static_cast<QStringList>(*lst_itr).at(filterSearch_Field2)).hasMatch();

			if (bFound)
			{
				if (bFirst)
				{
					bFirst = false;
					beginRemoveRows(QModelIndex(), 0, count()-1);
					m_indexProxy.clear();
					endRemoveRows();
				}
				beginInsertRows(QModelIndex(), count(), count());
				m_indexProxy.append(idx);
				endInsertRows();
			}
		}
		m_bFilterApplied = m_indexProxy.count() != m_modeldata.count();
	}
	else
	{
		if (m_bFilterApplied)
		{
			m_bFilterApplied = false;
			beginRemoveRows(QModelIndex(), 0, count()-1);
			m_indexProxy.clear();
			endRemoveRows();
			beginInsertRows(QModelIndex(), 0, m_modeldata.count());
			for( uint i (0); i < m_modeldata.count(); ++i )
				m_indexProxy.append(i);
			endInsertRows();
		}
	}
}

void TPListModel::makeFilterString(const QString& text)
{
	m_filterString = text;
	m_filterString = m_filterString.replace(',', ' ').simplified();
	const QStringList words(m_filterString.split(' '));

	if ( words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator itr_end(words.end());
		m_filterString.clear();

		do
		{
			if(static_cast<QString>(*itr).length() < 3)
				continue;
			if (!m_filterString.isEmpty())
				m_filterString.append('|');
			m_filterString.append(static_cast<QString>(*itr).toLower());
			if (m_filterString.endsWith('s', Qt::CaseInsensitive) )
				m_filterString.chop(1);
			m_filterString.remove('.');
			m_filterString.remove('(');
			m_filterString.remove(')');
		} while (++itr != itr_end);
	}
}
