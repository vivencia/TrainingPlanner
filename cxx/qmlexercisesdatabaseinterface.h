#ifndef QMLEXERCISESDATABASEINTERFACE_H
#define QMLEXERCISESDATABASEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QmlWorkoutInterface;

class QQmlComponent;
class QQuickItem;

class QmlExercisesDatabaseInterface : public QObject
{

Q_OBJECT

public:
	inline explicit QmlExercisesDatabaseInterface(QObject *parent) : QObject{parent}, m_exercisesComponent{nullptr} {}

	Q_INVOKABLE void saveExercise();
	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	Q_INVOKABLE void importExercises(const QString &filename = QString{});

	void getExercisesPage(QmlWorkoutInterface *connectPage = nullptr);

private:
	QQmlComponent *m_exercisesComponent;
	QQuickItem *m_exercisesPage;
	QVariantMap m_exercisesProperties;

	void createExercisesPage(QmlWorkoutInterface *connectPage);
	void createExercisesPage_part2(QmlWorkoutInterface *connectPage);

};

#endif // QMLEXERCISESDATABASEINTERFACE_H
