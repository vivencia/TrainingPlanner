#include "dbtrainingdaymodel.h"

void DBTrainingDayModel::fromDataBase(const QStringList& list)
{
	const QStringList exercises_names(list.at(0).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList setstypes(list.at(1).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList resttimes(list.at(2).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList subsets(list.at(3).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList reps(list.at(4).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList weights(list.at(5).split(record_separator2, Qt::SkipEmptyParts));
	const QStringList notes(list.at(6).split(record_separator2, Qt::SkipEmptyParts));

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
		m_ExerciseData[i]->nsets = m_ExerciseData.at(i)->type.count();
	}
}

void DBTrainingDayModel::getSaveInfo(QStringList& data) const
{
	for(uint i(0); i < m_ExerciseData.count(); ++i)
	{
		data[0].append(m_ExerciseData.at(i)->name + record_separator2);
		for(uint x(0); x < m_ExerciseData.at(i)->nsets; ++x)
		{
			data[1].append(m_ExerciseData.at(i)->type.at(x) + record_separator);
			data[2].append(m_ExerciseData.at(i)->resttime.at(x) + record_separator);
			data[3].append(m_ExerciseData.at(i)->subsets.at(x) + record_separator);
			data[4].append(m_ExerciseData.at(i)->reps.at(x) + record_separator);
			data[5].append(m_ExerciseData.at(i)->weight.at(x) + record_separator);
			data[6].append(m_ExerciseData.at(i)->notes.at(x) + record_separator);
		}
		data[1].append(record_separator2);
		data[2].append(record_separator2);
		data[3].append(record_separator2);
		data[4].append(record_separator2);
		data[5].append(record_separator2);
		data[6].append(record_separator2);
	}
}

QString DBTrainingDayModel::exerciseName(const uint exercise_idx) const
{
	return (exercise_idx < m_ExerciseData.count()) ? m_ExerciseData.at(exercise_idx)->name : QString();
}

void DBTrainingDayModel::setExerciseName(const QString& new_name, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
		m_ExerciseData[exercise_idx]->name = new_name;
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
	setTDayModified(true);
}

QString DBTrainingDayModel::exerciseName1(const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
		return idx != -1 ? QStringLiteral("1: ") + m_ExerciseData.at(exercise_idx)->name.left(idx-1) : m_ExerciseData.at(exercise_idx)->name;
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
			new_name1 = name1 + subrecord_separator + m_ExerciseData.at(exercise_idx)->name.mid(idx+1);
		else
			new_name1 = name1;
		m_ExerciseData[exercise_idx]->name = new_name1;
		setTDayModified(true);
	}
}

QString DBTrainingDayModel::exerciseName2(const uint exercise_idx) const
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const int idx(m_ExerciseData.at(exercise_idx)->name.indexOf(subrecord_separator));
		return idx != -1 ? QStringLiteral("2: ") + m_ExerciseData.at(exercise_idx)->name.mid(idx+1) : QString();
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
		m_ExerciseData[exercise_idx]->name = new_name2;
		setTDayModified(true);
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

void DBTrainingDayModel::newSet(const uint exercise_idx, const uint set_number, const uint type)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const uint total(m_ExerciseData.at(exercise_idx)->type.count());
		const int n(set_number - total + 1);
		const QString strType(QString::number(type));
		if (n >= 1)
		{
			m_ExerciseData[exercise_idx]->nsets += n;
			setTDayModified(true);

			for(uint i(0); i < n; ++i)
			{
				m_ExerciseData[exercise_idx]->notes.append(!m_ExerciseData.at(exercise_idx)->notes.isEmpty() ? m_ExerciseData.at(exercise_idx)->notes.last() : QString());
				m_ExerciseData[exercise_idx]->type.append(strType);
				switch (type) {
					case 0: //Regular
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
							m_ExerciseData[exercise_idx]->reps.append(m_ExerciseData.at(exercise_idx)->reps.last());
							m_ExerciseData[exercise_idx]->weight.append(m_ExerciseData.at(exercise_idx)->weight.last());
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("12"));
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("30"));
						}
						m_ExerciseData[exercise_idx]->subsets.append(QString());
					break;
					case 1: //Pyramid
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
							m_ExerciseData[exercise_idx]->reps.append(QString::number(m_ExerciseData.at(exercise_idx)->reps.last().toUInt() - 3));
							m_ExerciseData[exercise_idx]->weight.append(QString::number(m_ExerciseData.at(exercise_idx)->weight.last().toUInt() * 1.2));
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("15"));
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("20"));
						}
						m_ExerciseData[exercise_idx]->subsets.append(QString());
					break;
					case 2: //DropSet
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
							m_ExerciseData[exercise_idx]->subsets.append(m_ExerciseData.at(exercise_idx)->subsets.last());
							m_ExerciseData[exercise_idx]->reps.append(m_ExerciseData.at(exercise_idx)->reps.last());
							m_ExerciseData[exercise_idx]->weight.append(m_ExerciseData.at(exercise_idx)->weight.last());
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
							m_ExerciseData[exercise_idx]->subsets.append(QStringLiteral("3"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("15") + subrecord_separator + QStringLiteral("12")
												+ subrecord_separator + QStringLiteral("10") + subrecord_separator);
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("40") + subrecord_separator + QStringLiteral("30")
												+ subrecord_separator + QStringLiteral("20") + subrecord_separator);
						}
					break;
					case 3: //ClusterSet
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 0));
							m_ExerciseData[exercise_idx]->subsets.append(m_ExerciseData.at(exercise_idx)->subsets.last());
							m_ExerciseData[exercise_idx]->reps.append(m_ExerciseData.at(exercise_idx)->reps.last());
							m_ExerciseData[exercise_idx]->weight.append(m_ExerciseData.at(exercise_idx)->weight.last());
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("02:00"));
							m_ExerciseData[exercise_idx]->subsets.append(QStringLiteral("4"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("6"));
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("40"));
						}
					break;
					case 4: //GiantSet
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
							m_ExerciseData[exercise_idx]->reps.append(m_ExerciseData.at(exercise_idx)->reps.last());
							m_ExerciseData[exercise_idx]->weight.append(m_ExerciseData.at(exercise_idx)->weight.last());
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("15") + subrecord_separator + QStringLiteral("15") + subrecord_separator);
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("30") + subrecord_separator + QStringLiteral("30") + subrecord_separator);
						}
						m_ExerciseData[exercise_idx]->subsets.append(QString());
					break;
					case 5: //MyoReps
						if (!m_ExerciseData.at(exercise_idx)->reps.isEmpty())
						{
							m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 30));
							m_ExerciseData[exercise_idx]->subsets.append(QString::number(m_ExerciseData.at(exercise_idx)->subsets.last().toUInt() + 1));
							m_ExerciseData[exercise_idx]->reps.append(m_ExerciseData.at(exercise_idx)->reps.last());
							m_ExerciseData[exercise_idx]->weight.append(m_ExerciseData.at(exercise_idx)->weight.last());
						}
						else
						{
							m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("02:30"));
							m_ExerciseData[exercise_idx]->subsets.append(QStringLiteral("0"));
							m_ExerciseData[exercise_idx]->reps.append(QStringLiteral("15"));
							m_ExerciseData[exercise_idx]->weight.append(QStringLiteral("100"));
						}
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
			m_ExerciseData[exercise_idx]->type.remove(set_number);
			m_ExerciseData[exercise_idx]->resttime.remove(set_number);
			m_ExerciseData[exercise_idx]->subsets.remove(set_number);
			m_ExerciseData[exercise_idx]->reps.remove(set_number);
			m_ExerciseData[exercise_idx]->weight.remove(set_number);
			m_ExerciseData[exercise_idx]->notes.remove(set_number);
			m_ExerciseData[exercise_idx]->nsets--;
			setTDayModified(true);
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
	return 0;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint new_type, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->type[set_number] = QString::number(new_type);
		setTDayModified(true);
	}
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

