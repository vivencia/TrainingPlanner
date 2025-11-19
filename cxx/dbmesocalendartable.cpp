#include "dbmesocalendartable.h"

#include "dbcalendarmodel.h"

constexpr int n_fields{CALENDAR_DATABASE_TOTAL_FIELDS};
constexpr QLatin1StringView table_name{ "mesocycles_calendar_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"meso_id"_L1,	"INTEGER PRIMARY KEY"_L1},
	{"date"_L1,		"TEXT"_L1},
	{"data"_L1,		"TEXT"_L1},
};

DBMesoCalendarTable::DBMesoCalendarTable()
	: TPDatabaseTable{MESOCALENDAR_TABLE_ID}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_QDEBUG
	setObjectName("MesoCalendarTable");
	#endif
	setReadAllRecordsFunc([this] () { return getMesoCalendar(); });
}

QString DBMesoCalendarTable::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move("MesoCalendar"_L1 + dbfile_extension)};
	return fullpath ? dbFilePath() + filename : filename;
}

bool DBMesoCalendarTable::getMesoCalendar()
{
	bool success{false};
	auto model{m_dbModelInterface->model<DBCalendarModel>()};
	if (execQuery("SELECT %1,%2 FROM %3 WHERE %4=%5;"_L1.arg(field_names[CALENDAR_DATABASE_DATE][0],
		field_names[CALENDAR_DATABASE_DATA][0], table_name, field_names[CALENDAR_DATABASE_MESOID][0], model->mesoId()), true, false))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList calendar_day{n_fields, QString{}};
				calendar_day[CALENDAR_DATABASE_MESOID] = model->mesoId();
				calendar_day[CALENDAR_DATABASE_DATE] = std::move(m_workingQuery.value(0).toString());
				calendar_day[CALENDAR_DATABASE_DATA] = std::move(m_workingQuery.value(1).toString());
				m_dbModelInterface->modelData().append(std::move(calendar_day));
			} while (m_workingQuery.next());
			success = true;
		}
	}
	emit calendarLoaded(model->mesoIdx(), success);
	return success;
}

std::pair<QVariant,QVariant> DBMesoCalendarTable::mesoCalendarSavedInDB()
{
	bool success{false};
	auto model{m_dbModelInterface->model<DBCalendarModel>()};
	if (execQuery("SELECT %1 FROM %2 WHERE %1=%3;"_L1.arg(table_name, field_names[CALENDAR_DATABASE_MESOID][0],
					 model->mesoId()), true, false))
	{
		success = m_workingQuery.first();
		m_sqlLiteDB.close();
	}
	return std::pair<QVariant,QVariant>{success, success};
}
