#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoCalendarModel;

static const QString DBMesoCalendarFileName(u"MesoCalendar.db.sqlite"_qs);

class DBMesoCalendarTable : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(const QString& dbFilePath, DBMesoCalendarModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase();
	void getMesoCalendar();
	void saveMesoCalendar();
	void updateMesoCalendarEntry();
	void updateDayIsFinished();
	void dayInfo(const QDate& date, QStringList& dayInfoList);
	void changeMesoCalendar();
	void updateMesoCalendar();
	void removeMesoCalendar();
};

#endif // DBMESOCALENDARTABLE_H
