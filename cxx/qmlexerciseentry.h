#pragma once

#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_STRUCT(exerciseIdxEntry)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlSetEntry)
QT_FORWARD_DECLARE_CLASS(TPTimer)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)
Q_DECLARE_OPAQUE_POINTER(QmlSetEntry*)

class QmlExerciseEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(QmlSetEntry* workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged FINAL)
Q_PROPERTY(uint exerciseNumber READ exerciseNumber WRITE setExerciseNumber NOTIFY exerciseNumberChanged FINAL)
Q_PROPERTY(uint subExercisesCount READ subExercisesCount NOTIFY subExercisesCountChanged FINAL)
Q_PROPERTY(QString exerciseNumberLabel READ exerciseNumber NOTIFY exerciseNumberChanged FINAL)
Q_PROPERTY(QString exerciseName1 READ exerciseName1 WRITE setExerciseName1 NOTIFY exerciseName1Changed FINAL)
Q_PROPERTY(QString exerciseName2 READ exerciseName2 WRITE setExerciseName2 NOTIFY exerciseName2Changed FINAL)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber NOTIFY setsNumberChanged FINAL)
Q_PROPERTY(QString newSetType1 READ newSetReps1 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(QString newSetType2 READ newSetReps2 NOTIFY newSetDataChanged FINAL)
Q_PROPERTY(bool hasSets READ hasSets NOTIFY hasSetsChanged FINAL)
Q_PROPERTY(bool lastExercise READ lastExercise WRITE setLastExercise NOTIFY lastExerciseChanged FINAL)
Q_PROPERTY(bool isEditable READ isEditable WRITE setIsEditable NOTIFY isEditableChanged FINAL)
Q_PROPERTY(bool compositeExercise READ compositeExercise WRITE setCompositeExercise NOTIFY compositeExerciseChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool canEditRestTimeTracking READ canEditRestTimeTracking WRITE setCanEditRestTimeTracking NOTIFY canEditRestTimeTrackingChanged FINAL)
Q_PROPERTY(bool allSetsCompleted READ allSetsCompleted NOTIFY allSetsCompletedChanged FINAL)

public:
	explicit QmlExerciseEntry(QObject *parent, QmlWorkoutInterface *workoutPage,
														DBExercisesModel *workoutModel, const uint exercise_number);
	~QmlExerciseEntry();

	inline const QQuickItem *exerciseEntry() const { return m_exerciseEntry; }
	inline QQuickItem *exerciseEntry() { return m_exerciseEntry; }
	void setExerciseEntry(QQuickItem *item);

	inline QmlSetEntry *workingSet() const { return m_workingSet; }
	void setWorkingSet(QmlSetEntry *new_workingset);
	inline const uint exerciseNumber() const { return m_exerciseNumber; }
	inline const uint subExercisesCount() const { return m_exercisesIdxs.count(); }
	void setExerciseNumber(const uint new_value);
	inline const QString exerciseNumberLabel() const { return QString::number(m_exerciseNumber + 1); }
	QString setsNumber() const;

	const QString exerciseName1() const;
	const QString exerciseName2() const;
	void setExerciseName1(const QString &new_name);
	void setExerciseName2(const QString &new_name);
	QString newSetType1() const;
	QString newSetType2() const;

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
	Q_INVOKABLE void removeSetObject(const bool show_delete_dialog = true);
	Q_INVOKABLE void moveSet(const uint set_number, const uint new_set_number);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint new_type);
	Q_INVOKABLE void changeSetMode();
	Q_INVOKABLE void updateSetTypeForNextSets();
	Q_INVOKABLE void updateRestTimeForNextSets();
	Q_INVOKABLE void updateRepsForNextSets(const uint sub_set = 0);
	Q_INVOKABLE void updateWeightForNextSets(const uint sub_set = 0);
	Q_INVOKABLE void simpleExercisesList(const bool show, const bool multi_sel);

signals:
	void workingSetChanged();
	void exerciseNumberChanged();
	void subExercisesCountChanged();
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
	void setObjectCreated(QmlSetEntry *set);

private:
	QmlWorkoutInterface *m_workoutPage;
	DBExercisesModel *m_workoutModel;
	uint m_exerciseNumber;
	QQuickItem *m_exerciseEntry;
	bool m_bLast, m_bEditable, m_bCompositeExercise, m_bCanEditRestTimeTracking;
	TPTimer *m_setTimer;
	QList<exerciseIdxEntry*> m_exercisesIdxs;
	QmlSetEntry *m_workingSet;
	QVariantMap m_setObjectProperties;
	QQmlComponent *m_setComponents[3];
	QQuickItem *m_setsLayout;
	uint m_setsToBeCreated, m_expectedSetNumber;

	void insertSetObject(const uint exercise_idx, const uint set_number, QmlSetEntry *new_setobject);
	void createSetObject(const uint exercise_idx, const uint set_number);
	void createSetObject_part2(const uint exercise_idx, const uint set_number);
	uint findSetMode(const uint exercise_idx, const uint set_number) const;
	void startRestTimer(const bool bStopWatch);
	void stopRestTimer();

private slots:
	void setCreated(QmlSetEntry *set);
};
