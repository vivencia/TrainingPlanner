#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"

class DBMesoSplitModel;

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBTrainingDayModel(QObject *parent = nullptr) : TPListModel{parent}, m_tDayModified(false) {}
	~DBTrainingDayModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	inline void clearExercises() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; m_ExerciseData.clear(); setModified(true); }
	void fromDataBase(const QStringList& list);
	void getSaveInfo(QStringList& data) const;
	void convertMesoModelToTDayModel(DBMesoSplitModel* splitModel);
	Q_INVOKABLE void appendRow() { appendList(QStringList(9)); setId("-1"); }

	Q_INVOKABLE const int id() const { return count() == 1 ? m_modeldata.at(0).at(0).toInt() : -1; }
	inline const QString& idStr() const { return m_modeldata.at(0).at(0); }
	inline void setId(const QString& new_id) { m_modeldata[0][0] = new_id; }

	Q_INVOKABLE const int mesoId() const { return count() == 1 ? m_modeldata.at(0).at(1).toInt(): -1; }
	inline const QString& mesoIdStr() const { return m_modeldata.at(0).at(1); }
	Q_INVOKABLE inline void setMesoId(const QString& mesoid) { m_modeldata[0][1] = mesoid; }

	Q_INVOKABLE QDate date() const { return count() == 1 ? QDate::fromJulianDay(m_modeldata.at(0).at(2).toLongLong()) : QDate::currentDate(); }
	inline const QString& dateStr() const { return m_modeldata.at(0).at(2); }
	Q_INVOKABLE void setDate(const QDate& date) { m_modeldata[0][2] = QString::number(date.toJulianDay()); setModified(true); }

	Q_INVOKABLE QString trainingDay() const { return m_modeldata.at(0).at(3); }
	//Do not set model's modified to true because this is called from onTextChanged on TrainingDayInfo.qml, which gets called when the property
	//is changed even for the first time, i.e., when the page is receiving default values
	Q_INVOKABLE void setTrainingDay(const QString& trainingday ) { if (trainingday != m_modeldata.at(0).at(3)) { m_modeldata[0][3] = trainingday; } }

	Q_INVOKABLE QString splitLetter() const { return m_modeldata.at(0).at(4); }
	Q_INVOKABLE void setSplitLetter(const QString& splitletter ) { if (splitletter != m_modeldata.at(0).at(4)) { m_modeldata[0][4] = splitletter; setModified(true); } }

	Q_INVOKABLE QString timeIn() const { return m_modeldata.at(0).at(5); }
	Q_INVOKABLE void setTimeIn(const QString& timein) { if (timein != m_modeldata.at(0).at(5)) { m_modeldata[0][5] = timein; setModified(true); } }

	Q_INVOKABLE QString timeOut() const { return m_modeldata.at(0).at(6); }
	Q_INVOKABLE void setTimeOut(const QString& timeout) { if (timeout != m_modeldata.at(0).at(6)) {  m_modeldata[0][6] = timeout; setModified(true); } }

	Q_INVOKABLE QString location() const { return m_modeldata.at(0).at(7); }
	Q_INVOKABLE void setLocation(const QString& location) { m_modeldata[0][7] = location; setModified(true); }

	Q_INVOKABLE QString dayNotes() const { return m_modeldata.at(0).at(8); }
	Q_INVOKABLE void setDayNotes(const QString& day_notes) { m_modeldata[0][8] = day_notes; setModified(true); }

	Q_INVOKABLE const uint exercisesNumber() const { return m_ExerciseData.count(); }
	Q_INVOKABLE const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }

	Q_INVOKABLE QString exerciseName(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName(const QString& new_name, const uint exercise_idx);
	void newExercise(const QString& new_exercise, const uint idx);
	void removeExercise(const uint exercise_idx);

	Q_INVOKABLE QString exerciseName1(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName1(const QString& name1, const uint exercise_idx);

	Q_INVOKABLE QString exerciseName2(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName2(const QString& name2, const uint exercise_idx);

	void newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight);
	const QString& nextSetSuggestedReps(const uint exercise_idx, const uint type) const;
	const QString& nextSetSuggestedWeight(const uint exercise_idx, const uint type) const;
	void newSet(const uint exercise_idx, const uint set_number, const uint type);
	bool removeSet(const uint set_number, const uint exercise_idx);

	Q_INVOKABLE uint setType(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetType(const uint set_number, const uint new_type, const uint exercise_idx);

	Q_INVOKABLE QString setRestTime(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetRestTime(const uint set_number, const QString& new_time, const uint exercise_idx);

	Q_INVOKABLE QString setSubSets(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void newSetSubSet(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE uint setSubSets_int(const uint set_number, const uint exercise_idx) const { return setSubSets(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetSubSets(const uint set_number, const QString& new_subsets, const uint exercise_idx);

	Q_INVOKABLE QString setReps(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE uint setReps_int(const uint set_number, const uint exercise_idx) const { return setReps(set_number, exercise_idx).toUInt(); }
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
	bool m_tDayModified;
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
