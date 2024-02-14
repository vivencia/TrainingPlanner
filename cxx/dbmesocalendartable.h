#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"
#include "dbmesocalendarmodel.h"

#include <QObject>
#include <QSettings>

static const QString DBMesoCalendarFileName ( QStringLiteral("MesoCalendar.db.sqlite") );
static const QString DBMesoCalendarObjectName ( QStringLiteral("MesoCalendar") );
static const uint MESOCALENDAR_TABLE_ID = 0x0004;

class DBMesoCalendarTable : public TPDatabaseTable
{

public:
	explicit DBMesoCalendarTable(const QString& dbFilePath, QSettings* appSettings, DBMesoCalendarModel* model = nullptr);

	void createTable();
	void getMesoCalendar();
	void createMesoCalendar();
	void newMesoCalendarEntry();
	void updateMesoCalendarEntry();
	void removeMesoCalendar();

	//Call before starting a thread
	void setData(const QString& id, const QString& mesoId = QString(), const QString& calDate = QString(),
						const QString& calNDay = QString(), const QString& calSplit = QString());
};

#endif // DBMESOCALENDARTABLE_H
