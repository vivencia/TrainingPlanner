#include "dbmesocalendarmodel.h"

DBMesoCalendarModel::DBMesoCalendarModel(QObject *parent)
	: TPListModel{parent}
{
	m_tableId = MESOCALENDAR_TABLE_ID;
	setObjectName(DBMesoCalendarObjectName);

	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[mesoCalIdRole] = "mesoCalId";
	m_roleNames[mesoCalMesoIdRole] = "mesoCalMesoId";
	m_roleNames[mesoCalDateRole] = "mesoCalDate";
	m_roleNames[mesoCalNDayRole] = "mesoCalNDay";
	m_roleNames[mesoCalSplitRole] = "mesoCalSplit";
	m_roleNames[monthNbrRole] = "monthNbr";
	m_roleNames[yearNbrRole] = "yearNbr";
	m_roleNames[isTrainingDayRole] = "isTrainingDay";
}

void DBMesoCalendarModel::createModel(const uint mesoId, const QDate& startDate, const QDate& endDate, const QString& strSplit)
{
	const QString mesoid( QString::number(mesoId) );
	const int startmonth( startDate.month() );
	const int endmonth( endDate.month() );

	uint nMonths( endmonth > startmonth ? endmonth - startmonth + 1 : (12 - startmonth) + endmonth + 1 );

	uint trainingDayNumber(0), trainingDayNumberTotal(0);
	int splitIdx(0);
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
				month_info.append(QStringLiteral("-1,-1,-1,'N',") + strYear + ',' + strMonth);
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
								strSplit.at(splitIdx) + ',' +	strYear + ',' + strMonth);
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
			month_info.append(QStringLiteral("-1,-1,-1,'N',") + strYear + ',' + strMonth);
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
		if (m_modeldata.at(0).at(day).split(',').at(2).toInt() > 0)
			break;
	}
	const uint old_firstday(day);
	const uint old_firstmonth(m_modeldata.at(0).at(0).split(',').at(5).toUInt());
	const uint old_firsyear(m_modeldata.at(0).at(0).split(',').at(4).toUInt());

	QList<QStringList> oldInfo;
	const QDate today(QDate::currentDate());
	const uint old_lastmonth(bPreserveOldInfoUntilToday ? today.month() : m_modeldata.last().at(0).split(',').at(5).toUInt());
	const uint old_lastyear(bPreserveOldInfoUntilToday ? today.year() : m_modeldata.last().at(0).split(',').at(4).toUInt());

	uint i(0);
	for(; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(4).toUInt() <= old_lastyear)
		{
			if (m_modeldata.at(i).at(0).split(',').at(5).toUInt() <= old_lastmonth)
			{
				if (i == m_modeldata.count() - 1)
				{
					for(day = 0; day < m_modeldata.at(i).count(); ++day)
					{
						if (m_modeldata.at(i).at(day).split(',').at(1).toInt() == -1)
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
		year = m_modeldata.at(i).at(0).split(',').at(4).toUInt();
		if (year >= old_firsyear && year <= old_lastyear)
		{
			month = m_modeldata.at(i).at(0).split(',').at(5).toUInt();
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

void DBMesoCalendarModel::updateModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter, const QString& tDay)
{
	const uint year(startDate.year());
	const uint month(startDate.month());
	uint day(startDate.day()-1);
	int idx(mesoSplit.indexOf(splitLetter));
	uint tday(tDay.toUInt());
	if (tday == 0)
		tday = getLastTrainingDayBeforeDate(startDate);

	for(uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(4).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(5).toUInt() == month)
			{
				QStringList monthInfo(m_modeldata.at(i));
				QStringList dayInfo;
				do {
					for(; day < monthInfo.count(); ++day)
					{
						dayInfo = monthInfo.at(day).split(',');
						if (mesoSplit.at(idx) == u"R"_qs)
							dayInfo[2] = u"0"_qs;
						else
							dayInfo[2] = QString::number(tday++);
						dayInfo[3] = mesoSplit.at(idx);
						m_modeldata[i][day] = dayInfo.join(',');
						if (++idx >= mesoSplit.length())
							idx = 0;
					}
				} while(++i < m_modeldata.count());
				break;
			}
		}
	}
	emit calendarChanged();
}

void DBMesoCalendarModel::updateDay(const QDate& date, const QString& tDay, const QString& splitLetter)
{
	const uint year(date.year());
	const uint month(date.month());
	const uint day(date.day()-1);

	for(uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(0).split(',').at(4).toUInt() == year)
		{
			if (m_modeldata.at(i).at(0).split(',').at(5).toUInt() == month)
			{
				const QStringList dayInfo(m_modeldata.at(i).at(day).split(','));
				m_modeldata[i][day] = dayInfo.at(0) + ',' + dayInfo.at(1) + ',' + tDay + ',' + splitLetter + ',' + dayInfo.at(4) + ',' + dayInfo.at(5);
			}
		}
	}
	emit calendarChanged();
}

void DBMesoCalendarModel::updateFromModel(TPListModel* model)
{

}

int DBMesoCalendarModel::getTrainingDay(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(5).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(2).toInt();
		}
	}
	return -1;
}

QString DBMesoCalendarModel::getSplitLetter(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(5).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
			{
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(3);
			}
		}
	}
	return QStringLiteral("N");
}

bool DBMesoCalendarModel::isTrainingDay(const uint month, const uint day) const
{
	for( uint i(0); i < m_modeldata.count(); ++i)
	{
		if ( static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(0)).split(',').at(5).toUInt() == month)
		{
			if (day < m_modeldata.at(i).count())
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(3) != u"'N'"_qs;
		}
	}
	return false;
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
