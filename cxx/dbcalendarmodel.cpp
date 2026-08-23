#include "dbcalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendartable.h"
#include "tputils.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole + 1
};

DBCalendarModel::DBCalendarModel(DBMesocyclesModel *parent, DBMesoCalendarTable* db, const uint meso_idx)
	: QAbstractListModel{parent}, m_mesoModel{parent}, m_db{db}, m_mesoIdx(meso_idx)
{
	roleToString(year)
	roleToString(month)
	m_startDate = std::move(m_mesoModel->startDate(m_mesoIdx));
	m_dbmic = new DBModelInterfaceCalendar{this};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(m_db, &DBMesoCalendarTable::calendarLoaded, this, [this,conn] (const uint meso_idx, const bool success) {
		if (meso_idx == m_mesoIdx) {
			disconnect(*conn);
			if (success) {
				beginResetModel();
				const QDate &endDate{appUtils()->dateFromString(
							m_dbmic->modelData().constLast().at(DBMesoCalendarTable::CALDB_DATE), TPUtils::DF_DATABASE)};
				m_nMonths = appUtils()->calculateNumberOfMonths(m_startDate, endDate);
				emit nMonthsChanged();
				m_nCaldays = m_startDate.daysTo(endDate) + 1;
				endResetModel();
			}
			emit calendarLoaded(success);
		}
	});
	m_db->setReadAllRecordsFunc([this] (DBModelInterface*) { return m_db->getMesoCalendar(this->m_dbmic); });
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords, m_dbmic);
}

QDate DBCalendarModel::firstDateOfEachMonth(const uint index) const
{
	if (index < m_nMonths) {
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
	return !dayInfo(calendar_day, FLD_WORKOUTNUMBER).isEmpty();
}

QString DBCalendarModel::workoutNumber(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, FLD_WORKOUTNUMBER) : QString{};
}

QString DBCalendarModel::workoutNumber() const
{
	return dayInfo(m_curDay, FLD_WORKOUTNUMBER);
}

QString DBCalendarModel::splitLetter(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, FLD_SPLITLETTER) : QString{};
}

void DBCalendarModel::setSplitLetter(const QDate &date, const QString &new_splitletter)
{
	const auto cal_day{calendarDay(date)};
	if (cal_day != -1) {
		setDayInfo(cal_day, FLD_SPLITLETTER, new_splitletter);
		emit splitLetterChanged(date);
	}
}

QString DBCalendarModel::splitLetter() const
{
	return dayInfo(m_curDay, FLD_SPLITLETTER);
}

void DBCalendarModel::setSplitLetter(const QString &new_splitletter)
{
	setDayInfo(m_curDay, FLD_SPLITLETTER, new_splitletter);
	emit splitLetterChanged();
}

QString DBCalendarModel::dayEntryLabel(const QDate &date) const
{
	return isWorkoutDay(date) ? QString::number(date.day()) % '-' % splitLetter(date) : QString::number(date.day());
}

void DBCalendarModel::setSelectable(QList<QDate> &&dates, const bool selectable)
{
	if (!dates.isEmpty())
		m_selectableList = std::forward<QList<QDate>>(dates);
	if (!m_selectableList.isEmpty()) {
		QList<QDate>::const_iterator itr{m_selectableList.constBegin()};
		const QList<QDate>::const_iterator &itr_end{m_selectableList.constEnd()};
		QDate date{m_startDate};
		decltype(m_nCaldays) cal_day{0};
		do {
			if (itr != itr_end && *itr == date) {
				m_selectable[date] = selectable;
				++itr;
			} else {
				m_selectable[date] = !selectable;
			}
			date = std::move(date.addDays(1));
		} while (++cal_day < m_nCaldays);
		emit dataChanged(index(0, 0), index(m_nMonths, 0));
	}
}

QString DBCalendarModel::location(const int calendar_day) const
{
	return dayInfo(calendar_day, FLD_LOCATION);
}

QString DBCalendarModel::location() const
{
	return dayInfo(m_curDay, FLD_LOCATION);
}

void DBCalendarModel::setLocation(const QString &new_location)
{
	setDayInfo(m_curDay, FLD_LOCATION, new_location);
}

QString DBCalendarModel::notes() const
{
	return dayInfo(m_curDay, FLD_NOTES);
}

void DBCalendarModel::setNotes(const QString &new_notes)
{
	setDayInfo(m_curDay, FLD_NOTES, new_notes);
}

QTime DBCalendarModel::timeIn() const
{
	return appUtils()->timeFromString(dayInfo(m_curDay, FLD_TIMEIN));
}

void DBCalendarModel::setTimeIn(const QTime &new_timein)
{
	setDayInfo(m_curDay, FLD_TIMEIN, appUtils()->formatTime(new_timein));
}

QTime DBCalendarModel::timeOut() const
{
	return appUtils()->timeFromString(dayInfo(m_curDay, FLD_TIMEOUT));
}

void DBCalendarModel::setTimeOut(const QTime &new_timeout)
{
	setDayInfo(m_curDay, FLD_TIMEOUT, appUtils()->formatTime(new_timeout));
}

bool DBCalendarModel::completed_by_date(const QDate &date) const
{
	const auto cal_day{calendarDay(date)};
	return cal_day != -1 ? dayInfo(cal_day, FLD_WORKOUT_COMPLETED) == "1"_L1 : false;
}

bool DBCalendarModel::completed() const
{
	return dayInfo(m_curDay, FLD_WORKOUT_COMPLETED) == "1"_L1;
}

void DBCalendarModel::setCompleted(const bool completed)
{
	setDayInfo(m_curDay, FLD_WORKOUT_COMPLETED, completed ? "1"_L1 : "0"_L1);
	emit completedChanged(date(m_curDay), completed);
}

QVariant DBCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_nMonths) {
		switch (role) {
		case yearRole:		return firstDateOfEachMonth(row).year();
		case monthRole:		return firstDateOfEachMonth(row).month() - 1;
		}
	}
	return QVariant{};
}

void DBCalendarModel::changeSelectableDates(const bool use_selectable_list)
{
	if (use_selectable_list)
		setSelectable(std::move(QList<QDate>{}), true);
	else
		setAllDatesSelectable(true);
}

QString DBCalendarModel::dayInfo(const int calendar_day, const uint field) const
{
	return calendar_day != -1
		? appUtils()->getCompositeValue(field, m_dbmic->modelData().at(calendar_day).at(DBMesoCalendarTable::CALDB_DATA), record_separator)
		: QString{};
}

void DBCalendarModel::setDayInfo(const int calendar_day, const uint field, const QString &new_value)
{
	appUtils()->setCompositeValue(field, new_value, m_dbmic->modelData()[calendar_day][DBMesoCalendarTable::CALDB_DATA], record_separator);
	m_dbmic->setModified(calendar_day, DBMesoCalendarTable::CALDB_DATA);
	appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField, m_dbmic);
}
