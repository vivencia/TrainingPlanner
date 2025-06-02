#include "qmlsetentry.h"

#include "dbexercisesmodel.h"
#include "qmlexerciseentry.h"
#include "translationclass.h"

#include <QQuickItem>

QmlSetEntry::QmlSetEntry(QObject *parent, QmlExerciseEntry *parentExercise, DBExercisesModel *workoutModel,
							const uint exercise_number, const uint exercise_idx, const uint set_number)
	: QObject{parent}, m_parentExercise{parentExercise}, m_workoutModel{workoutModel}, m_setEntry{nullptr},
		m_exerciseNumber{exercise_number}, m_exerciseIdx{exercise_idx}, m_setNumber{set_number}
{
	m_setMode = SET_MODE_UNDEFINED;
	if (!m_workoutModel->setCompleted(m_exerciseNumber, m_exerciseIdx, m_setNumber))
	{
		if (m_setNumber > 0)
		{
			if (m_workoutModel->trackRestTime(m_exerciseNumber) || m_workoutModel->autoRestTime(m_exerciseNumber))
				m_setMode = SET_MODE_START_REST;
		}
	}
	else
		m_setMode = SET_MODE_SET_COMPLETED;
	m_bEditable = false;
	m_bLastSet = m_setNumber == m_workoutModel->setsNumber(m_exerciseNumber, m_exerciseIdx) - 1;

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		emit labelChanged();
		//Update labels that need two signals: one specific to the value they hold and this, for the words to be translated
		emit modeChanged();
		emit setNumberChanged();
		emit totalRepsChanged();
	});
}

QString QmlSetEntry::setNumberLabel() const
{
	return tr("Set #: ") + QString::number(m_setNumber + 1);
}

QString QmlSetEntry::totalRepsLabel() const
{
	return tr("Total # of reps: ") + QString::number(reps().toInt() * nSubSets());
}

QString QmlSetEntry::modeLabel() const
{
	switch (mode())
	{
		case SET_MODE_UNDEFINED: return std::move(tr("Set completed?"));
		case SET_MODE_START_REST: return std::move(tr("Start rest"));
		case SET_MODE_START_EXERCISE: return std::move(tr("Begin exercise"));
		case SET_MODE_SET_COMPLETED: break;
	}
	return QString{};
}

void QmlSetEntry::setSetNumber(const uint new_number)
{
	m_setNumber = new_number;
	emit setNumberChanged();
}

QString QmlSetEntry::exerciseName() const
{
	return m_workoutModel->exerciseName(m_exerciseNumber, m_exerciseIdx);
}

void QmlSetEntry::setExerciseName(const QString &new_value)
{
	if (new_value != exerciseName())
	{
		m_workoutModel->setExerciseName(m_exerciseNumber, m_exerciseIdx, new_value);
		emit exerciseNameChanged();
	}
}

const uint QmlSetEntry::type() const
{
	return m_workoutModel->setType(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setType(const uint new_type)
{
	m_parentExercise->changeSetType(m_setNumber, new_type);
	emit typeChanged();
	emit hasSubSetsChanged();
}

QString QmlSetEntry::restTime() const
{
	return m_workoutModel->setRestTime(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setRestTime(const QString &new_value, const bool update_model)
{
	emit restTimeChanged();
	if (update_model)
		m_workoutModel->setSetRestTime(m_exerciseNumber, m_exerciseIdx, m_setNumber, new_value);
}

QString QmlSetEntry::reps() const
{
	return m_workoutModel->setReps(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setReps(const QString &new_reps, const bool update_model)
{
	if (reps() != new_reps)
	{
		if (update_model)
			m_workoutModel->setSetReps(m_exerciseNumber, m_exerciseIdx, m_setNumber, new_reps);
		emit repsChanged();
		if (hasSubSets())
			emit totalRepsChanged();
	}
}

QString QmlSetEntry::weight() const
{
	return m_workoutModel->setWeight(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setWeight(const QString &new_weight, const bool update_model)
{
	if (weight() != new_weight)
	{
		if (update_model)
			m_workoutModel->setSetWeight(m_exerciseNumber, m_exerciseIdx, m_setNumber, new_weight);
		emit weightChanged();
	}
}

QString QmlSetEntry::subSets() const
{
	return m_workoutModel->setSubSets(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setSubSets(const QString &new_subsets, const bool update_model)
{
	if (subSets() != new_subsets)
	{
		if (update_model)
			m_workoutModel->setSetSubSets(m_exerciseNumber, m_exerciseIdx, m_setNumber, new_subsets);
		emit subSetsChanged();
		emit totalRepsChanged();
	}
}

void QmlSetEntry::addSubSet()
{
	const uint new_subsets_value(subSets().toUInt());
	if (new_subsets_value <= 4)
		setSubSets(QString::number(new_subsets_value+1));
}

void QmlSetEntry::removeSubSet()
{
	const uint new_subsets_value(subSets().toUInt());
	if (new_subsets_value >= 1)
		setSubSets(QString::number(new_subsets_value-1));
}

QString QmlSetEntry::notes() const
{
	return m_workoutModel->setNotes(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setNotes(const QString &new_notes)
{
	m_workoutModel->setSetNotes(m_exerciseNumber, m_exerciseIdx, m_setNumber, new_notes);
	emit notesChanged();
}

const bool QmlSetEntry::completed() const
{
	return m_workoutModel->setCompleted(m_exerciseNumber, m_exerciseIdx, m_setNumber);
}

void QmlSetEntry::setCompleted(const bool completed)
{
	m_workoutModel->setSetCompleted(m_exerciseNumber, m_exerciseIdx, m_setNumber, completed);
}

const bool QmlSetEntry::workingSet() const
{
	return m_workoutModel->workingSet(m_exerciseNumber, m_exerciseIdx) == m_setNumber;
}

void QmlSetEntry::setWorkingSet(const bool working)
{
	if (working)
		m_workoutModel->setWorkingSet(m_setNumber, m_exerciseNumber, m_exerciseIdx);
}

const bool QmlSetEntry::hasSubSets() const
{
	return m_workoutModel->setSubSets(m_exerciseNumber, m_exerciseIdx, m_setNumber) != '0';
}
