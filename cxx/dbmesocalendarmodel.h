#ifndef DBMESOCALENDARMODEL_H
#define DBMESOCALENDARMODEL_H

#include "tplistmodel.h"

class DBMesoCalendarModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

//Q_PROPERTY(int day READ getDay CONSTANT)

public:
	// Define the role names to be used
	enum RoleNames {
		mesoCalIdRole = Qt::UserRole,
		mesoCalMesoIdRole = Qt::UserRole+1,
		mesoCalDateRole = Qt::UserRole+2,
		mesoCalNDayRole = Qt::UserRole+3,
		mesoCalSplitRole = Qt::UserRole+4,
		monthNbrRole = Qt::UserRole+5,
		yearNbrRole = Qt::UserRole+6,
		isTrainingDayRole = Qt::UserRole+7
	};

	explicit DBMesoCalendarModel(QObject *parent = 0);

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 8; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	Q_INVOKABLE void createModel(const uint mesoId, const QDate& startDate, const QDate& endDate, const QString& strSplit);

	Q_INVOKABLE int getId(const uint day) const
	{	return day < m_indexProxy.count() ? dayInfo(day).at(0).toInt() : -1;	}

	Q_INVOKABLE int getMesoId() const
	{	return count() > 0 ? dayInfo(0).at(1).toInt() : -1;	}

	Q_INVOKABLE int getTrainingDay(const uint month, const uint day) const
	{	return dayInfo(month, day).at(2).toInt();	}

	Q_INVOKABLE QLatin1Char getSplit(const uint month, const uint day) const
	{	return QLatin1Char(dayInfo(month, day).at(3).toLatin1().constData()[0]);	}

	Q_INVOKABLE uint getMonth(const uint month) const
	{	return dayInfo(month).at(5).toInt();	}

	Q_INVOKABLE uint getYear(const uint month) const
	{	return dayInfo(month).at(4).toInt();	}

	Q_INVOKABLE bool isTrainingDay(const uint month, const uint day) const
	{	return (QLatin1Char(dayInfo(month, day).at(3).toLatin1().constData()[0]) != QLatin1Char('R'));	}

private:
	inline const QStringList dayInfo(const uint month, const uint day = 0) const
	{
		return static_cast<QString>(m_modeldata.at(m_indexProxy.at(month)).at(day)).split(',');
	}
};

#endif // DBMESOCALENDARMODEL_H
