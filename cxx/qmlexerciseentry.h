#ifndef QMLEXERCISEENTRY_H
#define QMLEXERCISEENTRY_H

#include <QObject>

class QQuickItem;
class TPTimer;
class DBTrainingDayModel;

class QmlExerciseEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint exerciseIdx READ exerciseIdx WRITE setExerciseIdx NOTIFY exerciseIdxChanged FINAL)
Q_PROPERTY(uint newSetType READ newSetType WRITE setNewSetType NOTIFY newSetTypeChanged FINAL)
Q_PROPERTY(uint nSets READ nSets WRITE setNSets NOTIFY nSetsChanged FINAL)
Q_PROPERTY(QString exerciseNumber READ exerciseNumber NOTIFY exerciseNumberChanged FINAL)
Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName NOTIFY exerciseNameChanged FINAL)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber NOTIFY setsNumberChanged FINAL)
Q_PROPERTY(QString restTime READ restTime WRITE setRestTime NOTIFY restTimeChanged FINAL)
Q_PROPERTY(QString repsNumber READ repsNumber WRITE setRepsNumber NOTIFY repsNumberChanged FINAL)
Q_PROPERTY(QString weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)
Q_PROPERTY(bool compositeExercise READ compositeExercise WRITE setCompositeExercise NOTIFY compositeExerciseChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool canEditRestTimeTracking READ canEditRestTimeTracking WRITE setCanEditRestTimeTracking NOTIFY canEditRestTimeTrackingChanged FINAL)

public:
	inline explicit QmlExerciseEntry(QObject* parent, const uint exercise_idx, DBTrainingDayModel* tDayModel)
		: QObject{parent}, m_exercise_idx(exercise_idx), m_tDayModel(tDayModel), m_type(0), m_setTimer(nullptr) {}

	inline const QQuickItem* exerciseEntry() const { return m_exerciseEntry; }
	inline QQuickItem* exerciseEntry() { return m_exerciseEntry; }
	inline void setExerciseEntry(QQuickItem* item) { m_exerciseEntry = item; }

	inline const uint exerciseIdx() const { return m_exercise_idx; }
	inline void setExerciseIdx(const uint new_value) { m_exercise_idx = new_value; emit exerciseIdxChanged(); emit exerciseNumberChanged(); }
	inline const QString exerciseNumber() const { return QString::number(m_exercise_idx + 1); }
	inline const uint newSetType() const { return m_type; }
	const QString exerciseName() const;
	void setExerciseName(const QString& new_value);
	void setNewSetType(const uint new_value);
	inline const uint nSets() const { return m_sets.toUInt(); }
	inline void setNSets(const uint new_value) { m_sets = QString::number(new_value); emit setsNumberChanged(); emit nSetsChanged(); }
	inline const QString& setsNumber() const { return m_sets; }
	inline void setSetsNumber(const QString& new_value) { m_sets = new_value; emit setsNumberChanged(); emit nSetsChanged(); }
	inline const QString& restTime() const { return m_restTime; }
	inline void setRestTime(const QString& new_value) { m_restTime = new_value; emit restTimeChanged(); }
	inline const QString& repsNumber() const { return m_reps; }
	inline QString& repsNumber() { return m_reps; }
	inline void setRepsNumber(const QString& new_value) { m_reps = new_value; emit repsNumberChanged(); }
	inline const QString& weight() const { return m_weight; }
	inline QString& weight() { return m_weight; }
	inline void setWeight(const QString& new_value) { m_weight = new_value; emit weightChanged(); }
	inline const bool compositeExercise() const { return bCompositeExercise; }
	inline void setCompositeExercise(const bool new_value) { bCompositeExercise = new_value; emit compositeExerciseChanged(); }
	inline const bool trackRestTime() const { return bTrackRestTime; }
	void setTrackRestTime(const bool new_value);
	inline const bool autoRestTime() const { return bAutoRestTime; }
	void setAutoRestTime(const bool new_value);
	inline const bool canEditRestTimeTracking() const { return bCanEditRestTimeTracking; }
	inline void setCanEditRestTimeTracking(const bool new_value) { bCanEditRestTimeTracking = new_value; emit canEditRestTimeTrackingChanged(); }

	Q_INVOKABLE void manageRestTime(const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);

signals:
	void exerciseIdxChanged();
	void exerciseNumberChanged();
	void newSetTypeChanged();
	void nSetsChanged();
	void exerciseNameChanged();
	void setsNumberChanged();
	void restTimeChanged();
	void repsNumberChanged();
	void weightChanged();
	void compositeExerciseChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void canEditRestTimeTrackingChanged();

private:
	uint m_exercise_idx;
	DBTrainingDayModel* m_tDayModel;
	QQuickItem* m_exerciseEntry;
	QList<QQuickItem*> m_setObjects;
	QString m_sets, m_reps, m_weight, m_restTime;
	bool bCompositeExercise, bTrackRestTime, bAutoRestTime, bCanEditRestTimeTracking;
	uint m_type;
	TPTimer* m_setTimer;
};

#endif // QMLEXERCISEENTRY_H
