#include "dbexercisesmodel.h"

#include "dbcalendarmodel.h"
#include "dbexerciseslistmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "pageslistmodel.h"
#include "return_codes.h"
#include "tpbool.h"
#include "tputils.h"
#include "translationclass.h"

#include <QApplication>
#include <QtMath>

#include <utility>
#include <ranges>

constexpr uint8_t UNSET_VALUE{255};

QT_FORWARD_DECLARE_STRUCT(stExercise)

inline DBModelInterfaceExercises::DBModelInterfaceExercises(DBExercisesModel *model) : DBModelInterface{model} {}

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
	TPBool sync_giant_sets;
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
	createRole(exerciseNumber, 1)
	createRole(setsNumber, 2)
	createRole(exerciseCompleted, 3)
	createRole(workingExercise, 4)
	createRole(workingSubExercise, 5)
	createRole(workingSet, 6)
	createRole(trackRestTime, 7)
	createRole(autoRestTime, 8)
};

DBWorkoutsOrSplitsTable *DBExercisesModel::database() const
{
	m_db->setDBModelInterface(m_dbModelInterface);
	return m_db;
}

void DBExercisesModel::plugDBModelInterfaceIntoDatabase()
{
	m_db->setDBModelInterface(m_dbModelInterface);
}

void DBExercisesModel::operator=(DBExercisesModel *other_model)
{
	if (m_calendarDay < 0) //only a split model might change its splitletter property. A workout model keeps it
		setSplitLetter(other_model->splitLetter());
	QString mesoid{std::move(m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_MESOID))};
	QString calendar_day{std::move(m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_CALENDARDAY))};
	QString split_letter{std::move(m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_SPLITLETTER))};
	clearExercises();
	m_dbModelInterface->clearData();
	m_dbModelInterface->modelData() = other_model->m_dbModelInterface->modelData();

	beginResetModel();
	for (const auto &exercise_entry : std::as_const(other_model->m_exerciseData))
	{
		const uint exercise_number{addExercise(-1, false)};
		m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_MESOID] = mesoid;
		m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_CALENDARDAY] = calendar_day;
		m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_SPLITLETTER] = split_letter;

		setTrackRestTime(exercise_number, exercise_entry->track_rest_time);
		setAutoRestTime(exercise_number, exercise_entry->auto_rest_time);
		uint exercise_idx{0};
		do {
			stExercise* sub_exercise{exercise_entry->m_exercises.at(exercise_idx)};
			setExerciseName(exercise_number, exercise_idx, sub_exercise->name);
			for (const auto set : std::as_const(sub_exercise->sets))
			{
				const uint set_number{addSet(exercise_number, exercise_idx)};
				_setSetType(exercise_number, exercise_idx, set_number, set->type);
				_setSetRestTime(exercise_number, exercise_idx, set_number, set->restTime);
				_setSetSubSets(exercise_number, exercise_idx, set_number, set->subsets);
				_setSetReps(exercise_number, exercise_idx, set_number, std::move(set->reps));
				_setSetWeight(exercise_number, exercise_idx, set_number, std::move(set->weight));
				_setSetNotes(exercise_number, exercise_idx, set_number, std::move(set->notes));
				_setSetCompleted(exercise_number, exercise_idx, set_number, set->completed);
			}
			if (++exercise_idx >= exercise_entry->m_exercises.count())
				break;
			addSubExercise(exercise_number, false);
		} while (true);
	}
	setWorkingExercise(0);
	setWorkingSubExercise(0, 0);
	setWorkingSet(0, 0, 0);
	endResetModel();
	m_dbModelInterface->setModifiedRows(0, exerciseCount());
	appThreadManager()->runAction(m_db, ThreadManager::InsertRecords);
}

