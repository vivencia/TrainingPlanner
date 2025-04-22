#ifndef QMLSETENTRY_H
#define QMLSETENTRY_H

#include <QObject>

#define SET_MODE_UNDEFINED 0
#define SET_MODE_START_REST 1
#define SET_MODE_START_EXERCISE 2
#define SET_MODE_SET_COMPLETED 3

class QmlExerciseEntry;
class DBWorkoutModel;

class QQuickItem;

class QmlSetEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(uint number READ number WRITE setNumber NOTIFY numberChanged FINAL)
Q_PROPERTY(uint mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
Q_PROPERTY(uint nSubSets READ nSubSets WRITE setNSubSets NOTIFY nSubSetsChanged FINAL)
Q_PROPERTY(QString exerciseName1 READ exerciseName1 WRITE setExerciseName1 NOTIFY exerciseName1Changed FINAL)
Q_PROPERTY(QString exerciseName2 READ exerciseName2 WRITE setExerciseName2 NOTIFY exerciseName2Changed FINAL)
Q_PROPERTY(QString strNumber READ strNumber NOTIFY strNumberChanged FINAL)
Q_PROPERTY(QString strTotalReps READ strTotalReps NOTIFY strTotalRepsChanged FINAL)
Q_PROPERTY(QString modeLabel READ modeLabel NOTIFY modeLabelChanged FINAL)
Q_PROPERTY(QString restTime READ restTime WRITE setRestTime NOTIFY restTimeChanged FINAL)
Q_PROPERTY(QString reps1 READ reps1 WRITE setReps1 NOTIFY reps1Changed FINAL)
Q_PROPERTY(QString weight1 READ weight1 WRITE setWeight1 NOTIFY weight1Changed FINAL)
Q_PROPERTY(QString reps2 READ reps2 WRITE setReps2 NOTIFY reps2Changed FINAL)
Q_PROPERTY(QString weight2 READ weight2 WRITE setWeight2 NOTIFY weight2Changed FINAL)
Q_PROPERTY(QString reps3 READ reps3 WRITE setReps3 NOTIFY reps3Changed FINAL)
Q_PROPERTY(QString weight3 READ weight3 WRITE setWeight3 NOTIFY weight3Changed FINAL)
Q_PROPERTY(QString reps4 READ reps4 WRITE setReps4 NOTIFY reps4Changed FINAL)
Q_PROPERTY(QString weight4 READ weight4 WRITE setWeight4 NOTIFY weight4Changed FINAL)
Q_PROPERTY(QString subSets READ subSets WRITE setSubSets NOTIFY subSetsChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)
Q_PROPERTY(bool isEditable READ isEditable WRITE setIsEditable NOTIFY isEditableChanged FINAL)
Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged FINAL)
Q_PROPERTY(bool lastSet READ lastSet WRITE setLastSet NOTIFY lastSetChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool current READ current WRITE setCurrent NOTIFY currentChanged FINAL)
Q_PROPERTY(bool hasSubSets READ hasSubSets NOTIFY hasSubSetsChanged FINAL)
Q_PROPERTY(bool isManuallyModified READ isManuallyModified WRITE setIsManuallyModified NOTIFY isManuallyModifiedChanged FINAL)

