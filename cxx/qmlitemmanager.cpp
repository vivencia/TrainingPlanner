#include "qmlitemmanager.h"

#include "dbcalendarmodel.h"
#include "dbinterface.h"
#include "dbexercisesmodel.h"
#include "dbexerciseslistmodel.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "homepagemesomodel.h"

#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmlworkoutinterface.h"
#include "qmluserinterface.h"

#include "pageslistmodel.h"
#include "osinterface.h"
#include "statistics/tpstatistics.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "tptimer.h"
#include "tpworkoutscalendar.h"
#include "translationclass.h"

#include "online_services/onlineuserinfo.h"
#include "online_services/tpmessagesmanager.h"
#include "online_services/tponlineservices.h"

#include "weather/weatherinfo.h"

#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>

QmlItemManager *QmlItemManager::_appItemManager{nullptr};
QQmlApplicationEngine *QmlItemManager::_appQmlEngine{nullptr};
QQuickWindow *QmlItemManager::_appMainWindow{nullptr};

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
	new PagesListModel{this};

	qmlRegisterType<DBUserModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBUserModel");
	qmlRegisterType<DBExercisesListModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBExercisesListModel");
	qmlRegisterType<DBMesocyclesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBExercisesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesoCalendarManager>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesoCalendarManager");
	qmlRegisterType<DBCalendarModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBCalendarModel");
	qmlRegisterType<TPTimer>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPTimer");
	qmlRegisterType<TPImage>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPImage");
	qmlRegisterType<QmlUserInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "UserManager");
	qmlRegisterType<QmlExercisesDatabaseInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExercisesListManager");
	qmlRegisterType<QMLMesoInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "MesoManager");
	qmlRegisterType<QmlMesoCalendarInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "CalendarManager");
	qmlRegisterType<QmlMesoSplitInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SplitManager");
	qmlRegisterType<QmlWorkoutInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WorkoutManager");
	qmlRegisterType<WeatherInfo>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WeatherInfo");
	qmlRegisterType<TPStatistics>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "Statistics");
	qmlRegisterType<PagesListModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "PagesListModel");
	qmlRegisterType<TPWorkoutsCalendar>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WorkoutsCalendar");
	qmlRegisterType<OnlineUserInfo>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "OnlineUserInfo");
	qmlRegisterType<TPMessagesManager>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "MessagesManager");
	qmlRegisterType<HomePageMesoModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "HomePageMesoModel");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties{9};
	properties[0] = std::move(QQmlContext::PropertyPair{ "appSettings"_L1, QVariant::fromValue(appSettings()) });
	properties[1] = std::move(QQmlContext::PropertyPair{ "appUtils"_L1, QVariant::fromValue(appUtils()) });
	properties[2] = std::move(QQmlContext::PropertyPair{ "appTr"_L1, QVariant::fromValue(appTr()) });
	properties[3] = std::move(QQmlContext::PropertyPair{ "userModel"_L1, QVariant::fromValue(appUserModel()) });
	properties[4] = std::move(QQmlContext::PropertyPair{ "exercisesModel"_L1, QVariant::fromValue(appExercisesList()) });
	properties[5] = std::move(QQmlContext::PropertyPair{ "itemManager"_L1, QVariant::fromValue(this) });
	properties[6] = std::move(QQmlContext::PropertyPair{ "appStatistics"_L1, QVariant::fromValue(appStatistics()) });
	properties[7] = std::move(QQmlContext::PropertyPair{ "appMessages"_L1, QVariant::fromValue(new TPMessagesManager{this}) });
	properties[8] = std::move(QQmlContext::PropertyPair{ "osInfo"_L1, QVariant::fromValue(appOsInterface()) });
	appQmlEngine()->rootContext()->setContextProperties(properties);
	appQmlEngine()->addImportPath(":/"_L1);
	appQmlEngine()->addImageProvider("tpimageprovider"_L1, new TPImageProvider{});

	QUrl url{};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [this] (const QObject *const obj, const QUrl &objUrl)
	{
		if (!obj)
		{
			#ifndef QT_NO_DEBUG
			qDebug () << "*******************Mainwindow not loaded*******************";
			#endif
			QCoreApplication::exit(-1);
		}
		else
		{
			_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
			appMainWindow()->setIcon(QIcon{"qrc:/images/app_icon.png"_L1});
			appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));

			QQuickItem *homePage{appMainWindow()->findChild<QQuickItem*>("homePage")};
			if (homePage)
				appPagesListModel()->insertHomePage(homePage);
			if (obj->objectName() == "mainWindow"_L1)
			{
				if (!appUserModel()->mainUserConfigured())
					appUserModel()->showFirstTimeUseDialog();
				else
					appOsInterface()->initialCheck();

				connect(appMainWindow(), SIGNAL(openFileChosen(QString,int)), this, SLOT(importSlot_FileChosen(QString,int)));
				connect(appMainWindow(), SIGNAL(openFileRejected(QString)), this, SLOT(importSlot_FileChosen(QString)));
				connect(appMainWindow(), SIGNAL(saveFileChosen(QString)), this, SLOT(exportSlot(QString)));
				connect(appMainWindow(), SIGNAL(saveFileRejected(QString)), this, SLOT(exportSlot(QString)));
			}
		}
	});

