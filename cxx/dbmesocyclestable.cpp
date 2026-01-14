#include "dbmesocyclestable.h"

#include "dbmesocyclesmodel.h"

constexpr int n_fields{MESO_TOTAL_FIELDS};
constexpr QLatin1StringView table_name{ "mesocycles_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1,					"INTEGER PRIMARY KEY AUTOINCREMENT"_L1},
	{"meso_name"_L1,			"TEXT"_L1},
	{"meso_start_date"_L1,		"INTEGER"_L1},
	{"meso_end_date"_L1,		"INTEGER"_L1},
	{"meso_note"_L1,			"TEXT"_L1},
	{"meso_nweeks"_L1,			"INTEGER"_L1},
	{"meso_split"_L1,			"TEXT"_L1},
	{"splitA"_L1,				"TEXT"_L1},
	{"splitB"_L1,				"TEXT"_L1},
	{"splitC"_L1,				"TEXT"_L1},
	{"splitD"_L1,				"TEXT"_L1},
	{"splitE"_L1,				"TEXT"_L1},
	{"splitF"_L1,				"TEXT"_L1},
	{"meso_coach"_L1,			"INTEGER"_L1},
	{"meso_client"_L1,			"INTEGER"_L1},
	{"meso_program_file"_L1,	"TEXT"_L1},
	{"meso_type"_L1,			"TEXT"_L1},
	{"real_meso"_L1,			"INTEGER"_L1},
};

DBMesocyclesTable::DBMesocyclesTable(DBModelInterfaceMesocycle *dbmodel_interface)
	: TPDatabaseTable{MESOCYCLES_TABLE_ID, dbmodel_interface}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_DEBUG
	setObjectName("MesocyclesTable");
	#endif
	setReadAllRecordsFunc<void>([this] (void *param) { return getAllMesocycles(param); });
}

QString DBMesocyclesTable::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move("Mesocycles"_L1 + dbfile_extension)};
	return fullpath ? dbFilePath() + filename : filename;
}

bool DBMesocyclesTable::getAllMesocycles(void *)
{
	bool success{false};
	if (execReadOnlyQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(table_name)))
	{
		if (m_workingQuery.first ())
		{
			do
			{
				QStringList meso_info{MESO_TOTAL_FIELDS};
				for (uint i{MESO_FIELD_ID}; i < MESO_TOTAL_FIELDS; ++i)
					meso_info[i] = std::move(m_workingQuery.value(i).toString());
				emit mesocyclesAcquired(meso_info, false);
			} while (m_workingQuery.next());
			success = true;
		}
	}
	emit mesocyclesAcquired(QStringList{}, true);
	return success;
}
