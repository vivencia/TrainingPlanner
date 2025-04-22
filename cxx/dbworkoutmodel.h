#pragma once

#include "tplistmodel.h"

#define WORKOUT_COL_CALENDARID 0
#define WORKOUT_COL_EXERCISES 1
#define WORKOUT_COL_SETTYPES 2
#define WORKOUT_COL_RESTTIMES 3
#define WORKOUT_COL_SUBSETS 4
#define WORKOUT_COL_REPS 5
#define WORKOUT_COL_WEIGHTS 6
#define WORKOUT_COL_NOTES 7
#define WORKOUT_COL_COMPLETED 8
#define TDAY_EXERCISES_TOTALCOLS WORKOUT_COL_COMPLETED+1

#define TDDAY_MODEL_ROW 0
#define USE_LAST_SET_DATA 100

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoSplitModel);

class DBWorkoutModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)

public:
	explicit DBWorkoutModel(DBMesoCalendarModel *parent, const uint meso_idx);
	~DBWorkoutModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	void fromDataBase(const QStringList& list, const bool bClearSomeFieldsForReUse = false);
	const QStringList getSaveInfo() const;
	void convertMesoSplitModelToTDayModel(DBMesoSplitModel* const splitModel);

	int exportToFile(const QString& filename, const bool = true, const bool = true, const bool = false) const override;
	int importFromFile(const QString& filename) override;
	bool updateFromModel(TPListModel* model) override;

	const QString exportExtraInfo() const;
	const QString formatSetTypeToExport(const QString& fieldValue) const;
	const QString formatSetTypeToImport(const QString& fieldValue) const;

	inline void appendRow()
	{
		appendList_fast(std::move(QStringList(TDAY_TOTAL_COLS)));
		setId("-1"_L1);
	}

	inline void clearExercises()
	{
		const uint n_exercises(m_ExerciseData.count());
		if (n_exercises > 0)
		{
			for(uint i(0); i < n_exercises; ++i)
				delete m_ExerciseData[i];
			m_ExerciseData.clear();
			emit tDayChanged();
		}
	}

	void moveExercise(const uint from, const uint to);
	uint getWorkoutNumberForTrainingDay() const;

	const uint inline exerciseCount() const { return m_ExerciseData.count(); }
	inline const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }
	inline const QString _setsNumber(const uint exercise_idx) const { return QString::number(m_ExerciseData.at(exercise_idx)->nsets); }

	inline bool trackRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_TrackRestTime; }
	inline void setTrackRestTime(const uint exercise_idx, const bool track_resttime) { m_ExerciseData.at(exercise_idx)->mb_TrackRestTime = track_resttime; }

	inline bool autoRestTime(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->mb_AutoRestTime; }
	inline void setAutoRestTime(const uint exercise_idx, const bool auto_resttime) { m_ExerciseData.at(exercise_idx)->mb_AutoRestTime = auto_resttime; }

	inline const QString& exerciseName(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->name; }
	inline void setExerciseName(const uint exercise_idx, const QString& new_name) { m_ExerciseData.at(exercise_idx)->name = new_name; }
	void newExercise(const uint exercise_idx);
	void removeExercise(const uint exercise_idx);

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
		uint nsets;
		QString exercise;
		QStringList settype;
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
