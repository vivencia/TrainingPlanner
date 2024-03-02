#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBTrainingDayModel(QObject *parent = nullptr) : TPListModel{parent} {}
	~DBTrainingDayModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	void fromDataBase(const QStringList& list);
	void getSaveInfo(QStringList& data) const;

	Q_INVOKABLE const int id() const { return count() == 1 ? m_modeldata.at(0).at(0).toInt() : -1; }
	Q_INVOKABLE const int mesoId() const { return count() == 1 ? m_modeldata.at(0).at(1).toInt(): -1; }

	Q_INVOKABLE QDate date() const { return count() == 1 ? QDate::fromJulianDay(m_modeldata.at(0).at(2).toLongLong()) : QDate::currentDate(); }
	Q_INVOKABLE void setDate(const QDate& date) { m_modeldata[0][2] = QString::number(date.toJulianDay()); }

	Q_INVOKABLE QString trainingDay() const { return count() == 1 ? m_modeldata.at(0).at(3) : QString(); }
	Q_INVOKABLE void setTrainingDay(const QString& trainingday ) { m_modeldata[0][3] = trainingday; }

	Q_INVOKABLE QString splitLetter() const { return count() == 1 ? m_modeldata.at(0).at(4) : QString(); }
	Q_INVOKABLE void setSplitLetter(const QString& splitletter ) { m_modeldata[0][4] = splitletter; }

	Q_INVOKABLE QString timeIn() const { return count() == 1 ? m_modeldata.at(0).at(5) : QString(); }
	Q_INVOKABLE void setTimeIn(const QString& timein) { m_modeldata[0][5] = timein; }

	Q_INVOKABLE QString timeOut() const { return count() == 1 ? m_modeldata.at(0).at(6) : QString(); }
	Q_INVOKABLE void setTimeOut(const QString& timeout) { m_modeldata[0][6] = timeout; }

	Q_INVOKABLE QString location() const { return count() == 1 ? m_modeldata.at(0).at(7) : QString(); }
	Q_INVOKABLE void setLocation(const QString& location) { m_modeldata[0][7] = location; }

	Q_INVOKABLE QString dayNotes() const { return count() == 1 ? m_modeldata.at(0).at(8) : QString(); }
	Q_INVOKABLE void setDayNotes(const QString& day_notes) { m_modeldata[0][8] = day_notes; }

	Q_INVOKABLE const uint exercisesNumber() const { return m_ExerciseData.count(); }
	Q_INVOKABLE const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }

	Q_INVOKABLE QString exerciseName(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName(const QString& new_name, const uint exercise_idx);
	Q_INVOKABLE void newExercise(const QString& new_exercise, const uint idx);

	Q_INVOKABLE QString exerciseName1(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName1(const QString& name1, const uint exercise_idx);

	Q_INVOKABLE QString exerciseName2(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName2(const QString& name2, const uint exercise_idx);

	Q_INVOKABLE void newSet(const uint exercise_idx, const uint set_number, const uint type, const QString& resttime,
					const QString& subsets, const QString& reps, const QString& weight, const QString& notes);
	Q_INVOKABLE void newSet(const uint exercise_idx, const uint set_number, const uint type);
	Q_INVOKABLE bool removeSet(const uint set_number, const uint exercise_idx);

	Q_INVOKABLE uint setType(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetType(const uint set_number, const uint new_type, const uint exercise_idx);

	Q_INVOKABLE QString setRestTime(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetRestTime(const uint set_number, const QString& new_time, const uint exercise_idx);

	Q_INVOKABLE QString setSubSets(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetSubSets(const uint set_number, const QString& new_subsets, const uint exercise_idx);

	Q_INVOKABLE QString setReps(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const QString& new_reps, const uint exercise_idx);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const QString& new_weight, const uint exercise_idx);

	Q_INVOKABLE QString setNotes(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetNotes(const uint set_number, const QString& new_notes, const uint exercise_idx);

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const uint subset, const QString& new_reps, const uint exercise_idx);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint subset, const QString& new_weight, const uint exercise_idx);

private:
	struct exerciseEntry {
		QString name;
		uint nsets;
		QStringList type;
		QStringList resttime;
		QStringList subsets;
		QStringList reps;
		QStringList weight;
		QStringList notes;

		exerciseEntry() : nsets(0) {}
	};

	QList<exerciseEntry*> m_ExerciseData;
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
