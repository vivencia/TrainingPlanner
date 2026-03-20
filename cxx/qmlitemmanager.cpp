#include "qmlitemmanager.h"
#include "dbcalendarmodel.h"
#include "dbexercisesmodel.h"
#include "dbexerciseslistmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"

#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmlworkoutinterface.h"
#include "qmluserinterface.h"

#include "pageslistmodel.h"
#include "osinterface.h"
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "tputils.h"

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

enum MESSAGE_ICON {
	MI_None = -1, MI_OK = 0, MI_Error = 1, MI_Warning = 2,
};

QmlItemManager::QmlItemManager(QQmlApplicationEngine *qml_engine) : QObject{nullptr}
{
	_appItemManager = this;
	REGISTER_QML_SINGLETON(QmlItemManager, this);
	_appQmlEngine = qml_engine;

	QQuickStyle::setStyle(appSettings()->themeStyle());
	QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
	appQmlEngine()->addImportPath(":/"_L1);
	appQmlEngine()->addImageProvider("tpimageprovider"_L1, new TPImageProvider{});

	QAnyStringView main_module{"Main"};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(),
																[this] (const QObject *const obj, const QUrl &objUrl) {
		if (!obj) {
			#ifndef QT_NO_DEBUG
			qDebug() << "*******************Mainwindow not loaded*******************";
			qDebug() << objUrl;
			#endif
			QCoreApplication::exit(-1);
		}
		else {
			_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
			appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));
			m_homePage = appMainWindow()->findChild<QQuickItem*>("homePage");

			appUserModel()->initUserSession();
			connect(appHomePage(), SIGNAL(mesosViewChanged(bool)), this, SLOT(homePageViewChanged(bool)));
			connect(appUtils(), &TPUtils::tpFileOpenRequest, this, &QmlItemManager::openTPFile);
			if (m_qml_testing) {
				connect(appUserModel(), &DBUserModel::mainUserConfigurationFinished, this, [this] () {
					connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoDataLoaded, this, [this] () {
						//showSimpleExercisesList(appHomePage(), QString{});
						/*connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::calendarReady, this, [this] (const uint meso_idx) {
							const int cal_day{appUserModel()->actualMesoModel()->calendar(0)->calendarDay(QDate::currentDate())};
							m_workout_model = appUserModel()->actualMesoModel()->workoutForDay(meso_idx, cal_day);
							if (!m_workout_model->exercisesLoaded())
							{
								connect(m_workout_model, &DBWorkoutModel::exerciseCountChanged, this, [this] () {
									emit cppDataForQMLReady();
								}, Qt::SingleShotConnection);
							}
						}, Qt::SingleShotConnection);
						appUserModel()->actualMesoModel()->getCalendarForMeso(0);*/
					}, Qt::SingleShotConnection);
				}, Qt::SingleShotConnection);
			}
		}
	});

#ifndef Q_OS_ANDROID
	#ifndef QT_NO_DEBUG
	m_qml_testing = false;
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1) {
		if (args.at(1) == "-test"_L1) {
			m_qml_testing = true;
			main_module = "Tests";
		}
		else if (args.at(1) == "-user"_L1) {
			if (!args.at(2).isEmpty()) {
				appSettings()->setReadOnlyGroup(GLOBAL_GROUP, true);
				appSettings()->setCurrentUser(args.at(2));
			}
			else
				qDebug() << "Warning: Missing user id in the command line arguments"_L1;
		}
	}
	#else
		url = std::move("qrc:/TpQml/qml/main.qml"_L1);
	#endif
#else
	url = std::move("qrc:/TpQml/qml/main.qml"_L1);
#endif
	appQmlEngine()->loadFromModule("TpQml", main_module);
}

void QmlItemManager::exitApp()
{
	qApp->quit();
}

void QmlItemManager::chooseFileToImport()
{
	QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
}

void QmlItemManager::displayImportDialogMessageAfterMesoSelection(const int meso_idx)
{
	appUserModel()->actualMesoModel()->setImportIdx(meso_idx);
	emit mesoForImportSelected();
}