bool DBExercisesModel::fromDatabase(const bool db_data_ok)
{
	auto end_func = [this] () -> bool {
		m_exercisesLoaded = true;
		setWorkingExercise(0);
		setWorkingSubExercise(0, 0);
		setWorkingSet(0, 0, 0);
		emit exerciseCountChanged();
		endResetModel();
		return m_exerciseData.count() > 0;
	};

	beginResetModel();
	if (!db_data_ok)
		return end_func();

	m_calendarDay = m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_CALENDARDAY).toInt();
	m_splitLetter = m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_SPLITLETTER).at(0);

	for (const auto &data : std::as_const(m_dbModelInterface->modelData()))
	{
		const auto exercise_number{addExercise(-1, false)};
		exerciseEntry *exercise_entry{m_exerciseData.at(exercise_number)};
		exercise_entry->track_rest_time = data.at(EXERCISES_FIELD_TRACKRESTTIMES) == "1"_L1;
		exercise_entry->auto_rest_time = data.at(EXERCISES_FIELD_AUTORESTTIMES) == "1"_L1;

		const QStringList &sub_exercises{data.at(EXERCISES_FIELD_EXERCISES).split(comp_exercises_separator, Qt::SkipEmptyParts)};
		const QStringList &settypes{data.at(EXERCISES_FIELD_SETTYPES).split(comp_exercises_separator)};
		const QStringList &resttimes{data.at(EXERCISES_FIELD_RESTTIMES).split(comp_exercises_separator)};
		const QStringList &subsets{data.at(EXERCISES_FIELD_SUBSETS).split(comp_exercises_separator)};
		const QStringList &reps{data.at(EXERCISES_FIELD_REPS).split(comp_exercises_separator)};
		const QStringList &weights{data.at(EXERCISES_FIELD_WEIGHTS).split(comp_exercises_separator)};
		const QStringList &notes{data.at(EXERCISES_FIELD_NOTES).split(comp_exercises_separator)};
		const QStringList &completed{data.at(EXERCISES_FIELD_COMPLETED).split(comp_exercises_separator)};

		const auto n_subexercises{sub_exercises.count()};
		for (uint exercise_idx{0}; exercise_idx < n_subexercises; ++exercise_idx)
		{
			addSubExercise(exercise_number, false);
			stExercise *sub_exercise{exercise_entry->m_exercises.at(exercise_idx)};
			sub_exercise->name = std::move(sub_exercises.at(exercise_idx));

			if (settypes.isEmpty())
				continue;
			const QString &set_types{settypes.at(exercise_idx)};
			const auto n_sets{appUtils()->nFieldsInCompositeString(set_types, set_separator)};
			if (n_sets == 0)
				continue;
			const QString &rest_times{resttimes.at(exercise_idx)};
			const QString &sub_sets{subsets.at(exercise_idx)};
			const QString &set_reps{reps.at(exercise_idx)};
			const QString &set_weights{weights.at(exercise_idx)};
			const QString &set_notes{notes.at(exercise_idx)};
			const QString &set_completed{completed.at(exercise_idx)};

			for (uint set_number{0}; set_number < n_sets; ++set_number)
			{
				static_cast<void>(addSet(exercise_number, exercise_idx, false));
				stSet *set{sub_exercise->sets.at(set_number)};
				set->type = static_cast<TPSetTypes>(appUtils()->getCompositeValue(set_number, set_types, set_separator).toUInt());
				if (!rest_times.isEmpty())
				{
					set->restTime = std::move(appUtils()->timeFromString(
						appUtils()->getCompositeValue(set_number, rest_times, set_separator), TPUtils::TF_QML_DISPLAY_NO_HOUR));
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
	}
	return end_func();
}

void DBExercisesModel::clearExercises(const bool from_qml)
{
	if (from_qml)
	{
		m_dbModelInterface->setRemovalRows(0, exerciseCount(), EXERCISES_FIELD_MESOID);
		m_db->setDBModelInterface(m_dbModelInterface);
		appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);
		beginResetModel();
	}
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
	if (from_qml)
	{
		endResetModel();
		emit exerciseCountChanged();
	}
}

const QString &DBExercisesModel::mesoId() const
{
	return m_mesoModel->id(m_mesoIdx);
}

void DBExercisesModel::setSplitLetter(const QChar &new_splitletter)
{
	if (m_splitLetter != new_splitletter)
	{
		m_splitLetter = new_splitletter;
		emit splitLetterChanged();
		uint modified_row{0};
		for (auto &data : m_dbModelInterface->modelData())
		{
			data[EXERCISES_FIELD_SPLITLETTER] = new_splitletter;
			m_dbModelInterface->setModified(modified_row++, EXERCISES_FIELD_SPLITLETTER);
		}
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateRecords);
	}
}

int DBExercisesModel::exportToFile(const QString &filename, QFile *out_file) const
{
	if (exerciseCount() == 0)
		return TP_RET_CODE_NOTHING_TO_EXPORT;

	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, false, true, false, true);
		if (!out_file)
			return TP_RET_CODE_OPEN_WRITE_FAILED;
	}

	const bool ret{appUtils()->writeDataToFile(out_file, identifierInFile(), m_dbModelInterface->modelData())};
	out_file->close();
	return ret ? TP_RET_CODE_EXPORT_OK : TP_RET_CODE_EXPORT_FAILED;
}

int DBExercisesModel::exportToFormattedFile(const QString &filename, QFile *out_file) const
{
	if (exerciseCount() == 0)
		return TP_RET_CODE_NOTHING_TO_EXPORT;

	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, false, true, false, true);
		if (!out_file)
			return TP_RET_CODE_OPEN_CREATE_FAILED;
	}

	const QString &strHeader {
		m_calendarDay >= 0 ?
			TPUtils::STR_START_FORMATTED_EXPORT % identifierInFile() % tr("Workout") % "\n\n"_L1 :
			TPUtils::STR_START_FORMATTED_EXPORT % identifierInFile() + tr("Exercises Sheet") + "\n\n"_L1
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
				out_file->write(QString{set->reps}.replace(comp_exercises_separator, comp_exercise_fancy_separator).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setWeightLabel().toUtf8().constData());
				out_file->write(QString{set->weight}.replace(comp_exercises_separator, comp_exercise_fancy_separator).toUtf8().constData());
				out_file->write("\n", 1);
				out_file->write(setNotesLabel().toUtf8().constData());
			}
		}
	}
	out_file->write("\n", 1);
	out_file->write(appUtils()->STR_END_FORMATTED_EXPORT.toUtf8().constData());
	out_file->close();
	return TP_RET_CODE_EXPORT_OK;
}

int DBExercisesModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	clearExercises();
	m_dbModelInterface->clearData();

	int ret{appUtils()->readDataFromFile(in_file, m_dbModelInterface->modelData(), EXERCISES_TOTALCOLS, identifierInFile())};
	if (ret != TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
	{
		const QString &mesoid{m_mesoModel->id(m_mesoIdx)};
		const QString &calendar_day{QString::number(m_calendarDay)};
		for (auto &exercise_entry : m_dbModelInterface->modelData())
		{
			exercise_entry[EXERCISES_FIELD_MESOID] = mesoid;
			exercise_entry[EXERCISES_FIELD_CALENDARDAY] = calendar_day;
			exercise_entry[EXERCISES_FIELD_SPLITLETTER] = m_splitLetter;
		}
		if (fromDatabase(true))
			ret = TP_RET_CODE_IMPORT_OK;
		else
			ret = TP_RET_CODE_IMPORT_FAILED;
	}
	in_file->close();
	return ret;
}

