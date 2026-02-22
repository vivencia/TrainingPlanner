#include "qmluserinterface.h"

#include "dbusermodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlUserInterface::getUserPage()
{
	if (m_userPage)
		appPagesListModel()->openPage(m_userPage);
	else {
		m_userProperties.insert("useMode"_L1, appUserModel()->appUseMode(0));
		m_userProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_userComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/UserPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_userComponent->status()) {
			case QQmlComponent::Ready:
				createUserPage();
			break;
			case QQmlComponent::Loading:
				connect(m_userComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
					createUserPage();
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_userComponent->errorString();
				#endif
			break;
		}
	}
}

void QmlUserInterface::getSettingsPage()
{
	if (m_settingsPage)
		appPagesListModel()->openPage(m_settingsPage);
	else {
		m_settingsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/SettingsPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_settingsComponent->status()) {
			case QQmlComponent::Ready:
				createSettingsPage();
			break;
			case QQmlComponent::Loading:
				connect(m_settingsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
					createSettingsPage();
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_settingsComponent->errorString();
				#endif
			break;
		}
	}
}

void QmlUserInterface::getCoachesPage()
{
	if (m_coachesPage)
		appPagesListModel()->openPage(m_coachesPage);
	else {
		m_coachesProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_coachesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/CoachesPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_coachesComponent->status()) {
			case QQmlComponent::Ready:
				createCoachesPage();
			break;
			case QQmlComponent::Loading:
				connect(m_coachesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
					createCoachesPage();
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_coachesComponent->errorString();
				#endif
			break;
		}
	}
}

void QmlUserInterface::getClientsPage()
{
	if (m_clientsPage)
		appPagesListModel()->openPage(m_clientsPage);
	else {
		m_clientsProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_clientsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ClientsPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_clientsComponent->status()) {
			case QQmlComponent::Ready:
				createClientsPage();
			break;
			case QQmlComponent::Loading:
				connect(m_clientsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
					createClientsPage();
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_clientsComponent->errorString();
				#endif
			break;
		}
	}
}

void QmlUserInterface::createUserPage()
{
	m_userPage = static_cast<QQuickItem*>(m_userComponent->createWithInitialProperties(m_userProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_userPage, QQmlEngine::CppOwnership);
	m_userPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_userPage, std::move(tr("User Settings")));
}

void QmlUserInterface::createSettingsPage()
{
	m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
	m_settingsPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_settingsPage, std::move(tr("Settings")));
}

void QmlUserInterface::createCoachesPage()
{
	m_coachesPage = static_cast<QQuickItem*>(m_coachesComponent->createWithInitialProperties(m_coachesProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_coachesPage, QQmlEngine::CppOwnership);
	m_coachesPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_coachesPage, std::move(tr("Coaches")));
}

void QmlUserInterface::createClientsPage()
{
	m_clientsPage = static_cast<QQuickItem*>(m_clientsComponent->createWithInitialProperties( m_clientsProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_clientsPage, QQmlEngine::CppOwnership);
	m_clientsPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_clientsPage, std::move(tr("Clients")));
}
