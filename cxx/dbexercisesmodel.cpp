#include "dbexercisesmodel.h"

#include "dbexerciseslistmodel.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "translationclass.h"

#include <QApplication>
#include <QtMath>

#include <utility>
#include <ranges>

#define UNSET_VALUE 11111

QT_FORWARD_DECLARE_STRUCT(stExercise)

struct stSet {
	uint set_number, mode;
	TPSetTypes type;
	QTime restTime;
	QString subsets, reps, weight, notes;
	TPBool completed;
	stExercise *parent;

	inline explicit stSet() : mode{0}, type{Unkown}, subsets{'0'}, notes{' '} {}
};

struct stExercise {
	QString name;
	uint exercise_idx, working_set;
	QList<stSet*> sets;
	exerciseEntry *parent;

	inline explicit stExercise() : working_set{UNSET_VALUE} {}
};

struct exerciseEntry {
	QList<stExercise*> m_exercises;
	TPBool track_rest_time, auto_rest_time;
	uint exercise_number, working_subexercise;

	inline explicit exerciseEntry() : working_subexercise{UNSET_VALUE} {}
};

enum RoleNames {
	exerciseNumberRole = Qt::UserRole,
	giantSetExerciseRole = Qt::UserRole+1,
	setsNumberRole = Qt::UserRole+2,
	exerciseCompletedRole = Qt::UserRole+3,
	workingExerciseRole = Qt::UserRole+4,
	workingSubExerciseRole = Qt::UserRole+5,
	workingSetRole = Qt::UserRole+6,
	trackRestTimeRole = Qt::UserRole+7,
	autoRestTimeRole = Qt::UserRole+8
};

static const QString &calendarDayExtraInfo{qApp->tr(" Workout #: ")};

void DBExercisesModel::operator=(DBExercisesModel *other_model)
{
	m_mesoId = other_model->mesoId();
	m_mesoIdx = other_model->mesoIdx();
	m_splitLetter = other_model->splitLetter();
	for (const auto exercise_entry : std::as_const(other_model->m_exerciseData))
	{
		const uint exercise_number{addExercise(false)};
		m_exerciseData.at(exercise_number)->exercise_number = exercise_entry->exercise_number;
		m_exerciseData.at(exercise_number)->track_rest_time = exercise_entry->track_rest_time;
		m_exerciseData.at(exercise_number)->auto_rest_time = exercise_entry->auto_rest_time;
		for (const auto sub_exercise : std::as_const(exercise_entry->m_exercises))
		{
			const uint exercise_idx{addSubExercise(exercise_number, false)};
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = sub_exercise->name;
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->exercise_idx = sub_exercise->exercise_idx;
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->working_set = sub_exercise->working_set;
			for (const auto set : std::as_const(sub_exercise->sets))
			{
				const uint set_number{addSet(exercise_number, exercise_idx, false)};
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->set_number = set->set_number;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type = set->type;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime = set->restTime;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->subsets = set->subsets;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps = set->reps;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight = set->weight;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes = set->notes;
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed = set->completed;
			}
		}
	}
	setWorkingExercise(0);
	setWorkingSubExercise(0, 0);
	setWorkingSet(0, 0, 0);
}

