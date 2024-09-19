#include "tpappcontrol.h"
#include "translationclass.h"
#include "tputils.h"
#include "dbinterface.h"
#include "dbusermodel.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "qmlitemmanager.h"
#include "osinterface.h"

#include <QQmlApplicationEngine>

TPAppControl* TPAppControl::app_control(nullptr);
QSettings* TPAppControl::app_settings(nullptr);
TranslationClass* TPAppControl::app_tr(nullptr);
TPUtils* TPUtils::app_utils(nullptr);
DBInterface* TPAppControl::app_db_interface(nullptr);
DBUserModel* TPAppControl::app_user_model(nullptr);
DBMesocyclesModel* TPAppControl::app_meso_model(nullptr);
DBExercisesModel* TPAppControl::app_exercises_model(nullptr);
QmlItemManager* TPAppControl::app_root_items_manager(nullptr);
QQmlApplicationEngine* TPAppControl::app_qml_engine(nullptr);
OSInterface* TPAppControl::app_os_interface(nullptr);

OSInterface* appOsInterface()
{
	if (!TPAppControl::app_os_interface)
		TPAppControl::app_os_interface = new OSInterface{};
	return TPAppControl::app_os_interface;
}

#ifdef Q_OS_ANDROID
	#define FONT_POINT_SIZE 15
	#define FONT_POINT_SIZE_LISTS 11
	#define FONT_POINT_SIZE_TEXT 13
	#define FONT_POINT_SIZE_TITLE 20
#else
	#define FONT_POINT_SIZE 12
	#define FONT_POINT_SIZE_LISTS 8
	#define FONT_POINT_SIZE_TEXT 10
	#define FONT_POINT_SIZE_TITLE 18
#endif

TPAppControl::TPAppControl()
{
	app_control = this;
	QSettings _appSettings{};
	app_settings = &_appSettings;
	TranslationClass _trClass{};
	app_tr = &_trClass;
	_trClass.selectLanguage();
	TPUtils _tPUtils;
	TPUtils::app_utils = &_tPUtils;
	DBInterface _dbInterface;
	app_db_interface = &_dbInterface;
	DBUserModel _dbUserModel;
	app_user_model = &_dbUserModel;
	DBMesocyclesModel _dbMesocyclesModel;
	app_meso_model = &_dbMesocyclesModel;
	DBExercisesModel _dbExercisesModel;
	app_exercises_model = &_dbExercisesModel;
	QmlItemManager _qmlItemManager{0xFFFF};
	app_root_items_manager = &_qmlItemManager;
	QQmlApplicationEngine _qmlEngine;
	app_qml_engine = &_qmlEngine;
	#ifdef Q_OS_ANDROID
	new URIHandler(&db, &db);
	#endif

	populateSettingsWithDefaultValue();
	appDBInterface()->init();

	QmlItemManager::configureQmlEngine();
	createItemManager();

#ifdef Q_OS_ANDROID
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &TPUtils::appResumed, this, &DBInterface::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(appMainWindow(), "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	appStartUpNotifications();
#endif

	const QUrl url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);
	});
	appQmlEngine()->addImportPath(u":/"_qs);
	appQmlEngine()->load(url);
	if (appQmlEngine()->rootObjects().isEmpty())
		QCoreApplication::exit(-1);
	rootItemsManager()->initQML();
}

void TPAppControl::cleanUp()
{
	appDBInterface()->cleanUpThreads();
	for(uint i(0); i < m_itemManager.count(); ++i)
		delete m_itemManager.at(i);
}

void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	rootItemsManager()->getClientsOrCoachesPage(bManageClients, bManageCoaches);
}

void TPAppControl::getSettingsPage(const uint startPageIndex)
{
	rootItemsManager()->getSettingsPage(startPageIndex);
}

void TPAppControl::getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (appExercisesModel()->count() == 0)
		appDBInterface()->getAllExercises();
	rootItemsManager()->getExercisesPage(bChooseButtonEnabled, connectPage);
}

void TPAppControl::getMesocyclePage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getMesocyclePage();
}

