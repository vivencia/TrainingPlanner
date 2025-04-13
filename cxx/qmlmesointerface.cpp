#include "qmlmesointerface.h"

#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesosplitinterface.h"
#include "qmltdayinterface.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QMLMesoInterface::~QMLMesoInterface()
{
	if (m_mesoComponent)
	{
		emit removePageFromMainMenu(m_mesoPage);
		delete m_mesoPage;
		delete m_mesoComponent;
	}
	if (m_exercisesPage)
		delete m_exercisesPage;
	if (m_calendarPage)
		delete m_calendarPage;

	for (const auto &it: m_tDayPages)
		delete it;
}

void QMLMesoInterface::setRealMeso(const bool new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_bRealMeso != new_value)
		{
			m_bRealMeso = new_value;
			emit realMesoChanged();
			appMesoModel()->setIsRealMeso(m_mesoIdx, m_bRealMeso);
			setEndDate(m_bRealMeso ? appMesoModel()->endDate(m_mesoIdx) : maximumMesoEndDate());
		}
	}
	else
		m_bRealMeso = new_value;
}

void QMLMesoInterface::setOwnMeso(const bool new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_bOwnMeso != new_value)
		{
			m_bOwnMeso = new_value;
			emit ownMesoChanged();
			appMesoModel()->setOwnMeso(m_mesoIdx, m_bOwnMeso);
			setClient(m_bOwnMeso ? appUserModel()->userId(0) : appUserModel()->defaultClient());
		}
	}
	else
		m_bOwnMeso = new_value;
}

bool QMLMesoInterface::isNewMeso() const
{
	return appMesoModel()->isNewMeso(m_mesoIdx);
}

bool QMLMesoInterface::isTempMeso() const
{
	return appMesoModel()->_id(m_mesoIdx) < 0;
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

void QMLMesoInterface::setName(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_name != new_value)
		{
			m_name = new_value;
			emit nameChanged();
			if (!isNewMeso())
				acceptName();
		}
	}
	else
		m_name = new_value;
}

void QMLMesoInterface::acceptName()
{
	appMesoModel()->setName(m_mesoIdx, m_name);
}

QString QMLMesoInterface::coach() const
{
	return appUserModel()->userNameFromId(m_coach);
}

void QMLMesoInterface::setCoach(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_coach != new_value)
		{
			m_coach = new_value;
			emit coachChanged();
			appMesoModel()->setCoach(m_mesoIdx, m_coach);
		}
	}
	else
		m_coach = new_value;
}

QString QMLMesoInterface::client() const
{
	return appUserModel()->userNameFromId(m_client);
}

void QMLMesoInterface::setClient(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_client != new_value)
		{
			m_client = new_value;
			emit clientChanged();
			appMesoModel()->setClient(m_mesoIdx, m_client);
			setMinimumMesoStartDate(appMesoModel()->getMesoMinimumStartDate(m_client, m_mesoIdx));
			setStartDate(m_minimumMesoStartDate);
		}
	}
	else
		m_client = new_value;
}

void QMLMesoInterface::setType(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
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

void QMLMesoInterface::setFile(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_file != new_value)
		{
			const QString &good_filepath{appUtils()->getCorrectPath(new_value)};
			if (!appUtils()->canReadFile(good_filepath))
				return;
			m_file = new_value;
			emit fileChanged();
			emit fileNameChanged();
			appMesoModel()->setFile(m_mesoIdx, m_file);
		}
	}
	else
		m_file = new_value;
}

void QMLMesoInterface::setStartDate(const QDate &new_value, const bool bFromQml)
{
	if (m_startDate != new_value)
	{
		m_startDate = new_value;
		m_strStartDate = std::move(appUtils()->formatDate(new_value));
		setWeeks(QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)), bFromQml);
		if (bFromQml)
		{
			emit startDateChanged();
			if (!isNewMeso())
				acceptStartDate();
		}
	}
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate &new_value)
{
	m_minimumMesoStartDate = new_value;
	emit minimumStartDateChanged();
}

void QMLMesoInterface::acceptStartDate()
{
	appMesoModel()->setStartDate(m_mesoIdx, m_startDate);
	appMesoModel()->setWeeks(m_mesoIdx, m_weeks);
}

void QMLMesoInterface::setEndDate(const QDate &new_value, const bool bFromQml)
{
	if (m_endDate != new_value)
	{
		m_endDate = new_value;
		m_strEndDate = std::move(appUtils()->formatDate(new_value));
		setWeeks(QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)), bFromQml);
		if (bFromQml)
		{
			emit endDateChanged();
			if (!isNewMeso())
				acceptEndDate();
		}
	}
}

