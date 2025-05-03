#include "dbworkoutmodel.h"

#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
//#include "dbexercisesmodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "exercises_helper/setpredictionvalues.h"

#include <QtMath>
#include <utility>

const auto &value = []<typename T>(const std::optional<T> &retValue) { return retValue.has_value() ? retValue.value() : T{}; };

using namespace SetPredictionValues;

struct st_set {
	uint number;
	TPSetTypes type;
	QTime restTime;
	uint subsets, reps, weight;
	QString notes;
	TPBool completed;
};

struct st_exercise {
	QString name;
	TPBool mb_TrackRestTime, mb_AutoRestTime;
	QList<st_set*> sets;
};

DBWorkoutModel::DBWorkoutModel(DBMesoCalendarManager *parent, const uint meso_idx, const uint calendar_day)
	: QAbstractListModel{parent}, m_calendarManager{parent},  m_mesoIdx{meso_idx}, m_calendarDay{calendar_day}
{
	setObjectName(DBTrainingDayObjectName);
}

void DBWorkoutModel::fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse)
{
	m_id = list.at(WORKOUT_COL_ID);

	QStringList exercises_names(std::move(list.at(WORKOUT_COL_EXERCISES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList setstypes(std::move(list.at(WORKOUT_COL_SETTYPES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList resttimes(std::move(list.at(WORKOUT_COL_RESTTIMES).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList subsets(std::move(list.at(WORKOUT_COL_SUBSETS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList reps(std::move(list.at(WORKOUT_COL_REPS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList weights(std::move(list.at(WORKOUT_COL_WEIGHTS).split(exercises_separator, Qt::SkipEmptyParts)));
	QStringList notes(std::move(list.at(WORKOUT_COL_NOTES).split(exercises_separator, Qt::SkipEmptyParts)));

	QStringList completed;
	if (!bClearSomeFieldsForReUse)
		completed = std::move(list.at(WORKOUT_COL_COMPLETED).split(exercises_separator, Qt::SkipEmptyParts));
	else //reset for new use
	{
		QString temp{list.at(WORKOUT_COL_COMPLETED)};
		completed = std::move(temp.replace(STR_ONE, STR_ZERO).split(exercises_separator, Qt::SkipEmptyParts));
	}

	for(uint i{0}; i < exercises_names.count(); ++i)
	{
		exerciseEntry *exercise_entry{new exerciseEntry};
		exercise_entry->exercise = std::move(exercises_names[i]);
		exercise_entry->settype = std::move(setstypes.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->resttime = std::move(resttimes.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->subsets = std::move(subsets.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->reps = std::move(reps.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->weight = std::move(weights.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->notes = std::move(notes.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->completed = std::move(completed.at(i).split(record_separator, Qt::SkipEmptyParts));
		exercise_entry->nsets = m_exerciseData.at(i)->settype.count();
		m_exerciseData.append(std::move(exercise_entry));
	}
	emit exerciseCountChanged();
	if (bClearSomeFieldsForReUse) //import from another workout. Save the copied data into the new workout
		emit workoutChanged();
}

const QStringList DBWorkoutModel::getSaveInfo() const
{
	QStringList data{WORKOUT_TOTALCOLS};
	data[WORKOUT_COL_ID] = m_id;
	data[WORKOUT_COL_MESOID] = appMesoModel()->id(m_mesoIdx);

	for (const auto exercise_info : m_exerciseData)
	{
		data[WORKOUT_COL_EXERCISES].append(exercise_info->exercise + exercises_separator);
		for(uint i{0}; i < exercise_info->nsets; ++i)
		{
			data[WORKOUT_COL_SETTYPES].append(exercise_info->settype.at(i) + record_separator);
			data[WORKOUT_COL_RESTTIMES].append(exercise_info->resttime.at(i) + record_separator);
			data[WORKOUT_COL_SUBSETS].append((i < exercise_info->subsets.count() ? exercise_info->subsets.at(i) : STR_ZERO) + record_separator);
			data[WORKOUT_COL_REPS].append(exercise_info->reps.at(i) + record_separator);
			data[WORKOUT_COL_WEIGHTS].append(exercise_info->weight.at(i) + record_separator);
			data[WORKOUT_COL_NOTES].append((i < exercise_info->notes.count() ? exercise_info->notes.at(i) : " "_L1) + record_separator);
			data[WORKOUT_COL_COMPLETED].append(exercise_info->completed.at(i) + record_separator);
		}
		data[WORKOUT_COL_SETTYPES].append(exercises_separator);
		data[WORKOUT_COL_RESTTIMES].append(exercises_separator);
		data[WORKOUT_COL_SUBSETS].append(exercises_separator);
		data[WORKOUT_COL_REPS].append(exercises_separator);
		data[WORKOUT_COL_WEIGHTS].append(exercises_separator);
		data[WORKOUT_COL_NOTES].append(exercises_separator);
		data[WORKOUT_COL_COMPLETED].append(exercises_separator);
	}
	return data;
}

void DBWorkoutModel::convertMesoSplitModelToTDayModel(const DBMesoSplitModel *const splitModel)
{	
	for(uint i{0}; i < splitModel->count(); ++i)
	{
		exerciseEntry *exercise_entry{new exerciseEntry};
		exercise_entry->exercise = std::move(splitModel->exerciseName(i, true));
		exercise_entry->settype = std::move(splitModel->_setsTypes(i).split(set_separator));
		exercise_entry->resttime = std::move(splitModel->_setsRestTimes(i).split(set_separator));
		exercise_entry->subsets = std::move(splitModel->_setsSubSets(i).split(set_separator));
		exercise_entry->reps = std::move(splitModel->_setsReps(i).split(set_separator));
		exercise_entry->weight = std::move(splitModel->_setsWeights(i).split(set_separator));
		exercise_entry->notes = std::move(splitModel->_setsNotes(i).split(set_separator));
		exercise_entry->nsets = exercise_entry->settype.count();
		exercise_entry->completed = std::move(appUtils()->makeCompositeValue(STR_ZERO, exercise_entry->nsets, set_separator).split(set_separator));
		m_exerciseData.append(exercise_entry);
	}
	emit exerciseCountChanged();
	emit workoutChanged(); //save now
}

int DBWorkoutModel::exportToFile(const QString &filename) const
{
	if (exerciseCount() == 0)
		return APPWINDOW_MSG_NOTHING_TO_EXPORT;

	QFile *outFile{appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (!outFile)
		return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;

	const QString &strHeader{"## "_L1 + tr("Workout") + " - 0x000"_L1 + QString::number(TRAININGDAY_TABLE_ID) + "\n\n"_L1};
	outFile->write(strHeader.toUtf8().constData());
	outFile->write(exportExtraInfo().toUtf8().constData());
	outFile->write("\n\n", 2);

	QString setsTypes, subSets;
	bool bHasSubsSets{false};
	for (uint i{0}; i < m_exerciseData.count(); ++i)
	{
		outFile->write(QString(QString::number(i+1) + ": "_L1).toUtf8().constData());
		outFile->write(QString(exerciseName(i)).replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
		outFile->write("\n", 1);
		outFile->write(tr("Number of sets: ").toUtf8().constData());
		outFile->write(QString::number(setsNumber(i)).toUtf8().constData());
		outFile->write("\n", 1);
		outFile->write(tr("Rest time between sets: ").toUtf8().constData());
		outFile->write(setRestTime(0, i).replace(set_separator, fancy_record_separator2).toUtf8().constData());
		outFile->write("\n", 1);

		for (uint n{0}; n < setsNumber(i); ++n)
		{
			const uint settype{setType(n, i)};
			setsTypes += formatSetTypeToExport(QString::number(settype)) + fancy_record_separator2;
			subSets += setSubSets(n, i) + fancy_record_separator2;
			if (settype == 2 || settype == 3 || settype == 5)
				bHasSubsSets = true;
		}
		outFile->write(tr("Type of sets: ").toUtf8().constData());
			setsTypes.chop(1);
		outFile->write(setsTypes.toUtf8().constData());
		setsTypes.clear();
		if (bHasSubsSets)
		{
			outFile->write(tr("Number of subsets: ").toUtf8().constData());
			subSets.chop(1);
			outFile->write(subSets.toUtf8().constData());
		}
		subSets.clear();
		outFile->write(tr("Initial number of reps: ").toUtf8().constData());
		outFile->write(setReps(0, 1).replace(set_separator, fancy_record_separator2).replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
		outFile->write("\n", 1);
		outFile->write(tr("Initial weight: ").toUtf8().constData());
		outFile->write(setWeight(0, 1).replace(set_separator, fancy_record_separator2).replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
		outFile->write("\n", 1);
		outFile->write(tr("Note for the sets: ").toUtf8().constData());
		outFile->write(setNotes(0, i).toUtf8().constData());
		outFile->write("\n\n", 2);
	}
	outFile->write(STR_END_EXPORT.toUtf8().constData());
	outFile->close();
	delete outFile;
	return APPWINDOW_MSG_EXPORT_OK;
}

int DBWorkoutModel::importFromFile(const QString& filename)
{
	QFile *inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	QString value;
	uint exercise_number(0);
	QString type, resttime, subsets, reps, weight, notes, strTypes;
	const QString tableIdStr("0x000"_L1 + QString::number(TRAININGDAY_TABLE_ID));
	bool bFoundModelInfo(false);
	char buf[128];
	qint64 lineLength(0);
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (!bFoundModelInfo)
					bFoundModelInfo = strstr(buf, tableIdStr.toLatin1().constData()) != NULL;
				else
				{
					value = buf;
					newExercise(exercise_number);
					setExerciseName(exercise_number, value.remove(0, value.indexOf(':') + 2).trimmed().replace(comp_exercise_fancy_separator, QChar(comp_exercise_separator)));

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					const uint nsets{value.remove(0, value.indexOf(':') + 2).trimmed().toUInt()};

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					resttime = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator));

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					strTypes = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator));
					type = std::move(formatSetTypeToImport(strTypes));

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					if (value.indexOf(tr("subsets")) == -1)
					{
						subsets = "0"_L1;
						reps = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator));
					}
					else
					{
						subsets = std::move(value.remove(0, value.indexOf(':') + 2).trimmed());
						if (inFile->readLine(buf, sizeof(buf)) == -1)
							return false;
						reps = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator));
					}

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					weight = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator).replace(comp_exercise_fancy_separator, QString(comp_exercise_separator)));

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					notes = std::move(value.remove(0, value.indexOf(':') + 2).trimmed().replace(fancy_record_separator2, set_separator).replace(comp_exercise_fancy_separator, QString(comp_exercise_separator)));
					if (notes.isEmpty())
						notes = " "_L1;

					newFirstSet(exercise_number, appUtils()->getCompositeValue(0, type, set_separator).toUInt(),
						appUtils()->getCompositeValue(0, reps, set_separator), appUtils()->getCompositeValue(0, weight, set_separator),
						appUtils()->getCompositeValue(0, resttime, set_separator), appUtils()->getCompositeValue(0, subsets, set_separator),
						appUtils()->getCompositeValue(0, notes, set_separator));
					for (uint i(1); i < nsets; ++i)
						newSet(exercise_number, i, appUtils()->getCompositeValue(i, type, set_separator).toUInt(),
						appUtils()->getCompositeValue(i, reps, set_separator), appUtils()->getCompositeValue(i, weight, set_separator),
						appUtils()->getCompositeValue(i, resttime, set_separator), appUtils()->getCompositeValue(i, subsets, set_separator));
				}
				++exercise_number;
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	return exerciseCount() > 0 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

//Don't put any colon ':' in here. Import will fail. All value()s returned from std::optional by m_calendarManager are assumed to
//contain a valid value because export will only be an option if those values are valid. Those checks are made elsewhere in the code path.
const QString DBWorkoutModel::exportExtraInfo() const
{
	const QChar &splitLetter{m_calendarManager->splitLetter(m_mesoIdx, m_calendarDay).value().at(0)};
	return tr("Workout #") + m_calendarManager->workoutNumber(m_mesoIdx, m_calendarDay).value() + tr(", split ") +
			splitLetter + " ("_L1 + appMesoModel()->muscularGroup(m_mesoIdx, splitLetter) + tr(") at ") +
			appUtils()->formatDate(m_calendarManager->date(m_mesoIdx, m_calendarDay).value());
}

const QString DBWorkoutModel::formatSetTypeToExport(const QString& fieldValue) const
{
	QString ret{fieldValue};
	ret.replace("0"_L1, tr("Regular"));
	ret.replace("1"_L1, tr("Pyramid"));
	ret.replace("2"_L1, tr("Drop Set"));
	ret.replace("3"_L1, tr("Cluster Set"));
	ret.replace("4"_L1, tr("Giant Set"));
	ret.replace("5"_L1, tr("Myo Reps"));
	ret.replace("6"_L1, tr("Inverted Pyramid"));
	return ret;
}

const QString DBWorkoutModel::formatSetTypeToImport(const QString& fieldValue) const
{
	QString ret;
	if (!fieldValue.isEmpty())
	{
		ret = fieldValue;
		ret.replace(tr("Regular"), "0"_L1);
		ret.replace(tr("Pyramid"), "1"_L1);
		ret.replace(tr("Drop Set"), "2"_L1);
		ret.replace(tr("Cluster Set"), "3"_L1);
		ret.replace(tr("Giant Set"), "4"_L1);
		ret.replace(tr("Myo Reps"), "5"_L1);
		ret.replace(tr("Inverted Pyramid"), "6"_L1);
	}
	else
		ret = "0"_L1;
	return ret;
}

void DBWorkoutModel::newExercise(const uint exercise_number)
{
	const uint total{static_cast<uint>(m_exerciseData.count())};
	const int n{static_cast<int>(exercise_number - total)};
	if (n >= 0)
	{
		for(uint i{0}; i <= n; ++i)
			m_exerciseData.append(new exerciseEntry);
		emit exerciseCountChanged();
	}
}

void DBWorkoutModel::removeExercise(const uint exercise_number)
{
	Q_ASSERT_X(exercise_number < m_exerciseData.count(), "DBWorkoutModel::removeExercise", "out of range exercise_number");
	delete m_exerciseData.at(exercise_number);
	m_exerciseData.remove(exercise_number);
	emit exerciseCountChanged();
}

void DBWorkoutModel::moveExercise(const uint from, const uint to)
{
	if (from < m_exerciseData.count() && to < m_exerciseData.count())
	{
		exerciseEntry *tempExerciseData(std::move(m_exerciseData[from]));

		if (to > from)
		{
			for(uint i{from}; i < to; ++i)
				m_exerciseData[i] = std::move(m_exerciseData[i+1]);
		}
		else
		{
			for(uint i{from}; i > to; --i)
				m_exerciseData[i] = std::move(m_exerciseData[i-1]);
		}
		m_exerciseData[to] = std::move(tempExerciseData);
		emit workoutChanged();
	}
}

int DBWorkoutModel::addExercise()
{
	exerciseEntry *new_exercise{new exerciseEntry};
	st_exercise* new_sub_exercise{new st_exercise};
	new_exercise->m_exercises.append(new_sub_exercise);
	m_exerciseData.append(new_exercise);
	emit workoutChanged();
	return m_exerciseData.count() - 1;
}

void DBWorkoutModel::delExercise(const uint exercise_number)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : exercise->m_exercises)
		qDeleteAll(sub_exercise->sets);
	qDeleteAll(exercise->m_exercises);
	delete exercise;
	m_exerciseData.remove(exercise_number);
	emit workoutChanged();
}

int DBWorkoutModel::addSubExercise(const uint exercise_number)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	st_exercise* new_sub_exercise{new st_exercise};
	exercise->m_exercises.append(new_sub_exercise);
	emit workoutChanged();
	return exercise->m_exercises.count() - 1;
}

void DBWorkoutModel::delSubExercise(const uint exercise_number, const uint exercise_idx)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	st_exercise* sub_exercise{exercise->m_exercises.at(exercise_idx)};
	qDeleteAll(sub_exercise->sets);
	delete sub_exercise;
	exercise->m_exercises.remove(exercise_idx);
	emit workoutChanged();
}

bool DBWorkoutModel::exerciseIsComposite(const uint exercise_number) const
{
	if (exercise_number < m_exerciseData.count())
		return m_exerciseData.at(exercise_number)->m_exercises.count() >= 2;
	else
		return false;
}

const uint DBWorkoutModel::setsNumber(const uint exercise_number) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(0)->sets.count();
}

void DBWorkoutModel::newSet(const uint exercise_number, const uint type, const uint reps, const uint weight,
							const QTime &rest_time, const uint sub_sets)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : exercise->m_exercises)
	{
		st_set *new_set{new st_set};
		new_set->number = sub_exercise->sets.count();
		new_set->type = static_cast<TPSetTypes>(type);
		new_set->restTime = rest_time;
		new_set->subsets = sub_sets;
		new_set->reps = reps;
		new_set->weight = weight;
		sub_exercise->sets.append(std::move(new_set));
	}
	emit setsNumberChanged(exercise_number);
}

void DBWorkoutModel::removeSet(const uint exercise_number, const uint set_number)
{
	exerciseEntry *exercise{m_exerciseData[exercise_number]};
	for (const auto sub_exercise : exercise->m_exercises)
	{
		delete sub_exercise->sets.at(set_number);
		sub_exercise->sets.remove(set_number);
		for (uint i{set_number}; i < sub_exercise->sets.count(); ++i)
			sub_exercise->sets.at(i)->number = i;
	}

	emit exerciseCompleted(exercise_number, allSetsCompleted(exercise_number));
	emit workoutChanged();
}

bool DBWorkoutModel::trackRestTime(const uint exercise_number, const uint exercise_idx) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->mb_TrackRestTime;
}

void DBWorkoutModel::setTrackRestTime(const uint exercise_number, const uint exercise_idx, const bool track_resttime)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->mb_TrackRestTime = track_resttime;
}

bool DBWorkoutModel::autoRestTime(const uint exercise_number, const uint exercise_idx) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->mb_AutoRestTime;
}

void DBWorkoutModel::setAutoRestTime(const uint exercise_number, const uint exercise_idx, const bool auto_resttime)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->mb_AutoRestTime = auto_resttime;
}

QString DBWorkoutModel::exerciseName(const uint exercise_number, const uint exercise_idx) const
{
	return m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name;
}

void DBWorkoutModel::setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name)
{
	m_exerciseData.at(exercise_number)->m_exercises.at(exercise_idx)->name = new_name;
}

uint DBWorkoutModel::setType(const uint exercise_number, const uint set_number) const
{
	if (set_number < m_exerciseData.at(exercise_number)->nsets)
		return m_exerciseData.at(exercise_number)->type.at(set_number).toUInt();
	return SET_TYPE_REGULAR;
}

void DBWorkoutModel::setSetType(const uint exercise_number, const uint set_number, const uint new_type)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setType()", "out of range set_number");
	m_exerciseData.at(exercise_number)->type[set_number] = QString::number(new_type);
}

void DBWorkoutModel::changeSetType(const uint exercise_number, const uint set_number, const uint old_type, const uint new_type)
{
	const QString& reps(m_exerciseData.at(exercise_number)->reps.at(set_number));
	const QString& weight(m_exerciseData.at(exercise_number)->weight.at(set_number));
	switch (old_type)
	{
		case SET_TYPE_REGULAR:
		case SET_TYPE_PYRAMID:
		case SET_TYPE_CLUSTER:
		case SET_TYPE_MYOREPS:
		case SET_TYPE_REVERSE_PYRAMID:
			if (new_type == SET_TYPE_GIANT)
			{
				m_exerciseData.at(exercise_number)->reps[set_number].append(comp_exercise_separator + reps + comp_exercise_separator);
				m_exerciseData.at(exercise_number)->weight[set_number].append(comp_exercise_separator + weight + comp_exercise_separator);
			}
			else if (new_type == SET_TYPE_DROP)
			{
				m_exerciseData.at(exercise_number)->reps[set_number].append(dropSetReps(reps));
				m_exerciseData.at(exercise_number)->weight[set_number].append(dropSetWeight(weight));
				m_exerciseData.at(exercise_number)->subsets[set_number] = "3"_L1;
			}
			else if (new_type == SET_TYPE_CLUSTER)
				m_exerciseData.at(exercise_number)->subsets[set_number]= "4"_L1;
		break;

		case SET_TYPE_GIANT:
		case SET_TYPE_DROP:
			if (new_type == SET_TYPE_DROP)
			{
				m_exerciseData.at(exercise_number)->subsets[set_number] = "3"_L1;
				const QString& new_reps(appUtils()->getCompositeValue(0, reps, comp_exercise_separator));
				m_exerciseData.at(exercise_number)->reps[set_number] = new_reps + dropSetReps(new_reps);
				const QString& new_weight(appUtils()->getCompositeValue(0, weight, comp_exercise_separator));
				m_exerciseData.at(exercise_number)->weight[set_number] = new_weight + dropSetWeight(new_weight);
			}
			else if (new_type == SET_TYPE_GIANT)
			{
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, reps, comp_exercise_separator), m_exerciseData.at(exercise_number)->reps[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, reps, comp_exercise_separator), m_exerciseData.at(exercise_number)->reps[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, weight, comp_exercise_separator), m_exerciseData.at(exercise_number)->weight[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, weight, comp_exercise_separator), m_exerciseData.at(exercise_number)->weight[set_number], comp_exercise_separator);
			}
			else
			{
				m_exerciseData.at(exercise_number)->reps[set_number] = appUtils()->getCompositeValue(0, reps, comp_exercise_separator);
				m_exerciseData.at(exercise_number)->weight[set_number] = appUtils()->getCompositeValue(0, weight, comp_exercise_separator);
				if (new_type == SET_TYPE_CLUSTER)
					m_exerciseData.at(exercise_number)->subsets[set_number]= "4"_L1;
			}
		break;
	}
	setSetType(exercise_number, set_number, new_type);
}

