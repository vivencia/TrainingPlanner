#include "dbmesocalendarmanager.h"

#include "dbcalendarmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "tputils.h"

#include <ranges>

void DBMesoCalendarManager::setDBDataReady(const uint meso_idx, const bool ready, const uint n_months)
{
	m_dbDataReady[meso_idx] = ready;
	m_calendars.at(meso_idx)->setNMonths(n_months);
	emit calendarChanged(meso_idx, MESOCALENDAR_TOTAL_COLS);
}

uint DBMesoCalendarManager::populateCalendarDays(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QString &split)
{
	QString::const_iterator splitletter{split.constBegin()};
	const qsizetype n_days{m_dayInfoList.at(meso_idx).count()};
	QDate day_date{start_date};
	for (uint i{0}; i < n_days; ++i)
	{
		day_date = std::move(start_date.addDays(i));
		stDayInfo *dayinfo{new stDayInfo{}};
		dayinfo->date = std::move(appUtils()->formatDate(day_date, TPUtils::DF_DATABASE));
		dayinfo->data = std::move(appUtils()->string_strings({appMesoModel()->id(meso_idx), "-1"_L1, dayinfo->date,
			QString::number(i+1), *splitletter, QString{}, QString{}, QString{}, QString{}, "0"_L1}, record_separator));
		m_dayInfoList[meso_idx][i] = dayinfo;
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
	}
	return appUtils()->calculateNumberOfMonths(start_date, end_date);
}

void DBMesoCalendarManager::createCalendar(const uint meso_idx)
{
	addCalendarForMeso(meso_idx);
	const QDate &startDate{appMesoModel()->startDate(meso_idx)};
	const uint n_months{populateCalendarDays(meso_idx, startDate, appMesoModel()->endDate(meso_idx), appMesoModel()->split(meso_idx))};
	setDBDataReady(meso_idx, n_months, true);
	appDBInterface()->saveMesoCalendar(meso_idx);
}

QString DBMesoCalendarManager::dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const
{
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day < m_dayInfoList.at(meso_idx).count() && m_dayInfoList.at(meso_idx).at(calendar_day) != nullptr)
			return  appUtils()->getCompositeValue(field, m_dayInfoList.at(meso_idx).at(calendar_day)->data, record_separator);
	}
	return QString{};
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
				emit calendarChanged(meso_idx, field, calendar_day);
		}
	}
}

void DBMesoCalendarManager::removeCalendarForMeso(const uint meso_idx, const bool remove_workouts)
{
	if (appMesoModel()->_id(meso_idx) >= 0)
	{
		appDBInterface()->removeMesoCalendar(meso_idx);
		if (remove_workouts)
			appDBInterface()->removeAllWorkouts(meso_idx);
	}
	if (meso_idx < m_calendars.count())
	{
		delete m_calendars.at(meso_idx);
		m_calendars.remove(meso_idx);
		qDeleteAll(m_workouts.at(meso_idx));
		m_workouts.remove(meso_idx);
		m_dbDataReady.remove(meso_idx);

		uint i{meso_idx};
		for (const auto calendar : m_calendars | std::views::drop(meso_idx) )
		{
			calendar->setMesoIdx(i);
			for (const auto workout: m_workouts.at(i))
				workout->setMesoIdx(i);
			i++;
		}
		qDeleteAll(m_dayInfoList.at(meso_idx));
		m_dayInfoList.remove(meso_idx);
	}
}

void DBMesoCalendarManager::addCalendarForMeso(const uint meso_idx)
{
	if (meso_idx >= m_dayInfoList.count())
	{
		uint n_days{0};
		if (appMesoModel()->_id(meso_idx) >= 0)
			n_days = static_cast<uint>(appMesoModel()->startDate(meso_idx).daysTo(appMesoModel()->endDate(meso_idx)) + 1);
		m_dbDataReady.append(TPBool{});
		m_dayInfoList.append(QList<stDayInfo*>{n_days});
		m_calendars.append(new DBCalendarModel{this, meso_idx});
		m_workouts.append(QList<DBExercisesModel*>{n_days});
	}
}

