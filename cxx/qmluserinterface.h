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
		: QObject{parent}, m_settingsComponent{nullptr}, m_coachesPage{nullptr}, m_clientsPage{nullptr}, m_userPage{nullptr} {}
	~QmlUserInterface();

	void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();

private:
	QQuickItem *m_settingsPage, *m_coachesPage, *m_clientsPage, *m_userPage;
	QQmlComponent *m_settingsComponent, *m_coachesComponent, *m_clientsComponent;
	QVariantMap m_settingsProperties, m_coachesProperties, m_clientsProperties;

	void createSettingsPage();
	void createCoachesPage();
	void createClientsPage();
};
