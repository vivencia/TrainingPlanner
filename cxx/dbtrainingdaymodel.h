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

	Q_INVOKABLE const int id() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(0).toInt() : -1; }
	Q_INVOKABLE const int mesoId() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(1).toInt(): -1; }

	Q_INVOKABLE QDate date() const { return currentRow() >= 0 ? QDate::fromJulianDay(m_modeldata.at(currentRow()).at(2).toLongLong()) : QDate::currentDate(); }
	Q_INVOKABLE void setDate(const QDate& date) { m_modeldata[currentRow()][2] = QString::number(date.toJulianDay()); }

	Q_INVOKABLE QString trainingDay() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(3) : QString(); }
	Q_INVOKABLE void setTrainingDay(const QString& trainingday ) { m_modeldata[currentRow()][3] = trainingday; }

	Q_INVOKABLE QString splitLetter() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(4) : QString(); }
	Q_INVOKABLE void setSplitLetter(const QString& splitletter ) { m_modeldata[currentRow()][4] = splitletter; }

	Q_INVOKABLE QString timeIn() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(5) : QString(); }
	Q_INVOKABLE void setTimeIn(const QString& timein) { m_modeldata[currentRow()][5] = timein; }

	Q_INVOKABLE QString timeOut() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(6) : QString(); }
	Q_INVOKABLE void setTimeOut(const QString& timeout) { m_modeldata[currentRow()][6] = timeout; }

	Q_INVOKABLE QString location() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(7) : QString(); }
	Q_INVOKABLE void setLocation(const QString& location) { m_modeldata[currentRow()][7] = location; }

	Q_INVOKABLE QString notes() const { return currentRow() >= 0 ? m_modeldata.at(currentRow()).at(8) : QString(); }
	Q_INVOKABLE void setNotes(const QString& notes) { m_modeldata[currentRow()][8] = notes; }

	Q_INVOKABLE bool exercisesOK() const { return m_workingExercise != m_ExerciseData.constEnd(); }
	Q_INVOKABLE void exercisesBegin() { m_workingExercise = m_ExerciseData.constBegin(); }
	Q_INVOKABLE bool exercisesNext() { ++m_workingExercise; return m_workingExercise != m_ExerciseData.constEnd(); }

	Q_INVOKABLE const uint exercisesNumber() const { return m_ExerciseData.count(); }
	Q_INVOKABLE const uint setsNumber() const { return m_workingExercise.value().at(0).split(record_separator).count(); }

	Q_INVOKABLE QString exerciseName() const;
	Q_INVOKABLE void setExerciseName(const QString& name);

	Q_INVOKABLE QString exerciseName1() const;
	Q_INVOKABLE void setExerciseName1(const QString& name1);

	Q_INVOKABLE QString exerciseName2() const;
	Q_INVOKABLE void setExerciseName2(const QString& name2);

	Q_INVOKABLE QString setType(const uint set_number) const;
	Q_INVOKABLE void setSetType(const uint set_number, const uint new_type);

	Q_INVOKABLE QString setRestTime(const uint set_number) const;
	Q_INVOKABLE void setSetRestTime(const uint set_number, const QString& new_time);

	Q_INVOKABLE QString setSubSets(const uint set_number) const;
	Q_INVOKABLE void setSetSubSets(const uint set_number, const QString& new_subsets);

	Q_INVOKABLE QString setReps(const uint set_number) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const QString& new_weight);

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const uint subset, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint subset, const QString& new_weight);

private:
	QHash<QString,QStringList> m_ExerciseData;
	QHash<QString,QStringList>::const_iterator m_workingExercise;

	const QString fillSubSets(const QString& subSetInfo, const uint subset, const QString& value);
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
