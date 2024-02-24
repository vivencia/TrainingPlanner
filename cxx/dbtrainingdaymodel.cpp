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
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	QString name(m_workingExercise.key());
	return idx != -1 ? name.replace(subrecord_separator, QStringLiteral(" + ")) : m_workingExercise.key();
}

QString DBTrainingDayModel::exerciseName1() const
{
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	return idx != -1 ? m_workingExercise.key().left(idx -1) : m_workingExercise.key();
}

QString DBTrainingDayModel::exerciseName2() const
{
	const int idx(m_workingExercise.key().indexOf(subrecord_separator));
	return idx != -1 ? m_workingExercise.key().mid(idx+1) : QString();
}

QString DBTrainingDayModel::setType(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(0).split(record_separator));
	return set_number < sets_info.count() ? sets_info.at(set_number) : QString();
}

QString DBTrainingDayModel::setRestTime(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(1).split(record_separator));
	return set_number < sets_info.count() ? sets_info.at(set_number) : QString();
}

QString DBTrainingDayModel::setSubSets(const uint set_number) const
{
	const QStringList sets_info(m_workingExercise.value().at(2).split(record_separator));
	return set_number < sets_info.count() ? sets_info.at(set_number) : QString();
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