bool DBExercisesModel::fromDataBase(const QStringList &data, const bool bClearSomeFieldsForReUse)
{
	m_id = std::move(data.at(EXERCISES_COL_ID));
	m_mesoId = std::move(data.at(EXERCISES_COL_MESOID));
	m_calendarDay = data.at(EXERCISES_COL_CALENDARDAY).toInt();
	m_splitLetter = data.at(EXERCISES_COL_SPLITLETTER).at(0);

	const QStringList &exercises(data.at(EXERCISES_COL_EXERCISES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &track_resttimes(data.at(EXERCISES_COL_TRACKRESTTIMES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &auto_resttimes(data.at(EXERCISES_COL_AUTORESTTIMES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &settypes(data.at(EXERCISES_COL_SETTYPES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &resttimes(data.at(EXERCISES_COL_RESTTIMES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &subsets(data.at(EXERCISES_COL_SUBSETS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &reps(data.at(EXERCISES_COL_REPS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &weights(data.at(EXERCISES_COL_WEIGHTS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList &notes(data.at(EXERCISES_COL_NOTES).split(exercises_separator, Qt::SkipEmptyParts));

	QStringList completed;
	if (!bClearSomeFieldsForReUse)
		completed = std::move(data.at(EXERCISES_COL_COMPLETED).split(exercises_separator, Qt::SkipEmptyParts));
	else //reset for new use
	{
		QString temp{data.at(EXERCISES_COL_COMPLETED)};
		completed = std::move(temp.replace(STR_ONE, STR_ZERO).split(exercises_separator, Qt::SkipEmptyParts));
	}

	for(uint exercise_number{0}; exercise_number < exercises.count(); ++exercise_number)
	{
		beginInsertRows(QModelIndex{}, exercise_number, exercise_number);
		static_cast<void>(addExercise(false));
		exerciseEntry *exercise_entry{m_exerciseData.at(exercise_number)};
		if (!track_resttimes.isEmpty())
			exercise_entry->track_rest_time = track_resttimes.at(exercise_number) == "1"_L1;
		if (!auto_resttimes.isEmpty())
			exercise_entry->auto_rest_time = auto_resttimes.at(exercise_number) == "1"_L1;

		const uint n_subexercises{appUtils()->nFieldsInCompositeString(exercises.at(exercise_number), comp_exercise_separator)};
		for (uint exercise_idx{0}; exercise_idx < n_subexercises; ++exercise_idx)
		{
			addSubExercise(exercise_number, false);
			stExercise *sub_exercise{exercise_entry->m_exercises.at(exercise_idx)};
			QString exercise_name{};
			if (exercise_idx < exercises.count())
			{
				exercise_name = std::move(appUtils()->getCompositeValue(exercise_idx, exercises.at(exercise_number), comp_exercise_separator));
				sub_exercise->name = std::move(exercise_name);
			}
			uint n_sets{0};
			QString set_types;
			if (exercise_idx < settypes.count())
			{
				set_types = std::move(appUtils()->getCompositeValue(exercise_idx, settypes.at(exercise_number), comp_exercise_separator));
				n_sets = appUtils()->nFieldsInCompositeString(set_types, set_separator);
			}
			if (n_sets == 0)
				continue;

			QString rest_times;
			if (exercise_entry->track_rest_time)
			{
				if (exercise_idx < resttimes.count())
					rest_times = std::move(appUtils()->getCompositeValue(exercise_idx, resttimes.at(exercise_number), comp_exercise_separator));
			}
			const QString &sub_sets{appUtils()->getCompositeValue(exercise_idx, subsets.at(exercise_number), comp_exercise_separator)};
			const QString &set_reps{appUtils()->getCompositeValue(exercise_idx, reps.at(exercise_number), comp_exercise_separator)};
			const QString &set_weights{appUtils()->getCompositeValue(exercise_idx, weights.at(exercise_number), comp_exercise_separator)};
			const QString &set_notes{appUtils()->getCompositeValue(exercise_idx, notes.at(exercise_number), comp_exercise_separator)};
			const QString &set_completed{appUtils()->getCompositeValue(exercise_idx, completed.at(exercise_number), comp_exercise_separator)};

			for (uint set_number{0}; set_number < n_sets; ++set_number)
			{
				static_cast<void>(addSet(exercise_number, exercise_idx, false));
				stSet *set{sub_exercise->sets.at(set_number)};
				set->type = static_cast<TPSetTypes>(appUtils()->getCompositeValue(set_number, set_types, set_separator).toUInt());
				if (!rest_times.isEmpty())
				{
					set->restTime = std::move(appUtils()->getTimeFromTimeString(
						appUtils()->getCompositeValue(set_number, rest_times, set_separator), TPUtils::TF_QML_DISPLAY_NO_SEC));
				}
				if (!sub_sets.isEmpty())
					set->subsets = std::move(appUtils()->getCompositeValue(set_number, sub_sets, set_separator));
				if (!set_reps.isEmpty())
					set->reps = std::move(appUtils()->getCompositeValue(set_number, set_reps, set_separator));
				if (!set_weights.isEmpty())
					set->weight = std::move(appUtils()->getCompositeValue(set_number, set_weights, set_separator));
				if (!set_notes.isEmpty())
					set->notes = std::move(appUtils()->getCompositeValue(set_number, set_notes, set_separator));
				if (!set_completed.isEmpty())
					set->completed = appUtils()->getCompositeValue(set_number, set_completed, set_separator) == "1"_L1;
				if (isWorkout())
					setModeForSet(set);
			}
		}
		endInsertRows();
	}
	if (m_exerciseData.count() > 0)
	{
		setWorkingExercise(0);
		setWorkingSubExercise(0, 0);
		setWorkingSet(0, 0, 0);
		emit exerciseCountChanged();
		emit dataChanged(index(0, 0), index(m_exerciseData.count()-1, 0));
		return true;
	}
	return false;
}

const QStringList DBExercisesModel::toDatabase(const bool to_export_file) const
{
	QStringList data{WORKOUT_TOTALCOLS};
	data[EXERCISES_COL_ID] = !to_export_file ? m_id : "-1"_L1;
	data[EXERCISES_COL_MESOID] = !to_export_file ? m_mesoId : "-1"_L1;
	data[EXERCISES_COL_CALENDARDAY] = std::move(QString::number(m_calendarDay));
	data[EXERCISES_COL_SPLITLETTER] = m_splitLetter;

	for (const auto exercise_entry : std::as_const(m_exerciseData))
	{
		data[EXERCISES_COL_TRACKRESTTIMES].append((exercise_entry->track_rest_time ? "1"_L1 : "0"_L1) + exercises_separator);
		data[EXERCISES_COL_AUTORESTTIMES].append((exercise_entry->auto_rest_time ? "1"_L1 : "0"_L1) + exercises_separator);
		for (const auto sub_exercise : std::as_const(exercise_entry->m_exercises))
		{
			data[EXERCISES_COL_EXERCISES].append(sub_exercise->name + comp_exercise_separator);
			for (const auto set : std::as_const(sub_exercise->sets))
			{
				data[EXERCISES_COL_SETTYPES].append(QString::number(set->type) + set_separator);
				data[EXERCISES_COL_RESTTIMES].append(appUtils()->formatTime(set->restTime) + set_separator);
				data[EXERCISES_COL_SUBSETS].append(set->subsets + set_separator);
				data[EXERCISES_COL_REPS].append(set->reps + set_separator);
				data[EXERCISES_COL_WEIGHTS].append(set->weight + set_separator);
				data[EXERCISES_COL_NOTES].append(set->notes + set_separator);
				data[EXERCISES_COL_COMPLETED].append((set->completed ? "1"_L1 : "0"_L1) + set_separator);
			}
			data[EXERCISES_COL_SETTYPES].append(comp_exercise_separator);
			data[EXERCISES_COL_RESTTIMES].append(comp_exercise_separator);
			data[EXERCISES_COL_SUBSETS].append(comp_exercise_separator);
			data[EXERCISES_COL_REPS].append(comp_exercise_separator);
			data[EXERCISES_COL_WEIGHTS].append(comp_exercise_separator);
			data[EXERCISES_COL_NOTES].append(comp_exercise_separator);
			data[EXERCISES_COL_COMPLETED].append(comp_exercise_separator);
		}
		data[EXERCISES_COL_EXERCISES].append(exercises_separator);
		data[EXERCISES_COL_SETTYPES].append(exercises_separator);
		data[EXERCISES_COL_RESTTIMES].append(exercises_separator);
		data[EXERCISES_COL_SUBSETS].append(exercises_separator);
		data[EXERCISES_COL_REPS].append(exercises_separator);
		data[EXERCISES_COL_WEIGHTS].append(exercises_separator);
		data[EXERCISES_COL_NOTES].append(exercises_separator);
		data[EXERCISES_COL_COMPLETED].append(exercises_separator);
	}
	return data;
}

void DBExercisesModel::clearExercises()
{
	for (auto exercise_entry : std::as_const(m_exerciseData))
	{
		for (const auto sub_exercise : std::as_const(exercise_entry->m_exercises))
		{
			qDeleteAll(sub_exercise->sets);
			delete sub_exercise;
		}
		delete exercise_entry;
	}
	m_exerciseData.clear();
}

int DBExercisesModel::exportToFile(const QString &filename, QFile *out_file) const
{
	if (exerciseCount() == 0)
		return APPWINDOW_MSG_NOTHING_TO_EXPORT;

	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text);
		if (!out_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	const QList<QStringList> &data{std::move(QList<QStringList>{} << toDatabase(true))};
	const bool ret{appUtils()->writeDataToFile(out_file, identifierInFile(), data)};
	out_file->close();
	return ret ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_EXPORT_FAILED;
}

int DBExercisesModel::exportToFormattedFile(const QString &filename, QFile *out_file) const
{
	if (exerciseCount() == 0)
		return APPWINDOW_MSG_NOTHING_TO_EXPORT;

	if (!out_file)
	{
		out_file = {appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
		if (!out_file)
			return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;
	}

	const QString &strHeader{
		m_calendarDay >= 0 ?
			"####"_L1 + identifierInFile() + tr("Workout") + "\n\n"_L1 :
			"####"_L1 + identifierInFile() + tr("Exercises Program") + "\n\n"_L1
	};

	out_file->write(strHeader.toUtf8().constData());
	out_file->write(exportExtraInfo().toUtf8().constData());

	for (const auto exercise_entry : std::as_const(m_exerciseData))
	{
		out_file->write("\n\n\n", 3);
		out_file->write(QString{exerciseNameLabel() + QString::number(exercise_entry->exercise_number + 1)}.toUtf8().constData());
		const uint giant_set{static_cast<uint>(exercise_entry->m_exercises.count())};
		if (giant_set > 1)
			out_file->write(QString{tr("Giant set with ") + QString::number(giant_set) + tr(" exercises")}.toUtf8().constData());

		out_file->write(trackRestTimeLabel().toUtf8().constData());
		out_file->write(QString{exercise_entry->track_rest_time ? tr("Yes") : tr("No")}.toUtf8().constData());
		out_file->write("\n", 1);
		out_file->write(autoRestTimeLabel().toUtf8().constData());
		out_file->write(QString{exercise_entry->auto_rest_time ? tr("Yes") : tr("No")}.toUtf8().constData());

		for (const auto sub_exercise : std::as_const(exercise_entry->m_exercises))
		{
			out_file->write("\n\n** ", 2);
			out_file->write(sub_exercise->name.toUtf8().constData());
			out_file->write("\n", 1);
			out_file->write(totalSetsLabel().toUtf8().constData());
			out_file->write(QString::number(sub_exercise->sets.count()).toUtf8().constData());

			for (const auto set : std::as_const(sub_exercise->sets))
			{
				out_file->write("\n\n", 2);
				out_file->write(QString{setNumberLabel() + QString::number(set->set_number) + '\n'}.toUtf8().constData());
				out_file->write(setTypeLabel().toUtf8().constData());
				out_file->write(formatSetTypeToExport(set->type).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setRestTimeLabel().toUtf8().constData());
				if (!exercise_entry->track_rest_time)
					out_file->write(restTimeUntrackedLabel().toUtf8().constData());
				else
					out_file->write(appUtils()->formatTime(set->restTime).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setRepsLabel().toUtf8().constData());
				out_file->write(QString{set->reps}.replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setWeightLabel().toUtf8().constData());
				out_file->write(QString{set->weight}.replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setNotesLabel().toUtf8().constData());
			}
		}
	}
	out_file->write("\n", 1);
	out_file->write(appUtils()->STR_END_FORMATTED_EXPORT.toUtf8().constData());
	out_file->close();
	return APPWINDOW_MSG_EXPORT_OK;
}

int DBExercisesModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList data{WORKOUT_TOTALCOLS};
	QList<QStringList> exercise_data{1};
	exercise_data[0] = std::move(data);
	int ret{appUtils()->readDataFromFile(in_file, exercise_data, WORKOUT_TOTALCOLS, identifierInFile())};
	if (ret != APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE)
	{
		if (fromDataBase(exercise_data.at(0)))
			ret = APPWINDOW_MSG_IMPORT_OK;
		else
			ret = APPWINDOW_MSG_IMPORT_FAILED;
	}
	in_file->close();
	return ret;
}

int DBExercisesModel::importFromFormattedFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	QString value;
	uint exercise_number(0);
	const char *identifier_in_file{QString{"####"_L1 + identifierInFile()}.toLatin1().constData()};
	bool found_table_id{false}, found_extra_info{false};
	char buf[128];
	qint64 lineLength(0);

	const QString &exercise_delimiter{exerciseNameLabel()};
	const char *exercise_delim{exercise_delimiter.toUtf8().constData()};
	const uint exercise_delim_len{static_cast<uint>(exercise_delimiter.length())};
	const char *sub_exercise_delim{"** "};
	const uint sub_exercise_delim_len{3};
	const QString &set_delimiter(tr("Set #: "));
	const char *set_delim{set_delimiter.toUtf8().constData()};
	const uint set_delim_len{static_cast<uint>(set_delimiter.length())};

	while ((lineLength = in_file->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, appUtils()->STR_END_FORMATTED_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (!found_table_id)
					found_table_id = strstr(buf, identifier_in_file) != NULL;
				else
				{
					if (!found_extra_info)
						found_extra_info = importExtraInfo(QString{buf}.simplified());
					else
					{
						if(strncmp(buf, exercise_delim, exercise_delim_len) == 0)
						{
							const uint exercise_number{addExercise(false)};
							uint exercise_idx{0};
							int set_number{-1};
							short next_field{EXERCISES_COL_TRACKRESTTIMES};
							constexpr short SETS_FIELDS{50};

							while ((lineLength = in_file->readLine(buf, sizeof(buf))) != -1)
							{
								if (lineLength > 5)
								{
									value = buf;
									switch (next_field) {
										case EXERCISES_COL_EXERCISES:
											if(strncmp(buf, sub_exercise_delim, sub_exercise_delim_len) == 0)
											{
												addSubExercise(exercise_number, false);
												value.remove(0, sub_exercise_delim_len);
												setExerciseName(exercise_number, exercise_idx, std::move(value.trimmed()));
												next_field = SETS_FIELDS;
											}
										break;
										case EXERCISES_COL_TRACKRESTTIMES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setTrackRestTime(exercise_number, value == tr("Yes"));
											next_field = EXERCISES_COL_AUTORESTTIMES;
										break;
										case EXERCISES_COL_AUTORESTTIMES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setAutoRestTime(exercise_number, value == tr("Yes"));
											next_field = EXERCISES_COL_EXERCISES;
										break;
										case SETS_FIELDS:
											if(strncmp(buf, set_delim, set_delim_len) == 0)
											{
												set_number = addSet(exercise_number, exercise_idx, false);
												next_field = EXERCISES_COL_SETTYPES;
											}
										break;
										case EXERCISES_COL_SETTYPES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setSetType(exercise_number, exercise_idx, set_number, formatSetTypeToImport(value), false);
											next_field = EXERCISES_COL_RESTTIMES;
										break;
										case EXERCISES_COL_RESTTIMES:
											if (!m_exerciseData.at(exercise_number)->track_rest_time)
												setSetRestTime(exercise_number, exercise_idx, set_number, "00:00"_L1);
											else
											{
												value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
												setSetRestTime(exercise_number, exercise_idx, set_number, value);
											}
											next_field = EXERCISES_COL_REPS;
										break;
										case EXERCISES_COL_REPS:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setSetReps(exercise_number, exercise_idx, set_number,
															std::move(value.replace(comp_exercise_fancy_separator, QString{comp_exercise_separator})));
											next_field = EXERCISES_COL_WEIGHTS;
										break;
										case EXERCISES_COL_WEIGHTS:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setSetWeight(exercise_number, exercise_idx, set_number,
															std::move(value.replace(comp_exercise_fancy_separator, QString{comp_exercise_separator})));
											next_field = EXERCISES_COL_NOTES;
										break;
										case EXERCISES_COL_NOTES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											setSetNotes(exercise_number, exercise_idx, set_number, std::move(value));
											next_field = SETS_FIELDS;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		else
			break;
	}
	in_file->close();
	return exerciseCount() > 0 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

int DBExercisesModel::newExercisesFromFile(const QString &filename, const std::optional<bool> &file_formatted)
{
	int import_result{APPWINDOW_MSG_IMPORT_FAILED};
	if (file_formatted.has_value())
	{
		if (file_formatted.value())
			import_result = importFromFormattedFile(filename);
		else
			import_result = importFromFile(filename);
	}
	else
	{
		import_result = importFromFile(filename);
		if (import_result == APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(filename);
	}
	return import_result;
}

const QString DBExercisesModel::formatSetTypeToExport(const uint type) const
{
	switch (type)
	{
		case Regular: return tr("Regular");
		case Pyramid: return tr("Pyramid");
		case ReversePyramid: return tr("Reverse Pyramid");
		case Drop: return tr("Drop Set");
		case Cluster: return tr("Cluster Set");
		case MyoReps: return tr("Myo Reps");
	}
	return QString{};
}

bool DBExercisesModel::importExtraInfo(const QString &maybe_extra_info, int &calendar_day, QChar &split_letter)
{
	if (maybe_extra_info.contains(splitLabel()))
	{
		split_letter = maybe_extra_info.sliced(maybe_extra_info.indexOf(':') + 2, 1).at(0);
		if (split_letter.cell() >= 'A' && split_letter.cell() <= 'F')
		{
			const int cal_day_idx{static_cast<int>(maybe_extra_info.indexOf(calendarDayExtraInfo) + calendarDayExtraInfo.length())};
			if (cal_day_idx > calendarDayExtraInfo.length()) //workoutModel
			{
				bool ok;
				calendar_day = maybe_extra_info.sliced(cal_day_idx, maybe_extra_info.indexOf(' ', cal_day_idx + 1)).toUInt(&ok);
				return ok;
			}
			else //splitModel
				return true;
		}
	}
	return false;
}

const uint DBExercisesModel::subExercisesCount(const uint exercise_number) const
{
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->m_exercises.count() : 0;
}

const uint DBExercisesModel::setsNumber(const uint exercise_number, const uint exercise_idx) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
		return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count();
	else
		return 0;
}

QString DBExercisesModel::muscularGroup() const
{
	return appMesoModel()->muscularGroup(m_mesoIdx, m_splitLetter);
}

uint DBExercisesModel::addExercise(const bool emit_signal)
{
	if (emit_signal)
		beginInsertRows(QModelIndex{}, exerciseCount(), exerciseCount());
	exerciseEntry *new_exercise{new exerciseEntry};
	const uint exercise_number{static_cast<uint>(m_exerciseData.count())};
	new_exercise->exercise_number = exercise_number;
	m_exerciseData.append(new_exercise);
	if (emit_signal)
	{
		setWorkingExercise(exercise_number);
		emit exerciseCountChanged();
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << exerciseNumberRole);
		endInsertRows();
	}
	return exercise_number;
}

void DBExercisesModel::delExercise(const uint exercise_number, const bool emit_signal)
{
	if (emit_signal)
		beginRemoveRows(QModelIndex{}, exercise_number, exercise_number);

	for (const auto exercise : m_exerciseData | std::views::drop(exercise_number + 1))
		exercise->exercise_number--;
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : std::as_const(exercise->m_exercises))
		qDeleteAll(sub_exercise->sets);
	qDeleteAll(exercise->m_exercises);
	delete exercise;
	m_exerciseData.remove(exercise_number);
	if (exercise_number >= workingExercise())
	{
		if (workingExercise() > 0)
			setWorkingExercise(exercise_number-1);
	}
	if (emit_signal)
	{
		emit dataChanged(index(exercise_number, 0), index(m_exerciseData.count() - 1, 0), QList<int>{} << exerciseNumberRole);
		emit exerciseCountChanged();
		emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_DEL_NOTIFY_IDX);
		endRemoveRows();
	}
}

void DBExercisesModel::moveExercise(const uint from, const uint to)
{
	if (from < m_exerciseData.count() && to < m_exerciseData.count())
	{
		beginMoveRows(QModelIndex{}, from, from, QModelIndex{}, to);
		exerciseEntry *tempExerciseData(std::move(m_exerciseData[from]));

		if (to > from)
		{
			for(uint i{from}; i < to; ++i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i + 1]);
				m_exerciseData.at(i)->exercise_number--;
			}
		}
		else
		{
			for(uint i{from}; i > to; --i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i - 1]);
				m_exerciseData.at(i)->exercise_number++;
			}
		}
		m_exerciseData[to] = std::move(tempExerciseData);
		m_exerciseData.at(to)->exercise_number = to;
		emit dataChanged(index(to > from ? from : to, 0), index(to > from ? to : from, 0), QList<int>{} << exerciseNumberRole);
		emit exerciseModified(from, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_MOVE_NOTIFY_IDX);
		endMoveRows();
	}
}

void DBExercisesModel::newExerciseFromExercisesList()
{
	setExerciseName(m_workingExercise, workingSubExercise(m_workingExercise), std::move(
		appExercisesList()->selectedEntriesValue(0, EXERCISES_LIST_COL_MAINNAME) + " - "_L1 +
		appExercisesList()->selectedEntriesValue(0, EXERCISES_LIST_COL_SUBNAME)));
	emit exerciseNameChanged(m_workingExercise, workingSubExercise(m_workingExercise));
	emit exerciseModified(m_workingExercise, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_EXERCISES);
}

uint DBExercisesModel::addSubExercise(const uint exercise_number, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* new_sub_exercise{new stExercise};
	const uint exercise_idx{static_cast<uint>(exercise->m_exercises.count())};
	new_sub_exercise->parent = exercise;
	new_sub_exercise->exercise_idx = exercise_idx;
	exercise->m_exercises.append(new_sub_exercise);
	if (emit_signal)
	{
		setWorkingSubExercise(exercise_idx, exercise_number);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << giantSetExerciseRole);
		emit subExerciseCountChanged(exercise_number);
	}
	return exercise_idx;
}

void DBExercisesModel::delSubExercise(const uint exercise_number, const uint exercise_idx, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : exercise->m_exercises | std::views::drop(exercise_idx + 1))
		sub_exercise->exercise_idx--;

	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	qDeleteAll(sub_exercise->sets);
	delete sub_exercise;
	exercise->m_exercises.remove(exercise_idx);
	if (workingSubExercise(exercise_number) >= exercise_idx)
	{
		if (workingSubExercise(exercise_number) > 0)
			setWorkingSubExercise(exercise_idx - 1, exercise_number);
	}
	if (emit_signal)
	{
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << giantSetExerciseRole);
		emit exerciseModified(exercise_number, exercise_idx, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_DEL_NOTIFY_IDX);
		emit subExerciseCountChanged(exercise_number);
	}
}

uint DBExercisesModel::addSet(const uint exercise_number, const uint exercise_idx, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	stSet* new_set{new stSet};
	const uint set_number{static_cast<uint>(sub_exercise->sets.count())};
	new_set->set_number = set_number;
	new_set->parent = sub_exercise;
	sub_exercise->sets.append(new_set);
	if (emit_signal)
	{
		if (isWorkout())
			setModeForSet(new_set);
		setWorkingSet(set_number, exercise_number, exercise_idx);
		emit setsNumberChanged(exercise_number, exercise_idx);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << setsNumberRole);
	}
	return set_number;
}

void DBExercisesModel::delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	for (const auto set : sub_exercise->sets | std::views::drop(set_number + 1))
		set->set_number--;
	delete sub_exercise->sets.at(set_number);
	sub_exercise->sets.remove(set_number);
	if (set_number >= workingSet(exercise_number, exercise_idx))
	{
		if (workingSet(exercise_number, exercise_idx) > 0)
			setWorkingSet(set_number - 1, exercise_number, exercise_idx);
	}
	if (emit_signal)
	{
		emit setsNumberChanged(exercise_number, exercise_idx);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << setsNumberRole);
		emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISE_DEL_NOTIFY_IDX);
	}
}

void DBExercisesModel::moveSet(const uint exercise_number, const uint exercise_idx, const uint from_set, const uint to_set)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	if (from_set < sub_exercise->sets.count() && to_set < sub_exercise->sets.count())
	{
		stSet *tempSet{std::move(sub_exercise->sets[from_set])};

		if (to_set > from_set)
		{
			for(uint i{from_set}; i < to_set; ++i)
			{
				sub_exercise->sets[i] = std::move(sub_exercise->sets[i + 1]);
				sub_exercise->sets.at(i)->set_number--;
			}
		}
		else
		{
			for(uint i{from_set}; i > to_set; --i)
			{
				sub_exercise->sets[i] = std::move(sub_exercise->sets[i - 1]);
				sub_exercise->sets.at(i)->set_number++;
			}
		}
		sub_exercise->sets[to_set] = std::move(tempSet);
		sub_exercise->sets.at(to_set)->set_number = to_set;
		emit setsNumberChanged(exercise_number, exercise_idx);
		emit exerciseModified(exercise_number, exercise_idx, from_set, EXERCISE_DEL_NOTIFY_IDX);
		emit exerciseModified(exercise_number, exercise_idx, to_set, EXERCISE_IGNORE_NOTIFY_IDX);
	}
}

bool DBExercisesModel::exerciseIsComposite(const uint exercise_number) const
{
	if (exercise_number < m_exerciseData.count())
		return m_exerciseData.at(exercise_number)->m_exercises.count() >= 2;
	else
		return false;
}

void DBExercisesModel::setWorkingExercise(const uint new_workingexercise)
{
	if (new_workingexercise < exerciseCount() && new_workingexercise != m_workingExercise)
	{
		m_workingExercise = new_workingexercise;
		if (workingSubExercise(m_workingExercise) == UNSET_VALUE)
			setWorkingSubExercise(0, m_workingExercise);
		emit workingExerciseChanged(m_workingExercise);
		emit dataChanged(index(new_workingexercise, 0), index(new_workingexercise, 0), QList<int>{} << workingExerciseRole);
	}
}

uint DBExercisesModel::workingSubExercise(int exercise_number) const
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->working_subexercise : 0;
}

void DBExercisesModel::setWorkingSubExercise(const uint new_workingsubexercise, int exercise_number)
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (new_workingsubexercise < subExercisesCount(exercise_number) && new_workingsubexercise != workingSubExercise(exercise_number))
	{
		m_exerciseData.at(exercise_number)->working_subexercise = new_workingsubexercise;
		if (workingSet(exercise_number, new_workingsubexercise) == UNSET_VALUE)
			setWorkingSet(0, exercise_number, new_workingsubexercise);
		emit workingSubExerciseChanged(exercise_number, new_workingsubexercise);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << workingSubExerciseRole);
	}
}

uint DBExercisesModel::workingSet(int exercise_number, int exercise_idx) const
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (exercise_idx < 0)
		exercise_idx = workingSubExercise(exercise_number);

	return exercise_number < m_exerciseData.count() ? (exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count() ?
			 m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->working_set : 0) : 0;
}

void DBExercisesModel::setWorkingSet(const uint new_workingset, int exercise_number, int exercise_idx)
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (exercise_idx < 0)
		exercise_idx = workingSubExercise(exercise_number);

	if (new_workingset < setsNumber(exercise_number, exercise_idx) && new_workingset != m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->working_set)
	{
		m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->working_set = new_workingset;
		emit workingSetChanged(exercise_number, exercise_idx, new_workingset);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << workingSetRole);
	}
}

QString DBExercisesModel::exerciseName(const uint exercise_number, const uint exercise_idx) const
{
	if (exercise_number < m_exerciseData.count())
	{
		if (exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
		{
			const QString &name{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name};
			return !name.isEmpty() ? name : tr("Choose exercise...");
		}
	}
	return QString{};
}

void DBExercisesModel::setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = new_name;
	emit exerciseNameChanged(exercise_number, exercise_idx);
	emit exerciseModified(exercise_number, exercise_idx, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_EXERCISES);
}

void DBExercisesModel::setExerciseName(const uint exercise_number, const uint exercise_idx, QString &&new_name)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = std::forward<QString>(new_name);
}

bool DBExercisesModel::trackRestTime(const uint exercise_number) const
{
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->track_rest_time : false;
}

void DBExercisesModel::setTrackRestTime(const uint exercise_number, const bool track_resttime)
{
	m_exerciseData.at(exercise_number)->track_rest_time = track_resttime;
	changeAllSetsMode(exercise_number);
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_TRACKRESTTIMES);
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << trackRestTimeRole);
}

