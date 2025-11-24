#include "dbcalendarmodel.h"

#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendartable.h"
#include "tputils.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1
};

//const auto &value = []<typename T>(const std::optional<T> &retValue) { return retValue.has_) ? retValue.) : T{}; };
//auto &&rvalue = []<typename T>(const std::optional<T> &retValue) mutable { return retValue.has_) ? retValue.) : T{}; };

DBCalendarModel::DBCalendarModel(DBMesoCalendarManager *parent, DBMesoCalendarTable* db, const uint meso_idx)
	: QAbstractListModel{parent}, m_calendarManager{parent}, m_db{db}, m_mesoIdx(meso_idx), m_nmonths{0}
{
	m_roleNames[yearRole] = std::move("year");
	m_roleNames[monthRole] = std::move("month");
	connect(m_calendarManager, &DBMesoCalendarManager::calendarChanged, this, [this] (const uint meso_idx, const int calendar_day, const uint field) {
		if (meso_idx == m_mesoIdx)
		{
			switch (field)
			{
				case MESOCALENDAR_COL_WORKOUTNUMBER:
					emit workoutNumberChanged(m_calendarManager->date(m_mesoIdx, calendar_day));
				break;
				case MESOCALENDAR_COL_SPLITLETTER:
					emit splitLetterChanged(m_calendarManager->date(m_mesoIdx, calendar_day));
				break;
				case MESOCALENDAR_COL_WORKOUT_COMPLETED:
					emit completedChanged(m_calendarManager->date(m_mesoIdx, calendar_day));
				break;
				default: break;
			}
			m_dbModelInterface->setModified(calendar_day, field);
			m_db->setDBModelInterface(m_dbModelInterface);
			appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField);
		}
	});

	m_dbModelInterface = new DBModelInterfaceCalendar{this};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(m_db, &DBMesoCalendarTable::calendarLoaded, this, [this,conn] (const uint meso_idx, const bool success)
	{
		if (meso_idx == m_mesoIdx)
		{
			disconnect(*conn);
			if (success)
			{
				const QDate startDate{appUtils()->getDateFromDateString(m_dbModelInterface->modelData().constLast().at(1), TPUtils::DF_DATABASE)};
				const QDate endDate{appUtils()->getDateFromDateString(m_dbModelInterface->modelData().constLast().at(1), TPUtils::DF_DATABASE)};
				m_nmonths = appUtils()->calculateNumberOfMonths(startDate, endDate);
				emit dataChanged(index(0), index(m_nmonths));
			}
		}
	});
	m_db->setDBModelInterface(m_dbModelInterface);
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
}

QDate DBCalendarModel::firstDateOfEachMonth(const uint index) const
{
	QDate date{};
	if (index < m_nmonths)
	{
		date = std::move(m_calendarManager->date(m_mesoIdx, 0));
		date.setDate(date.year(), date.month(), 1);
		uint i{0};
		while (i++ < index)
			date = std::move(date.addMonths(1));
	}
	return date;
}

const QString &DBCalendarModel::mesoId() const
{
	return appMesoModel()->id(m_mesoIdx);
}

int DBCalendarModel::getIndexFromDate(const QDate &date) const
{
	return m_calendarManager->nthMonth(m_mesoIdx, date);
}

QDate DBCalendarModel::date(const uint calendar_day) const
{
	return m_calendarManager->dateFromCalendarDay(m_mesoIdx, calendar_day);
}

bool DBCalendarModel::isPartOfMeso(const QDate &date) const
{
	return m_calendarManager->calendarDay(m_mesoIdx, date) != -1;
}

bool DBCalendarModel::isWorkoutDay(const QDate &date) const
{
	return isWorkoutDay(m_calendarManager->calendarDay(m_mesoIdx, date));
}

bool DBCalendarModel::isWorkoutDay(const uint calendar_day) const
{
	return !workoutNumber(calendar_day).isEmpty();
}

QString DBCalendarModel::dayText(const QDate &date) const
{
	return isPartOfMeso(date) ? QString::number(date.day()) + '-' + splitLetter(date) : QString::number(date.day());
}

