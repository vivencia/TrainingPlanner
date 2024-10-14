#ifndef QMLSETENTRY_H
#define QMLSETENTRY_H

#include <QObject>

#define SET_MODE_UNDEFINED 0
#define SET_MODE_START_REST 1
#define SET_MODE_START_EXERCISE 2
#define SET_MODE_SET_COMPLETED 3

class QQuickItem;
class DBTrainingDayModel;

class QmlSetEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(uint number READ number WRITE setMode NOTIFY numberChanged FINAL)
Q_PROPERTY(uint mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
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
Q_PROPERTY(bool finishButtonEnabled READ finishButtonEnabled WRITE setFinishButtonEnabled NOTIFY finishButtonEnabledChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool current READ current WRITE setCurrent NOTIFY currentChanged FINAL)

public:
	inline explicit QmlSetEntry(QObject* parent, DBTrainingDayModel* tDayModel, const uint exercise_idx)
		: QObject{parent}, m_tDayModel(tDayModel), m_exercise_idx(exercise_idx) {}

	inline const QQuickItem* setEntry() const { return m_setEntry; }
	inline QQuickItem* setEntry() { return m_setEntry; }
	inline void setSetEntry(QQuickItem* item) { m_setEntry = item; }

	inline const uint type() const { return m_type; }
	void setType(const uint new_value);

	inline const uint number() const { return m_number; }
	inline void setNumber(const uint new_value) { m_number = new_value; emit numberChanged(); }

	inline const QString strNumber() const { return QString::number(m_number + 1); }
	inline const QString strTotalReps() const { return QString::number(m_number + 1); }

	inline QString modeLabel() const
	{
		QString ret;
		switch (mode())
		{
			case SET_MODE_UNDEFINED: ret = tr("Set completed?"); break;
			case SET_MODE_START_REST: ret = tr("Start rest"); break;
			case SET_MODE_START_EXERCISE: ret = tr("Begin exercise"); break;
			case SET_MODE_SET_COMPLETED: break;
		}
		return ret;
	}

	inline QString restTime() const { return m_restTime; }
	void setRestTime(const QString& new_value);

	QString reps1() const;
	void setReps1(const QString& new_value);

	QString weight1() const;
	void setWeight1(const QString& new_value);

	QString reps2() const;
	void setReps2(const QString& new_value);

	QString weight2() const;
	void setWeight2(const QString& new_value);

	QString reps3() const;
	void setReps3(const QString& new_value);

	QString weight3() const;
	void setWeight3(const QString& new_value);

	QString reps4() const;
	void setReps4(const QString& new_value);

	QString weight4() const;
	void setWeight4(const QString& new_value);

	inline QString subSets() const { return m_subsets; }
	void setSubSets(const QString& new_value);

	inline QString notes() const { return m_notes; }
	void setNotes(const QString& new_value);

	inline const uint mode() const { return m_mode; }
	inline void setMode(const uint new_value) { m_mode = new_value; emit modeChanged(); emit modeLabelChanged(); }

	inline const bool isEditable() const { return m_bEditable; }
	inline void setIsEditable(const bool new_value) { m_bEditable = new_value; emit isEditableChanged(); }

	inline const bool completed() const { return m_bCompleted; }
	inline void setCompleted(const bool new_value) { m_bCompleted = new_value; emit completedChanged(); }

	inline const bool lastSet() const { return m_bLastSet; }
	inline void setLastSet(const bool new_value) { m_bLastSet = new_value; emit lastSetChanged(); }

	inline const bool finishButtonEnabled () const { return m_bFinishButtonEnabled; }
	inline void setFinishButtonEnabled(const bool new_value) { m_bFinishButtonEnabled = new_value; emit finishButtonEnabledChanged(); }

	inline const bool trackRestTime() const { return m_bTrackRestTime; }
	inline void setTrackRestTime(const bool new_value) { m_bTrackRestTime = new_value; emit trackRestTimeChanged(); }

	inline const bool autoRestTime() const { return m_bAutoRestTime; }
	inline void setAutoRestTime(const bool new_value) { m_bAutoRestTime = new_value; emit autoRestTimeChanged(); }

	inline const bool current() const { return m_bCurrent; }
	inline void setCurrent(const bool new_value) { m_bCurrent = new_value; emit currentChanged(); }

signals:
	void typeChanged();
	void numberChanged();
	void modeChanged();
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
	void finishButtonEnabledChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void currentChanged();

private:
	DBTrainingDayModel* m_tDayModel;
	uint m_exercise_idx;

	QQuickItem* m_setEntry;
	QString m_restTime, m_reps, m_weight, m_subsets, m_notes;
	uint m_type, m_number, m_mode;
	bool m_bEditable, m_bCompleted, m_bLastSet, m_bFinishButtonEnabled, m_bTrackRestTime, m_bAutoRestTime, m_bCurrent;
};

#endif // QMLSETENTRY_H
