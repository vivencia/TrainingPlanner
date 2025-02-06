#include "qmlitemmanager.h"

#include "dbinterface.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbusermodel.h"

#include "qmlexerciseentry.h"
#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmlsetentry.h"
#include "qmltdayinterface.h"
#include "qmluserinterface.h"

#include "pageslistmodel.h"
#include "osinterface.h"
#include "statistics/tpstatistics.h"
#include "tpglobals.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "tptimer.h"
#include "tpworkoutscalendar.h"
#include "translationclass.h"

#include "online_services/tponlineservices.h"
#include "weather/weatherinfo.h"

#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>

QmlItemManager* QmlItemManager::_appItemManager(nullptr);
QQmlApplicationEngine* QmlItemManager::_appQmlEngine(nullptr);
QQuickWindow* QmlItemManager::_appMainWindow(nullptr);

QmlItemManager::QmlItemManager(QQmlApplicationEngine* qml_engine)
		: QObject{nullptr}, m_usersManager(nullptr), m_exercisesListManager(nullptr),
			m_weatherPage(nullptr), m_statisticsPage(nullptr), m_allWorkoutsPage(nullptr)
{
	_appItemManager = this;
	appDBInterface()->init();
	_appQmlEngine = qml_engine;
	configureQmlEngine();
}

QmlItemManager::~QmlItemManager()
{
	if (m_weatherPage)
	{
		delete m_weatherPage;
		delete m_weatherComponent;
	}
	if (m_statisticsPage)
	{
		delete m_statisticsPage;
		delete m_statisticsComponent;
	}
	if (m_allWorkoutsPage)
	{
		delete m_allWorkoutsPage;
		delete m_allWorkoutsComponent;
		delete m_wokoutsCalendar;
	}
}

void QmlItemManager::configureQmlEngine()
{
	QQuickStyle::setStyle(appSettings()->themeStyle());

	qmlRegisterType<DBUserModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBUserModel");
	qmlRegisterType<DBExercisesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBTrainingDayModel");
	qmlRegisterType<TPTimer>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPTimer");
	qmlRegisterType<TPImage>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPImage");
	qmlRegisterType<QmlUserInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "UserManager");
	qmlRegisterType<QmlExercisesDatabaseInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExercisesListManager");
	qmlRegisterType<QMLMesoInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "MesoManager");
	qmlRegisterType<QmlMesoCalendarInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "CalendarManager");
	qmlRegisterType<QmlMesoSplitInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SplitManager");
	qmlRegisterType<QmlTDayInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TDayManager");
	qmlRegisterType<QmlExerciseEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExerciseEntryManager");
	qmlRegisterType<QmlSetEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SetEntryManager");
	qmlRegisterType<WeatherInfo>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WeatherInfo");
	qmlRegisterType<TPStatistics>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "Statistics");
	qmlRegisterType<PagesListModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "PagesListModel");
	qmlRegisterType<TPWorkoutsCalendar>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WorkoutsCalendar");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties(9);
	properties[0] = std::move(QQmlContext::PropertyPair{ "appSettings"_L1, QVariant::fromValue(appSettings()) });
	properties[1] = std::move(QQmlContext::PropertyPair{ "appUtils"_L1, QVariant::fromValue(appUtils()) });
	properties[2] = std::move(QQmlContext::PropertyPair{ "appTr"_L1, QVariant::fromValue(appTr()) });
	properties[3] = std::move(QQmlContext::PropertyPair{ "userModel"_L1, QVariant::fromValue(appUserModel()) });
	properties[4] = std::move(QQmlContext::PropertyPair{ "mesocyclesModel"_L1, QVariant::fromValue(appMesoModel()) });
	properties[5] = std::move(QQmlContext::PropertyPair{ "exercisesModel"_L1, QVariant::fromValue(appExercisesModel()) });
	properties[6] = std::move(QQmlContext::PropertyPair{ "itemManager"_L1, QVariant::fromValue(this) });
	properties[7] = std::move(QQmlContext::PropertyPair{ "appStatistics"_L1, QVariant::fromValue(appStatistics()) });
	properties[8] = std::move(QQmlContext::PropertyPair{ "pagesListModel"_L1, QVariant::fromValue(m_pagesManager = new PagesListModel{this}) });
	appQmlEngine()->rootContext()->setContextProperties(properties);

	const QUrl &url{"qrc:/qml/main.qml"_L1};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (const QObject* const obj, const QUrl& objUrl) {
		if (!obj && url == objUrl)
		{
			LOG_MESSAGE("*******************Mainwindow not loaded*******************")
			QCoreApplication::exit(-1);
		}
	});
	appQmlEngine()->addImportPath(":/"_L1);
	appQmlEngine()->addImageProvider("tpimageprovider"_L1, new TPImageProvider{});
	appQmlEngine()->load(url);

	_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
	connect(appMainWindow(), SIGNAL(openFileChosen(QString,int)), this, SLOT(importSlot_FileChosen(QString,int)));
	connect(appMainWindow(), SIGNAL(openFileRejected(QString)), this, SLOT(importSlot_FileChosen(QString)));
	appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));

	if (!appSettings()->mainUserConfigured())
	{
		QMetaObject::invokeMethod(appMainWindow(), "showFirstUseTimeDialog");
		connect(appUserModel(), &DBUserModel::mainUserConfigurationFinishedSignal, this, [this] () {
			disconnect(appUserModel(), nullptr, this, nullptr);
			appSettings()->setMainUserConfigured(true);
			appOsInterface()->initialCheck();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint) {
			appDBInterface()->saveUser(user_row);
		});
	}
	else
		appOsInterface()->initialCheck();

	connect(appMainWindow(), SIGNAL(saveFileChosen(QString)), this, SLOT(exportSlot(QString)));
	connect(appMainWindow(), SIGNAL(saveFileRejected(QString)), this, SLOT(exportSlot(QString)));
}

