#ifndef QMLUSERINTERFACE_H
#define QMLUSERINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlUserInterface : public QObject
{

Q_OBJECT

public:
	explicit inline QmlUserInterface(QObject *parent, QQmlApplicationEngine *qmlEngine, QQuickWindow *mainWindow)
		: QObject{parent}, m_qmlEngine{qmlEngine}, m_mainWindow{mainWindow},
			m_settingsComponent{nullptr}, m_coachesPage{nullptr}, m_clientsPage{nullptr}, m_userPage{nullptr} {}
	~QmlUserInterface();

	void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();

public slots:
	void userModifiedSlot(const uint user_row, const uint field);

private:
	QQmlApplicationEngine *m_qmlEngine;
	QQuickWindow *m_mainWindow;
	QQuickItem *m_settingsPage, *m_coachesPage, *m_clientsPage, *m_userPage;
	QQmlComponent *m_settingsComponent, *m_coachesComponent, *m_clientsComponent;
	QVariantMap m_settingsProperties, m_coachesProperties, m_clientsProperties;

	void createSettingsPage();
	void createCoachesPage();
	void createClientsPage();
};

#endif // QMLUSERINTERFACE_H
