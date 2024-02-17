#include "dbmesosplitmodel.h"

/*DBMesoSplitModel::DBMesoSplitModel(QObject *parent)
	: TPListModel{parent}
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[mesoSplitIdRole] = "mesoSplitId";
	m_roleNames[mesoSplitMesoIdRole] = "mesoSplitMesoId";
	m_roleNames[splitARole] = "splitA";
	m_roleNames[splitBRole] = "splitB";
	m_roleNames[splitCRole] = "splitC";
	m_roleNames[splitDRole] = "splitD";
	m_roleNames[splitERole] = "splitE";
	m_roleNames[splitFRole] = "splitF";
}

QVariant DBMesoSplitModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoSplitIdRole:
			case mesoSplitMesoIdRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case splitARole:
			case splitBRole:
			case splitCRole:
			case splitDRole:
			case splitERole:
			case splitFRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
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
			case mesoSplitIdRole:
			case mesoSplitMesoIdRole:
			case splitARole:
			case splitBRole:
			case splitCRole:
			case splitDRole:
			case splitERole:
			case splitFRole:
				m_modeldata[row].replace(role, value.toString());
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}*/

void DBMesoSplitModel::setWorkingSplit(const uint mesoIdx, QLatin1Char splitLetter)
{
	m_currentIndex = mesoIdx;
	switch (splitLetter)
	{
		case 'A': m_fldExercises = 8; break;
		case 'B': m_fldExercises = 13; break;
		case 'C': m_fldExercises = 18; break;
		case 'D': m_fldExercises = 23; break;
		case 'E': m_fldExercises = 28; break;
		case 'F': m_fldExercises = 33; break;
	}
}

void DBMesoSplitModel::replaceString(QString& oldString, const QString& newString, const uint n, QLatin1Char sep)
{
	uint idx(0);
	uint next_idx(0);
	uint count(0);
	while (count < n)
	{
		idx = oldString.indexOf(sep, idx, Qt::CaseInsensitive );
		count++;
		idx++;
	}
	next_idx = oldString.indexOf(sep, idx, Qt::CaseInsensitive );
	if (next_idx < 0)
		next_idx = oldString.length();
	oldString.replace(idx, next_idx - idx, newString);
}