uint TPAppControl::createNewMesocycle(const bool bCreatePage)
{
	QDate startDate, endDate, minimumStartDate;
	if (appMesoModel()->count() == 0)
	{
		minimumStartDate.setDate(2023, 0, 2); //first monday of that year
		startDate = QDate::currentDate();
		endDate = appUtils()->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (appMesoModel()->getInt(appMesoModel()->count() - 1, 8) == 1)
			minimumStartDate = appUtils()->getMesoStartDate(appMesoModel()->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		endDate = appUtils()->createFutureDate(minimumStartDate, 0, 2, 0);
	}

	const uint meso_idx = appMesoModel()->newMesocycle(QStringList() << STR_MINUS_ONE << qApp->tr("New Plan") << QString::number(startDate.toJulianDay()) <<
		QString::number(endDate.toJulianDay()) << QString() << QString::number(appUtils()->calculateNumberOfWeeks(startDate, endDate)) <<
		u"ABCDERR"_qs << QString() << appUserModel()->userName(0) << QString() << QString() << STR_ONE);

	QmlItemManager* itemMngr{new QmlItemManager{meso_idx}};
	m_itemManager.append(itemMngr);

	if (bCreatePage)
		itemMngr->createMesocyclePage(minimumStartDate, appUtils()->createFutureDate(startDate,0,6,0));

	return meso_idx;
}

void TPAppControl::removeMesocycle(const uint meso_idx)
{
	appDBInterface()->removeMesocycle(meso_idx);
	appMesoModel()->delMesocycle(meso_idx);
	QmlItemManager* itemMngr{m_itemManager.at(meso_idx)};
	QmlItemManager* tpObject(itemMngr);
	m_itemManager.remove(meso_idx);
	tpObject->disconnect();
	delete tpObject;
}

void TPAppControl::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	m_itemManager.at(meso_idx)->exportMeso(bShare, bCoachInfo);
}

void TPAppControl::getExercisesPlannerPage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getExercisesPlannerPage();
}

void TPAppControl::getMesoCalendarPage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getMesoCalendarPage();
}

void TPAppControl::getTrainingDayPage(const uint meso_idx, const QDate& date)
{
	m_itemManager.at(meso_idx)->getTrainingDayPage(date);
}

void TPAppControl::openRequestedFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return;
	}

	uint fileContents(0);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (inData.startsWith(u"##"_qs))
				{
					if (inData.indexOf(DBUserObjectName) != -1)
						fileContents |= IFC_USER;
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
						fileContents |= IFC_MESOSPLIT;
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						fileContents |= IFC_MESO;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						fileContents |= IFC_TDAY;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						fileContents |= IFC_EXERCISES;
				}
			}
		}
	}
	if (fileContents != 0)
	{
		QmlItemManager* itemMngr(nullptr);
		if (fileContents & IFC_MESO)
		{
			m_tempMesoIdx = createNewMesocycle(false);
			itemMngr = m_itemManager.at(m_tempMesoIdx);
		}
		else
		{
			if (fileContents & IFC_MESO || fileContents & IFC_TDAY)
				itemMngr = m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx());
			else if (fileContents & IFC_EXERCISES)
				itemMngr = rootItemsManager();
		}
		itemMngr->displayImportDialogMessage(fileContents, filename);
	}
}

/*Return values
 *	 0: success
 *	-1: Failed to open file
 *	-2: File format was not recognized
 *	-3: Nothing was imported, either because file was missing info or error in formatting
 *	-4: File has been previously imported
 */
int TPAppControl::importFromFile(const QString& filename, const bool bImportOptions[5])
{
	//if (filename.startsWith(u"file:"_qs))
	//	filename.remove(0, 7); //remove file://
	if (bImportOptions[0])
	{
		if (bImportOptions[1])
		{
			DBUserModel* modelUser{new DBUserModel};
			modelUser->importFromText()
		}
	}

	TPListModel* model(nullptr);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 2)
		{
			inData = buf;
			break;
		}
	}

	const bool bFancy(!inData.startsWith(u"##0x"_qs));
	int sep_idx(0);

	if (bFancy)
	{
		if (inData.indexOf(DBMesoSplitObjectName) != -1)
			model = new DBMesoSplitModel(this);
		else if (inData.indexOf(DBMesocyclesObjectName) != -1)
			model = new DBMesocyclesModel(this);
		//else if (inData.indexOf(DBTrainingDayObjectName) != -1)
			//model = new DBTrainingDayModel(this);
		else if (inData.indexOf(DBExercisesObjectName) != -1)
			model = new DBExercisesModel(this);
		else
			return -2;

		while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
		{
			if (lineLength > 2)
			{
				inData = buf;
				break;
			}
		}
	}


	model->deleteLater();
	if (!model->importExtraInfo(inData))
		return -4;

	if (model->importFromText(inFile, inData))
	{
		if (!importFromModel(model))
			return -4;
	}
}