QString DBWorkoutModel::setRestTime(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setRestTime()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->resttime.at(set_number);
}

void DBWorkoutModel::setSetRestTime(const uint exercise_number, const uint set_number, const QString& new_time)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSetRestTime()", "out of range set_number");
	m_exerciseData.at(exercise_number)->resttime[set_number] = new_time;
}

QString DBWorkoutModel::setSubSets(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSubSets()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->subsets.at(set_number);
}

void DBWorkoutModel::newSetSubSet(const uint exercise_number, const uint set_number)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::newSetSubSet()", "out of range set_number");
	m_exerciseData.at(exercise_number)->subsets[set_number] = QString::number(m_exerciseData.at(exercise_number)->subsets.at(set_number).toUInt() + 1);
	m_exerciseData.at(exercise_number)->reps[set_number].append(QString::number(
		m_exerciseData.at(exercise_number)->reps.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).constLast().toUInt() - 2) + comp_exercise_separator);
	m_exerciseData.at(exercise_number)->weight[set_number].append(QString::number(
		m_exerciseData.at(exercise_number)->weight.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).constLast().toUInt() - 10) + comp_exercise_separator);
}

void DBWorkoutModel::setSetSubSets(const uint exercise_number, const uint set_number, const QString& new_subsets)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->subsets.count(), "DBWorkoutModel::setType()", "out of range set_number");
	m_exerciseData.at(exercise_number)->subsets[set_number] = new_subsets;
}