void QmlItemManager::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}

void QmlItemManager::chooseFileToImport()
{
	QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_ANY));
}

void QmlItemManager::tryToImport(const QList<bool>& selectedFields)
{
	uint wanted_content{0};
	if (isBitSet(m_fileContents, IFC_MESO))
	{
		setBit(wanted_content, isBitSet(m_fileContents, IFC_USER) && selectedFields.at(1) ? IFC_USER : 0);
		setBit(wanted_content, selectedFields.at(0) ? IFC_MESO : 0);
	}
	if (isBitSet(m_fileContents, IFC_MESOSPLIT))
	{
		const int fieldStart{isBitSet(m_fileContents, IFC_MESO) ? 2 : 0};
		if (isBitSet(m_fileContents, IFC_MESO) && !isBitSet(wanted_content, IFC_MESO))
		{
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(this, &QmlItemManager::mesoForImportSelected, this, [this,conn,selectedFields] () {
				disconnect(*conn);
				tryToImport(selectedFields);
			});
			selectWhichMesoToImportInto();
			return;
		}
		const uint wanted_content_temp{wanted_content};
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_A))
			setBit(wanted_content, (selectedFields.at(fieldStart) ? IFC_MESOSPLIT_A : 0));
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_B))
			setBit(wanted_content, (selectedFields.at(fieldStart+1) ? IFC_MESOSPLIT_B : 0));
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_C))
			setBit(wanted_content, (selectedFields.at(fieldStart+2) ? IFC_MESOSPLIT_C : 0));
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_D))
			setBit(wanted_content, (selectedFields.at(fieldStart+3) ? IFC_MESOSPLIT_D : 0));
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_E))
			setBit(wanted_content, (selectedFields.at(fieldStart+4) ? IFC_MESOSPLIT_E : 0));
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_F))
			setBit(wanted_content, (selectedFields.at(fieldStart+5) ? IFC_MESOSPLIT_F : 0));
		if (wanted_content_temp != wanted_content)
			setBit(wanted_content, IFC_MESOSPLIT);
	}
	else
	{
		setBit(wanted_content, isBitSet(m_fileContents, IFC_TDAY) && selectedFields.at(0) ? IFC_TDAY : 0);
		setBit(wanted_content, isBitSet(m_fileContents, IFC_EXERCISES) && selectedFields.at(0) ? IFC_EXERCISES : 0);
	}
	importFromFile(m_importFilename, wanted_content);
}

void QmlItemManager::displayImportDialogMessageAfterMesoSelection(const int meso_idx)
{
	appMesoModel()->setImportIdx(meso_idx);
	emit mesoForImportSelected();
}

void QmlItemManager::getSettingsPage(const uint startPageIndex)
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this, appQmlEngine(), appMainWindow()};
	m_usersManager->getSettingsPage(startPageIndex);
}

