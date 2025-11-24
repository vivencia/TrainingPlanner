#include "qmlmesocalendarinterface.h"

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
	  m_calendarModel{appMesoModel()->mesoCalendarManager()->calendar(meso_idx)}
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appMesoModel()->mesoCalendarManager(), &DBMesoCalendarManager::calendarChanged, this,
						[this,conn] (const uint meso_idx, const uint field, const int calendar_day)
	{
		if (meso_idx == m_mesoIdx && field == MESOCALENDAR_TOTAL_COLS)
		{
			disconnect(*conn);
			m_calendarModel = appMesoModel()->mesoCalendarManager()->calendar(meso_idx);
			if (m_calPage)
			{
				m_calPage->setProperty("calendarModel", 0);
				m_calPage->setProperty("calendarModel", QVariant::fromValue(m_calendarModel));
			}
		}
	});
}

void QmlMesoCalendarInterface::cleanUp()
{
	delete m_calPage;
	delete m_calComponent;
}

void QmlMesoCalendarInterface::getMesoCalendarPage()
{
	if (!m_calComponent)
		createMesoCalendarPage();
	else
		appPagesListModel()->openPage(m_calPage);
}

void QmlMesoCalendarInterface::setSelectedDate(const QDate &new_date)
{
	if (new_date != m_selectedDate)
	{
		m_selectedDate = new_date;
		m_selectedSplitLetter = std::move(m_calendarModel->splitLetter(m_selectedDate));
		m_selectedWorkout = std::move(m_calendarModel->workoutNumber(m_selectedDate));
		emit selectedDateChanged();
	}
}

void QmlMesoCalendarInterface::changeSplitLetter(const QString &newSplitLetter, const bool bUntillTheEnd)
{
	if (!bUntillTheEnd)
		m_calendarModel->setSplitLetter(m_selectedDate, newSplitLetter);
	else
	{
		//TODO
	}
	emit selectedSplitLetterChanged();
}

void QmlMesoCalendarInterface::getWorkoutPage()
{
	qobject_cast<QMLMesoInterface*>(parent())->getWorkoutPage(m_selectedDate);
}

QString QmlMesoCalendarInterface::dayInfo()
{
	if (!m_calendarModel || !m_selectedDate.isValid())
		return QString{};

	if (m_selectedSplitLetter.isEmpty())
		return tr("Selected day is not part of the current mesocycle");
	else if (m_selectedSplitLetter != "R"_L1)
		return std::move(appUtils()->formatDate(m_selectedDate)) + std::move(tr(": Workout #")) +
			m_selectedWorkout + std::move(tr(" Split: ")) + m_selectedSplitLetter + " - "_L1 +
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
	switch (m_calComponent->status())
	{
		case QQmlComponent::Ready:
			createMesoCalendarPage_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_calComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createMesoCalendarPage_part2();
			}, Qt::SingleShotConnection);
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_calComponent->errorString();
			#endif
		break;
	}
}

void QmlMesoCalendarInterface::createMesoCalendarPage_part2()
{
	m_calProperties.insert("calendarManager"_L1, QVariant::fromValue(this));
	if (m_calendarModel)
		m_calProperties.insert("calendarModel"_L1, QVariant::fromValue(m_calendarModel));
	m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
	m_calPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	appPagesListModel()->openPage(m_calPage, std::move(tr("Calendar: ") + appMesoModel()->name(m_mesoIdx)), [this] () { cleanUp(); });

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field)
	{
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
