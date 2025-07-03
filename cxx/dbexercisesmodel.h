#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#define EXERCISES_COL_ID 0
#define EXERCISES_COL_MESOID 1
#define EXERCISES_COL_CALENDARDAY 2
#define EXERCISES_COL_SPLITLETTER 3
#define EXERCISES_COL_EXERCISES 4
#define EXERCISES_COL_TRACKRESTTIMES 5
#define EXERCISES_COL_AUTORESTTIMES 6
#define EXERCISES_COL_SETTYPES 7
#define EXERCISES_COL_RESTTIMES 8
#define EXERCISES_COL_SUBSETS 9
#define EXERCISES_COL_REPS 10
#define EXERCISES_COL_WEIGHTS 11
#define EXERCISES_COL_NOTES 12
#define EXERCISES_COL_COMPLETED 13
#define WORKOUT_TOTALCOLS EXERCISES_COL_COMPLETED+1

#define EXERCISE_IGNORE_NOTIFY_IDX 1000
#define EXERCISE_ADD_NOTIFY_IDX 100
#define EXERCISE_DEL_NOTIFY_IDX 101
#define EXERCISE_MOVE_NOTIFY_IDX 102


QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_STRUCT(exerciseEntry)
QT_FORWARD_DECLARE_STRUCT(stSet)

QT_FORWARD_DECLARE_CLASS(QFile)

enum {
	Unkown = -1,
	Regular = 0,
	Pyramid = 1,
	ReversePyramid = 2,
	Drop =  3,
	Cluster = 4,
	MyoReps = 5
} typedef TPSetTypes;

class DBExercisesModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)
Q_PROPERTY(int workingExercise READ workingExercise WRITE setWorkingExercise NOTIFY workingExerciseChanged)
Q_PROPERTY(int workingSubExercise READ workingSubExercise WRITE setWorkingSubExercise NOTIFY workingSubExerciseChanged)
Q_PROPERTY(int workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged)
Q_PROPERTY(bool isWorkout READ isWorkout CONSTANT FINAL)

Q_PROPERTY(QChar splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged FINAL)
Q_PROPERTY(QString muscularGroup READ muscularGroup NOTIFY muscularGroupChanged FINAL)
Q_PROPERTY(QString totalSetsLabel READ totalSetsLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setNumberLabel READ setNumberLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString trackRestTimeLabel READ trackRestTimeLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString autoRestTimeLabel READ autoRestTimeLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setTypeLabel READ setTypeLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setRestTimeLabel READ setRestTimeLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setTotalSubsets READ setTotalSubsets NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setRepsLabel READ setRepsLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setWeightLabel READ setWeightLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setNotesLabel READ setNotesLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setCompletedLabel READ setCompletedLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString restTimeUntrackedLabel READ restTimeUntrackedLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabel READ splitLabel NOTIFY labelChanged FINAL)

