#include "qmlsetentry.h"
#include "qmlexerciseentry.h"
#include "dbworkoutmodel.h"
#include "tputils.h"

#include <QQuickItem>

QString QmlSetEntry::exerciseName1() const
{
	return "1: "_L1 + std::move(appUtils()->getCompositeValue(0, m_exerciseName, comp_exercise_separator));
}

void QmlSetEntry::setExerciseName1(const QString& new_value)
{
	if (appUtils()->getCompositeValue(0, m_exerciseName, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_exerciseName, comp_exercise_separator);
		emit exerciseName1Changed();
	}
}

QString QmlSetEntry::exerciseName2() const
{
	QString exercisename{std::move(appUtils()->getCompositeValue(1, m_exerciseName, comp_exercise_separator))};
	return "2: "_L1 + (exercisename.isEmpty() ? std::move(tr("Add exercise ...")) : exercisename);
}

void QmlSetEntry::setExerciseName2(const QString& new_value)
{
	if (appUtils()->getCompositeValue(1, m_exerciseName, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_exerciseName, comp_exercise_separator);
		emit exerciseName2Changed();
	}
}

void QmlSetEntry::setType(const uint new_value, const bool bSetIsManuallyModified)
{
	//new_value already checked under QmlExerciseEntry::changeSetType
	m_type = new_value;
	emit typeChanged();
	if (bSetIsManuallyModified && !lastSet())
		setIsManuallyModified(true);
	const bool bHasSubSets(m_type == SET_TYPE_CLUSTER || m_type == SET_TYPE_DROP);
	if (m_bHasSubSets != bHasSubSets)
	{
		m_bHasSubSets = bHasSubSets;
		emit hasSubSetsChanged();
	}
}

void QmlSetEntry::setNumber(const uint new_value)
{
	if (m_number != new_value)
	{
		m_number = new_value;
		emit numberChanged();
		setEntry()->setProperty("Layout.row", m_number);
	}
}

void QmlSetEntry::setRestTime(const QString& new_value, const bool bJustUpdateValue, const bool bSetIsManuallyModified)
{
	m_restTime = new_value;
	if (bSetIsManuallyModified && !lastSet())
		setIsManuallyModified(true);
	if (bJustUpdateValue)
		emit restTimeChanged();
	else
		m_workoutModel->setSetRestTime(m_exercise_idx, number(), m_restTime);
}

QString QmlSetEntry::reps1() const
{
	return appUtils()->getCompositeValue(0, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps1(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(0, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_reps, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit reps1Changed();
		if (hasSubSets())
			emit strTotalRepsChanged();
		m_workoutModel->setSetReps(m_exercise_idx, number(), 0, m_reps);
	}
}

QString QmlSetEntry::weight1() const
{
	return appUtils()->getCompositeValue(0, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight1(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(0, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_weight, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit weight1Changed();
		m_workoutModel->setSetWeight(m_exercise_idx, number(), 0, m_weight);
	}
}

QString QmlSetEntry::reps2() const
{
	return appUtils()->getCompositeValue(1, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps2(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(1, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_reps, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit reps2Changed();
		emit strTotalRepsChanged();
		m_workoutModel->setSetReps(m_exercise_idx, number(), 1, m_reps);
	}
}

QString QmlSetEntry::weight2() const
{
	return appUtils()->getCompositeValue(1, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight2(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(1, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_weight, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit weight2Changed();
		m_workoutModel->setSetWeight(m_exercise_idx, number(), 1, m_weight);
	}
}

QString QmlSetEntry::reps3() const
{
	return appUtils()->getCompositeValue(2, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps3(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(2, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(2, new_value, m_reps, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit reps3Changed();
		emit strTotalRepsChanged();
		m_workoutModel->setSetReps(m_exercise_idx, number(), 2, m_reps);
	}
}

QString QmlSetEntry::weight3() const
{
	return appUtils()->getCompositeValue(2, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight3(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(2, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(2, new_value, m_weight, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit weight3Changed();
		m_workoutModel->setSetWeight(m_exercise_idx, number(), 2, m_weight);
	}
}

QString QmlSetEntry::reps4() const
{
	return appUtils()->getCompositeValue(3, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps4(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(3, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(3, new_value, m_reps, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit reps4Changed();
		emit strTotalRepsChanged();
		m_workoutModel->setSetReps(m_exercise_idx, number(), 3, m_reps);
	}
}

QString QmlSetEntry::weight4() const
{
	return appUtils()->getCompositeValue(3, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight4(const QString& new_value, const bool bSetIsManuallyModified)
{
	if (appUtils()->getCompositeValue(3, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(3, new_value, m_weight, comp_exercise_separator);
		if (bSetIsManuallyModified && !lastSet())
			setIsManuallyModified(true);
		emit weight4Changed();
		m_workoutModel->setSetWeight(m_exercise_idx, number(), 3, m_weight);
	}
}

void QmlSetEntry::setSubSets(const QString& new_value)
{
	const uint new_value_int(new_value.toUInt());
	if (new_value_int < 4)
	{
		if (m_subsets != new_value)
		{
			m_subsets = new_value;
			m_nsubsets = new_value_int;
			emit subSetsChanged();
			emit nSubSetsChanged();
			emit strTotalRepsChanged();
			m_workoutModel->setSetSubSets(m_exercise_idx, number(), new_value);
		}
	}
}

void QmlSetEntry::setNotes(const QString& new_value)
{
	m_notes = new_value;
	emit notesChanged();
	m_workoutModel->setSetNotes(m_exercise_idx, number(), new_value);
}
