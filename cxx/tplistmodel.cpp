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

void TPListModel::setEntireList( const QStringList& newlist )
{
	QStringList::const_iterator itr ( newlist.constBegin () );
	const QStringList::const_iterator itr_end ( newlist.constEnd () );
	m_modeldata.reserve(newlist.count());
	m_indexProxy.reserve(newlist.count());
	for ( uint idx(0); itr != itr_end; ++itr, ++idx )
	{
		m_modeldata.append(static_cast<QStringList>(*itr));
		m_indexProxy.append(idx);
	}
}

void TPListModel::updateList (const QStringList& list, const int row)
{
	const uint actual_row(m_indexProxy.at(row));
	m_modeldata.replace(actual_row, list);
	emit dataChanged(index(row, 0), index(row, list.count()-1));
}

void TPListModel::removeFromList (const int row)
{
	beginRemoveRows(QModelIndex(), row, row);
	m_modeldata.remove(row);
	m_indexProxy.remove(row);
	for( uint i (row); i < m_modeldata.count(); ++i )
		m_indexProxy[i] = i-1;
	emit countChanged();
	endRemoveRows();
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

QString TPListModel::makeFilterString(const QString& text) const
{
	QString filterStr(text);
	filterStr = filterStr.replace(',', ' ').simplified();
	const QStringList words(filterStr.split(' '));

	if ( words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator itr_end(words.end());
		filterStr.clear();

		do
		{
			if(static_cast<QString>(*itr).length() < 3)
				continue;
			if (!filterStr.isEmpty())
				filterStr.append('|');
			filterStr.append(static_cast<QString>(*itr).toLower());
			if (filterStr.endsWith('s', Qt::CaseInsensitive) )
				filterStr.chop(1);
			filterStr.remove('.');
			filterStr.remove('(');
			filterStr.remove(')');
		} while (++itr != itr_end);
	}
	return filterStr;
}

QVariant TPListModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		if (role == Qt::DisplayRole)
			return m_modeldata.at(row).at(Qt::DisplayRole);
	}
	return QVariant();
}

bool TPListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		if (role == Qt::DisplayRole)
		{
			m_modeldata[row].replace(role, value.toString());
			emit dataChanged(index, index, QList<int>() << role);
			return true;
		}
	}
	return false;
}
