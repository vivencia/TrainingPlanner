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

struct stSet {
	uint number;
	TPSetTypes type;
	QTime restTime;
	QString subsets, reps, weight, notes;
	TPBool completed;

	inline explicit stSet() : type{Regular}, subsets{'0'}, notes{' '} {}
};

struct stExercise {
	QString name;
	uint working_set;
	QList<stSet*> sets;

	inline explicit stExercise() : working_set{0} {}
};

struct exerciseEntry {
	QList<stExercise*> m_exercises;
	TPBool track_rest_time, auto_rest_time;
	uint number;
};

enum RoleNames {
	exerciseNumberRole = Qt::UserRole,
	giantSetExerciseRole = Qt::UserRole+1,
	setsNumberRole = Qt::UserRole+2,
	exerciseCompletedRole = Qt::UserRole+3,
	workingSetRole = Qt::UserRole+4
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
		m_exerciseData.at(exercise_number)->number = exercise_entry->number;
		m_exerciseData.at(exercise_number)->track_rest_time = exercise_entry->track_rest_time;
		m_exerciseData.at(exercise_number)->auto_rest_time = exercise_entry->auto_rest_time;
		for (const auto sub_exercise : std::as_const(exercise_entry->m_exercises))
		{
			const uint exercise_idx{addSubExercise(exercise_number, false)};
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = sub_exercise->name;
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->working_set = sub_exercise->working_set;
			for (const auto set : std::as_const(sub_exercise->sets))
			{
				const uint set_number{addSet(exercise_number, exercise_idx, false)};
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->number = set->number;
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
}

bool DBExercisesModel::fromDataBase(const QStringList &data, const bool bClearSomeFieldsForReUse)
{
	m_id = std::move(data.at(EXERCISES_COL_ID));
	m_mesoId = std::move(data.at(EXERCISES_COL_MESOID));
	m_calendarDay = data.at(EXERCISES_COL_CALENDARDAY).toUInt();
	m_splitLetter = data.at(EXERCISES_COL_SPLITLETTER).at(0);

	QStringList exercises(std::move(data.at(EXERCISES_COL_EXERCISES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList track_resttimes(std::move(data.at(EXERCISES_COL_TRACKRESTTIMES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList auto_resttimes(std::move(data.at(EXERCISES_COL_AUTORESTTIMES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList settypes(std::move(data.at(EXERCISES_COL_SETTYPES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList resttimes(std::move(data.at(EXERCISES_COL_RESTTIMES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList subsets(std::move(data.at(EXERCISES_COL_SUBSETS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList reps(std::move(data.at(EXERCISES_COL_REPS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList weights(std::move(data.at(EXERCISES_COL_WEIGHTS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList notes(std::move(data.at(EXERCISES_COL_NOTES).split(exercises_separator, Qt::SkipEmptyParts)));

	QStringList completed;
	if (!bClearSomeFieldsForReUse)
		completed = std::move(data.at(EXERCISES_COL_COMPLETED).split(exercises_separator, Qt::SkipEmptyParts));
	else //reset for new use
	{
		QString temp{data.at(EXERCISES_COL_COMPLETED)};
		completed = std::move(temp.replace(STR_ONE, STR_ZERO).split(exercises_separator, Qt::SkipEmptyParts));
	}

	for(uint exercise_idx{0}; exercise_idx < exercises.count(); ++exercise_idx)
	{
		const uint exercise_number{addExercise(false)};
		exerciseEntry *exercise_entry{m_exerciseData.at(exercise_idx)};
		exercise_entry->track_rest_time = appUtils()->getCompositeValue(exercise_idx, track_resttimes.at(exercise_idx), exercises_separator) == "1"_L1;
		exercise_entry->auto_rest_time = appUtils()->getCompositeValue(exercise_idx, auto_resttimes.at(exercise_idx), exercises_separator) == "1"_L1;
		do {
			addSubExercise(exercise_number, false);
			QString exercise_name{std::move(appUtils()->getCompositeValue(exercise_idx, exercises.at(exercise_idx), comp_exercise_separator))};
			if (exercise_name.isEmpty())
				break;
			stExercise *sub_exercise{exercise_entry->m_exercises.at(exercise_idx)};
			sub_exercise->name = std::move(exercise_name);
			const QString &set_types{appUtils()->getCompositeValue(exercise_idx, settypes.at(exercise_idx), comp_exercise_separator)};
			const QString &rest_times{appUtils()->getCompositeValue(exercise_idx, resttimes.at(exercise_idx), comp_exercise_separator)};
			const QString &sub_sets{appUtils()->getCompositeValue(exercise_idx, subsets.at(exercise_idx), comp_exercise_separator)};
			const QString &set_reps{appUtils()->getCompositeValue(exercise_idx, reps.at(exercise_idx), comp_exercise_separator)};
			const QString &set_weights{appUtils()->getCompositeValue(exercise_idx, weights.at(exercise_idx), comp_exercise_separator)};
			const QString &set_notes{appUtils()->getCompositeValue(exercise_idx, notes.at(exercise_idx), comp_exercise_separator)};
			const QString &set_completed{appUtils()->getCompositeValue(exercise_idx, completed.at(exercise_idx), comp_exercise_separator)};
			const uint n_sets{appUtils()->nFieldsInCompositeString(set_types, set_separator)};

			for (uint x{0}; x < n_sets; ++x)
			{
				const uint set_number{addSet(exercise_number, exercise_idx, false)};
				stSet *set{sub_exercise->sets.at(x)};
				set->type = 	static_cast<TPSetTypes>(appUtils()->getCompositeValue(set_number, set_types, set_separator).toUInt());
				set->restTime = std::move(appUtils()->getTimeFromTimeString(
						appUtils()->getCompositeValue(set_number, rest_times, set_separator), TPUtils::TF_QML_DISPLAY_NO_SEC));
				set->subsets = std::move(appUtils()->getCompositeValue(set_number, sub_sets, set_separator));
				set->reps = std::move(appUtils()->getCompositeValue(set_number, set_reps, set_separator));
				set->weight = std::move(appUtils()->getCompositeValue(set_number, set_weights, set_separator));
				set->notes = std::move(appUtils()->getCompositeValue(set_number, set_notes, set_separator));
				set->completed = appUtils()->getCompositeValue(set_number, set_completed, set_separator) == "1"_L1;
			}
		} while (true);
	}
	emit exerciseCountChanged();
	emit dataChanged(index(0, 0), index(m_exerciseData.count()-1, 0));
	return m_exerciseData.count() > 0;
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
		out_file->write(QString{exerciseNameLabel() + QString::number(exercise_entry->number+1)}.toUtf8().constData());
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
				out_file->write(QString{setNumberLabel() + QString::number(set->number) + '\n'}.toUtf8().constData());
				out_file->write(setTypeLabel().toUtf8().constData());
				out_file->write(formatSetTypeToExport(set).toUtf8().constData());
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
											setSetType(exercise_number, exercise_idx, set_number, formatSetTypeToImport(value));
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

const uint DBExercisesModel::setsNumber(const uint exercise_number) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(0)->sets.count();
}

uint DBExercisesModel::addExercise(const bool emit_signal)
{
	exerciseEntry *new_exercise{new exerciseEntry};
	new_exercise->number = m_exerciseData.count();
	stExercise* new_sub_exercise{new stExercise};
	new_exercise->m_exercises.append(new_sub_exercise);
	m_exerciseData.append(new_exercise);
	const uint exercise_number{static_cast<uint>(m_exerciseData.count())};
	if (emit_signal)
	{
		emit exerciseCountChanged();
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << exerciseNumberRole);
	}
	return exercise_number;
}

void DBExercisesModel::delExercise(const uint exercise_number, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (uint i{exercise_number}; i < m_exerciseData.count(); ++i)
		m_exerciseData.at(i)->number = i;
	for (const auto sub_exercise : std::as_const(exercise->m_exercises))
		qDeleteAll(sub_exercise->sets);
	qDeleteAll(exercise->m_exercises);
	delete exercise;
	m_exerciseData.remove(exercise_number);
	if (emit_signal)
	{
		emit dataChanged(index(exercise_number, 0), index(m_exerciseData.count() - 1, 0), QList<int>{} << exerciseNumberRole);
		emit exerciseCountChanged();
		emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_DEL_NOTIFY_IDX);
	}
}

void DBExercisesModel::moveExercise(const uint from, const uint to)
{
	if (from < m_exerciseData.count() && to < m_exerciseData.count())
	{
		exerciseEntry *tempExerciseData(std::move(m_exerciseData[from]));

		if (to > from)
		{
			for(uint i{from}; i < to; ++i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i+1]);
				m_exerciseData.at(i)->number--;
			}
		}
		else
		{
			for(uint i{from}; i > to; --i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i-1]);
				m_exerciseData.at(i)->number++;
			}
		}
		m_exerciseData[to] = std::move(tempExerciseData);
		m_exerciseData.at(to)->number = to;
		emit dataChanged(index(to > from ? from : to, 0), index(to > from ? to : from, 0), QList<int>{} << exerciseNumberRole);
		emit exerciseModified(from, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_MOVE_NOTIFY_IDX);
		emit exerciseModified(to, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX);
	}
}

void DBExercisesModel::newExerciseFromExercisesList()
{
	const uint n_subexercises{appExercisesList()->selectedEntriesCount()};
	if (n_subexercises == 0)
		return;

	const uint exercise_number{addExercise()};
	for (uint exercise_idx{0}; exercise_idx < n_subexercises; ++exercise_idx)
	{
		static_cast<void>(addSubExercise(exercise_number));
		setExerciseName(exercise_number, exercise_idx, std::move(
			appExercisesList()->selectedEntriesValue(exercise_idx, EXERCISES_LIST_COL_MAINNAME) + " - "_L1 +
			appExercisesList()->selectedEntriesValue(exercise_idx, EXERCISES_LIST_COL_SUBNAME)));
		emit exerciseNameChanged(exercise_number, exercise_idx);
	}
}

uint DBExercisesModel::addSubExercise(const uint exercise_number, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* new_sub_exercise{new stExercise};
	exercise->m_exercises.append(new_sub_exercise);
	if (emit_signal)
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << giantSetExerciseRole);
	return exercise->m_exercises.count() - 1;
}

void DBExercisesModel::delSubExercise(const uint exercise_number, const uint exercise_idx, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	qDeleteAll(sub_exercise->sets);
	delete sub_exercise;
	exercise->m_exercises.remove(exercise_idx);
	if (emit_signal)
	{
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << giantSetExerciseRole);
		emit exerciseModified(exercise_number, exercise_idx, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_DEL_NOTIFY_IDX);
	}
}

uint DBExercisesModel::addSet(const uint exercise_number, const uint exercise_idx, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	stSet* new_set{new stSet};
	new_set->number = sub_exercise->sets.count();
	sub_exercise->sets.append(new_set);
	if (emit_signal)
	{
		emit setsNumberChanged(exercise_number, exercise_idx);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << setsNumberRole);
	}
	return sub_exercise->sets.count() - 1;
}

void DBExercisesModel::delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool emit_signal)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	for (uint i{set_number}; i < sub_exercise->sets.count(); ++i)
		sub_exercise->sets.at(i)->number = i;
	delete sub_exercise->sets.at(set_number);
	sub_exercise->sets.remove(set_number);
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
				sub_exercise->sets[i] = std::move(sub_exercise->sets[i+1]);
				sub_exercise->sets.at(i)->number--;
			}
		}
		else
		{
			for(uint i{from_set}; i > to_set; --i)
			{
				sub_exercise->sets[i] = std::move(sub_exercise->sets[i-1]);
				sub_exercise->sets.at(i)->number++;
			}
		}
		sub_exercise->sets[to_set] = std::move(tempSet);
		sub_exercise->sets.at(to_set)->number = to_set;
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

uint DBExercisesModel::workingSet(const uint exercise_number) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(0)->working_set;
}

void DBExercisesModel::setWorkingSet(const uint exercise_number, const uint new_workingset)
{
	if (new_workingset != m_exerciseData.at(exercise_number)->m_exercises.at(0)->working_set)
	{
		m_exerciseData.at(exercise_number)->m_exercises.at(0)->working_set = new_workingset;
		emit workingSetChanged(exercise_number);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << workingSetRole);
	}
}

QString DBExercisesModel::exerciseName(const uint exercise_number, const uint exercise_idx) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name;
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
	return m_exerciseData.at(exercise_number)->track_rest_time;
}

void DBExercisesModel::setTrackRestTime(const uint exercise_number, const bool track_resttime)
{
	m_exerciseData.at(exercise_number)->track_rest_time = track_resttime;
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_TRACKRESTTIMES);
}

bool DBExercisesModel::autoRestTime(const uint exercise_number) const
{
	return m_exerciseData.at(exercise_number)->auto_rest_time;
}

void DBExercisesModel::setAutoRestTime(const uint exercise_number, const bool auto_resttime)
{
	m_exerciseData.at(exercise_number)->auto_rest_time = auto_resttime;
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_COL_AUTORESTTIMES);
}

uint DBExercisesModel::setType(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type;
}

void DBExercisesModel::setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type = static_cast<TPSetTypes>(new_type);
	emit setTypeChanged(exercise_number, exercise_idx, set_number);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_SETTYPES);
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

QString DBExercisesModel::setRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	return appUtils()->formatTime(m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime);
}

void DBExercisesModel::setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_time)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime =
						std::move(appUtils()->getTimeFromTimeString(new_time));
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_RESTTIMES);
}

void DBExercisesModel::setSetSuggestedRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	setSuggestedTime(set_number, m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_RESTTIMES);
}

QString DBExercisesModel::setSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->subsets;
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

QString DBExercisesModel::setReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset) const
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
		return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps;
	else
		return appUtils()->getCompositeValue(subset,
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator);
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

QString DBExercisesModel::setWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset) const
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
		return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight;
	else
		return appUtils()->getCompositeValue(subset,
				m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight, record_separator);
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
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes;
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
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed;
}

