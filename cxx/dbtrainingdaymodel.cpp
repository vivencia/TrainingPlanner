#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "runcommands.h"

#include <QtMath>

static QString multiUseString;

void DBTrainingDayModel::fromDataBase(const QStringList& list)
{
	const QStringList exercises_names(list.at(TDAY_EXERCISES_COL_NAMES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList setstypes(list.at(TDAY_EXERCISES_COL_TYPES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList resttimes(list.at(TDAY_EXERCISES_COL_RESTTIMES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList subsets(list.at(TDAY_EXERCISES_COL_SUBSETS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList reps(list.at(TDAY_EXERCISES_COL_REPS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList weights(list.at(TDAY_EXERCISES_COL_WEIGHTS).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList notes(list.at(TDAY_EXERCISES_COL_NOTES).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList completed(list.at(TDAY_EXERCISES_COL_COMPLETED).split(record_separator2, Qt::SkipEmptyParts));

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

void DBTrainingDayModel::getSaveInfo(QStringList& data) const
{
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
}

void DBTrainingDayModel::convertMesoModelToTDayModel(DBMesoSplitModel* splitModel)
{
	for(uint i(0); i < splitModel->count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		splitModel->setCurrentRow(i);
		m_ExerciseData[i]->name = splitModel->exerciseName();

		const uint type(splitModel->setType());
		if (type == 4)
			m_ExerciseData[i]->name.replace(u" + "_qs, QString(subrecord_separator));
		newFirstSet(i, type, splitModel->setsReps(), splitModel->setsWeight(), splitModel->setsSubsets(), splitModel->setsNotes());
		newSet(splitModel->setsNumber().toUInt() - 1, i, type);
	}
	setModified(true);
	emit exerciseCountChanged();
}

void DBTrainingDayModel::updateFromModel(TPListModel* model)
{
	DBTrainingDayModel* tDayModel(static_cast<DBTrainingDayModel*>(model));
	for (uint i(0); i < tDayModel->exerciseCount(); ++i)
	{
		newExercise(tDayModel->exerciseName(i) , i);
		newFirstSet(i, tDayModel->setType(0, i), tDayModel->setReps(0, i), tDayModel->setWeight(0, i),
			tDayModel->setSubSets(0, 1), tDayModel->setNotes(0, i));
		for (uint x(1); x < tDayModel->setsNumber(i); ++x)
		{
			newSet(x, i, tDayModel->setType(x, i), tDayModel->setReps(x, i), tDayModel->setWeight(x, i),
					tDayModel->setSubSets(x, 1));
		}
	}
	setModified(true);
	emit exerciseCountChanged();
	//The model created in DbManager::importFromFile can now sefaly be delete
	delete model;
}

bool DBTrainingDayModel::importExtraInfo(const QString& extraInfo)
{
	int spaceIdx(extraInfo.lastIndexOf(' '));
	if (spaceIdx != -1)
	{
		const int fSlashIdx(extraInfo.indexOf('/', spaceIdx+1));
		const int fSlashIdx2 = extraInfo.indexOf('/', fSlashIdx+1);
		const uint day(extraInfo.mid(spaceIdx+1, fSlashIdx-spaceIdx-1).toUInt());
		const uint month(extraInfo.mid(fSlashIdx+1, fSlashIdx2-fSlashIdx-1).toUInt());
		const uint year(extraInfo.right(4).toUInt());
		const QDate date(year, month, day);
		setDate(date);
		return true;
	}
	return false;
}

void DBTrainingDayModel::exportToText(QFile* outFile, const bool bFancy) const
{
	this->TPListModel::exportToText(outFile, bFancy);

	if (exerciseCount() == 0)
		return;

	if (bFancy)
	{
		const QString strHeader(tr("Exercises:\n\n"));
		outFile->write(strHeader.toUtf8().constData());
	}

	uint settype(0);
	QString value;
	for (uint i(0); i < m_ExerciseData.count(); ++i)
	{
		if (bFancy)
		{
			settype = setType(0, i);
			outFile->write(QString(QString::number(i+1) + ": ").toUtf8().constData());
			outFile->write(exerciseName(i).toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Number of sets: ").toUtf8().constData());
			outFile->write(QString::number(setsNumber(i)).toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Type of sets: ").toUtf8().constData());
			outFile->write(QString::number(settype).toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Rest time between sets: ").toUtf8().constData());
			outFile->write(setRestTime(0, i).toUtf8().constData());
			outFile->write("\n", 1);
			if (settype == 2 || settype == 3 || settype == 5)
			{
				outFile->write(tr("Number of subsets: ").toUtf8().constData());
				outFile->write(setSubSets(0, i).toUtf8().constData());
				outFile->write("\n", 1);
			}
			outFile->write(tr("Initial number of reps: ").toUtf8().constData());
			outFile->write(setReps(0, 1).replace(subrecord_separator, '|').toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Initial weight: ").toUtf8().constData());
			outFile->write(setWeight(0, 1).replace(subrecord_separator, '|').toUtf8().constData());
			outFile->write("\n", 1);
			outFile->write(tr("Note for the sets: ").toUtf8().constData());
			outFile->write(setNotes(0, i).toUtf8().constData());
			outFile->write("\n\n", 1);
		}
		else
		{
			value = m_ExerciseData.at(i)->name + record_separator + QString::number(m_ExerciseData.at(i)->nsets) +
						record_separator + m_ExerciseData.at(i)->type.at(0) + record_separator + m_ExerciseData.at(i)->resttime.at(0) +
						record_separator + m_ExerciseData.at(i)->subsets.at(0) + record_separator + m_ExerciseData.at(i)->reps.at(0) +
						record_separator + m_ExerciseData.at(i)->weight.at(0) + record_separator + m_ExerciseData.at(i)->notes.at(0) +
						record_separator2;
			outFile->write(value.toUtf8().constData());
		}
	}
}

bool DBTrainingDayModel::importFromFancyText(QFile* inFile)
{
	if (!this->TPListModel::importFromFancyText(inFile))
		return false;

	char buf[256];
	QString inData;
	int sep_idx(-1);

	uint exerciseNumber(0), nsets(0);
	QString type, resttime, subsets, reps, weight, notes;

	while (inFile->readLine(buf, sizeof(buf)) != -1) {
		inData = buf;
		sep_idx = inData.indexOf(QString::number(exerciseNumber+1) + ':');
		if (sep_idx != -1)
		{
			newExercise(inData.mid(sep_idx, inData.length() - sep_idx).trimmed(), exerciseNumber);

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			nsets = inData.mid(sep_idx, inData.length() - sep_idx).trimmed().toUInt();

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			type = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			resttime = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			if ((sep_idx = inData.indexOf(tr("subsets"))) == -1)
			{
				subsets = u"0"_qs;
				if ((sep_idx = inData.indexOf(':')) == -1)
					return false;
				reps = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();
			}
			else
			{
				if ((sep_idx = inData.indexOf(':')) == -1)
					return false;
				subsets = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();
				if (inFile->readLine(buf, sizeof(buf)) == -1)
					return false;
				if ((sep_idx = inData.indexOf(':')) == -1)
					return false;
				reps = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();
			}

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			weight = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();

			if (inFile->readLine(buf, sizeof(buf)) == -1)
				return false;
			inData = buf;
			if ((sep_idx = inData.indexOf(':')) == -1)
				return false;
			notes = inData.mid(sep_idx, inData.length() - sep_idx).trimmed();
			if (notes.isEmpty())
				notes = u" "_qs;

			newFirstSet(exerciseNumber, type.toUInt(), reps, weight, subsets, notes);
			for (uint i(1); i < nsets; ++i)
				newSet(i, exerciseNumber, type.toUInt());
		}
	}
	return exerciseCount() > 0;
}

bool DBTrainingDayModel::importFromText(const QString& data)
{
	if (!this->TPListModel::importFromText(data))
		return false;

	int chr_pos1(data.indexOf(record_separator2));
	int chr_pos2(data.indexOf(record_separator2, chr_pos1+1));
	int rec_sep1(chr_pos1+1);
	int rec_sep2(data.indexOf(record_separator, rec_sep1));
	const uint dataSize(data.length());

	uint exerciseNumber(0), nsets(0);
	QString type, resttime, subsets, reps, weight, notes;

	while (chr_pos2 > 0 && chr_pos2 < dataSize) {
		newExercise(data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1), exerciseNumber);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		nsets = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1).toUInt();
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		type = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		resttime = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		subsets = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		reps = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		weight = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		notes = data.mid(rec_sep1, rec_sep2 - rec_sep1 - 1);
		rec_sep1= rec_sep2+1;
		rec_sep2 = data.indexOf(record_separator, rec_sep1);

		newFirstSet(exerciseNumber, type.toUInt(), reps, weight, subsets, notes);
		for (uint i(1); i < nsets; ++i)
			newSet(i, exerciseNumber, type.toUInt());

		chr_pos1 = chr_pos2+1;
		chr_pos2 = data.indexOf(record_separator2, chr_pos1+1);
		exerciseNumber++;
	}
	return count() > 0;
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

QString DBTrainingDayModel::exerciseName(const uint exercise_idx) const
{
	QString name;
	if (exercise_idx < m_ExerciseData.count())
	{
		name = m_ExerciseData.at(exercise_idx)->name;
		if (name.endsWith(subrecord_separator))
			name.chop(1);
		return name.replace(subrecord_separator, QStringLiteral(" + "));
	}
	return name;
}

void DBTrainingDayModel::setExerciseName(const QString& new_name, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(new_name.indexOf('+'));
		if (idx == -1)
			m_ExerciseData.at(exercise_idx)->name = new_name;
		else
		{
			QString new_name_copy(new_name);
			m_ExerciseData.at(exercise_idx)->name = new_name_copy.replace(QStringLiteral(" + "), QChar(subrecord_separator));
		}
		m_CompositeExerciseList[exercise_idx] = idx != -1;
		emit compositeExerciseChanged(exercise_idx);
		setModified(true);
	}
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
	if (exercise_idx < m_ExerciseData.count())
	{
		delete m_ExerciseData.at(exercise_idx);
		m_ExerciseData.remove(exercise_idx);
		setModified(true);
		emit exerciseCountChanged();
	}
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
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
		return idx != -1 ? QStringLiteral("1: ") + m_ExerciseData.at(exercise_idx)->name.left(idx) : m_ExerciseData.at(exercise_idx)->name;
	}
	return QString();
}

void DBTrainingDayModel::setExerciseName1(const QString& name1, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
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
}

QString DBTrainingDayModel::exerciseName2(const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
		return idx != -1 ? QStringLiteral("2: ") + m_ExerciseData.at(exercise_idx)->name.sliced(idx+1) : tr("2: Add exercise ...");
	}
	return QString();
}

void DBTrainingDayModel::setExerciseName2(const QString& name2, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
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
	QString ret(mins <=9 ? QChar('0') + QString::number(mins) : QString::number(mins));
	ret += QChar(':') + (secs <=9 ? QChar('0') + QString::number(secs) : QString::number(secs));
	return ret;
}

static inline QString dropSetReps(const QString& reps)
{
	const float value(appLocale.toFloat(reps));
	return subrecord_separator + appLocale.toString(qCeil(value * 0.8)) + subrecord_separator +
				appLocale.toString(qCeil(value * 0.8 * 0.8)) + subrecord_separator;
}

static inline QString dropSetWeight(const QString& weight)
{
	const float value(appLocale.toFloat(weight));
	return subrecord_separator + appLocale.toString(value * 0.5, 'f', 2) + subrecord_separator +
				appLocale.toString(value * 0.5 * 0.5, 'f', 2) + subrecord_separator;
}

void DBTrainingDayModel::newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight,
										const QString& nSubsets, const QString& notes)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const QString strType(QString::number(type));
		m_ExerciseData.at(exercise_idx)->nsets = 1;
		setModified(true);
		m_ExerciseData.at(exercise_idx)->type.append(strType);
		m_ExerciseData.at(exercise_idx)->notes.append(notes);
		m_ExerciseData.at(exercise_idx)->completed.append(u"0"_qs);

		switch (type) {
			case SET_TYPE_REGULAR:
			case SET_TYPE_PYRAMID:
			case SET_TYPE_REVERSE_PYRAMID:
				m_ExerciseData.at(exercise_idx)->resttime.append(QStringLiteral("01:30"));
				m_ExerciseData.at(exercise_idx)->reps.append(nReps);
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
				m_ExerciseData.at(exercise_idx)->subsets.append(u"0"_qs);
			break;
			case SET_TYPE_DROP:
			{
				m_ExerciseData.at(exercise_idx)->resttime.append(QStringLiteral("01:30"));
				m_ExerciseData.at(exercise_idx)->reps.append(nReps + dropSetReps(nReps));
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight + dropSetWeight(nWeight));
				m_ExerciseData.at(exercise_idx)->subsets.append(u"3"_qs);
			}
			break;
			case SET_TYPE_CLUSTER:
				m_ExerciseData.at(exercise_idx)->resttime.append(QStringLiteral("02:00"));
				m_ExerciseData.at(exercise_idx)->reps.append(nReps);
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
				m_ExerciseData.at(exercise_idx)->subsets.append(nSubsets);
			break;
			case SET_TYPE_GIANT:
				m_ExerciseData.at(exercise_idx)->resttime.append(QStringLiteral("01:30"));
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
				m_ExerciseData.at(exercise_idx)->resttime.append(QStringLiteral("02:30"));
				m_ExerciseData.at(exercise_idx)->reps.append(nReps);
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight);
				m_ExerciseData.at(exercise_idx)->subsets.append(nSubsets);
			break;
		}
	}
}

const QString& DBTrainingDayModel::nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number, const uint sub_set) const
{
	if (set_number == 100)
	{
		multiUseString = sub_set == 100 ? m_ExerciseData.at(exercise_idx)->reps.last() :
							m_ExerciseData.at(exercise_idx)->reps.last().split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set);
	}
	else
	{
		const QString reps(m_ExerciseData.at(exercise_idx)->reps.at(set_number));
		multiUseString = sub_set == 100 ? reps : reps.contains(subrecord_separator) ?
													reps.split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set) :
													reps;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue(appLocale.toFloat(multiUseString));
		if (type == SET_TYPE_PYRAMID)
			lastSetValue = qCeil(lastSetValue * 0.8);
		else
			lastSetValue = qCeil(lastSetValue * 1.25);
		multiUseString = appLocale.toString(static_cast<int>(lastSetValue));
	}
	return multiUseString;
}

const QString& DBTrainingDayModel::nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number, const uint sub_set) const
{
	if (set_number == 100)
	{
		multiUseString = sub_set == 100 ? m_ExerciseData.at(exercise_idx)->weight.last() :
							m_ExerciseData.at(exercise_idx)->weight.last().split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set);
	}
	else
	{
		const QString weight(m_ExerciseData.at(exercise_idx)->weight.at(set_number));
		multiUseString = sub_set == 100 ? weight : weight.contains(subrecord_separator) ?
													weight.split(subrecord_separator, Qt::SkipEmptyParts).at(sub_set) :
													weight;
	}

	if (type == SET_TYPE_PYRAMID || type == SET_TYPE_REVERSE_PYRAMID)
	{
		float lastSetValue(appLocale.toFloat(multiUseString));
		if (type == SET_TYPE_PYRAMID)
			lastSetValue *= 1.2;
		else
			lastSetValue *= 0.8;
		multiUseString = appLocale.toString(lastSetValue, 'f', 2);
	}
	return multiUseString;
}

void DBTrainingDayModel::newSet(const uint set_number, const uint exercise_idx, const uint type,
							const QString& nReps, const QString& nWeight, const QString& nSubSets)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const uint total(m_ExerciseData.at(exercise_idx)->nsets);
		const int n(set_number - total + 1);
		const QString strType(QString::number(type));
		if (n >= 1)
		{
			m_ExerciseData.at(exercise_idx)->nsets += n;
			setModified(true);

			for(uint i(0); i < n; ++i)
			{
				m_ExerciseData.at(exercise_idx)->subsets.append(nSubSets.isEmpty() ? m_ExerciseData.at(exercise_idx)->subsets.last() : nSubSets);
				m_ExerciseData.at(exercise_idx)->reps.append(nReps.isEmpty() ? nextSetSuggestedReps(exercise_idx, type) : nReps);
				m_ExerciseData.at(exercise_idx)->weight.append(nWeight.isEmpty() ? nextSetSuggestedWeight(exercise_idx, type) : nWeight);
				m_ExerciseData.at(exercise_idx)->notes.append(m_ExerciseData.at(exercise_idx)->notes.last());
				m_ExerciseData.at(exercise_idx)->completed.append(u"0"_qs);
				m_ExerciseData.at(exercise_idx)->type.append(strType);

				if (strType != m_ExerciseData.at(exercise_idx)->type.at(set_number-1-i))
					changeSetType (set_number, exercise_idx, m_ExerciseData.at(exercise_idx)->type.at(set_number-1-i).toUInt(), type);

				switch (type)
				{
					case SET_TYPE_REGULAR:
					case SET_TYPE_PYRAMID:
					case SET_TYPE_DROP:
					case SET_TYPE_GIANT:
					case SET_TYPE_REVERSE_PYRAMID:
						m_ExerciseData.at(exercise_idx)->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
					break;
					case SET_TYPE_CLUSTER:
						m_ExerciseData.at(exercise_idx)->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 0));
					break;
					case SET_TYPE_MYOREPS:
						m_ExerciseData.at(exercise_idx)->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 30));
						m_ExerciseData.at(exercise_idx)->subsets.last() = QString::number(m_ExerciseData.at(exercise_idx)->subsets.last().toInt() + 1);
					break;
				}
			}
		}
	}
}

