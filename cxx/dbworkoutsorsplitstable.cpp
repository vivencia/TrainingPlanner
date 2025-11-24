#include "dbworkoutsorsplitstable.h"

#include "dbexercisesmodel.h"

constexpr int n_fields{EXERCISES_TOTALCOLS};
constexpr QLatin1StringView table_name{ "workouts_table"_L1 };
constexpr QLatin1StringView table_name_2{ "mesosplit_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1,					"INTEGER PRIMARY KEY AUTOINCREMENT"_L1},
	{"meso_id"_L1,				"INTEGER"_L1},
	{"calendar_day"_L1,			"INTEGER"_L1},
	{"split_letter"_L1,			"INTEGER"_L1},
	{"track_rest_time"_L1,		"TEXT"_L1},
	{"auto_rest_time"_L1,		"TEXT"_L1},
	{"exercises"_L1,			"TEXT"_L1},
	{"setsnotes"_L1,			"TEXT"_L1},
	{"setscompleted"_L1,		"INTEGER"_L1},
	{"setstypes"_L1,			"TEXT"_L1},
	{"setsresttimes"_L1,		"TEXT"_L1},
	{"setssubsets"_L1,			"TEXT"_L1},
	{"setsreps"_L1,				"TEXT"_L1},
	{"setsweights"_L1,			"TEXT"_L1},
};

using namespace Qt::Literals::StringLiterals;

DBWorkoutsOrSplitsTable::DBWorkoutsOrSplitsTable(const uint tableid)
	: TPDatabaseTable{tableid}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_DEBUG
	setObjectName(tableId() == WORKOUT_TABLE_ID ? "WorkoutsTable"_L1 : "SplitTable"_L1);
	#endif
	setReadAllRecordsFunc([this] () { return getExercises(); });
}

QString DBWorkoutsOrSplitsTable::dbFileName(const bool fullpath) const
{
	const QString &filename{(tableId() == WORKOUT_TABLE_ID ? "Workouts"_L1 : "MesocyclesSplits"_L1) + dbfile_extension};
	return fullpath ? dbFilePath() + filename : filename;
}

bool DBWorkoutsOrSplitsTable::getExercises()
{
	bool success{false};
	auto model{m_dbModelInterface->model<DBExercisesModel>()};

	m_strQuery = std::move(tableId() == WORKOUT_TABLE_ID ?
					"SELECT * FROM %1 WHERE %2=%3 AND %4=%5;"_L1.arg(
								table_name, field_names[EXERCISES_COL_MESOID][0], model->mesoId(),
								field_names[EXERCISES_COL_CALENDARDAY][0], QString::number(model->calendarDay())) :
					"SELECT * FROM %1 WHERE %2=%3 AND %4=%5;"_L1.arg(
								table_name, field_names[EXERCISES_COL_MESOID][0], model->mesoId(),
								field_names[EXERCISES_COL_SPLITLETTER][0], model->splitLetter())
	);
	if (execQuery(m_strQuery, true, false))
	{
		if (m_workingQuery.first())
		{
			QStringList exercises{EXERCISES_TOTALCOLS};
			for (uint i{EXERCISES_COL_ID}; i < EXERCISES_TOTALCOLS; ++i)
				exercises[i] = std::move(m_workingQuery.value(i).toString());
			m_dbModelInterface->modelData().append(std::move(exercises));
			success = true;
		}
	}
	emit exercisesLoaded(model->mesoIdx(), success,
				tableId() == WORKOUT_TABLE_ID ? QVariant{model->calendarDay()} : QVariant{model->splitLetter()});
	return success;
}

std::pair<QVariant,QVariant> DBWorkoutsOrSplitsTable::mesoHasAllSplitPlans(const QString &meso_id, const QString &split)
{
	bool success{false};
	bool yes{false};
	auto model{m_dbModelInterface->model<DBExercisesModel>()};
	m_strQuery = std::move("SELECT %1 FROM %2 WHERE %3=%4 AND %5=\'%6\';"_L1.arg(
		field_names[EXERCISES_COL_SETTYPES][0], table_name,
		field_names[EXERCISES_COL_MESOID][0], model->mesoId(),
		field_names[EXERCISES_COL_SPLITLETTER][0], model->splitLetter()));
	for (const auto &split_letter : split)
	{
		if (split_letter.cell() >= 'A' && split_letter.cell() <= 'F')
		{
			if (execQuery(m_strQuery.arg(split_letter), true, false))
			{
				if (m_workingQuery.first())
				{
					success = true;
					static_cast<void>(m_workingQuery.value(0).toInt(&yes));
					if (!yes)
						break;
				}
				else
					break;
			}
		}
	}
	return std::pair<QVariant,QVariant>{success, yes};
}

std::pair<QVariant,QVariant> DBWorkoutsOrSplitsTable::mesoHasSplitPlan()
{
	bool success{false};
	bool yes{false};
	auto model{m_dbModelInterface->model<DBExercisesModel>()};
	m_strQuery = std::move("SELECT %1 FROM %2 WHERE %3=%4 AND %5=\'%6\';"_L1.arg(
			field_names[EXERCISES_COL_SETTYPES][0], table_name,
			field_names[EXERCISES_COL_MESOID][0], model->mesoId(),
			field_names[EXERCISES_COL_SPLITLETTER][0], model->splitLetter()));
	if (execQuery(m_strQuery, true, false))
	{
		if (m_workingQuery.first())
		{
			success = true;
			m_workingQuery.value(0).toUInt(&yes);
		}
	}
	return std::pair<QVariant,QVariant>{success, yes};
}

std::pair<QVariant,QVariant> DBWorkoutsOrSplitsTable::getPreviousWorkoutsIds()
{
	auto model{m_dbModelInterface->model<DBExercisesModel>()};
	m_strQuery = std::move("SELECT %1 FROM %2 WHERE %3=%4 AND %5=\'%6\' "
					"AND %7<%8 ORDER BY %1 DESC LIMIT 5;"_L1.arg(
					field_names[EXERCISES_COL_CALENDARDAY][0], table_name,
					field_names[EXERCISES_COL_MESOID][0], model->mesoId(),
					field_names[EXERCISES_COL_SPLITLETTER][0], model->splitLetter(),
					field_names[EXERCISES_COL_CALENDARDAY][0], QString::number(model->calendarDay())));
	if (execQuery(m_strQuery, true, false))
	{
		if (m_workingQuery.first())
		{
			QVariantList ids;
			do {
				ids.append(m_workingQuery.value(0).toUInt());
			} while (m_workingQuery.next());
			return std::pair<QVariant,QVariant>{true, ids};
		}
	}
	return std::pair<QVariant,QVariant>{false, false};
}
