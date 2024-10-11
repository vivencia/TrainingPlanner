#ifndef QMLSETSINTERFACE_H
#define QMLSETSINTERFACE_H

#include <QObject>
#include <QVariantMap>

class DBTrainingDayModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlSetsInterface : public QObject
{

Q_OBJECT

public:
	explicit QmlSetsInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx);

	Q_INVOKABLE void getSetObjects(const uint exercise_idx);
	Q_INVOKABLE void addNewSet(const uint exercise_idx);
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text, const bool bChangeModel = true);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	Q_INVOKABLE void changeSetMode(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE QQuickItem* nextSetObject(const uint exercise_idx, const uint set_number) const;

signals:

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	uint m_mesoIdx;
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	uint m_expectedSetNumber;

	void createSetObject(const uint exercise_idx, const uint set_number, const bool bNewSet, const uint set_type,
									 const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0, const bool bNewSet = false);
	void enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed);
	void enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime, const uint bAutoRestTime, const uint except_set_number = 0);
	void findSetMode(const uint exercise_idx, const uint set_number);
	void findCurrentSet(const uint exercise_idx, const uint set_number);

	void startRestTimer(const uint exercise_idx, const uint set_number);
	void stopRestTimer(const uint exercise_idx, const uint set_number);
};

#endif // QMLSETSINTERFACE_H