bool DBExercisesModel::autoRestTime(const uint exercise_number) const
{
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->auto_rest_time : false;
}

void DBExercisesModel::setAutoRestTime(const uint exercise_number, const bool auto_resttime)
{
	m_exerciseData.at(exercise_number)->auto_rest_time = auto_resttime;
	changeAllSetsMode(exercise_number);
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_AUTORESTTIMES);
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << autoRestTimeRole);
}

int DBExercisesModel::setType(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type;
	}
	return -1;
}

void DBExercisesModel::setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type, const bool emit_signal)
{
	const QList<stSet*> &sets{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets};
	sets.at(set_number)->type = static_cast<TPSetTypes>(new_type);
	if (emit_signal)
	{
		setSuggestedTime(set_number, sets);
		setSuggestedSubSets(set_number, sets);
		setSuggestedReps(set_number, sets);
		setSuggestedWeight(set_number, sets);
		emit setTypeChanged(exercise_number, exercise_idx, set_number);
		emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_SETTYPES);
	}
}

void DBExercisesModel::changeSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type)
{
	const QList<stSet*> &sets{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets};
	sets.at(set_number)->type = static_cast<TPSetTypes>(new_type);
	setSuggestedTime(set_number, sets);
	setSuggestedSubSets(set_number, sets);
	setSuggestedReps(set_number, sets);
	setSuggestedWeight(set_number, sets);
	emit setTypeChanged(exercise_number, exercise_idx, set_number);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_SETTYPES);
}

