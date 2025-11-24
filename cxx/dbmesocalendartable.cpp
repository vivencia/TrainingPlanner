#include "dbmesocalendartable.h"

#include "dbcalendarmodel.h"

constexpr int n_fields{CALENDAR_DATABASE_TOTAL_FIELDS};
constexpr QLatin1StringView table_name{ "mesocycles_calendar_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1		"INTEGER PRIMARY KEY AUTOINCREMENT"_L1},
	{"meso_id"_L1,	"INTEGER"_L1},
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
	if (execQuery("SELECT * FROM %1 WHERE %2=%3;"_L1.arg(table_name,
						field_names[CALENDAR_DATABASE_MESOID][0], model->mesoId()), true, false))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList calendar_day{n_fields, QString{}};
				calendar_day[CALENDAR_DATABASE_ID] = std::move(m_workingQuery.value(CALENDAR_DATABASE_ID).toString());
				calendar_day[CALENDAR_DATABASE_MESOID] = model->mesoId();
				calendar_day[CALENDAR_DATABASE_DATE] = std::move(m_workingQuery.value(CALENDAR_DATABASE_DATE).toString());
				calendar_day[CALENDAR_DATABASE_DATA] = std::move(m_workingQuery.value(CALENDAR_DATABASE_DATA).toString());
				m_dbModelInterface->modelData().append(std::move(calendar_day));
			} while (m_workingQuery.next());
			success = true;
		}
	}
	emit calendarLoaded(model->mesoIdx(), success);
	return success;
}
