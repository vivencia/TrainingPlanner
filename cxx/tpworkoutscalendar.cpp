#include "tpworkoutscalendar.h"

#include "dbmesocyclesmodel.h"

TPWorkoutsCalendar::dayInfo TPWorkoutsCalendar::noDataDay;

TPWorkoutsCalendar::~TPWorkoutsCalendar()
{
	for (uint i(0); i < m_monthsList.count(); ++i)
		qDeleteAll(m_monthsList.at(i));
}

void TPWorkoutsCalendar::scanMesocycles(const uint startMesoIdx)
{
	if (appMesoModel()->count() == 0)
		return;

	for (uint i(startMesoIdx); i < appMesoModel()->count(); ++i)
	{
		const QDate& startdate(appMesoModel()->startDate(i));
		const QDate& enddate(appMesoModel()->endDate(i));
		const uint startmonth(startdate.month());
		const uint endmonth(enddate.month());
		const uint nMonths{endmonth > startmonth ? endmonth - startmonth + 1 : (12 - startmonth) + endmonth + 1};

		if (i == 0)
			setInitialDate(startdate);
		if (i == appMesoModel()->count() - 1)
			setFinalDate(enddate);

		QList<dayInfo*> month_info;
		uint month(startmonth);
		uint year(startdate.year());
		uint firstDay(startdate.day());
		uint day;
		month_info.reserve(31);
		for (uint x(0); x < nMonths; x++, month++)
		{
			if (month > 12)
			{
				year++;
				month = 1;
			}

			const uint daysInMonth(QDate(year, month, 1).daysInMonth());
			uint lastDay = i == nMonths - 1 ? enddate.day() : daysInMonth;

			for (day = 1 ; day < firstDay; ++day)
				month_info.append(new dayInfo()); //not part of the meso currently in the iteration

			for (; day <= lastDay; day++)
			{
				dayInfo* day_info = new dayInfo();
				day_info->meso_id = appMesoModel()->_id(i);
				day_info->meso_idx = i;
				day_info->date = std::move(QDate(year, month, day));
				month_info.append(day_info);
			}

			if (lastDay < daysInMonth)
			{
				lastDay = daysInMonth;
				for( ; day <= lastDay; ++day)
					month_info.append(new dayInfo()); //not part of the meso currently in the iteration
			}
			month_info.at(0)->start_day = firstDay;
			firstDay = 0;
			beginInsertRows(QModelIndex(), count(), count());
			m_monthsList.append(std::move(month_info));
			emit countChanged();
			endInsertRows();
			month_info.clear();
		}
	}
	m_bReady = true;
	emit readyChanged();
}

int TPWorkoutsCalendar::indexOf(const QDate& date) const
{
	const int month{date.month()};
	for (uint i(0); i < m_monthsList.count(); ++i)
		if (m_monthsList.at(i).at(m_monthsList.at(i).at(0)->start_day)->date.month() == month)
			return i;

	return -1;
}

QString TPWorkoutsCalendar::mesoName() const
{
	return m_selectedDay->meso_idx >= 0 ? appMesoModel()->name(m_selectedDay->meso_idx) : tr("Day is not part of any program");
}

void TPWorkoutsCalendar::reScanMesocycles(const uint meso_idx, const uint extra_info)
{
	if (extra_info == 9999)
		scanMesocycles(appMesoModel()->count() - 1); //new meso, only append the new information
	else
	{
		beginRemoveRows(QModelIndex(), 0, count()-1);
		for (uint i(0); i < m_monthsList.count(); ++i)
			qDeleteAll(m_monthsList.at(i));
		emit countChanged();
		endRemoveRows();
		scanMesocycles(0);
	}
}

QVariant TPWorkoutsCalendar::data(const QModelIndex& index, int role) const
{
	const int row(index.row());
	if (row >= 0 && row < m_monthsList.count())
	{
		switch (role)
		{
			case yearRole:
				return m_monthsList.at(row).at(m_monthsList.at(row).at(0)->start_day)->date.year();
			break;
			case monthRole:
				return m_monthsList.at(row).at(m_monthsList.at(row).at(0)->start_day)->date.month() - 1;
			break;
		}
	}
	return QVariant();
}

void TPWorkoutsCalendar::findDateInList()
{
	for (uint i(0); i < m_monthsList.count(); ++i)
	{
		const QDate& initDate{m_monthsList.at(i).at(m_monthsList.at(i).at(0)->start_day)->date};
		if (initDate.year() == m_selectedDate.year() && initDate.month() == m_selectedDate.month())
		{
			const QList<dayInfo*>& selectedMonth = m_monthsList.at(i);
			for (uint day(0); day < selectedMonth.count(); ++day)
			{
				if (selectedMonth.at(day)->date == m_selectedDate)
				{
					m_selectedDay = selectedMonth.at(day);
					return;
				}
			}
		}
	}
	m_selectedDay = &TPWorkoutsCalendar::noDataDay;
}