void DBMesoCalendarManager::addNewCalendarForMeso(const uint new_mesoidx)
{
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
	if (!appDBInterface()->mesoCalendarSavedInDB(meso_idx))
		return;

	if (!preserve_old_info)
	{
		removeCalendarForMeso(meso_idx, false);
		createCalendar(meso_idx);
		appDBInterface()->saveMesoCalendar(meso_idx);
		return;
	}

	appDBInterface()->removeMesoCalendar(meso_idx);
	const QDate &startDate{appMesoModel()->startDate(meso_idx)};
	const QDate &oldStartDate{date(meso_idx, 0)};

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
	setDBDataReady(meso_idx, n_months, true);
	appDBInterface()->saveMesoCalendar(meso_idx);
}

void DBMesoCalendarManager::alterCalendarSplits(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QChar &new_splitletter)
{
	int day{calendarDay(meso_idx, start_date)};
	if (day >= 0)
	{
		const int last_day{calendarDay(meso_idx, end_date)};
		if (last_day >= 0)
		{
			const int day_of_week{start_date.dayOfWeek() - 1};
			const QString &split{appMesoModel()->split(meso_idx)};
			uint i{0};

			QString::const_iterator splitletter{const_cast<QString::const_iterator>(split.begin())};
			do {
				if ((*splitletter) == new_splitletter)
				{
					if (i == day_of_week)
						break;
				}
				++i;
			} while (++splitletter != const_cast<QString::const_iterator>(split.end()));
			if (splitletter == const_cast<QString::const_iterator>(split.end()))
				return; //error: new_splitletter does not belong to split

			for (; day <= last_day; ++day)
			{
				setSplitLetter(meso_idx, day, *splitletter, false);
				if (++splitletter != const_cast<QString::const_iterator>(split.end()))
					splitletter = const_cast<QString::const_iterator>(split.begin());
			}
			emit calendarChanged(meso_idx, MESOCALENDAR_COL_SPLITLETTER);
		}
	}
}

DBExercisesModel *DBMesoCalendarManager::workoutForDay(const uint meso_idx, const int calendar_day)
{
	DBExercisesModel *w_model{nullptr};
	if (meso_idx < m_dayInfoList.count())
	{
		if (calendar_day >= 0)
		{
			QList<DBExercisesModel*> &workouts{m_workouts[meso_idx]};
			if (!workouts.at(calendar_day))
				workouts[calendar_day] = new DBExercisesModel{this, meso_idx, calendar_day};
			w_model = workouts.at(calendar_day);
		}
	}
	return w_model;
}

int DBMesoCalendarManager::importWorkoutFromFile(const QString &filename, const uint meso_idx, const QDate &date,
														const std::optional<bool> &file_formatted)
{
	DBExercisesModel *workout{workoutForDay(meso_idx, date)};
	workout->clearExercises();
	return workout->newExercisesFromFile(filename, file_formatted);
}

const int DBMesoCalendarManager::calendarDay(const uint meso_idx, const QDate &date) const
{
	if (meso_idx < m_dayInfoList.count())
	{
		const QDate &start_calendar_date{appMesoModel()->startDate(meso_idx)};
		const int calendar_day{static_cast<int>(start_calendar_date.daysTo(date))};
		if (calendar_day >= 0 && calendar_day < m_dayInfoList.at(meso_idx).count())
			return calendar_day;
	}
	return -1;
}

QDate DBMesoCalendarManager::dateFromCalendarDay(const uint meso_idx, const uint calendar_day) const
{
	const QDate &calendar_date{appMesoModel()->startDate(meso_idx).addDays(calendar_day)};
	if (calendar_date < appMesoModel()->endDate(meso_idx))
		return calendar_date;
	return QDate{};
}