bool DBTrainingDayModel::removeSet(const uint set_number, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
		{
			m_ExerciseData.at(exercise_idx)->type.remove(set_number);
			m_ExerciseData.at(exercise_idx)->resttime.remove(set_number);
			m_ExerciseData.at(exercise_idx)->subsets.remove(set_number);
			m_ExerciseData.at(exercise_idx)->reps.remove(set_number);
			m_ExerciseData.at(exercise_idx)->weight.remove(set_number);
			m_ExerciseData.at(exercise_idx)->notes.remove(set_number);
			m_ExerciseData.at(exercise_idx)->nsets--;
			setModified(true);
			return true;
		}
	}
	return false;
}

uint DBTrainingDayModel::setType(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
			return m_ExerciseData.at(exercise_idx)->type.at(set_number).toUInt();
	}
	return SET_TYPE_REGULAR;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint exercise_idx, const uint new_type)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData.at(exercise_idx)->type[set_number] = QString::number(new_type);
		setModified(true);
	}
}

void DBTrainingDayModel::changeSetType(const uint set_number, const uint exercise_idx, const uint old_type, const uint new_type)
{
	const QString reps(m_ExerciseData.at(exercise_idx)->reps.at(set_number));
	const QString weight(m_ExerciseData.at(exercise_idx)->weight.at(set_number));
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
		case SET_TYPE_DROP:
		case SET_TYPE_GIANT:
			if (new_type != SET_TYPE_DROP && new_type != SET_TYPE_GIANT)
			{
				m_ExerciseData.at(exercise_idx)->reps[set_number] = reps.left(reps.indexOf(subrecord_separator));
				m_ExerciseData.at(exercise_idx)->weight[set_number] = weight.left(weight.indexOf(subrecord_separator));
				if (new_type == SET_TYPE_CLUSTER)
					m_ExerciseData.at(exercise_idx)->subsets[set_number]= u"4"_qs;
			}
		break;
	}
	setSetType(set_number, exercise_idx, new_type);
}

