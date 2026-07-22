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
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>

//Test includes

#ifndef Q_OS_ANDROID
#ifndef QT_NO_DEBUG
//#include "online_services/tpmessagesmanager.h"
#endif
#endif

QmlItemManager *QmlItemManager::_appItemManager{nullptr};
QQmlApplicationEngine *QmlItemManager::_appQmlEngine{nullptr};
QQuickWindow *QmlItemManager::_appMainWindow{nullptr};

enum MESSAGE_ICON {
	MI_None,
	MI_OK,
	MI_Error,
	MI_Warning,
};

struct st_generalMessage {
	int message_id, msecs;
	QString message, image_source, button1text, button2text;
	QFlags<Qt::AlignmentFlag> position;
};

QmlItemManager::QmlItemManager() : QObject{nullptr}
{
	_appItemManager = this;
	REGISTER_QML_SINGLETON(QmlItemManager, this);
}

#include <QThread>
void QmlItemManager::startQmlEngine(QQmlApplicationEngine *qml_engine)
{
	qDebug() << "QmlItemManager::startQmlEngine running on thread: " << thread()->isMainThread();
	_appQmlEngine = qml_engine;
	QQuickStyle::setFallbackStyle("Material"_L1);
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
		} else {
			_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
			appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));
			m_homePage = appMainWindow()->findChild<QQuickItem*>("homePage");
			m_appPagesVisualParent = appMainWindow()->findChild<QQuickItem*>("appStackView");
#ifdef ENABLE_GENERAL_MESSAGES_POPUP
			createGeneralMessagesPopup();
#endif
			appUserModel()->initUserSession();
			connect(appHomePage(), SIGNAL(mesosViewChanged(bool)), this, SLOT(homePageViewChanged(bool)));
#ifndef Q_OS_ANDROID
	#ifndef QT_NO_DEBUG
			if (m_testType & TT_CORE) {
				runTests();
			} else if (m_testType & TT_QML) {
				connect(appUserModel(), &DBUserModel::userLoggedIn, this, [this] (const bool first_checkin) {
					//emit cppDataForQMLReady();
					m_homePage->setProperty("mesoManager", std::move(QVariant::fromValue(appUserModel()->actualMesoModel()->mesoManager(0))));
					/*connect(appMessagesManager(), &TPMessagesManager::TPMessageSent, this, [this] (const int requestid, const bool success) {
						if (requestid == 1111) {
							qDebug() << (success ? "Message sent" : "Message not sent");
							QMetaObject::invokeMethod(_appMainWindow, "openDialog");
						}
					});*/
					//appMessagesManager()->sendTPMessage("1759256421787", appUtils()->string_strings({TPMessagesManager::tpbinarymessage_prefix,
					//	"1759256421787/1759170252407/mesocycles//Hipertrofia 1.txt", "A simple message"}, record_separator), 1111);
				  });
			}
	#endif
#endif
		}
	});

#ifndef Q_OS_ANDROID
	#ifndef QT_NO_DEBUG
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1) {
		if (args.at(1) == "-test"_L1) {
			m_testType |= TT_CORE;
		} else if (args.at(1) == "-testqml"_L1) {
			m_testType |= TT_QML;
			main_module = "Tests";
		} else if (args.at(1) == "-user"_L1) {
			if (!args.at(2).isEmpty()) {
				appSettings()->setReadOnlyGroup(GLOBAL_GROUP, true);
				appSettings()->setCurrentUser(args.at(2));
			} else {
				qDebug() << "Warning: Missing user id in the command line arguments"_L1;
			}
		}
		if (m_testType & TT_CORE && !(m_testType & TT_QML)) { //test with no GUI
			if (runTests())
				::exit(0);
		}
	}
	#endif
#endif
	appQmlEngine()->loadFromModule("TpQml", main_module);
}

