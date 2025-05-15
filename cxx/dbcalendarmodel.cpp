#include "dbcalendarmodel.h"

#include "dbmesocalendarmanager.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1
};

const auto &value = []<typename T>(const std::optional<T> &retValue) { return retValue.has_value() ? retValue.value() : T{}; };
auto &&rvalue = []<typename T>(const std::optional<T> &retValue) mutable { return retValue.has_value() ? retValue.value() : T{}; };

DBCalendarModel::DBCalendarModel(DBMesoCalendarManager *parent, const uint meso_idx)
	: QAbstractListModel{parent}, m_calendarManager{parent}, m_mesoIdx(meso_idx), m_nmonths{0}
{
	m_roleNames[yearRole] = std::move("year");
	m_roleNames[monthRole] = std::move("month");
	connect(m_calendarManager, &DBMesoCalendarManager::calendarChanged, this, [this] (const uint meso_idx, const int calendar_day, const uint field) {
		if (meso_idx == m_mesoIdx)
		{
			switch (field)
			{
				case MESOCALENDAR_COL_WORKOUTNUMBER:
					emit workoutNumberChanged(value(m_calendarManager->date(m_mesoIdx, calendar_day)));
				break;
				case MESOCALENDAR_COL_SPLITLETTER:
					emit splitLetterChanged(value(m_calendarManager->date(m_mesoIdx, calendar_day)));
				break;
				case MESOCALENDAR_COL_TRAINING_COMPLETED:
					emit completedChanged(value(m_calendarManager->date(m_mesoIdx, calendar_day)));
				break;
				default: break;
			}
		}
	});
}

QDate DBCalendarModel::firstDateOfEachMonth(const uint index) const
{
	QDate date{};
	if (index < m_nmonths)
	{
		date = std::move(rvalue(m_calendarManager->date(m_mesoIdx, 0)));
		date.setDate(date.year(), date.month(), 1);
		uint i{0};
		while (i++ < index)
			date = std::move(date.addMonths(1));
	}
	return date;
}

bool DBCalendarModel::isPartOfMeso(const QDate &date) const
{
	return m_calendarManager->calendarDay(m_mesoIdx, date) != -1;
}

QString DBCalendarModel::workoutNumber(const QDate &date) const
{
	return value(m_calendarManager->workoutNumber(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, date)));
}

QString DBCalendarModel::splitLetter(const QDate &date) const
{
	return value(m_calendarManager->splitLetter(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, date)));
}

void DBCalendarModel::setSplitLetter(const int year, const int month, const int day, const QString &new_splitletter)
{
	m_calendarManager->setSplitLetter(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), new_splitletter);
}

QTime DBCalendarModel::timeIn(const int year, const int month, const int day) const
{
	return value(m_calendarManager->timeIn(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

void DBCalendarModel::setTimeIn(const int year, const int month, const int day, const QTime &new_timein)
{
	m_calendarManager->setTimeIn(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), new_timein);
}

QTime DBCalendarModel::timeOut(const int year, const int month, const int day) const
{
	return value(m_calendarManager->timeOut(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

void DBCalendarModel::setTimeOut(const int year, const int month, const int day, const QTime &new_timeout)
{
	m_calendarManager->setTimeOut(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), new_timeout);
}

QString DBCalendarModel::location(const int year, const int month, const int day) const
{
	return value(m_calendarManager->location(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

void DBCalendarModel::setLocation(const int year, const int month, const int day, const QString &new_location)
{
	m_calendarManager->setLocation(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), new_location);
}

QString DBCalendarModel::notes(const int year, const int month, const int day) const
{
	return value(m_calendarManager->notes(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

void DBCalendarModel::setNotes(const int year, const int month, const int day, const QString &new_notes)
{
	m_calendarManager->setNotes(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), new_notes);
}

bool DBCalendarModel::completed(const int year, const int month, const int day) const
{
	return value(m_calendarManager->trainingCompleted(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

void DBCalendarModel::setCompleted(const int year, const int month, const int day, const bool completed)
{
	m_calendarManager->setTrainingCompleted(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day}), completed);
}

QVariant DBCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_nmonths)
	{
		switch (role)
		{
			case yearRole:
				return value(m_calendarManager->nThDate(m_mesoIdx, row)).year();
			break;
			case monthRole:
				return value(m_calendarManager->nThDate(m_mesoIdx, row)).month() - 1;
			break;
		}
	}
	return QVariant{};
}
