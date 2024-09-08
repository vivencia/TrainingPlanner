#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "runcommands.h"

void DBMesoCalendarModel::createModel(const uint mesoId, const QDate& startDate, const QDate& endDate, const QString& strSplit)
{
	const QString mesoid( QString::number(mesoId) );
	const int startmonth( startDate.month() );
	const int endmonth( endDate.month() );
	int splitIdx(startDate.dayOfWeek() - 1);

	uint nMonths( endmonth > startmonth ? endmonth - startmonth + 1 : (12 - startmonth) + endmonth + 1 );
	uint trainingDayNumber(0), trainingDayNumberTotal(0);
	uint i(0);
	uint month( startmonth );
	uint year( startDate.year() );
	uint firstDay( startDate.day() );
	uint lastDay(0);
	uint day(1);

	QStringList month_info;
	QString strMonth, strYear;

	for (; i < nMonths; i++, month++)
	{
		if ( month > 12 ) {
			year++;
			month = 1;
		}

		strYear = QString::number(year);
		strMonth = QString::number(month);
		lastDay = i == nMonths - 1 ? endDate.day() : QDate(year, month, 1).daysInMonth();

		if (firstDay > 1)
		{
			for( ; day < firstDay; ++day)
				month_info.append(QStringLiteral("-1,-1,-1,N,-1,") + strYear + ',' + strMonth);
			firstDay = 0;
		}
		for( ; day <= lastDay; day++ ) {
			if (strSplit.at(splitIdx) != QLatin1Char('R'))
			{
				++trainingDayNumberTotal;
				trainingDayNumber = trainingDayNumberTotal;
			}
			else
				trainingDayNumber = 0;

			month_info.append(QStringLiteral("-1,") + mesoid + ',' + QString::number(trainingDayNumber) + ',' +
								strSplit.at(splitIdx) + u",0," + strYear + ',' + strMonth);
			splitIdx++;
			if (splitIdx == strSplit.length())
				splitIdx = 0;
		}
		if (i < nMonths - 1)
		{
			appendList(month_info);
			month_info.clear();
			day = 1;
		}
	}
	day = lastDay + 1;
	lastDay = QDate(year, month, 1).daysInMonth();
	if (day < lastDay)
	{
		for( ; day <= lastDay; ++day)
			month_info.append(QStringLiteral("-1,-1,-1,N,-1,") + strYear + ',' + strMonth);
		appendList(month_info);
	}
}

void DBMesoCalendarModel::changeModel(const uint mesoId, const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday)
{
	if (!bPreserveOldInfo)
	{
		clear();
		createModel(mesoId, newStartDate, newEndDate, newSplit);
		return;
	}

	uint day(0);
	for(; day < m_modeldata.at(0).count(); ++day)
	{
		if (m_modeldata.at(0).at(day).split(',').at(MESOCALENDAR_COL_TRAINING_DAY).toInt() > 0)
			break;
	}
	const uint old_firstday(day);
	const uint old_firstmonth(m_modeldata.at(0).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt());
	const uint old_firsyear(m_modeldata.at(0).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt());

	QList<QStringList> oldInfo;
	const QDate today(QDate::currentDate());
	const uint old_lastmonth(bPreserveOldInfoUntilToday ? today.month() : m_modeldata.last().at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt());
	const uint old_lastyear(bPreserveOldInfoUntilToday ? today.year() : m_modeldata.last().at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt());

	uint i(0);
	for(; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt() <= old_lastyear)
		{
			if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() <= old_lastmonth)
			{
				if (i == m_modeldata.count() - 1)
				{
					for(day = 0; day < m_modeldata.at(i).count(); ++day)
					{
						if (m_modeldata.at(i).at(day).split(',').at(MESOCALENDAR_COL_MESOID).toInt() == -1)
							break;
					}
				}
				else
					day = m_modeldata.at(i).count();

				oldInfo.append(QStringList());
				uint x(i == 0 ? old_firstday : 0);
				for(; x < day; ++x)
					oldInfo.last().append(m_modeldata.at(i).at(x));
			}
		}
	}
	const uint old_lastday(bPreserveOldInfoUntilToday ? today.day() - 1 : day);

	clear();
	createModel(mesoId, newStartDate, newEndDate, newSplit);

	uint year(0), month(0), y(0), lastday(0);
	for(i = 0; i < m_modeldata.count(); ++i)
	{
		year = m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt();
		if (year >= old_firsyear && year <= old_lastyear)
		{
			month = m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt();
			if (month >= old_firstmonth && month <= old_lastmonth)
			{
				day = y == 0 ? old_firstday : 0;
				lastday = y < oldInfo.count() - 1 ? oldInfo.at(y).count() : old_lastday;
				uint x(0);
				for(; x < lastday; ++day, ++x)
					m_modeldata[i][day] = oldInfo.at(y).at(x);
				y++;
			}
		}
	}
	emit calendarChanged();
}