void DBExercisesModel::setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed = completed;
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{} << exerciseCompletedRole);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_COL_COMPLETED);
}

bool DBExercisesModel::allSetsCompleted(const uint exercise_number) const
{
	for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(0)->sets))
	{
		if (!set->completed)
			return false;
	}
	return true;
}

bool DBExercisesModel::anySetCompleted(const uint exercise_number) const
{
	for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(0)->sets))
	{
		if (set->completed)
			return true;
	}
	return false;
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
			case setsNumberRole: return setsNumber(row);
			case exerciseCompletedRole: return allSetsCompleted(row);
			case workingSetRole: return workingSet(row);
		}
	}
	return QVariant{};
}

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_exerciseData.count())
	{
		switch (role) {
			case workingSetRole: setWorkingSet(row, value.toUInt()); //dataChanged is emitted in setWorkingSet()
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
	m_roleNames[workingSetRole] = std::move("workingSet");
	m_mesoId = appMesoModel()->id(m_mesoIdx);
	if (m_calendarDay >= 0)
	{
		m_splitLetter = m_calendarManager->splitLetter(m_mesoIdx, m_calendarDay).value().at(0);
		m_identifierInFile = &appUtils()->workoutFileIdentifier;
	}
	else
		m_identifierInFile = &appUtils()->splitFileIdentifier;
}

const QString DBExercisesModel::formatSetTypeToExport(stSet* set) const
{
	switch (set->type)
	{
		case Regular: return tr("Regular");
		case Pyramid: return tr("Pyramid");
		case ReversePyramid: return tr("Reverse Pyramid");
		case Drop: return tr("Drop Set");
		case Cluster: return tr("Cluster Set");
		case MyoReps: return tr("Myo Reps");
	}
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
						std::move(tr(" at ")) + std::move(appUtils()->formatDate(calendarManager()->date(mesoIdx(), m_calendarDay).value())));
	return extra_info;
}