QTime DBExercisesModel::suggestedRestTime(const QTime &prev_resttime, const uint set_type) const
{
	switch (set_type)
	{
		case Regular:
			return prev_resttime;
		break;
		case Pyramid:
		case ReversePyramid:
		case Drop:
			return prev_resttime.addSecs(30);
		break;
		case Cluster:
			return prev_resttime.addSecs(60);
		break;
		case MyoReps:
			return prev_resttime.addSecs(90);
		break;
	}
	return QTime{0, 0, 0};
}

QTime DBExercisesModel::restTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime;
	}
	return QTime{0, 0, 0};
}

QString DBExercisesModel::setRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	return appUtils()->formatTime(restTime(exercise_number, exercise_idx, set_number), TPUtils::TF_QML_DISPLAY_NO_HOUR);
}

void DBExercisesModel::setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_time)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime =
						std::move(appUtils()->getTimeFromTimeString(new_time, TPUtils::TF_QML_DISPLAY_NO_HOUR));
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_RESTTIMES);
}

QString DBExercisesModel::suggestedSubSets(const uint set_type)
{
	switch (set_type)
	{
		default:
			return "0"_L1;
		case Drop:
		case MyoReps:
			return "3"_L1;
		break;
		case Cluster:
			return "4"_L1;
		break;
	}
}

