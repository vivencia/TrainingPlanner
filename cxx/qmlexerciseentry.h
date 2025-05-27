#pragma once

#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlSetEntry)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(TPTimer)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlExerciseEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint exerciseNumber READ exerciseNumber WRITE setExerciseNumber NOTIFY exerciseNumberChanged FINAL)
Q_PROPERTY(uint newSetType READ newSetType WRITE setNewSetType NOTIFY newSetTypeChanged FINAL)
Q_PROPERTY(QString exerciseNumberLabel READ exerciseNumber NOTIFY exerciseNumberChanged FINAL)
Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName NOTIFY exerciseNameChanged FINAL)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber NOTIFY setsNumberChanged FINAL)
Q_PROPERTY(QString newSetType1 READ newSetReps1 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetType2 READ newSetReps2 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetRestTime1 READ newSetRestTime1 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetRestTime2 READ newSetRestTime2 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetReps1 READ newSetReps1 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetWeight1 READ newSetWeight1 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetReps2 READ newSetReps2 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetWeight2 READ newSetWeight2 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(bool hasSets READ hasSets NOTIFY hasSetsChanged FINAL)
Q_PROPERTY(bool lastExercise READ lastExercise WRITE setLastExercise NOTIFY lastExerciseChanged FINAL)
Q_PROPERTY(bool isEditable READ isEditable WRITE setIsEditable NOTIFY isEditableChanged FINAL)
Q_PROPERTY(bool compositeExercise READ compositeExercise WRITE setCompositeExercise NOTIFY compositeExerciseChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool canEditRestTimeTracking READ canEditRestTimeTracking WRITE setCanEditRestTimeTracking NOTIFY canEditRestTimeTrackingChanged FINAL)
Q_PROPERTY(bool allSetsCompleted READ allSetsCompleted NOTIFY allSetsCompletedChanged FINAL)

public:
	inline explicit QmlExerciseEntry(QObject *parent, QmlWorkoutInterface *workoutPage,
														DBExercisesModel *tDayModel, const uint exercise_number)
		: QObject{parent}, m_workoutPage{workoutPage}, m_workoutModel{tDayModel}, m_exerciseNumber{exercise_number}, m_type{0},
				m_setTimer{nullptr}, m_setComponents{nullptr} {}
	~QmlExerciseEntry();

	inline const QQuickItem *exerciseEntry() const { return m_exerciseEntry; }
	inline QQuickItem *exerciseEntry() { return m_exerciseEntry; }
	void setExerciseEntry(QQuickItem *item);

	inline const uint exerciseNumber() const { return m_exerciseNumber; }
	void setExerciseNumber(const uint new_value);
	inline const QString exerciseNumberLabel() const { return QString::number(m_exerciseNumber + 1); }
	QString setsNumber() const;

	const QString exerciseName1() const;
	const QString exerciseName2() const;
	void setExerciseName1(const QString &new_name);
	void setExerciseName2(const QString &new_name);
	QString newSetType1() const;
	QString newSetType2() const;
	QString newSetRestTime1() const;
	QString newSetRestTime2() const;
	QString newSetReps1() const;
	QString newSetReps2() const;
	QString newSetWeight1() const;
	QString newSetWeight2() const;

	const bool trackRestTime() const;
	void setTrackRestTime(const bool track_resttime);

	const bool autoRestTime() const;
	void setAutoRestTime(const bool auto_resttime);

	const bool hasSets() const;
	inline const bool lastExercise() const { return m_bLast; }
	inline void setLastExercise(const bool new_value) { m_bLast = new_value; emit lastExerciseChanged(); }

	const bool isEditable() const { return m_bEditable; }
	void setIsEditable(const bool editable);

	inline const bool compositeExercise() const { return m_bCompositeExercise; }
	inline void setCompositeExercise(const bool new_value) { m_bCompositeExercise = new_value; emit compositeExerciseChanged(); }

	inline const bool canEditRestTimeTracking() const { return m_bCanEditRestTimeTracking; }
	inline void setCanEditRestTimeTracking(const bool new_value) { m_bCanEditRestTimeTracking = new_value; emit canEditRestTimeTrackingChanged(); }

	const bool allSetsCompleted() const;

	Q_INVOKABLE void removeExercise(const bool bAsk = true);
	Q_INVOKABLE void exerciseCompleted();
	Q_INVOKABLE void moveExerciseUp();
	Q_INVOKABLE void moveExerciseDown();
	Q_INVOKABLE void createAvailableSets();
	Q_INVOKABLE void appendNewSet();
	Q_INVOKABLE void removeSetObject(const uint set_number, const bool bAsk = true);
	Q_INVOKABLE void moveSet(const uint exercise_idx, const uint set_number, const uint new_set_number);
	Q_INVOKABLE void changeSetType(const uint exercise_idx, const uint set_number, const uint new_type);
	Q_INVOKABLE void changeSetMode(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void simpleExercisesList(const bool show, const bool multi_sel, uint comp_exercise = 0);

signals:
	void exerciseNumberChanged();
	void newSetTypeChanged();
	void nSetsChanged();
	void exerciseNameChanged();
	void setsNumberChanged();
	void newSetDataChanged();
	void hasSetsChanged();
	void lastExerciseChanged();
	void isEditableChanged();
	void compositeExerciseChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void canEditRestTimeTrackingChanged();
	void setObjectCreated(const uint set_number);

private:
	QmlWorkoutInterface *m_workoutPage;
	DBExercisesModel *m_workoutModel;
	uint m_exerciseNumber;
	QQuickItem *m_exerciseEntry;
	bool m_bLast, m_bEditable, m_bCompositeExercise, m_bCanEditRestTimeTracking;
	uint m_type;
	TPTimer *m_setTimer;

	QList<QmlSetEntry*> m_setObjects;
	QVariantMap m_setObjectProperties;
	QQmlComponent *m_setComponents[3];
	QQuickItem *m_setsLayout;
	uint m_expectedSetNumber;

	void insertSetEntry(const uint set_number, QmlSetEntry *new_setobject);
	void createSetObject(const uint set_number);
	void createSetObject_part2(const uint set_number, const uint set_type_cpp);
	void setCreated(const uint set_number, const uint nsets, auto conn);
	inline void changeSetCompleteStatus(const uint set_number, const bool bCompleted);
	inline uint findSetMode(const uint set_number) const;
	[[maybe_unused]] inline int findCurrentSet();
	void startRestTimer(const uint set_number, const QString &startTime, const bool bStopWatch);
	void stopRestTimer(const uint set_number);
	inline bool noSetsCompleted() const;
};