void QmlItemManager::exportMeso(const uint meso_idx, const bool share)
{
	int exportFileMessageId{0};
	if (appExercisesList()->collectExportData()) {
		const QString &exportFileName{appSettings()->localAppFilesDir() + tr("TrainingPlanner Exercises List") + ".txt"_L1};
		exportFileMessageId = appExercisesList()->exportToFile(exportFileName);
		if (exportFileMessageId >= 0) {
			if (share) {
				appOsInterface()->shareFile(exportFileName);
				exportFileMessageId = TP_RET_CODE_SHARE_OK;
			}
			else
				QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, exportFileName));
		}
		appItemManager()->displayMessageOnAppWindow(exportFileMessageId, exportFileName);
	}
	else
		exportFileMessageId = TP_RET_CODE_NOTHING_TO_EXPORT;
	appItemManager()->displayMessageOnAppWindow(exportFileMessageId);
}

void QmlItemManager::getSettingsPage()
{
	usersManager()->getSettingsPage();
}

void QmlItemManager::getUserPage()
{
	usersManager()->getUserPage();
}

void QmlItemManager::getCoachesPage()
{
	usersManager()->getCoachesPage();
}

void QmlItemManager::getClientsPage()
{
	usersManager()->getClientsPage();
}

void QmlItemManager::getExercisesPage(QmlWorkoutInterface *connectPage)
{
	if (!m_exercisesListManager)
		m_exercisesListManager = new QmlExercisesDatabaseInterface{this};
	m_exercisesListManager->getExercisesPage(connectPage);
}

