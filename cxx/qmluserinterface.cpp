#include "qmluserinterface.h"

#include "dbusermodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlUserInterface::~QmlUserInterface()
{
	if (m_settingsComponent)
	{
		delete m_settingsPage;
		delete m_settingsComponent;
	}
	if (m_coachesPage)
	{
		delete m_coachesPage;
		delete m_coachesComponent;
	}
	if (m_clientsPage)
	{
		delete m_clientsPage;
		delete m_clientsComponent;
	}
}

void QmlUserInterface::getSettingsPage(const uint startPageIndex)
{
	if (m_settingsComponent)
	{
		m_settingsPage->setProperty("startPageIndex", startPageIndex);
		appPagesListModel()->openPage(m_settingsPage);
	}
	else
	{
		m_settingsProperties.insert("startPageIndex"_L1, startPageIndex);
		m_settingsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ConfigurationPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_settingsComponent->status())
		{
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
	else
	{
		m_coachesProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_coachesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/CoachesPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_coachesComponent->status())
		{
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
	else
	{
		m_clientsProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_clientsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ClientsPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_clientsComponent->status())
		{
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

void QmlUserInterface::createSettingsPage()
{
	m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->createWithInitialProperties(m_settingsProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
	m_settingsPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_settingsPage, std::move(tr("Settings")));
	m_userPage = m_settingsPage->findChild<QQuickItem*>("userPage"_L1);
	m_userPage->setProperty("useMode", appUserModel()->appUseMode(0));
	m_userPage->setProperty("userManager", QVariant::fromValue(this));
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
	m_clientsPage = static_cast<QQuickItem*>(m_clientsComponent->createWithInitialProperties(
			m_clientsProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_clientsPage, QQmlEngine::CppOwnership);
	m_clientsPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_clientsPage, std::move(tr("Clients")));
}
