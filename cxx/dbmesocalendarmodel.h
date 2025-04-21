#ifndef DBMESOCALENDARMODEL_H
#define DBMESOCALENDARMODEL_H

#include "tplistmodel.h"

#define MESOCALENDAR_COL_MESOID 0
#define MESOCALENDAR_COL_WORKOUTID 1
#define MESOCALENDAR_COL_DATE 2
#define MESOCALENDAR_COL_WORKOUTNUMBER 3
#define MESOCALENDAR_COL_SPLITLETTER 4
#define MESOCALENDAR_COL_TIMEIN 5
#define MESOCALENDAR_COL_TIMEOUT 6
#define MESOCALENDAR_COL_LOCATION 7
#define MESOCALENDAR_COL_NOTES 8
#define MESOCALENDAR_COL_TRAINING_COMPLETED 9
#define MESOCALENDAR_TOTAL_COLS MESOCALENDAR_COL_TRAINING_COMPLETED + 1

QT_FORWARD_DECLARE_CLASS(DBWorkoutModel)
QT_FORWARD_DECLARE_CLASS(DBCalendarModel)

class DBMesoCalendarModel : public TPListModel
{

Q_OBJECT

public:
	explicit inline DBMesoCalendarModel(QObject *parent, const uint meso_idx);
	std::optional<QString> dataForDatabase(const uint meso_idx) const;
	void dataFromDatabase(const uint meso_idx, const QString &dbdata);
	[[nodiscard]] const int calendarDay(const uint meso_idx, const QDate& date) const;

	[[nodiscard]] const std::optional<QString> mesoId(const uint meso_idx, const uint calendar_day) const;
	void setMesoId(const uint meso_idx, const uint calendar_day, const QString &new_meso_id);

	[[nodiscard]] const std::optional<QString> workoutId(const uint meso_idx, const uint calendar_day) const;
	void setWorkoutId(const uint meso_idx, const uint calendar_day, const QString &new_workout_id);

	[[nodiscard]] const std::optional<QDate> date(const uint meso_idx, const uint calendar_day) const;
	void setDate(const uint meso_idx, const uint calendar_day, const QDate &new_date);

	[[nodiscard]] const std::optional<QString> workoutNumber(const uint meso_idx, const uint calendar_day) const;
	void setWorkoutNumber(const uint meso_idx, const uint calendar_day, const QString &new_number);

	[[nodiscard]] const std::optional<QString> splitLetter(const uint meso_idx, const uint calendar_day) const;
	void setSplitLetter(const uint meso_idx, const uint calendar_day, const QString &new_splitletter);

	[[nodiscard]] const std::optional<QTime> timeIn(const uint meso_idx, const uint calendar_day) const;
	void setTimeIn(const uint meso_idx, const uint calendar_day, const QTime &new_timein);

	[[nodiscard]] const std::optional<QTime> timeOut(const uint meso_idx, const uint calendar_day) const;
	void setTimeOut(const uint meso_idx, const uint calendar_day, const QTime &new_timeout);

	[[nodiscard]] const std::optional<QString> location(const uint meso_idx, const uint calendar_day) const;
	void setLocation(const uint meso_idx, const uint calendar_day, const QString &new_location);

	[[nodiscard]] const std::optional<QString> notes(const uint meso_idx, const uint calendar_day) const;
	void setNotes(const uint meso_idx, const uint calendar_day, const QString &new_notes);

	[[nodiscard]] const std::optional<bool> trainingCompleted(const uint meso_idx, const uint calendar_day) const;
	void setTrainingCompleted(const uint meso_idx, const uint calendar_day, const bool completed);

	Q_INVOKABLE inline DBCalendarModel *calendar(const uint meso_idx) const { return m_calendars.at(meso_idx); }
	Q_INVOKABLE inline DBWorkoutModel *workout(const uint calendar_day) const { return m_workouts.at(calendar_day); }

private:
	QList<DBWorkoutModel*> m_workouts;
	QList<DBCalendarModel*> m_calendars;

	inline std::optional<QString> dayInfo(const uint meso_idx, const uint calendar_day, const uint field) const;
	inline void setDayInfo(const uint meso_idx, const uint calendar_day, const uint field, const QString &new_value);
};

#endif // DBMESOCALENDARMODEL_H
