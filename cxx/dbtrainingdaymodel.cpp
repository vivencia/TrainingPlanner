#include "dbtrainingdaymodel.h"
#include "tpglobals.h"
#include "dbmesosplitmodel.h"
//#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "tputils.h"

#include <QtMath>
#include <utility>

DBTrainingDayModel::DBTrainingDayModel(QObject* parent, const uint meso_idx)
	: TPListModel(parent, static_cast<int>(meso_idx))
{
	setObjectName(DBTrainingDayObjectName);
	m_tableId = TRAININGDAY_TABLE_ID;
	m_fieldCount = TDAY_TOTAL_COLS;
	m_exportName = std::move(tr("Single workout"));
}

void DBTrainingDayModel::fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse)
{
	const QStringList& exercises_names(list.at(TDAY_EXERCISES_COL_NAMES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& setstypes(list.at(TDAY_EXERCISES_COL_TYPES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& resttimes(list.at(TDAY_EXERCISES_COL_RESTTIMES).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& subsets(list.at(TDAY_EXERCISES_COL_SUBSETS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& reps(list.at(TDAY_EXERCISES_COL_REPS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& weights(list.at(TDAY_EXERCISES_COL_WEIGHTS).split(exercises_separator, Qt::SkipEmptyParts));
	const QStringList& notes(list.at(TDAY_EXERCISES_COL_NOTES).split(exercises_separator, Qt::SkipEmptyParts));

	QStringList completed;
	if (!bClearSomeFieldsForReUse)
		completed = std::move(list.at(TDAY_EXERCISES_COL_COMPLETED).split(exercises_separator, Qt::SkipEmptyParts));
	else //reset for new use
	{
		QString temp{list.at(TDAY_EXERCISES_COL_COMPLETED)};
		completed = std::move(temp.replace(STR_ONE, STR_ZERO).split(exercises_separator, Qt::SkipEmptyParts));
	}

	for(uint i(0); i < exercises_names.count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		m_ExerciseData[i]->name = std::move(exercises_names.at(i));
		m_ExerciseData[i]->type = std::move(setstypes.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->resttime = std::move(resttimes.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->subsets = std::move(subsets.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->reps = std::move(reps.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->weight = std::move(weights.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->notes = std::move(notes.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->completed = std::move(completed.at(i).split(record_separator, Qt::SkipEmptyParts));
		m_ExerciseData[i]->nsets = m_ExerciseData.at(i)->type.count();
	}
	emit exerciseCountChanged();
	if (bClearSomeFieldsForReUse) //import from another date. Save the copied data to the new date
		emit tDayChanged();
}

const QStringList DBTrainingDayModel::getSaveInfo() const
{
	QStringList data(TDAY_EXERCISES_TOTALCOLS);
	for(uint i(0); i < m_ExerciseData.count(); ++i)
	{
		data[TDAY_EXERCISES_COL_NAMES].append(m_ExerciseData.at(i)->name + exercises_separator);
		for(uint x(0); x < m_ExerciseData.at(i)->nsets; ++x)
		{
			data[TDAY_EXERCISES_COL_TYPES].append(m_ExerciseData.at(i)->type.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_RESTTIMES].append(m_ExerciseData.at(i)->resttime.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_SUBSETS].append((x < m_ExerciseData.at(i)->subsets.count() ? m_ExerciseData.at(i)->subsets.at(x) : "0"_L1) + record_separator);
			data[TDAY_EXERCISES_COL_REPS].append(m_ExerciseData.at(i)->reps.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_WEIGHTS].append(m_ExerciseData.at(i)->weight.at(x) + record_separator);
			data[TDAY_EXERCISES_COL_NOTES].append((x < m_ExerciseData.at(i)->notes.count() ? m_ExerciseData.at(i)->notes.at(x) : " "_L1) + record_separator);
			data[TDAY_EXERCISES_COL_COMPLETED].append(m_ExerciseData.at(i)->completed.at(x) + record_separator);
		}
		data[TDAY_EXERCISES_COL_TYPES].append(exercises_separator);
		data[TDAY_EXERCISES_COL_RESTTIMES].append(exercises_separator);
		data[TDAY_EXERCISES_COL_SUBSETS].append(exercises_separator);
		data[TDAY_EXERCISES_COL_REPS].append(exercises_separator);
		data[TDAY_EXERCISES_COL_WEIGHTS].append(exercises_separator);
		data[TDAY_EXERCISES_COL_NOTES].append(exercises_separator);
		data[TDAY_EXERCISES_COL_COMPLETED].append(exercises_separator);
	}
	return data;
}

void DBTrainingDayModel::convertMesoSplitModelToTDayModel(DBMesoSplitModel* const splitModel)
{
	for(uint i(0); i < splitModel->count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		m_ExerciseData[i]->name = splitModel->exerciseName(i, true);

		const uint nsets{splitModel->setsNumber(i)};
		uint set_type = splitModel->setType(i, 0);
		QString nreps, nweight;
		if (set_type != SET_TYPE_GIANT)
		{
			nreps = std::move(splitModel->setReps(i, 0));
			nweight = std::move(splitModel->setWeight(i, 0));
		}
		else
		{
			appUtils()->setCompositeValue(0, splitModel->setReps1(i, 0), nreps, comp_exercise_separator);
			appUtils()->setCompositeValue(1, splitModel->setReps2(i, 0), nreps, comp_exercise_separator);
			appUtils()->setCompositeValue(0, splitModel->setWeight1(i, 0), nweight, comp_exercise_separator);
			appUtils()->setCompositeValue(1, splitModel->setWeight2(i, 0), nweight, comp_exercise_separator);
		}
		newFirstSet(i, set_type, nreps, nweight, nextSetSuggestedTime(i, set_type, 0), splitModel->setSubsets(i, 0), splitModel->_setsNotes(i));
		for(uint x(1); x < nsets; ++x)
		{
			splitModel->setWorkingSet(i, x, false);
			set_type = splitModel->setType(i, x);
			if (set_type != SET_TYPE_GIANT)
			{
				nreps = std::move(splitModel->setReps(i, x));
				nweight = std::move(splitModel->setWeight(i, x));
			}
			else
			{
				appUtils()->setCompositeValue(0, splitModel->setReps1(i, x), nreps, comp_exercise_separator);
				appUtils()->setCompositeValue(1, splitModel->setReps2(i, x), nreps, comp_exercise_separator);
				appUtils()->setCompositeValue(0, splitModel->setWeight1(i, x), nweight, comp_exercise_separator);
				appUtils()->setCompositeValue(1, splitModel->setWeight2(i, x), nweight, comp_exercise_separator);
			}
			newSet(i, x, set_type, nreps, nweight, nextSetSuggestedTime(i, set_type), splitModel->setSubsets(i, x));
		}
	}
	emit exerciseCountChanged();
	emit tDayChanged(); //save now
}

int DBTrainingDayModel::exportToFile(const QString &filename, const bool, const bool, const bool) const
{
	if (exerciseCount() == 0)
		return APPWINDOW_MSG_NOTHING_TO_EXPORT;

	QFile *outFile{new QFile{filename}};
	const bool bOK{outFile->open(QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (bOK)
	{
		const QString &strHeader{"## "_L1 + exportName() + " - 0x000"_L1 + QString::number(tableID()) + "\n\n"_L1};
		outFile->write(strHeader.toUtf8().constData());
		outFile->write(exportExtraInfo().toUtf8().constData());
		outFile->write("\n\n", 2);

		QString setsTypes, subSets;
		bool bHasSubsSets{false};
		for (uint i{0}; i < m_ExerciseData.count(); ++i)
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

	QString value;
	uint exercise_idx(0);
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
					newExercise(exercise_idx);
					setExerciseName(exercise_idx, value.remove(0, value.indexOf(':') + 2).trimmed().replace(comp_exercise_fancy_separator, QChar(comp_exercise_separator)));

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

					newFirstSet(exercise_idx, appUtils()->getCompositeValue(0, type, set_separator).toUInt(),
						appUtils()->getCompositeValue(0, reps, set_separator), appUtils()->getCompositeValue(0, weight, set_separator),
						appUtils()->getCompositeValue(0, resttime, set_separator), appUtils()->getCompositeValue(0, subsets, set_separator),
						appUtils()->getCompositeValue(0, notes, set_separator));
					for (uint i(1); i < nsets; ++i)
						newSet(exercise_idx, i, appUtils()->getCompositeValue(i, type, set_separator).toUInt(),
						appUtils()->getCompositeValue(i, reps, set_separator), appUtils()->getCompositeValue(i, weight, set_separator),
						appUtils()->getCompositeValue(i, resttime, set_separator), appUtils()->getCompositeValue(i, subsets, set_separator));
				}
				++exercise_idx;
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	return exerciseCount() > 0 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBTrainingDayModel::updateFromModel(TPListModel* model)
{
	DBTrainingDayModel* tDayModel{static_cast<DBTrainingDayModel*>(model)};
	for (uint i{0}; i < tDayModel->exerciseCount(); ++i)
	{
		newExercise(i);
		setExerciseName(i, tDayModel->exerciseName(i));
		newFirstSet(i, tDayModel->setType(0, i), tDayModel->setReps(0, i), tDayModel->setWeight(0, i), tDayModel->setRestTime(0, i),
					tDayModel->setSubSets(0, 1), tDayModel->setNotes(0, i));
		for (uint x(1); x < tDayModel->setsNumber(i); ++x)
		{
			newSet(i, x, tDayModel->setType(x, i), tDayModel->setReps(x, i), tDayModel->setWeight(x, i), tDayModel->setRestTime(x, i),
					tDayModel->setSubSets(x, 1));
		}
	}
	if (tDayModel->importMode())
		delete tDayModel;
	if (exerciseCount() > 0)
	{
		emit exerciseCountChanged();
		return true;
	}
	return false;
}

//Don't put any colon ':' in here. Import will fail
const QString DBTrainingDayModel::exportExtraInfo() const
{
	return tr("Workout #") + m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER) + tr(", split ") + m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER) +
		" ("_L1 + appMesoModel()->mesoSplitModel()->splitX(m_mesoIdx, appUtils()->splitLetterToMesoSplitIndex(m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER))) +
		tr(") at ") + appUtils()->formatDate(date());
}

const QString DBTrainingDayModel::formatSetTypeToExport(const QString& fieldValue) const
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

const QString DBTrainingDayModel::formatSetTypeToImport(const QString& fieldValue) const
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

void DBTrainingDayModel::moveExercise(const uint from, const uint to)
{
	if (from < m_ExerciseData.count() && to < m_ExerciseData.count())
	{
		exerciseEntry* tempExerciseData(std::move(m_ExerciseData[from]));

		if (to > from)
		{
			for(uint i(from); i < to; ++i)
				m_ExerciseData[i] = std::move(m_ExerciseData[i+1]);
		}
		else
		{
			for(uint i(from); i > to; --i)
				m_ExerciseData[i] = std::move(m_ExerciseData[i-1]);
		}
		m_ExerciseData[to] = std::move(tempExerciseData);
		emit tDayChanged();
	}
}

uint DBTrainingDayModel::getWorkoutNumberForTrainingDay() const
{
	return appMesoModel()->mesoCalendarModel(mesoIdx())->getLastTrainingDayBeforeDate(date()) + 1;
}

void DBTrainingDayModel::newExercise(const uint exercise_idx)
{
	const uint total(m_ExerciseData.count());
	const int n(exercise_idx - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			m_ExerciseData.append(new exerciseEntry);
	}
	emit exerciseCountChanged();
}

void DBTrainingDayModel::removeExercise(const uint exercise_idx)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::removeExercise", "out of range exercise_idx");
	delete m_ExerciseData.at(exercise_idx);
	m_ExerciseData.remove(exercise_idx);
	emit exerciseCountChanged();
}

static QString increaseStringTimeBy(const QString& strtime, const uint add_mins, const uint add_secs)
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
	const QString& ret{(mins <= 9 ? STR_ZERO + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? STR_ZERO + QString::number(secs) : QString::number(secs))};
	return ret;
}

static inline QString dropSetReps(const QString& reps)
{
	const float value{appUtils()->appLocale()->toFloat(reps)};
	QString value1{std::move(appUtils()->appLocale()->toString(qCeil(value * 0.8)))};
	if (value1.contains('.') || value1.contains(','))
	{
		if (value1.last(2) != "50"_L1)
			value1.chop(3); //nn
		else
			value1.chop(1); // nn,5 or nn.5
	}
	QString value2{std::move(appUtils()->appLocale()->toString(qCeil(value * 0.8 * 0.8)))};
	if (value2.contains('.') || value2.contains(','))
	{
		if (value2.last(2) != "50"_L1)
			value2.chop(3); //nn
		else
			value2.chop(1); // nn,5 or nn.5
	}
	return comp_exercise_separator + value1 + comp_exercise_separator + value2 + comp_exercise_separator;
}

static inline QString dropSetWeight(const QString& weight)
{
	const float value{appUtils()->appLocale()->toFloat(weight)};
	QString value1{std::move(appUtils()->appLocale()->toString(value * 0.5, 'f', 2))};
	if (value1.contains('.') || value1.contains(','))
	{
		if (value1.last(2) != "50"_L1)
			value1.chop(3); //nn
		else
			value1.chop(1); // nn,5 or nn.5
	}
	QString value2{std::move(appUtils()->appLocale()->toString(value * 0.5 * 0.5, 'f', 2))};
	if (value2.contains('.') || value2.contains(','))
	{
		if (value2.last(2) != "50"_L1)
			value2.chop(3); //nn
		else
			value2.chop(1); // nn,5 or nn.5
	}
	return comp_exercise_separator + value1 + comp_exercise_separator + value2 + comp_exercise_separator;
}

void DBTrainingDayModel::newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight,
										const QString& nRestTime, const QString& nSubsets, const QString& notes)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::newFirstSet()", "out of range exercise_idx");
	const QString& strType{QString::number(type)};
	m_ExerciseData.at(exercise_idx)->nsets = 1;
	m_ExerciseData.at(exercise_idx)->type.append(strType);
	m_ExerciseData.at(exercise_idx)->notes.append(notes);
	m_ExerciseData.at(exercise_idx)->completed.append(STR_ZERO);
	m_ExerciseData.at(exercise_idx)->resttime.append(nRestTime);
	m_ExerciseData.at(exercise_idx)->reps.append(nReps);
	m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
	m_ExerciseData.at(exercise_idx)->subsets.append(nSubsets);
}

QString DBTrainingDayModel::nextSetSuggestedTime(const uint exercise_idx, const uint type, const uint set_number) const
{
	QString strSetTime;
	if (set_number == 0)
		return type != SET_TYPE_MYOREPS ?  "01:30"_L1 : "02:30"_L1;
	else
	{
		if (!m_ExerciseData.at(exercise_idx)->mb_TrackRestTime)
			return m_ExerciseData.at(exercise_idx)->resttime.at(0);
		if (m_ExerciseData.at(exercise_idx)->mb_AutoRestTime)
			return "00:00"_L1;
		strSetTime = set_number == USE_LAST_SET_DATA ?
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
			strSetTime = std::move(increaseStringTimeBy(strSetTime, 0, 30));
		break;
		case SET_TYPE_CLUSTER:
			strSetTime = std::move(increaseStringTimeBy(strSetTime , 1, 0));
		break;
		case SET_TYPE_MYOREPS:
			strSetTime = std::move(increaseStringTimeBy(strSetTime, 1, 30));
		break;
	}
	return strSetTime;
}

const QString DBTrainingDayModel::nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number, const uint sub_set) const
{
	QString strSetReps;
	switch (set_number)
	{
		case 0:
			switch (type)
			{
				case SET_TYPE_REGULAR:
				case SET_TYPE_GIANT:
					strSetReps = "12"_L1;
				break;
				case SET_TYPE_DROP:
				case SET_TYPE_PYRAMID:
					strSetReps = "15"_L1;
				break;
				case SET_TYPE_CLUSTER:
				case SET_TYPE_REVERSE_PYRAMID:
					strSetReps = "6"_L1;
				break;
				case SET_TYPE_MYOREPS:
					strSetReps = "20"_L1;
				break;
			}
			return strSetReps;
		break;
		case USE_LAST_SET_DATA:
			strSetReps = sub_set == USE_LAST_SET_DATA ? m_ExerciseData.at(exercise_idx)->reps.constLast() :
							std::move(m_ExerciseData.at(exercise_idx)->reps.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).at(sub_set));
		break;
		default:
			const QString& reps{m_ExerciseData.at(exercise_idx)->reps.at(set_number)};
			strSetReps = sub_set == USE_LAST_SET_DATA ? reps : reps.contains(comp_exercise_separator) ?
													std::move(reps.split(comp_exercise_separator, Qt::SkipEmptyParts).at(sub_set)) :
													std::move(reps);
		break;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue{appUtils()->appLocale()->toFloat(strSetReps)};
		if (type == SET_TYPE_PYRAMID)
			lastSetValue = qCeil(lastSetValue * 0.8);
		else
			lastSetValue = qCeil(lastSetValue * 1.25);
		strSetReps = appUtils()->appLocale()->toString(static_cast<int>(lastSetValue));
		if (strSetReps.contains('.') || strSetReps.contains(','))
		{
			if (strSetReps.last(2) != "50"_L1)
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
	switch (set_number)
	{
		case 0:
			return "20"_L1;
		break;
		case USE_LAST_SET_DATA:
			strStrWeight = sub_set == USE_LAST_SET_DATA ? m_ExerciseData.at(exercise_idx)->weight.constLast() :
							std::move(m_ExerciseData.at(exercise_idx)->weight.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).at(sub_set));
		break;
		default:
			const QString& weight{m_ExerciseData.at(exercise_idx)->weight.at(set_number)};
			strStrWeight = sub_set == USE_LAST_SET_DATA ? weight : weight.contains(comp_exercise_separator) ?
													std::move(weight.split(comp_exercise_separator, Qt::SkipEmptyParts).at(sub_set)) :
													std::move(weight);
		break;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue{appUtils()->appLocale()->toFloat(strStrWeight)};
		if (type == SET_TYPE_PYRAMID)
			lastSetValue *= 1.2;
		else
			lastSetValue *= 0.8;
		strStrWeight = appUtils()->appLocale()->toString(lastSetValue, 'f', 2);
		if (strStrWeight.contains('.') || strStrWeight.contains(','))
		{
			if (strStrWeight.last(2) != "50"_L1)
				strStrWeight.chop(3);
			else
				strStrWeight.chop(1);
		}
	}
	return strStrWeight;
}

void DBTrainingDayModel::newSet(const uint exercise_idx, const uint set_number, const uint type,
							const QString& nReps, const QString& nWeight, const QString& nRestTime, const QString& nSubSets)
{
	Q_ASSERT_X(exercise_idx < m_ExerciseData.count(), "DBTrainingDayModel::newSet()", "out of range exercise_idx");
	const uint total{m_ExerciseData.at(exercise_idx)->nsets};
	const int n(set_number - total + 1);
	const QString& strType{QString::number(type)};

	m_ExerciseData.at(exercise_idx)->nsets += n;

	for(uint i{0}; i < n; ++i)
	{
		m_ExerciseData.at(exercise_idx)->type.append(strType);
		m_ExerciseData.at(exercise_idx)->resttime.append(nRestTime.isEmpty() ? std::move(nextSetSuggestedTime(exercise_idx, type)) : nRestTime);
		m_ExerciseData.at(exercise_idx)->reps.append(nReps.isEmpty() ? std::move(nextSetSuggestedReps(exercise_idx, type)) : nReps);
		m_ExerciseData.at(exercise_idx)->weight.append(nWeight.isEmpty() ? std::move(nextSetSuggestedWeight(exercise_idx, type)) : nWeight);
		m_ExerciseData.at(exercise_idx)->notes.append(m_ExerciseData.at(exercise_idx)->notes.constLast());
		m_ExerciseData.at(exercise_idx)->completed.append(STR_ZERO);
		m_ExerciseData.at(exercise_idx)->subsets.append(nSubSets.isEmpty() ?
				(type != SET_TYPE_MYOREPS ?
					m_ExerciseData.at(exercise_idx)->subsets.constLast() :
					std::move(QString::number(m_ExerciseData.at(exercise_idx)->subsets.constLast().toInt() + 1)))
				 : nSubSets);

		const uint n_exercises{static_cast<uint>(m_ExerciseData.at(exercise_idx)->type.count())};
		if (n_exercises > 1)
		{
			if (strType != m_ExerciseData.at(exercise_idx)->type.at(n_exercises - 2))
				changeSetType(exercise_idx, set_number, m_ExerciseData.at(exercise_idx)->type.at(n_exercises - 2).toUInt(), type);
		}
	}
}

void DBTrainingDayModel::removeSet(const uint exercise_idx, const uint set_number)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::removeSet()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->type.remove(set_number);
	m_ExerciseData.at(exercise_idx)->resttime.remove(set_number);
	m_ExerciseData.at(exercise_idx)->subsets.remove(set_number);
	m_ExerciseData.at(exercise_idx)->reps.remove(set_number);
	m_ExerciseData.at(exercise_idx)->weight.remove(set_number);
	m_ExerciseData.at(exercise_idx)->notes.remove(set_number);
	m_ExerciseData.at(exercise_idx)->nsets--;
	emit exerciseCompleted(exercise_idx, allSetsCompleted(exercise_idx));
	emit tDayChanged();
}

uint DBTrainingDayModel::setType(const uint exercise_idx, const uint set_number) const
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
		return m_ExerciseData.at(exercise_idx)->type.at(set_number).toUInt();
	return SET_TYPE_REGULAR;
}

void DBTrainingDayModel::setSetType(const uint exercise_idx, const uint set_number, const uint new_type)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setType()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->type[set_number] = QString::number(new_type);
}

void DBTrainingDayModel::changeSetType(const uint exercise_idx, const uint set_number, const uint old_type, const uint new_type)
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
				m_ExerciseData.at(exercise_idx)->reps[set_number].append(comp_exercise_separator + reps + comp_exercise_separator);
				m_ExerciseData.at(exercise_idx)->weight[set_number].append(comp_exercise_separator + weight + comp_exercise_separator);
			}
			else if (new_type == SET_TYPE_DROP)
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number].append(dropSetReps(reps));
				m_ExerciseData.at(exercise_idx)->weight[set_number].append(dropSetWeight(weight));
				m_ExerciseData.at(exercise_idx)->subsets[set_number] = "3"_L1;
			}
			else if (new_type == SET_TYPE_CLUSTER)
				m_ExerciseData.at(exercise_idx)->subsets[set_number]= "4"_L1;
		break;

		case SET_TYPE_GIANT:
		case SET_TYPE_DROP:
			if (new_type == SET_TYPE_DROP)
			{
				m_ExerciseData.at(exercise_idx)->subsets[set_number] = "3"_L1;
				const QString& new_reps(appUtils()->getCompositeValue(0, reps, comp_exercise_separator));
				m_ExerciseData.at(exercise_idx)->reps[set_number] = new_reps + dropSetReps(new_reps);
				const QString& new_weight(appUtils()->getCompositeValue(0, weight, comp_exercise_separator));
				m_ExerciseData.at(exercise_idx)->weight[set_number] = new_weight + dropSetWeight(new_weight);
			}
			else if (new_type == SET_TYPE_GIANT)
			{
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, reps, comp_exercise_separator), m_ExerciseData.at(exercise_idx)->reps[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, reps, comp_exercise_separator), m_ExerciseData.at(exercise_idx)->reps[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(0, appUtils()->getCompositeValue(0, weight, comp_exercise_separator), m_ExerciseData.at(exercise_idx)->weight[set_number], comp_exercise_separator);
				appUtils()->setCompositeValue(1, appUtils()->getCompositeValue(1, weight, comp_exercise_separator), m_ExerciseData.at(exercise_idx)->weight[set_number], comp_exercise_separator);
			}
			else
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number] = appUtils()->getCompositeValue(0, reps, comp_exercise_separator);
				m_ExerciseData.at(exercise_idx)->weight[set_number] = appUtils()->getCompositeValue(0, weight, comp_exercise_separator);
				if (new_type == SET_TYPE_CLUSTER)
					m_ExerciseData.at(exercise_idx)->subsets[set_number]= "4"_L1;
			}
		break;
	}
	setSetType(exercise_idx, set_number, new_type);
}

