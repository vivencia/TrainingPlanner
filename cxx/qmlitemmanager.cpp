#include "qmlitemmanager.h"

#include "dbcalendarmodel.h"
#include "dbexercisesmodel.h"
#include "dbexerciseslistmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "homepagemesomodel.h"
#include "pageslistmodel.h"

#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmlworkoutinterface.h"
#include "qmluserinterface.h"

#include "pageslistmodel.h"
#include "osinterface.h"
#include "statistics/tpstatistics.h"
#include "tpbool.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "tptimer.h"
#include "translationclass.h"

#include "online_services/onlineuserinfo.h"
#include "online_services/tpchat.h"
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

#define REGISTER_QML_TYPE(cpp_name, qmlname) \
	qmlRegisterType<cpp_name> ("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, qmlname);

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
}

void QmlItemManager::configureQmlEngine()
{
	QQuickStyle::setStyle(appSettings()->themeStyle());
	QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

	REGISTER_QML_TYPE(DBUserModel, "DBUserModel")
	REGISTER_QML_TYPE(DBExercisesListModel, "DBExercisesListModel")
	REGISTER_QML_TYPE(DBMesocyclesModel, "MesocyclesModel")
	REGISTER_QML_TYPE(DBExercisesModel, "DBExercisesModel")
	REGISTER_QML_TYPE(DBCalendarModel, "DBCalendarModel")
	REGISTER_QML_TYPE(TPTimer, "TPTimer")
	REGISTER_QML_TYPE(TPImage, "TPImage")
	REGISTER_QML_TYPE(QmlUserInterface, "UserManager")
	REGISTER_QML_TYPE(QmlExercisesDatabaseInterface, "ExercisesListManager")
	REGISTER_QML_TYPE(QMLMesoInterface, "MesoManager")
	REGISTER_QML_TYPE(QmlMesoCalendarInterface, "CalendarManager")
	REGISTER_QML_TYPE(QmlMesoSplitInterface, "SplitManager")
	REGISTER_QML_TYPE(QmlWorkoutInterface, "WorkoutManager")
	REGISTER_QML_TYPE(WeatherInfo, "WeatherInfo")
	REGISTER_QML_TYPE(TPStatistics, "Statistics")
	REGISTER_QML_TYPE(PagesListModel, "PagesListModel")
	REGISTER_QML_TYPE(OnlineUserInfo, "OnlineUserInfo")
	REGISTER_QML_TYPE(TPMessagesManager, "MessagesManager")
	REGISTER_QML_TYPE(HomePageMesoModel, "HomePageMesoModel")
	REGISTER_QML_TYPE(TPChat, "ChatModel")

	QList<QQmlContext::PropertyPair> global_properties{9};
	global_properties[0] = std::move(QQmlContext::PropertyPair{ "appSettings"_L1, QVariant::fromValue(appSettings()) });
	global_properties[1] = std::move(QQmlContext::PropertyPair{ "appUtils"_L1, QVariant::fromValue(appUtils()) });
	global_properties[2] = std::move(QQmlContext::PropertyPair{ "appTr"_L1, QVariant::fromValue(appTr()) });
	global_properties[3] = std::move(QQmlContext::PropertyPair{ "userModel"_L1, QVariant::fromValue(appUserModel()) });
	global_properties[4] = std::move(QQmlContext::PropertyPair{ "exercisesModel"_L1, QVariant::fromValue(appExercisesList()) });
	global_properties[5] = std::move(QQmlContext::PropertyPair{ "itemManager"_L1, QVariant::fromValue(this) });
	global_properties[6] = std::move(QQmlContext::PropertyPair{ "appStatistics"_L1, QVariant::fromValue(appStatistics()) });
	global_properties[7] = std::move(QQmlContext::PropertyPair{ "appMessages"_L1, QVariant::fromValue(new TPMessagesManager{this}) });
	global_properties[8] = std::move(QQmlContext::PropertyPair{ "osInterface"_L1, QVariant::fromValue(appOsInterface()) });
	appQmlEngine()->rootContext()->setContextProperties(global_properties);
	appQmlEngine()->addImportPath(":/"_L1);
	appQmlEngine()->addImageProvider("tpimageprovider"_L1, new TPImageProvider{});

	QUrl url{};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(),
																[this] (const QObject *const obj, const QUrl &objUrl)
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
			appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));
			m_homePage = appMainWindow()->findChild<QQuickItem*>("homePage");

			appUserModel()->initUserSession();
			connect(appMainWindow(), SIGNAL(openFileChosen(QString,int)), this, SLOT(importSlot_FileChosen(QString,int)));
			connect(appMainWindow(), SIGNAL(openFileRejected(QString)), this, SLOT(importSlot_FileChosen(QString)));
			connect(appMainWindow(), SIGNAL(saveFileChosen(QString)), this, SLOT(exportSlot(QString)));
			connect(appMainWindow(), SIGNAL(saveFileRejected(QString)), this, SLOT(exportSlot(QString)));
			connect(appHomePage(), SIGNAL(mesosViewChanged(bool)), this, SLOT(homePageViewChanged(bool)));
			if (m_qml_testing)
			{
				connect(appUserModel(), &DBUserModel::mainUserConfigurationFinished, [this] () {
					connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoDataLoaded, [this] () {
						connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::calendarReady, this, [this] (const uint meso_idx) {
							const int cal_day{appUserModel()->actualMesoModel()->calendar(0)->calendarDay(QDate::currentDate())};
							m_workout_model = appUserModel()->actualMesoModel()->workoutForDay(meso_idx, cal_day);
							if (!m_workout_model->exercisesLoaded())
							{
								connect(m_workout_model, &DBWorkoutModel::exerciseCountChanged, [this] () {
									emit cppDataForQMLReady();
								});
							}
						});
						appUserModel()->actualMesoModel()->getCalendarForMeso(0);
					});
				});
			}
		}
	});

