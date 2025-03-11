#include "qmluserinterface.h"

#include "dbinterface.h"
#include "dbusermodel.h"

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
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
	}
	else
	{
		m_settingsProperties.insert("startPageIndex"_L1, startPageIndex);
		m_settingsComponent = new QQmlComponent{m_qmlEngine, QUrl{"qrc:/qml/Pages/ConfigurationPage.qml"_L1}, QQmlComponent::Asynchronous};
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

void QmlUserInterface::getCoachesPage()
{
	if (m_coachesPage)
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_coachesPage));
	else
	{
		m_coachesProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_coachesComponent = new QQmlComponent{m_qmlEngine, QUrl{"qrc:/qml/Pages/CoachesPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_coachesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_coachesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status) {
					return createCoachesPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createCoachesPage();
	}
}

void QmlUserInterface::getClientsPage()
{
	if (m_clientsPage)
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsPage));
	else
	{
		m_clientsProperties.insert("userManager"_L1, QVariant::fromValue(this));
		m_clientsComponent = new QQmlComponent{m_qmlEngine, QUrl{"qrc:/qml/Pages/ClientsPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_clientsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_clientsComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status) {
					return createClientsPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createClientsPage();
	}
}

void QmlUserInterface::userModifiedSlot(const uint user_row, const uint field)
{
	if (user_row == 0)
	{
		switch(field)
		{
			case USER_COL_APP_USE_MODE:
				m_userPage->setProperty("useMode", appUserModel()->appUseMode(0)); //if user_row == 0, then m_userPage exists
			break;
			case USER_COL_AVATAR:
				if (m_userPage)
					QMetaObject::invokeMethod(m_userPage, "avatarChangedBySexSelection", Q_ARG(int, static_cast<int>(user_row)));
				if (m_clientsPage)
					QMetaObject::invokeMethod(m_clientsPage, "avatarChangedBySexSelection", Q_ARG(int, static_cast<int>(user_row)));
		}
	}
}

void QmlUserInterface::createSettingsPage()
{
	#ifndef QT_NO_DEBUG
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
		m_userPage = m_settingsPage->findChild<QQuickItem*>("userPage"_L1);
		m_userPage->setProperty("useMode", appUserModel()->appUseMode(0));
		m_userPage->setProperty("userManager", QVariant::fromValue(this));

		connect(appUserModel(), SIGNAL(userModified(const uint,const uint)), this, SLOT(userModifiedSlot(const uint,const uint)),
			static_cast<Qt::ConnectionType>(Qt::UniqueConnection|Qt::AutoConnection));
	}
}

void QmlUserInterface::createCoachesPage()
{
	#ifndef QT_NO_DEBUG
	if (m_coachesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_coachesComponent->errorString();
		for (uint i(0); i < m_coachesComponent->errors().count(); ++i)
			qDebug() << m_coachesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_coachesComponent->status() == QQmlComponent::Ready)
	{
		m_coachesPage = static_cast<QQuickItem*>(m_coachesComponent->createWithInitialProperties(
				m_coachesProperties, m_qmlEngine->rootContext()));
		m_qmlEngine->setObjectOwnership(m_coachesPage, QQmlEngine::CppOwnership);
		m_coachesPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_coachesPage));

		connect(appUserModel(), SIGNAL(userModified(const uint,const uint)), this, SLOT(userModifiedSlot(const uint,const uint)),
			static_cast<Qt::ConnectionType>(Qt::UniqueConnection|Qt::AutoConnection));
	}
}

void QmlUserInterface::createClientsPage()
{
	#ifndef QT_NO_DEBUG
	if (m_clientsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_clientsComponent->errorString();
		for (uint i(0); i < m_clientsComponent->errors().count(); ++i)
			qDebug() << m_clientsComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_clientsComponent->status() == QQmlComponent::Ready)
	{
		m_clientsPage = static_cast<QQuickItem*>(m_clientsComponent->createWithInitialProperties(
				m_clientsProperties, m_qmlEngine->rootContext()));
		m_qmlEngine->setObjectOwnership(m_clientsPage, QQmlEngine::CppOwnership);
		m_clientsPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsPage));

		connect(appUserModel(), SIGNAL(userModified(const uint,const uint)), this, SLOT(userModifiedSlot(const uint,const uint)),
			static_cast<Qt::ConnectionType>(Qt::UniqueConnection|Qt::AutoConnection));
	}
}
