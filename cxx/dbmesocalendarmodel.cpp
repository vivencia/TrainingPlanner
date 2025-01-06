#include "dbmesocalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "tpglobals.h"
#include "tputils.h"

DBMesoCalendarModel::DBMesoCalendarModel(QObject *parent, const uint meso_idx)
	: TPListModel(parent, meso_idx)
{
	m_tableId = MESOCALENDAR_TABLE_ID;
	m_fieldCount = MESOCALENDAR_TOTAL_COLS;
	setObjectName(DBMesoCalendarObjectName);
}

void DBMesoCalendarModel::createModel()
{
	const QString& strMesoId(appMesoModel()->id(mesoIdx()));
	const QDate& startDate(appMesoModel()->startDate(mesoIdx()));
	const QDate& endDate(appMesoModel()->endDate(mesoIdx()));
	const QString& strSplit(appMesoModel()->split(mesoIdx()));
	const uint startmonth(startDate.month());
	const uint endmonth(endDate.month());
	const uint nMonths{endmonth > startmonth ? endmonth - startmonth + 1 : (12 - startmonth) + endmonth + 1};

	int splitIdx(startDate.dayOfWeek() - 1);
	uint trainingDayNumber, trainingDayNumberTotal(0);
	uint month(startmonth);
	uint year(startDate.year());
	uint firstDay(startDate.day());
	uint day;
	QStringList month_info;
	month_info.reserve(31);

	for (uint i(0); i < nMonths; i++, month++)
	{
		if (month > 12)
		{
			year++;
			month = 1;
		}

		const QString& strYear{QString::number(year)};
		const QString& strMonth{QString::number(month)};
		const uint daysInMonth(QDate(year, month, 1).daysInMonth());
		uint lastDay = i == nMonths - 1 ? endDate.day() : daysInMonth;

		for (day = 1 ; day < firstDay; ++day)
			month_info.append(std::move("-1,-1,-1,N,-1,"_L1 + strYear + ',' + strMonth));
		firstDay = 0;

		for (; day <= lastDay; day++)
		{
			if (strSplit.at(splitIdx) != QLatin1Char('R'))
			{
				++trainingDayNumberTotal;
				trainingDayNumber = trainingDayNumberTotal;
			}
			else
				trainingDayNumber = 0;

			month_info.append(std::move("-1,"_L1 + strMesoId + ',' + QString::number(trainingDayNumber) + ',' +
								strSplit.at(splitIdx) + ",0,"_L1 + strYear + ',' + strMonth));
			splitIdx++;
			if (splitIdx == strSplit.length())
				splitIdx = 0;
		}

		if (lastDay < daysInMonth)
		{
			lastDay = daysInMonth;
			for( ; day <= lastDay; ++day)
				month_info.append(std::move("-1,-1,-1,N,-1,"_L1 + strYear + ',' + strMonth));
		}
		appendList(std::move(month_info));
		month_info.clear();
	}
}

void DBMesoCalendarModel::changeModel(const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore, const QDate& endDate)
{
	if (!bPreserveOldInfo)
	{
		clear();
		createModel();
		return;
	}

	QList<QStringList> old_model_data{std::move(m_modeldata)};
	m_modeldata.clear();
	createModel();

	const QStringList& oldFirstMonthInfo{old_model_data.constFirst()};
	int old_firstday(0);
	for(; old_firstday < oldFirstMonthInfo.count(); ++old_firstday)
	{
		if (oldFirstMonthInfo.at(old_firstday).split(',').at(MESOCALENDAR_COL_TRAINING_DAY) != STR_MINUS_ONE)
			break;
	}
	int old_firstmonth(oldFirstMonthInfo.at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt());
	int old_firstyear(oldFirstMonthInfo.at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt());

	const QStringList& oldLastMonthInfo{old_model_data.constLast()};
	int old_lastday(0);
	for(; old_lastday < oldLastMonthInfo.count(); ++old_lastday)
	{
		if (oldLastMonthInfo.at(old_firstday).split(',').at(MESOCALENDAR_COL_TRAINING_DAY) != STR_MINUS_ONE)
			break;
	}
	int old_lastmonth(bPreserveOldInfoUntilDayBefore ? endDate.month() : oldLastMonthInfo.at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt());
	int old_lastyear(bPreserveOldInfoUntilDayBefore ? endDate.year() : oldLastMonthInfo.at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt());

	QDate startDate{old_firstyear, old_firstmonth, old_firstday};
	if (startDate < appMesoModel()->startDate(m_mesoIdx))
	{
		startDate = appMesoModel()->startDate(m_mesoIdx);
		old_firstday = startDate.day();
		old_firstmonth = startDate.month();
		old_firstyear = startDate.year();
	}
	QDate finalDate{old_lastyear, old_lastmonth, old_lastday};
	if (finalDate > appMesoModel()->endDate(m_mesoIdx))
	{
		finalDate = appMesoModel()->startDate(m_mesoIdx);
		old_lastday = finalDate.day();
		old_lastmonth = finalDate.month();
		old_lastyear = finalDate.year();
	}

	uint i(0);
	for(; i < m_modeldata.count(); ++i)
	{
		const QStringList& monthAndYear{m_modeldata.at(i).at(0).split(',')};
		const int new_year(monthAndYear.at(MESOCALENDAR_COL_YEAR).toInt());
		if (new_year >= old_firstyear && new_year <= old_lastyear)
		{
			const int new_month(monthAndYear.at(MESOCALENDAR_COL_MONTH).toInt());
			if (new_month >= old_firstmonth && new_month <= old_lastmonth)
				break;
		}
	}

	uint old_i(0);
	int month(-1), year(-1);
	for(; old_i < old_model_data.count(); ++old_i)
	{
		const QStringList& monthAndYear{old_model_data.at(old_i).at(0).split(',')};
		year = monthAndYear.at(MESOCALENDAR_COL_YEAR).toInt();
		if (year >= old_firstyear && year <= old_lastyear)
		{
			month = monthAndYear.at(MESOCALENDAR_COL_MONTH).toInt();
			if (month >= old_firstmonth && month <= old_lastmonth)
				break;
		}
	}

	for(; old_i < old_model_data.count(); ++old_i, ++i)
	{
		const int last_day(old_i == old_model_data.count() ? old_lastday : appUtils()->daysInMonth(month, year));
		for (; old_firstday < last_day; ++old_firstday)
			m_modeldata[i][old_firstday] = std::move(old_model_data[old_i][old_firstday]);
		month++;
		old_firstday = 0;
		if (month == 13)
		{
			month = 1;
			year++;
		}
	}
	emit calendarChanged(startDate, finalDate);
}