#ifndef Q_OS_ANDROID
	#ifndef QT_NO_DEBUG
	m_qml_testing = false;
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1)
	{
		if (args.at(1) == "-test"_L1)
		{
			m_qml_testing = true;
			url = std::move("qrc:/qml/tests.qml"_L1);
		}
		else if (args.at(1) == "-user"_L1)
		{
			if (!args.at(2).isEmpty())
			{
				appSettings()->setReadOnlyGroup(GLOBAL_GROUP, true);
				appSettings()->setCurrentUser(args.at(2));
			}
			else
				qDebug() << "Warning: Missing user id in the command line arguments"_L1;
		}
	}
	if (url.isEmpty())
		url = std::move("qrc:/qml/main.qml"_L1);
	#else
		url = std::move("qrc:/qml/main.qml"_L1);
	#endif
#else
	url = std::move("qrc:/qml/main.qml"_L1);
#endif

	appQmlEngine()->load(url);
}

void QmlItemManager::exitApp()
{
	qApp->quit();
}

void QmlItemManager::chooseFileToImport()
{
	QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_MESO));
}

void QmlItemManager::importFromSelectedFile(const QList<bool> &selectedFields)
{
	const bool formatted{m_importFilename.at(0).cell() == 'f'};
	int importFileMessageId{TP_RET_CODE_IMPORT_FAILED};

	m_importFilename.remove(0, 1);
	for (uint i{0}; i < selectedFields.count(); ++i)
	{
		switch (i)
		{
			case IFC_USER:
				importFileMessageId = appUserModel()->newUserFromFile(m_importFilename, formatted);
			break;
			case IFC_MESO:
				importFileMessageId = appUserModel()->actualMesoModel()->newMesoFromFile(m_importFilename, formatted);
			break;
			case IFC_MESOSPLIT_A:
			case IFC_MESOSPLIT_B:
			case IFC_MESOSPLIT_C:
			case IFC_MESOSPLIT_D:
			case IFC_MESOSPLIT_E:
			case IFC_MESOSPLIT_F:
				importFileMessageId = appUserModel()->actualMesoModel()->importSplitFromFile(m_importFilename, appUserModel()->actualMesoModel()->importIdx(), i, formatted);
			break;
			case IFC_EXERCISES:
				importFileMessageId = appExercisesList()->newExerciseFromFile(m_importFilename, formatted);
			break;
			case IFC_WORKOUT: //TODO Display a dialog offering all the possible dates into which to add the workout
				importFileMessageId = appUserModel()->actualMesoModel()->workingWorkout()->newExercisesFromFile(
																								m_importFilename, formatted);
			break;
		}
		if (importFileMessageId != TP_RET_CODE_SUCCESS)
			break;
	}
	displayMessageOnAppWindow(importFileMessageId, m_importFilename);
}

