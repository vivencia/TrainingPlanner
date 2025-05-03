#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#define WORKOUT_COL_ID 0
#define WORKOUT_COL_MESOID 1
#define WORKOUT_COL_CALENDARDAY 2
#define WORKOUT_COL_EXERCISES 3
#define WORKOUT_COL_SETTYPES 4
#define WORKOUT_COL_RESTTIMES 5
#define WORKOUT_COL_SUBSETS 6
#define WORKOUT_COL_REPS 7
#define WORKOUT_COL_WEIGHTS 8
#define WORKOUT_COL_NOTES 9
#define WORKOUT_COL_COMPLETED 10
#define WORKOUT_TOTALCOLS WORKOUT_COL_COMPLETED+1

#define TDDAY_MODEL_ROW 0
#define USE_LAST_SET_DATA 100

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoSplitModel);
QT_FORWARD_DECLARE_STRUCT(st_exercise);
QT_FORWARD_DECLARE_STRUCT(st_set);

class DBWorkoutModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)

public:
	explicit DBWorkoutModel(DBMesoCalendarManager *parent, const uint meso_idx, const uint calendar_day);
	~DBWorkoutModel() { for(uint i{0}; i < m_exerciseData.count(); ++i) delete m_exerciseData[i]; }

	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }

	void fromDataBase(const QStringList &list, const bool bClearSomeFieldsForReUse = false);
	const QStringList getSaveInfo() const;
	void convertMesoSplitModelToTDayModel(const DBMesoSplitModel *const splitModel);

	int exportToFile(const QString &filename) const;
	int importFromFile(const QString &filename);
	//bool updateFromModel(TPListModel *model) override;

	const QString exportExtraInfo() const;
	const QString formatSetTypeToExport(const QString &fieldValue) const;
	const QString formatSetTypeToImport(const QString &fieldValue) const;

	void newExercise(const uint exercise_number);
	void removeExercise(const uint exercise_number);
	Q_INVOKABLE void addSet(const uint row);
	Q_INVOKABLE void delSet(const uint row);
	void moveExercise(const uint from, const uint to);
	inline void clearExercises()
	{
		qDeleteAll(m_exerciseData);
		m_exerciseData.clear();
		emit workoutChanged();
	}

	const uint inline exerciseCount() const { return m_exerciseData.count(); }

	Q_INVOKABLE int addExercise();
	Q_INVOKABLE void delExercise(const uint exercise_number);
	Q_INVOKABLE int addSubExercise(const uint exercise_number);
	Q_INVOKABLE void delSubExercise(const uint exercise_number, const uint exercise_idx);
	Q_INVOKABLE bool exerciseIsComposite(const uint exercise_number) const;

	const uint setsNumber(const uint exercise_number) const;
	void newSet(const uint exercise_number, const uint type, const uint reps = 0, const uint weight = 0,
					const QTime &rest_time = QTime{0, 0, 0}, const uint sub_sets = 0);
	void removeSet(const uint exercise_number, const uint set_number);

	bool trackRestTime(const uint exercise_number, const uint exercise_idx = 0) const;
	void setTrackRestTime(const uint exercise_number, const uint exercise_idx, const bool track_resttime);
	bool autoRestTime(const uint exercise_number, const uint exercise_idx = 0) const;
	void setAutoRestTime(const uint exercise_number, const uint exercise_idx, const bool auto_resttime);

	Q_INVOKABLE QString exerciseName(const uint exercise_number, const uint exercise_idx = 0) const;
	Q_INVOKABLE void setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name);

	uint setType(const uint exercise_number, const uint set_number) const;
	void setSetType(const uint exercise_number, const uint set_number, const uint new_type);
	void changeSetType(const uint exercise_number, const uint set_number, const uint old_type, const uint new_type);

	QString setRestTime(const uint exercise_number, const uint set_number) const;
	void setSetRestTime(const uint exercise_number, const uint set_number, const QString &new_time);

	QString setSubSets(const uint exercise_number, const uint set_number) const;
	inline const QString setsSubSets(const uint exercise_number) const { return m_exerciseData.at(exercise_number)->subsets.join(record_separator); }
	void newSetSubSet(const uint exercise_number, const uint set_number);
	uint setSubSets_int(const uint exercise_number, const uint set_number) const { return setSubSets(set_number, exercise_number).toUInt(); }
	void setSetSubSets(const uint exercise_number, const uint set_number, const QString &new_subsets);

	QString setReps(const uint exercise_number, const uint set_number) const;
	inline const QString setsReps(const uint exercise_number) const { return m_exerciseData.at(exercise_number)->reps.join(record_separator); }
	uint setReps_int(const uint exercise_number, const uint set_number) const { return setReps(set_number, exercise_number).toUInt(); }
	void setSetReps(const uint exercise_number, const uint set_number, const QString &new_reps);

	QString setWeight(const uint exercise_number, const uint set_number) const;
	inline const QString setsWeight(const uint exercise_number) const { return m_exerciseData.at(exercise_number)->weight.join(record_separator); }
	void setSetWeight(const uint exercise_number, const uint set_number, const QString &new_weight);

	QString setNotes(const uint exercise_number, const uint set_number) const;
	inline const QString setsNotes(const uint exercise_number) const { return m_exerciseData.at(exercise_number)->notes.join(record_separator); }
	void setSetNotes(const uint exercise_number, const uint set_number, const QString &new_notes);

	QString setReps(const uint set_number, const uint subset, const uint exercise_number) const;
	void setSetReps(const uint exercise_number, const uint set_number, const uint subset, const QString &new_reps);

	QString setWeight(const uint set_number, const uint subset, const uint exercise_number) const;
	void setSetWeight(const uint exercise_number, const uint set_number, const uint subset, const QString &new_weight);

	bool setCompleted(const uint exercise_number, const uint set_number) const;
	void setSetCompleted(const uint exercise_number, const uint set_number, const bool completed);
	bool allSetsCompleted(const uint exercise_number) const;
	bool anySetCompleted(const uint exercise_number) const;

	QVariant data(const QModelIndex &index, int role) const override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return exerciseCount(); }

signals:
	void exerciseNameChanged(const int exercise_number);
	void setsNumberChanged(const int exercise_number);
	void setTypeChanged(const int exercise_number);
	void workingSetChanged(const int exercise_number);
	void workoutChanged();
	void exerciseCountChanged();
	void exerciseCompleted(const uint exercise_number, const bool completed);

private:
	DBMesoCalendarManager *m_calendarManager;
	QString m_id;
	uint m_mesoIdx, m_calendarDay;

	struct exerciseEntry {
		QList<st_exercise*> m_exercises;
	};

	QList<exerciseEntry*> m_exerciseData;

	friend class DBMesoSplitModel;
};
