#include "dbmesosplitmodel.h"

DBMesoSplitModel::DBMesoSplitModel(QObject *parent)
	: TPListModel{parent}, m_nextAddedExercisePos(2)
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseNameRole] = "exerciseName";
	m_roleNames[exerciseName1Role] = "exerciseName1";
	m_roleNames[exerciseName2Role] = "exerciseName2";
	m_roleNames[setTypeRole] = "setType";
	m_roleNames[setsNumberRole] = "setsNumber";
	m_roleNames[setsRepsRole] = "setsReps";
	m_roleNames[setsWeightRole] = "setsWeight";
	m_roleNames[setsReps1Role] = "setsReps1";
	m_roleNames[setsWeight1Role] = "setsWeight1";
	m_roleNames[setsReps2Role] = "setsReps2";
	m_roleNames[setsWeight2Role] = "setsWeight2";
}

QVariant DBMesoSplitModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case exerciseNameRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).replace(subrecord_separator, QStringLiteral(" + "));
			case exerciseName1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).left(idx-1) : QString();
			}
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).sliced(idx+1) : QString();
			}
			case setsNumberRole:
			case setsRepsRole:
			case setsWeightRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).left(idx) : m_modeldata.at(row).at(role-Qt::UserRole-2);
			}
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).sliced(idx+1) : m_modeldata.at(row).at(role-Qt::UserRole-4);
			}
			case setTypeRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

bool DBMesoSplitModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case exerciseNameRole:
			case setsNumberRole:
			case setTypeRole:
			case setsRepsRole:
			case setsWeightRole:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
			break;
			case exerciseName1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(subrecord_separator));
				if (idx == -1)
					m_modeldata[row][exerciseNameRole-Qt::UserRole] = value.toString();
				else
					m_modeldata[row][exerciseNameRole-Qt::UserRole].replace(0, idx-1, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
				emit dataChanged(index, index, QList<int>() << exerciseName1Role);
			}
			break;
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(subrecord_separator));
				if (idx == -1)
					m_modeldata[row][exerciseNameRole-Qt::UserRole] =
						m_modeldata[row][exerciseNameRole-Qt::UserRole] += subrecord_separator + value.toString();
				else
				{
					m_modeldata[row][exerciseNameRole-Qt::UserRole].truncate(idx);
					m_modeldata[row][exerciseNameRole-Qt::UserRole].append(value.toString());
				}
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
				emit dataChanged(index, index, QList<int>() << exerciseName2Role);
			}
			break;
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).indexOf(subrecord_separator));
				if (idx == -1)
					m_modeldata[row][role-Qt::UserRole-2] = value.toString();
				else
					m_modeldata[row][role-Qt::UserRole-2].replace(0, idx, value.toString());
				emit dataChanged(index, index, QList<int>() << role-2);
				emit dataChanged(index, index, QList<int>() << role);
			}
			break;
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).indexOf(subrecord_separator));
				if (idx == -1)
					m_modeldata[row][role-Qt::UserRole-4] += subrecord_separator + value.toString();
				else
					m_modeldata[row][role-Qt::UserRole-4].replace(idx+1, m_modeldata.at(row).at(role-Qt::UserRole-4).length() - idx - 1, value.toString());
				emit dataChanged(index, index, QList<int>() << role-4);
				emit dataChanged(index, index, QList<int>() << role);
			}
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>() << role);
		setModified(true);
		return true;
	}
	return false;
}

void DBMesoSplitModel::changeExercise(const QString& name1, const QString& name2, const QString& sets,
			const QString& reps, const QString& weight, const uint operation)
{
	const QString newName(name1 + u" - "_qs + name2);
	switch (operation)
	{
		case 0: //change the single exercise
			setExerciseName(newName);
			setSetsReps(reps);
			setSetsWeight(weight);
			setSetsNumber(sets);
		break;
		case 1: //add an exercise
			switch (m_nextAddedExercisePos)
			{
				case 1:
					setExerciseName1(newName);
					setSetsReps1(reps);
					setSetsWeight1(weight);
					m_nextAddedExercisePos = 2;
				break;
				case 2:
					setExerciseName2(newName);
					setSetsReps2(reps);
					setSetsWeight2(weight);
					m_nextAddedExercisePos = 1;
				break;
			}
		break;
		case 2: //remove an exercise
		{
			if (exerciseName1().indexOf(newName) != -1)
			{
				setExerciseName(exerciseName2());
				setSetsReps(setsReps2());
				setSetsWeight1(setsWeight2());
				m_nextAddedExercisePos = 2;
			}
			else if (exerciseName2().indexOf(newName) != -1)
			{
				setExerciseName(exerciseName1());
				setSetsReps2(setsReps1());
				setSetsWeight2(setsWeight1());
				m_nextAddedExercisePos = 1;
			}
		}
		break;
	}
}
