#pragma once

#include "tpdatabasetable.h"

#include <QDate>
#include <QObject>

#define CALENDAR_DATABASE_MESOID 0
#define CALENDAR_DATABASE_DATE 1
#define CALENDAR_DATABASE_DATA 2
#define CALENDAR_DATABASE_TOTAL_FIELDS 3

QT_FORWARD_DECLARE_CLASS(DBCalendarModel);
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar)

class DBMesoCalendarTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBMesoCalendarTable();
	QString dbFilePath() const override final;
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getMesoCalendar();
	std::pair<QVariant, QVariant> mesoCalendarSavedInDB();

signals:
	void calendarLoaded(const uint meso_idx, const bool success);
};
