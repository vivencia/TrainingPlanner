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

QString DBMesoSplitModel::exerciseName1() const
{
	return static_cast<QString>(m_modeldata.at(currentRow()).at(0)).split('&').at(0);
}

void DBMesoSplitModel::setExerciseName1(const QString& new_name)
{
	QStringList exercises(static_cast<QString>(m_modeldata.at(currentRow()).at(0)).split('&'));
	exercises[0] = new_name;
	m_modeldata[currentRow()][0] = exercises.join('&');
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << exerciseNameRole);
}

QString DBMesoSplitModel::exerciseName2() const
{
	return (static_cast<QString>(m_modeldata.at(currentRow()).at(0)).indexOf('&') != -1) ?
		static_cast<QString>(m_modeldata.at(currentRow()).at(0)).split('&').at(1) : QString();
}

void DBMesoSplitModel::setExerciseName2(const QString& new_name)
{
	QStringList exercises2(static_cast<QString>(m_modeldata.at(currentRow()).at(0)).split('&'));
	exercises2[1] = new_name;
	m_modeldata[currentRow()][0] = exercises2.join('&');
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << exerciseNameRole);
}

void DBMesoSplitModel::setSetTpe(const uint new_type)
{
	m_modeldata[currentRow()][1] = QString::number(new_type);
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setTypeRole);
}

void DBMesoSplitModel::setSetsNumber(const QString& new_setsnumber)
{
	m_modeldata[currentRow()][2] = new_setsnumber;
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setsNumberRole);
}

QString DBMesoSplitModel::setsReps1() const
{
	return static_cast<QString>(m_modeldata.at(currentRow()).at(3)).split('#').at(0);
}

void DBMesoSplitModel::setSetsReps1(const QString& new_setsreps)
{
	//QStringList values(static_cast<QString>(m_modeldata.at(currentRow()).at(3)).split('#'));
	//values[0] = new_setsreps;
	//m_modeldata[currentRow()][3] = values.join('#');
	//emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setsReps1Role);
	setData(index(currentRow(), 0), new_setsreps, setsReps1Role);
}

QString DBMesoSplitModel::setsReps2() const
{
	return (static_cast<QString>(m_modeldata.at(currentRow()).at(3)).indexOf('#') != -1) ?
		static_cast<QString>(m_modeldata.at(currentRow()).at(3)).split('#').at(1) : QString();
}

void DBMesoSplitModel::setSetsReps2(const QString& new_setsreps)
{
	QStringList values2(static_cast<QString>(m_modeldata.at(currentRow()).at(3)).split('#'));
	values2[1] = new_setsreps;
	m_modeldata[currentRow()][3] = values2.join('#');
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setsReps2Role);
}

QString DBMesoSplitModel::setsWeight1() const
{
	return static_cast<QString>(m_modeldata.at(currentRow()).at(4)).split('#').at(0);
}

void DBMesoSplitModel::setSetsWeight1(const QString& new_setsweight)
{
	QStringList values(static_cast<QString>(m_modeldata.at(currentRow()).at(4)).split('#'));
	values[0] = new_setsweight;
	m_modeldata[currentRow()][4] = values.join('#');
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setsWeight1Role);
}

QString DBMesoSplitModel::setsWeight2() const
{
	return (static_cast<QString>(m_modeldata.at(currentRow()).at(4)).indexOf('#') != -1) ?
		static_cast<QString>(m_modeldata.at(currentRow()).at(4)).split('#').at(1) : QString();
}

void DBMesoSplitModel::setSetsWeight2(const QString& new_setsweight)
{
	QStringList values2(static_cast<QString>(m_modeldata.at(currentRow()).at(4)).split('#'));
	values2[1] = new_setsweight;
	m_modeldata[currentRow()][4] = values2.join('#');
	emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << setsWeight2Role);
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
				m_modeldata[row][role-Qt::UserRole] = value.toString();
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
		qDebug() << index.row() << ", " << index.column() << " - " << role;
		emit dataChanged(index, index, QList<int>() << role);
		return true;
	}
	return false;
}
