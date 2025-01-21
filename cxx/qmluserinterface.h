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
	explicit inline QmlUserInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow),
			m_settingsComponent(nullptr), m_clientsOrCoachesPage(nullptr), m_userPage(nullptr) {}
	~QmlUserInterface();

	void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void removeUser(const uint user_row, const bool bCoach);
	Q_INVOKABLE void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);

public slots:
	void userModifiedSlot(const uint user_row, const uint field);

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
