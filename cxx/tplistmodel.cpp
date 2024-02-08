#include "tplistmodel.h"

void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 )
{
	using std::swap;
	swap (model1.m_modeldata, model2.m_modeldata);
	swap (model1.m_roleNames, model2.m_roleNames);
}

void TPListModel::copy ( const TPListModel& src_item )
{
	m_modeldata = src_item.m_modeldata;
	m_roleNames = src_item.m_roleNames;
}

TPListModel::~TPListModel ()
{
	m_modeldata.clear();
	m_roleNames.clear();
}

void TPListModel::setEntireList( const QStringList& newlist )
{
	QStringList::const_iterator itr ( newlist.constBegin () );
	const QStringList::const_iterator itr_end ( newlist.constEnd () );
	m_modeldata.reserve(newlist.count());
	for ( ; itr != itr_end; ++itr )
		m_modeldata.append(static_cast<QStringList>(*itr));
}

void TPListModel::updateList (const QStringList& list, const int row)
{
	m_modeldata.replace(row, list);
	emit dataChanged(index(row, 0), index(row, list.count()-1));
}

void TPListModel::removeFromList (const int row)
{
	beginRemoveRows(QModelIndex(), row, row);
	m_modeldata.remove(row);
	emit countChanged();
	endRemoveRows();
}

void TPListModel::appendList(const QStringList& list)
{
	beginInsertRows(QModelIndex(), count(), count());
	m_modeldata.append(list);
	emit countChanged();
	endInsertRows();
}

void TPListModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, count()-1);
	m_modeldata.clear();
	emit countChanged();
	endRemoveRows();
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
