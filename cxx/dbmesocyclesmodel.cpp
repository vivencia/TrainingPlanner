#include "dbmesocyclesmodel.h"

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: TPListModel(parent)
{
	m_tableId = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);

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

void DBMesocyclesModel::updateFromModel(TPListModel* model)
{
	if (model->count() > 0)
	{
		appendList(model->m_modeldata.at(0));
		setCurrentRow(count() -1);
	}
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
				m_modeldata[row][role-Qt::UserRole] = value.toString();
				setModified(true);
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}

QVariant DBMesocyclesModel::getMesoInfo(const int mesoid, const int role) const
{
	if (mesoid >= 0)
	{
		for(uint x(0); x < count(); ++x)
		{
			if (static_cast<QString>(m_modeldata.at(x).at(0)).toInt() == mesoid)
				return data(index(x), role);
		}
	}
	return QVariant();
}

int DBMesocyclesModel::getPreviousMesoId(const int current_mesoid) const
{
	if (current_mesoid >= 0)
	{
		for(uint x(0); x < count(); ++x)
		{
			if (static_cast<QString>(m_modeldata.at(x).at(0)).toInt() == current_mesoid)
			{
				if (x > 0)
					return static_cast<QString>(m_modeldata.at(x-1).at(0)).toInt();
			}
		}
	}
	return -1;
}

QDate DBMesocyclesModel::getPreviousMesoEndDate(const int current_mesoid) const
{
	if (current_mesoid >= 0)
	{
		for(uint x(0); x < count(); ++x)
		{
			if (static_cast<QString>(m_modeldata.at(x).at(0)).toInt() == current_mesoid)
			{
				if (x > 0)
					return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(x-1).at(3)).toLongLong());
			}
		}
	}
	// 1 or 0 meso records = no previous meso. The first meso can start anywhere in 2024
	return QDate(2024, 1, 1);
}

QDate DBMesocyclesModel::getNextMesoStartDate(const int mesoid) const
{
	if (mesoid >= 0)
	{
		for(uint x(0); x < count(); ++x)
		{
			if (static_cast<QString>(m_modeldata.at(x).at(0)).toInt() == mesoid)
			{
				if (x + 1 < count())
					return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(x+1).at(2)).toLongLong());
			}
		}
	}
	 //This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
	return QDate::currentDate().addMonths(6);
}

QDate DBMesocyclesModel::getLastMesoEndDate() const
{
	if ( count() > 0)
		return QDate::fromJulianDay(static_cast<QString>(m_modeldata.last().at(3)).toLongLong());
	return QDate::currentDate();
}