QString DBTrainingDayModel::setRestTime(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
			return m_ExerciseData.at(exercise_idx)->resttime.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetRestTime(const uint set_number, const uint exercise_idx, const QString& new_time)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData.at(exercise_idx)->resttime[set_number] = new_time;
		setModified(true);
	}
}

QString DBTrainingDayModel::setSubSets(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
			return m_ExerciseData.at(exercise_idx)->subsets.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::newSetSubSet(const uint set_number, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
	{
		m_ExerciseData.at(exercise_idx)->subsets[set_number] = QString::number(m_ExerciseData.at(exercise_idx)->subsets.at(set_number).toUInt() + 1);
		m_ExerciseData.at(exercise_idx)->reps[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->reps.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 2) + subrecord_separator);
		m_ExerciseData.at(exercise_idx)->weight[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->weight.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 10) + subrecord_separator);
		setModified(true);
	}
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const uint exercise_idx, const QString& new_subsets)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
	{
		m_ExerciseData.at(exercise_idx)->subsets[set_number] = new_subsets;
		setModified(true);
	}
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
			return m_ExerciseData.at(exercise_idx)->reps.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint exercise_idx, const QString& new_reps)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData.at(exercise_idx)->reps[set_number] = new_reps;
		setModified(true);
	}
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
			return m_ExerciseData.at(exercise_idx)->weight.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint exercise_idx, const QString& new_weight)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData.at(exercise_idx)->weight[set_number] = new_weight;
		setModified(true);
	}
}

