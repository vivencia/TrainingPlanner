#include "dbtrainingdaymodel.h"
#include "tpglobals.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "tputils.h"

#include <QtMath>

DBTrainingDayModel::DBTrainingDayModel(QObject* parent, const int meso_idx)
	: TPListModel(parent, meso_idx), mb_DayIsFinished(false), mb_DayIsEditable(false)
{
	setObjectName(DBTrainingDayObjectName);
	m_tableId = TRAININGDAY_TABLE_ID;
	m_fieldCount = TDAY_TOTAL_COLS;
	m_exportName = tr("Single workout");
}

void DBTrainingDayModel::fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse)
{
	const QStringList& exercises_names(list.at(TDAY_EXERCISES_COL_NAMES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& setstypes(list.at(TDAY_EXERCISES_COL_TYPES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& resttimes(list.at(TDAY_EXERCISES_COL_RESTTIMES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& subsets(list.at(TDAY_EXERCISES_COL_SUBSETS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& reps(list.at(TDAY_EXERCISES_COL_REPS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& weights(list.at(TDAY_EXERCISES_COL_WEIGHTS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList& notes(list.at(TDAY_EXERCISES_COL_NOTES).split(record_separator2, Qt::SkipEmptyParts));

	QStringList completed;
	if (!bClearSomeFieldsForReUse)
		completed = list.at(TDAY_EXERCISES_COL_COMPLETED).split(record_separator2, Qt::SkipEmptyParts);
	else //reset for new use
	{
		QString temp(list.at(TDAY_EXERCISES_COL_COMPLETED));
		completed = temp.replace(STR_ONE, STR_ZERO).split(record_separator2, Qt::SkipEmptyParts);
	}

	for(uint i(0); i < exercises_names.count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		m_ExerciseData[i]->name = exercises_names.at(i);
		m_ExerciseData[i]->type = setstypes.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->resttime = resttimes.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->subsets = subsets.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->reps = reps.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->weight = weights.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->notes = notes.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->completed = completed.at(i).split(record_separator, Qt::SkipEmptyParts);
		m_ExerciseData[i]->nsets = m_ExerciseData.at(i)->type.count();
	}
	setModified(true);
	emit exerciseCountChanged();
}

const QStringList DBTrainingDayModel::getSaveInfo() const
{
	QStringList data(TDAY_EXERCISES_TOTALCOLS);
	for(uint i(0); i < m_ExerciseData.count(); ++i)
	{
		data[TDAY_EXERCISES_COL_NAMES].append(m_ExerciseData.at(i)->name + record_separator2);
		for(uint x(0); x < m_ExerciseData.at(i)->nsets; ++x)
		{
			data[TDAY_EXERCISES_COL_TYPES].append(m_ExerciseData.at(i)->type.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_RESTTIMES].append(m_ExerciseData.at(i)->resttime.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_SUBSETS].append((x < m_ExerciseData.at(i)->subsets.count() ? m_ExerciseData.at(i)->subsets.at(x) : u"0"_qs) + record_separator);
			data[TDAY_EXERCISES_COL_REPS].append(m_ExerciseData.at(i)->reps.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_WEIGHTS].append(m_ExerciseData.at(i)->weight.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_NOTES].append((x < m_ExerciseData.at(i)->notes.count() ? m_ExerciseData.at(i)->notes.at(x) : u" "_qs) + record_separator);
			data[TDAY_EXERCISES_COL_COMPLETED].append(m_ExerciseData.at(i)->completed.at(x) + record_separator);
		}
		data[TDAY_EXERCISES_COL_TYPES].append(record_separator2);
		data[TDAY_EXERCISES_COL_RESTTIMES].append(record_separator2);
		data[TDAY_EXERCISES_COL_SUBSETS].append(record_separator2);
		data[TDAY_EXERCISES_COL_REPS].append(record_separator2);
		data[TDAY_EXERCISES_COL_WEIGHTS].append(record_separator2);
		data[TDAY_EXERCISES_COL_NOTES].append(record_separator2);
		data[TDAY_EXERCISES_COL_COMPLETED].append(record_separator2);
	}
	return data;
}

void DBTrainingDayModel::convertMesoSplitModelToTDayModel(DBMesoSplitModel* const splitModel)
{
	uint nsets(0);
	uint orig_workingset(0); //If the split is being viewed on MesoSplitPlanner.qml, do not disturb the view by changing the current viewed set
	for(uint i(0); i < splitModel->count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		m_ExerciseData[i]->name = splitModel->exerciseName(i);
		m_ExerciseData[i]->name.replace(u" + "_qs, QString(subrecord_separator));
		nsets = splitModel->setsNumber(i);
		orig_workingset = splitModel->workingSet(i);
		splitModel->setWorkingSet(i, 0, false);
		newFirstSet(i, splitModel->setType(i), splitModel->setsReps(i), splitModel->setsWeight(i),
								nextSetSuggestedTime(i, splitModel->setType(i), 0), splitModel->setsSubsets(i), splitModel->setsNotes(i));
		for(uint x(1); x < nsets; ++x)
		{
			splitModel->setWorkingSet(i, x, false);
			newSet(splitModel->setsNumber(i) - 1, i, splitModel->setType(i), splitModel->setsReps(i), splitModel->setsWeight(i),
				nextSetSuggestedTime(i, splitModel->setType(i)), splitModel->setsSubsets(i));
		}
		splitModel->setWorkingSet(i, orig_workingset, false);
	}
	setModified(true);
	emit exerciseCountChanged();
}

int DBTrainingDayModel::exportToFile(const QString& filename, const bool, const bool) const
{
	if (exerciseCount() == 0)
		return APPWINDOW_MSG_NOTHING_TO_EXPORT;

	QFile* outFile{new QFile(filename)};
	const bool bOK(outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text));
	if (bOK)
	{
		const QString& strHeader(u"## "_qs + exportName() + u"\n\n"_qs);
		outFile->write(strHeader.toUtf8().constData());
		outFile->write(exportExtraInfo().toUtf8().constData());
		outFile->write("\n\n", 2);

		uint settype(0);
		QString setsTypes, subSets;
		bool bHasSubsSets(false);
		for (uint i(0); i < m_ExerciseData.count(); ++i)
		{
			outFile->write(QString(QString::number(i+1) + ": ").toUtf8().constData());
			outFile->write(exerciseName(i).toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Number of sets: ").toUtf8().constData());
			outFile->write(QString::number(setsNumber(i)).toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Rest time between sets: ").toUtf8().constData());
			outFile->write(setRestTime(0, i).toUtf8().constData());
			outFile->write("\n", 1);

			for (uint n(0); n < setsNumber(i); ++n)
			{
				settype = setType(n, i);
				setsTypes += formatSetTypeToExport(QString::number(settype)) + '|';
				subSets += setSubSets(n, i) + '|';
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
			outFile->write(setReps(0, 1).replace(subrecord_separator, '|').toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Initial weight: ").toUtf8().constData());
			outFile->write(setWeight(0, 1).replace(subrecord_separator, '|').toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Note for the sets: ").toUtf8().constData());
			outFile->write(setNotes(0, i).toUtf8().constData());
			outFile->write("\n\n", 2);
		}
		outFile->write(STR_END_EXPORT.toUtf8().constData());
		outFile->close();
	}
	delete outFile;
	return bOK ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;
}

int DBTrainingDayModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	uint col(1);
	QString value;
	uint exerciseNumber(0), nsets(0), types(0);
	QString type, resttime, subsets, reps, weight, notes, strTypes;

	char buf[128];
	qint64 lineLength(0);
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, ":") != NULL) //dont't put a colon in exportExtraInfo()
				{
					value = buf;
					newExercise(value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator), exerciseNumber);

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					nsets = value.remove(0, value.indexOf(':') + 2).trimmed().toUInt();

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					resttime = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					strTypes = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);

					types = 0;
					do {
						type = formatSetTypeToImport(appUtils()->getCompositeValue(types, strTypes)) + subrecord_separator;
					} while (++types < nsets);

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					if (value.indexOf(tr("subsets")) == -1)
					{
						subsets = u"0"_qs;
						reps = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);
					}
					else
					{
						subsets = value.remove(0, value.indexOf(':') + 2).trimmed();
						if (inFile->readLine(buf, sizeof(buf)) == -1)
							return false;
						reps = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);
					}

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					weight = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);

					if (inFile->readLine(buf, sizeof(buf)) == -1)
						return false;
					value = buf;
					notes = value.remove(0, value.indexOf(':') + 2).trimmed().replace('|', subrecord_separator);
					if (notes.isEmpty())
						notes = u" "_qs;

					newFirstSet(exerciseNumber, type.toUInt(), reps, weight, resttime, subsets, notes);
					for (uint i(1); i < nsets; ++i)
						newSet(i, exerciseNumber, type.toUInt());
				}
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	return exerciseCount() > 0 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBTrainingDayModel::updateFromModel(const TPListModel* const model)
{
	const DBTrainingDayModel* const tDayModel(static_cast<const DBTrainingDayModel* const>(const_cast<TPListModel*>(model)));
	for (uint i(0); i < tDayModel->exerciseCount(); ++i)
	{
		newExercise(tDayModel->exerciseName(i) , i);
		newFirstSet(i, tDayModel->setType(0, i), tDayModel->setReps(0, i), tDayModel->setWeight(0, i), tDayModel->setRestTime(0, i),
					tDayModel->setSubSets(0, 1), tDayModel->setNotes(0, i));
		for (uint x(1); x < tDayModel->setsNumber(i); ++x)
		{
			newSet(x, i, tDayModel->setType(x, i), tDayModel->setReps(x, i), tDayModel->setWeight(x, i), tDayModel->setRestTime(x, i),
					tDayModel->setSubSets(x, 1));
		}
	}
	if (exerciseCount() > 0)
	{
		setImportMode(true);
		setModified(true);
		emit exerciseCountChanged();
		return true;
	}
	return false;
}

//Don't put any colon ':' in here. Import will fail
const QString DBTrainingDayModel::exportExtraInfo() const
{
	return tr("Workout #") + m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER) + tr(", split ") + m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER) +
		u" ("_qs + appMesoModel()->mesoSplitModel()->splitX(m_mesoIdx, appUtils()->splitLetterToMesoSplitIndex(m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER))) +
		tr(") at ") + appUtils()->formatDate(date());
}

const QString DBTrainingDayModel::formatSetTypeToExport(const QString& fieldValue) const
{
	QString ret{fieldValue};
	ret.replace(u"0"_qs, tr("Regular"));
	ret.replace(u"1"_qs, tr("Pyramid"));
	ret.replace(u"2"_qs, tr("Drop Set"));
	ret.replace(u"3"_qs, tr("Cluster Set"));
	ret.replace(u"4"_qs, tr("Giant Set"));
	ret.replace(u"5"_qs, tr("Myo Reps"));
	ret.replace(u"6"_qs, tr("Inverted Pyramid"));
	return ret;
}

const QString DBTrainingDayModel::formatSetTypeToImport(const QString& fieldValue) const
{
	QString ret;
	if (!fieldValue.isEmpty())
	{
		ret = fieldValue;
		ret.replace(tr("Regular"), u"0"_qs);
		ret.replace(tr("Pyramid"), u"1"_qs);
		ret.replace(tr("Drop Set"), u"2"_qs);
		ret.replace(tr("Cluster Set"), u"3"_qs);
		ret.replace(tr("Giant Set"), u"4"_qs);
		ret.replace(tr("Myo Reps"), u"5"_qs);
		ret.replace(tr("Inverted Pyramid"), u"6"_qs);
	}
	else
		ret = u"0"_qs;
	return ret;
}

void DBTrainingDayModel::setDayIsFinished(const bool finished)
{
	mb_DayIsFinished = finished;
	setModified(true);
	emit dayIsFinishedChanged();
	emit saveWorkout();
}

void DBTrainingDayModel::moveExercise(const uint from, const uint to)
{
	if (from < m_ExerciseData.count() && to < m_ExerciseData.count())
	{
		exerciseEntry* tempExerciseData(m_ExerciseData.at(from));

		if (to > from)
		{
			for(uint i(from); i < to; ++i)
				m_ExerciseData[i] = m_ExerciseData.at(i+1);
		}
		else
		{
			for(uint i(from); i > to; --i)
				m_ExerciseData[i] = m_ExerciseData.at(i-1);
		}
		m_ExerciseData[to] = tempExerciseData;
		setModified(true);
	}
}

uint DBTrainingDayModel::getWorkoutNumberForTrainingDay() const
{
	return appMesoModel()->mesoCalendarModel(mesoIdx())->getLastTrainingDayBeforeDate(date()) + 1;
}

QString DBTrainingDayModel::exerciseName(const uint exercise_idx) const
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::exerciseName", "out of range exercise_idx");
	QString name;
	name = m_ExerciseData.at(exercise_idx)->name;
	if (name.endsWith(subrecord_separator))
		name.chop(1);
	return name.replace(subrecord_separator, QStringLiteral(" + "));
}

void DBTrainingDayModel::setExerciseName(const QString& new_name, const uint exercise_idx)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::setExerciseName", "out of range exercise_idx");
	const int idx(new_name.indexOf('+'));
	if (idx == -1)
		m_ExerciseData.at(exercise_idx)->name = new_name;
	else
	{
		QString new_name_copy(new_name);
		m_ExerciseData.at(exercise_idx)->name = new_name_copy.replace(u" + "_qs, QChar(subrecord_separator));
	}
	m_CompositeExerciseList[exercise_idx] = idx != -1;
	emit compositeExerciseChanged(exercise_idx);
	setModified(true);
}

void DBTrainingDayModel::newExercise(const QString& new_exercise, const uint idx)
{
	const uint total(m_ExerciseData.count());
	const int n(idx - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			m_ExerciseData.append(new exerciseEntry);
	}
	m_ExerciseData[idx]->name = new_exercise;
	m_CompositeExerciseList[idx] = new_exercise.contains(subrecord_separator);
	emit compositeExerciseChanged(idx);
	setModified(true);
	emit exerciseCountChanged();
}

void DBTrainingDayModel::removeExercise(const uint exercise_idx)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::removeExercise", "out of range exercise_idx");
	delete m_ExerciseData.at(exercise_idx);
	m_ExerciseData.remove(exercise_idx);
	setModified(true);
	emit exerciseCountChanged();
}

void DBTrainingDayModel::changeExerciseName(const uint exercise_idx, DBExercisesModel* model)
{
	QString name;
	const uint nSel(model->selectedEntriesCount());

	if (nSel == 1)
		name = model->selectedEntriesValue_fast(0, 1) + u" - "_qs + model->selectedEntriesValue_fast(0, 2);
	else
	{
		for (uint i(0); i < nSel; ++i)
			name += model->selectedEntriesValue_fast(i, 1) + u" - "_qs + model->selectedEntriesValue_fast(i, 2) + subrecord_separator;
		name.chop(1);
	}
	setExerciseName(name, exercise_idx);
}

QString DBTrainingDayModel::exerciseName1(const uint exercise_idx) const
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::exerciseName1", "out of range exercise_idx");
	const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
	return idx != -1 ? u"1: "_qs + m_ExerciseData.at(exercise_idx)->name.left(idx) : m_ExerciseData.at(exercise_idx)->name;
}

void DBTrainingDayModel::setExerciseName1(const QString& name1, const uint exercise_idx)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::setExerciseName1", "out of range exercise_idx");
	const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
	QString new_name1;
	if (idx != -1)
		new_name1 = name1 + subrecord_separator + m_ExerciseData.at(exercise_idx)->name.sliced(idx+1);
	else
		new_name1 = name1;
	m_ExerciseData.at(exercise_idx)->name = new_name1.replace(u"1: "_qs, "");
	m_CompositeExerciseList[exercise_idx] = idx != -1;
	emit compositeExerciseChanged(exercise_idx);
	setModified(true);
}

QString DBTrainingDayModel::exerciseName2(const uint exercise_idx) const
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::exerciseName2", "out of range exercise_idx");
	const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
	return idx != -1 ? QStringLiteral("2: ") + m_ExerciseData.at(exercise_idx)->name.sliced(idx+1) : tr("2: Add exercise ...");
}

