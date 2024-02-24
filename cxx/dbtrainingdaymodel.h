#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBTrainingDayModel(QObject *parent = nullptr) : TPListModel{parent} {}
	void appendExercisesList(const QStringList& list);

	Q_INVOKABLE const uint id() const { return m_modeldata.at(currentRow()).at(0).toUInt(); }
	Q_INVOKABLE const uint mesoId() const { return m_modeldata.at(currentRow()).at(1).toUInt(); }
	Q_INVOKABLE QDate date() const { return QDate::fromJulianDay(m_modeldata.at(currentRow()).at(2).toLongLong()); }
	Q_INVOKABLE QString splitLetter() const { return m_modeldata.at(currentRow()).at(3); }
	Q_INVOKABLE QString timeIn() const { return m_modeldata.at(currentRow()).at(4); }
	Q_INVOKABLE QString timeOut() const { return m_modeldata.at(currentRow()).at(5); }
	Q_INVOKABLE QString location() const { return m_modeldata.at(currentRow()).at(6); }
	Q_INVOKABLE QString notes() const { return m_modeldata.at(currentRow()).at(7); }

	Q_INVOKABLE void exercisesBegin() { m_workingExercise = m_ExerciseData.constBegin(); }
	Q_INVOKABLE bool exercisesNext() { ++m_workingExercise; return m_workingExercise != m_ExerciseData.constEnd(); }

	Q_INVOKABLE const uint exercisesNumber() const { return m_ExerciseData.count(); }
	Q_INVOKABLE const uint setsNumber() const { return m_workingExercise.value().at(0).split(record_separator).count(); }

	Q_INVOKABLE QString exerciseName() const;
	Q_INVOKABLE QString exerciseName1() const;
	Q_INVOKABLE QString exerciseName2() const;

	Q_INVOKABLE QString setType(const uint set_number) const;
	Q_INVOKABLE QString setRestTime(const uint set_number) const;
	Q_INVOKABLE QString setSubSets(const uint set_number) const;
	Q_INVOKABLE QString setReps(const uint set_number) const;
	Q_INVOKABLE QString setWeight(const uint set_number) const;

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset) const;
	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset) const;

private:
	QHash<QString,QStringList> m_ExerciseData;
	QHash<QString,QStringList>::const_iterator m_workingExercise;
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
