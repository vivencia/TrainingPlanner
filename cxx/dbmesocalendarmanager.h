#pragma once

#include <QHash>
#include <QObject>

enum MesoCalendarFields
{
	MESOCALENDAR_COL_MESOID,
	MESOCALENDAR_COL_WORKOUTID,
	MESOCALENDAR_COL_DATE,
	MESOCALENDAR_COL_WORKOUTNUMBER,
	MESOCALENDAR_COL_SPLITLETTER,
	MESOCALENDAR_COL_TIMEIN,
	MESOCALENDAR_COL_TIMEOUT,
	MESOCALENDAR_COL_LOCATION,
	MESOCALENDAR_COL_NOTES,
	MESOCALENDAR_COL_WORKOUT_COMPLETED,
	MESOCALENDAR_TOTAL_COLS
};

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBCalendarModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarTable)
QT_FORWARD_DECLARE_CLASS(DBWorkoutsOrSplitsTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercises)

class DBMesoCalendarManager : public QObject
{

Q_OBJECT

public:
	explicit DBMesoCalendarManager(DBMesocyclesModel *meso_model);

	inline DBMesocyclesModel *mesoModel() const { return m_mesoModel; }
	void removeCalendarForMeso(const uint meso_idx, const bool remove_workouts);
	void getCalendarForMeso(const uint meso_idx);
	void remakeMesoCalendar(const uint meso_idx);
	void alterCalendarSplits(const uint meso_idx, const QDate &start_date, const QDate &end_date,
																							const QChar &new_splitletter);
	DBExercisesModel *workoutForDay(const uint meso_idx, const int calendar_day);
	inline DBExercisesModel *workoutForDay(const uint meso_idx, const QDate &date)
	{
		return workoutForDay(meso_idx, calendarDay(meso_idx, date));
	}
	int importWorkoutFromFile(const QString &filename, const uint meso_idx, const QDate &date,
															const std::optional<bool> &file_formatted = std::nullopt);

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

	inline DBCalendarModel *calendar(const uint meso_idx) const { return m_calendars.value(meso_idx); }
	inline DBCalendarModel *workingCalendar() const { return m_workingCalendar; }
	void setWorkingCalendar(const uint meso_idx);

	[[nodiscard]] inline DBWorkoutsOrSplitsTable *workoutsTable() const { return m_workoutsDB; }

signals:
	void calendarChanged(const uint meso_idx, const uint field, const int calendar_day = -1);

private:
	QHash<uint,QHash<uint,DBExercisesModel*>> m_workouts;
	QHash<uint,DBCalendarModel*> m_calendars;
	DBCalendarModel* m_workingCalendar;

	DBMesocyclesModel *m_mesoModel;
	DBMesoCalendarTable *m_calendarDB;
	DBWorkoutsOrSplitsTable *m_workoutsDB;
	DBModelInterfaceCalendar *m_workingDBmic;

	DBModelInterfaceExercises *getDBModelInterfaceExercises(const uint meso_idx, const uint calendar_day) const;
	uint populateCalendarDays(const uint meso_idx);
	inline QString dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const;
	inline void setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value, const bool emit_signal = true);
};