void QmlItemManager::getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this, appQmlEngine(), appMainWindow()};
	m_usersManager->getClientsOrCoachesPage(bManageClients, bManageCoaches);
}

void QmlItemManager::getExercisesPage(QmlTDayInterface *connectPage)
{
	if (!m_exercisesListManager)
		m_exercisesListManager = new QmlExercisesDatabaseInterface{this};
	m_exercisesListManager->getExercisesPage(connectPage);
}

void QmlItemManager::getWeatherPage()
{
	if (!m_weatherPage)
	{
		m_weatherComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/WeatherPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_weatherComponent->status() != QQmlComponent::Ready)
		{
			connect(m_weatherComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createWeatherPage_part2();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createWeatherPage_part2();
	}
	else
		m_pagesManager->addMainMenuShortCut(QString(), m_weatherPage);
}

void QmlItemManager::getStatisticsPage()
{
	if (!m_statisticsPage)
	{
		m_statisticsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/StatisticsPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_statisticsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_statisticsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createStatisticsPage_part2();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createStatisticsPage_part2();
	}
	else
		m_pagesManager->addMainMenuShortCut(QString(), m_statisticsPage);
}

void QmlItemManager::getAllWorkoutsPage()
{
	if (!m_allWorkoutsPage)
	{
		m_allWorkoutsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/AllWorkouts.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_allWorkoutsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_allWorkoutsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createAllWorkoutsPage_part2();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createAllWorkoutsPage_part2();
	}
	else
		m_pagesManager->addMainMenuShortCut(QString(), m_allWorkoutsPage);
}

const QString& QmlItemManager::setExportFileName(const QString& filename)
{
	m_exportFilename = appOsInterface()->appDataFilesPath() + filename;
	return m_exportFilename;
}

void QmlItemManager::continueExport(int exportMessageId, const bool bShare)
{
	const QString& filename{appUtils()->getFileName(m_exportFilename)};
	if (exportMessageId == APPWINDOW_MSG_EXPORT_OK)
	{
		if (bShare)
		{
			appOsInterface()->shareFile(m_exportFilename);
			exportMessageId = APPWINDOW_MSG_SHARE_OK;
		}
		else
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, filename));
	}
	displayMessageOnAppWindow(exportMessageId, filename);
}

void QmlItemManager::displayActivityResultMessage(const int requestCode, const int resultCode) const
{
	int message_id(0);
	switch (resultCode)
	{
		case -1: message_id = APPWINDOW_MSG_SHARE_OK; break;
		case 0: message_id = APPWINDOW_MSG_SHARE_FAILED; break;
		default: message_id = APPWINDOW_MSG_UNKNOWN_ERROR; break;
	}
	displayMessageOnAppWindow(message_id);
	QFile::remove(m_exportFilename);
}

