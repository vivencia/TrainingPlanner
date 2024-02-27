#include "dbtrainingdaymodel.h"

void DBTrainingDayModel::appendExercisesList(const QStringList& list)
{
	const QStringList exercises_names(list.at(0).split(record_separator));
	const QStringList setstypes(list.at(1).split(record_separator));
	const QStringList resttimes(list.at(2).split(record_separator));
	const QStringList subsets(list.at(3).split(record_separator));
	const QStringList reps(list.at(4).split(record_separator));
	const QStringList weights(list.at(5).split(record_separator));

	const uint n_sets(setstypes.count());
	QStringList setsInfo;
	QStringList::const_iterator itr(exercises_names.constBegin());
	const QStringList::const_iterator itr_end(exercises_names.constEnd());

	while (itr != itr_end)
	{
		for(uint i(0); i < n_sets; ++i)
		{
			setsInfo << setstypes.at(i) << resttimes.at(i) << subsets.at(i) << reps.at(i) << weights.at(i);
			m_ExerciseData.insert(*itr, setsInfo);
			setsInfo.clear();
			++itr;
		}
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

uint DBTrainingDayModel::setType(const uint set_number) const
{
	if ((*m_workingExercise).count() > 0)
	{
		const QStringList sets_info((*m_workingExercise).at(0).split(record_separator));
		return set_number < sets_info.count() ? sets_info.at(set_number).toUInt() : 0;
	}
	return 0;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint new_type)
{
	QStringList setTypeList(m_workingExercise.value().at(0).split(record_separator));
	const QString strNewType(QString::number(new_type));
	if (set_number >= setTypeList.count())
	{
		if (set_number > setTypeList.count())
		{
			const uint n(set_number-setTypeList.count());
			for(uint i(0); i < n; ++i)
				setTypeList.append(strNewType);
		}
		setTypeList.append(QString::number(new_type));
	}
	else
		setTypeList[set_number] = strNewType;
	m_ExerciseData[m_workingExercise.key()].replace(0, setTypeList.join(record_separator));
}

QString DBTrainingDayModel::setRestTime(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(1).split(record_separator));
	return set_number < sets_info.count() ? sets_info.at(set_number) : QString();
}

void DBTrainingDayModel::setSetRestTime(const uint set_number, const QString& new_time)
{
	QStringList setRestTimeList(m_workingExercise.value().at(1).split(record_separator));
	if (set_number >= setRestTimeList.count())
	{
		if (set_number > setRestTimeList.count())
		{
			const uint n(set_number-setRestTimeList.count());
			for(uint i(0); i < n; ++i)
				setRestTimeList.append(new_time);
		}
		setRestTimeList.append(new_time);
	}
	else
		setRestTimeList[set_number] = new_time;
	m_ExerciseData[m_workingExercise.key()].replace(1, setRestTimeList.join(record_separator));
}

QString DBTrainingDayModel::setSubSets(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(2).split(record_separator));
	return set_number < sets_info.count() ? sets_info.at(set_number) : QString();
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const QString& new_subsets)
{
	QStringList setSubSetsList(m_workingExercise.value().at(2).split(record_separator));
	if (set_number >= setSubSetsList.count())
	{
		if (set_number > setSubSetsList.count())
		{
			const uint n(set_number-setSubSetsList.count());
			for(uint i(0); i < n; ++i)
				setSubSetsList.append(new_subsets);
		}
		setSubSetsList.append(new_subsets);
	}
	else
		setSubSetsList[set_number] = new_subsets;
	m_ExerciseData[m_workingExercise.key()].replace(2, setSubSetsList.join(record_separator));
}

QString DBTrainingDayModel::setReps(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(3).split(record_separator));
	if (set_number < sets_info.count())
	{
		const int idx(sets_info.at(set_number).indexOf(subrecord_separator));
		return idx != -1 ? sets_info.at(set_number).left(idx-1) : sets_info.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const QString& new_reps)
{
	QStringList setRepsList(m_workingExercise.value().at(3).split(record_separator));
	if (set_number >= setRepsList.count())
	{
		if (set_number > setRepsList.count())
		{
			const uint n(set_number-setRepsList.count());
			for(uint i(0); i < n; ++i)
				setRepsList.append(new_reps);
		}
		setRepsList.append(new_reps);
	}
	else
		setRepsList[set_number] = new_reps;
	m_ExerciseData[m_workingExercise.key()].replace(3, setRepsList.join(record_separator));
}