int DBExercisesModel::importFromFormattedFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	beginResetModel();
	clearExercises();
	m_dbModelInterface->clearData();

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
		if (strstr(buf, appUtils()->STR_END_FORMATTED_EXPORT.latin1()) == NULL)
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
							const uint exercise_number{addExercise(-1, false)};
							uint exercise_idx{0};
							int set_number{-1};
							short next_field{EXERCISES_FIELD_TRACKRESTTIMES};
							constexpr short SETS_FIELDS{50};

							while ((lineLength = in_file->readLine(buf, sizeof(buf))) != -1)
							{
								if (lineLength > 5)
								{
									value = buf;
									switch (next_field) {
										case EXERCISES_FIELD_EXERCISES:
											if(strncmp(buf, sub_exercise_delim, sub_exercise_delim_len) == 0)
											{
												addSubExercise(exercise_number, false);
												value.remove(0, sub_exercise_delim_len);
												_setExerciseName(exercise_number, exercise_idx, std::move(value.trimmed()));
												next_field = SETS_FIELDS;
											}
										break;
										case EXERCISES_FIELD_TRACKRESTTIMES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setTrackRestTime(exercise_number, value == tr("Yes"));
											next_field = EXERCISES_FIELD_AUTORESTTIMES;
										break;
										case EXERCISES_FIELD_AUTORESTTIMES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setAutoRestTime(exercise_number, value == tr("Yes"));
											next_field = EXERCISES_FIELD_EXERCISES;
										break;
										case SETS_FIELDS:
											if(strncmp(buf, set_delim, set_delim_len) == 0)
											{
												set_number = addSet(exercise_number, exercise_idx, false);
												next_field = EXERCISES_FIELD_SETTYPES;
											}
										break;
										case EXERCISES_FIELD_SETTYPES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setSetType(exercise_number, exercise_idx, set_number, formatSetTypeToImport(value));
											next_field = EXERCISES_FIELD_RESTTIMES;
										break;
										case EXERCISES_FIELD_RESTTIMES:
											if (!m_exerciseData.at(exercise_number)->track_rest_time)
												_setSetRestTime(exercise_number, exercise_idx, set_number, QTime{0, 0, 0});
											else
											{
												value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
												_setSetRestTime(exercise_number, exercise_idx, set_number,
														appUtils()->timeFromString(value, TPUtils::TF_QML_DISPLAY_NO_HOUR));
											}
											next_field = EXERCISES_FIELD_REPS;
										break;
										case EXERCISES_FIELD_REPS:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setSetReps(exercise_number, exercise_idx, set_number,
															std::move(value.replace(comp_exercise_fancy_separator, QString{comp_exercises_separator})));
											next_field = EXERCISES_FIELD_WEIGHTS;
										break;
										case EXERCISES_FIELD_WEIGHTS:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setSetWeight(exercise_number, exercise_idx, set_number,
															std::move(value.replace(comp_exercise_fancy_separator, QString{comp_exercises_separator})));
											next_field = EXERCISES_FIELD_NOTES;
										break;
										case EXERCISES_FIELD_NOTES:
											value = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
											_setSetNotes(exercise_number, exercise_idx, set_number, std::move(value));
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

	m_exercisesLoaded = true;
	setWorkingExercise(0);
	setWorkingSubExercise(0, 0);
	setWorkingSet(0, 0, 0);
	emit exerciseCountChanged();
	endResetModel();
	return exerciseCount() > 0 ? TP_RET_CODE_IMPORT_OK : TP_RET_CODE_IMPORT_FAILED;
}

int DBExercisesModel::newExercisesFromFile(const QString &filename, const std::optional<bool> &file_formatted)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
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
		if (import_result == TP_RET_CODE_IMPORT_FAILED)
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
			const QString &extra_day_info{tr(" Workout #: ")};
			const int cal_day_idx{static_cast<int>(maybe_extra_info.indexOf(extra_day_info) + extra_day_info.length())};
			if (cal_day_idx > extra_day_info.length()) //workoutModel
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

QString DBExercisesModel::muscularGroup() const
{
	return m_mesoModel->muscularGroup(m_mesoIdx, m_splitLetter);
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

uint DBExercisesModel::addExercise(int exercise_number, const bool from_qml)
{
	bool reutilize_modeldata{false};
	if (exercise_number == -1)
		exercise_number = m_exerciseData.count();
	else
		reutilize_modeldata = true;

	if (from_qml)
		beginInsertRows(QModelIndex{}, exerciseCount(), exerciseCount());
	exerciseEntry *new_exercise{new exerciseEntry};
	new_exercise->exercise_number = exercise_number;
	m_exerciseData.append(new_exercise);
	if (from_qml)
	{
		endInsertRows();
		emit exerciseCountChanged();
		setWorkingExercise(exercise_number);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, exerciseNumberRole});

		if (!reutilize_modeldata || exercise_number >= m_dbModelInterface->modelData().count())
		{
			m_dbModelInterface->modelData().append(std::move(QStringList{} << std::move(appUtils()->newDBTemporaryId()) <<
				m_mesoModel->id(m_mesoIdx) << std::move(QString::number(m_calendarDay)) << m_splitLetter <<
				std::move("0"_L1) << std::move("0"_L1) << std::move(QString{comp_exercises_separator}) <<
				std::move(QString{comp_exercises_separator}) << std::move(QString{comp_exercises_separator}) <<
				std::move(QString{comp_exercises_separator}) << std::move(QString{comp_exercises_separator}) <<
				std::move(QString{comp_exercises_separator}) << std::move(QString{comp_exercises_separator}) <<
				std::move(QString{comp_exercises_separator})
			));
			m_dbModelInterface->setModified(exercise_number, -1);
		}
		appThreadManager()->queueAction(m_db, ThreadManager::InsertRecords);
		addSubExercise(exercise_number, true);
	}
	return exercise_number;
}

void DBExercisesModel::delExercise(const uint exercise_number, const bool from_qml)
{
	if (from_qml)
		beginRemoveRows(QModelIndex{}, exercise_number, exercise_number);

	for (const auto exercise : m_exerciseData | std::views::drop(exercise_number + 1))
		exercise->exercise_number--;
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : std::as_const(exercise->m_exercises))
		qDeleteAll(sub_exercise->sets);
	qDeleteAll(exercise->m_exercises);
	delete exercise;
	m_exerciseData.remove(exercise_number);
	if (from_qml)
	{
		endRemoveRows();
		emit exerciseCountChanged();
		if (exercise_number >= workingExercise())
		{
			if (workingExercise() > 0)
				setWorkingExercise(exercise_number - 1);
		}
		emit dataChanged(index(exercise_number, 0), index(m_exerciseData.count() - 1, 0), QList<int>{1, exerciseNumberRole});

		m_dbModelInterface->setRemovalInfo(exercise_number, QList<uint>{1, EXERCISES_FIELD_ID});
		appThreadManager()->queueAction(m_db, ThreadManager::DeleteRecords);
		m_dbModelInterface->modelData().remove(exercise_number);
	}
}

