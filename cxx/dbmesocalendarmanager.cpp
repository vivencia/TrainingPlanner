#include "dbmesocalendarmanager.h"

#include "dbcalendarmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendartable.h"
#include "dbmesocyclesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "thread_manager.h"
#include "tputils.h"

#include <ranges>

DBMesoCalendarManager::DBMesoCalendarManager(DBMesocyclesModel *meso_model)
	: QObject{meso_model}, m_mesoModel{meso_model}
{
	m_calendarDB = new DBMesoCalendarTable{};
	appThreadManager()->runAction(m_calendarDB, ThreadManager::CreateTable);
	m_workoutsDB = new DBWorkoutsOrSplitsTable{EXERCISES_TABLE_ID};
	appThreadManager()->runAction(m_workoutsDB, ThreadManager::CreateTable);
}

inline DBModelInterfaceExercises *DBMesoCalendarManager::getDBModelInterfaceExercises(const uint meso_idx, const uint calendar_day) const
{
	DBExercisesModel *model{m_workouts.value(meso_idx).value(calendar_day)};
	return model->dbModelInterface();
}

inline DBModelInterfaceCalendar *DBMesoCalendarManager::getDBModelInterfaceCalendar(const uint meso_idx) const
{
	DBCalendarModel *model{m_calendars.value(meso_idx)};
	return model->dbModelInterface();
	return nullptr;
}

uint DBMesoCalendarManager::populateCalendarDays(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QString &split)
{
	QString::const_iterator splitletter{split.constBegin()};
	const qsizetype n_days{m_workouts.value(meso_idx).count()};
	DBModelInterfaceCalendar *dbmic{getDBModelInterfaceCalendar(meso_idx)};
	QDate day_date{start_date};
	const QString &meso_id{m_mesoModel->id(meso_idx)};
	const QString &str_id{"-1"_L1};
	for (uint i{0}; i < n_days; ++i)
	{
		day_date = std::move(start_date.addDays(i));
		QStringList day_info{3};
		day_info[CALENDAR_DATABASE_ID] = str_id;
		day_info[CALENDAR_DATABASE_MESOID] = meso_id;
		day_info[CALENDAR_DATABASE_DATE] = std::move(appUtils()->formatDate(day_date, TPUtils::DF_DATABASE));
		day_info[CALENDAR_DATABASE_DATA] = std::move(appUtils()->string_strings({meso_id, str_id, day_info.at(CALENDAR_DATABASE_DATE),
			QString::number(i+1), *splitletter, QString{}, QString{}, QString{}, QString{}, "0"_L1}, record_separator));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
		dbmic->modelData().append(std::move(day_info));
	}
	return appUtils()->calculateNumberOfMonths(start_date, end_date);
}

QString DBMesoCalendarManager::dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const
{
	DBModelInterfaceCalendar *dbmic{getDBModelInterfaceCalendar(meso_idx)};
	return appUtils()->getCompositeValue(field, dbmic->modelData().at(calendar_day).at(CALENDAR_DATABASE_DATA), record_separator);
}

void DBMesoCalendarManager::setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value, const bool emit_signal)
{
	DBModelInterfaceCalendar *dbmic{getDBModelInterfaceCalendar(meso_idx)};
	appUtils()->setCompositeValue(field, new_value, dbmic->modelData()[calendar_day][CALENDAR_DATABASE_DATA], record_separator);
	if (emit_signal)
		emit calendarChanged(meso_idx, field, calendar_day);
}

void DBMesoCalendarManager::removeCalendarForMeso(const uint meso_idx, const bool remove_workouts)
{
	if (m_mesoModel->_id(meso_idx) >= 0)
	{
		DBModelInterfaceCalendar *dbmic{getDBModelInterfaceCalendar(meso_idx)};
		dbmic->setRemovalInfo(0, QList<uint>{1, CALENDAR_DATABASE_MESOID});
		m_calendarDB->setDBModelInterface(dbmic);
		appThreadManager()->runAction(m_calendarDB, ThreadManager::DeleteRecords);
		dbmic->modelData().remove(meso_idx);
		if (remove_workouts)
		{
			//Get the meso_id from the first workout. DeleteRecord will remove all the records based on it
			DBModelInterfaceExercises *dbmie{getDBModelInterfaceExercises(meso_idx, 0)};
			dbmie->setRemovalInfo(0, QList<uint>{1, CALENDAR_DATABASE_MESOID});
			m_workoutsDB->setDBModelInterface(dbmie);
			appThreadManager()->runAction(m_workoutsDB, ThreadManager::DeleteRecords);
		}
	}
	if (meso_idx < m_calendars.count())
	{
		delete m_calendars.value(meso_idx);
		m_calendars.remove(meso_idx);
		qDeleteAll(m_workouts.value(meso_idx));
		m_workouts.remove(meso_idx);

		uint i{meso_idx};
		for (const auto calendar : m_calendars | std::views::drop(meso_idx) )
		{
			calendar->setMesoIdx(i);
			for (const auto workout: m_workouts.value(i))
				workout->setMesoIdx(i);
			i++;
		}
	}
}

void DBMesoCalendarManager::getCalendarForMeso(const uint meso_idx, const bool create_calendar)
{
	if (!create_calendar)
		m_calendars.insert(meso_idx, new DBCalendarModel{this, m_calendarDB, meso_idx});
	else
	{
		const QDate &startDate{m_mesoModel->startDate(meso_idx)};
		const uint n_months{populateCalendarDays(meso_idx, startDate, m_mesoModel->endDate(meso_idx),
																							m_mesoModel->split(meso_idx))};
		DBCalendarModel *model{m_calendars.value(meso_idx)};
		model->setNMonths(n_months);
		DBModelInterfaceCalendar *dbmic{getDBModelInterfaceCalendar(meso_idx)};
		for (uint i{0}; i < model->count(); ++i)
			dbmic->setModified(i, 0);
		m_calendarDB->setDBModelInterface(dbmic);
		appThreadManager()->runAction(m_calendarDB, ThreadManager::InsertRecords);
	}
}

void DBMesoCalendarManager::remakeMesoCalendar(const uint meso_idx)
{
	removeCalendarForMeso(meso_idx, false);
	getCalendarForMeso(meso_idx, true);
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
			const QString &split{m_mesoModel->split(meso_idx)};
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
	DBExercisesModel *w_model{m_workouts.value(meso_idx).value(calendar_day)};
	if (w_model)
	{
		w_model = new DBExercisesModel{m_mesoModel, m_workoutsDB, meso_idx, calendar_day};
		m_workouts.value(meso_idx).insert(calendar_day, w_model);
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
	const QDate &start_calendar_date{m_mesoModel->startDate(meso_idx)};
	const int calendar_day{static_cast<int>(start_calendar_date.daysTo(date))};
	return calendar_day;
}

QDate DBMesoCalendarManager::dateFromCalendarDay(const uint meso_idx, const uint calendar_day) const
{
	const QDate &calendar_date{m_mesoModel->startDate(meso_idx).addDays(calendar_day)};
	if (calendar_date < m_mesoModel->endDate(meso_idx))
		return calendar_date;
	return QDate{};
}

const int DBMesoCalendarManager::nthMonth(const uint meso_idx, const QDate &date) const
{
	if (calendarDay(meso_idx, date) >= 0)
	{
		const QDate &start_calendar_date{m_mesoModel->startDate(meso_idx)};
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
