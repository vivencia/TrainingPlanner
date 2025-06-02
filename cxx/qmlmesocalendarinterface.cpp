#include "qmlmesocalendarinterface.h"

#include "dbinterface.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlMesoCalendarInterface::QmlMesoCalendarInterface(QObject *parent, const uint meso_idx)
	: QObject{parent}, m_calComponent{nullptr}, m_calPage{nullptr}, m_mesoIdx{meso_idx},
	  m_calendarModel{appMesoModel()->mesoCalendarManager()->calendar(meso_idx)}, m_selectedDate{std::move(appUtils()->today())}
{
	connect(m_calendarModel, &DBCalendarModel::modelChanged, this, [this] () {
		m_calPage->setProperty("calendarModel", 0);
		m_calPage->setProperty("calendarModel", QVariant::fromValue(m_calendarModel));
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
		if (!appMesoModel()->mesoCalendarManager()->hasDBData(m_mesoIdx))
			appDBInterface()->getMesoCalendar(m_mesoIdx);
		createMesoCalendarPage();
	}
	else
		emit addPageToMainMenu(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx), m_calPage);
}

void QmlMesoCalendarInterface::changeSplitLetter(const QString &newSplitLetter, const bool bUntillTheEnd)
{
	if (!bUntillTheEnd)
		m_calendarModel->setSplitLetter(m_selectedDate, newSplitLetter);
	else
	{
		//TODO
	}
}

void QmlMesoCalendarInterface::getWorkoutPage()
{
	qobject_cast<QMLMesoInterface*>(parent())->getWorkoutPage(m_selectedDate);
}

QString QmlMesoCalendarInterface::dayInfo()
{
	if (!m_selectedDate.isValid())
		return QString{};

	m_selectedSplitLetter = std::move(m_calendarModel->splitLetter(m_selectedDate));
	m_selectedTrainingDay = std::move(m_calendarModel->workoutNumber(m_selectedDate));
	emit selectedSplitLetterChanged();

	if (m_selectedSplitLetter.isEmpty())
		return tr("Selected day is not part of the current mesocycle");
	else if (m_selectedSplitLetter != "R"_L1)
		return std::move(appUtils()->formatDate(m_selectedDate)) + std::move(tr(": Workout #")) +
			m_selectedTrainingDay + std::move(tr(" Split: ")) + m_selectedSplitLetter + " - "_L1 +
				appMesoModel()->muscularGroup(m_mesoIdx, m_selectedSplitLetter.at(0));
	else
		return std::move(appUtils()->formatDate(m_selectedDate)) + std::move(tr(": Rest day"));

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
	m_calProperties.insert("calendarModel"_L1, QVariant::fromValue(m_calendarModel));

	if (m_calComponent->status() != QQmlComponent::Ready)
	{
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
			createMesoCalendarPage_part2();
		}, Qt::SingleShotConnection);
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
		for (uint i{0}; i < m_calComponent->errors().count(); ++i)
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