void QMLMesoInterface::setMaximumMesoEndDate(const QDate &new_value)
{
	m_maximumMesoEndDate = new_value;
}

void QMLMesoInterface::acceptEndDate()
{
	appMesoModel()->setEndDate(m_mesoIdx, m_endDate);
	appMesoModel()->setWeeks(m_mesoIdx, m_weeks);
}

void QMLMesoInterface::setWeeks(const QString &new_value, const bool bFromQml)
{
	if (m_weeks != new_value)
	{
		m_weeks = new_value;
		if (bFromQml)
			emit weeksChanged();
	}
}

void QMLMesoInterface::setSplit(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (new_value.contains("R"_L1))
		{
			m_split = new_value;
			emit splitChanged();
			appMesoModel()->setSplit(m_mesoIdx, m_split);
		}
	}
	else
		m_split = new_value;
}

void QMLMesoInterface::setNotes(const QString &new_value, const bool bFromQml)
{
	m_notes = new_value;
	if (bFromQml)
		emit notesChanged();
}

void QMLMesoInterface::setMuscularGroupA(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(0) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'A', new_value);
		else
			return;
	}
	m_muscularGroup[0] = new_value;
	emit muscularGroupAChanged();
}

void QMLMesoInterface::setMuscularGroupB(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(1) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'B', new_value);
		else
			return;
	}
	m_muscularGroup[1] = new_value;
	emit muscularGroupBChanged();
}

void QMLMesoInterface::setMuscularGroupC(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(2) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'C', new_value);
		else
			return;
	}
	m_muscularGroup[2] = new_value;
	emit muscularGroupCChanged();
}

void QMLMesoInterface::setMuscularGroupD(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(3) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'D', new_value);
		else
			return;
	}
	m_muscularGroup[3] = new_value;
	emit muscularGroupDChanged();
}

void QMLMesoInterface::setMuscularGroupE(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(4) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'E', new_value);
		else
			return;
	}
	m_muscularGroup[4] = new_value;
	emit muscularGroupEChanged();
}

void QMLMesoInterface::setMuscularGroupF(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_muscularGroup.at(5) != new_value)
			appMesoModel()->setMuscularGroup(m_mesoIdx, 'F', new_value);
		else
			return;
	}
	m_muscularGroup[5] = new_value;
	emit muscularGroupFChanged();
}

void QMLMesoInterface::changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday)
{
	appDBInterface()->changeMesoCalendar(m_mesoIdx, preserve_old_cal, preserve_untilyesterday);
}

void QMLMesoInterface::getCalendarPage()
{
	if (!m_calendarPage)
		m_calendarPage = new QmlMesoCalendarInterface{this, m_mesoIdx};
	m_calendarPage->getMesoCalendarPage();
}

void QMLMesoInterface::getExercisesPlannerPage()
{
	if (!m_exercisesPage)
		m_exercisesPage = new QmlMesoSplitInterface{this, m_mesoIdx};
	m_exercisesPage->getExercisesPlannerPage();
}

void QMLMesoInterface::getTrainingDayPage(const QDate &date)
{
	QmlTDayInterface *tDayPage(m_tDayPages.value(date));
	if (!tDayPage)
	{
		tDayPage = new QmlTDayInterface{this, m_mesoIdx, date};
		m_tDayPages.insert(date, tDayPage);
	}
	tDayPage->getTrainingDayPage();
}

void QMLMesoInterface::getMesocyclePage()
{
	if (!m_mesoComponent)
		createMesocyclePage();
	else
		emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);
}

