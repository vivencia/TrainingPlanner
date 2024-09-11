#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"

#define TDAY_COL_ID 0
#define TDAY_COL_MESOID 1
#define TDAY_COL_DATE 2
#define TDAY_COL_TRAININGDAYNUMBER 3
#define TDAY_COL_SPLITLETTER 4
#define TDAY_COL_TIMEIN 5
#define TDAY_COL_TIMEOUT 6
#define TDAY_COL_LOCATION 7
#define TDAY_COL_NOTES 8

#define TDAY_EXERCISES_COL_NAMES 0
#define TDAY_EXERCISES_COL_TYPES 1
#define TDAY_EXERCISES_COL_RESTTIMES 2
#define TDAY_EXERCISES_COL_SUBSETS 3
#define TDAY_EXERCISES_COL_REPS 4
#define TDAY_EXERCISES_COL_WEIGHTS 5
#define TDAY_EXERCISES_COL_NOTES 6
#define TDAY_EXERCISES_COL_COMPLETED 7

#define SET_TYPE_REGULAR 0
#define SET_TYPE_PYRAMID 1
#define SET_TYPE_DROP 2
#define SET_TYPE_CLUSTER 3
#define SET_TYPE_GIANT 4
#define SET_TYPE_MYOREPS 5
#define SET_TYPE_REVERSE_PYRAMID 6

class DBExercisesModel;
class DBMesoSplitModel;

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)
Q_PROPERTY(bool dayIsEditable READ dayIsEditable WRITE setDayIsEditable NOTIFY dayIsEditableChanged FINAL)

