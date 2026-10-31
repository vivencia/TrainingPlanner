#include "dbexerciseslisttable.h"

#include "dbexerciseslistmodel.h"

constexpr int n_fields{DBExercisesListModel::N_FIELDS};
constexpr QLatin1StringView table_name{ "exercises_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1,				"INTEGER PRIMARY KEY AUTOINCREMENT"_L1},
	{"primary_name"_L1,		"TEXT"_L1},
	{"secondary_name"_L1,	"TEXT"_L1},
	{"muscular_group"_L1,	"TEXT"_L1},
	{"media_path"_L1,		"TEXT"_L1},
};

DBExercisesListTable::DBExercisesListTable(DBModelInterfaceExercisesList *dbmi)
	: TPDatabaseTable{EXERCISES_TABLE_ID}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_DEBUG
	setObjectName("ExercisesListTable");
	#endif
	setReadAllRecordsFunc([this,dbmi] (DBModelInterface*) { return getAllExercises(dbmi); });
}

QString DBExercisesListTable::dbFileName(const bool fullpath) const
{
	const QString &filename{"ExercisesList"_L1 % dbfile_extension};
	return fullpath ? dbFilePath() % filename : filename;
}

bool DBExercisesListTable::getAllExercises(void *data)
{
	bool success{false};
	if (execReadOnlyQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(table_name))) {
		if (m_workingQuery.first()) {
			DBModelInterfaceExercisesList *dbmi{static_cast<DBModelInterfaceExercisesList*>(data)};
			do {
				QStringList data{n_fields};
				for (uint i{DBExercisesListModel::ID}; i < n_fields; ++i)
					data[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				dbmi->modelData().append(std::move(data));
			} while (m_workingQuery.next ());
			success = true;
		}
	}
	return success;
}

