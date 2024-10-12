#ifndef QMLEXERCISEINTERFACE_H
#define QMLEXERCISEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QmlExerciseEntry;
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
	inline explicit QmlExerciseInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, DBTrainingDayModel *tDayModel, QQuickItem* parentLayout)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_exercisesComponent(nullptr), m_tDayModel(tDayModel), m_parentLayout(parentLayout) {}
	~QmlExerciseInterface();

	Q_INVOKABLE void createExerciseObject();
	void createExercisesObjects();
	void removeExerciseObject(const uint exercise_idx);
	void clearExercises();
	void moveExercise(const uint exercise_idx, const uint new_idx);
	inline uint exercisesCount() const { return m_exercisesList.count(); }

private:
	QQmlApplicationEngine* m_qmlEngine;
	QVariantMap m_exercisesProperties;
	QQmlComponent* m_exercisesComponent;
	DBTrainingDayModel* m_tDayModel;
	QList<QmlExerciseEntry*> m_exercisesList;
	QQuickItem* m_parentLayout;

	void createExerciseObject_part2(const uint exercise_idx);
};

#endif // QMLEXERCISEINTERFACE_H
