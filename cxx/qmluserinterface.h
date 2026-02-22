#pragma once

#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlUserInterface : public QObject
{

Q_OBJECT

public:
	explicit inline QmlUserInterface(QObject *parent)
		: QObject{parent}, m_userPage{nullptr}, m_settingsPage{nullptr}, m_coachesPage{nullptr}, m_clientsPage{nullptr} {}

	void getUserPage();
	void getSettingsPage();
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();

private:
	QQuickItem *m_userPage, *m_settingsPage, *m_coachesPage, *m_clientsPage;
	QQmlComponent *m_userComponent, *m_settingsComponent, *m_coachesComponent, *m_clientsComponent;
	QVariantMap m_userProperties, m_coachesProperties, m_clientsProperties;

	void createUserPage();
	void createSettingsPage();
	void createCoachesPage();
	void createClientsPage();
};