QString DBTrainingDayModel::setRestTime(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setRestTime()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->resttime.at(set_number);
}

void DBTrainingDayModel::setSetRestTime(const uint exercise_idx, const uint set_number, const QString& new_time)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetRestTime()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->resttime[set_number] = new_time;
}

QString DBTrainingDayModel::setSubSets(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSubSets()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->subsets.at(set_number);
}

void DBTrainingDayModel::newSetSubSet(const uint exercise_idx, const uint set_number)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::newSetSubSet()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->subsets[set_number] = QString::number(m_ExerciseData.at(exercise_idx)->subsets.at(set_number).toUInt() + 1);
	m_ExerciseData.at(exercise_idx)->reps[set_number].append(QString::number(
		m_ExerciseData.at(exercise_idx)->reps.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).constLast().toUInt() - 2) + comp_exercise_separator);
	m_ExerciseData.at(exercise_idx)->weight[set_number].append(QString::number(
		m_ExerciseData.at(exercise_idx)->weight.constLast().split(comp_exercise_separator, Qt::SkipEmptyParts).constLast().toUInt() - 10) + comp_exercise_separator);
}

void DBTrainingDayModel::setSetSubSets(const uint exercise_idx, const uint set_number, const QString& new_subsets)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->subsets.count(), "DBTrainingDayModel::setType()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->subsets[set_number] = new_subsets;
}