void QMLMesoInterface::exportMeso(const bool bShare, const bool bCoachInfo)
{
	int exportFileMessageId(APPWINDOW_MSG_EXPORT_OK);
	const QString &exportFileName{appItemManager()->setExportFileName(appMesoModel()->name(m_mesoIdx) + std::move(tr(" - TP Complete Meso.txt")))};
	static_cast<void>(QFile::remove(exportFileName)); //remove any left overs to avoid errors
	if (bCoachInfo)
	{
		appUserModel()->setExportRow(appUserModel()->getRowByCoachName(appMesoModel()->coach(m_mesoIdx)));
		exportFileMessageId = appUserModel()->exportToFile(exportFileName);
	}
	if (exportFileMessageId == APPWINDOW_MSG_EXPORT_OK)
	{
		appMesoModel()->setExportRow(m_mesoIdx);
		exportFileMessageId = appMesoModel()->exportToFile(exportFileName);
		if (exportFileMessageId == APPWINDOW_MSG_EXPORT_OK)
		{
			QChar splitletter('R');
			uint i(0);
			for (; i < appMesoModel()->split(m_mesoIdx).length(); ++i)
			{
				splitletter = appMesoModel()->split(m_mesoIdx).at(i);
				if (splitletter != 'R')
					break;
			}
			if (splitletter != 'R')
			{
				DBMesoSplitModel *splitModel(plannerSplitModel(splitletter));
				if (!splitModel)
				{
					auto conn = std::make_shared<QMetaObject::Connection>();
					*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this,&exportFileMessageId,&exportFileName]
																					(const uint table_idx, QVariant data) {
						if (table_idx == MESOSPLIT_TABLE_ID)
						{
							disconnect(*conn);
							const QMap<QChar,DBMesoSplitModel*> &allSplits(data.value<QMap<QChar,DBMesoSplitModel*>>());
							for (const auto splitModel : allSplits)
								splitModel->exportToFile(exportFileName);
							/*QMap<QChar,DBMesoSplitModel*>::const_iterator splitModel(allSplits.constBegin());
							const QMap<QChar,DBMesoSplitModel*>::const_iterator mapEnd(allSplits.constEnd());
							do {
								(*splitModel)->exportToFile(exportFileName);
							} while (++splitModel != mapEnd);*/
							appItemManager()->continueExport(exportFileMessageId, bShare);
						}
					});
					appDBInterface()->loadAllSplits(m_mesoIdx);
					return;
				}
				else
				{
					do
					{
						if (splitModel)
							splitModel->exportToFile(exportFileName);
						if (++i < appMesoModel()->split(m_mesoIdx).length())
						{
							splitletter = appMesoModel()->split(m_mesoIdx).at(i);
							splitModel = splitletter != 'R' ? plannerSplitModel(splitletter) : nullptr;
						}
						else
							break;
					} while (true);
					appItemManager()->continueExport(exportFileMessageId, bShare);
				}
			}
		}
	}
	emit displayMessageOnAppWindow(exportFileMessageId, exportFileName);
}

void QMLMesoInterface::importMeso(const QString &filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appItemManager()->openRequestedFile(filename, IFC_MESO);
}

void QMLMesoInterface::sendMesocycleFileToServer()
{
	appMesoModel()->sendMesoToUser(m_mesoIdx);
}

void QMLMesoInterface::incorporateMeso()
{
	appDBInterface()->saveMesocycle(m_mesoIdx);
}

DBMesoSplitModel *QMLMesoInterface::plannerSplitModel(const QChar &splitLetter)
{
	return m_exercisesPage ? m_exercisesPage->splitModel(splitLetter) : nullptr;
}

DBTrainingDayModel *QMLMesoInterface::tDayModelForToday()
{
	QmlTDayInterface *tDayPage(m_tDayPages.value(QDate::currentDate()));
	return tDayPage ? tDayPage->tDayModel() : nullptr;
}