void DBTrainingDayModel::setExerciseName2(const QString& name2, const uint exercise_idx)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::setExerciseName2", "out of range exercise_idx");		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
	QString new_name2;
	if (idx != -1)
		new_name2 = m_ExerciseData.at(exercise_idx)->name.left(idx) + subrecord_separator + name2;
	else
		new_name2 = subrecord_separator + name2;
	m_CompositeExerciseList[exercise_idx] = idx != -1;
	emit compositeExerciseChanged(exercise_idx);
	m_ExerciseData.at(exercise_idx)->name = new_name2.replace(u"2: "_qs, "");
	setModified(true);
}

static QString increaseStringTimeBy(const QString& strtime, const uint add_mins, const uint add_secs)
{
	uint secs(QStringView{strtime}.mid(3, 2).toUInt());
	uint mins(QStringView{strtime}.left(2).toUInt());

	secs += add_secs;
	if (secs > 59)
	{
		secs -= 60;
		mins++;
	}
	mins += add_mins;
	const QString& ret((mins <= 9 ? STR_ZERO + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? STR_ZERO + QString::number(secs) : QString::number(secs)));
	return ret;
}

static inline QString dropSetReps(const QString& reps)
{
	const float value(appUtils()->appLocale()->toFloat(reps));
	QString value1(appUtils()->appLocale()->toString(qCeil(value * 0.8)));
	if (value1.contains('.') || value1.contains(','))
	{
		if (value1.right(2) != u"50"_qs)
			value1.chop(3); //nn
		else
			value1.chop(1); // nn,5 or nn.5
	}
	QString value2(appUtils()->appLocale()->toString(qCeil(value * 0.8 * 0.8)));
	if (value2.contains('.') || value2.contains(','))
	{
		if (value2.right(2) != u"50"_qs)
			value2.chop(3); //nn
		else
			value2.chop(1); // nn,5 or nn.5
	}
	return subrecord_separator + value1 + subrecord_separator + value2 + subrecord_separator;
}

