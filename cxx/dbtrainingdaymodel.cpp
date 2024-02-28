#include "dbtrainingdaymodel.h"

void DBTrainingDayModel::appendExercisesList(const QStringList& list)
{
	const QStringList exercises_names(list.at(0).split(record_separator, Qt::SkipEmptyParts));
	const QStringList setstypes(list.at(1).split(record_separator, Qt::SkipEmptyParts));
	const QStringList resttimes(list.at(2).split(record_separator, Qt::SkipEmptyParts));
	const QStringList subsets(list.at(3).split(record_separator, Qt::SkipEmptyParts));
	const QStringList reps(list.at(4).split(record_separator, Qt::SkipEmptyParts));
	const QStringList weights(list.at(5).split(record_separator, Qt::SkipEmptyParts));
	const QStringList notes(list.at(6).split(record_separator, Qt::SkipEmptyParts));

	const uint n_sets(setstypes.count());
	QStringList setsInfo;
	QStringList::const_iterator itr(exercises_names.constBegin());
	const QStringList::const_iterator itr_end(exercises_names.constEnd());

	while (itr != itr_end)
	{
		for(uint i(0); i < n_sets; ++i)
		{
			setsInfo << setstypes.at(i) << resttimes.at(i) << subsets.at(i) << reps.at(i) << weights.at(i) << notes.at(i);
			m_ExerciseData.insert(*itr, setsInfo);
			setsInfo.clear();
			++itr;
		}
	}
}

void DBTrainingDayModel::getSaveInfo(QStringList& data) const
{
	QHash<QString,QStringList>::const_iterator itr(m_ExerciseData.constBegin());
	const QHash<QString,QStringList>::const_iterator itr_end(m_ExerciseData.constEnd());
	while (itr != itr_end)
	{
		data[0].append(itr.key() + record_separator);
		data[1].append((*itr).at(0));
		data[2].append((*itr).at(1));
		data[3].append((*itr).at(2));
		data[4].append((*itr).at(3));
		data[5].append((*itr).at(4));
		data[6].append((*itr).at(5));
		++itr;
	}
}

QString DBTrainingDayModel::exerciseName() const
{
	if (m_ExerciseData.count() > 0)
	{
		const int idx(m_workingExercise.key().indexOf(subrecord_separator));
		QString name(m_workingExercise.key());
		return idx != -1 ? name.replace(subrecord_separator, QStringLiteral(" + ")) : m_workingExercise.key();
	}
	return QString();
}

void DBTrainingDayModel::setExerciseName(const QString& name)
{
	const QString key(m_workingExercise.key());
	m_workingExercise = m_ExerciseData.insert(name, m_workingExercise.value());
	m_ExerciseData.remove(key);
}

void DBTrainingDayModel::newExerciseName(const QString& new_exercise)
{
	m_workingExercise = m_ExerciseData.insert(new_exercise, QStringList());
}

QString DBTrainingDayModel::exerciseName1() const
{
	if (m_ExerciseData.count() > 0)
	{
		const int idx(m_workingExercise.key().indexOf(subrecord_separator));
		return idx != -1 ? QStringLiteral("1: ") + m_workingExercise.key().left(idx -1) : m_workingExercise.key();
	}
	return QString();
}

void DBTrainingDayModel::setExerciseName1(const QString& name1)
{
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	QString new_name1;
	if (idx != -1)
		new_name1 = name1 + subrecord_separator + m_workingExercise.key().mid(idx+1);
	else
		new_name1 = name1;
	setExerciseName(name1);
}

QString DBTrainingDayModel::exerciseName2() const
{
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	return idx != -1 ? QStringLiteral("2: ") + m_workingExercise.key().mid(idx+1) : QString();
}

void DBTrainingDayModel::setExerciseName2(const QString& name2)
{
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	QString new_name2;
	if (idx != -1)
		new_name2 = m_workingExercise.key().left(idx) + subrecord_separator + name2;
	else
		new_name2 = subrecord_separator + name2;
	setExerciseName(new_name2);
}

void DBTrainingDayModel::newSet(const uint set_number, const uint type, const QString& resttime,
					const QString& subsets, const QString& reps, const QString& weight, const QString& notes)
{
	const uint n_sets(setsNumber());
	const QString str_type(QString::number(type));

	if (set_number >= n_sets)
	{
		const uint n(set_number-n_sets+1);
		QHash<QString,QStringList>::iterator workingExercise(m_ExerciseData.find(m_workingExercise.key()));
		for(uint i(0); i < n; ++i)
		{
			(*workingExercise)[0].append(str_type + record_separator);
			(*workingExercise)[1].append(resttime + record_separator);
			(*workingExercise)[2].append(subsets + record_separator);
			(*workingExercise)[3].append(reps + record_separator);
			(*workingExercise)[4].append(weight + record_separator);
			(*workingExercise)[5].append(notes + record_separator);
		}
	}
}

bool DBTrainingDayModel::removeSet(const uint set_number)
{
	if (set_number < setsNumber())
	{
		(*m_workingExercise)[0].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		(*m_workingExercise)[1].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		(*m_workingExercise)[2].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		(*m_workingExercise)[3].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		(*m_workingExercise)[4].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		(*m_workingExercise)[5].split(record_separator, Qt::SkipEmptyParts).remove(set_number);
		return true;
	}
	return false;
}

