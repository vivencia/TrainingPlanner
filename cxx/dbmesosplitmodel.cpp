#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"

DBMesoSplitModel::DBMesoSplitModel(QObject *parent)
	: TPListModel{parent}, m_nextAddedExercisePos(2)
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseNameRole] = "exerciseName";
	m_roleNames[exerciseName1Role] = "exerciseName1";
	m_roleNames[exerciseName2Role] = "exerciseName2";
	m_roleNames[setTypeRole] = "setType";
	m_roleNames[setsNumberRole] = "setsNumber";
	m_roleNames[setsSubsetsRole] = "setsSubsets";
	m_roleNames[setsRepsRole] = "setsReps";
	m_roleNames[setsWeightRole] = "setsWeight";
	m_roleNames[setsReps1Role] = "setsReps1";
	m_roleNames[setsWeight1Role] = "setsWeight1";
	m_roleNames[setsReps2Role] = "setsReps2";
	m_roleNames[setsWeight2Role] = "setsWeight2";
	m_roleNames[setsNotesRole] = "setsNotes";
}

void DBMesoSplitModel::convertFromTDayModel(DBTrainingDayModel* tDayModel)
{
	m_modeldata.clear();
	m_indexProxy.clear();
	QStringList exerciseInfo;
	QString repsOrweight;
	for (uint i(0); i < tDayModel->m_ExerciseData.count(); ++i)
	{
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->name);
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->type.at(0));
		exerciseInfo.append(QString::number(tDayModel->m_ExerciseData.at(i)->nsets));
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->subsets.at(0));

		//DBTrainingDayModel can handle composite sets that end with subrecord_separator. DBMesoSplitModel cannot
		repsOrweight = tDayModel->m_ExerciseData.at(i)->reps.at(0);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);
		exerciseInfo.append(repsOrweight);
		repsOrweight = tDayModel->m_ExerciseData.at(i)->weight.at(0);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);

		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->notes.at(0));
		exerciseInfo.append(repsOrweight);
		m_modeldata.append(exerciseInfo);
		m_indexProxy.append(i);
		exerciseInfo.clear();
	}
	setReady(true);
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
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).left(idx) : QString();
			}
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(exerciseNameRole-Qt::UserRole)).sliced(idx+1) : QString();
			}
			case setsNumberRole:
			case setsSubsetsRole:
			case setsRepsRole:
			case setsWeightRole:
			case setsNotesRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-3)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-3)).left(idx) : m_modeldata.at(row).at(role-Qt::UserRole-3);
			}
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-5)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-5)).sliced(idx+1) : m_modeldata.at(row).at(role-Qt::UserRole-5);
			}
			case setTypeRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value)
{
	const int idx(static_cast<QString>(m_modeldata.at(row).at(column)).indexOf(subrecord_separator));
	if (idx == -1)
		m_modeldata[row][column] = pos == 1 ? value : m_modeldata[row][column] += subrecord_separator + value;
	else
	{
		if (pos == 1)
			m_modeldata[row][column].replace(0, idx, value);
		else
		{
			m_modeldata[row][column].truncate(idx+1);
			m_modeldata[row][column].append(value);
		}
	}
}

bool DBMesoSplitModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case exerciseNameRole:
			case setsRepsRole:
			case setsWeightRole:
			case setsNumberRole:
			case setTypeRole:
			case setsSubsetsRole:
			case setsNotesRole:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
			break;
			case exerciseName1Role:
				replaceCompositeValue(row, 0, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
			break;
			case exerciseName2Role:
				replaceCompositeValue(row, 0, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
			break;
			case setsReps1Role:
				replaceCompositeValue(row, 4, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << setsRepsRole);
			case setsReps2Role:
				replaceCompositeValue(row, 4, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << setsRepsRole);
			break;
			case setsWeight1Role:
				replaceCompositeValue(row, 5, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << setsWeightRole);
			case setsWeight2Role:
				replaceCompositeValue(row, 5, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << setsWeightRole);
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>() << role);
		setModified(true);
		return true;
	}
	return false;
}

void DBMesoSplitModel::changeExercise(const QString& name, const QString& sets, const QString& reps,
						const QString& weight, const uint operation)
{
	switch (operation)
	{
		case 0: //change the single exercise
			setExerciseName(name);
			setSetsReps(reps);
			setSetsWeight(weight);
			setSetsNumber(sets);
		break;
		case 1: //add an exercise
			switch (m_nextAddedExercisePos)
			{
				case 1:
					setExerciseName1(name);
					setSetsReps1(reps);
					setSetsWeight1(weight);
					m_nextAddedExercisePos = 2;
				break;
				case 2:
					setExerciseName2(name);
					setSetsReps2(reps);
					setSetsWeight2(weight);
					m_nextAddedExercisePos = 1;
				break;
			}
		break;
		case 2: //remove an exercise
		{
			if (exerciseName1().indexOf(name) != -1)
			{
				setExerciseName(exerciseName2());
				setSetsReps(setsReps2());
				setSetsWeight1(setsWeight2());
				m_nextAddedExercisePos = 2;
			}
			else if (exerciseName2().indexOf(name) != -1)
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
