#include "qmluserinterface.h"
#include "dbusermodel.h"
#include "dbinterface.h"

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
	if (m_clientsOrCoachesComponent)
	{
		delete m_clientsOrCoachesPage;
		delete m_clientsOrCoachesComponent;
	}
}

void QmlUserInterface::getSettingsPage(const uint startPageIndex)
{
	if (m_settingsComponent)
	{
		m_settingsPage->setProperty("startPageIndex", startPageIndex);
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
	}
	else
	{
		m_settingsProperties.insert(u"startPageIndex"_s, startPageIndex);
		m_settingsComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/ConfigurationPage.qml"_s}, QQmlComponent::Asynchronous};
		if (m_settingsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_settingsComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status) {
					return createSettingsPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createSettingsPage();
	}
}

void QmlUserInterface::getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	setClientsOrCoachesPagesProperties(bManageClients, bManageCoaches);

	if (m_clientsOrCoachesComponent)
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
	else
	{
		m_clientsOrCoachesProperties.insert(u"userManager"_s, QVariant::fromValue(this));
		m_clientsOrCoachesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/ClientsOrCoachesPage.qml"_s}, QQmlComponent::Asynchronous};
		if (m_clientsOrCoachesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_clientsOrCoachesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status) {
					return createClientsOrCoachesPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createClientsOrCoachesPage();
	}
}

void QmlUserInterface::removeUser(const uint user_row, const bool bCoach)
{
	appDBInterface()->removeUser(user_row, bCoach);
	const int curUserRow(appUserModel()->removeUser(user_row, bCoach));
	int firstUserRow(-1), lastUserRow(-1);
	if (curUserRow > 0)
	{
		firstUserRow = appUserModel()->findFirstUser(bCoach);
		lastUserRow = appUserModel()->findLastUser(bCoach);
	}
	m_clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
	m_clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
	m_clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
}

void QmlUserInterface::createSettingsPage()
{
	#ifdef DEBUG
	if (m_settingsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_settingsComponent->errorString();
		for (uint i(0); i < m_settingsComponent->errors().count(); ++i)
			qDebug() << m_settingsComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_settingsComponent->status() == QQmlComponent::Ready)
	{
		m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->createWithInitialProperties(m_settingsProperties, m_qmlEngine->rootContext()));
		m_qmlEngine->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
		m_settingsPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
		m_userPage = m_settingsPage->findChild<QQuickItem*>(u"userPage"_s);
		m_userPage->setProperty("useMode", appUserModel()->appUseMode(0));
		m_userPage->setProperty("userManager", QVariant::fromValue(this));

		connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE)
			m_userPage->setProperty("useMode", appUserModel()->appUseMode(0));
		appDBInterface()->saveUser(user_row);
	});
	}
}

void QmlUserInterface::createClientsOrCoachesPage()
{
	#ifndef QT_NO_DEBUG
	if (m_clientsOrCoachesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_clientsOrCoachesComponent->errorString();
		for (uint i(0); i < m_clientsOrCoachesComponent->errors().count(); ++i)
			qDebug() << m_clientsOrCoachesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_clientsOrCoachesComponent->status() == QQmlComponent::Ready)
	{
		m_clientsOrCoachesPage = static_cast<QQuickItem*>(m_clientsOrCoachesComponent->createWithInitialProperties(
				m_clientsOrCoachesProperties, m_qmlEngine->rootContext()));
		m_qmlEngine->setObjectOwnership(m_clientsOrCoachesPage, QQmlEngine::CppOwnership);
		m_clientsOrCoachesPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
	}
}

void QmlUserInterface::setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches)
{
	int curUserRow(0), firstUserRow(-1), lastUserRow(-1);

	if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_USER)
		return;

	if (bManageClients)
	{
		if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = appUserModel()->findFirstUser(false);
			lastUserRow = appUserModel()->findLastUser(false);
			curUserRow = appUserModel()->currentClient(0);
		}
	}

	if (bManageCoaches)
	{
		if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_USER_WITH_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = appUserModel()->findFirstUser(true);
			lastUserRow = appUserModel()->findLastUser(true);
			curUserRow = appUserModel()->currentCoach(0);
		}
	}

	if (curUserRow == 0)
		curUserRow = lastUserRow;
	if (m_clientsOrCoachesPage)
	{
		m_clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
		m_clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
		m_clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
		m_clientsOrCoachesPage->setProperty("showUsers", bManageClients);
		m_clientsOrCoachesPage->setProperty("showCoaches", bManageCoaches);

	}
	else
	{
		m_clientsOrCoachesProperties.insert(u"curUserRow"_s, curUserRow);
		m_clientsOrCoachesProperties.insert(u"firstUserRow"_s, firstUserRow);
		m_clientsOrCoachesProperties.insert(u"lastUserRow"_s, lastUserRow);
		m_clientsOrCoachesProperties.insert(u"showUsers"_s, bManageClients);
		m_clientsOrCoachesProperties.insert(u"showCoaches"_s, bManageCoaches);
	}
}
