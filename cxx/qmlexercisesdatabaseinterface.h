#ifndef QMLEXERCISESDATABASEINTERFACE_H
#define QMLEXERCISESDATABASEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QmlTDayInterface;

class QQmlComponent;
class QQuickItem;

class QmlExercisesDatabaseInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString exerciseSubNameLabel READ exerciseSubNameLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString muscularGroupLabel READ muscularGroupLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString setsNumberLabel READ setsNumberLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString repsNumberLabel READ repsNumberLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString weightLabel READ weightLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString mediaLabel READ mediaLabel NOTIFY labelsChanged FINAL)

public:
	inline explicit QmlExercisesDatabaseInterface(QObject* parent) : QObject{parent}, m_exercisesComponent(nullptr) {}
	~QmlExercisesDatabaseInterface();

	QString exerciseNameLabel() const;
	QString exerciseSubNameLabel() const;
	QString muscularGroupLabel() const;
	QString setsNumberLabel() const;
	QString repsNumberLabel() const;
	QString weightLabel() const;
	QString mediaLabel() const;

	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	Q_INVOKABLE void importExercises(const QString& filename = QString());

	void getExercisesPage(QmlTDayInterface *connectPage = nullptr);

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void labelsChanged();

private:
	QQmlComponent* m_exercisesComponent;
	QQuickItem* m_exercisesPage;
	QVariantMap m_exercisesProperties;

	void createExercisesPage(QmlTDayInterface* connectPage);
	void createExercisesPage_part2(QmlTDayInterface* connectPage);

};

#endif // QMLEXERCISESDATABASEINTERFACE_H
