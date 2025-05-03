#include "dbmesocalendarmanager.h"

#include "dbcalendarmodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbworkoutmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <ranges>

uint DBMesoCalendarManager::populateCalendarDays(const uint meso_idx, QDate &start_date, const QDate &end_date, const QString &split)
{
	const uint n_days{static_cast<uint>(start_date.daysTo(end_date))+1};
	m_dayInfoList[meso_idx].reserve(n_days);
	QString::const_iterator splitletter{split.constBegin()};

	for (uint i{0}; i < n_days; ++i)
	{
		stDayInfo *dayinfo{new stDayInfo{}};
		dayinfo->date = appUtils()->formatDate(start_date, TPUtils::DF_DATABASE);
		dayinfo->data = std::move(appUtils()->string_strings({appMesoModel()->id(meso_idx), STR_MINUS_ONE, dayinfo->date,
			QString::number(i+1), *splitletter, QString{}, QString{}, QString{}, QString{}, STR_ZERO}, record_separator));
		m_dayInfoList[meso_idx].append(dayinfo);

		start_date = std::move(start_date.addDays(1));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
	}
	return appUtils()->calculateNumberOfMonths(start_date, end_date);
}

void DBMesoCalendarManager::createCalendar(const uint meso_idx)
{
	QDate startDate{std::move(appMesoModel()->startDate(meso_idx))};
	const uint n_months{populateCalendarDays(meso_idx, startDate, appMesoModel()->endDate(meso_idx), appMesoModel()->split(meso_idx))};
	m_calendars.at(meso_idx)->setNMonths(n_months);
	setDBDataReady(meso_idx, true, true);
}

std::optional<QString> DBMesoCalendarManager::dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const
{
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day < m_dayInfoList.at(meso_idx).count())
			return  appUtils()->getCompositeValue(field, m_dayInfoList.at(meso_idx).at(calendar_day)->data, record_separator);
	}
	return std::nullopt;
}

void DBMesoCalendarManager::setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value, const bool emit_signal)
{
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day < m_dayInfoList.at(meso_idx).count())
		{
			stDayInfo *dayinfo{m_dayInfoList.at(meso_idx).at(calendar_day)};
			appUtils()->setCompositeValue(field, new_value, dayinfo->data, record_separator);
			dayinfo->modified = true;
			if (emit_signal)
				emit calendarChanged(meso_idx, calendar_day, field);
		}
	}
}

void DBMesoCalendarManager::removeCalendarForMeso(const uint meso_idx)
{
	appDBInterface()->removeMesoCalendar(meso_idx);
	appDBInterface()->removeWorkoutsForMeso(meso_idx);
	delete m_calendars.at(meso_idx);
	m_calendars.remove(meso_idx);
	qDeleteAll(m_workouts.at(meso_idx));
	m_workouts.remove(meso_idx);
	m_dbDataReady.remove(meso_idx);
	for (uint i{meso_idx}; i < m_calendars.count(); ++i)
	{
		m_calendars.at(i)->setMesoIdx(i);
		for (const auto workout: m_workouts.at(i))
			workout->setMesoIdx(i);
	}
	qDeleteAll(m_dayInfoList.at(meso_idx));
	m_dayInfoList.remove(meso_idx);
}

void DBMesoCalendarManager::addCalendarForMeso(const uint meso_idx)
{
	uint n_days{0};
	if (appMesoModel()->_id(meso_idx) >= 0)
		n_days = static_cast<uint>(appMesoModel()->startDate(meso_idx).daysTo(appMesoModel()->endDate(meso_idx)) + 1);
	m_dayInfoList.append(QList<stDayInfo*>{n_days});
	m_calendars.append(new DBCalendarModel{this, meso_idx});
	m_workouts.append(QList<DBWorkoutModel*>{n_days});
}

void DBMesoCalendarManager::addNewCalendarForMeso(const uint new_mesoidx)
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

void DBMesoCalendarManager::remakeMesoCalendar(const uint meso_idx, const bool preserve_old_info)
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

	appDBInterface()->removeMesoCalendar(meso_idx);
	appDBInterface()->removeWorkoutsForMeso(meso_idx);
	QDate startDate{std::move(appMesoModel()->startDate(meso_idx))};
	QDate oldStartDate{std::move(date(meso_idx, 0).value())};

	//Backup the old info
	uint n_days_with_oldinfo{static_cast<uint>(oldStartDate.daysTo(QDate::currentDate())) - 1};
	QList<stDayInfo*> dayInfoBackup{n_days_with_oldinfo};
	for (auto &day_info : mesoCalendar(meso_idx) | std::views::reverse)
	{
		dayInfoBackup[n_days_with_oldinfo] = std::move(day_info);
		--n_days_with_oldinfo;
	}

	//Create a new calendar
	qDeleteAll(m_dayInfoList.at(meso_idx));
	const uint n_new_days{static_cast<uint>(startDate.daysTo(appMesoModel()->endDate(meso_idx))) + 1};
	m_dayInfoList.resize(n_new_days);
	const uint n_months{populateCalendarDays(meso_idx, startDate, appMesoModel()->endDate(meso_idx), appMesoModel()->split(meso_idx))};

	//Move old information into the new calendar
	QList<stDayInfo*> &meso_calendar{mesoCalendar(meso_idx)};
	const uint firstReplaceableCalendarDay{static_cast<uint>(startDate.daysTo(oldStartDate))};
	auto old_day_info{dayInfoBackup.begin()};
	for (uint i{firstReplaceableCalendarDay}; i < dayInfoBackup.count(); ++i)
	{
		meso_calendar[i] = std::move(*old_day_info);
		++old_day_info;
	}

	qDeleteAll(dayInfoBackup);
	m_calendars.at(meso_idx)->setNMonths(n_months);
	setDBDataReady(meso_idx, true, true);
}