static inline QString dropSetWeight(const QString& weight)
{
	const float value(appUtils()->appLocale()->toFloat(weight));
	QString value1(appUtils()->appLocale()->toString(value * 0.5, 'f', 2));
	if (value1.contains('.') || value1.contains(','))
	{
		if (value1.right(2) != u"50"_qs)
			value1.chop(3); //nn
		else
			value1.chop(1); // nn,5 or nn.5
	}
	QString value2(appUtils()->appLocale()->toString(value * 0.5 * 0.5, 'f', 2));
	if (value2.contains('.') || value2.contains(','))
	{
		if (value2.right(2) != u"50"_qs)
			value2.chop(3); //nn
		else
			value2.chop(1); // nn,5 or nn.5
	}
	return subrecord_separator + value1 + subrecord_separator + value2 + subrecord_separator;
}

void DBTrainingDayModel::newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight,
										const QString& nRestTime, const QString& nSubsets, const QString& notes)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::newFirstSet()", "out of range exercise_idx");
	const QString strType(QString::number(type));
	m_ExerciseData.at(exercise_idx)->nsets = 1;
	setModified(true);
	m_ExerciseData.at(exercise_idx)->type.append(strType);
	m_ExerciseData.at(exercise_idx)->notes.append(notes);
	m_ExerciseData.at(exercise_idx)->completed.append(u"0"_qs);
	m_ExerciseData.at(exercise_idx)->resttime.append(nRestTime);

	switch (type)
	{
		case SET_TYPE_REGULAR:
		case SET_TYPE_PYRAMID:
		case SET_TYPE_REVERSE_PYRAMID:
			m_ExerciseData.at(exercise_idx)->reps.append(nReps);
			m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
			m_ExerciseData.at(exercise_idx)->subsets.append(u"0"_qs);
		break;
		case SET_TYPE_DROP:
		{
			m_ExerciseData.at(exercise_idx)->reps.append(nReps + dropSetReps(nReps));
			m_ExerciseData.at(exercise_idx)->weight.append(nWeight + dropSetWeight(nWeight));
			m_ExerciseData.at(exercise_idx)->subsets.append(u"3"_qs);
		}
		break;
		case SET_TYPE_CLUSTER:
			m_ExerciseData.at(exercise_idx)->reps.append(nReps);
			m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
			m_ExerciseData.at(exercise_idx)->subsets.append(nSubsets);
		break;
		case SET_TYPE_GIANT:
			if (nReps.indexOf(subrecord_separator) == -1)
				m_ExerciseData.at(exercise_idx)->reps.append(nReps + subrecord_separator + nReps + subrecord_separator);
			else
				m_ExerciseData.at(exercise_idx)->reps.append(nReps + subrecord_separator);
			if (nWeight.indexOf(subrecord_separator) == -1)
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight + subrecord_separator + nWeight + subrecord_separator);
			else
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight + subrecord_separator);
			m_ExerciseData.at(exercise_idx)->subsets.append(u"0"_qs);
		break;
		case SET_TYPE_MYOREPS:
			m_ExerciseData.at(exercise_idx)->reps.append(nReps);
			m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
			m_ExerciseData.at(exercise_idx)->subsets.append(nSubsets);
		break;
	}
}

