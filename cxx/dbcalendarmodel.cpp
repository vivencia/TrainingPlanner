#include "dbcalendarmodel.h"

#include "dbmesocalendarmodel.h"

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1,
	dateRole = Qt::UserRole+MESOCALENDAR_COL_DATE,
	workoutRole = Qt::UserRole+MESOCALENDAR_COL_WORKOUTNUMBER,
	splitLetterRole = Qt::UserRole+MESOCALENDAR_COL_SPLITLETTER,
	timeInRole = Qt::UserRole+MESOCALENDAR_COL_TIMEIN,
	timeOutRole = Qt::UserRole+MESOCALENDAR_COL_TIMEOUT,
	locationRole = Qt::UserRole+MESOCALENDAR_COL_LOCATION,
	notesRole = Qt::UserRole+MESOCALENDAR_COL_NOTES,
	completedRole = Qt::UserRole+MESOCALENDAR_COL_TRAINING_COMPLETED,
};

DBCalendarModel::DBCalendarModel(DBMesoCalendarModel *parent, const uint meso_idx, const uint n_months)
	: QAbstractItemModel{parent}, m_calendarManager{parent}, m_mesoIdx(meso_idx), m_nmonths{n_months}
{
	m_roleNames[yearRole] = std::move("year");
	m_roleNames[monthRole] = std::move("month");
	m_roleNames[dateRole] = std::move("date");
	m_roleNames[workoutRole] = std::move("workoutNumber");
	m_roleNames[splitLetterRole] = std::move("splitLetter");
	m_roleNames[timeInRole] = std::move("timeIn");
	m_roleNames[timeOutRole] = std::move("timeOut");
	m_roleNames[locationRole] = std::move("location");
	m_roleNames[notesRole] = std::move("notes");
	m_roleNames[completedRole] = std::move("completed");
}

QVariant DBCalendarModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_nmonths)
	{
		switch (role)
		{
			case yearRole:
				return m_calendarManager->date(m_mesoIdx, row)->year();
			break;
			case monthRole:
				return m_calendarManager->date(m_mesoIdx, row)->month() - 1;
			break;
		}
	}
	return QVariant{};
}