void QmlItemManager::displayImportDialogMessageAfterMesoSelection(const int meso_idx)
{
	appUserModel()->actualMesoModel()->setImportIdx(meso_idx);
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

void QmlItemManager::showSimpleExercisesList(QQuickItem *parentPage, const QString &filter) const
{
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
	if (exportMessageId == TP_RET_CODE_SUCCESS)
	{
		if (bShare)
		{
			appOsInterface()->shareFile(m_exportFilename);
			exportMessageId = TP_RET_CODE_SUCCESS;
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
		case -1: message_id = TP_RET_CODE_SUCCESS; break;
		case 0: message_id = TP_RET_CODE_SHARE_FAILED; break;
		default: message_id = TP_RET_CODE_UNKNOWN_ERROR; break;
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
		displayMessageOnAppWindow(TP_RET_CODE_OPEN_READ_FAILED, corrected_filename);
		return;
	}
	if (!formatted.has_value())
	{
		displayMessageOnAppWindow(TP_RET_CODE_WRONG_IMPORT_FILE_TYPE, corrected_filename);
		return;
	}
	if (file_contents == 0)
	{
		displayMessageOnAppWindow(TP_RET_CODE_IMPORT_FAILED, corrected_filename);
		return;
	}
	if (isBitSet(wanted_content, IFC_MESO) && !isBitSet(file_contents, IFC_MESO))
	{
		displayMessageOnAppWindow(TP_RET_CODE_WRONG_IMPORT_FILE_TYPE, tr("The TP file does not contain any information for a Training Program"));
		return;
	}
	if (isBitSet(wanted_content, IFC_MESOSPLIT) && !isBitSet(file_contents, IFC_MESOSPLIT))
	{
		displayMessageOnAppWindow(TP_RET_CODE_WRONG_IMPORT_FILE_TYPE, tr("The TP file does not contain any information for an exercises plan"));
		return;
	}
	if (isBitSet(wanted_content, IFC_WORKOUT) && !isBitSet(file_contents, IFC_WORKOUT))
	{
		displayMessageOnAppWindow(TP_RET_CODE_WRONG_IMPORT_FILE_TYPE, tr("The TP file does not contain any information for a workout"));
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
		const bool newMesoImport{appUserModel()->actualMesoModel()->importIdx() < 0};
		if (isBitSet(file_contents, IFC_MESOSPLIT_A))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('A'))
				importOptions[IFC_MESOSPLIT_A] = std::move(tr("Exercises Program A"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_A);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_B))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('B'))
				importOptions[IFC_MESOSPLIT_B] = std::move(tr("Exercises Program B"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_B);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_C))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('C'))
				importOptions[IFC_MESOSPLIT_C] = std::move(tr("Exercises Program C"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_C);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_D))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('D'))
				importOptions[IFC_MESOSPLIT_D] = std::move(tr("Exercises Program D"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_D);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_E))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('E'))
				importOptions[IFC_MESOSPLIT_E] = std::move(tr("Exercises Program E"));
			else
				unSetBit(file_contents, IFC_MESOSPLIT_E);
		}
		if (isBitSet(file_contents, IFC_MESOSPLIT_F))
		{
			if (!newMesoImport && appUserModel()->actualMesoModel()->split(appUserModel()->actualMesoModel()->importIdx()).contains('F'))
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

enum MESSAGE_ICON {
	MI_None = -1, MI_OK = 0, MI_Error = 1, MI_Warning = 2,
};

void QmlItemManager::displayMessageOnAppWindow(const int message_id, const QString &fileName,
															const QString &image_source, const uint msecs) const
{
	QString title, message;
	int icon_to_use{MI_Error}; //Only applicable when image_source is an empty string
	if (message_id < TP_RET_CODE_CUSTOM_ERROR)
	{
		icon_to_use = MI_OK;
		switch (message_id)
		{
			case TP_RET_CODE_CUSTOM_SUCCESS:
				title = std::move(appUtils()->getCompositeValue(0, fileName, record_separator));
				message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
			break;
			case TP_RET_CODE_EXPORT_OK:
				title = std::move(tr("Succesfully exported"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_SHARE_OK:
				title = std::move(tr("Succesfully shared"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_IMPORT_OK:
				title = std::move(tr("Successfully imported"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
		}
	}
	else if (message_id < TP_RET_CODE_CUSTOM_WARNING)
	{
		icon_to_use = MI_Error;
		switch (message_id)
		{
			case TP_RET_CODE_CUSTOM_ERROR:
				title = std::move(appUtils()->getCompositeValue(0, fileName, record_separator));
				message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
			break;
			case TP_RET_CODE_UNKNOWN_ERROR:
				title = std::move(tr("Unknown Error"));
				message = fileName;
			break;
			case TP_RET_CODE_FILE_NOT_FOUND:
				title = std::move(tr("File not found!"));
				message = fileName;
			break;
			case TP_RET_CODE_OPEN_READ_FAILED:
				title = std::move(tr("Failed to open file"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_WRONG_IMPORT_FILE_TYPE:
				title = std::move(tr("Error"));
				message = std::move(tr("File type not recognized"));
			break;
			case TP_RET_CODE_CORRUPT_FILE:
				title = std::move(tr("Error"));
				message = std::move(appUtils()->getFileName(fileName) +  tr("\n is formatted wrongly or corrupted"));
			break;
			case TP_RET_CODE_SHARE_FAILED:
				title = std::move(tr("Sharing failed"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_EXPORT_FAILED:
				title = std::move(tr("Export failed"));
				message = std::move(tr("Operation canceled"));
			break;
			case TP_RET_CODE_IMPORT_FAILED:
				title = std::move(tr("Import from file failed"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_OPEN_CREATE_FAILED:
				title = std::move(tr("Could not open file for exporting"));
				message = std::move(appUtils()->getFileName(fileName));
			break;
			case TP_RET_CODE_SERVER_UNREACHABLE:
				title = std::move(tr("Online server unavailable"));
				message = std::move(tr("Try it again later"));
			break;
		}
	}
	else if (message_id < TP_RET_CODE_CUSTOM_MESSAGE)
	{
		icon_to_use = MI_Warning;
		switch (message_id)
		{
			case TP_RET_CODE_CUSTOM_WARNING:
				title = std::move(tr("Warning! ") + appUtils()->getCompositeValue(0, fileName, record_separator));
				message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
			break;
			case TP_RET_CODE_NOTHING_TODO:
				title = std::move(tr("Nothing to be done"));
				message = std::move(tr("File had already been imported"));
			break;
			case TP_RET_CODE_NO_MESO:
				title = std::move(tr("No program to import into"));
				message = std::move(tr("Either create a new training plan or import from a complete program file"));
			break;
			case TP_RET_CODE_NOTHING_TO_EXPORT:
				title = std::move(tr("Nothing to export"));
				message = std::move(tr("Only exercises that do not come by default with the app can be exported"));
			break;
			case TP_RET_CODE_OPERATION_CANCELED:
				title = std::move(tr("Warning"));
				message = std::move(tr("Operation canceled"));
			break;
		}
	}
	else
	{
		icon_to_use = MI_None;
		title = std::move(appUtils()->getCompositeValue(0, fileName, record_separator));
		message = std::move(appUtils()->getCompositeValue(1, fileName, record_separator));
	}

	QString img_src;
	if (image_source.isEmpty())
	{
		switch (icon_to_use)
		{
			case MI_Error: img_src = std::move("error"); break;
			case MI_Warning: img_src = std::move("warning"); break;
			case MI_OK: img_src = std::move("set-completed"); break;
			case MI_None: break;
		}
	}
	else
		img_src = image_source;

	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message),
					Q_ARG(QString, img_src), Q_ARG(int, static_cast<int>(msecs)));
}

void QmlItemManager::exportSlot(const QString &filePath)
{
	int messageId(TP_RET_CODE_EXPORT_FAILED);
	if (!filePath.isEmpty())
	{
		messageId = TP_RET_CODE_EXPORT_OK;
		QFile file{filePath};
		if (file.exists())
			messageId = file.remove() ? TP_RET_CODE_EXPORT_OK : TP_RET_CODE_EXPORT_FAILED;
		if (messageId == TP_RET_CODE_EXPORT_OK)
			if (!appUtils()->copyFile(m_exportFilename, filePath))
				messageId = TP_RET_CODE_EXPORT_FAILED;
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
		displayMessageOnAppWindow(TP_RET_CODE_OPERATION_CANCELED);
}

void QmlItemManager::homePageViewChanged(const bool own_mesos_view)
{
	appUserModel()->actualMesoModel()->setCurrentMesosView(own_mesos_view);
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