void QmlItemManager::exitApp()
{
	qApp->quit();
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
		connect(m_firstTimeDlgComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { showFirstTimeDialog(); });
	} else {
		if (!m_firstTimeDlg) {
			switch (m_firstTimeDlgComponent->status()) {
			case QQmlComponent::Ready:
				m_firstTimeDlgComponent->disconnect();
				m_firstTimeDlg = m_firstTimeDlgComponent->create(appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_firstTimeDlg) {
					qDebug() << m_firstTimeDlgComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_firstTimeDlg, QQmlEngine::CppOwnership);
				m_firstTimeDlg->setProperty("parent", QVariant::fromValue(m_homePage));
				showFirstTimeDialog();
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
	#ifndef QT_NO_DEBUG
				qDebug() << m_firstTimeDlgComponent->errorString();
	#endif
				return;
			}
		} else {
			appPagesManager()->openPopup(m_firstTimeDlg, m_homePage);
		}
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
	if (!m_simpleExercisesListComponent) {
		m_simpleExercisesListComponent = new QQmlComponent{appQmlEngine(), "TpQml.Exercises"_L1, "SimpleExercisesListPanel"_L1,
																							QQmlComponent::Asynchronous};
		connect(m_simpleExercisesListComponent, &QQmlComponent::statusChanged, this, [this,parentPage,filter]
										(QQmlComponent::Status status) { showSimpleExercisesList(parentPage, filter); });

	} else {
		if (!m_simpleExercisesList) {
			switch (m_simpleExercisesListComponent->status()) {
			case QQmlComponent::Ready:
				m_simpleExercisesListComponent->disconnect();
				createSimpleExercisesList(parentPage);
				showSimpleExercisesList(parentPage, filter);
				break;
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				#ifndef QT_NO_DEBUG
				qDebug() << m_simpleExercisesListComponent->errorString();
				#endif
				break;
			}
		} else {
			appExercisesList()->setFilter(filter);
			int name_field_ypos{0};
			QMetaObject::invokeMethod(parentPage, "getExerciseNameFieldYPos", Q_RETURN_ARG(int, name_field_ypos));
			appPagesListModel()->openPopup(m_simpleExercisesList, parentPage,
								name_field_ypos <= appSettings()->pageHeight() / 2 ? Qt::AlignTop : Qt::AlignBaseline);
		}
	}
}

void QmlItemManager::getWeatherPage()
{
	if (!m_weatherComponent) {
		m_weatherComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "WeatherPage"_L1, QQmlComponent::Asynchronous};
		connect(m_weatherComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getWeatherPage(); });
	} else {
		if (!m_weatherPage) {
			switch (m_weatherComponent->status()) {
			case QQmlComponent::Ready:
				m_weatherComponent->disconnect();
				m_weatherPage = static_cast<QQuickItem*>(m_weatherComponent->create(appQmlEngine()->rootContext()));
				appQmlEngine()->setObjectOwnership(m_weatherPage, QQmlEngine::CppOwnership);
				m_weatherPage->setParentItem(appItemManager()->appPagesVisualParent());
				appPagesListModel()->openPage(m_weatherPage, std::move(tr("Weather Forecast")));
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
		} else {
			appPagesListModel()->openPage(m_weatherPage);
		}
	}
}

