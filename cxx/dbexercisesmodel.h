#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#define EXERCISES_COL_ID 0
#define EXERCISES_COL_MESOID 1
#define EXERCISES_COL_CALENDARDAY 2
#define EXERCISES_COL_SPLITLETTER 3
#define EXERCISES_COL_EXERCISES 4
#define EXERCISES_COL_SETTYPES 5
#define EXERCISES_COL_RESTTIMES 6
#define EXERCISES_COL_SUBSETS 7
#define EXERCISES_COL_REPS 8
#define EXERCISES_COL_WEIGHTS 9
#define EXERCISES_COL_NOTES 10
#define EXERCISES_COL_COMPLETED 11
#define WORKOUT_TOTALCOLS EXERCISES_COL_COMPLETED+1

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_STRUCT(exerciseEntry)
QT_FORWARD_DECLARE_STRUCT(stSet)

QT_FORWARD_DECLARE_CLASS(QFile)

enum {
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
Q_PROPERTY(int workingExercise READ workingExercise WRITE setWorkingSExercise NOTIFY workingExerciseChanged)

Q_PROPERTY(QString totalSetsLabel READ totalSetsLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString setNumberLabel READ setNumberLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelChanged FINAL)
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
			m_calendarManager{parent}, m_mesoIdx{meso_idx}, m_calendarDay{calendar_day}, m_splitLetter{'N'}
	{
		commonConstructor();
	}
	inline explicit DBExercisesModel(DBMesoCalendarManager *parent, const uint meso_idx, const QChar &splitletter)
		: QAbstractListModel{reinterpret_cast<QObject*>(parent)},
			m_calendarManager{parent}, m_mesoIdx{meso_idx}, m_calendarDay{-1}, m_splitLetter{splitletter}
	{
		commonConstructor();
	}
	~DBExercisesModel() { clearExercises(); }

	void operator=(DBExercisesModel *other_model);

	inline DBMesoCalendarManager *calendarManager() const { return m_calendarManager; }
	inline const QString &id() const {return m_id; }
	inline void setId(const QString &new_id) { m_id = new_id; }
	inline const QString &mesoId() const {return m_mesoId; }
	inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	inline int calendarDay() const { return m_calendarDay; }
	inline const QChar &splitLetter() const { return m_splitLetter; }
	inline bool importModel() const { return m_importMode; }
	inline void setImportMode(const bool import_mode) { m_importMode = import_mode; }

	bool fromDataBase(const QStringList &data, const bool bClearSomeFieldsForReUse = false);
	const QStringList toDatabase(const bool to_export_file = false) const;
	void clearExercises();

	int exportToFile(const QString &filename, QFile *out_file = nullptr) const;
	int exportToFormattedFile(const QString &filename, QFile *out_file = nullptr) const;
	int importFromFile(const QString &filename, QFile *in_file = nullptr);
	int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	inline const QString &identifierInFile() const { return *m_identifierInFile; }

	const uint inline exerciseCount() const { return m_exerciseData.count(); }
	const uint setsNumber(const uint exercise_number) const;

	Q_INVOKABLE uint addExercise(const bool emit_signal = true);
	Q_INVOKABLE void delExercise(const uint exercise_number, const bool emit_signal = true);
	void moveExercise(const uint from, const uint to);
	Q_INVOKABLE uint addSubExercise(const uint exercise_number, const bool emit_signal = true);
	Q_INVOKABLE void delSubExercise(const uint exercise_number, const uint exercise_idx, const bool emit_signal = true);
	Q_INVOKABLE uint addSet(const uint exercise_number, const uint exercise_idx, const bool emit_signal = true);
	Q_INVOKABLE void delSet(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool emit_signal = true);
	void moveSet(const uint exercise_number, const uint exercise_idx, const uint from_set, const uint to_set);
	Q_INVOKABLE bool exerciseIsComposite(const uint exercise_number) const;

	inline uint workingExercise() const { return m_workingExercise; }
	inline void setWorkingExercise(const uint new_workingexercise)
	{
		if (new_workingexercise != m_workingExercise)
		{
			m_workingExercise = new_workingexercise;
			emit workingExerciseChanged();
		}
	}
	uint workingSet(const uint exercise_number) const;
	void setWorkingSet(const uint exercise_number, const uint new_workingset);

