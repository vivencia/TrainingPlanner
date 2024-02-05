#include "dbexercisesmodel.h"

DBExercisesModel::DBExercisesModel(QObject *parent)
	: TPListModel(parent)
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

const QString& DBExercisesModel::data(const uint row, int role) const
{
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
				return m_modeldata.at(row).at(role-Qt::UserRole);
		}
	}
	return QStringLiteral("");
}

bool DBExercisesModel::setData(const uint row, const QString& value, int role)
{
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
				m_modeldata[row].replace(role, value);
				return true;
		}
	}
	return false;
}
