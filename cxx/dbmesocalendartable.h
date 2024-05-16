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
	void getMesoCalendar();
	void createMesoCalendar();
	void updateMesoCalendarEntry();
	virtual void updateFromModel();
	void changeMesoCalendar();
	void updateMesoCalendar();
	void removeMesoCalendar();

	//Call before starting a thread
	void setData(const QString& mesoId, const QString& calNDay, const QString& calSplit);
};

#endif // DBMESOCALENDARTABLE_H
