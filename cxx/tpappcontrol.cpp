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
TPUtils* TPAppControl::app_utils(nullptr);
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
		TPAppControl::app_os_interface = new OsInterface{};
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
	app_utils = &_tPUtils;
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

	if (bCreatePage)
	{
		QmlItemManager* itemMngr{new QmlItemManager{meso_idx}};
		m_itemManager.append(itemMngr);
		itemMngr->createMesocyclePage(minimumStartDate, appUtils()->createFutureDate(startDate,0,6,0));
	}
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

/*Return values
 *	 0: success
 *	-1: Failed to open file
 *	-2: File format was not recognized
 *	-3: Nothing was imported, either because file was missing info or error in formatting
 */
int DbManager::importFromFile(QString filename, QFile* inFile)
{
	if (!inFile)
	{
		if (filename.startsWith(u"file:"_qs))
			filename.remove(0, 7); //remove file://
		inFile = new QFile(filename, this);
		inFile->deleteLater();
		if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
			return -1;
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
		else if (inData.indexOf(DBTrainingDayObjectName) != -1)
			model = new DBTrainingDayModel(this);
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
	else
	{
		inData = inData.left(2);
		inData.chop(1);
		switch (inData.toUInt())
		{
			case EXERCISES_TABLE_ID: model = new DBExercisesModel; break;
			case MESOCYCLES_TABLE_ID: model = new DBMesocyclesModel; break;
			case MESOSPLIT_TABLE_ID: model = new DBMesoSplitModel; break;
			case TRAININGDAY_TABLE_ID: model = new DBTrainingDayModel; break;
			default:
				return -2;
		}
	}

	model->deleteLater();
	if (!model->importExtraInfo(inData))
		return -4;

	bool bDataImportSuccessfull(false);
	if (bFancy)
		bDataImportSuccessfull = model->importFromFancyText(inFile, inData);
	else
	{
		const QString data(inFile->readAll());
		if (!data.isEmpty())
		{
			sep_idx = data.indexOf(u"##0x"_qs);
			if (sep_idx == -1)
				bDataImportSuccessfull = model->importFromText(data);
			else
			{
				inFile->seek(sep_idx);
				bDataImportSuccessfull = model->importFromText(data.left(sep_idx));
			}
		}
	}

	if (bDataImportSuccessfull)
		importFromModel(model);

	if (!inFile->atEnd())
		return importFromFile(filename, inFile);
	else
		return 0;
}

void DbManager::importFromModel(TPListModel* model)
{
	mb_importMode = true;
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			updateExercisesList(static_cast<DBExercisesModel*>(model));
		break;
		case MESOCYCLES_TABLE_ID:
			createNewMesocycle(model->getFast(0, MESOCYCLES_COL_REALMESO) == u"1"_qs, model->getFast(0, 1), false);
			saveMesocycle(true, model->getFast(0, MESOCYCLES_COL_NAME), model->getDate(0, MESOCYCLES_COL_STARTDATE),
								model->getDate(0, MESOCYCLES_COL_ENDDATE), model->getFast(0, MESOCYCLES_COL_NOTE),
								model->getFast(0, MESOCYCLES_COL_WEEKS), model->getFast(0, MESOCYCLES_COL_SPLIT),
								model->getFast(0, MESOCYCLES_COL_DRUGS), QString(), QString(), QString(),
							QString(), QString(), QString(), false, false, false);
		break;
		case MESOSPLIT_TABLE_ID:
		{
			if (static_cast<DBMesoSplitModel*>(model)->completeSplit())
			{
				DBMesoSplitModel* splitModel(m_currentMesoManager->getSplitModel(static_cast<DBMesoSplitModel*>(model)->splitLetter().at(0)));
				splitModel->updateFromModel(model);
				updateMesoSplitComplete(splitModel);
				// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
				// by the time mb_splitsLoaded is ever checked upon
				mb_splitsLoaded = true;
			}
			else
			{
				updateMesoSplit(model->getFast(0, 0), model->getFast(0, 1), model->getFast(0, 2), model->getFast(0, 3),
								model->getFast(0, 4), model->getFast(0, 5));
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			DBTrainingDayModel* tDayModel(m_currentMesoManager->gettDayModel(dayDate));
			tDayModel->updateFromModel(model);
			if (mesoCalendarModel->count() == 0)
			{
				connect( this, &DbManager::databaseReady, this, [&,dayDate] {
					connect( this, &DbManager::getPage, this, [&] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(dayDate), item);
							}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					return getTrainingDay(dayDate); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				getMesoCalendar(false);
			}
			else
			{
				connect( this, &DbManager::getPage, this, [&] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(dayDate), item);
							}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				getTrainingDay(dayDate);
			}
		}
		break;
	}
	mb_importMode = false;
}

void DbManager::saveFileDialogClosed(QString finalFileName, bool bResultOK)
{
	int resultCode(-5);
	if (finalFileName.startsWith(u"file:"_qs))
		finalFileName.remove(0, 7); //remove file://
	if (bResultOK)
	{
		bResultOK = QFile::copy(exportFileName(), finalFileName);
		resultCode = bResultOK ? 3 : -10;
	}
	QFile::remove(exportFileName());
	m_mainWindow->setProperty("importExportFilename", finalFileName);
	QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, resultCode));
}

