#include "qmlmesocalendarinterface.h"

#include "qmlmesointerface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbinterface.h"
#include "tpappcontrol.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlMesoCalendarInterface::QmlMesoCalendarInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_calComponent(nullptr), m_mesoIdx(meso_idx)
{
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
			m_mesoIdx = new_meso_idx;
	});
}

QmlMesoCalendarInterface::~QmlMesoCalendarInterface()
{
	emit removePageFromMainMenu(m_calPage);
	delete m_calPage;
	delete m_calComponent;
}

void QmlMesoCalendarInterface::getMesoCalendarPage()
{
	if (!m_calComponent)
	{
		appDBInterface()->getMesoCalendar(m_mesoIdx);
		createMesoCalendarPage();
	}
	else
		emit addPageToMainMenu(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx), m_calPage);
}

void QmlMesoCalendarInterface::createMesoCalendarPage()
{
	m_calComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/MesoCalendar.qml"_qs}, QQmlComponent::Asynchronous};
	m_calProperties.insert(u"calendarManager"_qs, QVariant::fromValue(this));
	m_calProperties.insert(u"mesoCalendarModel"_qs, QVariant::fromValue(appMesoModel()->mesoCalendarModel(m_mesoIdx)));

	if (m_calComponent->status() != QQmlComponent::Ready)
	{
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
			createMesoCalendarPage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createMesoCalendarPage_part2();
}

void QmlMesoCalendarInterface::createMesoCalendarPage_part2()
{
	m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, m_qmlEngine->rootContext()));
	#ifdef DEBUG
	if (m_calComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_calComponent->errorString();
		for (uint i(0); i < m_calComponent->errors().count(); ++i)
			qDebug() << m_calComponent->errors().at(i).description();
		return;
	}
	#endif
	m_qmlEngine->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
	m_calPage->setParentItem(m_mainWindow->findChild<QQuickItem*>("appStackView"));
	emit addPageToMainMenu(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx), m_calPage);
}