inline bool DBExercisesModel::importExtraInfo(const QString &maybe_extra_info)
{
	return importExtraInfo(maybe_extra_info, m_calendarDay, m_splitLetter);
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
	switch (set->type)
	{
		case Regular:
			set->restTime = std::move(prev_set_time);
		break;
		case Pyramid:
		case ReversePyramid:
		case Drop:
			set->restTime = std::move(prev_set_time.addSecs(30));
		break;
		case Cluster:
			set->restTime = std::move(prev_set_time.addSecs(60));
		break;
		case MyoReps:
			set->restTime = std::move(prev_set_time.addSecs(90));
		break;
	}
}

void DBExercisesModel::setSuggestedSubSets(const uint set_number, const QList<stSet*> &sets)
{
	stSet *set{sets.at(set_number)};
	switch (set->type)
	{
		case Regular:
		case Pyramid:
		case ReversePyramid:
			set->subsets = "0"_L1;
		case Drop:
		case MyoReps:
			set->subsets = "3"_L1;
		break;
		case Cluster:
			set->subsets = "4"_L1;
		break;
	}
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

	switch (set->type)
	{
		case Regular:
			set->reps = set_number == 0 ? std::move("12"_L1) : prev_set_reps;
		break;
		case Pyramid:
			set->reps = set_number == 0 ? std::move("15"_L1) : std::move(appUtils()->appLocale()->toString(qCeil(prev_set_reps.toUInt() * 0.8)));
		break;
		case ReversePyramid:
			set->reps = set_number == 0 ? std::move("5"_L1) : std::move(appUtils()->appLocale()->toString(qCeil(prev_set_reps.toUInt() * 1.25)));
		break;
		case Drop:
			set->reps = set_number == 0 ? std::move(DBExercisesModel::dropSetReps("15"_L1, 3, 0)) :
										std::move(DBExercisesModel::dropSetReps(prev_set_reps, prev_set->subsets.toUInt(), from_subset));
		break;
		case Cluster:
			set->reps = set_number == 0 ? std::move(DBExercisesModel::clusterReps("24"_L1, 4)) :
										std::move(DBExercisesModel::clusterReps(prev_set_reps, prev_set->subsets.toUInt(), from_subset));
		case MyoReps:
			set->reps = set_number == 0 ? std::move(DBExercisesModel::myorepsReps("20"_L1, 4)) :
										std::move(DBExercisesModel::myorepsReps(prev_set_reps, prev_set->subsets.toUInt(), from_subset));
		break;
	}
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

	switch (set->type)
	{
		case Regular:
			set->weight = set_number == 0 ? std::move("20"_L1) : prev_set_weight;
		break;
		case Pyramid:
			set->weight = set_number == 0 ? std::move("20"_L1) :
											std::move(appUtils()->appLocale()->toString(qCeil(prev_set_weight.toUInt() * 1.2)));
		break;
		case ReversePyramid:
			set->weight = set_number == 0 ? std::move("80"_L1) :
											std::move(appUtils()->appLocale()->toString(qCeil(prev_set_weight.toUInt() * 0.6)));
		break;
		case Drop:
			set->weight = set_number == 0 ? std::move(DBExercisesModel::dropSetWeight("80"_L1, 3, 0)) :
										std::move(DBExercisesModel::dropSetReps(prev_set_weight, prev_set->subsets.toUInt(), from_subset));
		break;
		case Cluster:
			set->weight = set_number == 0 ? std::move(DBExercisesModel::clusterWeight("100"_L1, 4)) :
										std::move(DBExercisesModel::clusterReps(prev_set_weight, prev_set->subsets.toUInt(), from_subset));
		case MyoReps:
			set->weight = set_number == 0 ? std::move(DBExercisesModel::myorepsWeight("100"_L1, 4)) :
									std::move(DBExercisesModel::myorepsReps(prev_set_weight, prev_set->subsets.toUInt(), from_subset));
		break;
	}
}