public:
	inline explicit DBExercisesModel(DBMesoCalendarManager *parent, const uint meso_idx, const int calendar_day)
		: QAbstractListModel{reinterpret_cast<QObject*>(parent)},
			m_calendarManager{parent}, m_mesoIdx{meso_idx}, m_calendarDay{calendar_day}, m_splitLetter{'N'}, m_workingExercise{11111}
	{
		commonConstructor();
	}
	inline explicit DBExercisesModel(DBMesoCalendarManager *parent, const uint meso_idx, const QChar &splitletter)
		: QAbstractListModel{reinterpret_cast<QObject*>(parent)},
			m_calendarManager{parent}, m_mesoIdx{meso_idx}, m_calendarDay{-1}, m_splitLetter{splitletter}, m_workingExercise{11111}
	{
		commonConstructor();
	}
	~DBExercisesModel() { clearExercises(); }

	void operator=(DBExercisesModel *other_model);

	[[nodiscard]] inline DBMesoCalendarManager *calendarManager() const { return m_calendarManager; }
	[[nodiscard]] inline const QString &id() const {return m_id; }
	inline void setId(const QString &new_id) { m_id = new_id; }
	[[nodiscard]] inline const QString &mesoId() const {return m_mesoId; }
	inline void setMesoId(const QString &new_mesoid) { m_mesoId = new_mesoid; }
	[[nodiscard]] inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	[[nodiscard]] inline int calendarDay() const { return m_calendarDay; }
	inline void setCalendarDay(const uint new_calendarday) { m_calendarDay = new_calendarday; }
	[[nodiscard]] inline const QChar &splitLetter() const { return m_splitLetter; }
	inline void setSplitLetter(const QChar &new_splitletter)
	{
		if (m_splitLetter != new_splitletter)
		{
			m_splitLetter = new_splitletter;
			emit splitLetterChanged();
		}
	}
	[[nodiscard]] inline bool importModel() const { return m_importMode; }
	inline void setImportMode(const bool import_mode) { m_importMode = import_mode; }

	[[nodiscard]] bool fromDataBase(const QStringList &data, const bool bClearSomeFieldsForReUse = false);
	[[nodiscard]] const QStringList toDatabase(const bool to_export_file = false) const;
	void clearExercises();

	[[nodiscard]] int exportToFile(const QString &filename, QFile *out_file = nullptr) const;
	[[nodiscard]] int exportToFormattedFile(const QString &filename, QFile *out_file = nullptr) const;
	[[nodiscard]] int importFromFile(const QString &filename, QFile *in_file = nullptr);
	[[nodiscard]] int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	[[maybe_unused]] int newExercisesFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);
	[[nodiscard]] inline const QString &identifierInFile() const { return *m_identifierInFile; }
	[[nodiscard]] const QString formatSetTypeToExport(const uint type) const;
	[[nodiscard]] static bool importExtraInfo(const QString &maybe_extra_info, int &calendar_day, QChar &split_letter);

	inline const bool isWorkout() const { return m_calendarDay != -1; }
	const uint inline exerciseCount() const { return m_exerciseData.count(); }
	Q_INVOKABLE const uint subExercisesCount(const uint exercise_number) const;
	Q_INVOKABLE const uint setsNumber(const uint exercise_number, const uint exercise_idx) const;

	QString muscularGroup() const;
	[[maybe_unused]] Q_INVOKABLE uint addExercise(const bool emit_signal = true);
	Q_INVOKABLE void delExercise(const uint exercise_number, const bool emit_signal = true);
	Q_INVOKABLE void moveExercise(const uint from, const uint to);
	[[maybe_unused]] Q_INVOKABLE uint addSubExercise(const uint exercise_number, const bool emit_signal = true);
	Q_INVOKABLE void delSubExercise(const uint exercise_number, const uint exercise_idx, const bool emit_signal = true);
	[[maybe_unused]] Q_INVOKABLE uint addSet(const uint exercise_number, const uint exercise_idx, const bool emit_signal = true);
	Q_INVOKABLE void delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool emit_signal = true);
	void moveSet(const uint exercise_number, const uint exercise_idx, const uint from_set, const uint to_set);
	Q_INVOKABLE bool exerciseIsComposite(const uint exercise_number) const;

	[[nodiscard]] inline uint workingExercise() const { return m_workingExercise; }
	void setWorkingExercise(const uint new_workingexercise);
	[[nodiscard]] uint workingSubExercise(int exercise_number = -1) const;
	void setWorkingSubExercise(const uint new_workingsubexercise, int exercise_number = -1);
	[[nodiscard]] uint workingSet(int exercise_number = -1, int exercise_idx = -1) const;
	void setWorkingSet(const uint new_workingset, int exercise_number = -1, int exercise_idx = -1);

	[[nodiscard]] Q_INVOKABLE QString exerciseName(const uint exercise_number, const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name);
	void setExerciseName(const uint exercise_number, const uint exercise_idx, QString &&new_name);

	[[nodiscard]] Q_INVOKABLE bool trackRestTime(const uint exercise_number) const;
	Q_INVOKABLE void setTrackRestTime(const uint exercise_number, const bool track_resttime);
	[[nodiscard]] Q_INVOKABLE bool autoRestTime(const uint exercise_number) const;
	Q_INVOKABLE void setAutoRestTime(const uint exercise_number, const bool auto_resttime);

	[[nodiscard]] Q_INVOKABLE int setType(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type, const bool emit_signal = true);
	void changeSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type);

	[[nodiscard]] QTime suggestedRestTime(const QTime &prev_resttime, const uint set_type) const;
	[[nodiscard]] QTime restTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	[[nodiscard]] Q_INVOKABLE QString setRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_time);

	[[nodiscard]] QString suggestedSubSets(const uint set_type);
	[[nodiscard]] Q_INVOKABLE QString setSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_subsets);
	void addSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number);
	void delSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number);

	[[nodiscard]] QString suggestedReps(const QString &prev_reps, const uint set_type, const uint subset = 0) const;
	[[nodiscard]] Q_INVOKABLE QString setReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset = 0) const;
	Q_INVOKABLE void setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_reps, const uint subset = 0);
	void setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_reps);

	[[nodiscard]] QString suggestedWeight(const QString &prev_weight, const uint set_type, const uint subset = 0) const;
	[[nodiscard]] Q_INVOKABLE QString setWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset = 0) const;
	Q_INVOKABLE void setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_weight, const uint subset = 0);
	void setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_weight);

	[[nodiscard]] Q_INVOKABLE QString setNotes(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_notes);
	void setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_notes);

	[[nodiscard]] Q_INVOKABLE bool setCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed);
	[[nodiscard]] bool allSetsCompleted(int exercise_number = -1, int exercise_idx = -1) const;
	[[nodiscard]] bool anySetCompleted(int exercise_number = -1, int exercise_idx = -1) const;
	[[nodiscard]] bool noSetsCompleted(int exercise_number = -1, int exercise_idx = -1) const;

	[[nodiscard]] Q_INVOKABLE uint setMode(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	void setSetMode(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint mode);
	void setSetNextMode(const uint exercise_number, const uint exercise_idx, const uint set_number);
	[[nodiscard]] Q_INVOKABLE QString setModeLabel(const uint exercise_number, const uint exercise_idx, const uint set_number) const;

	inline QString totalSetsLabel() const { return tr("Number of sets: "); }
	inline QString setNumberLabel() const { return tr("Set #: "); }
	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString trackRestTimeLabel() const { return tr("Track rest times: "); }
	inline QString autoRestTimeLabel() const { return tr("Auto tracking: "); }
	inline QString setTypeLabel() const { return tr("Set type: "); }
	inline QString setRestTimeLabel() const { return tr("Rest time: "); }
	inline QString setTotalSubsets() const { return tr("Number of subsets: "); }
	inline QString setRepsLabel() const { return tr("Repetitions: "); }
	inline QString setWeightLabel() const { return tr("Weight: "); }
	inline QString setNotesLabel() const { return tr("Notes/Instructions for the set: "); }
	inline QString setCompletedLabel() const { return tr("Completed?"); }
	inline QString restTimeUntrackedLabel() const { return tr("As needed"); }
	static inline QString splitLabel() { return tr("Split: "); }

	void clearPreviousWorkouts() { m_previousWorkouts.clear(); }
	inline void appendPreviousWorkout(const uint calendar_day) { m_previousWorkouts.append(calendar_day); }
	const QList<uint> &previousWorkouts() const { return m_previousWorkouts; }

	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return exerciseCount(); }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