QString DBWorkoutModel::setReps(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setReps()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->reps.at(set_number);
}

void DBWorkoutModel::setSetReps(const uint exercise_number, const uint set_number, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSetReps()", "out of range set_number");
	m_exerciseData.at(exercise_number)->reps[set_number] = new_reps;
}

QString DBWorkoutModel::setWeight(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setWeight()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->weight.at(set_number);
}

void DBWorkoutModel::setSetWeight(const uint exercise_number, const uint set_number, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSetWeight()", "out of range set_number");
	m_exerciseData.at(exercise_number)->weight[set_number] = new_weight;
}

QString DBWorkoutModel::setNotes(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->notes.count(), "DBWorkoutModel::setNotes()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->notes.at(set_number);
}

void DBWorkoutModel::setSetNotes(const uint exercise_number, const uint set_number, const QString& new_notes)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->notes.count(), "DBWorkoutModel::setSetNotes()", "out of range set_number");
	m_exerciseData.at(exercise_number)->notes[set_number] = new_notes;
}

bool DBWorkoutModel::setCompleted(const uint exercise_number, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->completed.count(), "DBWorkoutModel::setCompleted()", "out of range set_number");
	return m_exerciseData.at(exercise_number)->completed.at(set_number) == STR_ONE;
}

void DBWorkoutModel::setSetCompleted(const uint exercise_number, const uint set_number, const bool completed)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->completed.count(), "DBWorkoutModel::setSetCompleted()", "out of range set_number");
	m_exerciseData.at(exercise_number)->completed[set_number] = completed ? STR_ONE : STR_ZERO;
	emit exerciseCompleted(exercise_number, allSetsCompleted(exercise_number));
	if (completed)
		emit workoutChanged();
}