QString DBExercisesModel::dropSetReps(const QString &reps, const uint n_subsets, const uint from_subset)
{
	QString new_drop_reps{reps};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, reps, record_separator))};

	for (uint subset{from_subset}; subset < n_subsets; ++subset)
	{
		value *= 0.8;
		QString res{std::move(appUtils()->appLocale()->toString(qCeil(value)))};
		appUtils()->setCompositeValue(subset, res, new_drop_reps, record_separator);
	}

	return new_drop_reps;
}

QString DBExercisesModel::clusterReps(const QString &total_reps, const uint n_subsets, const uint from_subset)
{
	const int value{qFloor(total_reps.toUInt()/n_subsets)};
	const QString &subSetValues{QString::number(value)};
	return appUtils()->makeCompositeValue(subSetValues, n_subsets - from_subset, record_separator);
}

QString DBExercisesModel::myorepsReps(const QString &first_set_reps, const uint n_sets, const uint from_subset)
{
	return appUtils()->makeCompositeValue(first_set_reps, n_sets - from_subset, set_separator);
}

QString DBExercisesModel::dropSetWeight(const QString& weight, const uint n_subsets, const uint from_subset)
{
	QString new_drop_weight{weight};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, weight, record_separator))};

	for (uint subset{from_subset}; subset < n_subsets; ++subset)
	{
		value *= 0.5;
		appUtils()->setCompositeValue(subset, QString::number(qCeil(value)), new_drop_weight, record_separator);
	}

	return new_drop_weight;
}

QString DBExercisesModel::clusterWeight(const QString &constant_weight, const uint n_subsets, const uint from_subset)
{
	return appUtils()->makeCompositeValue(constant_weight, n_subsets - from_subset, record_separator);
}

QString DBExercisesModel::myorepsWeight(const QString &first_set_weight, const uint n_sets, const uint from_subset)
{
	return appUtils()->makeCompositeValue(first_set_weight, n_sets - from_subset, set_separator);
}