void DBTrainingDayModel::setSetRestTime(const uint set_number, const QString& new_time, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->resttime[set_number] = new_time;
		setTDayModified(true);
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

void DBTrainingDayModel::newSetSubSet(const uint exercise_idx, const uint set_number)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
	{
		m_ExerciseData[exercise_idx]->subsets[set_number] = QString::number(m_ExerciseData.at(exercise_idx)->subsets.at(set_number).toUInt() + 1);
		m_ExerciseData[exercise_idx]->reps[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->reps.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 2) + subrecord_separator);
		m_ExerciseData[exercise_idx]->weight[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->weight.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 10) + subrecord_separator);
		setTDayModified(true);
	}
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const QString& new_subsets, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
	{
		m_ExerciseData.at(exercise_idx)->subsets[set_number] = new_subsets;
		setTDayModified(true);
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

void DBTrainingDayModel::setSetReps(const uint set_number, const QString& new_reps, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->reps[set_number] = new_reps;
		setTDayModified(true);
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

void DBTrainingDayModel::setSetWeight(const uint set_number, const QString& new_weight, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->weight[set_number] = new_weight;
		setTDayModified(true);
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
		m_ExerciseData[exercise_idx]->notes[set_number] = new_notes;
		setTDayModified(true);
	}
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

void DBTrainingDayModel::setSetReps(const uint set_number, const uint subset, const QString& new_reps, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		QStringList subSetReps(m_ExerciseData.at(exercise_idx)->reps.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
		const uint total(subSetReps.count());
		const int n(subset - total);
		if (n >= 0)
		{
			for(uint i(0); i <= n; ++i)
				subSetReps.append(new_reps + subrecord_separator);
		}
		subSetReps[subset] = new_reps;
		m_ExerciseData[exercise_idx]->reps[set_number] = subSetReps.join(subrecord_separator);
		setTDayModified(true);
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

void DBTrainingDayModel::setSetWeight(const uint set_number, const uint subset, const QString& new_weight, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		QStringList subSetWeight(m_ExerciseData.at(exercise_idx)->weight.at(set_number).split(subrecord_separator, Qt::SkipEmptyParts));
		const uint total(subSetWeight.count());
		const int n(subset - total);
		if (n >= 0)
		{
			for(uint i(0); i <= n; ++i)
				subSetWeight.append(new_weight + subrecord_separator);
		}
		subSetWeight[subset] = new_weight;
		m_ExerciseData[exercise_idx]->weight[set_number] = subSetWeight.join(subrecord_separator);
		setTDayModified(true);
	}
}
