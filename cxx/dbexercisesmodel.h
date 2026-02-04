#pragma once

#include "dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

enum ExercisesSheetFields {
	EXERCISES_FIELD_ID,
	EXERCISES_FIELD_MESOID,
	EXERCISES_FIELD_CALENDARDAY,
	EXERCISES_FIELD_SPLITLETTER,
	EXERCISES_FIELD_TRACKRESTTIMES,
	EXERCISES_FIELD_AUTORESTTIMES,
	EXERCISES_FIELD_EXERCISES,
	EXERCISES_FIELD_NOTES,
	EXERCISES_FIELD_COMPLETED,
	EXERCISES_FIELD_SETTYPES,
	EXERCISES_FIELD_RESTTIMES,
	EXERCISES_FIELD_SUBSETS,
	EXERCISES_FIELD_REPS,
	EXERCISES_FIELD_WEIGHTS,
	EXERCISES_TOTALCOLS
};

constexpr uint8_t EXERCISE_IGNORE_NOTIFY_IDX{255};

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBWorkoutsOrSplitsTable)
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
	MyoReps = 5,
} typedef TPSetTypes;

enum SetMode {
	SM_NOT_COMPLETED = 1,
	SM_START_REST = 2,
	SM_START_EXERCISE = 3,
	SM_COMPLETED = 4,
};

