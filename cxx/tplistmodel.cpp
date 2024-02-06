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
			return true;
		}
	}
	return false;
}