QString DBExercisesModel::setSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->subsets;
	}
	return "0"_L1;
}

void DBExercisesModel::setSetSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_subsets)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->subsets = new_subsets;
}

void DBExercisesModel::addSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type >= Drop)
	{
		const uint old_subsets{setSubSets(exercise_number, exercise_idx, set_number).toUInt()};
		setSetSubSets(exercise_number, exercise_idx, set_number, QString::number(old_subsets + 1));
		setSuggestedReps(set_number, m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets, old_subsets);
	}
}

void DBExercisesModel::delSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type >= Drop)
	{
		const uint new_subsets{setSubSets(exercise_number, exercise_idx, set_number).toUInt() - 1};
		appUtils()->removeFieldFromCompositeValue(new_subsets,
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator);
		appUtils()->removeFieldFromCompositeValue(new_subsets,
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight, record_separator);
		setSetSubSets(exercise_number, exercise_idx, set_number, QString::number(new_subsets));
		emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_SUBSETS);
	}
}

QString DBExercisesModel::suggestedReps(const QString &prev_reps, const uint set_type, const uint subset) const
{
	switch (set_type)
	{
		case Regular:
			return prev_reps.isEmpty() ? std::move("12"_L1) : prev_reps;
		break;
		case Pyramid:
			return prev_reps.isEmpty() ? std::move("15"_L1) : std::move(appUtils()->appLocale()->toString(qCeil(prev_reps.toUInt() * 0.8)));
		break;
		case ReversePyramid:
			return prev_reps.isEmpty() ? std::move("5"_L1) : std::move(appUtils()->appLocale()->toString(qCeil(prev_reps.toUInt() * 1.25)));
		break;
		case Drop:
			return prev_reps.isEmpty() ? std::move(dropSetReps("15"_L1, 0)) : std::move(dropSetReps(prev_reps, subset));
		break;
		case Cluster:
			return prev_reps.isEmpty() ? std::move(clusterReps("24"_L1, 4)) : std::move(clusterReps(prev_reps, subset));
		case MyoReps:
			return prev_reps.isEmpty() ? std::move(myorepsReps("20"_L1, 4)) : std::move(myorepsReps(prev_reps, subset));
		break;
	}
	return QString{};
}

