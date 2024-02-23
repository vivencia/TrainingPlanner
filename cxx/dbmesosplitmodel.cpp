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
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).replace(record_separator, QStringLiteral(" + "));
			case exerciseName1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(record_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).left(idx-1) : QString();
			}
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(record_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).mid(idx+1) : QString();
			}
			case setsNumberRole:
			case setsRepsRole:
			case setsWeightRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).indexOf(record_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).left(idx-1) : QString();
			}
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).indexOf(record_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).mid(idx+1) : QString();
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
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(record_separator));
				if (idx == -1)
					m_modeldata[row][exerciseNameRole-Qt::UserRole] = value.toString();
				else
					static_cast<QString>(m_modeldata[row][exerciseNameRole-Qt::UserRole]).replace(0, idx-1, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
				emit dataChanged(index, index, QList<int>() << exerciseName1Role);
			}
			break;
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(record_separator));
				if (idx == -1)
					m_modeldata[row][exerciseNameRole-Qt::UserRole] =
						static_cast<QString>(m_modeldata[row][exerciseNameRole-Qt::UserRole]) += QString(record_separator) + value.toString();
				else
				{
					static_cast<QString>(m_modeldata[row][exerciseNameRole-Qt::UserRole]).truncate(idx);
					static_cast<QString>(m_modeldata[row][exerciseNameRole-Qt::UserRole]).append(value.toString());
				}
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
				emit dataChanged(index, index, QList<int>() << exerciseName2Role);
			}
			break;
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).indexOf(record_separator));
				if (idx == -1)
					m_modeldata[row][role-Qt::UserRole-2] =
						static_cast<QString>(m_modeldata[row][role-Qt::UserRole-2]) += QString(record_separator) + value.toString();
				else
					static_cast<QString>(m_modeldata[row][role-Qt::UserRole-2]).replace(0, idx-1, value.toString());
				emit dataChanged(index, index, QList<int>() << role-2);
				emit dataChanged(index, index, QList<int>() << role);
			}
			break;
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).indexOf(record_separator));
				if (idx == -1)
					m_modeldata[row][role-Qt::UserRole-4] =
						static_cast<QString>(m_modeldata[row][role-Qt::UserRole-4]) += QString(record_separator) + value.toString();
				else
					static_cast<QString>(m_modeldata[row][role-Qt::UserRole-4]).replace(0, idx-1, value.toString());
				emit dataChanged(index, index, QList<int>() << role-4);
				emit dataChanged(index, index, QList<int>() << role);
			}
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>() << role);
		return true;
	}
	return false;
}

void DBMesoSplitModel::changeExercise(const QString& name1, const QString& name2, const QString& sets,
			const QString& reps, const QString& weight, const uint operation)
{
	switch (operation)
	{
		case 0: //change the single exercise
			setExerciseName(name1 + QStringLiteral(" - ") + name2);
			setSetsReps(reps);
			setSetsWeight(weight);
			setSetsNumber(sets);
		break;
		case 1: //add an exercise
			switch (m_nextAddedExercisePos)
			{
				case 1:
					setExerciseName1(name1 + QStringLiteral(" - ") + name2);
					setSetsReps1(reps);
					setSetsWeight1(weight);
					m_nextAddedExercisePos = 2;
				break;
				case 2:
					setExerciseName2(name1 + QStringLiteral(" - ") + name2);
					setSetsReps2(reps);
					setSetsWeight2(weight);
					m_nextAddedExercisePos = 1;
				break;
			}
		break;
		case 2: //remove an exercise
		{
			if (exerciseName1().indexOf(name1) != -1)
			{
				setExerciseName1(QString());
				setSetsReps1(QString());
				setSetsWeight1(QString());
			}
			else if (exerciseName2().indexOf(name1) != -1)
			{
				setExerciseName2(QString());
				setSetsReps2(QString());
				setSetsWeight2(QString());
			}
		}
		break;
	}
}
