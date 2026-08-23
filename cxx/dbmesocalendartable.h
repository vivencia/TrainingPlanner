#pragma once

#include "tpdatabasetable.h"

#include <QDate>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar)

class DBMesoCalendarTable final : public TPDatabaseTable
{

Q_OBJECT

public:

	enum MesoCalendarDatabaseFields {
		CALDB_ID,
		CALDB_MESOID,
		CALDB_DATE,
		CALDB_DATA,
		CALDB_TOTAL_FIELDS
	};

	explicit DBMesoCalendarTable();
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getMesoCalendar(DBModelInterfaceCalendar *dbmi);
	std::pair<QVariant,QVariant> removeMesoCalendar(const QString &mesoid);

signals:
	void calendarLoaded(const uint meso_idx, const bool success);
};