QString DBExercisesModel::setReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
		{
			if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
				return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps;
			else
				return appUtils()->getCompositeValue(subset,
					m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator);
		}
	}
	return "0"_L1;
}

void DBExercisesModel::setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_reps,
										const uint subset)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
		m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps = new_reps;
	else
		appUtils()->setCompositeValue(subset, new_reps,
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_REPS);
}

void DBExercisesModel::setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_reps)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps = std::forward<QString>(new_reps);
}

QString DBExercisesModel::suggestedWeight(const QString &prev_weight, const uint set_type, const uint subset) const
{
	switch (set_type)
	{
		case Regular:
			return prev_weight.isEmpty() ? std::move("20"_L1) : prev_weight;
		break;
		case Pyramid:
			return prev_weight.isEmpty() ? std::move("20"_L1) :
											std::move(appUtils()->appLocale()->toString(qCeil(prev_weight.toUInt() * 1.2)));
		break;
		case ReversePyramid:
			return prev_weight.isEmpty() ? std::move("80"_L1) :
											std::move(appUtils()->appLocale()->toString(qCeil(prev_weight.toUInt() * 0.6)));
		break;
		case Drop:
			return prev_weight.isEmpty() ? std::move(dropSetWeight("80"_L1, 0)) : std::move(dropSetReps(prev_weight, subset));
		break;
		case Cluster:
			return prev_weight.isEmpty() ? std::move(clusterWeight("100"_L1, 4)) : std::move(clusterReps(prev_weight, subset));
		case MyoReps:
			return prev_weight.isEmpty() ? std::move(myorepsWeight("100"_L1, 4)) : std::move(myorepsReps(prev_weight, subset));
		break;
	}
	return QString {};
}

QString DBExercisesModel::setWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
		{
			if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
				return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight;
			else
				return appUtils()->getCompositeValue(subset,
					m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight, record_separator);
		}
	}
	return "0"_L1;
}

void DBExercisesModel::setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_weight,
										const uint subset)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
		m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight = new_weight;
	else
		appUtils()->setCompositeValue(subset, new_weight,
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight, record_separator);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_WEIGHTS);
}