bool TPAppControl::importFromModel(TPListModel* model)
{
	mb_importMode = true;
	bool bOK(true);
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			updateExercisesList(static_cast<DBExercisesModel*>(model));
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(static_cast<DBMesocyclesModel*>(model)))
			{
				const uint meso_idx = createNewMesocycle(false);
				for (uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_TOTAL_COLS; ++i)
					appMesoModel()->setFast(meso_idx, i, model->getFast(0, i));
				saveMesocycle(meso_idx);
				emit appMesoModel()->currentRowChanged(); //notify main.qml::btnWorkout to evaluate its enabled state
			}
			else
				bOK = false;
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* splitModel = static_cast<DBMesoSplitModel*>(model);
			const uint meso_idx = splitModel->mesoIdx();
			QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
			if (splitModel->completeSplit())
			{
				DBMesoSplitModel* mesoSplitModel(itemMngr->getSplitModel(splitModel->splitLetter().at(0)));
				if (mesoSplitModel->updateFromModel(splitModel))
				{
					saveMesoSplitComplete(mesoSplitModel);
					// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
					// by the time mb_splitsLoaded is ever checked upon
					mb_splitsLoaded = true;
				}
				else
					bOK = false;
			}
			else
			{
				for (uint i(0); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
					appMesoModel()->mesoSplitModel()->setFast(meso_idx, i, splitModel->getFast(0, i));
				appMesoModel()->mesoSplitModel()->setFast(meso_idx, 1, appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
				saveMesoSplit(meso_idx);
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			const uint meso_idx = static_cast<DBTrainingDayModel*>(model)->mesoIdx();
			DBTrainingDayModel* tDayModel(m_itemManager.at(meso_idx)->gettDayModel(dayDate));
			if (tDayModel->updateFromModel(model))
				getTrainingDay(meso_idx, dayDate);
			else
				bOK = false;
		}
		break;
	}
	mb_importMode = false;
	return bOK;
}

void TPAppControl::populateSettingsWithDefaultValue()
{
	if (appSettings()->value("appVersion").toString().isEmpty())
	{
		appSettings()->setValue("appVersion", TP_APP_VERSION);
		appSettings()->setValue("weightUnit", u"(kg)"_qs);
		appSettings()->setValue("themeStyle", u"Material"_qs);
		appSettings()->setValue("colorScheme", u"Blue"_qs);
		appSettings()->setValue("primaryDarkColor", u"#1976D2"_qs);
		appSettings()->setValue("primaryColor", u"#25b5f3"_qs);
		appSettings()->setValue("primaryLightColor", u"#BBDEFB"_qs);
		appSettings()->setValue("paneBackgroundColor", u"#1976d2"_qs);
		appSettings()->setValue("entrySelectedColor", u"#6495ed"_qs);
		appSettings()->setValue("exercisesListVersion", u"0"_qs);
		appSettings()->setValue("backupFolder", u""_qs);
		appSettings()->setValue("fontColor", u"white"_qs);
		appSettings()->setValue("disabledFontColor", u"lightgray"_qs);
		appSettings()->setValue("iconFolder", u"white/"_qs);
		appSettings()->setValue("fontSize", FONT_POINT_SIZE);
		appSettings()->setValue("fontSizeLists", FONT_POINT_SIZE_LISTS);
		appSettings()->setValue("fontSizeText", FONT_POINT_SIZE_TEXT);
		appSettings()->setValue("fontSizeTitle", FONT_POINT_SIZE_TITLE);
		appSettings()->setValue("lastViewedOwnMesoIdx", -1);
		appSettings()->setValue("lastViewedOtherMesoIdx", -1);
		appSettings()->setValue("alwaysAskConfirmation", true);
		appSettings()->sync();
	}
}

void TPAppControl::createItemManager()
{
	const uint n_mesos(appMesoModel()->count());
	m_itemManager.reserve(n_mesos);
	for (uint i(0); i < n_mesos; ++i)
		m_itemManager.append(new QmlItemManager{i});
}