void DBExercisesModel::moveExercise(const uint from, const uint to)
{
	if (from < m_exerciseData.count() && to < m_exerciseData.count())
	{
		beginMoveRows(QModelIndex{}, from, from, QModelIndex{}, to + 1);
		exerciseEntry *tempExerciseData(std::move(m_exerciseData[from]));

		if (to > from)
		{
			for(uint i{from}; i < to; ++i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i + 1]);
				m_exerciseData.at(i)->exercise_number--;
				m_dbModelInterface->modelData()[i].swap(m_dbModelInterface->modelData()[i + 1]);
				m_dbModelInterface->setAllFieldsModified(i, EXERCISES_TOTALCOLS);
				m_dbModelInterface->setAllFieldsModified(i + 1, EXERCISES_TOTALCOLS);
			}
		}
		else
		{
			for(uint i{from}; i > to; --i)
			{
				m_exerciseData[i] = std::move(m_exerciseData[i - 1]);
				m_exerciseData.at(i)->exercise_number++;
				m_dbModelInterface->modelData()[i].swap(m_dbModelInterface->modelData()[i - 1]);
				m_dbModelInterface->setAllFieldsModified(i, EXERCISES_TOTALCOLS);
				m_dbModelInterface->setAllFieldsModified(i + 1, EXERCISES_TOTALCOLS);
			}
		}
		m_exerciseData[to] = std::move(tempExerciseData);
		m_exerciseData.at(to)->exercise_number = to;
		endMoveRows();
		emit dataChanged(index(to > from ? from : to, 0), index(to > from ? to : from, 0), QList<int>{1, exerciseNumberRole});
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateRecords);
	}
}

void DBExercisesModel::newExerciseChosen()
{
	appPagesListModel()->prevPage();
	newExerciseFromExercisesList();
}

void DBExercisesModel::newExerciseFromExercisesList()
{
	setExerciseName(m_workingExercise, workingSubExercise(m_workingExercise), std::move(
						appExercisesList()->selectedEntriesValue(0, EXERCISES_LIST_FIELD_MAINNAME) % " - "_L1 %
						appExercisesList()->selectedEntriesValue(0, EXERCISES_LIST_FIELD_SUBNAME)));
}

void DBExercisesModel::saveExercises(const int exercise_number, const int exercise_idx, const int set_number, const int field)
{
	if (exercise_idx == EXERCISE_IGNORE_NOTIFY_IDX)
	{
		switch (field)
		{
			case EXERCISES_FIELD_TRACKRESTTIMES:
				m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_TRACKRESTTIMES] = std::move(
								m_exerciseData.at(exercise_number)->track_rest_time ? "1"_L1 : "0"_L1);
			break;
			case EXERCISES_FIELD_AUTORESTTIMES:
				m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_AUTORESTTIMES] = std::move(
								m_exerciseData.at(exercise_number)->track_rest_time ? "1"_L1 : "0"_L1);
			break;
		}
	}
	else if (set_number == EXERCISE_IGNORE_NOTIFY_IDX)
	{
		if (field == EXERCISES_FIELD_EXERCISES)
				appUtils()->setCompositeValue(exercise_idx,
					m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_EXERCISES], comp_exercises_separator);
	}
	else
	{
		stSet *set_info{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)};
		QString str_setinfo{std::move(appUtils()->getCompositeValue(exercise_idx,
					m_dbModelInterface->modelData().at(exercise_number).at(field), comp_exercises_separator))};
		switch (field)
		{
			case EXERCISES_FIELD_NOTES:
				appUtils()->setCompositeValue(set_number, set_info->notes, str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_NOTES], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_COMPLETED:
				appUtils()->setCompositeValue(set_number, set_info->completed ? "1"_L1 : "0"_L1, str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_COMPLETED], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_SETTYPES:
				appUtils()->setCompositeValue(set_number, QString::number(set_info->type), str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_SETTYPES], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_RESTTIMES:
				appUtils()->setCompositeValue(set_number, appUtils()->formatTime(set_info->restTime,
													TPUtils::TF_QML_DISPLAY_NO_HOUR), str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_RESTTIMES], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_SUBSETS:
				appUtils()->setCompositeValue(set_number, set_info->subsets, str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_SUBSETS], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_REPS:
				appUtils()->setCompositeValue(set_number, set_info->reps, str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_REPS], comp_exercises_separator);
			break;
			case EXERCISES_FIELD_WEIGHTS:
				appUtils()->setCompositeValue(set_number, set_info->weight, str_setinfo, set_separator);
				appUtils()->setCompositeValue(exercise_idx, str_setinfo,
					m_dbModelInterface->modelData()[exercise_number][EXERCISES_FIELD_WEIGHTS], comp_exercises_separator);
			break;
		}
	}
	m_dbModelInterface->setModified(exercise_number, field);
	appThreadManager()->queueAction(m_db, ThreadManager::UpdateOneField);
}