QString DBTrainingDayModel::nextSetSuggestedTime(const uint exercise_idx, const uint type, const uint set_number) const
{
	QString strSetTime;
	if (set_number == 0)
		return type != SET_TYPE_MYOREPS ?  u"01:30"_qs : u"02:30"_qs;
	else
	{
		if (!m_ExerciseData.at(exercise_idx)->mb_TrackRestTime)
			return m_ExerciseData.at(exercise_idx)->resttime.at(0);
		if (m_ExerciseData.at(exercise_idx)->mb_AutoRestTime)
			return u"00:00"_qs;
		strSetTime = set_number == 100 ?
			m_ExerciseData.at(exercise_idx)->resttime.constLast() :
			m_ExerciseData.at(exercise_idx)->resttime.at(set_number);
	}

	switch (type)
	{
		case SET_TYPE_REGULAR:
		case SET_TYPE_PYRAMID:
		case SET_TYPE_DROP:
		case SET_TYPE_GIANT:
		case SET_TYPE_REVERSE_PYRAMID:
			strSetTime = increaseStringTimeBy(strSetTime, 0, 30);
		break;
		case SET_TYPE_CLUSTER:
			strSetTime = increaseStringTimeBy(strSetTime , 1, 0);
		break;
		case SET_TYPE_MYOREPS:
			strSetTime = increaseStringTimeBy(strSetTime, 1, 30);
		break;
	}
	return strSetTime;
}

