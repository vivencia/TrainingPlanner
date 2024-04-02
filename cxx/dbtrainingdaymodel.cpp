#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"

#include <QtMath>

static QString multiUseString;

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
			data[3].append((x < m_ExerciseData.at(i)->subsets.count() ? m_ExerciseData.at(i)->subsets.at(x) : QString()) + record_separator);
			data[4].append(m_ExerciseData.at(i)->reps.at(x) + record_separator);
			data[5].append(m_ExerciseData.at(i)->weight.at(x) + record_separator);
			data[6].append((x < m_ExerciseData.at(i)->notes.count() ? m_ExerciseData.at(i)->notes.at(x) : QString()) + record_separator);
		}
		data[1].append(record_separator2);
		data[2].append(record_separator2);
		data[3].append(record_separator2);
		data[4].append(record_separator2);
		data[5].append(record_separator2);
		data[6].append(record_separator2);
	}
}

void DBTrainingDayModel::convertMesoModelToTDayModel(DBMesoSplitModel* splitModel)
{
	for(uint i(0); i < splitModel->count(); ++i)
	{
		m_ExerciseData.append(new exerciseEntry);
		m_ExerciseData[i]->name = splitModel->getRow_const(i).at(0);

		const uint type(splitModel->getRow_const(i).at(1).toUInt());
		newFirstSet(i, type, splitModel->getRow_const(i).at(3), splitModel->getRow_const(i).at(4));
		newSet(i, splitModel->getRow_const(i).at(2).toUInt() - 1, type);
	}
	setModified(true);
}

QString DBTrainingDayModel::exerciseName(const uint exercise_idx) const
{
	QString name;
	if (exercise_idx < m_ExerciseData.count())
	{
		name = m_ExerciseData.at(exercise_idx)->name;
		return name.replace(subrecord_separator, QStringLiteral(" + "));
	}
	return name;
}

void DBTrainingDayModel::setExerciseName(const QString& new_name, const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		m_ExerciseData[exercise_idx]->name = new_name;
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
	setModified(true);
}

void DBTrainingDayModel::removeExercise(const uint exercise_idx)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		delete m_ExerciseData[exercise_idx];
		m_ExerciseData.remove(exercise_idx);
		setModified(true);
	}
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
		m_ExerciseData[exercise_idx]->name = new_name1;
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
		m_ExerciseData[exercise_idx]->name = (idx != -1 ? m_ExerciseData.at(exercise_idx)->name.left(idx) :
									m_ExerciseData.at(exercise_idx)->name) + subrecord_separator + name2;
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

void DBTrainingDayModel::newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const QString strType(QString::number(type));
		m_ExerciseData[exercise_idx]->nsets = 1;
		setModified(true);
		m_ExerciseData[exercise_idx]->notes.append(u" "_qs);
		m_ExerciseData[exercise_idx]->type.append(strType);

		switch (type) {
			case 0: //Regular
			case 1: //Pyramid
			case 6: //Reverse Pyramid
				m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
				m_ExerciseData[exercise_idx]->reps.append(nReps);
				m_ExerciseData[exercise_idx]->weight.append(nWeight);
				m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
			break;
			case 2: //DropSet
			{
				m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
				m_ExerciseData[exercise_idx]->subsets.append(QStringLiteral("3"));
				const uint nreps(nReps.toUInt());
				m_ExerciseData[exercise_idx]->reps.append(nReps + subrecord_separator + QString::number(nreps - 3) +
												subrecord_separator + QString::number(nreps - 6) + subrecord_separator);
				const uint nweight(nWeight.toUInt());
				m_ExerciseData[exercise_idx]->weight.append(nWeight + subrecord_separator + QString::number(nweight - 10) +
												subrecord_separator + QString::number(nweight - 20) + subrecord_separator);
			}
			break;
			case 3: //ClusterSet
				m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("02:00"));
				m_ExerciseData[exercise_idx]->subsets.append(QStringLiteral("4"));
				m_ExerciseData[exercise_idx]->reps.append(nReps);
				m_ExerciseData[exercise_idx]->weight.append(nWeight);
			break;
			case 4: //GiantSet
				m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("01:30"));
				m_ExerciseData[exercise_idx]->reps.append(nReps + subrecord_separator + nReps + subrecord_separator);
				m_ExerciseData[exercise_idx]->weight.append(nWeight + subrecord_separator + nWeight + subrecord_separator);
				m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
			break;
			case 5: //MyoReps
				m_ExerciseData[exercise_idx]->resttime.append(QStringLiteral("02:30"));
				m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
				m_ExerciseData[exercise_idx]->reps.append(nReps);
				m_ExerciseData[exercise_idx]->weight.append(nWeight);
			break;
		}
	}
}

