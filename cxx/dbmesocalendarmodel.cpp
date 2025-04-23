#include "dbmesocalendarmodel.h"

#include "dbcalendarmodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbworkoutmodel.h"
#include "tpglobals.h"
#include "tputils.h"

void DBMesoCalendarModel::createCalendar(const uint meso_idx)
{
	QDate startDate{std::move(appMesoModel()->startDate(meso_idx))};
	const QDate &endDate{appMesoModel()->endDate(meso_idx)};
	const QString &split{appMesoModel()->split(meso_idx)};
	const uint n_days{static_cast<uint>(startDate.daysTo(endDate))+1};
	m_dayInfoList[meso_idx].reserve(n_days);
	QString::const_iterator splitletter{split.constBegin()};

	for (uint i{0}; i < n_days; ++i)
	{
		stDayInfo *dayinfo{new stDayInfo{}};
		dayinfo->date = appUtils()->formatDate(startDate, TPUtils::DF_DATABASE);
		dayinfo->data = std::move(appUtils()->string_strings({appMesoModel()->id(meso_idx), STR_MINUS_ONE, dayinfo->date,
			QString::number(i+1), *splitletter, QString{}, QString{}, QString{}, QString{}, STR_ZERO}, record_separator));
		m_dayInfoList[meso_idx].append(dayinfo);

		startDate = std::move(startDate.addDays(1));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
	}
	const uint n_months{appUtils()->calculateNumberOfMonths(startDate, endDate)};
	m_calendars.at(meso_idx)->setNMonths(n_months);
	setDBDataReady(meso_idx, true, true);
}

std::optional<QString> DBMesoCalendarModel::dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const
{
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day < m_dayInfoList.at(meso_idx).count())
			return  appUtils()->getCompositeValue(field, m_dayInfoList.at(meso_idx).at(calendar_day)->data, record_separator);
	}
	return std::nullopt;
}

void DBMesoCalendarModel::setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value)
{
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day < m_dayInfoList.at(meso_idx).count())
		{
			stDayInfo *dayinfo{m_dayInfoList.at(meso_idx).at(calendar_day)};
			appUtils()->setCompositeValue(field, new_value, dayinfo->data, record_separator);
			dayinfo->modified = true;
			emit calendarChanged(meso_idx, calendar_day, field);
		}
	}
}

void DBMesoCalendarModel::removeCalendarForMeso(const uint meso_idx)
{
	appDBInterface()->removeMesoCalendar(meso_idx);
	appDBInterface()->removeWorkoutsForMeso(meso_idx);
	delete m_calendars.at(meso_idx);
	m_calendars.remove(meso_idx);
	delete m_workouts.at(meso_idx);
	m_workouts.remove(meso_idx);
	m_dbDataReady.remove(meso_idx);
	for (uint i{meso_idx}; i < m_calendars.count(); ++i)
	{
		m_calendars.at(i)->setMesoIdx(i);
		m_workouts.at(i)->setMesoIdx(i);
	}
	qDeleteAll(m_dayInfoList.at(meso_idx));
	m_dayInfoList.remove(meso_idx);
}

void DBMesoCalendarModel::addCalendarForMeso(const uint meso_idx)
{
	uint n_days{0};
	if (appMesoModel()->_id(meso_idx) >= 0)
		n_days = static_cast<uint>(appMesoModel()->startDate(meso_idx).daysTo(appMesoModel()->endDate(meso_idx)) + 1);
	m_dayInfoList.append(QList<stDayInfo*>{n_days});
	m_calendars.append(new DBCalendarModel{this, meso_idx});
	m_workouts.append(new DBWorkoutModel{this, meso_idx});
}

void DBMesoCalendarModel::addNewCalendarForMeso(const uint new_mesoidx)
{
	addCalendarForMeso(new_mesoidx);
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, this, [this,conn,new_mesoidx] (const uint meso_idx) {
		if (meso_idx == new_mesoidx)
		{
			disconnect(*conn);
			createCalendar(meso_idx);
		}
	});
}

