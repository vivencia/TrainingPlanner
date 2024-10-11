#ifndef QMLEXERCISEINTERFACE_H
#define QMLEXERCISEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class DBTrainingDayModel;
class TPTimer;
class QmlSetsInterface;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;

class QmlExerciseInterface : public QObject
{

Q_OBJECT

public:
	explicit QmlExerciseInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, const uint exercise_idx);
	~QmlExerciseInterface();

	Q_INVOKABLE void createExerciseObject();
	Q_INVOKABLE void manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);
	Q_INVOKABLE uint exerciseSetsCount(const uint exercise_idx) const;
	Q_INVOKABLE uint exerciseDefaultSetType(const uint exercise_idx);
	Q_INVOKABLE void setExerciseDefaultSetType(const uint exercise_idx, const uint set_type);
	Q_INVOKABLE const QString exerciseSets(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseSets(const uint exercise_idx, const QString& new_nsets);
	Q_INVOKABLE const QString exerciseRestTime(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseRestTime(const uint exercise_idx, const QString& new_resttime);
	Q_INVOKABLE const QString exerciseReps(const uint exercise_idx, const uint composite_idx) const;
	Q_INVOKABLE void setExerciseReps(const uint exercise_idx, const uint composite_idx, const QString& new_nreps);
	Q_INVOKABLE const QString exerciseWeight(const uint exercise_idx, const uint composite_idx) const;
	Q_INVOKABLE void setExerciseWeight(const uint exercise_idx, const uint composite_idx, const QString& new_nweight);

signals:

private:
	QQmlApplicationEngine* m_qmlEngine;
	uint m_exerciseIdx;
	QVariantMap m_tDayExerciseEntryProperties;
	QQmlComponent* m_tDayExercisesComponent;
	QQuickItem* m_exerciseEntry;
	QString nSets, nReps, nWeight, restTime;
	uint newSetType;
	TPTimer* m_setTimer;
	QList<QmlSetsInterface*> m_setObjects;

	void createExercisesObjects();
	void createExerciseObject_part2(const int object_idx = -1);
	inline uint exercisesCount() const;
	inline QQuickItem* exerciseEntryItem(const uint exercise_idx);
	inline QQuickItem* exerciseEntryItem(const uint exercise_idx) const;
	inline QQuickItem* exerciseSetItem(const uint exercise_idx, const uint set_number);
	inline QQuickItem* exerciseSetItem(const uint exercise_idx, const uint set_number) const;
	inline void appendExerciseEntry();
	void removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow = false);
	inline void setExerciseItem(const uint exercise_idx, QQuickItem* new_exerciseItem);
	inline const QString& exerciseReps(const uint exercise_idx) const;
	inline void setExerciseReps(const uint exercise_idx, const QString& nreps);
	inline const QString& exerciseWeights(const uint exercise_idx) const;
	inline void setExerciseWeight(const uint exercise_idx, const QString& nweight);
	inline void insertExerciseSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject);
	inline void appendExerciseSet(const uint exercise_idx, QQuickItem* new_setObject);
	inline void removeExerciseSet(const uint exercise_idx, const uint set_number);
	inline void clearExerciseEntries(const bool bDeleteNow = false);
};

#endif // QMLEXERCISEINTERFACE_H
