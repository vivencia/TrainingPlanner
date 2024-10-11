#include "qmlmesointerface.h"
#include "dbmesocyclesmodel.h"
#include "dbinterface.h"
#include "osinterface.h"
#include "tpappcontrol.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QMLMesoInterface::QMLMesoInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
	: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_mesoComponent(nullptr), m_mesoIdx(meso_idx)
{
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
			m_mesoIdx = new_meso_idx;
	});
	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE) {
			if (m_mesoComponent)
				m_mesoPage->setProperty("useMode", appUserModel()->appUseMode(0));
		}
	});
}

QMLMesoInterface::~QMLMesoInterface()
{
	emit removePageFromMainMenu(m_mesoPage);
	delete m_mesoPage;
	delete m_mesoComponent;
}

void QMLMesoInterface::changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday)
{
	appDBInterface()->changeMesoCalendar(m_mesoIdx, preserve_old_cal, preserve_untilyesterday);
}

void QMLMesoInterface::getMesocyclePage()
{
	appMesoModel()->setCurrentMesoIdx(m_mesoIdx);
	if (!m_mesoComponent)
		createMesocyclePage();
	else
		emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);
}

void QMLMesoInterface::exportMeso(const bool bShare, const bool bCoachInfo)
{
	int exportFileMessageId(0);
	const QString& exportFileName{appOsInterface()->appDataFilesPath() + appMesoModel()->name(m_mesoIdx) + tr(" - TP Complete Meso.txt")};
	if (bCoachInfo)
	{
		appUserModel()->setExportRow(appUserModel()->getRowByCoachName(appMesoModel()->coach(m_mesoIdx)));
		appUserModel()->exportToFile(exportFileName);
	}
	appMesoModel()->setExportRow(m_mesoIdx);
	exportFileMessageId = appMesoModel()->exportToFile(exportFileName);
	//appDBInterface()->loadCompleteMesoSplits(m_mesoIdx, allSplitModels(), false);
	//exportMesoSplit(bShare, u"X"_qs, exportFileName, true);
	if (bShare)
	{
		appOsInterface()->shareFile(exportFileName);
		exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
	}
	else
		QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, exportFileName));

	emit displayMessageOnAppWindow(exportFileMessageId, exportFileName);
}

void QMLMesoInterface::importMeso(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(m_mainWindow, "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_MESO);
}

void QMLMesoInterface::createMesocyclePage(const QDate& minimumMesoStartDate, const QDate& maximumMesoEndDate, const QDate& calendarStartDate)
{
	const int meso_id(appMesoModel()->_id(m_mesoIdx));
	m_mesoProperties.insert(u"mesoManager"_qs, QVariant::fromValue(this));
	m_mesoProperties.insert(u"mesoIdx"_qs, m_mesoIdx);
	m_mesoProperties.insert(u"minimumMesoStartDate"_qs, !minimumMesoStartDate.isNull() ?
								minimumMesoStartDate : appMesoModel()->getPreviousMesoEndDate(meso_id));
	m_mesoProperties.insert(u"maximumMesoEndDate"_qs, !maximumMesoEndDate.isNull() ?
								maximumMesoEndDate : appMesoModel()->getNextMesoStartDate(meso_id));
	m_mesoProperties.insert(u"calendarStartDate"_qs, !calendarStartDate.isNull() ?
								calendarStartDate: appMesoModel()->startDate(m_mesoIdx));
	m_mesoMuscularGroupId = QTime::currentTime().msecsSinceStartOfDay();
	m_mesoProperties.insert(u"muscularGroupId"_qs, m_mesoMuscularGroupId);
	m_mesoCalendarChanged = false;

	m_mesoComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/MesoCycle.qml"_qs}, QQmlComponent::Asynchronous};
	if (m_mesoComponent->status() != QQmlComponent::Ready)
	{
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status) {
					createMesocyclePage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createMesocyclePage_part2();
}

void QMLMesoInterface::createMesocyclePage_part2()
{
	#ifdef DEBUG
	if (m_mesoComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_mesoComponent->errorString();
		for (uint i(0); i < m_mesoComponent->errors().count(); ++i)
			qDebug() << m_mesoComponent->errors().at(i).description();
		return;
	}
	#endif
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, m_qmlEngine->rootContext()));
	m_qmlEngine->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(m_mainWindow->findChild<QQuickItem*>("appStackView"));
	m_mesoPage->setProperty("useMode", appUserModel()->appUseMode(0));
	m_mesoPage->setProperty("bOwnMeso", appMesoModel()->isOwnMeso(m_mesoIdx));
	m_mesoPage->setProperty("bRealMeso", appMesoModel()->isRealMeso(m_mesoIdx));

	emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);

	connect(appUserModel(), &DBUserModel::userAdded, this, [this] (const uint user_row) {
		QMetaObject::invokeMethod(m_mesoPage, "updateCoachesModel", Q_ARG(int, static_cast<int>(user_row)));
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, this, [this] (const uint meso_idx) {
		if (meso_idx == m_mesoIdx)
			QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
	});
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const uint initiator_id, const int splitIndex, const QChar& splitLetter) {
		if (meso_idx == m_mesoIdx && initiator_id != m_mesoMuscularGroupId )
			QMetaObject::invokeMethod(m_mesoPage, "updateMuscularGroup", Q_ARG(int, splitIndex), Q_ARG(QString, QString(splitLetter)));
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint meso_field) {
		if (meso_idx == m_mesoIdx)
		{
			if (!appMesoModel()->isNewMeso(meso_idx))
				appDBInterface()->saveMesocycle(meso_idx);
			QMetaObject::invokeMethod(m_mesoPage, "updateFieldValues", Q_ARG(int, static_cast<int>(meso_field)), Q_ARG(int, static_cast<int>(meso_idx)));
			m_mesoPage->setProperty("bRealMeso", appMesoModel()->isRealMeso(meso_idx));
		}
	});
}

/*
DBTrainingDayModel* QmlTDayInterface::gettDayModel(const QDate& date)
{
	if (!m_tDayModels.contains(date))
	{
		DBTrainingDayModel* tDayModel{new DBTrainingDayModel(this, m_mesoIdx)};
		m_tDayModels.insert(date, tDayModel);
		m_CurrenttDayModel = tDayModel;
	}
	else
		m_CurrenttDayModel = m_tDayModels.value(date);
	return m_CurrenttDayModel;
}
*/
