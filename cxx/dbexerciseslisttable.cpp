#include "dbexerciseslisttable.h"

#include "dbexerciseslistmodel.h"
#include "tpsettings.h"
#include "tputils.h"

constexpr int n_fields{6};
constexpr QLatin1StringView table_name{ "exercises_table"_L1 };
constexpr QLatin1StringView field_names[n_fields][2] {
	{"id"_L1,				"INTEGER PRIMARY KEY"_L1},
	{"primary_name"_L1,		"TEXT"_L1},
	{"secondary_name"_L1,	"TEXT"_L1},
	{"muscular_group"_L1,	"TEXT"_L1},
	{"media_path"_L1,		"TEXT"_L1},
	{"from_list"_L1,		"INTEGER"_L1},
};

DBExercisesListTable::DBExercisesListTable(DBModelInterfaceExercisesList* dbmodel_interface, const QString &list_version)
	: TPDatabaseTable{EXERCISES_TABLE_ID, dbmodel_interface}
{
	m_tableName = &table_name;
	m_fieldNames = field_names;
	m_fieldCount = n_fields;
	setUpConnection();
	#ifndef QT_NO_DEBUG
	setObjectName("ExercisesListTable");
	#endif
	m_updateList = list_version != appSettings()->exercisesListVersion();
	setReadAllRecordsFunc([this] () { return getAllExercises(); });
}

QString DBExercisesListTable::dbFileName(const bool fullpath) const
{
	const QString &filename{std::move("ExercisesList"_L1 + dbfile_extension)};
	return fullpath ? dbFilePath() + filename : filename;
}

bool DBExercisesListTable::getAllExercises()
{
	bool success{false};
	if (m_updateList)
		success = updateExercisesList();
	else if (execReadOnlyQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(table_name)))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList data{EXERCISES_TOTAL_COLS};
				for (uint i{EXERCISES_LIST_COL_ID}; i < EXERCISES_LIST_COL_ACTUALINDEX; ++i)
					data[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				data[EXERCISES_LIST_COL_ACTUALINDEX] = std::move(QString::number(m_dbModelInterface->modelData().count()));
				data[EXERCISES_LIST_COL_SELECTED] = std::move("0"_L1);
				m_dbModelInterface->modelData().append(std::move(data));
			} while (m_workingQuery.next ());
			success = true;
		}
		else //for whatever reason the database table is empty. Populate it with the app provided exercises list
		{
			m_sqlLiteDB.close();
			success = updateExercisesList();
		}
	}
	return success;
}

bool DBExercisesListTable::updateExercisesList()
{
	bool success{false};
	if (getExercisesList())
	{
		//remove previous list entries from DB
		if (execSingleWriteQuery("DELETE FROM %1 WHERE %2=1;"_L1.arg(table_name, field_names[EXERCISES_LIST_COL_FROMAPPLIST][0])))
		{
			auto model{m_dbModelInterface->model<DBExercisesListModel>()};
			for (const auto &data : std::as_const(m_exercisesList))
			{
				QStringList fields{std::move(data.split(';'))};
				model->newExerciseFromList(std::move(fields[0]), std::move(fields[1]), std::move(fields.at(2).trimmed()));
			}
			success = true;
		}
	}
	return success;
}

bool DBExercisesListTable::getExercisesList()
{
	QFile *exercises_list_file{appUtils()->openFile(":/extras/exerciseslist.lst"_L1)};
	if (exercises_list_file)
	{
		m_exercisesList.reserve(304);
		QString line{1024, QChar{0}};
		QTextStream stream{exercises_list_file};
		stream.readLineInto(&line); //skip first line
		while (stream.readLineInto(&line))
			m_exercisesList.append(std::move(line));
		exercises_list_file->close();
		delete exercises_list_file;
	}
	if (m_exercisesList.isEmpty())
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "****** ERROR ******";
		qDebug() << "DBExercisesListTable::updateExercisesList -> m_exercisesList is empty"_L1;
		#endif
		return false;
	}
	else
		return true;
}
