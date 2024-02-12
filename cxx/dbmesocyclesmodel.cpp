#include "dbmesocyclesmodel.h"

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: TPListModel(parent)
{
	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[mesoIdRole] = "mesoId";
	m_roleNames[mesoNameRole] = "mesoName";
	m_roleNames[mesoStartDateRole] = "mesoStartDate";
	m_roleNames[mesoEndDateRole] = "mesoEndDate";
	m_roleNames[mesoNoteRole] = "mesoNote";
	m_roleNames[mesoWeeksRole] = "mesoWeeks";
	m_roleNames[mesoSplitRole] = "mesoSplit";
	m_roleNames[mesoDrugsRole] = "mesoDrugs";
	m_roleNames[realMesoRole] = "realMeso";
}

QVariant DBMesocyclesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoIdRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case mesoStartDateRole:
			case mesoEndDateRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong());
			case mesoNameRole:
			case mesoNoteRole:
			case mesoWeeksRole:
			case mesoSplitRole:
			case mesoDrugsRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case realMesoRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)) == QStringLiteral("1");
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

bool DBMesocyclesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoIdRole:
			case mesoNameRole:
			case mesoStartDateRole:
			case mesoEndDateRole:
			case mesoNoteRole:
			case mesoWeeksRole:
			case mesoSplitRole:
			case mesoDrugsRole:
			case realMesoRole:
				m_modeldata[row].replace(role, value.toString());
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}
