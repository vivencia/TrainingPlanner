#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"
#include "tpglobals.h"

#define TDAY_COL_ID 0
#define TDAY_COL_MESOID 1
#define TDAY_COL_DATE 2
#define TDAY_COL_TRAININGDAYNUMBER 3
#define TDAY_COL_SPLITLETTER 4
#define TDAY_COL_TIMEIN 5
#define TDAY_COL_TIMEOUT 6
#define TDAY_COL_LOCATION 7
#define TDAY_COL_NOTES 8
#define TDAY_TOTAL_COLS TDAY_COL_NOTES + 1

#define TDAY_EXERCISES_COL_NAMES 0
#define TDAY_EXERCISES_COL_TYPES 1
#define TDAY_EXERCISES_COL_RESTTIMES 2
#define TDAY_EXERCISES_COL_SUBSETS 3
#define TDAY_EXERCISES_COL_REPS 4
#define TDAY_EXERCISES_COL_WEIGHTS 5
#define TDAY_EXERCISES_COL_NOTES 6
#define TDAY_EXERCISES_COL_COMPLETED 7
#define TDAY_EXERCISES_TOTALCOLS TDAY_EXERCISES_COL_COMPLETED+1

#define TDDAY_MODEL_ROW 0

class DBExercisesModel;
class DBMesoSplitModel;
class appWindowMessageID;

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)
Q_PROPERTY(QString splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged FINAL)

