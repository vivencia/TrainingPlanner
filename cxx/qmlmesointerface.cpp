#include "qmlmesointerface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "qmltdayinterface.h"
#include "qmlmesosplitinterface.h"
#include "qmlmesocalendarinterface.h"
#include "dbinterface.h"
#include "osinterface.h"
#include "tpappcontrol.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QMLMesoInterface::QMLMesoInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
	: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_mesoComponent(nullptr), m_mesoIdx(meso_idx),
		m_exercisesPage(nullptr), m_calendarPage(nullptr)
{
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
		{
			m_mesoIdx = new_meso_idx;
			const QMap<QDate,QmlTDayInterface*>::const_iterator itr_end(m_tDayPages.constEnd());
			QMap<QDate,QmlTDayInterface*>::const_iterator itr(m_tDayPages.constBegin());
			while (itr != itr_end)
			{
				(*itr)->setMesoIdx(new_meso_idx);
				++itr;
			}
			if (m_exercisesPage)
				m_exercisesPage->setMesoIdx(new_meso_idx);
			if (m_calendarPage)
				m_calendarPage->setMesoIdx(new_meso_idx);
		}
	});
	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE)
			setPropertiesBasedOnUseMode();
	});
}

QMLMesoInterface::~QMLMesoInterface()
{
	emit removePageFromMainMenu(m_mesoPage);
	delete m_mesoPage;
	delete m_mesoComponent;
	if (m_exercisesPage)
		delete m_exercisesPage;
	if (m_calendarPage)
		delete m_calendarPage;
	const QMap<QDate,QmlTDayInterface*>::const_iterator itr_end(m_tDayPages.constEnd());
	QMap<QDate,QmlTDayInterface*>::const_iterator itr(m_tDayPages.constBegin());
	while (itr != itr_end)
	{
		delete *itr;
		++itr;
	}
}

void QMLMesoInterface::setOwnMeso(const bool new_value, const bool bFromQml)
{
	if (m_bOwnMeso != new_value)
	{
		m_bOwnMeso = new_value;
		if (bFromQml)
			appMesoModel()->setOwnMeso(m_mesoIdx, m_bOwnMeso);
	}
}

bool QMLMesoInterface::isNewMeso() const
{
	return appMesoModel()->isNewMeso(m_mesoIdx);
}

QString QMLMesoInterface::nameLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_NAME);
}

QString QMLMesoInterface::coachLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_COACH);
}

QString QMLMesoInterface::clientLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_CLIENT);
}

QString QMLMesoInterface::typeLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_TYPE);
}

QString QMLMesoInterface::startDateLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_STARTDATE);
}

QString QMLMesoInterface::endDateLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_ENDDATE);
}

QString QMLMesoInterface::weeksLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_WEEKS);
}

QString QMLMesoInterface::splitLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_SPLIT);
}

QString QMLMesoInterface::notesLabel() const
{
	return appMesoModel()->columnLabel(MESOCYCLES_COL_NOTE);
}

void QMLMesoInterface::setName(const QString& new_value, const bool bFromQml)
{
	if (m_name != new_value)
	{
		m_name = new_value;
		if (bFromQml)
		{
			emit nameChanged();
			appMesoModel()->setName(m_mesoIdx, m_name);
		}
	}
}

void QMLMesoInterface::setCoach(const QString& new_value, const bool bFromQml)
{
	if (m_coach != new_value)
	{
		m_coach = new_value;
		if (bFromQml)
		{
			emit coachChanged();
			appMesoModel()->setCoach(m_mesoIdx, m_coach);
		}
	}
}

void QMLMesoInterface::setClient(const QString& new_value, const bool bFromQml)
{
	if (m_client != new_value)
	{
		m_client = new_value;
		if (bFromQml)
		{
			emit clientChanged();
			appMesoModel()->setClient(m_mesoIdx, m_client);
		}
	}
}

void QMLMesoInterface::setType(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		const QString& good_filepath(appUtils()->getCorrectPath(new_value));
		if (!appUtils()->canReadFile(good_filepath))
			return;
		if (m_type != new_value)
		{
			m_type = new_value;
			emit typeChanged();
			appMesoModel()->setType(m_mesoIdx, m_type);
		}
	}
	else
		m_type = new_value;
}

QString QMLMesoInterface::fileName() const
{
	return appUtils()->getFileName(m_file);
}

void QMLMesoInterface::setFile(const QString& new_value, const bool bFromQml)
{
	if (m_file != new_value)
	{
		m_file = new_value;
		if (bFromQml)
			appMesoModel()->setFile(m_mesoIdx, m_file);
	}
}