QString DBTrainingDayModel::setReps(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setReps()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->reps.at(set_number);
}

void DBTrainingDayModel::setSetReps(const uint exercise_idx, const uint set_number, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetReps()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->reps[set_number] = new_reps;
}

QString DBTrainingDayModel::setWeight(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setWeight()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->weight.at(set_number);
}

void DBTrainingDayModel::setSetWeight(const uint exercise_idx, const uint set_number, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetWeight()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->weight[set_number] = new_weight;
}

QString DBTrainingDayModel::setNotes(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->notes.count(), "DBTrainingDayModel::setNotes()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->notes.at(set_number);
}

void DBTrainingDayModel::setSetNotes(const uint exercise_idx, const uint set_number, const QString& new_notes)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->notes.count(), "DBTrainingDayModel::setSetNotes()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->notes[set_number] = new_notes;
}

bool DBTrainingDayModel::setCompleted(const uint exercise_idx, const uint set_number) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->completed.count(), "DBTrainingDayModel::setCompleted()", "out of range set_number");
	return m_ExerciseData.at(exercise_idx)->completed.at(set_number) == STR_ONE;
}

void DBTrainingDayModel::setSetCompleted(const uint exercise_idx, const uint set_number, const bool completed)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->completed.count(), "DBTrainingDayModel::setSetCompleted()", "out of range set_number");
	m_ExerciseData.at(exercise_idx)->completed[set_number] = completed ? STR_ONE : STR_ZERO;
	emit exerciseCompleted(exercise_idx, allSetsCompleted(exercise_idx));
	if (completed)
		emit tDayChanged();
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

