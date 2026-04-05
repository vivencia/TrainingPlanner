#pragma once

#include <QObject>
#include <qqml.h>
#include <QVariantMap>

class QmlWorkoutInterface;

class QQmlComponent;
class QQuickItem;

class QmlExercisesDatabaseInterface : public QObject
{

Q_OBJECT
QML_VALUE_TYPE(ExercisesListManager)
QML_UNCREATABLE("")

public:
	inline explicit QmlExercisesDatabaseInterface(QObject *parent) : QObject{parent} {}

	Q_INVOKABLE void saveExercise();
	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	Q_INVOKABLE void importExercises(const QString &filename = QString{});

	void getExercisesPage(QmlWorkoutInterface *connect_page = nullptr);

private:
	QQmlComponent *m_exercisesComponent{nullptr};
	QQuickItem *m_exercisesPage{nullptr};
	QVariantMap m_exercisesProperties;

	void createExercisesPage(QmlWorkoutInterface *connect_page);
};