void DBExercisesModel::addSubExercise(const uint exercise_number, const bool from_qml)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* new_sub_exercise{new stExercise};
	const uint exercise_idx{static_cast<uint>(exercise->m_exercises.count())};
	new_sub_exercise->parent = exercise;
	new_sub_exercise->exercise_idx = exercise_idx;
	exercise->m_exercises.append(new_sub_exercise);
	if (from_qml)
	{
		setWorkingSubExercise(exercise_idx, exercise_number);
		emit subExerciseCountChanged(exercise_number);
		QList<int> modified_fields{};
		for (uint i{EXERCISES_FIELD_EXERCISES}; i < EXERCISES_TOTALCOLS; ++i)
		{
			appUtils()->setCompositeValue(exercise_idx, QString{}, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator);
			modified_fields.append(i);
		}
		m_dbModelInterface->setModified(exercise_number, modified_fields);
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateSeveralFields);

		setExerciseName(exercise_number, exercise_idx, tr("Choose exercise..."));
	}
}

void DBExercisesModel::delSubExercise(const uint exercise_number, const uint exercise_idx, const bool from_qml)
{
	removeAllSets(exercise_number, exercise_idx, from_qml);
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	qDeleteAll(sub_exercise->sets);

	if (exercise_idx == 0)
		setExerciseName(exercise_number, exercise_idx, tr("Choose exercise..."));
	else
	{
		for (const auto sub_exercise : exercise->m_exercises | std::views::drop(exercise_idx + 1))
			sub_exercise->exercise_idx--;
		exercise->m_exercises.remove(exercise_idx);
		delete sub_exercise;
	}

	if (from_qml && exercise_idx > 0)
	{
		emit subExerciseCountChanged(exercise_number);
		if (workingSubExercise(exercise_number) >= exercise_idx)
		{
			if (workingSubExercise(exercise_number) > 0)
				setWorkingSubExercise(exercise_idx - 1, exercise_number);
		}

		QList<int> modified_fields{};
		for (uint i{EXERCISES_FIELD_EXERCISES}; i < EXERCISES_TOTALCOLS; ++i)
		{
			if (appUtils()->removeFieldFromCompositeValue(exercise_idx,
									m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator))
				modified_fields.append(i);
		}
		m_dbModelInterface->setModified(exercise_number, modified_fields);
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateSeveralFields);
	}
}

uint DBExercisesModel::addSet(const uint exercise_number, const uint exercise_idx, const bool from_qml)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	stSet* new_set{new stSet};
	const uint set_number{static_cast<uint>(sub_exercise->sets.count())};
	new_set->set_number = set_number;
	new_set->parent = sub_exercise;
	sub_exercise->sets.append(new_set);
	if (from_qml)
	{
		emit setsNumberChanged(exercise_number, exercise_idx);
		if (isWorkout())
			setModeForSet(new_set);
		setWorkingSet(set_number, exercise_number, exercise_idx);
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, setsNumberRole});

		QList<int> modified_fields{};
		for (uint i{EXERCISES_FIELD_NOTES}; i < EXERCISES_TOTALCOLS; ++i)
		{
			QString sub_exercise_info{std::move(appUtils()->getCompositeValue(exercise_idx, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator))};
			sub_exercise_info.append(set_separator);
			appUtils()->setCompositeValue(exercise_idx, sub_exercise_info, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator);
			modified_fields.append(i);
		}
		m_dbModelInterface->setModified(exercise_number, modified_fields);
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateSeveralFields);
	}
	return set_number;
}

void DBExercisesModel::delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool from_qml)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	stExercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	for (const auto set : sub_exercise->sets | std::views::drop(set_number + 1))
		set->set_number--;
	delete sub_exercise->sets.at(set_number);
	sub_exercise->sets.remove(set_number);

	if (from_qml)
	{
		emit setsNumberChanged(exercise_number, exercise_idx);
		if (set_number >= workingSet(exercise_number, exercise_idx))
		{
			if (workingSet(exercise_number, exercise_idx) > 0)
				setWorkingSet(set_number - 1, exercise_number, exercise_idx);
		}
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, setsNumberRole});

		QList<int> modified_fields{};
		for (uint i{EXERCISES_FIELD_NOTES}; i < EXERCISES_TOTALCOLS; ++i)
		{
			QString sub_exercise_info{std::move(appUtils()->getCompositeValue(exercise_idx, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator))};
			if (appUtils()->removeFieldFromCompositeValue(set_number, sub_exercise_info, set_separator))
			{
				appUtils()->setCompositeValue(exercise_idx, sub_exercise_info, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator);
				modified_fields.append(i);
			}
		}
		m_dbModelInterface->setModified(exercise_number, modified_fields);
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateSeveralFields);
	}
}