const QString& DBTrainingDayModel::nextSetSuggestedReps(const uint exercise_idx, const uint type) const
{
	if (type == 1 || type == 6)
	{
		uint lastSetValue(m_ExerciseData.at(exercise_idx)->reps.last().toUInt());
		if (type == 1) //Pyramid
			lastSetValue -= 3;
		else //Reverse Pyramid
			lastSetValue += 5;
		multiUseString = QString::number(lastSetValue);
		return multiUseString;
	}
	else
		return m_ExerciseData.at(exercise_idx)->reps.last();
}

const QString& DBTrainingDayModel::nextSetSuggestedWeight(const uint exercise_idx, const uint type) const
{
	if (type == 1 || type == 6)
	{
		uint lastSetValue(m_ExerciseData.at(exercise_idx)->weight.last().toUInt());
		if (type == 1) //Pyramid
			lastSetValue = qFloor(lastSetValue * 1.2);
		else //Reverse Pyramid
			lastSetValue = qFloor(lastSetValue * 0.2);
		multiUseString = QString::number(lastSetValue);
		return multiUseString;
	}
	else
		return m_ExerciseData.at(exercise_idx)->reps.last();
}

void DBTrainingDayModel::newSet(const uint exercise_idx, const uint set_number, const uint type)
{
	if (exercise_idx < m_ExerciseData.count())
	{
		const uint total(m_ExerciseData.at(exercise_idx)->nsets);
		const int n(set_number - total + 1);
		const QString strType(QString::number(type));
		if (n >= 1)
		{
			m_ExerciseData[exercise_idx]->nsets += n;
			setModified(true);

			for(uint i(0); i < n; ++i)
			{
				m_ExerciseData[exercise_idx]->notes.append(m_ExerciseData.at(exercise_idx)->notes.last());
				m_ExerciseData[exercise_idx]->type.append(strType);
				m_ExerciseData[exercise_idx]->reps.append(nextSetSuggestedReps(exercise_idx, type));
				m_ExerciseData[exercise_idx]->weight.append(nextSetSuggestedWeight(exercise_idx, type));

				switch (type)
				{
					case 0: //Regular
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
						m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
					break;
					case 1: //Pyramid
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
						m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
					break;
					case 2: //DropSet
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
						m_ExerciseData[exercise_idx]->subsets.append(m_ExerciseData.at(exercise_idx)->subsets.last());
					break;
					case 3: //ClusterSet
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 0));
						m_ExerciseData[exercise_idx]->subsets.append(m_ExerciseData.at(exercise_idx)->subsets.last());
					break;
					case 4: //GiantSet
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
						m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
					break;
					case 5: //MyoReps
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 1, 30));
						m_ExerciseData[exercise_idx]->subsets.append(QString::number(m_ExerciseData.at(exercise_idx)->subsets.last().toUInt() + 1));
					break;
					case 6: //Reverse Pyramid
						m_ExerciseData[exercise_idx]->resttime.append(increaseStringTimeBy(m_ExerciseData.at(exercise_idx)->resttime.last(), 0, 30));
						m_ExerciseData[exercise_idx]->subsets.append(u"0"_qs);
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
	return 0;
}

void DBTrainingDayModel::setSetType(const uint set_number, const uint new_type, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->type[set_number] = QString::number(new_type);
		setModified(true);
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

void DBTrainingDayModel::newSetSubSet(const uint exercise_idx, const uint set_number)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->subsets.count())
	{
		m_ExerciseData[exercise_idx]->subsets[set_number] = QString::number(m_ExerciseData.at(exercise_idx)->subsets.at(set_number).toUInt() + 1);
		m_ExerciseData[exercise_idx]->reps[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->reps.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 2) + subrecord_separator);
		m_ExerciseData[exercise_idx]->weight[set_number].append(QString::number(
			m_ExerciseData.at(exercise_idx)->weight.last().split(subrecord_separator, Qt::SkipEmptyParts).last().toUInt() - 10) + subrecord_separator);
		setModified(true);
	}
}

void DBTrainingDayModel::setSetSubSets(const uint set_number, const QString& new_subsets, const uint exercise_idx)
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

void DBTrainingDayModel::setSetReps(const uint set_number, const QString& new_reps, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->reps[set_number] = new_reps;
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

void DBTrainingDayModel::setSetWeight(const uint set_number, const QString& new_weight, const uint exercise_idx)
{
	if (set_number < m_ExerciseData.at(exercise_idx)->nsets)
	{
		m_ExerciseData[exercise_idx]->weight[set_number] = new_weight;
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
		m_ExerciseData[exercise_idx]->notes[set_number] = new_notes;
		setModified(true);
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
		setModified(true);
	}
}