void QmlItemManager::getStatisticsPage()
{
	if (!m_statisticsPage) {
		m_statisticsComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/TpQml/qml/Pages/StatisticsPage.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_statisticsComponent->status() != QQmlComponent::Ready) {
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

void QmlItemManager::showOnlineMessagesManagerDialog(const bool show)
{
	if (m_messagesManagerPopup) {
		if (show)
			appPagesListModel()->raisePopup(m_messagesManagerPopup);
		else
			appPagesListModel()->hidePopup(m_messagesManagerPopup);
		appSettings()->setShowOnlineMessagesDialog(show);
	}
}

void QmlItemManager::displayWindowMessage(const int message_id, const int msecs, QFlags<Qt::AlignmentFlag> position,
																			const QString &title, const QString &message)
{
	displayMessageOnAppWindow(message_id, std::move(appUtils()->string_strings({title, message}, record_separator)),
																							position, QString{}, msecs);
}

void QmlItemManager::displayMessageOnAppWindow(const int message_id, QString &&message, QFlags<Qt::AlignmentFlag> position,
											   QString &&image_source, const int msecs, QString &&button1text,
											   QString &&button2text) const
{
#ifdef ENABLE_GENERAL_MESSAGES_POPUP
	if (!m_canDisplayMessage) {
		st_generalMessage *g_message{new st_generalMessage};
		g_message->message_id = message_id;
		g_message->message = std::forward<QString>(message);
		g_message->position = position;
		g_message->image_source = std::forward<QString>(image_source);
		g_message->msecs = msecs;
		g_message->button1text = std::forward<QString>(button1text);
		g_message->button2text = std::forward<QString>(button2text);
		const_cast<QmlItemManager*>(this)->m_messagesQueue.append(g_message);
		return;
	}

	QString title;
	MESSAGE_ICON icon_to_use{MI_Error}; //Only applicable when image_source is an empty string
	if (message_id < TP_RET_CODE_CUSTOM_ERROR) {
		icon_to_use = MI_OK;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_SUCCESS:
			title = std::move(appUtils()->getCompositeValue(0, message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, message, record_separator));
			break;
		case TP_RET_CODE_EXPORT_OK:
			title = std::move(tr("Succesfully exported"));
			break;
		case TP_RET_CODE_SHARE_OK:
			title = std::move(tr("Succesfully shared"));
			break;
		case TP_RET_CODE_IMPORT_OK:
			if (message.isEmpty()) { //from QML
				title = std::move(tr("User configuration imported"));
#ifndef Q_OS_ANDROID
				message = std::move(tr("Click on Next to start using the app"));
#else
				message = std::move(tr("Tap on Next to start using the app"));
#endif
			} else {
				title = std::move(tr("Successfully imported"));
			}
			break;
		case TP_RET_CODE_USER_OK:
			title = std::move(tr("Existing user account found"));
			if (message.isEmpty()) {
#ifndef Q_OS_ANDROID
				message = std::move(tr("You can click on the Import button to download all the data for the user"));
#else
				message = std::move(tr("You can tap on the Import button to download all the data for the user"));
#endif
			}
			break;
		}
	} else if (message_id < TP_RET_CODE_CUSTOM_WARNING) {
		icon_to_use = MI_Error;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_ERROR:
			title = std::move(appUtils()->getCompositeValue(0, message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, message, record_separator));
			break;
		case TP_RET_CODE_UNKNOWN_ERROR:
			title = std::move(tr("Unknown Error"));
			break;
		case TP_RET_CODE_FILE_NOT_FOUND:
			title = std::move(tr("File not found!"));
			break;
		case TP_RET_CODE_OPEN_READ_FAILED:
			title = std::move(tr("Failed to open file"));
			break;
		case TP_RET_CODE_WRONG_IMPORT_FILE_TYPE:
			title = std::move(tr("Error"));
			if (message.isEmpty())
				message = std::move(tr("File type not recognized"));
			break;
		case TP_RET_CODE_CORRUPT_FILE:
			title = std::move(tr("Error! File format not recognized"));
			break;
		case TP_RET_CODE_SHARE_FAILED:
			title = std::move(tr("Sharing failed"));
			break;
		case TP_RET_CODE_EXPORT_FAILED:
			title = std::move(tr("Export failed"));
			break;
		case TP_RET_CODE_IMPORT_FAILED:
			if (message.isEmpty()) { //from QML
				title = std::move(tr("User data not imported"));
				message = std::move(tr("Could not retrieve the data from the server"));
			} else {
				title = std::move(tr("Import from file failed"));
			}
			break;
		case TP_RET_CODE_OPEN_CREATE_FAILED:
			title = std::move(tr("Could not open file for exporting"));
			break;
		case TP_RET_CODE_USER_DOES_NOT_EXIST:
			title = std::move(tr("User account not found"));
			break;
		case TP_RET_CODE_SERVER_UNREACHABLE:
			title = std::move(tr("Can't connect to server"));
			break;
		}
	} else if (message_id < TP_RET_CODE_CUSTOM_MESSAGE) {
		icon_to_use = MI_Warning;
		switch (message_id) {
		case TP_RET_CODE_CUSTOM_WARNING:
			title = std::move(tr("Warning! ") % appUtils()->getCompositeValue(0, message, record_separator));
			message = std::move(appUtils()->getCompositeValue(1, message, record_separator));
			break;
		case TP_RET_CODE_NOTHING_TODO:
			title = std::move(tr("Nothing to be done"));
			message += std::move(tr("File had already been imported"));
			break;
		case TP_RET_CODE_NO_MESO:
			title = std::move(tr("No program to import into"));
			message += std::move(tr("Either create a new training plan or import from a complete program file"));
			break;
		case TP_RET_CODE_NOTHING_TO_EXPORT:
			title = std::move(tr("Nothing to export"));
			message += std::move(tr("Only exercises that do not come by default with the app can be exported"));
			break;
		case TP_RET_CODE_OPERATION_CANCELED:
			title = std::move(tr("Warning"));
			message += std::move(tr("Operation canceled"));
			break;
		}
	} else {
		icon_to_use = MI_None;
		title = std::move(appUtils()->getCompositeValue(0, message, record_separator));
		message = std::move(appUtils()->getCompositeValue(1, message, record_separator));
	}

	QString img_src;
	if (image_source.isEmpty()) {
		switch (icon_to_use) {
		case MI_Error:		img_src = std::move("error");			break;
		case MI_Warning:	img_src = std::move("warning");			break;
		case MI_OK:			img_src = std::move("set-completed");	break;
		case MI_None:												break;
		}
	} else {
		img_src = image_source;
	}

	m_generalMessagesPopup->setProperty("show_position", std::move(QVariant{position}));
	m_generalMessagesPopup->setProperty("title", std::move(QVariant{title}));
	m_generalMessagesPopup->setProperty("message", std::move(QVariant{message}));
	m_generalMessagesPopup->setProperty("imageSource", std::move(QVariant{img_src}));
	if (!button1text.isEmpty()) {
		m_generalMessagesPopup->setProperty("button1Text", std::move(QVariant{button1text}));
		connect(m_generalMessagesPopup, SIGNAL(button1Clicked()), this, SLOT(generalMessagesButton1Clicked()), Qt::UniqueConnection);
	}
	if (!button2text.isEmpty()) {
		m_generalMessagesPopup->setProperty("button2Text", std::move(QVariant{button2text}));
		connect(m_generalMessagesPopup, SIGNAL(button2Clicked()), this, SLOT(generalMessagesButton2Clicked()), Qt::UniqueConnection);
	}
	if (msecs == 0)
		QMetaObject::invokeMethod(m_generalMessagesPopup, "tpOpen");
	else
		QMetaObject::invokeMethod(m_generalMessagesPopup, "showTimed", Q_ARG(int, msecs));
#endif //ENABLE_GENERAL_MESSAGES_POPUP
}