public:
	explicit DBTrainingDayModel(QObject* parent, const uint meso_idx = 0);
	~DBTrainingDayModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	inline void clearExercises() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; m_ExerciseData.clear(); }
	void fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse = false);
	const QStringList getSaveInfo() const;
	void convertMesoSplitModelToTDayModel(DBMesoSplitModel* const splitModel);

	int exportToFile(const QString& filename, const bool = true, const bool = true) const override;
	int importFromFile(const QString& filename) override;
	bool updateFromModel(const TPListModel* const model) override;

	const QString exportExtraInfo() const;
	const QString formatSetTypeToExport(const QString& fieldValue) const;
	const QString formatSetTypeToImport(const QString& fieldValue) const;

	inline void appendRow() { appendList(QStringList(TDAY_TOTAL_COLS)); setId(u"-1"_s); }
	void moveExercise(const uint from, const uint to);
	Q_INVOKABLE uint getWorkoutNumberForTrainingDay() const;

	Q_INVOKABLE inline const int id() const { return count() == 1 ? idStr().toInt() : -1; }
	inline const QString& idStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_ID); }
	inline void setId(const QString& new_id) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_ID] = new_id; }

	Q_INVOKABLE inline const int mesoId() const { return count() == 1 ? mesoIdStr().toInt(): -1; }
	inline const QString& mesoIdStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_MESOID); }
	Q_INVOKABLE inline void setMesoId(const QString& mesoid) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_MESOID] = mesoid; }

	Q_INVOKABLE QDate date() const { return count() == 1 ? QDate::fromJulianDay(dateStr().toLongLong()) : QDate::currentDate(); }
	inline const QString& dateStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_DATE); }
	Q_INVOKABLE void setDate(const QDate& date, const bool bEmitSave = true)
	{
		m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_DATE] = QString::number(date.toJulianDay());
		if (bEmitSave)
			emit tDayChanged();
	}
	inline void setDateStr(const QString& date_str) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_DATE] = date_str; }

	Q_INVOKABLE inline QString trainingDay() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER); }
	//Do not set model's modified to true because this is called from onTextChanged on TrainingDayInfo.qml, which gets called when the property
	//is changed even for the first time, i.e., when the page is receiving default values
	Q_INVOKABLE inline void setTrainingDay(const QString& trainingday, const bool bEmitSave = true)
	{
		if (trainingday != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TRAININGDAYNUMBER] = trainingday;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline QChar _splitLetter() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER).at(0); }
	Q_INVOKABLE inline QString splitLetter() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER); }
	Q_INVOKABLE inline void setSplitLetter(const QString& splitletter, const bool bEmitSave = true)
	{
		if (splitletter != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_SPLITLETTER] = splitletter;
			emit splitLetterChanged();
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	Q_INVOKABLE inline QString timeIn() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEIN); }
	Q_INVOKABLE inline void setTimeIn(const QString& timein, const bool bEmitSave = true)
	{
		if (timein != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEIN))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TIMEIN] = timein;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	Q_INVOKABLE inline QString timeOut() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEOUT); }
	Q_INVOKABLE inline void setTimeOut(const QString& timeout, const bool bEmitSave = true)
	{
		if (timeout != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEOUT))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TIMEOUT] = timeout;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	Q_INVOKABLE inline QString location() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_LOCATION); }
	Q_INVOKABLE inline void setLocation(const QString& location, const bool bEmitSave = true)
	{
		if (location != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_LOCATION))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_LOCATION] = location;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	Q_INVOKABLE inline QString dayNotes() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_NOTES); }
	Q_INVOKABLE inline void setDayNotes(const QString& day_notes, const bool bEmitSave = true)
	{
		m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_NOTES] = day_notes;
		if (bEmitSave)
			emit tDayChanged();
	}

	const uint inline exerciseCount() const { return m_ExerciseData.count(); }
	Q_INVOKABLE inline const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }
	inline const QString _setsNumber(const uint exercise_idx) const { return QString::number(m_ExerciseData.at(exercise_idx)->nsets); }

	Q_INVOKABLE inline bool trackRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_TrackRestTime; }
	Q_INVOKABLE inline void setTrackRestTime(const uint exercise_idx, const bool track_resttime) { m_ExerciseData[exercise_idx]->mb_TrackRestTime = track_resttime; }

	Q_INVOKABLE inline bool autoRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_AutoRestTime; }
	Q_INVOKABLE inline void setAutoRestTime(const uint exercise_idx, const bool auto_resttime) { m_ExerciseData[exercise_idx]->mb_AutoRestTime = auto_resttime; }

	Q_INVOKABLE QString exerciseName(const uint exercise_idx) const;
	inline const QString& _exerciseName(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->name; }

	Q_INVOKABLE void setExerciseName(const uint exercise_idx, const QString& new_name);
	void newExercise(const uint exercise_idx, const QString& new_exercise);
	void removeExercise(const uint exercise_idx);
	Q_INVOKABLE void changeExerciseName(const uint exercise_idx, DBExercisesModel* model);

	Q_INVOKABLE QString exerciseName1(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName1(const uint exercise_idx, const QString& name1);

	Q_INVOKABLE QString exerciseName2(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName2(const uint exercise_idx, const QString& name2);

	void newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight, const QString& nRestTime,
					 const QString& nSubsets = u"0"_s, const QString& notes = u" "_s);
	Q_INVOKABLE QString nextSetSuggestedTime(const uint exercise_idx, const uint type, const uint set_number = 100) const;
	const QString nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	const QString nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	void newSet(const uint exercise_idx, const uint set_number, const uint type, const QString& nReps = QString(),
					const QString& nWeight = QString(), const QString& nRestTime = QString(), const QString& nSubSets = QString());
	void removeSet(const uint exercise_idx, const uint set_number);

	Q_INVOKABLE uint setType(const uint exercise_idx, const uint set_number) const;
	inline const QString setsTypes(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->type.join(record_separator); }

	Q_INVOKABLE void setSetType(const uint exercise_idx, const uint set_number, const uint new_type);
	void changeSetType(const uint exercise_idx, const uint set_number, const uint old_type, const uint new_type);

	Q_INVOKABLE QString setRestTime(const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetRestTime(const uint exercise_idx, const uint set_number, const QString& new_time);

	Q_INVOKABLE QString setSubSets(const uint exercise_idx, const uint set_number) const;
	inline const QString setsSubSets(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->subsets.join(record_separator); }
	Q_INVOKABLE void newSetSubSet(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE uint setSubSets_int(const uint exercise_idx, const uint set_number) const { return setSubSets(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetSubSets(const uint exercise_idx, const uint set_number, const QString& new_subsets);

	Q_INVOKABLE QString setReps(const uint exercise_idx, const uint set_number) const;
	inline const QString setsReps(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->reps.join(record_separator); }
	Q_INVOKABLE uint setReps_int(const uint exercise_idx, const uint set_number) const { return setReps(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetReps(const uint exercise_idx, const uint set_number, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint exercise_idx, const uint set_number) const;
	inline const QString setsWeight(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->weight.join(record_separator); }
	Q_INVOKABLE void setSetWeight(const uint exercise_idx, const uint set_number, const QString& new_weight);

	Q_INVOKABLE QString setNotes(const uint exercise_idx, const uint set_number) const;
	inline const QString setsNotes(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->notes.join(record_separator); }
	Q_INVOKABLE void setSetNotes(const uint exercise_idx, const uint set_number, const QString& new_notes);

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetReps(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_weight);

	bool setCompleted(const uint exercise_idx, const uint set_number) const;
	void setSetCompleted(const uint exercise_idx, const uint set_number, const bool completed);
	bool allSetsCompleted(const uint exercise_idx) const;
	bool anySetCompleted(const uint exercise_idx) const;

signals:
	void tDayChanged();
	void exerciseCountChanged();
	void splitLetterChanged();
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

	friend class DBMesoSplitModel;
};

//Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
