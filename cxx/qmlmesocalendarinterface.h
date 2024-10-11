#ifndef QMLMESOCALENDARINTERFACE_H
#define QMLMESOCALENDARINTERFACE_H

#include <QObject>
#include <QObject>
#include <QVariantMap>

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlMesoCalendarInterface : public QObject
{

Q_OBJECT

public:
	explicit QmlMesoCalendarInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx);
	~QmlMesoCalendarInterface();

	Q_INVOKABLE void getMesoCalendarPage();

signals:
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_mesoIdx, m_lastUsedCalMesoID;

	void createMesoCalendarPage();
	void createMesoCalendarPage_part2();
};

#endif // QMLMESOCALENDARINTERFACE_H
