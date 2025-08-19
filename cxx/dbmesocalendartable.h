#pragma once

#include "tpdatabasetable.h"

#include <QDate>
#include <QObject>

class DBMesoCalendarManager;

struct st_workoutDayInfo {
	int meso_id;
	QString trainingDay;
	QString splitLetter;
	bool completed;
	QDate date;

	explicit inline st_workoutDayInfo(): meso_id{-1}, trainingDay{" - "_L1}, splitLetter{'-'}, completed{false} {}
};

class DBMesoCalendarTable final : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(DBMesoCalendarManager *model);
	inline ~DBMesoCalendarTable() { clearWorkoutsInfoList(); }

	static QLatin1StringView tableName();
	static QLatin1StringView createTableQuery();

	void updateTable() override final {}
	void getMesoCalendar();
	void saveMesoCalendar();
	bool mesoCalendarSavedInDB(const QString &meso_id);

	void workoutDayInfoForEntireMeso();
	inline const QList<st_workoutDayInfo*> &workoutsInfo() const { return m_workoutsInfoList; }

	//Functions for TPStatistics
	void completedDaysForSplitWithinTimePeriod();
	inline const QList<QDate> &retrievedDates() const { return m_completedWorkoutDates; }

private:
	DBMesoCalendarManager *m_model;
	QList<QDate> m_completedWorkoutDates;
	QList<st_workoutDayInfo*> m_workoutsInfoList;

	inline void clearWorkoutsInfoList() { qDeleteAll(m_workoutsInfoList); m_workoutsInfoList.clear(); }
};
