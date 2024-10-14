#ifndef QMLEXERCISEINTERFACE_H
#define QMLEXERCISEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QmlExerciseEntry;
class DBTrainingDayModel;
class TPTimer;
class QmlTDayInterface;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;

class QmlExerciseInterface : public QObject
{

Q_OBJECT

public:
	inline explicit QmlExerciseInterface(QObject* parent, QmlTDayInterface* tDayPage, QQmlApplicationEngine* qmlEngine,
										DBTrainingDayModel *tDayModel, QQuickItem* parentLayout)
		: QObject{parent}, m_tDayPage(tDayPage), m_qmlEngine(qmlEngine), m_tDayModel(tDayModel), m_parentLayout(parentLayout), m_exercisesComponent(nullptr) {}
	~QmlExerciseInterface();

	void createExerciseObject();
	void createExercisesObjects();
	void removeExerciseObject(const uint exercise_idx);
	void clearExercises();
	void moveExercise(const uint exercise_idx, const uint new_idx);
	inline uint exercisesCount() const { return m_exercisesList.count(); }

private:
	QmlTDayInterface* m_tDayPage;
	QQmlApplicationEngine* m_qmlEngine;
	DBTrainingDayModel* m_tDayModel;
	QQuickItem* m_parentLayout;
	QVariantMap m_exercisesProperties;
	QQmlComponent* m_exercisesComponent;
	QList<QmlExerciseEntry*> m_exercisesList;

	void createExerciseObject_part2(const uint exercise_idx);
};

#endif // QMLEXERCISEINTERFACE_H
