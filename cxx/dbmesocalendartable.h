#pragma once

#include "tpdatabasetable.h"

#include <QDate>
#include <QObject>

enum MesoCalendarDatabaseFields {
	CALENDAR_DATABASE_ID,
	CALENDAR_DATABASE_MESOID,
	CALENDAR_DATABASE_DATE,
	CALENDAR_DATABASE_DATA,
	CALENDAR_DATABASE_TOTAL_FIELDS
};

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar)

class DBMesoCalendarTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBMesoCalendarTable();
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getMesoCalendar(DBModelInterfaceCalendar *dbmi);

signals:
	void calendarLoaded(const uint meso_idx, const bool success);
};
