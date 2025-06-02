#include "tpworkoutscalendar.h"

#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendartable.h"

TPWorkoutsCalendar::dayInfo TPWorkoutsCalendar::noDataDay;
st_workoutDayInfo* TPWorkoutsCalendar::noWorkoutDay{new st_workoutDayInfo{}};

TPWorkoutsCalendar::~TPWorkoutsCalendar()
{
	for (uint i{0}; i < m_monthsList.count(); ++i)
		qDeleteAll(m_monthsList.at(i));
}

void TPWorkoutsCalendar::scanMesocycles(const uint startMesoIdx)
{
	if (appMesoModel()->count() == 0)
		return;

	for (uint i{startMesoIdx}; i < appMesoModel()->count(); ++i)
	{
		const QDate& startdate{appMesoModel()->startDate(i)};
		const QDate& enddate{appMesoModel()->endDate(i)};
		const uint startmonth{static_cast<uint>(startdate.month())};
		const uint endmonth{static_cast<uint>(enddate.month())};
		const uint nMonths{endmonth > startmonth ? endmonth - startmonth + 1 : (12 - startmonth) + endmonth + 1};

		if (i == 0)
			setInitialDate(startdate);
		if (i == appMesoModel()->count() - 1)
			setFinalDate(enddate);

		QList<dayInfo*> month_info;
		uint month{startmonth};
		uint year{static_cast<uint>(startdate.year())};
		uint firstDay{static_cast<uint>(startdate.day())};
		uint day;
		month_info.reserve(31);
		for (uint x{0}; x < nMonths; x++, month++)
		{
			if (month > 12)
			{
				year++;
				month = 1;
			}

			const uint daysInMonth{static_cast<uint>(QDate(year, month, 1).daysInMonth())};
			uint lastDay = i == nMonths - 1 ? enddate.day() : daysInMonth;

			for (day = 1 ; day < firstDay; ++day)
				month_info.append(new dayInfo{}); //not part of the meso currently in the iteration

			for (; day <= lastDay; day++)
			{
				dayInfo* day_info = new dayInfo{};
				day_info->meso_id = appMesoModel()->_id(i);
				day_info->meso_idx = i;
				day_info->date = std::move(QDate(year, month, day));
				month_info.append(day_info);
			}

			if (lastDay < daysInMonth)
			{
				lastDay = daysInMonth;
				for( ; day <= lastDay; ++day)
					month_info.append(new dayInfo{}); //not part of the meso currently in the iteration
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
	for (uint i{0}; i < m_monthsList.count(); ++i)
		if (m_monthsList.at(i).at(m_monthsList.at(i).at(0)->start_day)->date.month() == month)
			return i;

	return -1;
}

void TPWorkoutsCalendar::viewSelectedWorkout()
{
	appMesoModel()->openSpecificWorkout(m_selectedDay->meso_idx, m_selectedDay->date);
}

QString TPWorkoutsCalendar::mesoName() const
{
	return m_selectedDay->meso_idx >= 0 ? appMesoModel()->name(m_selectedDay->meso_idx) : tr("Day is not part of any program");
}

QString TPWorkoutsCalendar::trainingDay() const
{
	return m_selectedDayWorkoutInfo->trainingDay;
}

QString TPWorkoutsCalendar::splitLetter() const
{
	return m_selectedDayWorkoutInfo->splitLetter;
}

bool TPWorkoutsCalendar::workoutCompleted() const
{
	return m_selectedDayWorkoutInfo->completed;
}

void TPWorkoutsCalendar::reScanMesocycles(const uint meso_idx, const uint extra_info)
{
	if (extra_info == 9999)
		scanMesocycles(appMesoModel()->count() - 1); //new meso, only append the new information
	else
	{
		beginRemoveRows(QModelIndex(), 0, count()-1);
		for (uint i{0}; i < m_monthsList.count(); ++i)
			qDeleteAll(m_monthsList.at(i));
		emit countChanged();
		endRemoveRows();
		scanMesocycles(0);
	}
}

QVariant TPWorkoutsCalendar::data(const QModelIndex& index, int role) const
{
	const int row{index.row()};
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
	return QVariant{};
}

void TPWorkoutsCalendar::findDateInList()
{
	m_selectedDay = &TPWorkoutsCalendar::noDataDay;
	for (uint i{0}; i < m_monthsList.count(); ++i)
	{
		const QDate &initDate{m_monthsList.at(i).at(m_monthsList.at(i).at(0)->start_day)->date};
		if (initDate.year() == m_selectedDate.year() && initDate.month() == m_selectedDate.month())
		{
			const QList<dayInfo*> &selectedMonth{m_monthsList.at(i)};
			for (uint day{0}; day < selectedMonth.count(); ++day)
			{
				if (selectedMonth.at(day)->date == m_selectedDate)
				{
					m_selectedDay = selectedMonth.at(day);
					break;
				}
			}
		}
		if (m_selectedDay != &TPWorkoutsCalendar::noDataDay)
			break;
	}
	emit selectedDateChanged();
	getWorkoutInfo();
}

void TPWorkoutsCalendar::getWorkoutInfo()
{
	if (m_selectedDay->meso_idx == -1)
	{
		m_selectedDayWorkoutInfo = noWorkoutDay;
		emit workoutChanged();
		setCanViewWorkout(false);
		return;
	}
	else if (m_workoutInfoList.isEmpty() || m_selectedDay->meso_id != m_workoutInfoList.at(0)->meso_id)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_idx, QVariant data) {
			if (table_idx == MESOCALENDAR_TABLE_ID)
			{
				disconnect(*conn);
				m_workoutInfoList = std::move(data.value<QList<st_workoutDayInfo*>>());
				findWorkoutInList();
			}
		});
		//appDBInterface()->getWorkoutDayInfoForAllWorkouts(m_selectedDay->meso_id);
		return;
	}
	findWorkoutInList();
}

void TPWorkoutsCalendar::findWorkoutInList()
{
	m_selectedDayWorkoutInfo = noWorkoutDay;
	for (uint i{0}; i < m_workoutInfoList.count(); ++i)
	{
		if (m_selectedDay->date == m_workoutInfoList.at(i)->date)
		{
			m_selectedDayWorkoutInfo = m_workoutInfoList.at(i);
			break;
		}
	}
	setCanViewWorkout(m_selectedDayWorkoutInfo != noWorkoutDay && m_selectedDayWorkoutInfo->splitLetter != "R"_L1);
	emit workoutChanged();
}
