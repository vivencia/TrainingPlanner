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
#define USE_LAST_SET_DATA 100

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

	inline void appendRow() { appendList_fast(std::move(QStringList(TDAY_TOTAL_COLS))); setId("-1"_L1); }
	void moveExercise(const uint from, const uint to);
	uint getWorkoutNumberForTrainingDay() const;

	inline const int id() const { return count() == 1 ? idStr().toInt() : -1; }
	inline const QString& idStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_ID); }
	inline void setId(const QString& new_id) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_ID] = new_id; }

	inline const int mesoId() const { return count() == 1 ? mesoIdStr().toInt(): -1; }
	inline const QString& mesoIdStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_MESOID); }
	inline void setMesoId(const QString& mesoid) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_MESOID] = mesoid; }

	QDate date() const { return count() == 1 ? QDate::fromJulianDay(dateStr().toLongLong()) : QDate::currentDate(); }
	inline const QString& dateStr() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_DATE); }
	inline void setDate(const QDate& date)
	{
		m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_DATE] = std::move(QString::number(date.toJulianDay()));
	}
	inline void setDateStr(const QString& date_str) { m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_DATE] = date_str; }

	inline const QString& trainingDay() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER); }
	inline void setTrainingDay(const QString& trainingday, const bool bEmitSave = true)
	{
		if (trainingday != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TRAININGDAYNUMBER))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TRAININGDAYNUMBER] = trainingday;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline QChar _splitLetter() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER).at(0); }
	inline const QString& splitLetter() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER); }
	inline void setSplitLetter(const QString& splitletter, const bool bEmitSave = true)
	{
		if (splitletter != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_SPLITLETTER))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_SPLITLETTER] = splitletter;
			emit splitLetterChanged();
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline const QString& timeIn() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEIN); }
	inline void setTimeIn(const QString& timein, const bool bEmitSave = true)
	{
		if (timein != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEIN))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TIMEIN] = timein;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline const QString& timeOut() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEOUT); }
	inline void setTimeOut(const QString& timeout, const bool bEmitSave = true)
	{
		if (timeout != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_TIMEOUT))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_TIMEOUT] = timeout;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline const QString& location() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_LOCATION); }
	inline void setLocation(const QString& location, const bool bEmitSave = true)
	{
		if (location != m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_LOCATION))
		{
			m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_LOCATION] = location;
			if (bEmitSave)
				emit tDayChanged();
		}
	}

	inline const QString& dayNotes() const { return m_modeldata.at(TDDAY_MODEL_ROW).at(TDAY_COL_NOTES); }
	inline void setDayNotes(const QString& day_notes, const bool bEmitSave = true)
	{
		m_modeldata[TDDAY_MODEL_ROW][TDAY_COL_NOTES] = day_notes;
		if (bEmitSave)
			emit tDayChanged();
	}

	const uint inline exerciseCount() const { return m_ExerciseData.count(); }
	inline const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }
	inline const QString _setsNumber(const uint exercise_idx) const { return QString::number(m_ExerciseData.at(exercise_idx)->nsets); }

	inline bool trackRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_TrackRestTime; }
	inline void setTrackRestTime(const uint exercise_idx, const bool track_resttime) { m_ExerciseData[exercise_idx]->mb_TrackRestTime = track_resttime; }

	inline bool autoRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_AutoRestTime; }
	inline void setAutoRestTime(const uint exercise_idx, const bool auto_resttime) { m_ExerciseData[exercise_idx]->mb_AutoRestTime = auto_resttime; }

	QString exerciseName(const uint exercise_idx) const;
	inline const QString& _exerciseName(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->name; }

	void setExerciseName(const uint exercise_idx, const QString& new_name);
	void newExercise(const uint exercise_idx, const QString& new_exercise);
	void removeExercise(const uint exercise_idx);
	void changeExerciseName(const uint exercise_idx, DBExercisesModel* model);

	QString exerciseName1(const uint exercise_idx) const;
	void setExerciseName1(const uint exercise_idx, const QString& name1);

	QString exerciseName2(const uint exercise_idx) const;
	void setExerciseName2(const uint exercise_idx, const QString& name2);

	void newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight, const QString& nRestTime,
					 const QString& nSubsets = "0"_L1, const QString& notes = " "_L1);
	QString nextSetSuggestedTime(const uint exercise_idx, const uint type, const uint set_number = 100) const;
	const QString nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	const QString nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	void newSet(const uint exercise_idx, const uint set_number, const uint type, const QString& nReps = QString(),
					const QString& nWeight = QString(), const QString& nRestTime = QString(), const QString& nSubSets = QString());
	void removeSet(const uint exercise_idx, const uint set_number);

	uint setType(const uint exercise_idx, const uint set_number) const;
	inline const QString setsTypes(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->type.join(record_separator); }

	void setSetType(const uint exercise_idx, const uint set_number, const uint new_type);
	void changeSetType(const uint exercise_idx, const uint set_number, const uint old_type, const uint new_type);

	QString setRestTime(const uint exercise_idx, const uint set_number) const;
	void setSetRestTime(const uint exercise_idx, const uint set_number, const QString& new_time);

	QString setSubSets(const uint exercise_idx, const uint set_number) const;
	inline const QString setsSubSets(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->subsets.join(record_separator); }
	void newSetSubSet(const uint exercise_idx, const uint set_number);
	uint setSubSets_int(const uint exercise_idx, const uint set_number) const { return setSubSets(set_number, exercise_idx).toUInt(); }
	void setSetSubSets(const uint exercise_idx, const uint set_number, const QString& new_subsets);

	QString setReps(const uint exercise_idx, const uint set_number) const;
	inline const QString setsReps(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->reps.join(record_separator); }
	uint setReps_int(const uint exercise_idx, const uint set_number) const { return setReps(set_number, exercise_idx).toUInt(); }
	void setSetReps(const uint exercise_idx, const uint set_number, const QString& new_reps);

	QString setWeight(const uint exercise_idx, const uint set_number) const;
	inline const QString setsWeight(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->weight.join(record_separator); }
	void setSetWeight(const uint exercise_idx, const uint set_number, const QString& new_weight);

	QString setNotes(const uint exercise_idx, const uint set_number) const;
	inline const QString setsNotes(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->notes.join(record_separator); }
	void setSetNotes(const uint exercise_idx, const uint set_number, const QString& new_notes);

	QString setReps(const uint set_number, const uint subset, const uint exercise_idx) const;
	void setSetReps(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_reps);

	QString setWeight(const uint set_number, const uint subset, const uint exercise_idx) const;
	void setSetWeight(const uint exercise_idx, const uint set_number, const uint subset, const QString& new_weight);

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

		inline exerciseEntry() : nsets(0), mb_TrackRestTime(false), mb_AutoRestTime(false) {}
	};

	QList<exerciseEntry*> m_ExerciseData;

	friend class DBMesoSplitModel;
};

//Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
