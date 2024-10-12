#include "qmlexerciseentry.h"
#include "dbtrainingdaymodel.h"
#include "tpglobals.h"

void QmlExerciseEntry::setNewSetType(const uint new_value)
{
	m_type = new_value;
	emit newSetTypeChanged();

	const uint nsets(nSets());
	QString strSets;

	if (nsets == 0)
	{
		switch(m_type)
		{
			case SET_TYPE_DROP: strSets = u"1"_qs; break;
			case SET_TYPE_CLUSTER: strSets = u"2"_qs; break;
			case SET_TYPE_MYOREPS: strSets = u"3"_qs; break;
			default: strSets = u"4"_qs; break;
		}
	}
	setSetsNumber(strSets);
	setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_type, nsets));
	setRepsNumber(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, m_type, nsets));
	setWeight(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, m_type, nsets));
}

const QString QmlExerciseEntry::exerciseName() const
{
	return m_tDayModel->exerciseName(m_exercise_idx);
}

void QmlExerciseEntry::setExerciseName(const QString& new_value)
{
	m_tDayModel->setExerciseName(m_exercise_idx, new_value);
	emit exerciseNameChanged();
}

void QmlExerciseEntry::setTrackRestTime(const bool new_value)
{
	bTrackRestTime = new_value;
	emit trackRestTimeChanged();
	m_tDayModel->setTrackRestTime(bTrackRestTime, m_exercise_idx);
	if (!bTrackRestTime)
		setAutoRestTime(false);
	//enableDisableSetsRestTime(exercise_idx, bTrackRestTime, bAutoRestTime);
}

void QmlExerciseEntry::setAutoRestTime(const bool new_value)
{
	bAutoRestTime = new_value;
	emit autoRestTimeChanged();
	m_tDayModel->setAutoRestTime(m_exercise_idx, false);
	if (!bAutoRestTime)
		setRestTime(u"00:00"_qs);
	//else
		//TODO based on the last set value
	//enableDisableSetsRestTime(exercise_idx, bTrackRestTime, bAutoRestTime);
}