QString DBTrainingDayModel::setNotes(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->notes.count())
			return m_ExerciseData.at(exercise_idx)->notes.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetNotes(const uint set_number, const QString& new_notes, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->notes.count())
	{
		m_ExerciseData.at(exercise_idx)->notes[set_number] = new_notes;
		setModified(true);
	}
}

bool DBTrainingDayModel::setCompleted(const uint set_number, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->notes.count())
			return m_ExerciseData.at(exercise_idx)->completed.at(set_number) == u"1"_qs;
	}
	return false;
}

void DBTrainingDayModel::setSetCompleted(const uint set_number, const uint exercise_idx, const bool completed)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->notes.count())
	{
		m_ExerciseData.at(exercise_idx)->completed[set_number] = completed ? u"1"_qs : u"0"_qs;
		emit exerciseCompleted(exercise_idx, allSetsCompleted(exercise_idx));
		if (completed)
		{
			if (modified())
				emit saveWorkout();
		}
	}
}

bool DBTrainingDayModel::allSetsCompleted(const uint exercise_idx) const
{
	const uint nsets(m_ExerciseData.at(exercise_idx)->nsets);
	if (nsets > 0)
	{
		for (uint i(0); i < nsets; ++i)
		{
			if (m_ExerciseData.at(exercise_idx)->completed.at(i) == u"0"_qs)
				return false;
		}
		return true;
	}
	return false;
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint subset, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
		{
			const QStringList subSetReps(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
			if (subset < subSetReps.count())
				return subSetReps.at(subset);
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_reps)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
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
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint subset, const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
		{
			const QStringList subSetWeight(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
			if (subset < subSetWeight.count())
				return subSetWeight.at(subset);
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_weight)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
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
}