uint DBTrainingDayModel::setType(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
		return !(*m_workingExercise).at(0).isEmpty() ?
			(*m_workingExercise).at(0).split(record_separator, Qt::SkipEmptyParts).at(set_number).toUInt() : 0;
	return 0;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint new_type)
{
	QStringList setTypeList(m_workingExercise.value().at(0).split(record_separator));
	setTypeList[set_number] = QString::number(new_type);
	m_ExerciseData[m_workingExercise.key()][0] = setTypeList.join(record_separator);
}

QString DBTrainingDayModel::setRestTime(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
		return !(*m_workingExercise).at(1).isEmpty() ?
			(*m_workingExercise).at(1).split(record_separator, Qt::SkipEmptyParts).at(set_number) : QString();
	return QString();
}

void DBTrainingDayModel::setSetRestTime(const uint set_number, const QString& new_time)
{
	QStringList setRestTimeList(m_workingExercise.value().at(1).split(record_separator));
	setRestTimeList[set_number] = new_time;
	m_ExerciseData[m_workingExercise.key()][1] = setRestTimeList.join(record_separator);
}

QString DBTrainingDayModel::setSubSets(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
		return !(*m_workingExercise).at(2).isEmpty() ?
			(*m_workingExercise).at(2).split(record_separator, Qt::SkipEmptyParts).at(set_number) : QString();
	return QString();
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const QString& new_subsets)
{
	QStringList setSubSetList(m_workingExercise.value().at(2).split(record_separator));
	setSubSetList[set_number] = new_subsets;
	m_ExerciseData[m_workingExercise.key()][2] = setSubSetList.join(record_separator);
}

QString DBTrainingDayModel::setReps(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
	{
		if (!(*m_workingExercise).at(3).isEmpty())
		{
			QString reps( (*m_workingExercise).at(3).split(record_separator, Qt::SkipEmptyParts).at(set_number) );
			const int idx(reps.indexOf(subrecord_separator));
			return idx != -1 ? reps.left(idx-1) : reps;
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const QString& new_reps)
{
	QStringList setRepsList(m_workingExercise.value().at(3).split(record_separator));
	setRepsList[set_number] = new_reps;
	m_ExerciseData[m_workingExercise.key()][3] = setRepsList.join(record_separator);
}

QString DBTrainingDayModel::setWeight(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
	{
		if (!(*m_workingExercise).at(4).isEmpty())
		{
			QString reps( (*m_workingExercise).at(4).split(record_separator, Qt::SkipEmptyParts).at(set_number) );
			const int idx(reps.indexOf(subrecord_separator));
			return idx != -1 ? reps.left(idx-1) : reps;
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const QString& new_weight)
{
	QStringList setWeightList(m_workingExercise.value().at(4).split(record_separator));
	setWeightList[set_number] = new_weight;
	m_ExerciseData[m_workingExercise.key()][4] = setWeightList.join(record_separator);
}

QString DBTrainingDayModel::setNotes(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
		return !(*m_workingExercise).at(5).isEmpty() ?
			(*m_workingExercise).at(5).split(record_separator, Qt::SkipEmptyParts).at(set_number) : QString();
	return QString();
}

void DBTrainingDayModel::setSetNotes(const uint set_number, const QString& new_notes)
{
	QStringList setNotesList(m_workingExercise.value().at(5).split(record_separator));
	setNotesList[set_number] = new_notes;
	m_ExerciseData[m_workingExercise.key()][5] = setNotesList.join(record_separator);
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint subset) const
{
	if ((*m_workingExercise).count() > 0)
	{
		if (!(*m_workingExercise).at(3).isEmpty())
		{
			const QString reps( (*m_workingExercise).at(3).split(record_separator, Qt::SkipEmptyParts).at(set_number) );
			const QStringList subset_info(reps.split(subrecord_separator, Qt::SkipEmptyParts));
			if (subset < subset_info.count())
				return subset_info.at(subset);
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint subset, const QString& new_reps)
{
	QStringList setRepsList(m_workingExercise.value().at(3).split(record_separator));
	if (set_number < setRepsList.count())
	{
		QStringList subreps(setRepsList.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
		if (subset < subreps.count())
			subreps[subset] = new_reps;
		setRepsList[set_number] = subreps.join(subrecord_separator);
		m_ExerciseData[m_workingExercise.key()][3] = setRepsList.join(record_separator);
	}
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint subset) const
{
	if ((*m_workingExercise).count() > 0)
	{
		if (!(*m_workingExercise).at(4).isEmpty())
		{
			const QString reps( (*m_workingExercise).at(4).split(record_separator, Qt::SkipEmptyParts).at(set_number) );
			const QStringList subset_info(reps.split(subrecord_separator, Qt::SkipEmptyParts));
			if (subset < subset_info.count())
				return subset_info.at(subset);
		}
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint subset, const QString& new_weight)
{
	QStringList setWeightsList(m_workingExercise.value().at(4).split(record_separator));
	if (set_number < setWeightsList.count())
	{
		QStringList subweights(setWeightsList.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
		if (subset < subweights.count())
			subweights[subset] = new_weight;
		setWeightsList[set_number] = subweights.join(subrecord_separator);
		m_ExerciseData[m_workingExercise.key()][4] = setWeightsList.join(record_separator);
	}
}
