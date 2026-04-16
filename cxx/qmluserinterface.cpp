#include "qmluserinterface.h"

#include "pageslistmodel.h"
#include "qmlitemmanager.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlUserInterface::getUserPage()
{
	if (!m_userComponent) {
		m_userProperties["userManager"_L1] = std::move(QVariant::fromValue(this));
		m_userComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "UserPage"_L1, QQmlComponent::Asynchronous};
		connect(m_userComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getUserPage(); });
	}
	else {
		if (!m_userPage) {
			switch (m_userComponent->status()) {
			case QQmlComponent::Ready:
				m_userComponent->disconnect();
				createUserPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_userComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_userPage);
	}
}

void QmlUserInterface::getSettingsPage()
{
	if (!m_settingsComponent) {
		m_settingsComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "SettingsPage"_L1, QQmlComponent::Asynchronous};
		connect(m_settingsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getSettingsPage(); });
	}
	else {
		if (!m_settingsPage) {
			switch (m_settingsComponent->status()) {
			case QQmlComponent::Ready:
				m_settingsComponent->disconnect();
				createSettingsPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_settingsComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_settingsPage);
	}
}

void QmlUserInterface::getCoachesPage()
{
	if (!m_coachesComponent) {
		m_coachesProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_coachesComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "CoachesPage"_L1, QQmlComponent::Asynchronous};
		connect(m_coachesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getCoachesPage(); });
	}
	else {
		if (!m_coachesPage) {
			switch (m_coachesComponent->status()) {
			case QQmlComponent::Ready:
				m_coachesComponent->disconnect();
				createCoachesPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_coachesComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_coachesPage);
	}
}

void QmlUserInterface::getClientsPage()
{
	if (!m_clientsComponent) {
		m_clientsProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_clientsComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "ClientsPage"_L1, QQmlComponent::Asynchronous};
		connect(m_clientsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getClientsPage(); });
	}
	else {
		if (!m_clientsPage) {
			switch (m_clientsComponent->status()) {
			case QQmlComponent::Ready:
				m_clientsComponent->disconnect();
				createClientsPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_clientsComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_clientsPage);
	}
}

void QmlUserInterface::createUserPage()
{
	m_userPage = static_cast<QQuickItem*>(m_userComponent->createWithInitialProperties(m_userProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_userPage, QQmlEngine::CppOwnership);
	m_userPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appPagesListModel()->openPage(m_userPage, std::move(tr("User Settings")));
}

void QmlUserInterface::createSettingsPage()
{
	m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
	m_settingsPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appPagesListModel()->openPage(m_settingsPage, std::move(tr("Settings")));
}

void QmlUserInterface::createCoachesPage()
{
	m_coachesPage = static_cast<QQuickItem*>(m_coachesComponent->createWithInitialProperties(m_coachesProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_coachesPage, QQmlEngine::CppOwnership);
	m_coachesPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appPagesListModel()->openPage(m_coachesPage, std::move(tr("Coaches")));
}

void QmlUserInterface::createClientsPage()
{
	m_clientsPage = static_cast<QQuickItem*>(m_clientsComponent->createWithInitialProperties( m_clientsProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_clientsPage, QQmlEngine::CppOwnership);
	m_clientsPage->setParentItem(appMainWindow()->contentItem());
	appPagesListModel()->openPage(m_clientsPage, std::move(tr("Clients")));
}
