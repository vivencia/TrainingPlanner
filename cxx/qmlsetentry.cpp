#include "qmlsetentry.h"
#include "qmlexerciseentry.h"
#include "dbtrainingdaymodel.h"
#include "tputils.h"

#include <QQuickItem>

QString QmlSetEntry::exerciseName1() const
{
	return appUtils()->getCompositeValue(0, m_exerciseName, comp_exercise_separator);
}

void QmlSetEntry::setExerciseName1(const QString& new_value, const bool bFromQML)
{
	if (appUtils()->getCompositeValue(0, m_exerciseName, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_reps, comp_exercise_separator);
		emit exerciseName1Changed();
		if (bFromQML)
			m_parentExercise->setExerciseName(m_exerciseName, false);
	}
}

QString QmlSetEntry::exerciseName2() const
{
	return appUtils()->getCompositeValue(1, m_exerciseName, comp_exercise_separator);
}

void QmlSetEntry::setExerciseName2(const QString& new_value, const bool bFromQML)
{
	if (appUtils()->getCompositeValue(1, m_exerciseName, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_reps, comp_exercise_separator);
		emit exerciseName2Changed();
		if (bFromQML)
			m_parentExercise->setExerciseName(m_exerciseName, false);
	}
}

void QmlSetEntry::setType(const uint new_value)
{
	//new_value already checked under QmlExerciseEntry::changeSetType
	m_type = new_value;
	emit typeChanged();
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

void QmlSetEntry::setRestTime(const QString& new_value, const bool bJustUpdateValue)
{
	m_restTime = new_value;
	if (bJustUpdateValue)
		emit restTimeChanged();
	else
		m_tDayModel->setSetRestTime(m_exercise_idx, number(), m_restTime);
}

QString QmlSetEntry::reps1() const
{
	return appUtils()->getCompositeValue(0, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps1(const QString& new_value)
{
	if (appUtils()->getCompositeValue(0, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_reps, comp_exercise_separator);
		emit reps1Changed();
		if (hasSubSets())
			emit strTotalRepsChanged();
		m_tDayModel->setSetReps(m_exercise_idx, number(), 0, new_value);
	}
}

QString QmlSetEntry::weight1() const
{
	return appUtils()->getCompositeValue(0, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight1(const QString& new_value)
{
	if (appUtils()->getCompositeValue(0, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(0, new_value, m_weight, comp_exercise_separator);
		emit weight1Changed();
		m_tDayModel->setSetWeight(m_exercise_idx, number(), 0, new_value);
	}
}

QString QmlSetEntry::reps2() const
{
	return appUtils()->getCompositeValue(1, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps2(const QString& new_value)
{
	if (appUtils()->getCompositeValue(1, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_reps, comp_exercise_separator);
		emit reps2Changed();
		m_tDayModel->setSetReps(m_exercise_idx, number(), 1, new_value);
	}
}

QString QmlSetEntry::weight2() const
{
	return appUtils()->getCompositeValue(1, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight2(const QString& new_value)
{
	if (appUtils()->getCompositeValue(1, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(1, new_value, m_weight, comp_exercise_separator);
		emit weight2Changed();
		m_tDayModel->setSetWeight(m_exercise_idx, number(), 1, new_value);
	}
}

QString QmlSetEntry::reps3() const
{
	return appUtils()->getCompositeValue(2, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps3(const QString& new_value)
{
	if (appUtils()->getCompositeValue(2, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(2, new_value, m_reps, comp_exercise_separator);
		emit reps3Changed();
		m_tDayModel->setSetReps(m_exercise_idx, number(), 2, new_value);
	}
}

QString QmlSetEntry::weight3() const
{
	return appUtils()->getCompositeValue(2, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight3(const QString& new_value)
{
	if (appUtils()->getCompositeValue(2, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(2, new_value, m_weight, comp_exercise_separator);
		emit weight3Changed();
		m_tDayModel->setSetWeight(m_exercise_idx, number(), 2, new_value);
	}
}

QString QmlSetEntry::reps4() const
{
	return appUtils()->getCompositeValue(3, m_reps, comp_exercise_separator);
}

void QmlSetEntry::setReps4(const QString& new_value)
{
	if (appUtils()->getCompositeValue(3, m_reps, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(3, new_value, m_reps, comp_exercise_separator);
		emit reps4Changed();
		m_tDayModel->setSetReps(m_exercise_idx, number(), 3, new_value);
	}
}

QString QmlSetEntry::weight4() const
{
	return appUtils()->getCompositeValue(3, m_weight, comp_exercise_separator);
}

void QmlSetEntry::setWeight4(const QString& new_value)
{
	if (appUtils()->getCompositeValue(3, m_weight, comp_exercise_separator) != new_value)
	{
		appUtils()->setCompositeValue(3, new_value, m_weight, comp_exercise_separator);
		emit weight4Changed();
		m_tDayModel->setSetWeight(m_exercise_idx, number(), 3, new_value);
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
			m_tDayModel->setSetSubSets(m_exercise_idx, number(), new_value);
		}
	}
}

void QmlSetEntry::setNotes(const QString& new_value)
{
	m_notes = new_value;
	emit notesChanged();
	m_tDayModel->setSetNotes(m_exercise_idx, number(), new_value);
}
