#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoCalendarModel;

static const QString& DBMesoCalendarFileName("MesoCalendar.db.sqlite"_L1);

struct st_workoutDayInfo {
	int meso_id;
	QString trainingDay;
	QString splitLetter;
	bool completed;
	QDate date;

	explicit inline st_workoutDayInfo(): meso_id{-1}, trainingDay{" - "_L1}, splitLetter{'-'}, completed(false) {}
};

class DBMesoCalendarTable final : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(const QString& dbFilePath, DBMesoCalendarModel* model = nullptr);
	inline ~DBMesoCalendarTable() { clearWorkoutsInfoList(); }

	void createTable() override final;
	void updateTable() override final;
	void getMesoCalendar();
	void saveMesoCalendar();
	void updateMesoCalendarEntry();
	void updateDayIsFinished();
	void dayInfo(const QDate& date, QStringList& dayInfoList);
	void changeMesoCalendar();
	void updateMesoCalendar();

	void workoutDayInfoForEntireMeso();
	inline const QList<st_workoutDayInfo*>& workoutsInfo() const { return m_workoutsInfoList; }

	//Functions for TPStatistics
	void completedDaysForSplitWithinTimePeriod();
	inline const QList<QDate>& retrievedDates() const { return m_completedWorkoutDates; }

private:
	DBMesoCalendarModel* m_model;
	QList<QDate> m_completedWorkoutDates;
	QList<st_workoutDayInfo*> m_workoutsInfoList;

	inline void clearWorkoutsInfoList() { qDeleteAll(m_workoutsInfoList); m_workoutsInfoList.clear(); }
};

#endif // DBMESOCALENDARTABLE_H
