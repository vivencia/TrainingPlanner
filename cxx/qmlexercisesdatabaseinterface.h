#pragma once

#include <QObject>
#include <qqml.h>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlExercisesDatabaseInterface : public QObject
{

Q_OBJECT
QML_VALUE_TYPE(ExercisesListManager)
QML_UNCREATABLE("")

public:
	inline explicit QmlExercisesDatabaseInterface(QObject *parent) : QObject{parent} {}

	void getExercisesPage(QmlWorkoutInterface *connect_page = nullptr);

private:
	QQmlComponent *m_exercisesComponent{nullptr};
	QQuickItem *m_exercisesPage{nullptr};

	void createExercisesPage(QmlWorkoutInterface *connect_page);
};