	bool trackRestTime(const uint exercise_number, const uint exercise_idx = 0) const;
	void setTrackRestTime(const uint exercise_number, const uint exercise_idx, const bool track_resttime);
	bool autoRestTime(const uint exercise_number, const uint exercise_idx = 0) const;
	void setAutoRestTime(const uint exercise_number, const uint exercise_idx, const bool auto_resttime);

	Q_INVOKABLE QString exerciseName(const uint exercise_number, const uint exercise_idx = 0) const;
	Q_INVOKABLE void setExerciseName(const uint exercise_number, const uint exercise_idx, const QString &new_name);
	void setExerciseName(const uint exercise_number, const uint exercise_idx, QString &&new_name);

	Q_INVOKABLE uint setType(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type);
	void changeSetType(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint new_type);

	Q_INVOKABLE QString setRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetRestTime(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_time);

	Q_INVOKABLE QString setSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetSubSets(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_subsets);
	void addSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number);
	void delSetSubSet(const uint exercise_number, const uint exercise_idx, const uint set_number);

	Q_INVOKABLE QString setReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset = 0) const;
	Q_INVOKABLE void setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_reps, const uint subset = 0);
	void setSetReps(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_reps);

	Q_INVOKABLE QString setWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint subset = 0) const;
	Q_INVOKABLE void setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_weight, const uint subset = 0);
	void setSetWeight(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_weight);

	Q_INVOKABLE QString setNotes(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, const QString &new_notes);
	void setSetNotes(const uint exercise_number, const uint exercise_idx, const uint set_number, QString &&new_notes);

	Q_INVOKABLE bool setCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void setSetCompleted(const uint exercise_number, const uint exercise_idx, const uint set_number, const bool completed);
	bool allSetsCompleted(const uint exercise_number) const;
	bool anySetCompleted(const uint exercise_number) const;

	inline QString totalSetsLabel() const { return tr("Number of sets: "); }
	inline QString setNumberLabel() const { return tr("Set #: "); }
	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString setTypeLabel() const { return tr("Type: "); }
	inline QString setRestTimeLabel() const { return tr("Rest time: "); }
	inline QString setTotalSubsets() const { return tr("Number of subsets: "); }
	inline QString setRepsLabel() const { return tr("Repetitions: "); }
	inline QString setWeightLabel() const { return tr("Weight: "); }
	inline QString setNotesLabel() const { return tr("Notes/Instructions for the set: "); }
	inline QString setCompletedLabel() const { return tr("Completed?"); }
	inline QString restTimeUntrackedLabel() const { return tr("As needed"); }
	inline QString splitLabel() const { return tr("Split: "); }

	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return exerciseCount(); }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void exerciseNameChanged(const uint exercise_number, const uint exercise_idx);
	void setsNumberChanged(const int exercise_number, const uint exercise_idx);
	void setTypeChanged(const int exercise_number, const uint exercise_idx, const uint set_number);
	void workingExerciseChanged();
	void workingSetChanged(const int exercise_number);
	void exerciseCountChanged();
	void exerciseCompleted(const uint exercise_number, const bool completed);
	void labelChanged();

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

	void commonConstructor();
	const QString formatSetTypeToExport(stSet *set) const;
	TPSetTypes formatSetTypeToImport(const QString &fieldValue) const;
	const QString exportExtraInfo() const;
	bool importExtraInfo(const QString &maybe_extra_info);
	QString increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs);
	void setSuggestedTime(const uint set_number, const QList<stSet*> &sets);
	void setSuggestedSubSets(const uint set_number, const QList<stSet*> &sets);
	void setSuggestedReps(const uint set_number, const QList<stSet*> &sets, const uint from_subset = 0);
	void setSuggestedWeight(const uint set_number, const QList<stSet *> &sets, const uint from_subset = 0);
	QString dropSetReps(const QString &reps, const uint n_subsets, const uint from_subset = 0);
	QString clusterReps(const QString &total_reps, const uint n_subsets, const uint from_subset = 0);
	QString myorepsReps(const QString &first_set_reps, const uint n_sets, const uint from_subset = 0);
	QString dropSetWeight(const QString &weight, const uint n_subsets, const uint from_subset = 0);
	QString clusterWeight(const QString &constant_weight, const uint n_subsets, const uint from_subset = 0);
	QString myorepsWeight(const QString &first_set_weight, const uint n_sets, const uint from_subset = 0);
};