void QmlItemManager::getPasswordDialog(const QString& title, const QString& message) const
{
	connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), this, SLOT(qmlPasswordDialogClosed_slot(int,QString)),
		static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	QMetaObject::invokeMethod(appMainWindow(), "showPasswordDialog", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::selectWhichMesoToImportInto()
{
	QString message;
	if (isBitSet(m_fileContents, IFC_MESOSPLIT))
		message = std::move(tr("You're trying to import an exercises selection plan. Select into which program you'd wish to incorporate it."));
	else if (isBitSet(m_fileContents, IFC_TDAY))
		message = std::move(tr("You're trying to import a one day workout. Select in which program you'd wish to include it."));
	QStringList mesoInfo;
	QList<int> idxsList;
	const QDate& today{QDate::currentDate()};
	for (uint i{0}; i < appMesoModel()->count(); ++i)
	{
		if (appMesoModel()->isDateWithinMeso(i, today))
		{
			mesoInfo.append(std::move(appMesoModel()->name(i) + '\n' + appMesoModel()->columnLabel(MESOCYCLES_COL_CLIENT) + appMesoModel()->client(i)));
			idxsList.append(i);
		}
	}

	QMetaObject::invokeMethod(appMainWindow(), "selectMesoDialog", Q_ARG(QString, message), Q_ARG(QStringList, mesoInfo), Q_ARG(QList<int>, idxsList));
}

void QmlItemManager::displayImportDialogMessage(const uint fileContents, const QString& filename)
{
	m_fileContents = fileContents;
	m_importFilename = filename;

	QStringList importOptions;
	if (isBitSet(m_fileContents, IFC_MESO))
	{
		importOptions.append(std::move(tr("Complete Training Plan")));
		if (isBitSet(m_fileContents, IFC_USER))
			importOptions.append(std::move(tr("Coach information")));
	}
	if (isBitSet(m_fileContents, IFC_MESOSPLIT))
	{
		const bool newMesoImport(appMesoModel()->importIdx() == -1);
		if (newMesoImport && !(isBitSet(m_fileContents, IFC_MESO)))
		{
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(this, &QmlItemManager::mesoForImportSelected, this, [this,conn] () {
				disconnect(*conn);
				displayImportDialogMessage(m_fileContents, m_importFilename);
			});
			selectWhichMesoToImportInto();
			return;
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_A))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('A'))
					importOptions.append(std::move(tr("Exercises Program A")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_A);
			}
			else
				importOptions.append(std::move(tr("Exercises Program A")));
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_B))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('B'))
					importOptions.append(std::move(tr("Exercises Program B")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_B);
			}
			else
				importOptions.append(std::move(tr("Exercises Program B")));
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_C))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('C'))
					importOptions.append(std::move(tr("Exercises Program C")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_C);
			}
			else
				importOptions.append(std::move(tr("Exercises Program C")));
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_D))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('D'))
					importOptions.append(std::move(tr("Exercises Program D")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_D);
			}
			else
				importOptions.append(std::move(tr("Exercises Program D")));
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_E))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('E'))
					importOptions.append(std::move(tr("Exercises Program E")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_E);
			}
			else
				importOptions.append(std::move(tr("Exercises Program E")));
		}
		if (isBitSet(m_fileContents, IFC_MESOSPLIT_F))
		{
			if (!newMesoImport)
			{
				if (appMesoModel()->split(appMesoModel()->importIdx()).contains('F'))
					importOptions.append(std::move(tr("Exercises Program F")));
				else
					unSetBit(m_fileContents, IFC_MESOSPLIT_F);
			}
			else
				importOptions.append(std::move(tr("Exercises Program F")));
		}
	}
	else
	{
		if (isBitSet(m_fileContents, IFC_TDAY))
			importOptions.append(std::move(tr("One Workout")));
		else if (isBitSet(m_fileContents, IFC_EXERCISES))
			importOptions.append(std::move(tr("Exercises database update")));
	}

	const QList<bool> selectedFields(importOptions.count(), true);
	QMetaObject::invokeMethod(appMainWindow(), "createImportConfirmDialog", Q_ARG(QStringList, importOptions), Q_ARG(QList<bool>, selectedFields));
}

void QmlItemManager::openRequestedFile(const QString& filename, const int wanted_content)
{
	#ifdef Q_OS_ANDROID
	const QString& androidFilename{appOsInterface()->readFileFromAndroidFileDialog(filename)};
	if (androidFilename.isEmpty())
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_OPEN_FAILED, filename);
		return;
	}
	QFile* inFile{new QFile{androidFilename}};
	#else
	QFile* inFile{new QFile{filename}};
	#endif
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
				if (strstr(buf, "0x00") != NULL)
				{
					inData = buf;
					if (inData.indexOf(QString::number(USER_TABLE_ID)) != -1)
					{
						if (wanted_content == IFC_ANY || wanted_content == IFC_USER)
							setBit(fileContents, IFC_USER);
					}
					else if (inData.indexOf(QString::number(MESOSPLIT_TABLE_ID)) != -1)
					{
						if (wanted_content == IFC_ANY || wanted_content == IFC_MESOSPLIT)
							setBit(fileContents, IFC_MESOSPLIT);
					}
					else if (inData.indexOf(QString::number(MESOCYCLES_TABLE_ID)) != -1)
					{
						if (wanted_content == IFC_ANY || wanted_content == IFC_MESO)
							setBit(fileContents, IFC_MESO);
					}
					else if (inData.indexOf(QString::number(TRAININGDAY_TABLE_ID)) != -1)
					{
						if (wanted_content == IFC_ANY || wanted_content == IFC_TDAY)
							setBit(fileContents, IFC_TDAY);
					}
					else if (inData.indexOf(QString::number(EXERCISES_TABLE_ID)) != -1)
					{
						if (wanted_content == IFC_ANY || wanted_content == IFC_EXERCISES)
							setBit(fileContents, IFC_EXERCISES);
					}
				}
			}
			else
			{
				if (wanted_content == IFC_ANY || wanted_content == IFC_MESOSPLIT)
				{
					if (isBitSet(fileContents, IFC_MESOSPLIT))
					{
						inData = buf;
						int idx(inData.indexOf(tr("Split: ")));
						if (idx != -1)
						{
							idx += 7;
							switch (inData.sliced(idx+2, 1).at(0).toLatin1())
							{
								case 'A': setBit(fileContents, IFC_MESOSPLIT_A); break;
								case 'B': setBit(fileContents, IFC_MESOSPLIT_B); break;
								case 'C': setBit(fileContents, IFC_MESOSPLIT_C); break;
								case 'D': setBit(fileContents, IFC_MESOSPLIT_D); break;
								case 'E': setBit(fileContents, IFC_MESOSPLIT_E); break;
								case 'F': setBit(fileContents, IFC_MESOSPLIT_F); break;
							}
						}
					}
				}
			}
		}
	}
	inFile->close();
	delete inFile;

	if (fileContents != 0)
	{
		if (wanted_content == IFC_ANY)
			appMesoModel()->setImportIdx(-1);
		#ifdef Q_OS_ANDROID
			displayImportDialogMessage(fileContents, androidFilename);
		#else
			displayImportDialogMessage(fileContents, filename);
		#endif
	}
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE);
}