void DBExercisesModel::setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_weight)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight = std::forward<QString>(new_weight);
}

QString DBExercisesModel::setNotes(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes;
	}
	return QString{};
}

void DBExercisesModel::setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_notes)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes = new_notes;
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_NOTES);
}

void DBExercisesModel::setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_notes)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes = std::forward<QString>(new_notes);
}

bool DBExercisesModel::setCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (!isWorkout())
		return true;
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed;
	}
	return false;
}

void DBExercisesModel::setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed = completed;
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << exerciseCompletedRole);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_COMPLETED);
}

bool DBExercisesModel::allSetsCompleted(int exercise_number, int exercise_idx) const
{
	if (!isWorkout())
		return true;
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (exercise_idx < 0)
		exercise_idx = workingSubExercise(exercise_number);
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets))
		{
			if (!set->completed)
				return false;
		}
		return true;
	}
	return false;
}

bool DBExercisesModel::anySetCompleted(int exercise_number, int exercise_idx) const
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (exercise_idx < 0)
		exercise_idx = workingSubExercise(exercise_number);
	for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets))
	{
		if (set->completed)
			return true;
	}
	return false;
}

bool DBExercisesModel::noSetsCompleted(int exercise_number, int exercise_idx) const
{
	if (exercise_number < 0)
		exercise_number = m_workingExercise;
	if (exercise_idx < 0)
		exercise_idx = workingSubExercise(exercise_number);
	for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets))
		if (set->completed)
			return false;
	return true;
}

uint DBExercisesModel::setMode(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
	{
		if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->mode;
	}
	return 0;
}

void DBExercisesModel::setSetMode(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint mode)
{
	stSet *set{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)};
	setSetMode(set, mode);
}

uint DBExercisesModel::getSetNextMode(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	uint mode{setMode(exercise_number, exercise_idx, set_number)};
	if (trackRestTime(exercise_number))
	{
		const bool auto_time{autoRestTime(exercise_number)};
		switch (mode)
		{
			case SM_COMPLETED:
			case SM_NOT_COMPLETED:
				mode = auto_time ? SM_START_EXERCISE : SM_START_REST;
			break;
			case SM_START_REST:
				mode = SM_START_EXERCISE;
			break;
			case SM_START_EXERCISE:
				mode = SM_COMPLETED;
			break;
		}
	}
	else
	{
		switch (mode)
		{
			case SM_COMPLETED:
				mode = SM_NOT_COMPLETED;
			break;
			case SM_NOT_COMPLETED : mode = SM_COMPLETED; break;
			default:
				mode = SM_NOT_COMPLETED;
		}
	}
	return mode;
}

QString DBExercisesModel::setModeLabel(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	switch (setMode(exercise_number, exercise_idx, set_number))
	{
		case SM_NOT_COMPLETED: return tr("Mark as completed"); break;
		case SM_START_REST: return tr("Start rest time");
		case SM_START_EXERCISE: return tr("Start exercise");
		case SM_COMPLETED: return tr("Completed!");
	}
	return QString{};
}

QVariant DBExercisesModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_exerciseData.count())
	{
		switch (role)
		{
			case exerciseNumberRole: return QString::number(row+1);
			case giantSetExerciseRole: return exerciseIsComposite(row);
			case setsNumberRole: return setsNumber(row, workingSubExercise(row));
			case exerciseCompletedRole: return allSetsCompleted(row);
			case workingExerciseRole: return workingExercise();
			case workingSubExerciseRole: return workingSubExercise(row);
			case workingSetRole: return workingSet(row, workingSubExercise(row));
			case trackRestTimeRole: return trackRestTime(row);
			case autoRestTimeRole: return autoRestTime(row);
		}
	}
	return QVariant{};
}

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_exerciseData.count())
	{
		switch (role) { //dataChanged is emitted in the setters
			case workingExerciseRole: setWorkingExercise(value.toUInt());
			case workingSubExerciseRole: setWorkingSubExercise(row, value.toUInt());
			case workingSetRole: setWorkingSet(row, workingSubExercise(row), value.toUInt());
			case trackRestTimeRole: setTrackRestTime(row, value.toBool());
			case autoRestTimeRole: setAutoRestTime(row, value.toBool());
			break;
		}
		return true;
	}
	return false;
}

void DBExercisesModel::commonConstructor()
{
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesModel::labelChanged);
	m_roleNames[exerciseNumberRole] = std::move("exerciseNumber");
	m_roleNames[giantSetExerciseRole] = std::move("giantSetExercise");
	m_roleNames[setsNumberRole] = std::move("setsNumber");
	m_roleNames[exerciseCompletedRole] = std::move("exerciseCompleted");
	m_roleNames[workingExerciseRole] = std::move("workingExercise");
	m_roleNames[workingSubExerciseRole] = std::move("workingSubExercise");
	m_roleNames[workingSetRole] = std::move("workingSet");
	m_roleNames[trackRestTimeRole] = std::move("trackRestTime");
	m_roleNames[autoRestTimeRole] = std::move("autoRestTime");

	m_mesoId = appMesoModel()->id(m_mesoIdx);
	if (m_calendarDay >= 0)
	{
		m_splitLetter = std::move(m_calendarManager->splitLetter(m_mesoIdx, m_calendarDay));
		m_identifierInFile = &appUtils()->workoutFileIdentifier;
	}
	else
		m_identifierInFile = &appUtils()->splitFileIdentifier;

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx)
		{
			if (field >= MESOCYCLES_COL_SPLITA && field <= MESOCYCLES_COL_SPLITF)
			{
				if (static_cast<int>(m_splitLetter.cell()) - static_cast<int>('A') == field - MESOCYCLES_COL_SPLITA)
					emit muscularGroupChanged();
			}
		}
	});
}

void DBExercisesModel::changeCalendarDayId()
{
	if (m_calendarManager->workoutId(m_mesoIdx, m_calendarDay) == "-1"_L1)
		m_calendarManager->setWorkoutId(m_mesoIdx, m_calendarDay, id());
}

TPSetTypes DBExercisesModel::formatSetTypeToImport(const QString& fieldValue) const
{
	if (fieldValue == tr("Pyramid"))
		return Pyramid;
	else if (fieldValue == tr("Reverse Pyramid"))
		return ReversePyramid;
	else if (fieldValue == tr("Drop Set"))
		return Drop;
	else if (fieldValue == tr("Cluster Set"))
		return Cluster;
	else if (fieldValue == tr("Myo Reps"))
		return MyoReps;
	else
		return Regular;
}