const QString DBTrainingDayModel::nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number, const uint sub_set) const
{
	QString strSetReps;
	if (set_number == 100)
	{
		strSetReps = sub_set == 100 ? m_ExerciseData.at(exercise_idx)->reps.constLast() :
							m_ExerciseData.at(exercise_idx)->reps.constLast().split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set);
	}
	else
	{
		const QString& reps(m_ExerciseData.at(exercise_idx)->reps.at(set_number));
		strSetReps = sub_set == 100 ? reps : reps.contains(subrecord_separator) ?
													reps.split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set) :
													reps;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue(appUtils()->appLocale()->toFloat(strSetReps));
		if (type == SET_TYPE_PYRAMID)
			lastSetValue = qCeil(lastSetValue * 0.8);
		else
			lastSetValue = qCeil(lastSetValue * 1.25);
		strSetReps = appUtils()->appLocale()->toString(static_cast<int>(lastSetValue));
		if (strSetReps.contains('.') || strSetReps.contains(','))
		{
			if (strSetReps.right(2) != u"50"_qs)
				strSetReps.chop(3); //nn
			else
				strSetReps.chop(1); // nn,5 or nn.5
		}
	}
	return strSetReps;
}

const QString DBTrainingDayModel::nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number, const uint sub_set) const
{
	QString strStrWeight;
	if (set_number == 100)
	{
		strStrWeight = sub_set == 100 ? m_ExerciseData.at(exercise_idx)->weight.constLast() :
							m_ExerciseData.at(exercise_idx)->weight.constLast().split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set);
	}
	else
	{
		const QString& weight(m_ExerciseData.at(exercise_idx)->weight.at(set_number));
		strStrWeight = sub_set == 100 ? weight : weight.contains(subrecord_separator) ?
													weight.split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set) :
													weight;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue(appUtils()->appLocale()->toFloat(strStrWeight));
		if (type == SET_TYPE_PYRAMID)
			lastSetValue *= 1.2;
		else
			lastSetValue *= 0.8;
		strStrWeight = appUtils()->appLocale()->toString(lastSetValue, 'f', 2);
		if (strStrWeight.contains('.') || strStrWeight.contains(','))
		{
			if (strStrWeight.right(2) != u"50"_qs)
				strStrWeight.chop(3);
			else
				strStrWeight.chop(1);
		}
	}
	return strStrWeight;
}