const int DBMesoCalendarManager::nthMonth(const uint meso_idx, const QDate &date) const
{
	if (calendarDay(meso_idx, date) >= 0)
	{
		const QDate &start_calendar_date{appMesoModel()->startDate(meso_idx)};
		return appUtils()->calculateNumberOfMonths(start_calendar_date, date) - 1;
	}
	return -1;
}

QString DBMesoCalendarManager::mesoId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_MESOID);
}

QString DBMesoCalendarManager::workoutId(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID);
}

void DBMesoCalendarManager::setWorkoutId(const uint meso_idx, const uint calendar_day, const QString &new_workout_id)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTID, new_workout_id);
}

QDate DBMesoCalendarManager::date(const uint meso_idx, const uint calendar_day) const
{
	const QString &value{dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE)};
	return !value.isEmpty() ? appUtils()->getDateFromDateString(value, TPUtils::DF_DATABASE) : QDate{};
}

QDate DBMesoCalendarManager::nThDate(const uint meso_idx, const uint nth_month) const
{
	const QDate &initialDate{date(meso_idx, 0)};
	if (initialDate.isValid())
		return initialDate.addMonths(nth_month);
	return initialDate;
}

void DBMesoCalendarManager::setDate(const uint meso_idx, const uint calendar_day, const QDate &new_date)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_DATE, appUtils()->formatDate(new_date, TPUtils::DF_DATABASE));
}

QString DBMesoCalendarManager::workoutNumber(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER);
}

void DBMesoCalendarManager::setWorkoutNumber(const uint meso_idx, const uint calendar_day, const QString &new_number)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUTNUMBER, new_number);
}

const QChar DBMesoCalendarManager::splitLetter(const uint meso_idx, const uint calendar_day) const
{
	const QString &day_info{dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER)};
	return !day_info.isEmpty() ? day_info.at(0) : QChar{};
}

void DBMesoCalendarManager::setSplitLetter(const uint meso_idx, const uint calendar_day, const QString &new_splitletter, const bool emit_signal)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_SPLITLETTER, new_splitletter, emit_signal);
}

QTime DBMesoCalendarManager::timeIn(const uint meso_idx, const uint calendar_day) const
{
	const QString &value{dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN)};
	return !value.isEmpty() ? appUtils()->getTimeFromTimeString(value, TPUtils::TF_QML_DISPLAY_NO_SEC) : QTime{0,0};
}

void DBMesoCalendarManager::setTimeIn(const uint meso_idx, const uint calendar_day, const QTime &new_timein)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEIN, appUtils()->formatTime(new_timein, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

QTime DBMesoCalendarManager::timeOut(const uint meso_idx, const uint calendar_day) const
{
	const QString &value{dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEOUT)};
	return !value.isEmpty() ? appUtils()->getTimeFromTimeString(value, TPUtils::TF_QML_DISPLAY_NO_SEC) : QTime{0, 0};
}

void DBMesoCalendarManager::setTimeOut(const uint meso_idx, const uint calendar_day, const QTime &new_timeout)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_TIMEOUT, appUtils()->formatTime(new_timeout, TPUtils::TF_QML_DISPLAY_NO_SEC));
}

QString DBMesoCalendarManager::location(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION);
}

void DBMesoCalendarManager::setLocation(const uint meso_idx, const uint calendar_day, const QString &new_location)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_LOCATION, new_location);
}

QString DBMesoCalendarManager::notes(const uint meso_idx, const uint calendar_day) const
{
	return dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES);
}

void DBMesoCalendarManager::setNotes(const uint meso_idx, const uint calendar_day, const QString &new_notes)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_NOTES, new_notes);
}

bool DBMesoCalendarManager::workoutCompleted(const uint meso_idx, const uint calendar_day) const
{
	const QString &value{dayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUT_COMPLETED)};
	return value == '1';
}

void DBMesoCalendarManager::setWorkoutCompleted(const uint meso_idx, const uint calendar_day, const bool completed)
{
	setDayInfo(meso_idx, calendar_day, MESOCALENDAR_COL_WORKOUT_COMPLETED, completed ? "1"_L1 : "0"_L1);
}
