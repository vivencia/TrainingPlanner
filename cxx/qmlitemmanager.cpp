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

QmlItemManager *QmlItemManager::_appItemManager{nullptr};
QQmlApplicationEngine *QmlItemManager::_appQmlEngine{nullptr};
QQuickWindow *QmlItemManager::_appMainWindow{nullptr};

enum MESSAGE_ICON {
	MI_None,
	MI_OK,
	MI_Error,
	MI_Warning,
};

QmlItemManager::QmlItemManager(QQmlApplicationEngine *qml_engine) : QObject{nullptr}
{
	_appItemManager = this;
	REGISTER_QML_SINGLETON(QmlItemManager, this);
	_appQmlEngine = qml_engine;

	QQuickStyle::setStyle(appSettings()->themeStyle());
	QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
#ifndef QT_NO_DEBUG
	appQmlEngine()->clearComponentCache();
#endif
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
			qApp->exit(-1);
		}
		else {
			_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
			appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));
			m_homePage = appMainWindow()->findChild<QQuickItem*>("homePage");
			m_appPagesVisualParent = appMainWindow()->findChild<QQuickItem*>("appStackView");

			appUserModel()->initUserSession();
			connect(AppHomePage(), SIGNAL(mesosViewChanged(bool)), this, SLOT(homePageViewChanged(bool)));
			connect(appUtils(), &TPUtils::tpFileOpenRequest, this, &QmlItemManager::openTPFile);
			if (m_qml_testing) {
				connect(appUserModel(), &DBUserModel::mainUserConfigurationFinished, this, [this] () {
					connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoDataLoaded, this, [this] () {
						//showSimpleExercisesList(AppHomePage(), QString{});
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

void QmlItemManager::showFirstTimeDialog()
{
	if (!m_firstTimeDlgComponent) {
		m_firstTimeDlgComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "FirstTimeDialog"_L1, QQmlComponent::Asynchronous};
		switch (m_firstTimeDlgComponent->status()) {
		case QQmlComponent::Ready:
			showFirstTimeDialog();
			break;
		case QQmlComponent::Loading:
			connect(m_firstTimeDlgComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				showFirstTimeDialog();
			}, Qt::SingleShotConnection);
			break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
			qDebug() << m_firstTimeDlgComponent->errorString();
#endif
			break;
		}
	}
	else {
		if (!m_firstTimeDlg) {
			m_firstTimeDlg = m_firstTimeDlgComponent->createWithInitialProperties(QVariantMap{
										{std::pair<QLatin1StringView, QVariant>{"parentPage", QVariant::fromValue(m_homePage)}}},
																											appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG

#endif
			appQmlEngine()->setObjectOwnership(m_firstTimeDlg, QQmlEngine::CppOwnership);
			m_firstTimeDlg->setProperty("parent", QVariant::fromValue(m_homePage));
		}
		QMetaObject::invokeMethod(m_firstTimeDlg, "showInWindow", Q_ARG(int, -Qt::AlignCenter));
	}
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
		m_simpleExercisesListComponent = new QQmlComponent{appQmlEngine(), "TpQml.Exercises"_L1, "SimpleExercisesListPanel"_L1, QQmlComponent::Asynchronous};

		switch (m_simpleExercisesListComponent->status()) {
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
	if (!m_weatherComponent) {
		m_weatherComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "WeatherPage"_L1, QQmlComponent::Asynchronous};
		connect(m_weatherComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getWeatherPage(); });
	}
	else {
		switch (m_weatherComponent->status()) {
		case QQmlComponent::Ready:
			m_weatherComponent->disconnect();
			break;
#ifndef QT_NO_DEBUG
		case QQmlComponent::Loading:
			return;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_weatherComponent->errorString();
			return;
#else
		default: return;
#endif
		}
		if (!m_weatherPage) {
			m_weatherPage = static_cast<QQuickItem*>(m_weatherComponent->create(appQmlEngine()->rootContext()));
			appQmlEngine()->setObjectOwnership(m_weatherPage, QQmlEngine::CppOwnership);
			m_weatherPage->setParentItem(appItemManager()->AppPagesVisualParent());
			appPagesListModel()->openPage(m_weatherPage, std::move(tr("Weather Forecast")));
		}
		else
			appPagesListModel()->openPage(m_weatherPage);
	}
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
	QFlags<Qt::AlignmentFlag> postion,const QString &image_source, const int msecs, const QString& button1text, const QString &button2text) const
{
	QString title, message;
	MESSAGE_ICON icon_to_use{MI_Error}; //Only applicable when image_source is an empty string
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

	if (!button1text.isEmpty())
		connect(appMainWindow(), SIGNAL(generalMessagesPopupClicked(int)), this, SLOT(generalMessagesPopupClicked(int)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "showAppMainMessageDialog", Q_ARG(int, -postion), Q_ARG(QString, title),
		Q_ARG(QString, message), Q_ARG(QString, img_src), Q_ARG(int, msecs), Q_ARG(QString, button1text), Q_ARG(QString, button2text));
}

void QmlItemManager::showOnlineMessagesManagerDialog(const bool show)
{
	QMetaObject::invokeMethod(appMainWindow(), "showOnlineMessagesDialog", Q_ARG(bool, show));
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
	QMetaObject::invokeMethod(m_simpleExercisesList, "showInWindow", Q_ARG(int, name_field_ypos <= appSettings()->pageHeight() / 2 ? -2 : 0));
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

void QmlItemManager::createStatisticsPage_part2()
{
	m_statisticsPage = static_cast<QQuickItem*>(m_statisticsComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_statisticsPage, QQmlEngine::CppOwnership);
	m_statisticsPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appPagesListModel()->openPage(m_statisticsPage, std::move(tr("Statistics")));
}

inline QmlUserInterface *QmlItemManager::usersManager()
{
	static QmlUserInterface *users_manager{nullptr};
	if (!users_manager)
		users_manager = new QmlUserInterface{this};
	return users_manager;
}
