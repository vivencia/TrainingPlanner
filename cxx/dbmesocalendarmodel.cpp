#include "dbmesocalendarmodel.h"

DBMesoCalendarModel::DBMesoCalendarModel(QObject *parent)
	: TPListModel{parent}
{
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

QVariant DBMesoCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoCalIdRole:
			case mesoCalMesoIdRole:
			case mesoCalNDayRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case mesoCalDateRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong());
			case mesoCalSplitRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case monthNbrRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong()).month() - 1;
			case yearNbrRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong()).year();
			case isTrainingDayRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)) != QStringLiteral("R");
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

bool DBMesoCalendarModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoCalIdRole:
			case mesoCalMesoIdRole:
			case mesoCalNDayRole:
			case mesoCalDateRole:
			case mesoCalSplitRole:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
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

QString DBMesoCalendarModel::getSplit(const uint month, const uint day) const
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
				return static_cast<QString>(static_cast<QStringList>(m_modeldata.at(i)).at(day)).split(',').at(2).toInt() > 0;
		}
	}
	return false;
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

/*
 * function refactoryDatabase(newStartDate, newEndDate, newSplit, bPreserveOldInfo, bPreserveOldInfoUntilToday) {
		if (bCalendarInSyncWithMeso) return; //See comment under readDatabase()

		console.log(newStartDate.toDateString() + "    " + mesoStartDate.toDateString());
		console.log(newEndDate.toDateString() + "    " + mesoEndDate.toDateString());
		console.log(newSplit + "    " + mesoSplit);

		//First, populate the Array as if starting a new calendar
		let old_monthsmodel = [];
		let monthsmodel = getMesoMonths(newStartDate, newEndDate, newSplit);
		if (bPreserveOldInfo) { //preserve old calendar info (splitLetter and trainingDay)
			old_monthsmodel = getMesoMonths(mesoStartDate, mesoEndDate, mesoSplit);
			var i = 0, x = 0, iz = 0, xz = 0;
			var startyear, startmonth, startday;
			var bFound = false;
			var bInitialPaddingAccounted = false;

			if (newStartDate <= mesoStartDate) { //If newStartDate and mesoStartDate are the same, the = sign could be in either of these conditionals
				//New meso will start earlier than the old one. Navigate through current model until we reach the old one start date
				startyear = mesoStartDate.getFullYear();
				startmonth = mesoStartDate.getMonth();
				startday = mesoStartDate.getDate();
			}
			else if (newStartDate > mesoStartDate) {
				//New meso will start later than the old one. Cannot copy old date into days that do not exis anymore. Navigate
				//through the old model until we reach the start date of the new model
				startyear = newStartDate.getFullYear();
				startmonth = newStartDate.getMonth();
				startday = newStartDate.getDate();
			}

			for(i = 0; i < monthsmodel.length && bFound === false; i++) {
				if (monthsmodel[i].yearNbr === startyear) {
					if (monthsmodel[i].monthNbr === startmonth) {
						while ( iz < monthsmodel[i].daySplit.length ) {
							if (monthsmodel[i].daySplit[iz].dayNbr === startday) {
								bFound = true;
								break;
							}
							iz++;
						}
					}
				}
			}
			if (bFound) {
				--i; //the increment happens before the conditional checking after the first iteration
				bFound = false;
			}

			//Navigate through the old info until we find the right start date. The model for the month might include padding information to fill
			//the whole month (before startday and after end day) not only the meso included dates
			for(x = 0; x < old_monthsmodel.length && bFound === false; x++) {
				if (old_monthsmodel[x].yearNbr === startyear) {
					if (old_monthsmodel[x].monthNbr === startmonth) {
						while ( xz < old_monthsmodel[x].daySplit.length ) {
							if (monthsmodel[x].daySplit[xz].dayNbr === startday) {
								bFound = true;
								break;
							}
							xz++;
						}
					}
				}
			}
			if (bFound) --x;

			var enddate;
			if (bPreserveOldInfoUntilToday) //preserve old info only until the day before the updating of the meso
				enddate = JSF.getPreviousDate(today);
			else
				enddate = mesoEndDate; //preserve all the old info.
			if (enddate > newEndDate) // The new meso might be shorter than the old and even end before today
				enddate = newEndDate;

			const endyear = enddate.getFullYear();
			const endmonth = enddate.getMonth();
			const endday = enddate.getDate();

			while (i < monthsmodel.length) { // => Start copying old information into new. i is either 0 or found in the iterations above. z too
				while ( iz < monthsmodel[i].daySplit.length ) {
					//console.log("Prev info: " + monthsmodel[i].yearNbr + "/" + monthsmodel[i].monthNbr + "/" + monthsmodel[i].daySplit[iz].dayNbr);
					//console.log(monthsmodel[i].daySplit[iz].daySplitLetter);
					monthsmodel[i].daySplit[iz].daySplitLetter = old_monthsmodel[x].daySplit[xz].daySplitLetter;
					monthsmodel[i].daySplit[iz].isTrainingDay = old_monthsmodel[x].daySplit[xz].isTrainingDay;
					monthsmodel[i].daySplit[iz].trainingDayNumber = old_monthsmodel[x].daySplit[xz].trainingDayNumber;
					//console.log("New info: " + monthsmodel[i].yearNbr + "/" + monthsmodel[i].monthNbr + "/" + monthsmodel[i].daySplit[iz].dayNbr);
					//console.log(monthsmodel[i].daySplit[iz].daySplitLetter);
					xz++;
					if (xz >= old_monthsmodel[x].daySplit.length)
						break;
					iz++;
					if (monthsmodel[i].daySplit[iz].dayNbr > endday) {
						if (monthsmodel[i].monthNbr >= endmonth) {
							if (monthsmodel[i].yearNbr >= endyear) {
								x = old_monthsmodel.length; //just to force another break below and leave the loops
								break;
							}
						}
					}
				}
				x++;
				if (x >= old_monthsmodel.length) break;
				i++;
				iz = xz = 0;
			}
		}

		Database.deleteMesoCalendar(mesoId);
		//saveModelToDatabase(monthsmodel);
		dlgProgressIndicator.months_arr = monthsmodel;
		dlgProgressIndicator.meso_id = mesoId;
		dlgProgressIndicator.open();
		dlgProgressIndicator.init("Creating database. Please wait...", 0, 50);
		mesoMonthsModel.clear();
		for (let newmonth of monthsmodel)
			mesoMonthsModel.append(newmonth);
	}
	*/
