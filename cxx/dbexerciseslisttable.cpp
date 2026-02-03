#include "dbexerciseslisttable.h"

#include "dbexerciseslistmodel.h"

constexpr int n_fields{EXERCISES_TOTAL_COLS};
constexpr QLatin1StringView table_name{ "exercises_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1,				"INTEGER PRIMARY KEY AUTOINCREMENT"_L1},
	{"primary_name"_L1,		"TEXT"_L1},
	{"secondary_name"_L1,	"TEXT"_L1},
	{"muscular_group"_L1,	"TEXT"_L1},
	{"media_path"_L1,		"TEXT"_L1},
	{"from_list"_L1,		"INTEGER"_L1},
	{"actual_index"			"INTEGER"_L1},
	{"selected"_L1			"INTEGER"_L1},
};

DBExercisesListTable::DBExercisesListTable(DBModelInterfaceExercisesList* dbmodel_interface)
	: TPDatabaseTable{EXERCISES_TABLE_ID, dbmodel_interface}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_DEBUG
	setObjectName("ExercisesListTable");
	#endif
	setReadAllRecordsFunc<void>([this] (void *param) { return getAllExercises(param); });
}

QString DBExercisesListTable::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move("ExercisesList"_L1 % dbfile_extension)};
	return fullpath ? dbFilePath() % filename : filename;
}

bool DBExercisesListTable::getAllExercises(void *)
{
	bool success{false};
	if (execReadOnlyQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(table_name)))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList data{EXERCISES_TOTAL_COLS};
				for (uint i{EXERCISES_LIST_FIELD_ID}; i < EXERCISES_LIST_FIELD_ACTUALINDEX; ++i)
					data[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				data[EXERCISES_LIST_FIELD_ACTUALINDEX] = std::move(QString::number(m_dbModelInterface->modelData().count()));
				data[EXERCISES_LIST_FIELD_SELECTED] = std::move("0"_L1);
				m_dbModelInterface->modelData().append(std::move(data));
			} while (m_workingQuery.next ());
			success = true;
		}
	}
	return success;
}