void DBTrainingDayModel::newSet(const uint set_number, const uint exercise_idx, const uint type,
							const QString& nReps, const QString& nWeight, const QString& nRestTime, const QString& nSubSets)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::newSet()", "out of range exercise_idx");
	const uint total(m_ExerciseData.at(exercise_idx)->nsets);
	const int n(set_number - total + 1);
	const QString& strType(QString::number(type));
	uint n_exercises(0);
	if (n >= 1)
	{
		m_ExerciseData.at(exercise_idx)->nsets += n;
		setModified(true);

		for(uint i(0); i < n; ++i)
		{
			m_ExerciseData.at(exercise_idx)->type.append(strType);
			m_ExerciseData.at(exercise_idx)->resttime.append(nRestTime.isEmpty() ? nextSetSuggestedTime(exercise_idx, type) : nRestTime);
			m_ExerciseData.at(exercise_idx)->reps.append(nReps.isEmpty() ? nextSetSuggestedReps(exercise_idx, type) : nReps);
			m_ExerciseData.at(exercise_idx)->weight.append(nWeight.isEmpty() ? nextSetSuggestedWeight(exercise_idx, type) : nWeight);
			m_ExerciseData.at(exercise_idx)->notes.append(m_ExerciseData.at(exercise_idx)->notes.constLast());
			m_ExerciseData.at(exercise_idx)->completed.append(u"0"_qs);
			m_ExerciseData.at(exercise_idx)->subsets.append(nSubSets.isEmpty() ?
					(type != SET_TYPE_MYOREPS ?
						m_ExerciseData.at(exercise_idx)->subsets.constLast() :
						QString::number(m_ExerciseData.at(exercise_idx)->subsets.constLast().toInt() + 1))
					 : nSubSets);

			n_exercises = m_ExerciseData.at(exercise_idx)->type.count();
			if (n_exercises > 1)
			{
				if (strType != m_ExerciseData.at(exercise_idx)->type.at(n_exercises - 2))
					changeSetType (set_number, exercise_idx, m_ExerciseData.at(exercise_idx)->type.at(n_exercises - 2).toUInt(), type);
			}
		}
	}
}