int DbManager::parseFile(QString filename)
{
	if (filename.startsWith(u"file:"_qs))
		filename.remove(0, 7); //remove file://
	QFile* inFile(new QFile(filename, this));
	inFile->deleteLater();
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
		return -1;

	qint64 lineLength(0);
	char buf[128];
	QString inData;
	QString tableMessage;
	bool createMessage[4] = { false };

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (!inData.startsWith(u"##0x"_qs)) //Fancy
				{
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
					{
						if (createMessage[1] && createMessage[0])
							continue;
						createMessage[0] = true;
					}
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						createMessage[1] = true;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						createMessage[2] = true;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						createMessage[3] = true;
					else
						return -2;
				}
				else
				{
					inData = inData.left(2);
					inData.chop(1);
					switch (inData.toUInt())
					{
						case EXERCISES_TABLE_ID: createMessage[3] = true; break;
						case MESOCYCLES_TABLE_ID: createMessage[1] = true; break;
						case MESOSPLIT_TABLE_ID:
							if (createMessage[1] && createMessage[0])
								continue;
							createMessage[0] = true;
						break;
						case TRAININGDAY_TABLE_ID: createMessage[3] = true; break;
						default: return -2;
					}
				}
				if (createMessage[0])
				{
					if (tableMessage.isEmpty())
						tableMessage = tr("a new Training Split Exercise Plan");
					else
					{
						if (createMessage[1])
							tableMessage += tr("new Training Split Exercise Plans");
						else
							tableMessage = tr("new Training Split Exercise Plans");
					}
				}
				else if (createMessage[1])
					tableMessage = tr("an entire Mesocycle Plan, including ");
				else if (createMessage[2])
					tableMessage = tr("One Training Day");
				else if (createMessage[3])
				{
					if (!createMessage[1])
						tableMessage = tr("An updated exercises database list");
					else
						tableMessage.append(tr("and an updated exercises database list"));
				}
			}
		}
	}

	if (createMessage[0] || createMessage[1] || createMessage[2] || createMessage[3])
	{
		const QString message(tr("This will import data to create: %1"));
		QMetaObject::invokeMethod(m_mainWindow, "confirmImport", Q_ARG(QString, message.arg(tableMessage)));
		return 1;
	}
	else
		return -3;
}

void TPAppControl::exportMeso(const bool bShare, const bool bFancy)
{
	if (!mb_splitsLoaded)
		loadCompleteMesoSplits(false);
	const QString suggestedName(mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_NAME) + tr(" - TP Complete Meso.txt"));
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	mesocyclesModel->setExportRow(m_MesoIdx);

	if (exportToFile(mesocyclesModel, exportFileName(), bFancy, outFile))
	{
		mesoSplitModel->setExportRow(m_MesoIdx);
		exportToFile(mesoSplitModel, QString(), bFancy, outFile);
		exportMesoSplit(u"X"_qs, bShare, bFancy, outFile);

		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		m_mainWindow->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -10));
	}
}
void TPAppControl::exportMesoSplit(const QString& splitLetter, const bool bShare, QFile* outFileInUse)
{
	QString mesoSplit;
	QString suggestedName;
	QFile* outFile(nullptr);
	const uint meso_idx(itemMngr->mesoIdx());

	if (!outFileInUse)
	{
		if (splitLetter == u"X"_qs)
		{
			mesoSplit = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
			suggestedName = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_NAME) + tr(" - Exercises Plan.txt");
		}
		else
		{
			mesoSplit = splitLetter;
			suggestedName = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_NAME) +
								tr(" - Exercises Plan - Split ") + splitLetter + u".txt"_qs;
		}
		//setExportFileName(suggestedName);
	}
	else
	{
		outFile = outFileInUse;
		mesoSplit = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
	}

	QString mesoLetters;
	bool bExportToFileOk(true);

	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());
	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;
		mesoLetters.append(static_cast<QChar>(*itr));
		//bExportToFileOk &= exportToFile(const_cast<QmlItemManager*>(itemMngr)->getSplitModel(static_cast<QChar>(*itr)),
		//									exportFileName(), outFile);
	} while (++itr != itr_end);

	if (outFileInUse)
		return;

	if (bExportToFileOk)
	{
		#ifdef Q_OS_ANDROID
		/*setExportFileName("app_logo.png");
		if (!QFile::exists(exportFileName()))
		{
			QFile::copy(":/images/app_logo.png", exportFileName());
			QFile::setPermissions(exportFileName(), QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
		}
		sendFile(exportFileName(), tr("Send file"), u"image/png"_qs, 10);*/
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
	}
}

void TPAppControl::exportTrainingDay(const DBTrainingDayModel* tDayModel, const bool bShare)
{
	const QString suggestedName(tr(" - Workout ") + tDayModel->splitLetter() + u".txt"_qs);
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	if (exportToFile(tDayModel, exportFileName(), outFile))
	{
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
	}
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