bool DBWorkoutModel::allSetsCompleted(const uint exercise_number) const
{
	const uint nsets(m_exerciseData.at(exercise_number)->nsets);
	if (nsets > 0)
	{
		for (uint i(0); i < nsets; ++i)
		{
			if (m_exerciseData.at(exercise_number)->completed.at(i) == STR_ZERO)
				return false;
		}
		return true;
	}
	return false;
}

bool DBWorkoutModel::anySetCompleted(const uint exercise_number) const
{
	const uint nsets(m_exerciseData.at(exercise_number)->nsets);
	if (nsets > 0)
	{
		for (uint i(0); i < nsets; ++i)
		{
			if (m_exerciseData.at(exercise_number)->completed.at(i) == STR_ONE)
				return true;
		}
		return false;
	}
	return true;
}

QString DBWorkoutModel::setReps(const uint set_number, const uint subset, const uint exercise_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setReps()", "out of range set_number");
	const QStringList& subSetReps(m_exerciseData.at(exercise_number)->reps.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts));
	return subset < subSetReps.count() ? subSetReps.at(subset) : QString();
}

void DBWorkoutModel::setSetReps(const uint exercise_number, const uint set_number, const uint subset, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSetReps()", "out of range set_number");
	QStringList subSetReps(std::move(m_exerciseData.at(exercise_number)->reps.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts)));
	const uint total(subSetReps.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetReps.append(new_reps);
	}
	else
		subSetReps[subset] = new_reps;
	m_exerciseData.at(exercise_number)->reps[set_number] = std::move(subSetReps.join(comp_exercise_separator) + comp_exercise_separator);
}

QString DBWorkoutModel::setWeight(const uint set_number, const uint subset, const uint exercise_number) const
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setWeight()", "out of range set_number");
	const QStringList& subSetWeight(m_exerciseData.at(exercise_number)->weight.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts));
	return subset < subSetWeight.count() ? subSetWeight.at(subset) : QString();
}

void DBWorkoutModel::setSetWeight(const uint exercise_number, const uint set_number, const uint subset, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_exerciseData.at(exercise_number)->nsets, "DBWorkoutModel::setSetWeight()", "out of range set_number");
	QStringList subSetWeight(std::move(m_exerciseData.at(exercise_number)->weight.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts)));
	const uint total(subSetWeight.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetWeight.append(new_weight);
	}
	else
		subSetWeight[subset] = new_weight;
	m_exerciseData.at(exercise_number)->weight[set_number] = std::move(subSetWeight.join(comp_exercise_separator) + comp_exercise_separator);
}