void QmlItemManager::importFromFile(const QString& filename, const int wanted_content)
{
	int importFileMessageId{APPWINDOW_MSG_IMPORT_FAILED};
	if (isBitSet(wanted_content, IFC_USER))
	{
		DBUserModel* usermodel{new DBUserModel{this, false}};
		usermodel->deleteLater();
		if (usermodel->importFromFile(filename) == APPWINDOW_MSG_READ_FROM_FILE_OK)
			importFileMessageId = incorporateImportedData(usermodel);
	}
	if (isBitSet(wanted_content, IFC_MESO))
	{
		DBMesocyclesModel* mesomodel{new DBMesocyclesModel{this, false}};
		mesomodel->deleteLater();
		if (mesomodel->importFromFile(filename) == APPWINDOW_MSG_READ_FROM_FILE_OK)
			importFileMessageId = incorporateImportedData(mesomodel, wanted_content);
	}
	if (isBitSet(wanted_content, IFC_MESOSPLIT))
	{
		DBMesoSplitModel* splitModels[6];
		for (uint i(0), ifc(IFC_MESOSPLIT_A); i < 6; ++i, ++ifc)
		{
			if (isBitSet(wanted_content, ifc))
			{
				splitModels[i] = new DBMesoSplitModel{this, true};
				splitModels[i]->setImportMode(true);
				splitModels[i]->setSplitLetter(static_cast<char>('A' + i));
				splitModels[i]->setMesoIdx(appMesoModel()->importIdx());
				if (splitModels[i]->importFromFile(filename) == APPWINDOW_MSG_READ_FROM_FILE_OK)
					importFileMessageId = incorporateImportedData(splitModels[i], wanted_content);
			}
		}
	}
	else if (isBitSet(wanted_content, IFC_TDAY))
	{
		DBTrainingDayModel* tDayModel{new DBTrainingDayModel{this}};
		tDayModel->setImportMode(true);
		if (tDayModel->importFromFile(filename) == APPWINDOW_MSG_READ_FROM_FILE_OK)
			importFileMessageId = incorporateImportedData(tDayModel);
	}
	else if (isBitSet(wanted_content, IFC_EXERCISES))
	{
		DBExercisesModel* exercisesModel{new DBExercisesModel{this, false}};
		exercisesModel->deleteLater();
		if (exercisesModel->importFromFile(filename) == APPWINDOW_MSG_READ_FROM_FILE_OK)
			importFileMessageId = incorporateImportedData(exercisesModel);
	}

	displayMessageOnAppWindow(importFileMessageId, filename);
}

