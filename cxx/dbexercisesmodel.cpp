#include "dbexercisesmodel.h"

DBExercisesModel::DBExercisesModel(QObject *parent)
	: TPListModel(parent), m_selectedEntryToReplace(0)
{
	m_tableId = EXERCISES_TABLE_ID;
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

	filterSearch_Field1 = 3; //First look for muscularGroup
	filterSearch_Field2 = 1; //Then look for mainName
}

QVariant DBExercisesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
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
				if (!m_bFilterApplied)
				{
					//MSG_OUT("NO filter: DBExercisesModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_modeldata.at(row).at(role-Qt::UserRole))
					return m_modeldata.at(row).at(role-Qt::UserRole);
				}
				else
				{
					//MSG_OUT("Filter: DBExercisesModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole))
					return m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole);
				}
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
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
				if (!m_bFilterApplied)
					m_modeldata[row][role-Qt::UserRole] = value.toString();
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = value.toString();
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}

int DBExercisesModel::manageSelectedEntries(const uint index, const uint max_selected)
{
	if (max_selected == 1)
	{
		m_selectedEntries.clear();
		m_selectedEntries.append(index);
	}
	else
	{
		const int idx(m_selectedEntries.indexOf(index));
		if (idx == -1)
		{
			if (m_selectedEntries.count() < max_selected)
				m_selectedEntries.append(index);
			else
			{
				if (m_selectedEntryToReplace >= max_selected - 1)
					m_selectedEntryToReplace = 0;
				const uint itemToDeselect(m_selectedEntries.at(m_selectedEntryToReplace));
				m_selectedEntries[m_selectedEntryToReplace] = index;
				m_selectedEntryToReplace++;
				return itemToDeselect;
			}
		}
		else
			m_selectedEntries.remove(idx, 1);
	}
	emit entryIsSelectedChanged();
	return -1;
}
