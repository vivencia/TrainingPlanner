#ifndef QMLEXERCISESDATABASEINTERFACE_H
#define QMLEXERCISESDATABASEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlExercisesDatabaseInterface : public QObject
{

Q_OBJECT

public:
	inline explicit QmlExercisesDatabaseInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_exercisesComponent(nullptr) {}

	~QmlExercisesDatabaseInterface();

	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	Q_INVOKABLE void importExercises(const QString& filename = QString());

	Q_INVOKABLE void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage = nullptr);

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_exercisesComponent;
	QQuickItem* m_exercisesPage;
	QVariantMap m_exercisesProperties;

	void createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	void createExercisesPage_part2(QQuickItem* connectPage);

};

#endif // QMLEXERCISESDATABASEINTERFACE_H