void DBExercisesModel::removeAllSets(const uint exercise_number, const uint exercise_idx, const bool from_qml)
{
	for (auto set_number{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count() - 1}; set_number >= 0; --set_number)
		delSet(exercise_number, exercise_idx, set_number, from_qml);
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
				for (uint field{EXERCISES_FIELD_NOTES}; field < EXERCISES_TOTALCOLS; ++field)
				{
					QString sub_exercise_info{std::move(appUtils()->getCompositeValue(
						exercise_idx, m_dbModelInterface->modelData()[exercise_number][field], comp_exercises_separator))};
					QString set_info_from{std::move(appUtils()->getCompositeValue(i, sub_exercise_info, set_separator))};
					QString set_info_to{std::move(appUtils()->getCompositeValue(i + 1, sub_exercise_info, set_separator))};
					appUtils()->setCompositeValue(i, set_info_to, sub_exercise_info, set_separator);
					appUtils()->setCompositeValue(i + 1, set_info_from, sub_exercise_info, set_separator);
					appUtils()->setCompositeValue(exercise_idx, sub_exercise_info, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator);
					m_dbModelInterface->setModified(exercise_number, field);
				}
			}
		}
		else
		{
			for(uint i{from_set}; i > to_set; --i)
			{
				sub_exercise->sets[i] = std::move(sub_exercise->sets[i - 1]);
				sub_exercise->sets.at(i)->set_number++;
				for (uint field{EXERCISES_FIELD_NOTES}; field < EXERCISES_TOTALCOLS; ++field)
				{
					QString sub_exercise_info{std::move(appUtils()->getCompositeValue(
						exercise_idx, m_dbModelInterface->modelData()[exercise_number][field], comp_exercises_separator))};
					QString set_info_from{std::move(appUtils()->getCompositeValue(i, sub_exercise_info, set_separator))};
					QString set_info_to{std::move(appUtils()->getCompositeValue(i - 1, sub_exercise_info, set_separator))};
					appUtils()->setCompositeValue(i, set_info_to, sub_exercise_info, set_separator);
					appUtils()->setCompositeValue(i - 1, set_info_from, sub_exercise_info, set_separator);
					appUtils()->setCompositeValue(exercise_idx, sub_exercise_info, m_dbModelInterface->modelData()[exercise_number][i], comp_exercises_separator);
					m_dbModelInterface->setModified(exercise_number, field);
				}
			}
		}
		sub_exercise->sets[to_set] = std::move(tempSet);
		sub_exercise->sets.at(to_set)->set_number = to_set;
		emit setsNumberChanged(exercise_number, exercise_idx);
		appThreadManager()->queueAction(m_db, ThreadManager::UpdateRecords);
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
		emit dataChanged(index(new_workingexercise, 0), index(new_workingexercise, 0), QList<int>{1, workingExerciseRole});
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
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, workingSubExerciseRole});
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
		emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, workingSetRole});
	}
}

QString DBExercisesModel::exerciseName(const uint exercise_number, const uint exercise_idx) const
{
	if (exercise_number < m_exerciseData.count())
	{
		if (exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
			return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name;
	}
	return QString{};
}

void DBExercisesModel::setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = new_name;
	emit exerciseNameChanged(exercise_number, exercise_idx);
	emit exerciseModified(exercise_number, exercise_idx, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_FIELD_EXERCISES);
}

QString DBExercisesModel::allExerciseNames(const uint exercise_number) const
{
	QString names;
	for (const auto exercise : std::as_const(m_exerciseData.at(exercise_number)->m_exercises))
			names += std::move(exercise->name % " | "_L1);
	names.chop(3);
	return names;
}

bool DBExercisesModel::trackRestTime(const uint exercise_number) const
{
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->track_rest_time : false;
}

void DBExercisesModel::setTrackRestTime(const uint exercise_number, const bool track_resttime)
{
	m_exerciseData.at(exercise_number)->track_rest_time = track_resttime;
	changeAllSetsMode(exercise_number);
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_FIELD_TRACKRESTTIMES);
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, trackRestTimeRole});
}

bool DBExercisesModel::autoRestTime(const uint exercise_number) const
{
	return exercise_number < m_exerciseData.count() ? m_exerciseData.at(exercise_number)->auto_rest_time : false;
}

void DBExercisesModel::setAutoRestTime(const uint exercise_number, const bool auto_resttime)
{
	m_exerciseData.at(exercise_number)->auto_rest_time = auto_resttime;
	changeAllSetsMode(exercise_number);
	emit exerciseModified(exercise_number, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISE_IGNORE_NOTIFY_IDX, EXERCISES_FIELD_AUTORESTTIMES);
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, autoRestTimeRole});
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

