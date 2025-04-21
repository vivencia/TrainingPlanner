#include "dbcalendarmodel.h"

#include "dbmesocalendarmodel.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1,
};

const auto &value = []<typename T>(const std::optional<T> &retValue) { return retValue.has_value() ? retValue.value() : T{}; };

DBCalendarModel::DBCalendarModel(DBMesoCalendarModel *parent, const uint meso_idx, const uint n_months)
	: QAbstractItemModel{parent}, m_calendarManager{parent}, m_mesoIdx(meso_idx), m_nmonths{n_months}
{
	m_roleNames[yearRole] = std::move("year");
	m_roleNames[monthRole] = std::move("month");
}

QString DBCalendarModel::workoutNumber(const int year, const int month, const int day) const
{
	return value(m_calendarManager->workoutNumber(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
}

QString DBCalendarModel::splitLetter(const int year, const int month, const int day) const
{
	return value(m_calendarManager->splitLetter(m_mesoIdx, m_calendarManager->calendarDay(m_mesoIdx, QDate{year, month-1, day})));
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