void DBTrainingDayModel::removeSet(const uint set_number, const uint exercise_idx)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::removeSet()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->type.remove(set_number);
	m_ExerciseData.at(exercise_idx)->resttime.remove(set_number);
	m_ExerciseData.at(exercise_idx)->subsets.remove(set_number);
	m_ExerciseData.at(exercise_idx)->reps.remove(set_number);
	m_ExerciseData.at(exercise_idx)->weight.remove(set_number);
	m_ExerciseData.at(exercise_idx)->notes.remove(set_number);
	m_ExerciseData.at(exercise_idx)->nsets--;
	setModified(true);
	emit exerciseCompleted(exercise_idx, allSetsCompleted(exercise_idx));
	emit saveWorkout();
}

uint DBTrainingDayModel::setType(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setType()", "out of range set_number");
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
		return m_ExerciseData.at(exercise_idx)->type.at(set_number).toUInt();
	return SET_TYPE_REGULAR;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint exercise_idx, const uint new_type)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setType()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->type[set_number] = QString::number(new_type);
	setModified(true);
}

void DBTrainingDayModel::changeSetType(const uint set_number, const uint exercise_idx, const uint old_type, const uint new_type)
{
	const QString& reps(m_ExerciseData.at(exercise_idx)->reps.at(set_number));
	const QString& weight(m_ExerciseData.at(exercise_idx)->weight.at(set_number));
	switch (old_type)
	{
		case SET_TYPE_REGULAR:
		case SET_TYPE_PYRAMID:
		case SET_TYPE_CLUSTER:
		case SET_TYPE_MYOREPS:
		case SET_TYPE_REVERSE_PYRAMID:
			if (new_type == SET_TYPE_GIANT)
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number].append(subrecord_separator + reps + subrecord_separator);
				m_ExerciseData.at(exercise_idx)->weight[set_number].append(subrecord_separator + weight + subrecord_separator);
			}
			else if (new_type == SET_TYPE_DROP)
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number].append(dropSetReps(reps));
				m_ExerciseData.at(exercise_idx)->weight[set_number].append(dropSetWeight(weight));
				m_ExerciseData.at(exercise_idx)->subsets[set_number] = u"3"_qs;
			}
			else if (new_type == SET_TYPE_CLUSTER)
				m_ExerciseData.at(exercise_idx)->subsets[set_number]= u"4"_qs;
		break;

		case SET_TYPE_GIANT:
		case SET_TYPE_DROP:
			if (new_type == SET_TYPE_DROP)
			{
				m_ExerciseData.at(exercise_idx)->subsets[set_number] = u"3"_qs;
				const QString& new_reps(appUtils()->getCompositeValue(0, reps));
				m_ExerciseData.at(exercise_idx)->reps[set_number] = new_reps + dropSetReps(new_reps);
				const QString& new_weight(appUtils()->getCompositeValue(0, weight));
				m_ExerciseData.at(exercise_idx)->weight[set_number] = new_weight + dropSetWeight(new_weight);
			}
			else if (new_type == SET_TYPE_GIANT)
			{
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, reps), m_ExerciseData.at(exercise_idx)->reps[set_number]);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, reps), m_ExerciseData.at(exercise_idx)->reps[set_number]);
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, weight), m_ExerciseData.at(exercise_idx)->weight[set_number]);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, weight), m_ExerciseData.at(exercise_idx)->weight[set_number]);
			}
			else
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number] = appUtils()->getCompositeValue(0, reps);
				m_ExerciseData.at(exercise_idx)->weight[set_number] = appUtils()->getCompositeValue(0, weight);
				if (new_type == SET_TYPE_CLUSTER)
					m_ExerciseData.at(exercise_idx)->subsets[set_number]= u"4"_qs;
			}
		break;
	}
	setSetType(set_number, exercise_idx, new_type);
}