QString DBTrainingDayModel::setWeight(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(4).split(record_separator));
	if (set_number < sets_info.count())
	{
		const int idx(sets_info.at(set_number).indexOf(subrecord_separator));
		return idx != -1 ? sets_info.at(set_number).left(idx-1) : sets_info.at(set_number);
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const QString& new_weight)
{
	QStringList setWeightList(m_workingExercise.value().at(4).split(record_separator));
	if (set_number >= setWeightList.count())
	{
		if (set_number > setWeightList.count())
		{
			const uint n(set_number-setWeightList.count());
			for(uint i(0); i < n; ++i)
				setWeightList.append(new_weight);
		}
		setWeightList.append(new_weight);
	}
	else
		setWeightList[set_number] = new_weight;
	m_ExerciseData[m_workingExercise.key()].replace(4, setWeightList.join(record_separator));
}

QString DBTrainingDayModel::setReps(const uint set_number, const uint subset) const
{
	const QStringList sets_info(m_workingExercise.value().at(3).split(record_separator));
	if (set_number < sets_info.count())
	{
		const QStringList subset_info(sets_info.at(set_number).split(subrecord_separator));
		if (subset < subset_info.count())
			return subset_info.at(subset);
	}
	return QString();
}

void DBTrainingDayModel::setSetReps(const uint set_number, const uint subset, const QString& new_reps)
{
	QStringList setRepsList(m_workingExercise.value().at(3).split(record_separator));
	if (set_number >= setRepsList.count())
	{
		const uint nsets(setRepsList.count());
		const QString subSetsReps(fillSubSets(setRepsList.at(nsets-1), subset, new_reps));

		if (set_number > setRepsList.count())
		{
			const uint n(set_number-setRepsList.count());
			for(uint i(0); i < n; ++i)
				setRepsList.append(subSetsReps);
		}
		setRepsList.append(subSetsReps);
	}
	else
		setRepsList[set_number] = fillSubSets(setRepsList.at(set_number), subset, new_reps);
	m_ExerciseData[m_workingExercise.key()].replace(3, setRepsList.join(record_separator));
}

QString DBTrainingDayModel::setWeight(const uint set_number, const uint subset) const
{
	const QStringList sets_info(m_workingExercise.value().at(4).split(record_separator));
	if (set_number < sets_info.count())
	{
		const QStringList subset_info(sets_info.at(set_number).split(subrecord_separator));
		if (subset < subset_info.count())
			return subset_info.at(subset);
	}
	return QString();
}

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint subset, const QString& new_weight)
{
	QStringList setWeightList(m_workingExercise.value().at(4).split(record_separator));
	if (set_number >= setWeightList.count())
	{
		const uint nsets(setWeightList.count());
		const QString subSetsWeights(fillSubSets(setWeightList.at(nsets-1), subset, new_weight));

		if (set_number > setWeightList.count())
		{
			const uint n(set_number-setWeightList.count());
			for(uint i(0); i < n; ++i)
				setWeightList.append(subSetsWeights);
		}
		setWeightList.append(subSetsWeights);
	}
	else
		setWeightList[set_number] = fillSubSets(setWeightList.at(set_number), subset, new_weight);
	m_ExerciseData[m_workingExercise.key()].replace(4, setWeightList.join(record_separator));
}

const QString DBTrainingDayModel::fillSubSets(const QString& subSetInfo, const uint subset, const QString& value)
{
	QStringList setSubSetList(subSetInfo.split(subrecord_separator));
	if (subset > setSubSetList.count())
	{
		const uint n_sub(subset-setSubSetList.count()+1);
		for(uint i(0); i < n_sub; ++i)
			setSubSetList.append(value);
	}
	setSubSetList[subset] = value;
	return setSubSetList.join(subrecord_separator);
}
