#ifndef QMLEXERCISEENTRY_H
#define QMLEXERCISEENTRY_H

#include <QObject>

#include "qmlexerciseinterface.h"

class QQuickItem;
class DBTrainingDayModel;
class QmlTDayInterface;
class QmlSetEntry;

class QQmlApplicationEngine;

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
Q_PROPERTY(QString repsForExercise1 READ repsForExercise1 WRITE setRepsForExercise1 NOTIFY repsForExercise1Changed FINAL)
Q_PROPERTY(QString weightForExercise1 READ weightForExercise1 WRITE setWeightForExercise1 NOTIFY weightForExercise1Changed FINAL)
Q_PROPERTY(QString repsForExercise2 READ repsForExercise2 WRITE setRepsForExercise2 NOTIFY repsForExercise2Changed FINAL)
Q_PROPERTY(QString weightForExercise2 READ weightForExercise2 WRITE setWeightForExercise2 NOTIFY weightForExercise2Changed FINAL)
Q_PROPERTY(bool compositeExercise READ compositeExercise WRITE setCompositeExercise NOTIFY compositeExerciseChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool canEditRestTimeTracking READ canEditRestTimeTracking WRITE setCanEditRestTimeTracking NOTIFY canEditRestTimeTrackingChanged FINAL)

public:
	inline explicit QmlExerciseEntry(QObject* parent, QmlTDayInterface* tDayPage, QQmlApplicationEngine* qmlEngine,
										DBTrainingDayModel* tDayModel, const uint exercise_idx)
		: QObject{parent}, m_tDayPage(tDayPage), m_qmlEngine(qmlEngine), m_tDayModel(tDayModel),
			m_exercise_idx(exercise_idx), m_type(0), m_setTimer(nullptr), m_setComponents{nullptr} {}
	~QmlExerciseEntry();

	inline const QQuickItem* exerciseEntry() const { return m_exerciseEntry; }
	inline QQuickItem* exerciseEntry() { return m_exerciseEntry; }
	void setExerciseEntry(QQuickItem* item);

	inline const uint exerciseIdx() const { return m_exercise_idx; }
	void setExerciseIdx(const uint new_value);

	inline const QString exerciseNumber() const { return QString::number(m_exercise_idx + 1); }
	inline const uint newSetType() const { return m_type; }
	void setNewSetType(const uint new_value);

	const QString exerciseName() const;
	void setExerciseName(const QString& new_value);

	inline const uint nSets() const { return m_sets.toUInt(); }
	inline void setNSets(const uint new_value) { m_sets = QString::number(new_value); emit setsNumberChanged(); emit nSetsChanged(); }

	inline QString setsNumber() const { return m_sets; }
	inline void setSetsNumber(const QString& new_value) { m_sets = new_value; emit setsNumberChanged(); emit nSetsChanged(); }

	inline QString restTime() const { return m_restTime; }
	inline void setRestTime(const QString& new_value) { m_restTime = new_value; emit restTimeChanged(); }

	QString repsForExercise1();
	void setRepsForExercise1(const QString& new_value);

	QString repsForExercise2();
	void setRepsForExercise2(const QString& new_value);

	inline const QString& reps() const { return m_reps; }
	inline QString& reps() { return m_reps; }
	inline void setReps(const QString& new_value) { m_reps = new_value; emit repsForExercise1Changed(); if (m_bCompositeExercise) emit repsForExercise2Changed(); }

	QString weightForExercise1();
	void setWeightForExercise1(const QString& new_value);

	QString weightForExercise2();
	void setWeightForExercise2(const QString& new_value);

	inline const QString& weight() const { return m_weight; }
	inline QString& weight() { return m_weight; }
	inline void setWeight(const QString& new_value) { m_weight = new_value; emit weightForExercise1Changed(); if (m_bCompositeExercise) emit weightForExercise2Changed(); }

	inline const bool compositeExercise() const { return m_bCompositeExercise; }
	inline void setCompositeExercise(const bool new_value) { m_bCompositeExercise = new_value; emit compositeExerciseChanged(); }

	inline const bool trackRestTime() const { return m_bTrackRestTime; }
	void setTrackRestTime(const bool new_value);

	inline const bool autoRestTime() const { return m_bAutoRestTime; }
	void setAutoRestTime(const bool new_value);

	inline const bool canEditRestTimeTracking() const { return m_bCanEditRestTimeTracking; }
	inline void setCanEditRestTimeTracking(const bool new_value) { m_bCanEditRestTimeTracking = new_value; emit canEditRestTimeTrackingChanged(); }

	void createAvailableSets();
	Q_INVOKABLE void appendNewSet();
	Q_INVOKABLE void removeSetObject(const uint set_number);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint new_type);
	Q_INVOKABLE void changeSetMode(const uint set_number);
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE QQuickItem* nextSetObject(const uint exercise_idx, const uint set_number) const;

signals:
	void exerciseIdxChanged();
	void exerciseNumberChanged();
	void newSetTypeChanged();
	void nSetsChanged();
	void exerciseNameChanged();
	void setsNumberChanged();
	void restTimeChanged();
	void repsForExercise1Changed();
	void weightForExercise1Changed();
	void repsForExercise2Changed();
	void weightForExercise2Changed();
	void compositeExerciseChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void canEditRestTimeTrackingChanged();
	void setObjectCreated(const uint set_number);

private:
	QmlTDayInterface* m_tDayPage;
	QQmlApplicationEngine* m_qmlEngine;
	DBTrainingDayModel* m_tDayModel;
	uint m_exercise_idx;
	QQuickItem* m_exerciseEntry;
	QString m_sets, m_reps, m_weight, m_restTime;
	bool m_bCompositeExercise, m_bTrackRestTime, m_bAutoRestTime, m_bCanEditRestTimeTracking;
	uint m_type;
	TPTimer* m_setTimer;

	QList<QmlSetEntry*> m_setObjects;
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	QQuickItem* m_setsLayout;
	uint m_expectedSetNumber;

	inline uint findSetMode(const uint set_number) const;
	inline void findCurrentSet();
	void insertSetEntry(const uint set_number, QmlSetEntry* new_setobject);
	void createSetObject(const uint set_number, const uint type, const QString& resttime, const QString& nreps, const QString& weight);
	void createSetObject_part2(const uint set_number, const uint set_type_cpp);
	void enableDisableExerciseCompletedButton();
	void startRestTimer(const uint set_number);
	void stopRestTimer(const uint set_number);
};

#endif // QMLEXERCISEENTRY_H