public:
	inline explicit QmlSetEntry(QObject* parent, QmlExerciseEntry* parentExercise, DBWorkoutModel* tDayModel, const uint exercise_idx)
		: QObject{parent}, m_parentExercise(parentExercise), m_tDayModel(tDayModel), m_exercise_idx(exercise_idx), m_setEntry(nullptr),
		m_type(9999), m_number(9999), m_mode(9999), m_nsubsets(9999),
		m_bEditable(false), m_bCompleted(false), m_bLastSet(false), m_bTrackRestTime(false), m_bAutoRestTime(false),
		m_bCurrent(false), m_bHasSubSets(false), m_bIsManuallyModified(false) {}

	inline const uint exerciseIdx() const { return m_exercise_idx; }
	inline void setExerciseIdx(const uint new_value) { m_exercise_idx = new_value; }

	inline const QQuickItem* setEntry() const { return m_setEntry; }
	inline QQuickItem* setEntry() { return m_setEntry; }
	inline void setSetEntry(QQuickItem* item) { m_setEntry = item; }

	QString exerciseName1() const;
	void setExerciseName1(const QString& new_value);

	QString exerciseName2() const;
	void setExerciseName2(const QString& new_value);

	inline const uint type() const { return m_type; }
	void setType(const uint new_value, const bool bSetIsManuallyModified = true);

	inline const uint number() const { return m_number; }
	void setNumber(const uint new_value);

	inline const uint nSubSets() const { return m_nsubsets; }
	void setNSubSets(const int new_value) { if (new_value >= 0 && new_value < 4) setSubSets(QString::number(new_value)); }

	inline const QString strNumber() const { return QString::number(m_number + 1); }
	inline const QString strTotalReps() const { return tr("Total # of reps: ") + QString::number(m_number + 1); }

	inline QString modeLabel() const
	{
		QString ret;
		switch (mode())
		{
			case SET_MODE_UNDEFINED: ret = std::move(tr("Set completed?")); break;
			case SET_MODE_START_REST: ret = std::move(tr("Start rest")); break;
			case SET_MODE_START_EXERCISE: ret = std::move(tr("Begin exercise")); break;
			case SET_MODE_SET_COMPLETED: break;
		}
		return ret;
	}

	inline QString restTime() const { return m_restTime; }
	void setRestTime(const QString& new_value, const bool bJustUpdateValue = true, const bool bSetIsManuallyModified = true);

	QString reps1() const;
	void setReps1(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString weight1() const;
	void setWeight1(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString reps2() const;
	void setReps2(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString weight2() const;
	void setWeight2(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString reps3() const;
	void setReps3(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString weight3() const;
	void setWeight3(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString reps4() const;
	void setReps4(const QString& new_value, const bool bSetIsManuallyModified = true);

	QString weight4() const;
	void setWeight4(const QString& new_value, const bool bSetIsManuallyModified = true);

	inline QString subSets() const { return m_subsets; }
	void setSubSets(const QString& new_value);

	inline QString notes() const { return m_notes; }
	void setNotes(const QString& new_value);

	inline const uint mode() const { return m_mode; }
	inline void setMode(const uint new_value) { if (m_mode != new_value) { m_mode = new_value; emit modeChanged(); emit modeLabelChanged(); }}

	inline const bool isEditable() const { return m_bEditable; }
	inline void setIsEditable(const bool new_value) { if (m_bEditable != new_value) { m_bEditable = new_value; emit isEditableChanged(); } }

	inline const bool completed() const { return m_bCompleted; }
	inline void setCompleted(const bool new_value) { if (m_bCompleted != new_value)
						{ m_bCompleted = new_value; emit completedChanged(); setIsManuallyModified(false); } }

	inline const bool lastSet() const { return m_bLastSet; }
	inline void setLastSet(const bool new_value) { if (m_bLastSet != new_value) { m_bLastSet = new_value; emit lastSetChanged(); } }

	inline const bool trackRestTime() const { return m_bTrackRestTime; }
	inline void setTrackRestTime(const bool new_value) { m_bTrackRestTime = new_value; emit trackRestTimeChanged(); }

	inline const bool autoRestTime() const { return m_bAutoRestTime; }
	inline void setAutoRestTime(const bool new_value) { m_bAutoRestTime = new_value; emit autoRestTimeChanged(); }

	inline const bool current() const { return m_bCurrent; }
	inline void setCurrent(const bool new_value) { if (m_bCurrent != new_value) { m_bCurrent = new_value; emit currentChanged(); } }

	inline const bool hasSubSets() const { return m_bHasSubSets; }

	inline const bool isManuallyModified() const { return m_bIsManuallyModified; }
	inline void setIsManuallyModified(const bool new_value) { if (m_bIsManuallyModified != new_value)
										{ m_bIsManuallyModified = new_value; emit isManuallyModifiedChanged(); } }

	//Called by QmlExerciseEntry upon new set creation
	inline void _setExerciseName(const QString& new_value) { m_exerciseName = new_value; }
	inline void _setType(const uint new_value) { m_type = new_value; }
	inline void _setNumber(const uint new_value) { m_number = new_value; }
	inline void _setRestTime(const QString& new_value) { m_restTime = new_value; }
	inline void _setReps(const QString& new_value) { m_reps = new_value; }
	inline void _setWeight(const QString& new_value) { m_weight = new_value; }
	inline void _setSubSets(const QString& new_value) { m_subsets = new_value; m_nsubsets = new_value.toUInt(); }
	inline void _setNotes(const QString& new_value) { m_notes = new_value; }
	inline void _setMode(const uint new_value) { m_mode = new_value; }
	inline void _setEditable(const bool new_value) { m_bEditable = new_value; }
	inline void _setCompleted(const bool new_value) { m_bCompleted = new_value; }
	inline void _setLastSet(const bool new_value) { m_bLastSet = new_value; }
	inline void _setTrackRestTime(const bool new_value) { m_bTrackRestTime = new_value; }
	inline void _setAutoRestTime(const bool new_value) { m_bAutoRestTime = new_value; }
	inline void _setCurrent(const bool new_value) { m_bCurrent = new_value; }
	inline void _setHasSubSets(const bool new_value) { m_bHasSubSets = new_value; }

signals:
	void exerciseName1Changed();
	void exerciseName2Changed();
	void typeChanged();
	void numberChanged();
	void modeChanged();
	void nSubSetsChanged();
	void strNumberChanged();
	void strTotalRepsChanged();
	void modeLabelChanged();
	void restTimeChanged();
	void reps1Changed();
	void weight1Changed();
	void reps2Changed();
	void weight2Changed();
	void reps3Changed();
	void weight3Changed();
	void reps4Changed();
	void weight4Changed();
	void subSetsChanged();
	void notesChanged();
	void isEditableChanged();
	void completedChanged();
	void lastSetChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void currentChanged();
	void hasSubSetsChanged();
	void isManuallyModifiedChanged();

private:
	QmlExerciseEntry* m_parentExercise;
	DBWorkoutModel* m_tDayModel;
	uint m_exercise_idx;

	QQuickItem* m_setEntry;
	QString m_exerciseName, m_restTime, m_reps, m_weight, m_subsets, m_notes;
	uint m_type, m_number, m_mode, m_nsubsets;
	bool m_bEditable, m_bCompleted, m_bLastSet, m_bTrackRestTime, m_bAutoRestTime, m_bCurrent, m_bHasSubSets, m_bIsManuallyModified;
};

#endif // QMLSETENTRY_H