#ifndef Q_OS_ANDROID
	connect(appUserModel(), &DBUserModel::userIdChanged, this, [this] () {
		emit userChangedSignal();
	});
	#ifndef QT_NO_DEBUG
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1)
	{
		if (args.at(1) == "-test"_L1)
			url = std::move("qrc:/qml/tests.qml"_L1);
	}
	if (url.isEmpty())
		url = std::move("qrc:/qml/main.qml"_L1);
	#endif
#else
	url = std::move("qrc:/qml/main.qml"_L1);
#endif

	appQmlEngine()->load(url);
}

void QmlItemManager::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}

void QmlItemManager::chooseFileToImport()
{
	QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_MESO));
}

void QmlItemManager::importFromSelectedFile(const QList<bool> &selectedFields)
{
	const bool formatted{m_importFilename.at(0).cell() == 'f'};
	int importFileMessageId{APPWINDOW_MSG_IMPORT_FAILED};

	m_importFilename.remove(0, 1);
	for (uint i{0}; i < selectedFields.count(); ++i)
	{
		switch (i)
		{
			case IFC_USER:
				importFileMessageId = appUserModel()->newUserFromFile(m_importFilename, formatted);
			break;
			case IFC_MESO:
				importFileMessageId = appMesoModel()->newMesoFromFile(m_importFilename, true, formatted);
			break;
			case IFC_MESOSPLIT_A:
			case IFC_MESOSPLIT_B:
			case IFC_MESOSPLIT_C:
			case IFC_MESOSPLIT_D:
			case IFC_MESOSPLIT_E:
			case IFC_MESOSPLIT_F:
				importFileMessageId = appMesoModel()->importSplitFromFile(m_importFilename, appMesoModel()->importIdx(), i, formatted);
			break;
			case IFC_EXERCISES:
				importFileMessageId = appExercisesList()->newExerciseFromFile(m_importFilename, formatted);
			break;
			case IFC_WORKOUT: //TODO Display a dialog offering all the possible dates into which to add the workout
				importFileMessageId = appMesoModel()->mesoCalendarManager()->importWorkoutFromFile(m_importFilename, appMesoModel()->importIdx(),
																				QDate::currentDate(), formatted);
			break;
		}
		if (importFileMessageId != APPWINDOW_MSG_IMPORT_OK)
			break;
	}
	displayMessageOnAppWindow(importFileMessageId, m_importFilename);
}

void QmlItemManager::displayImportDialogMessageAfterMesoSelection(const int meso_idx)
{
	appMesoModel()->setImportIdx(meso_idx);
	emit mesoForImportSelected();
}

void QmlItemManager::getSettingsPage(const uint startPageIndex)
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this};
	m_usersManager->getSettingsPage(startPageIndex);
}

void QmlItemManager::getCoachesPage()
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this};
	m_usersManager->getCoachesPage();
}

void QmlItemManager::getClientsPage()
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this};
	m_usersManager->getClientsPage();
}

void QmlItemManager::getExercisesPage(QmlWorkoutInterface *connectPage)
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
			connect(m_weatherComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				if (status == QQmlComponent::Ready)
					createWeatherPage_part2();
#ifndef QT_NO_DEBUG
				else if (status == QQmlComponent::Error)
				{
					qDebug() << m_weatherComponent->errorString();
					return;
				}
#endif
			}, Qt::SingleShotConnection);
		}
		else
			createWeatherPage_part2();
	}
	else
		appPagesListModel()->openPage(m_weatherPage);
}

void QmlItemManager::getStatisticsPage()
{
	if (!m_statisticsPage)
	{
		m_statisticsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/StatisticsPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_statisticsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_statisticsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				if (status == QQmlComponent::Ready)
					createStatisticsPage_part2();
#ifndef QT_NO_DEBUG
				else if (status == QQmlComponent::Error)
				{
					qDebug() << m_statisticsComponent->errorString();
					return;
				}
#endif
			}, Qt::SingleShotConnection);
		}
		else
			createStatisticsPage_part2();
	}
	else
		appPagesListModel()->openPage(m_statisticsPage);
}