QString DBTrainingDayModel::setRestTime(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setRestTime()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->resttime.at(set_number);
}

void DBTrainingDayModel::setSetRestTime(const uint set_number, const uint exercise_idx, const QString& new_time)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetRestTime()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->resttime[set_number] = new_time;
	setModified(true);
}

QString DBTrainingDayModel::setSubSets(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSubSets()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->subsets.at(set_number);
}

void DBTrainingDayModel::newSetSubSet(const uint set_number, const uint exercise_idx)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::newSetSubSet()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->subsets[set_number] = QString::number(m_ExerciseData.at(exercise_idx)->subsets.at(set_number).toUInt() + 1);
	m_ExerciseData.at(exercise_idx)->reps[set_number].append(QString::number(
		m_ExerciseData.at(exercise_idx)->reps.constLast().split(subrecord_separator, Qt::SkipEmptyParts).constLast().toUInt() - 2) + subrecord_separator);
	m_ExerciseData.at(exercise_idx)->weight[set_number].append(QString::number(
		m_ExerciseData.at(exercise_idx)->weight.constLast().split(subrecord_separator, Qt::SkipEmptyParts).constLast().toUInt() - 10) + subrecord_separator);
	setModified(true);
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const uint exercise_idx, const QString& new_subsets)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->subsets.count(), "DBTrainingDayModel::setType()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->subsets[set_number] = new_subsets;
	setModified(true);
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setReps()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->reps.at(set_number);
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint exercise_idx, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetReps()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->reps[set_number] = new_reps;
	setModified(true);
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setWeight()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->weight.at(set_number);
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint exercise_idx, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetWeight()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->weight[set_number] = new_weight;
	setModified(true);
}

QString DBTrainingDayModel::setNotes(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->notes.count(), "DBTrainingDayModel::setNotes()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->notes.at(set_number);
}

void DBTrainingDayModel::setSetNotes(const uint set_number, const QString& new_notes, const uint exercise_idx)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->notes.count(), "DBTrainingDayModel::setSetNotes()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->notes[set_number] = new_notes;
	setModified(true);
}

bool DBTrainingDayModel::setCompleted(const uint set_number, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->completed.count(), "DBTrainingDayModel::setCompleted()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->completed.at(set_number) == u"1"_qs;
}

void DBTrainingDayModel::setSetCompleted(const uint set_number, const uint exercise_idx, const bool completed)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->completed.count(), "DBTrainingDayModel::setSetCompleted()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->completed[set_number] = completed ? STR_ONE : STR_ZERO;
	emit exerciseCompleted(exercise_idx, allSetsCompleted(exercise_idx));
	if (completed)
	{
		if (modified())
			emit saveWorkout();
	}
}

bool DBTrainingDayModel::allSetsCompleted(const uint exercise_idx) const
{
	const uint nsets(m_ExerciseData.at(exercise_idx)->nsets);
	if (nsets > 0)
	{
		for (uint i(0); i < nsets; ++i)
		{
			if (m_ExerciseData.at(exercise_idx)->completed.at(i) == STR_ZERO)
				return false;
		}
		return true;
	}
	return false;
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint subset, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setReps()", "out of range set_number");
	const QStringList& subSetReps(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
	return subset < subSetReps.count() ? subSetReps.at(subset) : QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetReps()", "out of range set_number");
	QStringList subSetReps(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
	const uint total(subSetReps.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetReps.append(new_reps);
	}
	else
		subSetReps[subset] = new_reps;
	m_ExerciseData.at(exercise_idx)->reps[set_number] = subSetReps.join(subrecord_separator) + subrecord_separator;
	setModified(true);
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint subset, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setWeight()", "out of range set_number");
	const QStringList& subSetWeight(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
	return subset < subSetWeight.count() ? subSetWeight.at(subset) : QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetWeight()", "out of range set_number");
	QStringList subSetWeight(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
	const uint total(subSetWeight.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetWeight.append(new_weight);
	}
	else
		subSetWeight[subset] = new_weight;
	m_ExerciseData.at(exercise_idx)->weight[set_number] = subSetWeight.join(subrecord_separator) + subrecord_separator;
	setModified(true);
}