class DBModelInterfaceExercises : public DBModelInterface
{

public:
	explicit DBModelInterfaceExercises(DBExercisesModel *model);
	inline const QList<QStringList> &modelData() const { return m_modelData; }
	inline QList<QStringList> &modelData() { return m_modelData; }

private:
	QList<QStringList> m_modelData;
};

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
	//DBWorkoutModel
	inline explicit DBExercisesModel(DBMesocyclesModel *meso_model, DBWorkoutsOrSplitsTable* db,
																				const uint meso_idx, const int calendar_day)
		: QAbstractListModel{reinterpret_cast<QObject*>(meso_model)}, m_mesoModel{meso_model}, m_db{db},
			m_mesoIdx{meso_idx}, m_calendarDay{calendar_day}, m_splitLetter{'N'}, m_workingExercise{11111}, m_exercisesLoaded{false}
	{
		commonConstructor(true);
	}
	//DBSplitModel, no need for a calendar manager
	inline explicit DBExercisesModel(DBMesocyclesModel *meso_model, DBWorkoutsOrSplitsTable *db,
														const uint meso_idx, const QChar &splitletter, const bool load_from_db)
		: QAbstractListModel{reinterpret_cast<QObject*>(meso_model)}, m_db{db}, m_mesoModel{meso_model},
			m_mesoIdx{meso_idx}, m_calendarDay{-1}, m_splitLetter{splitletter}, m_workingExercise{11111}
	{
		commonConstructor(load_from_db);
	}
	~DBExercisesModel() { clearExercises(false); }
	inline DBModelInterfaceExercises *dbModelInterface() const { return m_dbModelInterface; }
	DBWorkoutsOrSplitsTable *database() const;
	void plugDBModelInterfaceIntoDatabase();

	void operator=(DBExercisesModel *other_model);
	bool fromDatabase(const bool db_data_ok);
	Q_INVOKABLE void clearExercises(const bool from_qml = true);

	[[nodiscard]] inline const bool exercisesLoaded() const { return m_exercisesLoaded; }
	[[nodiscard]] inline const QString &id() const { return m_dbModelInterface->modelData().at(0).at(EXERCISES_FIELD_ID); }
	[[nodiscard]] const QString &mesoId() const;
	[[nodiscard]] inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	[[nodiscard]] inline int calendarDay() const { return m_calendarDay; }
	inline void setCalendarDay(const uint new_calendarday) { m_calendarDay = new_calendarday; }
	[[nodiscard]] inline const QChar &splitLetter() const { return m_splitLetter; }
	void setSplitLetter(const QChar &new_splitletter);
	inline void setImportMode(const bool import_mode) { m_importMode = import_mode; }

	[[nodiscard]] inline const bool isWorkout() const { return m_calendarDay != -1; }
	[[nodiscard]] int exportToFile(const QString &filename, QFile *out_file = nullptr) const;
	[[nodiscard]] int exportToFormattedFile(const QString &filename, QFile *out_file = nullptr) const;
	[[nodiscard]] int importFromFile(const QString &filename, QFile *in_file = nullptr);
	[[nodiscard]] int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	[[maybe_unused]] int newExercisesFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);
	[[nodiscard]] inline const QString &identifierInFile() const { return *m_identifierInFile; }
	[[nodiscard]] const QString formatSetTypeToExport(const uint type) const;
	[[nodiscard]] static bool importExtraInfo(const QString &maybe_extra_info, int &calendar_day, QChar &split_letter);

	[[nodiscard]] QString muscularGroup() const;
	[[nodiscard]] const uint inline exerciseCount() const { return m_exerciseData.count(); }
	Q_INVOKABLE const uint subExercisesCount(const uint exercise_number) const;
	Q_INVOKABLE const uint setsNumber(const uint exercise_number, const uint exercise_idx) const;

	[[maybe_unused]] Q_INVOKABLE uint addExercise(int exercise_number = -1, const bool from_qml = true);
	Q_INVOKABLE void delExercise(const uint exercise_number, const bool from_qml = true);
	Q_INVOKABLE void moveExercise(const uint from, const uint to);
	[[maybe_unused]] Q_INVOKABLE void addSubExercise(const uint exercise_number, const bool from_qml = true);
	Q_INVOKABLE void delSubExercise(const uint exercise_number, const uint exercise_idx, const bool from_qml = true);
	[[maybe_unused]] Q_INVOKABLE uint addSet(const uint exercise_number, const uint exercise_idx, const bool from_qml = true);
	Q_INVOKABLE void delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool from_qml = true);
	Q_INVOKABLE void removeAllSets(const uint exercise_number, const uint exercise_idx, const bool from_qml = true);
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
	[[nodiscard]] Q_INVOKABLE QString allExerciseNames(const uint exercise_number) const;

	[[nodiscard]] Q_INVOKABLE bool trackRestTime(const uint exercise_number) const;
	Q_INVOKABLE void setTrackRestTime(const uint exercise_number, const bool track_resttime);
	[[nodiscard]] Q_INVOKABLE bool autoRestTime(const uint exercise_number) const;
	Q_INVOKABLE void setAutoRestTime(const uint exercise_number, const bool auto_resttime);

	[[nodiscard]] Q_INVOKABLE int setType(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type, const bool from_qml = true);

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

	[[nodiscard]] QString suggestedWeight(const QString &prev_weight, const uint set_type, const uint n_sets, const uint subset = 0) const;
	[[nodiscard]] Q_INVOKABLE QString setWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset = 0) const;
	Q_INVOKABLE void setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_weight, const uint subset = 0);

	[[nodiscard]] Q_INVOKABLE QString setNotes(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_notes);

	[[nodiscard]] Q_INVOKABLE bool setCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed);
	[[nodiscard]] Q_INVOKABLE bool allSetsCompleted(int exercise_number = -1, int exercise_idx = -1) const;
	void setAllSetsCompleted(const bool completed, int exercise_number = -1, int exercise_idx = -1);

	[[nodiscard]] Q_INVOKABLE uint setMode(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	void setSetMode(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint mode);
	[[nodiscard]] uint getSetNextMode(const uint exercise_number, const uint exercise_idx, const uint set_number);
	[[nodiscard]] Q_INVOKABLE QString setModeLabel(const uint exercise_number, const uint exercise_idx, const uint set_number) const;

	[[nodiscard]] Q_INVOKABLE bool syncGiantSets(const uint exercise_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSyncGiantSets(const uint exercise_number, const uint exercise_idx, const bool sync);

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

	QVariant data(const QModelIndex &index, int role) const override final;
	[[maybe_unused]] bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return exerciseCount(); }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

public slots:
	void newExerciseChosen();
	void newExerciseFromExercisesList();
	void saveExercises(const int exercise_number, const int exercise_idx, const int set_number, const int field);

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
	void startRestTimer(const uint exercise_number, const uint exercise_idx, const uint set_number);
	void stopRestTimer();

private:
	DBMesocyclesModel *m_mesoModel;
	const QString *m_identifierInFile;
	uint m_mesoIdx, m_workingExercise;
	int m_calendarDay;
	bool m_importMode, m_exercisesLoaded;
	QChar m_splitLetter;
	QList<exerciseEntry*> m_exerciseData;
	QHash<int, QByteArray> m_roleNames;

	DBWorkoutsOrSplitsTable *m_db;
	DBModelInterfaceExercises *m_dbModelInterface;

	void commonConstructor(const bool load_from_db);
	TPSetTypes formatSetTypeToImport(const QString &fieldValue) const;
	const QString exportExtraInfo() const;
	inline bool importExtraInfo(const QString &maybe_extra_info);
	void setSetMode(stSet *set, const uint mode);
	void setModeForSet(stSet *set);
	void changeAllSetsMode(const uint exercise_number);
	void _setExerciseName(const uint exercise_number, const uint exercise_idx, QString &&new_name);
	void _setTrackRestTime(const uint exercise_number, const bool track_resttime);
	void _setAutoRestTime(const uint exercise_number, const bool auto_resttime);
	void _setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type);
	void _setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QTime &time);
	void _setSetSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_subsets);
	void _setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_reps);
	void _setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_weight);
	void _setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_notes);
	void _setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed);
	QString increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs);
	QString dropSetReps(const QString &reps, const uint from_subset = 0) const;
	QString clusterReps(const QString &total_reps, const uint from_subset = 0) const;
	QString myorepsReps(const QString &first_set_reps, const uint n_sets, const uint from_set = 0) const;
	QString dropSetWeight(const QString &weight, const uint from_subset = 0) const;
	QString clusterWeight(const QString &constant_weight, const uint from_subset = 0) const;
	QString myorepsWeight(const QString &first_set_weight, const uint n_sets, const uint from_set = 0)const;
};