bool DBTrainingDayModel::anySetCompleted(const uint exercise_idx) const
{
	const uint nsets(m_ExerciseData.at(exercise_idx)->nsets);
	if (nsets > 0)
	{
		for (uint i(0); i < nsets; ++i)
		{
			if (m_ExerciseData.at(exercise_idx)->completed.at(i) == STR_ONE)
				return true;
		}
		return false;
	}
	return true;
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint subset, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setReps()", "out of range set_number");
	const QStringList& subSetReps(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts));
	return subset < subSetReps.count() ? subSetReps.at(subset) : QString();
}

void DBTrainingDayModel::setSetReps(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_reps)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetReps()", "out of range set_number");
	QStringList subSetReps(std::move(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts)));
	const uint total(subSetReps.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetReps.append(new_reps);
	}
	else
		subSetReps[subset] = new_reps;
	m_ExerciseData.at(exercise_idx)->reps[set_number] = std::move(subSetReps.join(comp_exercise_separator) + comp_exercise_separator);
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint subset, const uint exercise_idx) const
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setWeight()", "out of range set_number");
	const QStringList& subSetWeight(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts));
	return subset < subSetWeight.count() ? subSetWeight.at(subset) : QString();
}

void DBTrainingDayModel::setSetWeight(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_weight)
{
	Q_ASSERT_X(set_number < m_ExerciseData.at(exercise_idx)->nsets, "DBTrainingDayModel::setSetWeight()", "out of range set_number");
	QStringList subSetWeight(std::move(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(comp_exercise_separator, Qt::SkipEmptyParts)));
	const uint total(subSetWeight.count());
	const int n(subset - total);
	if (n >= 0)
	{
		for(uint i(0); i <= n; ++i)
			subSetWeight.append(new_weight);
	}
	else
		subSetWeight[subset] = new_weight;
	m_ExerciseData.at(exercise_idx)->weight[set_number] = std::move(subSetWeight.join(comp_exercise_separator) + comp_exercise_separator);
}