void DBMesoCalendarManager::alterCalendarSplits(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QChar &new_splitletter)
{
	int day{calendarDay(meso_idx, start_date)};
	if (day >= 0)
	{
		const int last_day{calendarDay(meso_idx, end_date)};
		if (last_day >= 0)
		{
			const int day_of_week{start_date.dayOfWeek()-1};
			const QString &split{appMesoModel()->split(meso_idx)};
			uint i {0};

			QString::iterator splitletter = const_cast<QString::iterator>(split.begin());
			do {
				if ((*splitletter) == new_splitletter)
				{
					if (i == day_of_week)
						break;
				}
				++i;
			} while (++splitletter != split.cend());
			if (splitletter == split.cend())
				return; //error: new_splitletter does not belong to split

			for (; day <= last_day; ++day)
			{
				setSplitLetter(meso_idx, day, *splitletter, false);
				if (++splitletter != split.cend())
					splitletter = const_cast<QString::iterator>(split.begin());
			}
			emit calendarChanged(meso_idx);
		}
	}
}

DBWorkoutModel *DBMesoCalendarManager::workoutForDay(const uint meso_idx, const QDate &date)
{
	DBWorkoutModel *w_model{nullptr};
	if (meso_idx < m_dayInfoList.count())
	{
		const int cal_day{calendarDay(meso_idx, date)};
		if (cal_day >= 0)
		{
			QList<DBWorkoutModel*> *workouts{&(m_workouts[meso_idx])};
			if (cal_day >= workouts->count())
			{
				for (uint i{static_cast<uint>(workouts->count())}; i < cal_day; ++i)
					workouts->append(new DBWorkoutModel{this, meso_idx, i});
			}
			else
				w_model = workouts->at(cal_day);
		}
	}
	return w_model;
}

const int DBMesoCalendarManager::calendarDay(const uint meso_idx, const QDate &date) const
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

const std::optional<QString> DBMesoCalendarManager::mesoId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_MESOID);
}

void DBMesoCalendarManager::setMesoId(const uint meso_idx, const uint calendar_day, const QString &new_meso_id)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_MESOID, new_meso_id);
}

const std::optional<QString> DBMesoCalendarManager::workoutId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID);
}

void DBMesoCalendarManager::setWorkoutId(const uint meso_idx, const uint calendar_day, const QString &new_workout_id)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID, new_workout_id);
}

const std::optional<QDate> DBMesoCalendarManager::date(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE))};
	if (value.has_value())
		return appUtils()->getDateFromDateString(value.value(), TPUtils::DF_DATABASE);
	return std::nullopt;
}

const std::optional<QDate> DBMesoCalendarManager::nThDate(const uint meso_idx, const uint nth_month) const
{
	std::optional<QDate> initialDate{std::move(date(meso_idx, 0))};
	if (initialDate.has_value())
		return initialDate.value().addMonths(nth_month);
	return initialDate;
}

void DBMesoCalendarManager::setDate(const uint meso_idx, const uint calendar_day, const QDate &new_date)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE, appUtils()->formatDate(new_date, TPUtils::DF_DATABASE));
}

const std::optional<QString> DBMesoCalendarManager::workoutNumber(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER);
}

void DBMesoCalendarManager::setWorkoutNumber(const uint meso_idx, const uint calendar_day, const QString &new_number)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER, new_number);
}

const std::optional<QString> DBMesoCalendarManager::splitLetter(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER);
}

void DBMesoCalendarManager::setSplitLetter(const uint meso_idx, const uint calendar_day, const QString &new_splitletter, const bool emit_signal)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER, new_splitletter, emit_signal);
}

const std::optional<QTime> DBMesoCalendarManager::timeIn(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN))};
	if (value.has_value())
		return appUtils()->getTimeFromTimeString(value.value(), TPUtils::TF_QML_DISPLAY_NO_SEC);
	return std::nullopt;
}

void DBMesoCalendarManager::setTimeIn(const uint meso_idx, const uint calendar_day, const QTime &new_timein)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, appUtils()->formatTime(new_timein, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

const std::optional<QTime> DBMesoCalendarManager::timeOut(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEOUT))};
	if (value.has_value())
		return appUtils()->getTimeFromTimeString(value.value(), TPUtils::TF_QML_DISPLAY_NO_SEC);
	return std::nullopt;
}

void DBMesoCalendarManager::setTimeOut(const uint meso_idx, const uint calendar_day, const QTime &new_timeout)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, appUtils()->formatTime(new_timeout, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

const std::optional<QString> DBMesoCalendarManager::location(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION);
}

void DBMesoCalendarManager::setLocation(const uint meso_idx, const uint calendar_day, const QString &new_location)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION, new_location);
}

const std::optional<QString> DBMesoCalendarManager::notes(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES);
}

void DBMesoCalendarManager::setNotes(const uint meso_idx, const uint calendar_day, const QString &new_notes)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES, new_notes);
}

const std::optional<bool> DBMesoCalendarManager::trainingCompleted(const uint meso_idx, const uint calendar_day) const
{
	const std::optional<QString> value{std::move(dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TRAINING_COMPLETED))};
	if (value.has_value())
		return value.value() == STR_ONE;
	return std::nullopt;
}

void DBMesoCalendarManager::setTrainingCompleted(const uint meso_idx, const uint calendar_day, const bool completed)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, completed ? STR_ONE : STR_ZERO);
}