void DBMesoCalendarModel::updateModel(const QDate& startDate, const QString& newSplitLetter)
{
	const QString& mesoSplit(appMesoModel()->split(mesoIdx()));
	uint year(startDate.year());
	uint month(startDate.month());
	uint day(startDate.day()-1);
	int tday(-1);
	int idx(mesoSplit.lastIndexOf(newSplitLetter));

	for(uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
			{
				const QStringList& monthInfo(m_modeldata.at(i));
				QStringList dayInfo;
				for(; day < monthInfo.count(); ++day)
				{
					dayInfo = std::move(monthInfo.at(day).split(','));
					if (tday == -1)
					{
						tday = dayInfo[MESOCALENDAR_COL_TRAINING_DAY].toUInt();
						if (tday == 0 && mesoSplit.at(idx) != "R"_L1)
						{
							uint x(day-1);
							do {
								tday = getTrainingDay(month, x);
								if (tday > 0) break;
								--x;
							} while (x > 0);
						}
					}
					if (mesoSplit.at(idx) == "R"_L1)
						dayInfo[MESOCALENDAR_COL_TRAINING_DAY] = STR_ZERO;
					else
						dayInfo[MESOCALENDAR_COL_TRAINING_DAY] = std::move(QString::number(tday++));
					dayInfo[MESOCALENDAR_COL_SPLITLETTER] = mesoSplit.at(idx);
					m_modeldata[i][day] = std::move(dayInfo.join(','));
					if (++idx >= mesoSplit.length())
						idx = 0;
				}
				++month;
				day = 0;
				if (month == 13)
				{
					month = 1;
					year++;
				}
			}
		}
	}
	emit calendarChanged(startDate, appMesoModel()->endDate(mesoIdx()));
}

void DBMesoCalendarModel::updateDay(const QDate& date, const QString& tDay, const QString& splitLetter)
{
	const uint year{static_cast<uint>(date.year())};
	const uint month{static_cast<uint>(date.month())};
	const uint day{static_cast<uint>(date.day()-1)};

	for(uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
			{
				const QStringList& dayInfo{m_modeldata.at(i).at(day).split(',')};
				m_modeldata[i][day] = std::move(dayInfo.at(MESOCALENDAR_COL_ID) + ',' + dayInfo.at(MESOCALENDAR_COL_MESOID) + ',' +
						tDay + ',' + splitLetter + ',' + m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_TRAININGCOMPLETE) + ',' + dayInfo.at(MESOCALENDAR_COL_YEAR) + ',' +
						dayInfo.at(MESOCALENDAR_COL_MONTH));
			}
		}
	}
	emit calendarChanged(date, date);
}

int DBMesoCalendarModel::getTrainingDay(const uint month, const uint day) const
{
	for( uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_TRAINING_DAY).toInt();
		}
	}
	return -1;
}

QString DBMesoCalendarModel::getSplitLetter(const uint month, const uint day) const
{
	for(uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_SPLITLETTER);
		}
	}
	return "A"_L1; //avoid errors downstream
}

bool DBMesoCalendarModel::isTrainingDay(const uint month, const uint day) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_TRAINING_DAY).toUInt() > 0;
		}
	}
	return false;
}

bool DBMesoCalendarModel::isPartOfMeso(const uint month, const uint day) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_SPLITLETTER) != "N"_L1;
		}
	}
	return false;
}

bool DBMesoCalendarModel::isDayFinished(const uint month, const uint day) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_TRAININGCOMPLETE) == STR_ONE;
		}
	}
	return false;
}

void DBMesoCalendarModel::setDayIsFinished(const QDate& date, const bool bFinished)
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == date.month())
		{
			QStringList dayInfo{std::move(m_modeldata.at(i).at(date.day()-1).split(','))};
			dayInfo.replace(MESOCALENDAR_COL_TRAININGCOMPLETE, bFinished ? STR_ONE : STR_ZERO);
			m_modeldata[i].replace(date.day()-1, std::move(dayInfo.join(',')));
		}
	}
}

uint DBMesoCalendarModel::getLastTrainingDayBeforeDate(const QDate& date) const
{
	int month(date.month());
	int day(date.day());
	int tday(0);
	do {
		tday = getTrainingDay(month, day-1);
		if (tday > 0)
			return tday;
		if (--day < 0)
		{
			if (--month < 0)
				return 0;
			QDate newDate(date.year(), month, 1);
			day = newDate.daysInMonth();
		}
	} while (tday != -1);
	return 0;
}