void QmlItemManager::startMessagesManager()
{
	if (!m_messagesManagerComponent) {
		m_messagesManagerComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "OnlineMessages"_L1, QQmlComponent::Asynchronous};
		connect(m_messagesManagerComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { startMessagesManager(); });
	} else {
		if (!m_messagesManagerPopup) {
			switch (m_messagesManagerComponent->status()) {
			case QQmlComponent::Ready:
				m_messagesManagerComponent->disconnect();
				m_messagesManagerPopup = m_messagesManagerComponent->create(appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				m_messagesManagerPopup->setProperty("objectName", std::move(QVariant{"onlineMessages"}));
				if (!m_messagesManagerPopup) {
					qDebug() << m_messagesManagerComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_messagesManagerPopup, QQmlEngine::CppOwnership);
				m_messagesManagerPopup->setProperty("parent", QVariant::fromValue(m_homePage));
				startMessagesManager();
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_messagesManagerComponent->errorString();
#endif
				return;
			}
		} else {
			appPagesManager()->openPopup(m_messagesManagerPopup, m_homePage, Qt::AlignBaseline);
		}
	}
}

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::homePageViewChanged(const bool own_mesos_view)
{
	appUserModel()->actualMesoModel()->setCurrentMesosView(own_mesos_view);
}