void DBMesoCalendarModel::remakeMesoCalendar(const uint meso_idx, const bool preserve_old_info)
{
	if (!hasDBData(meso_idx))
	{
		createCalendar(meso_idx);
		return;
	}

	if (!preserve_old_info)
	{
		removeCalendarForMeso(meso_idx);
		createCalendar(meso_idx);
		return;
	}

	QDate startDate{std::move(appMesoModel()->startDate(meso_idx))};
	QDate endDate{std::move(date(meso_idx, 0).value())};
	if (startDate < endDate)
	{
		uint n_days{static_cast<uint>(startDate.daysTo(endDate))};

		endDate = std::move(QDate::currentDate());

	}

	endDate = std::move(appMesoModel()->endDate(meso_idx));


	const QString &split{appMesoModel()->split(meso_idx)};
	const uint n_days{static_cast<uint>(startDate.daysTo(endDate))+1};
	m_dayInfoList[meso_idx].reserve(n_days);
	QString::const_iterator splitletter{split.constBegin()};

	for (uint i{0}; i < n_days; ++i)
	{
		stDayInfo *dayinfo{new stDayInfo{}};
		dayinfo->date = appUtils()->formatDate(startDate, TPUtils::DF_DATABASE);
		dayinfo->data = std::move(appUtils()->string_strings({appMesoModel()->id(meso_idx), STR_MINUS_ONE, dayinfo->date,
			QString::number(i+1), *splitletter, QString{}, QString{}, QString{}, QString{}, STR_ZERO}, record_separator));
		m_dayInfoList[meso_idx].append(dayinfo);

		startDate = std::move(startDate.addDays(1));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
	}
	const uint n_months{appUtils()->calculateNumberOfMonths(startDate, endDate)};
	m_calendars.at(meso_idx)->setNMonths(n_months);
	setDBDataReady(meso_idx, true, true);
}

const int DBMesoCalendarModel::calendarDay(const uint meso_idx, const QDate& date) const
{
	if (meso_idx < m_dayInfoList.count())
	{
		QDate calendarDate{std::move(appMesoModel()->startDate(meso_idx))};
		const int calendar_day{static_cast<int>(calendarDate.daysTo(date))};
		if (calendar_day >= 0 && calendar_day < m_dayInfoList.at(meso_idx).count())
			return calendar_day;
	}
	return -1;
}

const std::optional<QString> DBMesoCalendarModel::mesoId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_MESOID);
}

void DBMesoCalendarModel::setMesoId(const uint meso_idx, const uint calendar_day, const QString &new_meso_id)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_MESOID, new_meso_id);
}

const std::optional<QString> DBMesoCalendarModel::workoutId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID);
}

void DBMesoCalendarModel::setWorkoutId(const uint meso_idx, const uint calendar_day, const QString &new_workout_id)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID, new_workout_id);
}

const std::optional<QDate> DBMesoCalendarModel::date(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE))};
	if (value.has_value())
		return appUtils()->getDateFromDateString(value.value(), TPUtils::DF_DATABASE);
	return std::nullopt;
}

const std::optional<QDate> DBMesoCalendarModel::nThDate(const uint meso_idx, const uint nth_month) const
{
	std::optional<QDate> initialDate{std::move(date(meso_idx, 0))};
	if (initialDate.has_value())
		return initialDate.value().addMonths(nth_month);
	return initialDate;
}

void DBMesoCalendarModel::setDate(const uint meso_idx, const uint calendar_day, const QDate &new_date)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE, appUtils()->formatDate(new_date, TPUtils::DF_DATABASE));
}

const std::optional<QString> DBMesoCalendarModel::workoutNumber(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER);
}

void DBMesoCalendarModel::setWorkoutNumber(const uint meso_idx, const uint calendar_day, const QString &new_number)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER, new_number);
}

const std::optional<QString> DBMesoCalendarModel::splitLetter(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER);
}

void DBMesoCalendarModel::setSplitLetter(const uint meso_idx, const uint calendar_day, const QString &new_splitletter)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER, new_splitletter);
}

const std::optional<QTime> DBMesoCalendarModel::timeIn(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN))};
	if (value.has_value())
		return appUtils()->getTimeFromTimeString(value.value(), TPUtils::TF_QML_DISPLAY_NO_SEC);
	return std::nullopt;
}

void DBMesoCalendarModel::setTimeIn(const uint meso_idx, const uint calendar_day, const QTime &new_timein)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, appUtils()->formatTime(new_timein, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

const std::optional<QTime> DBMesoCalendarModel::timeOut(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEOUT))};
	if (value.has_value())
		return appUtils()->getTimeFromTimeString(value.value(), TPUtils::TF_QML_DISPLAY_NO_SEC);
	return std::nullopt;
}

void DBMesoCalendarModel::setTimeOut(const uint meso_idx, const uint calendar_day, const QTime &new_timeout)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, appUtils()->formatTime(new_timeout, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

const std::optional<QString> DBMesoCalendarModel::location(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION);
}

void DBMesoCalendarModel::setLocation(const uint meso_idx, const uint calendar_day, const QString &new_location)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION, new_location);
}

const std::optional<QString> DBMesoCalendarModel::notes(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES);
}

void DBMesoCalendarModel::setNotes(const uint meso_idx, const uint calendar_day, const QString &new_notes)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES, new_notes);
}

const std::optional<bool> DBMesoCalendarModel::trainingCompleted(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TRAINING_COMPLETED))};
	if (value.has_value())
		return value.value() == STR_ONE;
	return std::nullopt;
}

void DBMesoCalendarModel::setTrainingCompleted(const uint meso_idx, const uint calendar_day, const bool completed)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, completed ? STR_ONE : STR_ZERO);
}
