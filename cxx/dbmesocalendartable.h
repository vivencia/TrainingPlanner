#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoCalendarModel;

static const QString& DBMesoCalendarFileName("MesoCalendar.db.sqlite"_L1);

class DBMesoCalendarTable final : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(const QString& dbFilePath, DBMesoCalendarModel* model = nullptr);

	void createTable() override;
	void updateTable() override;
	void getMesoCalendar();
	void saveMesoCalendar();
	void updateMesoCalendarEntry();
	void updateDayIsFinished();
	void dayInfo(const QDate& date, QStringList& dayInfoList);
	void changeMesoCalendar();
	void updateMesoCalendar();

	//Functions for TPStatistics
	void completedDaysForSplitWithinTimePeriod();
	inline const QList<QDate>& retrievedDates() const { return m_completedWorkoutDates; }

private:
	DBMesoCalendarModel* m_model;
	QList<QDate> m_completedWorkoutDates;
};

#endif // DBMESOCALENDARTABLE_H
