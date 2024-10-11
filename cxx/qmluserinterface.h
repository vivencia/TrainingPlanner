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
	explicit QmlUserInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow);
	~QmlUserInterface();

	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	Q_INVOKABLE void removeUser(const uint user_row, const bool bCoach);

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQuickItem* m_settingsPage, *m_clientsOrCoachesPage, *m_userPage;
	QQmlComponent* m_settingsComponent, *m_clientsOrCoachesComponent;
	QVariantMap m_settingsProperties, m_clientsOrCoachesProperties;

	void createSettingsPage();
	void createClientsOrCoachesPage();
	void setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches);
};

#endif // QMLUSERINTERFACE_H
