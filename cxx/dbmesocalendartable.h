#ifndef DBMESOCALENDARTABLE_H
#define DBMESOCALENDARTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoCalendarModel;

static const QString DBMesoCalendarFileName(u"MesoCalendar.db.sqlite"_qs);

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
	void removeMesoCalendar();

private:
	DBMesoCalendarModel* m_model;
};

#endif // DBMESOCALENDARTABLE_H