public slots:
	void newExerciseFromExercisesList();

signals:
	void setModeChanged(const int exercise_number, const int exercise_idx, const int set_number, const int mode);
	void splitLetterChanged();
	void muscularGroupChanged();
	void exerciseNameChanged(const int exercise_number, const int exercise_idx);
	void setsNumberChanged(const int exercise_number, const int exercise_idx);
	void setTypeChanged(const int exercise_number, const int exercise_idx, const int set_number);
	void workingExerciseChanged(const int exercise_number);
	void workingSubExerciseChanged(const int exercise_number, const int exercise_idx);
	void workingSetChanged(const int exercise_number, const int exercise_idx, const int set_number);
	void exerciseCountChanged();
	void subExerciseCountChanged(const int exercise_number);
	void exerciseCompleted(const int exercise_number, const bool completed);
	void labelChanged();
	void exerciseModified(const int exercise_number, const int exercise_idx, const int set_number, const int field);

private:
	DBMesoCalendarManager *m_calendarManager;
	QString m_id, m_mesoId;
	const QString *m_identifierInFile;
	uint m_mesoIdx, m_workingExercise;
	int m_calendarDay;
	bool m_importMode;
	QChar m_splitLetter;
	QList<exerciseEntry*> m_exerciseData;
	QHash<int, QByteArray> m_roleNames;
	QList<uint> m_previousWorkouts;

	void commonConstructor();
	TPSetTypes formatSetTypeToImport(const QString &fieldValue) const;
	const QString exportExtraInfo() const;
	inline bool importExtraInfo(const QString &maybe_extra_info);
	void setModeForSet(const uint exercise_number, const uint exercise_idx, const uint set_number);
	QString increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs);
	void setSuggestedTime(const uint set_number, const QList<stSet*> &sets);
	void setSuggestedSubSets(const uint set_number, const QList<stSet*> &sets);
	void setSuggestedReps(const uint set_number, const QList<stSet*> &sets, const uint from_subset = 0);
	void setSuggestedWeight(const uint set_number, const QList<stSet *> &sets, const uint from_subset = 0);
	QString dropSetReps(const QString &reps, const uint from_subset = 0) const;
	QString clusterReps(const QString &total_reps, const uint from_subset = 0) const;
	QString myorepsReps(const QString &first_set_reps, const uint n_sets, const uint from_set = 0) const;
	QString dropSetWeight(const QString &weight, const uint from_subset = 0) const;
	QString clusterWeight(const QString &constant_weight, const uint from_subset = 0) const;
	QString myorepsWeight(const QString &first_set_weight, const uint n_sets, const uint from_set = 0)const;
};