void QmlItemManager::generalMessagesPopupClosed()
{
	m_canDisplayMessage = m_messagesQueue.isEmpty();
	if (!m_canDisplayMessage) {
		st_generalMessage *g_message{m_messagesQueue.first()};
		if (g_message) {
			m_canDisplayMessage = true;
			displayMessageOnAppWindow(g_message->message_id, std::move(g_message->message), g_message->position,
										std::move(g_message->image_source), g_message->msecs,
										std::move(g_message->button1text), std::move(g_message->button2text));
			m_canDisplayMessage = false;
			delete g_message;
			m_messagesQueue.removeFirst();
		}
	}
}

#ifndef Q_OS_ANDROID
#ifndef QT_NO_DEBUG
//Return: true for exiting the app upon return; false for letting some other function call ::exit() when appropriate
bool QmlItemManager::runTests()
{
	return true;
}
#endif
#endif

void QmlItemManager::createGeneralMessagesPopup()
{
	if (!m_generalMessagesPopupComponent) {
#ifndef QT_NO_DEBUG
		m_generalMessagesPopupProperties["objectName"] = std::move(QVariant{"generalMessages"});
#endif
		m_generalMessagesPopupProperties["button1Text"] = std::move(QVariant{QString{}});
		m_generalMessagesPopupProperties["button2Text"] = std::move(QVariant{QString{}});
		m_generalMessagesPopupComponent = new QQmlComponent{appQmlEngine(), "TpQml.Widgets"_L1, "TPBalloonTip"_L1,
																							QQmlComponent::Asynchronous};
		connect(m_generalMessagesPopupComponent, &QQmlComponent::statusChanged, this, [this]
																					(QQmlComponent::Status status) {
			createGeneralMessagesPopup();
		});
	} else {
		if (!m_generalMessagesPopup) {
			switch (m_generalMessagesPopupComponent->status()) {
			case QQmlComponent::Ready:
				m_generalMessagesPopupComponent->disconnect();
				m_generalMessagesPopup = m_generalMessagesPopupComponent->createWithInitialProperties(
														m_generalMessagesPopupProperties, appQmlEngine()->rootContext());
				appQmlEngine()->setObjectOwnership(m_generalMessagesPopup, QQmlEngine::CppOwnership);
				m_generalMessagesPopup->setProperty("parent", std::move(QVariant::fromValue(m_homePage)));
				connect(m_generalMessagesPopup, SIGNAL(popupClosed(QObject*)), this, SLOT(generalMessagesNoButtonClicked(QObject*)));
				connect(m_generalMessagesPopup, SIGNAL(closeActionExeced()), this, SLOT(generalMessagesPopupClosed()));
				generalMessagesPopupClosed();
				break;
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_generalMessagesPopupComponent->errorString();
#endif
				break;
			}
		}
	}
}

void QmlItemManager::createSimpleExercisesList(QQuickItem *parentPage)
{
	m_simpleExercisesList = m_simpleExercisesListComponent->createWithInitialProperties(m_simpleExercisesListProperties,
																							appQmlEngine()->rootContext());
	appQmlEngine()->setObjectOwnership(m_simpleExercisesList, QQmlEngine::CppOwnership);
	m_simpleExercisesList->setProperty("parent", QVariant::fromValue(parentPage));
	connect(m_simpleExercisesList, SIGNAL(exerciseSelected(QQuickItem*)), this, SIGNAL(selectedExerciseFromSimpleExercisesList(QQuickItem*)));
}

void QmlItemManager::createStatisticsPage_part2()
{
	m_statisticsPage = static_cast<QQuickItem*>(m_statisticsComponent->create(appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_statisticsPage, QQmlEngine::CppOwnership);
	m_statisticsPage->setParentItem(appItemManager()->appPagesVisualParent());
	appPagesListModel()->openPage(m_statisticsPage, std::move(tr("Statistics")));
}

inline QmlUserInterface *QmlItemManager::usersManager()
{
	static QmlUserInterface *users_manager{nullptr};
	if (!users_manager)
		users_manager = new QmlUserInterface{this};
	return users_manager;
}