void DBMesoCalendarModel::updateModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter)
{
	uint year(startDate.year());
	uint month(startDate.month());
	uint day(startDate.day()-1);
	int tday(0);
	int idx(mesoSplit.indexOf(splitLetter));

	for(uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
			{
				QStringList monthInfo(m_modeldata.at(i));
				QStringList dayInfo;
				for(; day < monthInfo.count(); ++day)
				{
					dayInfo = monthInfo.at(day).split(',');
					if (tday == 0)
					{
						if (dayInfo[MESOCALENDAR_COL_TRAINING_DAY] != u"0"_qs)
							tday = dayInfo[MESOCALENDAR_COL_TRAINING_DAY].toUInt();
						else
						{
							uint x(day-1);
							do {
								tday = getTrainingDay(month, x);
								if (tday > 0) break;
								--x;
							} while (tday > 0);
							if (tday <= 0) //will only try one month behind. If there is ever a month with no training then we have a problem
							{
								x = 30;
								do {
									tday = getTrainingDay(month, x);
									if (tday > 0) break;
									--x;
								} while (tday > 0);
							}
							if (tday < 0)
								tday = 0;
						}
					}

					if (mesoSplit.at(idx) == u"R"_qs)
						dayInfo[MESOCALENDAR_COL_TRAINING_DAY] = u"0"_qs;
					else
						dayInfo[MESOCALENDAR_COL_TRAINING_DAY] = QString::number(tday++);
					dayInfo[MESOCALENDAR_COL_SPLITLETTER] = mesoSplit.at(idx);
					m_modeldata[i][day] = dayInfo.join(',');
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
	emit calendarChanged();
}

void DBMesoCalendarModel::updateDay(const QDate& date, const QString& tDay, const QString& splitLetter, const QString& dayIsFinished)
{
	const uint year(date.year());
	const uint month(date.month());
	const uint day(date.day()-1);

	for(uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_YEAR).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
			{
				const QStringList dayInfo(m_modeldata.at(i).at(day).split(','));
				m_modeldata[i][day] = dayInfo.at(MESOCALENDAR_COL_ID) + ',' + dayInfo.at(MESOCALENDAR_COL_MESOID) + ',' +
						tDay + ',' + splitLetter + ',' + dayIsFinished + ',' + dayInfo.at(MESOCALENDAR_COL_YEAR) + ',' +
						dayInfo.at(MESOCALENDAR_COL_MONTH);
			}
		}
	}
	emit calendarChanged();
}

QString DBMesoCalendarModel::getInfoLabelText(const uint year, const uint month, const uint day) const
{
	const bool bDayOK(isPartOfMeso(month, day));
	if (bDayOK)
	{
		const QString splitLetter(getSplitLetter(month, day));
		const QDate date(year, month, day);
		if (splitLetter != u"R")
			return runCmd()->formatDate(date) + tr(": Workout #") + QString::number(getTrainingDay(month, day)) + tr(" Split: ") +
					splitLetter + u" - "_qs + m_mesoSplitModel->get(m_mesoSplitModel->currentRow(), (static_cast<int>(splitLetter.at(0).cell()) - static_cast<int>('A')) + 2);
		else
			return runCmd()->formatDate(date) + tr(": Rest day");
	}
	else
		return tr("Selected day is not part of the current mesocycle");
}

int DBMesoCalendarModel::getTrainingDay(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(MESOCALENDAR_COL_TRAINING_DAY).toInt();
		}
	}
	return -1;
}

QString DBMesoCalendarModel::getSplitLetter(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
			{
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(MESOCALENDAR_COL_SPLITLETTER);
			}
		}
	}
	return QStringLiteral("N");
}

bool DBMesoCalendarModel::isTrainingDay(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(MESOCALENDAR_COL_TRAINING_DAY).toUInt() > 0;
		}
	}
	return false;
}

bool DBMesoCalendarModel::isPartOfMeso(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(MESOCALENDAR_COL_SPLITLETTER) != u"N"_qs;
		}
	}
	return false;
}

bool DBMesoCalendarModel::isDayFinished(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(MESOCALENDAR_COL_TRAININGCOMPLETE) == u"1"_qs;
		}
	}
	return false;
}

void DBMesoCalendarModel::setDayIsFinished(const QDate& date, const bool bFinished)
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(MESOCALENDAR_COL_MONTH).toUInt() == date.month())
		{
			QStringList dayInfo(m_modeldata.at(i).at(date.day()-1).split(','));
			dayInfo.replace(MESOCALENDAR_COL_TRAININGCOMPLETE, bFinished ? u"1"_qs : u"0"_qs);
			m_modeldata[i].replace(date.day()-1, dayInfo.join(','));
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