void QMLMesoInterface::createMesocyclePage()
{
	if (!appMesoModel()->isNewMeso(m_mesoIdx))
	{
		setName(appMesoModel()->name(m_mesoIdx), false);
		setStartDate(appMesoModel()->startDate(m_mesoIdx), false);
		setEndDate(appMesoModel()->endDate(m_mesoIdx), false);
		setMinimumMesoStartDate(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
		setMaximumMesoEndDate(appMesoModel()->getMesoMaximumEndDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
	}
	else
	{
		setName(std::move(tr("New Program")), false);
		const QDate &minimumStartDate{appUtils()->getNextMonday(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), 99999))};
		const QDate &currentDate{QDate::currentDate()};
		setStartDate(currentDate, false);
		setEndDate(appUtils()->createDate(currentDate, 0, 2, 0), false);
		setMinimumMesoStartDate(minimumStartDate);
		setMaximumMesoEndDate(appUtils()->createDate(currentDate, 0, 6, 0));
	}

	setCoach(appMesoModel()->coach(m_mesoIdx), false);
	setClient(appMesoModel()->client(m_mesoIdx), false);
	setType(appMesoModel()->type(m_mesoIdx), false);
	setFile(appMesoModel()->file(m_mesoIdx), false);
	setPropertiesBasedOnUseMode();
	setOwnMeso(appMesoModel()->isOwnMeso(m_mesoIdx), false);
	setRealMeso(appMesoModel()->isRealMeso(m_mesoIdx), false);
	setSplit(appMesoModel()->split(m_mesoIdx), false);
	setNotes(appMesoModel()->notes(m_mesoIdx), false);
	setNewMesoFieldCounter(appMesoModel()->newMesoFieldCounter(m_mesoIdx));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'A'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'B'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'C'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'D'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'E'));
	m_muscularGroup.append(appMesoModel()->muscularGroup(m_mesoIdx, 'F'));
	m_muscularGroup.append(std::move(tr("Rest day")));

	m_mesoProperties.insert("mesoManager"_L1, QVariant::fromValue(this));

	m_mesoComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesocyclePage.qml"_L1}, QQmlComponent::Asynchronous};
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
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"_L1));

	connect(this, &QMLMesoInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QMLMesoInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);

	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE)
			setPropertiesBasedOnUseMode();
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
		{
			m_mesoIdx = new_meso_idx;
			const QMap<QDate,QmlTDayInterface*>::const_iterator itr_end(m_tDayPages.constEnd());
			QMap<QDate,QmlTDayInterface*>::const_iterator itr(m_tDayPages.constBegin());
			while (itr != itr_end)
			{
				(*itr)->setMesoIdx(m_mesoIdx);
				++itr;
			}
			if (m_exercisesPage)
				m_exercisesPage->setMesoIdx(m_mesoIdx);
			if (m_calendarPage)
				m_calendarPage->setMesoIdx(m_mesoIdx);
		}
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx)
			QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
	});
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar &splitLetter) {
		if (meso_idx == m_mesoIdx)
			updateMuscularGroupFromOutside(splitIndex);
	});

	if (appMesoModel()->isNewMeso(m_mesoIdx))
	{
		connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, this, [this] (const uint meso_idx) {
			if (meso_idx == m_mesoIdx)
				emit isNewMesoChanged();
		});
		connect(appMesoModel(), &DBMesocyclesModel::newMesoFieldCounterChanged, this, [this] (const uint meso_idx, const uint /*not yet used*/) {
			if (meso_idx == m_mesoIdx)
				setNewMesoFieldCounter(appMesoModel()->newMesoFieldCounter(m_mesoIdx));
		});
	}

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint meso_field) {
		if (meso_idx == m_mesoIdx)
		{
			if (!appMesoModel()->isNewMeso(m_mesoIdx))
			{
				appDBInterface()->saveMesocycle(m_mesoIdx);
				if (!ownMeso())
					appMesoModel()->checkIfCanExport(m_mesoIdx);
				else
					sendMesocycleFileToServer();
			}
		}
	});
	connect(appMesoModel(), &DBMesocyclesModel::canExportChanged, this, [this] (const uint meso_idx, const bool can_export) {
		if (meso_idx == m_mesoIdx)
		{
			m_bCanExport = can_export;
			emit canExportChanged();
		}
	});

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		appMesoModel()->fillColumnNames();
		emit labelsChanged();
	});
}

void QMLMesoInterface::setPropertiesBasedOnUseMode()
{
	const uint useMode{appUserModel()->appUseMode(0)};
	setOwnerIsCoach(useMode == APP_USE_MODE_SINGLE_COACH || useMode == APP_USE_MODE_COACH_USER_WITH_COACH);
	setHasCoach(useMode == APP_USE_MODE_SINGLE_USER_WITH_COACH || useMode == APP_USE_MODE_COACH_USER_WITH_COACH);
	emit labelsChanged();
}

void QMLMesoInterface::updateMuscularGroupFromOutside(const uint splitIndex)
{
	switch (splitIndex)
	{
		case 0: setMuscularGroupA(appMesoModel()->muscularGroup(m_mesoIdx, 'A'), false); break;
		case 1: setMuscularGroupB(appMesoModel()->muscularGroup(m_mesoIdx, 'B'), false); break;
		case 2: setMuscularGroupC(appMesoModel()->muscularGroup(m_mesoIdx, 'C'), false); break;
		case 3: setMuscularGroupD(appMesoModel()->muscularGroup(m_mesoIdx, 'D'), false); break;
		case 4: setMuscularGroupE(appMesoModel()->muscularGroup(m_mesoIdx, 'E'), false); break;
		case 5: setMuscularGroupF(appMesoModel()->muscularGroup(m_mesoIdx, 'F'), false); break;
	}
}
