#ifndef DBMESOCALENDARMODEL_H
#define DBMESOCALENDARMODEL_H

#include "tplistmodel.h"

class DBMesoCalendarModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:

	explicit DBMesoCalendarModel(QObject *parent = 0) : TPListModel{parent}
	{	m_tableId = MESOCALENDAR_TABLE_ID;	setObjectName(DBMesoCalendarObjectName); }

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 8; }
	Q_INVOKABLE void createModel(const uint mesoId, const QDate& startDate, const QDate& endDate, const QString& strSplit);
	void changeModel(const uint mesoId, const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday);
	void updateModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter, const QString& tDay);
	void updateDay(const QDate& date, const QString& tDay, const QString& splitLetter);

	Q_INVOKABLE int getMesoId() const
	{	return count() > 0 ? static_cast<QString>(m_modeldata.at(0).at(0)).split(',').at(1).toUInt() : -1;	}

	Q_INVOKABLE uint getMonth(const uint index) const
	{
		return index < count() ? static_cast<QString>(m_modeldata.at(index).at(0)).split(',').at(5).toUInt() - 1 : 0;
	}

	Q_INVOKABLE uint getYear(const uint index) const
	{
		return index < count() ? static_cast<QString>(m_modeldata.at(index).at(0)).split(',').at(4).toUInt() : 0;
	}

	Q_INVOKABLE uint getIndex(const QDateTime& date) const
	{
		for( uint i(0); i < m_modeldata.count(); ++i)
		{
			if (m_modeldata.at(i).at(0).split(',').at(5).toUInt() == date.date().month())
				return i;
		}
		return 0;
	}

	Q_INVOKABLE int getTrainingDay(const uint month, const uint day) const;
	Q_INVOKABLE QString getSplitLetter(const uint month, const uint day) const;
	Q_INVOKABLE bool isTrainingDay(const uint month, const uint day) const;
	Q_INVOKABLE bool isDayFinished(const uint month, const uint day) const;
	void setDayIsFinished(const QDate& date, const bool bFinished);
	Q_INVOKABLE uint getLastTrainingDayBeforeDate(const QDate& date) const;

signals:
	void calendarChanged();
};

#endif // DBMESOCALENDARMODEL_H
