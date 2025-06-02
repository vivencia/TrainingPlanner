#ifndef QMLSETENTRY_H
#define QMLSETENTRY_H

#include <QObject>

#define SET_MODE_UNDEFINED 0
#define SET_MODE_START_REST 1
#define SET_MODE_START_EXERCISE 2
#define SET_MODE_SET_COMPLETED 3

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlExerciseEntry)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlSetEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(uint setNumber READ setNumber WRITE setSetNumber NOTIFY setNumberChanged FINAL)
Q_PROPERTY(uint mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
Q_PROPERTY(uint nSubSets READ nSubSets NOTIFY subSetsChanged FINAL)

Q_PROPERTY(QString modeLabel READ modeLabel NOTIFY modeChanged FINAL)
Q_PROPERTY(QString setNumberLabel READ setNumberLabel NOTIFY setNumberChanged FINAL)
Q_PROPERTY(QString totalRepsLabel READ totalRepsLabel NOTIFY totalRepsChanged FINAL)
Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName NOTIFY exerciseNameChanged FINAL)
Q_PROPERTY(QString restTime READ restTime WRITE setRestTime NOTIFY restTimeChanged FINAL)
Q_PROPERTY(QString reps READ reps WRITE setReps NOTIFY repsChanged FINAL)
Q_PROPERTY(QString weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)
Q_PROPERTY(QString subSets READ subSets WRITE setSubSets NOTIFY subSetsChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)
Q_PROPERTY(bool workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged FINAL)
Q_PROPERTY(bool isEditable READ isEditable WRITE setIsEditable NOTIFY isEditableChanged FINAL)
Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged FINAL)
Q_PROPERTY(bool lastSet READ lastSet WRITE setLastSet NOTIFY lastSetChanged FINAL)
Q_PROPERTY(bool hasSubSets READ hasSubSets NOTIFY hasSubSetsChanged FINAL)

public:
	explicit QmlSetEntry(QObject *parent, QmlExerciseEntry *parentExercise, DBExercisesModel *workoutModel,
								const uint exercise_number, const uint exercise_idx, const uint set_number);

	inline uint exerciseIdx() const { return m_exerciseIdx; }
	void setExerciseIdx(const uint new_exerciseidx) { m_exerciseIdx = new_exerciseidx; }
	inline const uint exerciseNumber() const { return m_exerciseIdx; }
	inline void setExerciseNumber(const uint new_value) { m_exerciseIdx = new_value; }

	inline const QQuickItem *setEntry() const { return m_setEntry; }
	inline QQuickItem *setEntry() { return m_setEntry; }
	inline void setSetEntry(QQuickItem *item) { m_setEntry = item; }

	QString setNumberLabel() const;
	QString totalRepsLabel() const;
	QString modeLabel() const;

	const uint setNumber() const { return m_setNumber; }
	void setSetNumber(const uint new_number);

	QString exerciseName() const;
	void setExerciseName(const QString &new_value);

	const uint type() const;
	void setType(const uint new_type);

	QString restTime() const;
	void setRestTime(const QString &new_value, const bool update_model = true);

	QString reps() const;
	void setReps(const QString &new_reps, const bool update_model = true);

	QString weight() const;
	void setWeight(const QString &new_weight, const bool update_model = true);

	QString subSets() const;
	void setSubSets(const QString &new_subsets, const bool update_model = true);
	inline const uint nSubSets() const { return subSets().toUInt(); }
	Q_INVOKABLE void addSubSet();
	Q_INVOKABLE void removeSubSet();

	QString notes() const;
	void setNotes(const QString &new_notes);

	const bool completed() const;
	void setCompleted(const bool completed);

	const bool workingSet() const;
	void setWorkingSet(const bool working);

	const bool hasSubSets() const;

	inline const uint mode() const { return m_setMode; }
	inline void setMode(const uint new_setmode) { if (m_setMode != new_setmode) { m_setMode = new_setmode; emit modeChanged(); }}

	inline const bool isEditable() const { return m_bEditable; }
	inline void setIsEditable(const bool new_value) { if (m_bEditable != new_value) { m_bEditable = new_value; emit isEditableChanged(); } }

	inline const bool lastSet() const { return m_bLastSet; }
	inline void setLastSet(const bool new_value) { if (m_bLastSet != new_value) { m_bLastSet = new_value; emit lastSetChanged(); } }

signals:
	void setNumberChanged();
	void exerciseNameChanged();
	void typeChanged();
	void modeChanged();
	void totalRepsChanged();
	void labelChanged();
	void restTimeChanged();
	void repsChanged();
	void weightChanged();
	void subSetsChanged();
	void notesChanged();
	void isEditableChanged();
	void completedChanged();
	void lastSetChanged();
	void workingSetChanged();
	void hasSubSetsChanged();

private:
	QmlExerciseEntry *m_parentExercise;
	DBExercisesModel *m_workoutModel;
	uint m_exerciseNumber, m_exerciseIdx, m_setNumber, m_setMode;

	QQuickItem *m_setEntry;
	bool m_bEditable, m_bLastSet;
};

#endif // QMLSETENTRY_H