void QmlItemManager::getAllWorkoutsPage()
{
	if (!m_allWorkoutsPage)
	{
		m_allWorkoutsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/AllWorkouts.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_allWorkoutsComponent->status() != QQmlComponent::Ready)
		{
			connect(m_allWorkoutsComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				if (status == QQmlComponent::Ready)
					createAllWorkoutsPage_part2();
#ifndef QT_NO_DEBUG
				else if (status == QQmlComponent::Error)
				{
					qDebug() << m_allWorkoutsComponent->errorString();
					return;
				}
#endif
			}, Qt::SingleShotConnection);
		}
		else
			createAllWorkoutsPage_part2();
	}
	else
		appPagesListModel()->openPage(m_allWorkoutsPage);
}

void QmlItemManager::showSimpleExercisesList(QQuickItem *parentPage, const QString &filter) const
{
	if (appExercisesList()->count() == 0)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		const int conn_id{appDBInterface()->getAllExercises()};
		*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn_id,conn,filter] (const int _conn_id) {
			if (conn_id == _conn_id)
			{
				disconnect(*conn);
				appExercisesList()->setFilter(filter);
			}
		});
	}
	else
		appExercisesList()->setFilter(filter);
	QMetaObject::invokeMethod(parentPage, "showSimpleExercisesList");
}

void QmlItemManager::hideSimpleExercisesList(QQuickItem *parentPage) const
{
	QMetaObject::invokeMethod(parentPage, "hideSimpleExercisesList");
}

const QString &QmlItemManager::setExportFileName(const QString &filename)
{
	m_exportFilename = std::move(appSettings()->localAppFilesDir() + filename);
	return m_exportFilename;
}

void QmlItemManager::continueExport(int exportMessageId, const bool bShare)
{
	const QString &filename{appUtils()->getFileName(m_exportFilename)};
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

void QmlItemManager::getPasswordDialog(const QString &title, const QString &message) const
{
	connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), this,
				SLOT(qmlPasswordDialogClosed_slot(int,QString)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "showPasswordDialog", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::openRequestedFile(const QString &filename, const int wanted_content)
{
	QString corrected_filename;
	#ifdef Q_OS_ANDROID
	const QString &androidFilename{appOsInterface()->readFileFromAndroidFileDialog(filename)};
	corrected_filename = std::move(appUtils()->getCorrectPath(filename));
	#else
	corrected_filename = filename;
	#endif

	std::optional<bool> formatted;
	uint file_contents;
	if (!appUtils()->scanFile(corrected_filename, formatted, file_contents))
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_OPEN_FAILED, corrected_filename);
		return;
	}
	if (!formatted.has_value())
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE, corrected_filename);
		return;
	}
	if (file_contents == 0)
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_FAILED, corrected_filename);
		return;
	}
	if (isBitSet(wanted_content, IFC_MESO) && !isBitSet(file_contents, IFC_MESO))
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_UNKNOWN_ERROR, tr("The TP file does not contain any information for a Training Program"));
		return;
	}
	if (isBitSet(wanted_content, IFC_MESOSPLIT) && !isBitSet(file_contents, IFC_MESOSPLIT))
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_UNKNOWN_ERROR, tr("The TP file does not contain any information for an exercises plan"));
		return;
	}
	if (isBitSet(wanted_content, IFC_WORKOUT) && !isBitSet(file_contents, IFC_WORKOUT))
	{
		displayMessageOnAppWindow(APPWINDOW_MSG_UNKNOWN_ERROR, tr("The TP file does not contain any information for a workout"));
		return;
	}

	QStringList importOptions{ifc_count};
	if (isBitSet(file_contents, IFC_MESO))
	{
		importOptions[IFC_MESO] = std::move(tr("Complete Training Plan"));
		if (isBitSet(file_contents, IFC_USER))
			importOptions[IFC_USER] = std::move(tr("Coach information"));
	}
	if (isBitSet(file_contents, IFC_MESOSPLIT))
	{
		const bool newMesoImport{appMesoModel()->importIdx() < 0};
		if (isBitSet(file_contents, IFC_MESOSPLIT_A))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('A'))
				importOptions[IFC_MESOSPLIT_A] = std::move(tr("Exercises Program A"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_A);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_B))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('B'))
				importOptions[IFC_MESOSPLIT_B] = std::move(tr("Exercises Program B"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_B);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_C))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('C'))
				importOptions[IFC_MESOSPLIT_C] = std::move(tr("Exercises Program C"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_C);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_D))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('D'))
				importOptions[IFC_MESOSPLIT_D] = std::move(tr("Exercises Program D"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_D);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_E))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('E'))
				importOptions[IFC_MESOSPLIT_E] = std::move(tr("Exercises Program E"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_E);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_F))
		{
			if (!newMesoImport && appMesoModel()->split(appMesoModel()->importIdx()).contains('F'))
				importOptions[IFC_MESOSPLIT_F] = std::move(tr("Exercises Program F"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_F);
		}
	}
	else
	{
		if (isBitSet(file_contents, IFC_WORKOUT))
			importOptions[IFC_WORKOUT] = std::move(tr("One Workout"));
		else if (isBitSet(file_contents, IFC_EXERCISES))
			importOptions[IFC_EXERCISES] = std::move(tr("Exercises database update"));
	}

	m_importFilename = QString{formatted ? 'f' : 'r'} + filename;
	const QList<TPBool> selectedFields{ifc_count};
	QMetaObject::invokeMethod(appMainWindow(), "createImportConfirmDialog", Q_ARG(QStringList, importOptions), Q_ARG(QList<TPBool>, selectedFields));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::mainWindowStarted() const
{
	appOsInterface()->initialCheck();
}