void DBExercisesModel::setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type, const bool from_qml)
{
	const QList<stSet*> &sets{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets};
	stSet *set{sets.at(set_number)};
	set->type = static_cast<TPSetTypes>(new_type);
	if (from_qml)
	{
		if (set_number != 0)
		{
			stSet *prev_set{sets.at(set_number - 1)};
			setSetRestTime(exercise_number, exercise_idx, set_number, appUtils()->formatTime(
										suggestedRestTime(prev_set->restTime, set->type), TPUtils::TF_QML_DISPLAY_NO_HOUR));
			setSetReps(exercise_number, exercise_idx, set_number, suggestedReps(prev_set->reps, set->type));
			setSetWeight(exercise_number, exercise_idx, set_number, suggestedWeight(prev_set->weight, set->type, sets.count()));
		}
		else
		{
			setSetRestTime(exercise_number, exercise_idx, set_number, appUtils()->formatTime(
										suggestedRestTime(QTime{0,0,0}, set->type), TPUtils::TF_QML_DISPLAY_NO_HOUR));
			setSetReps(exercise_number, exercise_idx, set_number, suggestedReps(QString{}, set->type));
			setSetWeight(exercise_number, exercise_idx, set_number, suggestedWeight(QString{}, set->type, sets.count()));
		}
		setSetSubSets(exercise_number, exercise_idx, set_number, suggestedSubSets(set->type));
		emit setTypeChanged(exercise_number, exercise_idx, set_number);
		emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_SETTYPES);
	}
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
						std::move(appUtils()->timeFromString(new_time, TPUtils::TF_QML_DISPLAY_NO_HOUR));
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_RESTTIMES);
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
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_SUBSETS);
}

void DBExercisesModel::addSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type >= Drop)
	{
		const uint n_subsets{setSubSets(exercise_number, exercise_idx, set_number).toUInt()};
		setSetSubSets(exercise_number, exercise_idx, set_number, QString::number(n_subsets + 1));
		QString prev_set_reps;
		stSet *set{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)};
		if (set_number > 0)
		{
			stSet *prev_set{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number - 1)};
			prev_set_reps = prev_set->reps;
		}
		setSetReps(exercise_number, exercise_idx, set_number, suggestedReps(prev_set_reps, set->type, n_subsets), n_subsets);
	}
}

void DBExercisesModel::delSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type >= Drop)
	{
		const uint new_subsets{setSubSets(exercise_number, exercise_idx, set_number).toUInt() - 1};
		if (appUtils()->removeFieldFromCompositeValue(new_subsets,
							m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator))
		{
			if (appUtils()->removeFieldFromCompositeValue(new_subsets,
							m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight, record_separator))
			{
				setSetSubSets(exercise_number, exercise_idx, set_number, QString::number(new_subsets));
				emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_SUBSETS);
			}
		}
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

void DBExercisesModel::setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number,
																				const QString &new_reps, const uint subset)
{
	if (m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type < Drop)
		m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps = new_reps;
	else
		appUtils()->setCompositeValue(subset, new_reps,
			m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps, record_separator);
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_REPS);
}

QString DBExercisesModel::suggestedWeight(const QString &prev_weight, const uint set_type, const uint n_sets, const uint subset) const
{
	switch (set_type)
	{
		case Regular:
			return prev_weight.isEmpty() ? "20"_L1 : prev_weight;
		break;
		case Pyramid:
			return prev_weight.isEmpty() ? "20"_L1 : appUtils()->appLocale()->toString(qCeil(prev_weight.toUInt() * 1.2));
		break;
		case ReversePyramid:
			return prev_weight.isEmpty() ? "80"_L1 : appUtils()->appLocale()->toString(qCeil(prev_weight.toUInt() * 0.6));
		break;
		case Drop:
			return prev_weight.isEmpty() ? "80"_L1 : dropSetReps(prev_weight, subset);
		break;
		case Cluster:
			return prev_weight.isEmpty() ? clusterWeight("100"_L1) : clusterReps(prev_weight, subset);
		case MyoReps:
			return prev_weight.isEmpty() ? myorepsWeight("100"_L1, n_sets) : myorepsReps(prev_weight, n_sets, subset);
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
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_WEIGHTS);
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
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_NOTES);
}

bool DBExercisesModel::setCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	if (isWorkout())
	{
		if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
		{
			if (set_number < m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.count())
				return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed;
		}
	}
	return false;
}

void DBExercisesModel::setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed = completed;
	emit dataChanged(index(exercise_number, 0), index(exercise_number, 0), QList<int>{1, exerciseCompletedRole});
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_COMPLETED);
}

bool DBExercisesModel::allSetsCompleted(int exercise_number, int exercise_idx) const
{
	if (isWorkout())
	{
		if (exercise_number >= 0 && exercise_number < m_exerciseData.count())
		{
			if (exercise_idx >= 0 && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
			{
				const QList<stSet*> &sets{m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets};
				for (const stSet* set : sets)
				{
					if (!set->completed)
						return false;
				}
				return true;
			}
		}
	}
	return false;
}

void DBExercisesModel::setAllSetsCompleted(const bool completed, int exercise_number, int exercise_idx)
{
	const uint last_exercise_number{exercise_number == -1 ? exerciseCount() : exercise_number + 1};
	for (int i{exercise_number == -1 ? 0 : exercise_number}; i < last_exercise_number; ++i)
	{
		const uint last_exercise_idx{exercise_idx == -1 ? static_cast<uint>(m_exerciseData.at(i)->m_exercises.count()) : exercise_idx + 1};
		for (int x{exercise_idx == -1 ? 0 : exercise_idx}; x < last_exercise_idx; ++x)
		{
			for (const auto set : std::as_const(m_exerciseData.at(i)->m_exercises.at(x)->sets))
				set->completed = completed;
		}
	}
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

bool DBExercisesModel::syncGiantSets(const uint exercise_number, const uint exercise_idx) const
{
	if (exercise_number < m_exerciseData.count() && exercise_idx < m_exerciseData.at(exercise_number)->m_exercises.count())
		return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sync_giant_sets;
	return false;
}

void DBExercisesModel::setSyncGiantSets(const uint exercise_number, const uint exercise_idx, const bool sync)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sync_giant_sets = sync;
	if (sync)
	{
		for (const auto set : std::as_const(m_exerciseData.at(exercise_number)->m_exercises.at(0)->sets))
		{
			addSet(exercise_number, exercise_idx);
			setSetType(exercise_number, exercise_idx, set->set_number, set->type);
			setSetRestTime(exercise_number, exercise_idx, set->set_number, appUtils()->formatTime(set->restTime, TPUtils::TF_QML_DISPLAY_NO_HOUR));
			setSetSubSets(exercise_number, exercise_idx, set->set_number, set->subsets);
			setSetReps(exercise_number, exercise_idx, set->set_number, set->reps);
			setSetWeight(exercise_number, exercise_idx, set->set_number, set->weight);
			setSetNotes(exercise_number, exercise_idx, set->set_number, set->notes);
			setSetCompleted(exercise_number, exercise_idx, set->set_number, set->completed);
		}
		setWorkingSet(0, exercise_number, exercise_idx);
	}
}

QVariant DBExercisesModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_exerciseData.count())
	{
		switch (role)
		{
			case exerciseNumberRole: return QString::number(row+1);
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
		//dataChanged is emitted in the setters
		switch (role)
		{
			case workingExerciseRole:		setWorkingExercise(value.toUInt());
			case workingSubExerciseRole:	setWorkingSubExercise(row, value.toUInt());
			case workingSetRole:			setWorkingSet(row, workingSubExercise(row), value.toUInt());
			case trackRestTimeRole:			setTrackRestTime(row, value.toBool());
			case autoRestTimeRole:			setAutoRestTime(row, value.toBool());
			break;
		}
		return true;
	}
	return false;
}

