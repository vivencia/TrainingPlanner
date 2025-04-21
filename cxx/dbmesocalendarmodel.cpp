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

std::optional<QString> DBMesoCalendarModel::dataForDatabase(const uint meso_idx) const
{
	if (meso_idx < m_modeldata.count())
	{
		QString dbdata{std::move(std::accumulate(m_modeldata.at(meso_idx).cbegin(),
												 m_modeldata.at(meso_idx).cend(),
												 QString{},
												 [this] (QString data, const QString &day_info) {
				return data + day_info + set_separator;
		}))};
		return dbdata;
	}
	return std::nullopt;
}

void DBMesoCalendarModel::dataFromDatabase(const uint meso_idx, const QString &dbdata)
{
	if (meso_idx == count())
		appendList_fast(std::move(std::move(dbdata.split(set_separator, Qt::SkipEmptyParts))));
}

std::optional<QString> DBMesoCalendarModel::dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const
{
	if (meso_idx < m_modeldata.count())
	{
		if (calendar_day < m_modeldata.at(meso_idx).count())
			return  appUtils()->getCompositeValue(field, m_modeldata.at(meso_idx).at(calendar_day), record_separator);
	}
	return std::nullopt;
}

void DBMesoCalendarModel::setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value)
{
	if (meso_idx < m_modeldata.count())
	{
		if (calendar_day < m_modeldata.at(meso_idx).count())
			appUtils()->setCompositeValue(field, new_value, m_modeldata[meso_idx][calendar_day], record_separator);
	}
}

const int DBMesoCalendarModel::calendarDay(const uint meso_idx, const QDate& date) const
{
	if (meso_idx < m_modeldata.count())
	{
		QDate calendarDate{std::move(appMesoModel()->startDate(meso_idx))};
		const int calendar_day{static_cast<int>(calendarDate.daysTo(date))};
		if (calendar_day >= 0 && calendar_day < m_modeldata.at(meso_idx).count())
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
