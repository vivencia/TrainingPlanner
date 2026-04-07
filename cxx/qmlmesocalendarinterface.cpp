#include "qmlmesocalendarinterface.h"

#include "dbmesocyclesmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlMesoCalendarInterface::QmlMesoCalendarInterface(QObject *parent, DBMesocyclesModel *meso_model,
																			DBCalendarModel* calendar_model, const uint meso_idx)
	: QObject{parent}, m_mesoModel{meso_model}, m_mesoIdx{meso_idx}, m_calendarModel{calendar_model}
{
	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx) {
			switch (field) {
			case DBMesocyclesModel::MESO_FIELD_NAME: emit nameLabelChanged(); break;
			case DBMesocyclesModel::MESO_FIELD_STARTDATE:
			case DBMesocyclesModel::MESO_FIELD_ENDDATE: emit dateLabelChanged() ; break;
			default: break;
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
	if (!m_calComponent) {
		m_calProperties["calendarManager"_L1] = std::move(QVariant::fromValue(this));
		m_calProperties["calendarModel"_L1] = std::move(QVariant::fromValue(m_calendarModel));
		m_calComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages", "MesoCalendarPage", QQmlComponent::Asynchronous};
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getMesoCalendarPage(); });
	}
	else {
		if (!m_calPage) {
			switch (m_calComponent->status()) {
			case QQmlComponent::Ready:
				m_calComponent->disconnect();
				createMesoCalendarPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_calComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_calPage);
	}
}

void QmlMesoCalendarInterface::changeSplitLetter(const QString &newSplitLetter, const bool bUntillTheEnd)
{
	if (!bUntillTheEnd)
		m_calendarModel->setSplitLetter(newSplitLetter);
	else
	{
		//TODO
	}
}

void QmlMesoCalendarInterface::getWorkoutPage()
{
	qobject_cast<QMLMesoInterface*>(parent())->getWorkoutPage(m_calendarModel->currentDate());
}

QString QmlMesoCalendarInterface::dayInfo()
{
	if (m_calendarModel->isPartOfMeso()) {
		if (m_calendarModel->splitLetter() != 'R')
			return appUtils()->formatDate(m_calendarModel->currentDate()) % tr(": Workout #") % m_calendarModel->workoutNumber() %
			tr(" Split: ") % m_calendarModel->splitLetter() % " - "_L1 % m_mesoModel->muscularGroup(m_mesoIdx,
			m_calendarModel->splitLetter().at(0));
		else
			return appUtils()->formatDate(m_calendarModel->currentDate()) % tr(": Rest day");
	}
	else
		return tr("Selected day is not part of the current mesocycle");
}

QString QmlMesoCalendarInterface::nameLabel() const
{
	return m_mesoModel->name(m_mesoIdx);
}

QString QmlMesoCalendarInterface::dateLabel() const
{
	return tr("from  <b>") % appUtils()->formatDate(m_mesoModel->startDate(m_mesoIdx)) % tr("</b>  through  <b>") %
														appUtils()->formatDate(m_mesoModel->endDate(m_mesoIdx)) % "</b>"_L1;
}

void QmlMesoCalendarInterface::createMesoCalendarPage()
{
	m_calPage = qobject_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
	m_calPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appPagesListModel()->openPage(m_calPage, std::move(tr("Calendar: ") + m_mesoModel->name(m_mesoIdx)), [this] () { cleanUp(); });

	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		switch (field) {
		case DBMesocyclesModel::MESO_FIELD_NAME:
			emit nameLabelChanged();
			break;
		case DBMesocyclesModel::MESO_FIELD_STARTDATE:
		case DBMesocyclesModel::MESO_FIELD_ENDDATE:
			emit dateLabelChanged();
			break;
		}
	});
}