int QmlItemManager::incorporateImportedData(TPListModel* model, const int wanted_content)
{
	bool ok(false);
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			if ((ok = appExercisesModel()->updateFromModel(model)))
				appDBInterface()->saveExercises();
		break;
		case USER_TABLE_ID:
			if (appUserModel()->isDifferent(model))
			{
				if ((ok = appUserModel()->updateFromModel(model)))
					appDBInterface()->saveUser(appUserModel()->count()-1);
			}
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(model))
			{
				const uint meso_idx{appMesoModel()->createNewMesocycle(false)};
				if ((ok = appMesoModel()->updateFromModel(meso_idx, model)))
				{
					//If we are importing a complete program with splits as well, let them save split code to the database. The code to save a
					//a simple split can interfere with the multiple split threads
					if (!isBitSet(wanted_content, IFC_MESOSPLIT))
						appMesoModel()->setImportMode(false);
					appDBInterface()->saveMesocycle(meso_idx);
				}
			}
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* newSplitModel{static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model))};
			DBMesoSplitModel* splitModel{appMesoModel()->mesoManager(appMesoModel()->currentMesoIdx())->plannerSplitModel(newSplitModel->_splitLetter())};
			if (splitModel) //exercises planner page for the current meso has been loaded in the session
			{
				if ((ok = splitModel->updateFromModel(newSplitModel)))
					appDBInterface()->saveMesoSplitComplete(splitModel);
			}
			else //exercises planner page for the current meso has NOT -yet- been loaded during the session
			{
				appMesoModel()->setMuscularGroup(newSplitModel->mesoIdx(), newSplitModel->_splitLetter(), newSplitModel->muscularGroup(), false);
				appDBInterface()->saveMesoSplitComplete(newSplitModel);
				ok = true;
			}
			//If we imported a complete plan we could not change the import mode before because saving a simple split is disabled when importing
			//Now it is safe(and necessary) to set the correct import state to the main model
			if (isBitSet(wanted_content, IFC_MESO))
				appMesoModel()->setImportMode(false);
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			DBTrainingDayModel* newTDayModel{static_cast<DBTrainingDayModel*>(const_cast<TPListModel*>(model))};
			DBTrainingDayModel* tDayModel{appMesoModel()->mesoManager(appMesoModel()->currentMesoIdx())->tDayModelForToday()};
			if (tDayModel)
			{
				if ((ok = tDayModel->updateFromModel(newTDayModel)))
					appDBInterface()->saveTrainingDay(tDayModel);
			}
			else
			{
				appDBInterface()->saveTrainingDay(newTDayModel);
				ok = true;
			}
		}
		break;
	}
	return ok ? APPWINDOW_MSG_IMPORT_OK : APPWINDOW_MSG_IMPORT_FAILED_SAME_DATA;
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::mainWindowStarted() const
{
	appOsInterface()->initialCheck();
}