void QmlItemManager::showSimpleExercisesList(QQuickItem *parentPage, const QString &filter)
{
	appExercisesList()->setFilter(filter);
	if (!m_simpleExercisesList) {
		m_simpleExercisesListProperties.insert("parentPage", QVariant::fromValue(parentPage));
		m_simpleExercisesListComponent = new QQmlComponent{appQmlEngine(),
										QUrl{"qrc:/TpQml/qml/ExercisesAndSets/SimpleExercisesListPanel.qml"_L1}, QQmlComponent::Asynchronous};

		switch (m_simpleExercisesListComponent->status())
		{
			case QQmlComponent::Ready:
				createSimpleExercisesList(parentPage);
			break;
			case QQmlComponent::Loading:
				connect(m_simpleExercisesListComponent, &QQmlComponent::statusChanged, this, [this,parentPage] (QQmlComponent::Status status) {
					createSimpleExercisesList(parentPage);
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_simpleExercisesListComponent->errorString();
				#endif
			break;
		}
	}
	else {
		QQuickItem *cur_parent_page{m_simpleExercisesList->property("parentPage").value<QQuickItem*>()};
		if (parentPage != cur_parent_page)
			QMetaObject::invokeMethod(m_simpleExercisesList, "changeParentPage", Q_ARG(QQuickItem*, parentPage));
		showSimpleExercisesList();
	}
}

void QmlItemManager::getWeatherPage()
{
	if (!m_weatherPage) {
		m_weatherComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/TpQml/qml/Pages/WeatherPage.qml"_L1}, QQmlComponent::Asynchronous};
		switch (m_weatherComponent->status()) {
			case QQmlComponent::Ready:
				createWeatherPage_part2();
			break;
			case QQmlComponent::Loading:
				connect(m_weatherComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
					createWeatherPage_part2();
				}, Qt::SingleShotConnection);
			break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_weatherComponent->errorString();
				#endif
			break;
		}
	}
	else
		appPagesListModel()->openPage(m_weatherPage);
}

void QmlItemManager::getStatisticsPage()
{
	if (!m_statisticsPage)
	{
		m_statisticsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/TpQml/qml/Pages/StatisticsPage.qml"_L1}, QQmlComponent::Asynchronous};
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

void QmlItemManager::displayMessageOnAppWindow(const int message_id, const QString &filename_or_message,
											   const QString &image_source, const uint msecs) const
{
	QString title, message;
	int icon_to_use{MI_Error}; //Only applicable when image_source is an empty string
	if (message_id < TP_RET_CODE_CUSTOM_ERROR) {
		icon_to_use = MI_OK;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_SUCCESS:
			title = std::move(appUtils()->getCompositeValue(0, filename_or_message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, filename_or_message, record_separator));
			break;
		case TP_RET_CODE_EXPORT_OK:
			title = std::move(tr("Succesfully exported"));
			message = std::move(filename_or_message);
			break;
		case TP_RET_CODE_SHARE_OK:
			title = std::move(tr("Succesfully shared"));
			message = std::move(filename_or_message);
			break;
		case TP_RET_CODE_IMPORT_OK:
			title = std::move(tr("Successfully imported"));
			message = std::move(appUtils()->getFileName(filename_or_message));
			break;
		}
	}
	else if (message_id < TP_RET_CODE_CUSTOM_WARNING) {
		icon_to_use = MI_Error;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_ERROR:
			title = std::move(appUtils()->getCompositeValue(0, filename_or_message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, filename_or_message, record_separator));
			break;
		case TP_RET_CODE_UNKNOWN_ERROR:
			title = std::move(tr("Unknown Error"));
			message = filename_or_message;
			break;
		case TP_RET_CODE_FILE_NOT_FOUND:
			title = std::move(tr("File not found!"));
			message = filename_or_message;
			break;
		case TP_RET_CODE_OPEN_READ_FAILED:
			title = std::move(tr("Failed to open file"));
			message = std::move(appUtils()->getFileName(filename_or_message));
			break;
		case TP_RET_CODE_WRONG_IMPORT_FILE_TYPE:
			title = std::move(tr("Error"));
			message = std::move(tr("File type not recognized"));
			break;
		case TP_RET_CODE_CORRUPT_FILE:
			title = std::move(tr("Error"));
			message = std::move(appUtils()->getFileName(filename_or_message) +  tr("\n is formatted wrongly or corrupted"));
			break;
		case TP_RET_CODE_SHARE_FAILED:
			title = std::move(tr("Sharing failed"));
			message = std::move(appUtils()->getFileName(filename_or_message));
			break;
		case TP_RET_CODE_EXPORT_FAILED:
			title = std::move(tr("Export failed"));
			message = filename_or_message;
			break;
		case TP_RET_CODE_IMPORT_FAILED:
			title = std::move(tr("Import from file failed"));
			message = std::move(appUtils()->getFileName(filename_or_message));
			break;
		case TP_RET_CODE_OPEN_CREATE_FAILED:
			title = std::move(tr("Could not open file for exporting"));
			message = std::move(appUtils()->getFileName(filename_or_message));
			break;
		case TP_RET_CODE_SERVER_UNREACHABLE:
			title = std::move(tr("Online server unavailable"));
			message = std::move(tr("Try it again later"));
			break;
		}
	}
	else if (message_id < TP_RET_CODE_CUSTOM_MESSAGE) {
		icon_to_use = MI_Warning;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_WARNING:
			title = std::move(tr("Warning! ") + appUtils()->getCompositeValue(0, filename_or_message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, filename_or_message, record_separator));
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
	else {
		icon_to_use = MI_None;
		title = std::move(appUtils()->getCompositeValue(0, filename_or_message, record_separator));
		message = std::move(appUtils()->getCompositeValue(1, filename_or_message, record_separator));
	}

	QString img_src;
	if (image_source.isEmpty()) {
		switch (icon_to_use) {
		case MI_Error:		img_src = std::move("error");			break;
		case MI_Warning:	img_src = std::move("warning");			break;
		case MI_OK:			img_src = std::move("set-completed");	break;
		case MI_None:												break;
		}
	}
	else
		img_src = image_source;

	QMetaObject::invokeMethod(appMainWindow(), "showAppMainMessageDialog", Q_ARG(QString, title), Q_ARG(QString, message),
				Q_ARG(QString, img_src), Q_ARG(int, static_cast<int>(msecs)), Q_ARG(QString, QString{}), Q_ARG(QString, QString{}));
}

void QmlItemManager::displayActivityResultMessage(const int requestCode, const int resultCode) const
{
	int message_id(0);
	switch (resultCode) {
		case -1: message_id = TP_RET_CODE_SUCCESS; break;
		case 0: message_id = TP_RET_CODE_SHARE_FAILED; break;
		default: message_id = TP_RET_CODE_UNKNOWN_ERROR; break;
	}
	displayMessageOnAppWindow(message_id);
}

void QmlItemManager::getPasswordDialog(const QString &title, const QString &message) const
{
	connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), SLOT(qmlPasswordDialogClosed_slot(int,QString)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "showPasswordDialog", Q_ARG(QString, title), Q_ARG(QString, message));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::openTPFile(uint32_t tp_filetype, const QString &filename, const bool formatted, const QVariant &extra_info)
{
	std::shared_ptr<QMetaObject::Connection> conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &QmlItemManager::qmlImportDialogClose, this, [=,this] (bool result) -> void {
		disconnect(*conn);
		switch (tp_filetype) {
		case TPUtils::FT_TP_USER_PROFILE:
			appUserModel()->newUserFromFile(filename, formatted);
			break;
		case TPUtils::FT_TP_PROGRAM:
			appUserModel()->actualMesoModel()->newMesoFromFile(filename, false, formatted);
			break;
		case TPUtils::FT_TP_WORKOUT_A:
		case TPUtils::FT_TP_WORKOUT_B:
		case TPUtils::FT_TP_WORKOUT_C:
		case TPUtils::FT_TP_WORKOUT_D:
		case TPUtils::FT_TP_WORKOUT_E:
		case TPUtils::FT_TP_WORKOUT_F:
			appUserModel()->actualMesoModel()->newWorkoutFromFile(filename, formatted, extra_info);
			break;
		case TPUtils::FT_TP_EXERCISES:
			appExercisesList()->newExerciseFromFile(filename, formatted);
			break;
		}
	});
	connect(appMainWindow(), SIGNAL(tpFileOpenInquiryResult(bool)), appItemManager(), SLOT(qmlImportDialogClose(bool)), Qt::SingleShotConnection);
	QString str_type, str_details, str_image;
	const QString &str_extra_info{extra_info.toString()};
	const QString &coach{appUserModel()->userNameFromId(appUtils()->getCompositeValue(0, str_extra_info, record_separator))};

	switch (tp_filetype) {
	case TPUtils::FT_TP_USER_PROFILE:
	{
		const bool is_coach{appUtils()->getCompositeValue(1, str_extra_info, record_separator) == "1"_L1};
		str_type = std::move(is_coach ? tr("data for a new coach") : tr("data for a new client"));
		str_details = coach;
		str_image = std::move(is_coach ? "manage-coaches"_L1 : "manage-clients"_L1);
	}
	break;
	case TPUtils::FT_TP_PROGRAM:
		str_type = std::move(tr("program"));
		str_details = std::move(tr("A complete exercises program from coach ") % coach);
		str_image = std::move("meso_preview"_L1);
		break;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F:
	{
		const uint meso_idx{appUtils()->getCompositeValue(1, str_extra_info, record_separator).toUInt()};
		const QChar &splitletter{appUtils()->getCompositeValue(2, str_extra_info, record_separator).at(0)};
		str_type = std::move(tr("workout"));
		str_details = std::move(tr("An extra workout from ") % coach % tr(" for the program: ") %
								appUserModel()->actualMesoModel()->name(meso_idx) % tr(" for the next time you train ") %
								appUserModel()->actualMesoModel()->muscularGroup(meso_idx, splitletter));
		str_image = std::move("workout_preview"_L1);
	}
	break;
	case TPUtils::FT_TP_EXERCISES:
		str_type = std::move(tr("Excercise Description"));
		str_details = std::move(tr("A new exercise for the exercises database from ") % coach);
		str_image = std::move("exerciselist_preview"_L1);
		break;
	default:
		Q_UNREACHABLE();
	}
	QMetaObject::invokeMethod(appMainWindow(), "confirmTPFileOpening", Q_ARG(QString, str_type), Q_ARG(QString, str_details),
																													Q_ARG(QString, str_image));
}

void QmlItemManager::mainWindowStarted() const
{
	appOsInterface()->initialCheck();
}

void QmlItemManager::homePageViewChanged(const bool own_mesos_view)
{
	appUserModel()->actualMesoModel()->setCurrentMesosView(own_mesos_view);
}

void QmlItemManager::showSimpleExercisesList()
{
	int name_field_ypos{0};
	QQuickItem *parent_page{m_simpleExercisesList->property("parentPage").value<QQuickItem*>()};
	QMetaObject::invokeMethod(parent_page, "getExerciseNameFieldYPos", Q_RETURN_ARG(int,name_field_ypos));
	QMetaObject::invokeMethod(m_simpleExercisesList, "show",
													Q_ARG(int, name_field_ypos <= appSettings()->pageHeight() / 2 ? -2 : 0));
}
void QmlItemManager::createSimpleExercisesList(QQuickItem *parentPage)
{
	m_simpleExercisesList = m_simpleExercisesListComponent->createWithInitialProperties(m_simpleExercisesListProperties,
																								appQmlEngine()->rootContext());
	appQmlEngine()->setObjectOwnership(m_simpleExercisesList, QQmlEngine::CppOwnership);
	m_simpleExercisesList->setProperty("parent", QVariant::fromValue(parentPage));
	connect(m_simpleExercisesList, SIGNAL(exerciseSelected(QQuickItem*)), this, SIGNAL(selectedExerciseFromSimpleExercisesList(QQuickItem*)));
	showSimpleExercisesList();
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

inline QmlUserInterface *QmlItemManager::usersManager()
{
	static QmlUserInterface *users_manager{nullptr};
	if (!users_manager)
		users_manager = new QmlUserInterface{this};
	return users_manager;
}
