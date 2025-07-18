#pragma once

#include <QObject>

#define MESOCALENDAR_COL_MESOID 0
#define MESOCALENDAR_COL_WORKOUTID 1
#define MESOCALENDAR_COL_DATE 2
#define MESOCALENDAR_COL_WORKOUTNUMBER 3
#define MESOCALENDAR_COL_SPLITLETTER 4
#define MESOCALENDAR_COL_TIMEIN 5
#define MESOCALENDAR_COL_TIMEOUT 6
#define MESOCALENDAR_COL_LOCATION 7
#define MESOCALENDAR_COL_NOTES 8
#define MESOCALENDAR_COL_WORKOUT_COMPLETED 9
#define MESOCALENDAR_TOTAL_COLS MESOCALENDAR_COL_WORKOUT_COMPLETED + 1

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBCalendarModel)

class TPBool {
public:
	// Default constructor initializes the value to false
	inline TPBool() : m_value(false) {}

	// Constructor to initialize with a specific boolean value
	inline explicit TPBool(bool val) : m_value{val} {}

	// Operator overloading to allow implicit conversion to bool
	operator bool() const {
		return m_value;
	}

	bool operator=(bool val) { m_value = val; return m_value; }

private:
	bool m_value;
};

struct stDayInfo
{
	QString data;
	QString date;
	TPBool modified;
};

class DBMesoCalendarManager : public QObject
{

Q_OBJECT

friend class DBMesoCalendarTable;

public:
	explicit inline DBMesoCalendarManager(QObject *parent) : QObject{parent} {}
	void removeCalendarForMeso(const uint meso_idx, const bool remove_workouts);
	void addCalendarForMeso(const uint meso_idx);
	void addNewCalendarForMeso(const uint new_mesoidx);
	void remakeMesoCalendar(const uint meso_idx, const bool preserve_old_info);
	void alterCalendarSplits(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QChar &new_splitletter);
	DBExercisesModel *workoutForDay(const uint meso_idx, const int calendar_day);
	inline DBExercisesModel *workoutForDay(const uint meso_idx, const QDate &date)
	{
		return workoutForDay(meso_idx, calendarDay(meso_idx, date));
	}
	int importWorkoutFromFile(const QString &filename, const uint meso_idx, const QDate &date,
												const std::optional<bool> &file_formatted = std::nullopt);

	[[nodiscard]] inline bool hasCalendarInfo(const uint meso_idx) const
	{
			return meso_idx < m_dayInfoList.count() ? m_dayInfoList.at(meso_idx).count() > 0 : false;
	}
	[[nodiscard]] inline const QList<stDayInfo*> &dayInfo(const uint meso_idx) const { return m_dayInfoList.at(meso_idx); }
	[[nodiscard]] const int calendarDay(const uint meso_idx, const QDate& date) const;
	[[nodiscard]] QDate dateFromCalendarDay(const uint meso_idx, const uint calendar_day) const;
	[[nodiscard]] const int nthMonth(const uint meso_idx, const QDate &date) const;

	[[nodiscard]] QString mesoId(const uint meso_idx, const uint calendar_day) const;

	[[nodiscard]] QString workoutId(const uint meso_idx, const uint calendar_day) const;
	void setWorkoutId(const uint meso_idx, const uint calendar_day, const QString &new_workout_id);

	[[nodiscard]] QDate date(const uint meso_idx, const uint calendar_day) const;
	void setDate(const uint meso_idx, const uint calendar_day, const QDate &new_date);
	[[nodiscard]] QDate nThDate(const uint meso_idx, const uint nth_month) const;

	[[nodiscard]] QString workoutNumber(const uint meso_idx, const uint calendar_day) const;
	void setWorkoutNumber(const uint meso_idx, const uint calendar_day, const QString &new_number);

	[[nodiscard]] const QChar splitLetter(const uint meso_idx, const uint calendar_day) const;
	void setSplitLetter(const uint meso_idx, const uint calendar_day, const QString &new_splitletter, const bool emit_signal = true);

	[[nodiscard]] QTime timeIn(const uint meso_idx, const uint calendar_day) const;
	void setTimeIn(const uint meso_idx, const uint calendar_day, const QTime &new_timein);

	[[nodiscard]] QTime timeOut(const uint meso_idx, const uint calendar_day) const;
	void setTimeOut(const uint meso_idx, const uint calendar_day, const QTime &new_timeout);

	[[nodiscard]] QString location(const uint meso_idx, const uint calendar_day) const;
	void setLocation(const uint meso_idx, const uint calendar_day, const QString &new_location);

	[[nodiscard]] QString notes(const uint meso_idx, const uint calendar_day) const;
	void setNotes(const uint meso_idx, const uint calendar_day, const QString &new_notes);

	[[nodiscard]] bool workoutCompleted(const uint meso_idx, const uint calendar_day) const;
	void setWorkoutCompleted(const uint meso_idx, const uint calendar_day, const bool completed);

	Q_INVOKABLE inline DBCalendarModel *calendar(const uint meso_idx) const
	{
		return meso_idx < m_calendars.count() ? m_calendars.at(meso_idx) : nullptr;
	}

	inline bool hasDBData(const uint meso_idx) const
	{
		return meso_idx < m_dbDataReady.count() ? m_dbDataReady.at(meso_idx) : false;
	}

signals:
	void calendarChanged(const uint meso_idx, const uint field, const int calendar_day = -1);

private:
	QList<QList<stDayInfo*>> m_dayInfoList;
	QList<QList<DBExercisesModel*>> m_workouts;
	QList<DBCalendarModel*> m_calendars;
	QList<TPBool> m_dbDataReady;

	inline const QList<stDayInfo*> &mesoCalendar(const uint meso_idx) const { return m_dayInfoList.at(meso_idx); }
	inline QList<stDayInfo*> &mesoCalendar(const uint meso_idx) { return m_dayInfoList[meso_idx]; }
	void setDBDataReady(const uint meso_idx, const bool ready, const uint n_months);

	uint populateCalendarDays(const uint meso_idx, const QDate &start_date, const QDate &end_date, const QString &split);
	void createCalendar(const uint meso_idx);
	inline QString dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const;
	inline void setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value, const bool emit_signal = true);
};
