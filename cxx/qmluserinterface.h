#pragma once

#include <QObject>
#include <qqml.h>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlUserInterface : public QObject
{

Q_OBJECT
QML_VALUE_TYPE(UserManager)
QML_UNCREATABLE("")

public:
	explicit inline QmlUserInterface(QObject *parent) : QObject{parent} {}

	void getUserPage();
	void getSettingsPage();
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();

private:
	QQuickItem *m_userPage{nullptr}, *m_settingsPage{nullptr}, *m_coachesPage{nullptr}, *m_clientsPage{nullptr};
	QQmlComponent *m_userComponent{nullptr}, *m_settingsComponent{nullptr}, *m_coachesComponent{nullptr}, *m_clientsComponent{nullptr};
	QVariantMap m_userProperties, m_coachesProperties, m_clientsProperties;

	void createUserPage();
	void createSettingsPage();
	void createCoachesPage();
	void createClientsPage();
};
