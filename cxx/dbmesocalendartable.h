#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBMesoCalendarModel;

static const QString DBMesoCalendarFileName ( QStringLiteral("MesoCalendar.db.sqlite") );

class DBMesoCalendarTable : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(const QString& dbFilePath, QSettings* appSettings, DBMesoCalendarModel* model = nullptr);

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

	//Call before starting a thread
	void setData(const QString& calNDay, const QString& calSplit, const QString& dayIsFinished);
};

#endif // DBMESOCALENDARTABLE_H
