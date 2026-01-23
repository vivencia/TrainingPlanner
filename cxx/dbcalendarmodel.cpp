#include "dbcalendarmodel.h"

#include "dbmesocyclesmodel.h"
#include "dbmesocalendartable.h"
#include "tputils.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole + 1
};

DBCalendarModel::DBCalendarModel(DBMesocyclesModel *parent, DBMesoCalendarTable* db, const uint meso_idx)
	: QAbstractListModel{parent}, m_mesoModel{parent}, m_db{db}, m_mesoIdx(meso_idx), m_nMonths{-1}, m_curDay{-1}, m_nCaldays{-1}
{
	roleToString(year)
	roleToString(month)
	m_startDate = std::move(m_mesoModel->startDate(m_mesoIdx));

	m_dbmic = new DBModelInterfaceCalendar{this};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(m_db, &DBMesoCalendarTable::calendarLoaded, this, [this,conn] (const uint meso_idx, const bool success)
	{
		if (meso_idx == m_mesoIdx)
		{
			disconnect(*conn);
			if (success)
			{
				beginResetModel();
				const QDate endDate{appUtils()->dateFromString(
								m_dbmic->modelData().constLast().at(CALENDAR_DATABASE_DATE), TPUtils::DF_DATABASE)};
				m_nMonths = appUtils()->calculateNumberOfMonths(m_startDate, endDate);
				emit nMonthsChanged();
				m_nCaldays = m_startDate.daysTo(endDate) + 1;
				endResetModel();
			}
			emit calendarLoaded(success);
		}
	});
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords, m_dbmic);
}

QDate DBCalendarModel::firstDateOfEachMonth(const uint index) const
{
	if (index < m_nMonths)
	{
		const QDate &date{m_startDate.addMonths(index)};
		return QDate{date.year(), date.month(), 1};
	}
	return QDate{};
}

const QString &DBCalendarModel::mesoId() const
{
	return m_mesoModel->id(m_mesoIdx);
}

int DBCalendarModel::calendarDay(const QDate &date) const
{
	const int cal_day{static_cast<int>(m_startDate.daysTo(date))};
	if (cal_day >= 0 && cal_day < m_nCaldays)
		return cal_day;
	return -1;
}

int DBCalendarModel::getIndexFromDate(const QDate &date) const
{
	return appUtils()->calculateNumberOfMonths(m_startDate, date) - 1;
}

QDate DBCalendarModel::date(const uint calendar_day) const
{
	const QDate &calendar_date{m_startDate.addDays(calendar_day)};
	if (calendar_date < m_mesoModel->endDate(m_mesoIdx))
		return calendar_date;
	return QDate{};
}

bool DBCalendarModel::isWorkoutDay(const int calendar_day)
{
	return !dayInfo(calendar_day, CALENDAR_FIELD_WORKOUTNUMBER).isEmpty();
}

QString DBCalendarModel::workoutNumber(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, CALENDAR_FIELD_WORKOUTNUMBER) : QString{};
}

QString DBCalendarModel::workoutNumber() const
{
	return dayInfo(m_curDay, CALENDAR_FIELD_WORKOUTNUMBER);
}

QString DBCalendarModel::splitLetter(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, CALENDAR_FIELD_SPLITLETTER) : QString{};
}

void DBCalendarModel::setSplitLetter(const QDate &date, const QString &new_splitletter)
{
	const auto cal_day{calendarDay(date)};
	if (cal_day != -1)
	{
		setDayInfo(cal_day, CALENDAR_FIELD_SPLITLETTER, new_splitletter);
		emit splitLetterChanged();
	}
}

QString DBCalendarModel::splitLetter() const
{
	return dayInfo(m_curDay, CALENDAR_FIELD_SPLITLETTER);
}

void DBCalendarModel::setSplitLetter(const QString &new_splitletter)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_SPLITLETTER, new_splitletter);
	emit splitLetterChanged();
}

QString DBCalendarModel::dayEntryLabel(const QDate &date) const
{
	return isWorkoutDay(date) ? QString::number(date.day()) % '-' % splitLetter(date) : QString::number(date.day());
}

QString DBCalendarModel::location(const int calendar_day) const
{
	return dayInfo(calendar_day, CALENDAR_FIELD_LOCATION);
}

QString DBCalendarModel::location() const
{
	return dayInfo(m_curDay, CALENDAR_FIELD_LOCATION);
}

void DBCalendarModel::setLocation(const QString &new_location)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_LOCATION, new_location);
}

QString DBCalendarModel::notes() const
{
	return dayInfo(m_curDay, CALENDAR_FIELD_NOTES);
}

void DBCalendarModel::setNotes(const QString &new_notes)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_NOTES, new_notes);
}

QTime DBCalendarModel::timeIn() const
{
	return appUtils()->timeFromString(dayInfo(m_curDay, CALENDAR_FIELD_TIMEIN));
}

void DBCalendarModel::setTimeIn(const QTime &new_timein)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_TIMEIN, appUtils()->formatTime(new_timein));
}

QTime DBCalendarModel::timeOut() const
{
	return appUtils()->timeFromString(dayInfo(m_curDay, CALENDAR_FIELD_TIMEOUT));
}

void DBCalendarModel::setTimeOut(const QTime &new_timeout)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_TIMEOUT, appUtils()->formatTime(new_timeout));
}

bool DBCalendarModel::completed_by_date(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, CALENDAR_FIELD_WORKOUT_COMPLETED) == "1"_L1 : false;
}

bool DBCalendarModel::completed() const
{
	return dayInfo(m_curDay, CALENDAR_FIELD_WORKOUT_COMPLETED) == "1"_L1;
}

void DBCalendarModel::setCompleted(const bool completed)
{
	setDayInfo(m_curDay, CALENDAR_FIELD_WORKOUT_COMPLETED, completed ? "1"_L1 : "0"_L1);
	emit completedChanged(date(m_curDay), completed);
}

QVariant DBCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_nMonths)
	{
		switch (role)
		{
			case yearRole: return firstDateOfEachMonth(row).year();
			case monthRole: return firstDateOfEachMonth(row).month() - 1;
		}
	}
	return QVariant{};
}

QString DBCalendarModel::dayInfo(const int calendar_day, const uint field) const
{
	return calendar_day != -1 ?
		appUtils()->getCompositeValue(field, m_dbmic->modelData().at(calendar_day).at(CALENDAR_DATABASE_DATA), record_separator) :
		QString{};
}

void DBCalendarModel::setDayInfo(const int calendar_day, const uint field, const QString &new_value)
{
	appUtils()->setCompositeValue(field, new_value, m_dbmic->modelData()[calendar_day][CALENDAR_DATABASE_DATA], record_separator);
	m_dbmic->setModified(calendar_day, CALENDAR_DATABASE_DATA);
	m_db->setDBModelInterface(m_dbmic);
	appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField);
}