QString DBCalendarModel::workoutNumber(const QDate &date) const
{
	return workoutNumber(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QString DBCalendarModel::workoutNumber(const uint calendar_day) const
{
	return m_calendarManager->workoutNumber(m_mesoIdx, calendar_day);
}

QString DBCalendarModel::splitLetter(const QDate &date) const
{
	return splitLetter(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QString DBCalendarModel::splitLetter(const uint calendar_day) const
{
	return m_calendarManager->splitLetter(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setSplitLetter(const QDate &date, const QString &new_splitletter)
{
	setSplitLetter(m_calendarManager->calendarDay(m_mesoIdx, date), new_splitletter);
}

void DBCalendarModel::setSplitLetter(const uint calendar_day, const QString &new_splitletter)
{
	m_calendarManager->setSplitLetter(m_mesoIdx, calendar_day, new_splitletter);
}

QTime DBCalendarModel::timeIn(const QDate &date) const
{
	return timeIn(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QTime DBCalendarModel::timeIn(const uint calendar_day) const
{
	return m_calendarManager->timeIn(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setTimeIn(const QDate &date, const QTime &new_timein)
{
	setTimeIn(m_calendarManager->calendarDay(m_mesoIdx, date), new_timein);
}

void DBCalendarModel::setTimeIn(const uint calendar_day, const QTime &new_timein)
{
	m_calendarManager->setTimeIn(m_mesoIdx, calendar_day, new_timein);
}

QTime DBCalendarModel::timeOut(const QDate &date) const
{
	return timeOut(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QTime DBCalendarModel::timeOut(const uint calendar_day) const
{
	return m_calendarManager->timeOut(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setTimeOut(const QDate &date, const QTime &new_timeout)
{
	setTimeOut(m_calendarManager->calendarDay(m_mesoIdx, date), new_timeout);
}

void DBCalendarModel::setTimeOut(const uint calendar_day, const QTime &new_timeout)
{
	m_calendarManager->setTimeOut(m_mesoIdx, calendar_day, new_timeout);
}

QString DBCalendarModel::location(const QDate &date) const
{
	return location(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QString DBCalendarModel::location(const uint calendar_day) const
{
	return m_calendarManager->location(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setLocation(const QDate &date, const QString &new_location)
{
	setLocation(m_calendarManager->calendarDay(m_mesoIdx, date), new_location);
}

void DBCalendarModel::setLocation(const uint calendar_day, const QString &new_location)
{
	m_calendarManager->setLocation(m_mesoIdx, calendar_day, new_location);
}

QString DBCalendarModel::notes(const QDate &date) const
{
	return notes(m_calendarManager->calendarDay(m_mesoIdx, date));
}

QString DBCalendarModel::notes(const uint calendar_day) const
{
	return m_calendarManager->notes(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setNotes(const QDate &date, const QString &new_notes)
{
	setNotes(m_calendarManager->calendarDay(m_mesoIdx, date), new_notes);
}

void DBCalendarModel::setNotes(const uint calendar_day, const QString &new_notes)
{
	m_calendarManager->setNotes(m_mesoIdx, calendar_day, new_notes);
}

bool DBCalendarModel::completed(const QDate &date) const
{
	return completed(m_calendarManager->calendarDay(m_mesoIdx, date));
}

bool DBCalendarModel::completed(const uint calendar_day) const
{
	return m_calendarManager->workoutCompleted(m_mesoIdx, calendar_day);
}

void DBCalendarModel::setCompleted(const QDate &date, const bool completed)
{
	setCompleted(m_calendarManager->calendarDay(m_mesoIdx, date), completed);
}

void DBCalendarModel::setCompleted(const uint calendar_day, const bool completed)
{
	m_calendarManager->setWorkoutCompleted(m_mesoIdx, calendar_day, completed);
}

QVariant DBCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_nmonths)
	{
		switch (role)
		{
			case yearRole:
				return m_calendarManager->nThDate(m_mesoIdx, row).year();
			break;
			case monthRole:
				return m_calendarManager->nThDate(m_mesoIdx, row).month() - 1;
			break;
		}
	}
	return QVariant{};
}