void QmlItemManager::displayMessageOnAppWindow(const int message_id, const QString& fileName) const
{
	QString title, message;
	switch (message_id)
	{
		case APPWINDOW_MSG_CUSTOM_MESSAGE:
		{
			const qsizetype sep_idx{fileName.lastIndexOf(record_separator)};
			title = sep_idx >= 1 ? fileName.left(sep_idx) : std::move(tr("Message"));
			message = sep_idx >= 1 ? fileName.right(fileName.length() - sep_idx - 1) : fileName;
		}
		break;
		case APPWINDOW_MSG_CUSTOM_WARNING:
			title = std::move(tr("WARNING!"));
			message = fileName;
		break;
		case APPWINDOW_MSG_CUSTOM_ERROR:
			title = std::move(tr("ERROR!"));
			message = fileName;
		break;
		case APPWINDOW_MSG_EXPORT_OK:
			title = std::move(tr("Succesfully exported"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_SHARE_OK:
			title = std::move(tr("Succesfully shared"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_IMPORT_OK:
			title = std::move(tr("Successfully imported"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_OPEN_FAILED:
			title = std::move(tr("Failed to open file"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_UNKNOWN_FILE_FORMAT:
			title = std::move(tr("Error"));
			message = std::move(tr("File type not recognized"));
		break;
		case APPWINDOW_MSG_CORRUPT_FILE:
			title = std::move(tr("Error"));
			message = std::move(tr("File is formatted wrongly or is corrupted"));
		break;
		case APPWINDOW_MSG_NOTHING_TODO:
			title = std::move(tr("Nothing to be done"));
			message = std::move(tr("File had already been imported"));
		break;
		case APPWINDOW_MSG_NO_MESO:
			title = std::move(tr("No program to import into"));
			message = std::move(tr("Either create a new training plan or import from a complete program file"));
		break;
		case APPWINDOW_MSG_NOTHING_TO_EXPORT:
			title = std::move(tr("Nothing to export"));
			message = std::move(tr("Only exercises that do not come by default with the app can be exported"));
		break;
		case APPWINDOW_MSG_SHARE_FAILED:
			title = std::move(tr("Sharing failed"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_EXPORT_FAILED:
			title = std::move(tr("Export failed"));
			message = std::move(tr("Operation canceled"));
		break;
		case APPWINDOW_MSG_IMPORT_FAILED_SAME_DATA:
			title = std::move(tr("Import failed"));
			message = std::move(tr("The file does not contain any new data that is not already in use"));
		break;
		case APPWINDOW_MSG_IMPORT_CANCELED:
			title = std::move(tr("Import failed"));
			message = std::move(tr("Operation canceled"));
		break;
		case APPWINDOW_MSG_IMPORT_FAILED:
			title = std::move(tr("Import from file failed"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED:
			title = std::move(tr("Could not open file for exporting"));
			message = std::move(appUtils()->getFileName(fileName));
		break;
		case APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE:
			title = std::move(tr("Cannot import"));
			message = std::move(tr("Contents of the file are incompatible with the requested operation"));
		break;
		case APPWINDOW_MSG_UNKNOWN_ERROR:
			title = std::move(tr("Error"));
			message = std::move(tr("Something went wrong"));
		break;
	}
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::exportSlot(const QString& filePath)
{
	int messageId(APPWINDOW_MSG_EXPORT_FAILED);
	if (!filePath.isEmpty())
	{
		messageId = APPWINDOW_MSG_EXPORT_OK;
		QFile file{filePath};
		if (file.exists())
			messageId = file.remove() ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_EXPORT_FAILED;
		if (messageId == APPWINDOW_MSG_EXPORT_OK)
			if (!QFile::copy(m_exportFilename, filePath))
				messageId = APPWINDOW_MSG_EXPORT_FAILED;
	}
	displayMessageOnAppWindow(messageId);
	QFile::remove(m_exportFilename);
	m_exportFilename.clear();
}

void QmlItemManager::importSlot_FileChosen(const QString& filePath, const int fileType)
{
	if (!filePath.isEmpty())
		openRequestedFile(filePath, static_cast<importFileContents>(fileType));
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_CANCELED);
}

void QmlItemManager::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	m_pagesManager->addMainMenuShortCut(label, page);
}

void QmlItemManager::removeMainMenuShortCut(QQuickItem* page)
{
	m_pagesManager->removeMainMenuShortCut(page);
}

void QmlItemManager::createWeatherPage_part2()
{
	m_weatherPage = static_cast<QQuickItem*>(m_weatherComponent->create(appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (m_weatherComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_weatherComponent->errorString();
		for (uint i(0); i < m_weatherComponent->errors().count(); ++i)
			qDebug() << m_weatherComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_weatherPage, QQmlEngine::CppOwnership);
	m_weatherPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_pagesManager->addMainMenuShortCut(QString(), m_weatherPage);
}

void QmlItemManager::createStatisticsPage_part2()
{
	m_statisticsPage = static_cast<QQuickItem*>(m_statisticsComponent->create(appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (m_statisticsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_statisticsComponent->errorString();
		for (uint i(0); i < m_statisticsComponent->errors().count(); ++i)
			qDebug() << m_statisticsComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_statisticsPage, QQmlEngine::CppOwnership);
	m_statisticsPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_pagesManager->addMainMenuShortCut(QString(), m_statisticsPage);
}

void QmlItemManager::createAllWorkoutsPage_part2()
{
	m_wokoutsCalendar = new TPWorkoutsCalendar{this};
	QVariantMap allWorkoutsProperties;
	allWorkoutsProperties.insert("workoutsCalendar", QVariant::fromValue(m_wokoutsCalendar));
	m_allWorkoutsPage = static_cast<QQuickItem*>(m_allWorkoutsComponent->createWithInitialProperties(allWorkoutsProperties, appQmlEngine()->rootContext()));

	#ifndef QT_NO_DEBUG
	if (m_allWorkoutsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_allWorkoutsComponent->errorString();
		for (uint i(0); i < m_allWorkoutsComponent->errors().count(); ++i)
			qDebug() << m_allWorkoutsComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_allWorkoutsPage, QQmlEngine::CppOwnership);
	m_allWorkoutsPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_pagesManager->addMainMenuShortCut(QString(), m_allWorkoutsPage);

	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);
	connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);

	m_wokoutsCalendar->scanMesocycles();
}