public:
	explicit DBTrainingDayModel(QObject* parent = nullptr, const int meso_idx = -1);
	~DBTrainingDayModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	inline void clearExercises() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; m_ExerciseData.clear(); setModified(true); }
	void fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse = false);
	void getSaveInfo(QStringList& data) const;
	void convertMesoSplitModelToTDayModel(DBMesoSplitModel* splitModel);
	virtual bool updateFromModel(const TPListModel* model) override;

	const QString& formatSetTypeToExport(const QString& fieldValue) const;
	virtual void exportToText(QFile* outFile) const override;
	const QString& formatSetTypeToImport(const QString& fieldValue) const;
	virtual bool importFromText(QFile* inFile, QString& inData) override;

	inline void appendRow() { appendList(QStringList(9)); setId(u"-1"_qs); }
	void moveExercise(const uint from, const uint to);
	Q_INVOKABLE inline bool compositeExercise(const uint exercise_idx) const { return static_cast<bool>(m_CompositeExerciseList.value(exercise_idx)); }

	inline bool dayIsEditable() const { return mb_DayIsEditable; }
	inline void setDayIsEditable(const bool editable) { mb_DayIsEditable = editable; emit dayIsEditableChanged(); }

	inline bool dayIsFinished() const { return mb_DayIsFinished; }
	void setDayIsFinished(const bool finished);

	Q_INVOKABLE inline const int id() const { return count() == 1 ? m_modeldata.at(0).at(TDAY_COL_ID).toInt() : -1; }
	inline const QString& idStr() const { return m_modeldata.at(0).at(TDAY_COL_ID); }
	inline void setId(const QString& new_id) { m_modeldata[0][TDAY_COL_ID] = new_id; }

	Q_INVOKABLE inline const int mesoId() const { return count() == 1 ? m_modeldata.at(0).at(TDAY_COL_MESOID).toInt(): -1; }
	inline const QString& mesoIdStr() const { return m_modeldata.at(0).at(TDAY_COL_MESOID); }
	Q_INVOKABLE inline void setMesoId(const QString& mesoid) { m_modeldata[0][TDAY_COL_MESOID] = mesoid; }

	/*Q_INVOKABLE QDate date() const { return count() == 1 ? QDate::fromJulianDay(m_modeldata.at(0).at(TDAY_COL_DATE).toLongLong()) : QDate::currentDate(); }
	inline const QString& dateStr() const { return m_modeldata.at(0).at(TDAY_COL_DATE); }
	Q_INVOKABLE void setDate(const QDate& date) { m_modeldata[0][TDAY_COL_DATE] = QString::number(date.toJulianDay()); setModified(true); }*/

	Q_INVOKABLE inline QString trainingDay() const { return m_modeldata.at(0).at(TDAY_COL_TRAININGDAYNUMBER); }
	//Do not set model's modified to true because this is called from onTextChanged on TrainingDayInfo.qml, which gets called when the property
	//is changed even for the first time, i.e., when the page is receiving default values
	Q_INVOKABLE inline void setTrainingDay(const QString& trainingday )
	{ if (trainingday != m_modeldata.at(0).at(TDAY_COL_TRAININGDAYNUMBER)) { m_modeldata[0][TDAY_COL_TRAININGDAYNUMBER] = trainingday; setModified(true); } }

	Q_INVOKABLE inline QString splitLetter() const { return m_modeldata.at(0).at(TDAY_COL_SPLITLETTER); }
	Q_INVOKABLE inline void setSplitLetter(const QString& splitletter )
	{ if (splitletter != m_modeldata.at(0).at(TDAY_COL_SPLITLETTER)) { m_modeldata[0][TDAY_COL_SPLITLETTER] = splitletter; setModified(true); } }

	Q_INVOKABLE inline QString timeIn() const { return m_modeldata.at(0).at(TDAY_COL_TIMEIN); }
	Q_INVOKABLE inline void setTimeIn(const QString& timein)
	{ if (timein != m_modeldata.at(0).at(TDAY_COL_TIMEIN)) { m_modeldata[0][TDAY_COL_TIMEIN] = timein; setModified(true); } }

	Q_INVOKABLE inline QString timeOut() const { return m_modeldata.at(0).at(TDAY_COL_TIMEOUT); }
	Q_INVOKABLE inline void setTimeOut(const QString& timeout)
	{ if (timeout != m_modeldata.at(0).at(TDAY_COL_TIMEOUT)) { m_modeldata[0][TDAY_COL_TIMEOUT] = timeout; setModified(true); } }

	Q_INVOKABLE inline QString location() const { return m_modeldata.at(0).at(TDAY_COL_LOCATION); }
	Q_INVOKABLE inline void setLocation(const QString& location) { m_modeldata[0][TDAY_COL_LOCATION] = location; setModified(true); }

	Q_INVOKABLE inline QString dayNotes() const { return m_modeldata.at(0).at(TDAY_COL_NOTES); }
	Q_INVOKABLE inline void setDayNotes(const QString& day_notes) { m_modeldata[0][TDAY_COL_NOTES] = day_notes; setModified(true); }

	const uint inline exerciseCount() const { return m_ExerciseData.count(); }
	Q_INVOKABLE inline const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }

	Q_INVOKABLE inline bool trackRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_TrackRestTime; }
	Q_INVOKABLE inline void setTrackRestTime(const bool track_resttime, const uint exercise_idx) { m_ExerciseData[exercise_idx]->mb_TrackRestTime = track_resttime; }

	Q_INVOKABLE inline bool autoRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_AutoRestTime; }
	Q_INVOKABLE inline void setAutoRestTime(const bool auto_resttime, const uint exercise_idx) { m_ExerciseData[exercise_idx]->mb_AutoRestTime = auto_resttime; }

	Q_INVOKABLE QString exerciseName(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName(const QString& new_name, const uint exercise_idx);
	void newExercise(const QString& new_exercise, const uint idx);
	void removeExercise(const uint exercise_idx);
	Q_INVOKABLE void changeExerciseName(const uint exercise_idx, DBExercisesModel* model);

	Q_INVOKABLE QString exerciseName1(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName1(const QString& name1, const uint exercise_idx);

	Q_INVOKABLE QString exerciseName2(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName2(const QString& name2, const uint exercise_idx);

	void newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight, const QString& nRestTime,
					 const QString& nSubsets = u"0"_qs, const QString& notes = u" "_qs);
	Q_INVOKABLE QString nextSetSuggestedTime(const uint exercise_idx, const uint type, const uint set_number = 100) const;
	const QString& nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	const QString& nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	void newSet(const uint set_number, const uint exercise_idx, const uint type, const QString& nReps = QString(),
					const QString& nWeight = QString(), const QString& nRestTime = QString(), const QString& nSubSets = QString());
	bool removeSet(const uint set_number, const uint exercise_idx);

	Q_INVOKABLE uint setType(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	void changeSetType(const uint set_number, const uint exercise_idx, const uint old_type, const uint new_type);

	Q_INVOKABLE QString setRestTime(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetRestTime(const uint set_number, const uint exercise_idx, const QString& new_time);

	Q_INVOKABLE QString setSubSets(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void newSetSubSet(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE uint setSubSets_int(const uint set_number, const uint exercise_idx) const { return setSubSets(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetSubSets(const uint set_number, const uint exercise_idx, const QString& new_subsets);

	Q_INVOKABLE QString setReps(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE uint setReps_int(const uint set_number, const uint exercise_idx) const { return setReps(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetReps(const uint set_number, const uint exercise_idx, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint exercise_idx, const QString& new_weight);

	Q_INVOKABLE QString setNotes(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetNotes(const uint set_number, const QString& new_notes, const uint exercise_idx);

	Q_INVOKABLE bool setCompleted(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetCompleted(const uint set_number, const uint exercise_idx, const bool completed);
	Q_INVOKABLE bool allSetsCompleted(const uint exercise_idx) const;

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_weight);

signals:
	void saveWorkout();
	void exerciseCountChanged();
	void compositeExerciseChanged(const uint exercise_idx);
	void dayIsFinishedChanged();
	void dayIsEditableChanged();
	void exerciseCompleted(const uint exercise_idx, const bool completed);

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
		QStringList completed;
		bool mb_TrackRestTime, mb_AutoRestTime;

		exerciseEntry() : nsets(0) {}
	};

	QList<exerciseEntry*> m_ExerciseData;
	bool mb_DayIsFinished, mb_DayIsEditable;
	QMap<uint, bool> m_CompositeExerciseList;

	friend class DBMesoSplitModel;
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
