#include "dbexercisesmodel.h"

DBExercisesModel::DBExercisesModel(QObject *parent)
	: TPListModel(parent), m_selectedEntryToReplace(0)
{
	m_tableId = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);

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
	m_roleNames[fromListRole] = "fromList";
	m_roleNames[actualIndexRole] = "actualIndex";
	m_roleNames[selectedRole] = "selected";

	filterSearch_Field1 = 3; //First look for muscularGroup
	filterSearch_Field2 = 1; //Then look for mainName
	filterSearch_Field3 = 2; //Finally, subName
}

void DBExercisesModel::clear()
{
	clearSelectedEntries();
	TPListModel::clear();
}

void DBExercisesModel::updateFromModel(TPListModel* model)
{
	if (model->count() > 0)
	{
		QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
		uint lastIndex(m_modeldata.count());
		do {
			//Only import user added or modified exercise entries
			if ((*lst_itr).at(9) == u"0"_qs)
			{
				m_modifiedIndices.append(lastIndex++);
				appendList((*lst_itr));
			}
		} while (++lst_itr != lst_itrend);
	}
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
			case fromListRole:
			case selectedRole:
				return !m_bFilterApplied ? bool(m_modeldata.at(row).at(role-Qt::UserRole) == u"1"_qs) :
					bool(m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole) == u"1"_qs);
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

			case fromListRole:
			case selectedRole:
				if (!m_bFilterApplied)
					m_modeldata[row][role-Qt::UserRole] = value.toBool() ? u"1"_qs : u"0"_qs;
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = value.toBool() ? u"1"_qs : u"0"_qs;
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}

void DBExercisesModel::clearSelectedEntries()
{
	for (uint i(0); i < m_selectedEntries.count(); ++i)
		setSelected(m_selectedEntries.at(i), false);
	m_selectedEntries.clear();
	m_selectedEntryToReplace = 0;
}

int DBExercisesModel::manageSelectedEntries(uint index, const uint max_selected)
{
	if (m_bFilterApplied)
		index = m_modeldata.at(m_indexProxy.at(index)).at(9).toUInt();

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
	return -1;
}