void DBExercisesModel::commonConstructor(const bool load_from_db)
{
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesModel::labelChanged);
	roleToString(exerciseNumber)
	roleToString(setsNumber)
	roleToString(exerciseCompleted)
	roleToString(workingExercise)
	roleToString(workingSubExercise)
	roleToString(workingSet)
	roleToString(trackRestTime)
	roleToString(autoRestTime)

	if (m_calendarDay >= 0)
	{
		m_splitLetter = m_mesoModel->calendar(m_mesoIdx)->splitLetter().at(0);
		m_identifierInFile = appUtils()->workoutFileIdentifier;
	}
	else
		m_identifierInFile = appUtils()->splitFileIdentifier;
	m_identifierInFile += m_splitLetter;

	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field)
	{
		if (meso_idx == m_mesoIdx)
		{
			if (field >= MESO_FIELD_SPLITA && field <= MESO_FIELD_SPLITF)
			{
				if (static_cast<int>(m_splitLetter.cell()) - static_cast<int>('A') == field - MESO_FIELD_SPLITA)
					emit muscularGroupChanged();
			}
		}
	});

	connect(this, &DBExercisesModel::exerciseModified, this, &DBExercisesModel::saveExercises);

	if (load_from_db)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_db, &DBWorkoutsOrSplitsTable::exercisesLoaded, this, [this,conn]
														(const uint meso_idx, const bool success, const QVariant &extra_info)
		{
			if (meso_idx == m_mesoIdx && m_calendarDay != -1 ?
												extra_info.toInt() == m_calendarDay : extra_info.toChar() == m_splitLetter)
			{
				disconnect(*conn);
				static_cast<void>(fromDatabase(success));
			}
		});
		m_dbModelInterface = new DBModelInterfaceExercises{this};
		appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords, m_dbModelInterface);
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

const QString DBExercisesModel::exportExtraInfo() const
{
	QString extra_info{std::move(splitLabel() % splitLetter() % " ("_L1 % m_mesoModel->muscularGroup(
																							m_mesoIdx, splitLetter()) % ')')};
	if (m_calendarDay >= 0)
		extra_info += tr(" Workout #: ") % QString::number(m_calendarDay) % tr(" at ") % appUtils()->formatDate(
																		m_mesoModel->calendar(m_mesoIdx)->date(m_calendarDay));
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
	if (!isWorkout())
		return;
	exerciseEntry *exercise{m_exerciseData.at(exercise_number)};
	for (const auto sub_exercise : std::as_const(exercise->m_exercises))
	{
		for (const auto set : std::as_const(sub_exercise->sets))
			setModeForSet(set);
	}
}

void DBExercisesModel::_setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QTime &time)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->restTime = time;
	emit exerciseModified(exercise_number, exercise_idx, set_number, EXERCISES_FIELD_RESTTIMES);
}

void DBExercisesModel::_setExerciseName(const uint exercise_number, const uint exercise_idx, QString &&new_name)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = std::forward<QString>(new_name);
}

void DBExercisesModel::_setTrackRestTime(const uint exercise_number, const bool track_resttime)
{
	m_exerciseData.at(exercise_number)->track_rest_time = track_resttime;
	changeAllSetsMode(exercise_number);
}

void DBExercisesModel::_setAutoRestTime(const uint exercise_number, const bool auto_resttime)
{
	m_exerciseData.at(exercise_number)->auto_rest_time = auto_resttime;
	changeAllSetsMode(exercise_number);
}

void DBExercisesModel::_setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->type = static_cast<TPSetTypes>(new_type);
}

void DBExercisesModel::_setSetSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_subsets)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->subsets = new_subsets;
}

void DBExercisesModel::_setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_reps)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->reps = std::forward<QString>(new_reps);
}

void DBExercisesModel::_setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_weight)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->weight = std::forward<QString>(new_weight);
}

void DBExercisesModel::_setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_notes)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->notes = std::forward<QString>(new_notes);
}

void DBExercisesModel::_setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->sets.at(set_number)->completed = completed;
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