void QMLMesoInterface::setRealMeso(const bool new_value, const bool bFromQml)
{
	if (m_bRealMeso != new_value)
	{
		m_bRealMeso = new_value;
		if (bFromQml)
		{
			emit realMesoChanged();
			appMesoModel()->setIsRealMeso(m_mesoIdx, m_bRealMeso);
			setMaximumMesoEndDate(m_bRealMeso ? maximumMesoEndDate() : appMesoModel()->endDate(m_mesoIdx));
		}
	}
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate& new_value, const bool bFromQml)
{
	if (m_minimumMesoStartDate != new_value)
	{
		m_minimumMesoStartDate = new_value;
		m_startDate = appUtils()->formatDate(m_minimumMesoStartDate);
		if (bFromQml)
		{
			emit minimumMesoStartDateChanged();
			emit startDateChanged();
			appMesoModel()->setStartDate(m_mesoIdx, m_minimumMesoStartDate);
			appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_minimumMesoStartDate, m_maximumMesoEndDate)));
		}
	}
}

void QMLMesoInterface::setMaximumMesoEndDate(const QDate& new_value, const bool bFromQml)
{
	if (m_maximumMesoEndDate != new_value)
	{
		m_maximumMesoEndDate = new_value;
		m_endDate = appUtils()->formatDate(m_maximumMesoEndDate);
		if (bFromQml)
		{
			emit maximumMesoEndDateChanged();
			emit endDateChanged();
			appMesoModel()->setEndDate(m_mesoIdx, m_maximumMesoEndDate);
			appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_minimumMesoStartDate, m_maximumMesoEndDate)));
		}
	}
}

void QMLMesoInterface::setWeeks(const QString& new_value, const bool bFromQml)
{
	if (m_weeks != new_value)
	{
		m_weeks = new_value;
		if (bFromQml)
			emit weeksChanged();
	}
}

void QMLMesoInterface::setSplit(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_split != new_value)
		{
			if (new_value.contains(u"R"_qs))
			{
				m_split = new_value;
				emit splitChanged();
				appMesoModel()->setSplit(m_mesoIdx, m_split);
			}
		}
	}
	else
		m_split = new_value;
}

void QMLMesoInterface::setNotes(const QString& new_value, const bool bFromQml)
{
	m_notes = new_value;
	if (bFromQml)
		emit notesChanged();
}

void QMLMesoInterface::setMuscularGroupA(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(0) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'A', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[0] = new_value;
	emit muscularGroupAChanged();
}

void QMLMesoInterface::setMuscularGroupB(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(1) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'B', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[1] = new_value;
	emit muscularGroupBChanged();
}

void QMLMesoInterface::setMuscularGroupC(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(2) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'C', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[2] = new_value;
	emit muscularGroupCChanged();
}

void QMLMesoInterface::setMuscularGroupD(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(3) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'D', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[3] = new_value;
	emit muscularGroupDChanged();
}

void QMLMesoInterface::setMuscularGroupE(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(4) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'E', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[4] = new_value;
	emit muscularGroupEChanged();
}

void QMLMesoInterface::setMuscularGroupF(const QString& new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(5) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'F', new_value, m_muscularGroupId);
		else
			return;
	}
	m_muscularGroup[5] = new_value;
	emit muscularGroupFChanged();
}

void QMLMesoInterface::setCalendarStartDate(const QDate& new_value)
{
	if (m_calendarStartDate != new_value)
	{
		m_calendarStartDate = new_value;
		emit calendarStartDateChanged();
	}
}

void QMLMesoInterface::changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday)
{
	appDBInterface()->changeMesoCalendar(m_mesoIdx, preserve_old_cal, preserve_untilyesterday);
}

void QMLMesoInterface::getCalendarPage()
{
	if (!m_calendarPage)
		m_calendarPage = new QmlMesoCalendarInterface(this, this, m_qmlEngine, m_mainWindow, m_mesoIdx);
	m_calendarPage->getMesoCalendarPage();
}

void QMLMesoInterface::getExercisesPlannerPage()
{
	if (!m_exercisesPage)
		m_exercisesPage = new QmlMesoSplitInterface(this, m_qmlEngine, m_mainWindow, m_mesoIdx);
	m_exercisesPage->getExercisesPlannerPage();
}

