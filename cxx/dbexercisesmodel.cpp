#include "dbexercisesmodel.h"

DBExercisesModel::DBExercisesModel(QObject *parent)
	: QAbstractListModel(parent)
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseIdRole] = "exerciseId";
	m_roleNames[mainNameRole] = "mainName";
	m_roleNames[subNameRole] = "subName";
	m_roleNames[muscularGroupRole] = "muscularGroup";
	m_roleNames[nSetsRole] = "nSets";
	m_roleNames[nRepsRole] = "nReps";
	m_roleNames[nWeightRole] = "nWeight";
	m_roleNames[uWeightRole] = "uWeight";
	m_roleNames[mediaPathRole] = "mediaPath";
	m_roleNames[actualIndexRole] = "actualIndex";
}

void DBExercisesModel::setEntireList( const QStringList& newlist )
{
	QStringList::const_iterator itr ( newlist.constBegin () );
	const QStringList::const_iterator itr_end ( newlist.constEnd () );
	m_data.reserve(new_list.count());
	for ( ; itr != itr_end; ++itr )
		m_data.append(QStringList>(*itr));
}

QVariant DBExercisesModel::data(const QModelIndex &index, int role) const
{
	const int row (index.row());
	if( row >= 0 && row < m_data.count() )
	{
		switch(role) {
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case nSetsRole:
			case nRepsRole:
			case nWeightRole:
			case uWeightRole:
			case mediaPathRole:
			case actualIndexRole:
				return m_data.at(row).at(role);
		}
	}
	return QVariant();
}

virtual bool DBExercisesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row (index.row());
	if( row >= 0 && row < m_data.count() )
	{
		switch(role) {
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case nSetsRole:
			case nRepsRole:
			case nWeightRole:
			case uWeightRole:
			case mediaPathRole:
			case actualIndexRole:
				m_data.at(row).replace(role, value.toString());
				return true;
		}
	}
	return false;
}