//Don't put any colon ':' in here. Import will fail. All value()s returned from std::optional by calendarManager() are assumed to
//contain a valid value because export will only be an option if those values are valid. Those checks are made elsewhere in the code path.
const QString DBExercisesModel::exportExtraInfo() const
{
	QString extra_info{std::move(splitLabel() + splitLetter() + " ("_L1 + appMesoModel()->muscularGroup(mesoIdx(), splitLetter()) + ')')};
	if (m_calendarDay >= 0)
		extra_info += std::forward<QString>(calendarDayExtraInfo + std::move(QString::number(m_calendarDay)) +
				std::move(tr(" at ")) + std::move(appUtils()->formatDate(calendarManager()->date(mesoIdx(), m_calendarDay))));
	return extra_info;
}

inline bool DBExercisesModel::importExtraInfo(const QString &maybe_extra_info)
{
	return importExtraInfo(maybe_extra_info, m_calendarDay, m_splitLetter);
}

void DBExercisesModel::setSetMode(stSet *set, const uint mode)
{
	set->mode = mode;
	emit setModeChanged(set->parent->parent->exercise_number, set->parent->exercise_idx, set->set_number, mode);
	if (mode == SM_COMPLETED || mode == SM_NOT_COMPLETED)
		setSetCompleted(set->parent->parent->exercise_number, set->parent->exercise_idx, set->set_number, mode == SM_COMPLETED);
}

void DBExercisesModel::setModeForSet(stSet *set)
{
	uint mode{SM_COMPLETED};
	const uint exercise_number{set->parent->parent->exercise_number};
	if (!setCompleted(exercise_number, set->parent->exercise_idx, set->set_number))
	{
		if (trackRestTime(exercise_number))
			mode = autoRestTime(exercise_number) ? SM_START_EXERCISE : SM_START_REST;
		else
			mode = SM_NOT_COMPLETED;
	}
	setSetMode(set, mode);
}

void DBExercisesModel::changeAllSetsMode(const uint exercise_number)
{
	exerciseEntry *exercise{m_exerciseData.at(exercise_number)};
	for (const auto sub_exercise : std::as_const(exercise->m_exercises))
	{
		for (const auto set : std::as_const(sub_exercise->sets))
			setModeForSet(set);
	}
}

QString DBExercisesModel::increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs)
{
	uint secs{QStringView{strtime}.sliced(3, 2).toUInt()};
	uint mins{QStringView{strtime}.first(2).toUInt()};

	secs += add_secs;
	if (secs > 59)
	{
		secs -= 60;
		mins++;
	}
	mins += add_mins;
	const QString &ret{(mins <= 9 ? "0"_L1 + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? "0"_L1 + QString::number(secs) : QString::number(secs))};
	return ret;
}

void DBExercisesModel::setSuggestedTime(const uint set_number, const QList<stSet*> &sets)
{
	QTime prev_set_time;
	if (set_number == 0)
		prev_set_time.setHMS(0, 1, 0);
	else
		prev_set_time = sets.at(set_number-1)->restTime;

	stSet *set{sets.at(set_number)};
	set->restTime = std::move(suggestedRestTime(prev_set_time, set->type));
}

void DBExercisesModel::setSuggestedSubSets(const uint set_number, const QList<stSet*> &sets)
{
	stSet *set{sets.at(set_number)};
	set->subsets = std::move(suggestedSubSets(set->type));
}

void DBExercisesModel::setSuggestedReps(const uint set_number, const QList<stSet*> &sets, const uint from_subset)
{
	stSet *set{sets.at(set_number)};
	stSet *prev_set{nullptr};
	QString prev_set_reps;
	if (set_number != 0)
	{
		prev_set = sets.at(set_number-1);
		prev_set_reps = std::move(appUtils()->getCompositeValue(0, prev_set->reps, record_separator));
	}
	set->reps = std::move(suggestedReps(prev_set_reps, set->type, from_subset));
}

void DBExercisesModel::setSuggestedWeight(const uint set_number, const QList<stSet*> &sets, const uint from_subset)
{
	stSet *set{sets.at(set_number)};
	stSet *prev_set{nullptr};
	QString prev_set_weight;
	if (set_number != 0)
	{
		prev_set = sets.at(set_number-1);
		prev_set_weight = std::move(appUtils()->getCompositeValue(0, prev_set->weight, record_separator));
	}
	set->weight = std::move(suggestedWeight(prev_set_weight, set->type, from_subset));
}

QString DBExercisesModel::dropSetReps(const QString &reps, const uint from_subset) const
{
	QString new_drop_reps{reps};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, reps, record_separator))};
	uint n_subsets{appUtils()->nFieldsInCompositeString(reps, record_separator)};
	if (n_subsets == 0)
			n_subsets = 3;
	for (uint subset{from_subset}; subset < n_subsets; ++subset)
	{
		value *= 0.8;
		QString res{std::move(appUtils()->appLocale()->toString(qCeil(value)))};
		appUtils()->setCompositeValue(subset, res, new_drop_reps, record_separator);
	}

	return new_drop_reps;
}

QString DBExercisesModel::clusterReps(const QString &total_reps, const uint from_subset) const
{
	uint n_subsets{appUtils()->nFieldsInCompositeString(total_reps, record_separator)};
	if (n_subsets == 0)
		n_subsets = 4;
	const int value{qFloor(total_reps.toUInt()/n_subsets)};
	const QString &subSetValues{QString::number(value)};
	return appUtils()->makeCompositeValue(subSetValues, n_subsets - from_subset, record_separator);
}

QString DBExercisesModel::myorepsReps(const QString &first_set_reps, const uint n_sets, const uint from_set) const
{
	return appUtils()->makeCompositeValue(first_set_reps, n_sets - from_set, set_separator);
}

QString DBExercisesModel::dropSetWeight(const QString& weight, const uint from_subset) const
{
	QString new_drop_weight{weight};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, weight, record_separator))};
	uint n_subsets{appUtils()->nFieldsInCompositeString(weight, record_separator)};
	if (n_subsets == 0)
		n_subsets = 3;
	for (uint subset{from_subset}; subset < n_subsets; ++subset)
	{
		value *= 0.5;
		appUtils()->setCompositeValue(subset, QString::number(qCeil(value)), new_drop_weight, record_separator);
	}

	return new_drop_weight;
}

QString DBExercisesModel::clusterWeight(const QString &constant_weight, const uint from_subset) const
{
	uint n_subsets{appUtils()->nFieldsInCompositeString(constant_weight, record_separator)};
	if (n_subsets == 0)
		n_subsets = 4;
	return appUtils()->makeCompositeValue(constant_weight, n_subsets - from_subset, record_separator);
}

QString DBExercisesModel::myorepsWeight(const QString &first_set_weight, const uint n_sets, const uint from_set) const
{
	return appUtils()->makeCompositeValue(first_set_weight, n_sets - from_set, set_separator);
}
