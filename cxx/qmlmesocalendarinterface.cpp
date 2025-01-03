#include "qmlmesocalendarinterface.h"

#include "dbinterface.h"
#include "dbmesocalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

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
		if (!appMesoModel()->mesoCalendarModel(m_mesoIdx)->isReady())
			appDBInterface()->getMesoCalendar(m_mesoIdx);
		createMesoCalendarPage();
	}
	else
		emit addPageToMainMenu(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx), m_calPage);
}

void QmlMesoCalendarInterface::changeCalendar(const bool bUntillTheEnd, const QString& newSplitLetter)
{
	if (!bUntillTheEnd)
		appDBInterface()->updateMesoCalendarEntry(m_mesoIdx, m_selectedDate, m_selectedTrainingDay, newSplitLetter);
	else
		appDBInterface()->updateMesoCalendarModel(m_mesoIdx, m_selectedDate, newSplitLetter);
}

void QmlMesoCalendarInterface::getTrainingDayPage(const QDate& date)
{
	qobject_cast<QMLMesoInterface*>(parent())->getTrainingDayPage(date);
}

QString QmlMesoCalendarInterface::dayInfo(const uint year, const uint month, const uint day)
{
	if (day == 0 || year == 0)
		return QString();
	const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
	const bool bDayOK(mesoCal->isPartOfMeso(month, day));
	if (bDayOK)
	{
		m_selectedSplitLetter = std::move(mesoCal->getSplitLetter(month, day));
		emit selectedSplitLetterChanged();
		m_selectedTrainingDay = std::move(QString::number(mesoCal->getTrainingDay(month, day)));
		static_cast<void>(m_selectedDate.setDate(static_cast<int>(year), static_cast<int>(month), static_cast<int>(day+1)));

		if (m_selectedSplitLetter != "R"_L1)
			return std::move(appUtils()->formatDate(m_selectedDate)) + std::move(tr(": Workout #")) +
				m_selectedTrainingDay + std::move(tr(" Split: ")) + m_selectedSplitLetter + " - "_L1 +
				std::move(appMesoModel()->mesoSplitModel()->splitX(m_mesoIdx, appUtils()->splitLetterToMesoSplitIndex(m_selectedSplitLetter)));
		else
			return std::move(appUtils()->formatDate(m_selectedDate)) + std::move(tr(": Rest day"));
	}
	else
		return tr("Selected day is not part of the current mesocycle");
}


QString QmlMesoCalendarInterface::nameLabel() const
{
	return appMesoModel()->name(m_mesoIdx);
}

QString QmlMesoCalendarInterface::dateLabel() const
{
	return std::move(tr("from  <b>")) + std::move(appUtils()->formatDate(appMesoModel()->startDate(m_mesoIdx))) +
			std::move(tr("</b>  through  <b>")) + std::move(appUtils()->formatDate(appMesoModel()->endDate(m_mesoIdx))) + "</b>"_L1;
}

void QmlMesoCalendarInterface::createMesoCalendarPage()
{
	m_calComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesoCalendarPage.qml"_L1}, QQmlComponent::Asynchronous};
	m_calProperties.insert("calendarManager"_L1, QVariant::fromValue(this));
	m_calProperties.insert("mesoCalendarModel"_L1, QVariant::fromValue(appMesoModel()->mesoCalendarModel(m_mesoIdx)));

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
	m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (m_calComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_calComponent->errorString();
		for (uint i(0); i < m_calComponent->errors().count(); ++i)
			qDebug() << m_calComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
	m_calPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));

	connect(this, &QmlMesoCalendarInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QmlMesoCalendarInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx), m_calPage);

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		switch (field)
		{
			case MESOCYCLES_COL_NAME:
				emit nameLabelChanged();
			break;
			case MESOCYCLES_COL_STARTDATE:
			case MESOCYCLES_COL_ENDDATE:
				emit dateLabelChanged();
			break;
		}
	});
}