void QMLMesoInterface::getTrainingDayPage(const QDate& date)
{
	QmlTDayInterface* tDayPage(m_tDayPages.value(date));
	if (!tDayPage)
	{
		tDayPage = new QmlTDayInterface(this, m_qmlEngine, m_mainWindow, m_mesoIdx, date);
		connect(tDayPage, &QmlTDayInterface::requestMesoSplitModel, this, [=,this] (const QChar& splitletter) {
			if (!m_exercisesPage)
				m_exercisesPage = new QmlMesoSplitInterface(this, m_qmlEngine, m_mainWindow, m_mesoIdx);
			DBMesoSplitModel* splitModel(m_exercisesPage->getSplitModel(splitletter));
			if (!splitModel->isReady())
			{
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_idx, const QVariant data) {
					if (table_idx == MESOSPLIT_TABLE_ID)
					{
						disconnect(*conn);
						tDayPage->loadExercisesFromMesoPlan(splitModel);
					}
				});
			}
			else
				appDBInterface()->loadCompleteMesoSplit(splitModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));

		connect(tDayPage, &QmlTDayInterface::convertTDayToSplitPlan, this, [=,this] (const DBTrainingDayModel* const tDayModel) {
			if (!m_exercisesPage)
				m_exercisesPage = new QmlMesoSplitInterface(this, m_qmlEngine, m_mainWindow, m_mesoIdx);
			DBMesoSplitModel* splitModel(m_exercisesPage->getSplitModel(tDayModel->_splitLetter()));
			if (!splitModel->isReady())
			{
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_idx, const QVariant data) {
					if (table_idx == MESOSPLIT_TABLE_ID)
					{
						disconnect(*conn);
						splitModel->convertFromTDayModel(tDayModel);
					}
				});
			}
			else
				splitModel->convertFromTDayModel(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		m_tDayPages.insert(date, tDayPage);
	}
	tDayPage->getTrainingDayPage();
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

	m_muscularGroupId = QTime::currentTime().msecsSinceStartOfDay();

	setName(appMesoModel()->name(m_mesoIdx), false);
	setCoach(appMesoModel()->coach(m_mesoIdx), false);
	setClient(appMesoModel()->client(m_mesoIdx), false);
	setType(appMesoModel()->type(m_mesoIdx), false);
	setFile(appMesoModel()->file(m_mesoIdx), false);
	setPropertiesBasedOnUseMode();
	setOwnMeso(appMesoModel()->isOwnMeso(m_mesoIdx), false);
	setRealMeso(appMesoModel()->isRealMeso(m_mesoIdx), false);
	setMinimumMesoStartDate(!minimumMesoStartDate.isNull() ? minimumMesoStartDate : appMesoModel()->getPreviousMesoEndDate(meso_id), false);
	setMaximumMesoEndDate(!maximumMesoEndDate.isNull() ? maximumMesoEndDate : appMesoModel()->getNextMesoStartDate(meso_id), false);
	setWeeks(appMesoModel()->nWeeks(m_mesoIdx), false);
	setNotes(appMesoModel()->notes(m_mesoIdx), false);
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'A'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'B'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'C'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'D'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'E'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'F'));
	m_muscularGroup.append(std::move(tr("Rest day")));
	setCalendarStartDate(!calendarStartDate.isNull() ? calendarStartDate: appMesoModel()->startDate(m_mesoIdx));

	m_mesoProperties.insert(u"mesoManager"_qs, QVariant::fromValue(this));

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
	#ifndef QT_NO_DEBUG
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

	emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);

	connect(appUserModel(), &DBUserModel::userAdded, this, [this] (const uint user_row) {
		QMetaObject::invokeMethod(m_mesoPage, "updateCoachesAndClientsModels", Q_ARG(int, static_cast<int>(user_row)));
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, this, [this] (const uint meso_idx) {
		if (meso_idx == m_mesoIdx)
			QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
	});
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const uint initiator_id, const int splitIndex, const QChar& splitLetter) {
		if (meso_idx == m_mesoIdx && initiator_id != m_muscularGroupId )
			updateMuscularGroupFromOutside(splitIndex);
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint meso_field) {
		if (meso_idx == m_mesoIdx)
		{
			if (!appMesoModel()->isNewMeso(meso_idx))
				appDBInterface()->saveMesocycle(meso_idx);
		}
	});
	connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, this, [this] (const uint meso_idx) {
		if (meso_idx == m_mesoIdx)
			emit isNewMesoChanged();
	});
}

void QMLMesoInterface::setPropertiesBasedOnUseMode()
{
	const uint useMode(appUserModel()->appUseMode(0));
	setOwnerIsCoach(useMode == APP_USE_MODE_SINGLE_COACH || useMode == APP_USE_MODE_COACH_USER_WITH_COACH);
	setHasCoach(useMode == APP_USE_MODE_SINGLE_USER_WITH_COACH || useMode == APP_USE_MODE_COACH_USER_WITH_COACH);
}

void QMLMesoInterface::updateMuscularGroupFromOutside(const uint splitIndex)
{
	switch (splitIndex)
	{
		case 0: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'A'), false); break;
		case 1: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'B'), false); break;
		case 2: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'C'), false); break;
		case 3: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'D'), false); break;
		case 4: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'E'), false); break;
		case 5: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'F'), false); break;
	}
}
