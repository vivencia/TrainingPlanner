#include "dbmesosplitmodel.h"

DBMesoSplitModel::DBMesoSplitModel(QObject *parent)
	: TPListModel{parent}
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseNameRole] = "exerciseName";
	m_roleNames[exerciseName1Role] = "exerciseName1";
	m_roleNames[exerciseName2Role] = "exerciseName2";
	m_roleNames[setTypeRole] = "setType";
	m_roleNames[setsNumberRole] = "setsNumber";
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
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).replace('&', QStringLiteral(" + "));
			case exerciseName1Role:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-7)).split('&').at(0);
			case exerciseName2Role:
			{
				if (static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-8)).indexOf('&') != -1)
					return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-8)).split('&').at(1);
				else
					return QString();
			}
			case setsNumberRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case setsReps1Role:
			case setsWeight1Role:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).split('#').at(0);
			case setsReps2Role:
			case setsWeight2Role:
			{
				if (static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).indexOf('#') != -1)
					return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-2)).split('#').at(1);
				else
					return QString();
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
				m_modeldata[role-Qt::UserRole].replace(role, value.toString());
			break;
			case exerciseName1Role:
			{
				QStringList exercises(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-7)).split('&'));
				exercises[0] = value.toString();
				m_modeldata[row][role-Qt::UserRole-7] = exercises.join('&');
			}
			break;
			case exerciseName2Role:
			{
				QStringList exercises2(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-8)).split('&'));
				exercises2[1] = value.toString();
				m_modeldata[row][role-Qt::UserRole-8] = exercises2.join('&');
			}
			break;
			case setsReps1Role:
			case setsWeight1Role:
			{
				QStringList values(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).split('#'));
				values[0] = value.toString();
				m_modeldata[row][role-Qt::UserRole] = values.join('#');
			}
			break;
			case setsReps2Role:
			case setsWeight2Role:
			{
				QStringList values2(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-1)).split('#'));
				values2[1] = value.toString();
				m_modeldata[row][role-Qt::UserRole-1] = values2.join('#');
			}
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>() << role);
		return true;
	}
	return false;
}