void QmlItemManager::displayMessageOnAppWindow(const int message_id, const QString &fileName,
		const QString &image_source, const uint msecs) const
{
	QString title, message;
	switch (message_id)
	{
		case APPWINDOW_MSG_CUSTOM_MESSAGE:
		{
			title = std::move(appUtils()->getCompositeValue(0, fileName, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
		}
		break;
		case APPWINDOW_MSG_CUSTOM_WARNING:
		{
			title = std::move(tr("Warning! ") + appUtils()->getCompositeValue(0, fileName, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
		}
		break;
		case APPWINDOW_MSG_CUSTOM_ERROR:
		{
			title = std::move(tr("Error! ") + appUtils()->getCompositeValue(0, fileName, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
		}
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
			message = std::move(appUtils()->getFileName(fileName));
		break;
	}
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message),
					Q_ARG(QString, image_source), Q_ARG(int, static_cast<int>(msecs)));
}

void QmlItemManager::exportSlot(const QString &filePath)
{
	int messageId(APPWINDOW_MSG_EXPORT_FAILED);
	if (!filePath.isEmpty())
	{
		messageId = APPWINDOW_MSG_EXPORT_OK;
		QFile file{filePath};
		if (file.exists())
			messageId = file.remove() ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_EXPORT_FAILED;
		if (messageId == APPWINDOW_MSG_EXPORT_OK)
			if (!appUtils()->copyFile(m_exportFilename, filePath))
				messageId = APPWINDOW_MSG_EXPORT_FAILED;
	}
	displayMessageOnAppWindow(messageId);
	QFile::remove(m_exportFilename);
	m_exportFilename.clear();
}

void QmlItemManager::importSlot_FileChosen(const QString &filePath, const int content_type)
{
	if (!filePath.isEmpty())
		openRequestedFile(filePath, content_type);
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_CANCELED);
}

void QmlItemManager::createWeatherPage_part2()
{
	m_weatherPage = static_cast<QQuickItem*>(m_weatherComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_weatherPage, QQmlEngine::CppOwnership);
	m_weatherPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	appPagesListModel()->openPage(m_weatherPage, std::move(tr("Weather Forecast")));
}

void QmlItemManager::createStatisticsPage_part2()
{
	m_statisticsPage = static_cast<QQuickItem*>(m_statisticsComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_statisticsPage, QQmlEngine::CppOwnership);
	m_statisticsPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	appPagesListModel()->openPage(m_statisticsPage, std::move(tr("Statistics")));
}

void QmlItemManager::createAllWorkoutsPage_part2()
{
	m_wokoutsCalendar = new TPWorkoutsCalendar{this};
	QVariantMap allWorkoutsProperties;
	allWorkoutsProperties.insert("workoutsCalendar", QVariant::fromValue(m_wokoutsCalendar));
	m_allWorkoutsPage = static_cast<QQuickItem*>(m_allWorkoutsComponent->createWithInitialProperties(allWorkoutsProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_allWorkoutsPage, QQmlEngine::CppOwnership);
	m_allWorkoutsPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	appPagesListModel()->openPage(m_allWorkoutsPage, std::move(tr("All Workouts")));

	//connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);
	connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, m_wokoutsCalendar, &TPWorkoutsCalendar::reScanMesocycles);

	m_wokoutsCalendar->scanMesocycles();
}
