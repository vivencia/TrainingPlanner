#include "osinterface.h"

#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "online_services/scan_network.h"
#include "online_services/tponlineservices.h"

#ifdef Q_OS_ANDROID
#include "dbmesocyclesmodel.h"
#include "tpandroidnotification.h"

#include <QJniObject>
#include <qnativeinterface.h>
#include <QtCore/6.9.1/QtCore/private/qandroidextras_p.h>

//"(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;"
// String f(Context, Uri)

#define NOTIFY_DO_NOTHING 0xA
#define MESOCYCLE_NOTIFICATION 0x14
#define SPLIT_NOTIFICATION 0x1E
#define CALENDAR_NOTIFICATION 0x28
#define NOTIFY_START_WORKOUT 0x32
#define USER_NOTIFICATION 0x3C

#endif //Q_OS_ANDROID

#ifdef LOCAL_TPSERVER
#include <QNetworkInterface>
#ifdef TPSERVER_MACHINE
#include <QHash>
#include <QProcess>

extern "C"
{
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <linux/if.h>
	#include <netinet/ether.h>
	#include <netdb.h>
}

static const QString &tp_server_config_script{"/var/www/html/trainingplanner/scripts/init_script.sh"_L1};

enum procExitCodes {
	TPSERVER_OK,
	TPSERVER_ERROR,
	TPSERVER_NGINX_ERROR,
	TPSERVER_PHPFPM_ERROR,
	TPSERVER_CONFIG_ERROR,
	TPSERVER_OK_LOCALHOST,
	TPSERVER_PAUSED,
	TPSERVER_PAUSED_LOCALHOST,
	TPSERVER_PAUSED_FAILED,
};

#endif //TPSERVER_MACHINE
#endif //LOCAL_TPSERVER

#include <QFileInfo>
#include <QGuiApplication>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QTcpSocket>
#include <QTimer>

OSInterface *OSInterface::_app_os_interface{nullptr};

#ifdef LOCAL_TPSERVER
constexpr int CONNECTION_CHECK_TIMEOUT{5*60*1000};
constexpr int CONNECTION_ERR_TIMEOUT{10*1000};
#else
constexpr int CONNECTION_CHECK_TIMEOUT{10*60*1000};
constexpr int CONNECTION_ERR_TIMEOUT{20*1000};
#endif

enum connectMessagesIndex {
	internetMessage,
	serverMessage,
	interfaceMessage,
};

OSInterface::OSInterface(QObject *parent) : QObject{parent}
{
	_app_os_interface = this;
	REGISTER_QML_SINGLETON(OSInterface, this);
	m_connectionMessages.resize(3);
	checkInternetConnection();
	connect(qApp, &QCoreApplication::aboutToQuit, this, [this] () {
		appOnlineServices()->userLogout(111111);
	});
	connect(appOnlineServices(), &TPOnlineServices::serverStatusChanged, this, [this]
									(const uint online_status, const QString &server_address, const int request_id) {
		localServerProcessResult(online_status);
	}, Qt::QueuedConnection);
#ifdef Q_OS_ANDROID
	initAndroidInterface();
#endif
}

void OSInterface::checkInternetConnection()
{
	bool is_connected{false};
	QTcpSocket checkConnectionSocket;
	checkConnectionSocket.connectToHost("google.com"_L1, 443); // 443 for HTTPS or use Port 80 for HTTP
	checkConnectionSocket.waitForConnected(2000);
	is_connected = checkConnectionSocket.state() == QTcpSocket::ConnectedState;
	checkConnectionSocket.close();
	if (!m_currentNetworkStatus[internetMessage].has_value() || m_currentNetworkStatus[internetMessage].value() != is_connected) {
		setNetStatus(internetMessage, is_connected, std::move(is_connected ?
								tr("Device is connected to the internet") : tr("Device is not connected to the internet")));
		emit internetStatusChanged();
	}

#ifndef LOCAL_TPSERVER //TODO
	if (is_connected)
		checkServer(remote_server_address, remove_server_port);
#endif
}

#ifdef Q_OS_ANDROID
void OSInterface::initAndroidInterface()
{
	m_workoutDoneMessage = std::move(tr("Your training routine seems to go well. Workout for the day is concluded"));
	const QJniObject &context{QNativeInterface::QAndroidApplication::context()};

	context.callStaticMethod<void>(
		"org/vivenciasoftware/TrainingPlanner/QShareUtils",
		"setActivityContext",
		"(Landroid/content/Context;)V",
		context.object());

	context.callStaticMethod<void>(
		"org/vivenciasoftware/TrainingPlanner/NotificationClient",
		"setActivityContext",
		"(Landroid/content/Context;)V",
		context.object());

	context.callStaticMethod<void>(
		"org/vivenciasoftware/TrainingPlanner/TPService",
		"startTPService",
		"(Landroid/content/Context;)V",
		context.object());

	/*context.callStaticObjectMethod(
		"org/vivenciasoftware/TrainingPlanner/NotificationClient",
		"testNetworkConnection",
		"(Ljava/lang/String;)Ljava/lang/String;",
		QJniObject::fromString("https://www.google.com").object<jstring>()
	);*/
	//qDebug() << "SSL supported:" << QSslSocket::supportsSsl();
	//qDebug() << "SSL version:" << QSslSocket::sslLibraryVersionString();
	mb_appSuspended = false;
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI weren't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(this, &OSInterface::appResumed, this, &OSInterface::checkPendingIntents);
	connect(this, &OSInterface::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		int message_id(0);
		switch (resultCode) {
		case -1: message_id = TP_RET_CODE_SUCCESS; break;
		case 0: message_id = TP_RET_CODE_SHARE_FAILED; break;
		default: message_id = TP_RET_CODE_UNKNOWN_ERROR; break;
		}
		qDebug() << "Activity result: request code = " << requestCode << ", result code = " << resultCode;
	});

	connect(qApp, &QGuiApplication::applicationStateChanged, this, [&] (Qt::ApplicationState state) {
		if (state == Qt::ApplicationSuspended) {
			mb_appSuspended = true;
			emit appSuspended();
		}
		else if (state == Qt::ApplicationActive) {
			if (mb_appSuspended) {
				emit appResumed();
				mb_appSuspended = false;
			}
		}
	});

	m_AndroidNotification = new TPAndroidNotification{this};
}

void OSInterface::setFileUrlReceived(const QString &url) const
{
	QString androidUrl{std::move(appUtils()->getCorrectPath(url))};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_FILE_NOT_FOUND, std::move(androidUrl));
}

void OSInterface::setFileReceivedAndSaved(const QString &url) const
{
	QString androidUrl{std::move(appUtils()->getCorrectPath(url))};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_FILE_NOT_FOUND, std::move(androidUrl));
}

void OSInterface::onActivityResult(int requestCode, int resultCode)
{
	#ifndef QT_NO_DEBUG
	// we're getting RESULT_OK only if edit is done
	if (resultCode == -1)
		qDebug() << "OSInterface::onActivityResult() -> Send Activity Result OK"_L1;
	else if (resultCode == 0)
		qDebug() << "OSInterface::onActivityResult() -> Send Activity Result Canceled"_L1;
	else
		qDebug() << "OSInterface::onActivityResult() -> Send Activity wrong result code: "_L1 <<
															resultCode << " from request: "_L1 << requestCode;
	#endif
	emit activityFinishedResult(requestCode, resultCode);
}

void OSInterface::execNotification(const short action, const short id)
{
	for (qsizetype i{0}; i < m_notifications.count(); ++i) {
		if (m_notifications.at(i)->id == id && !m_notifications.at(i)->resolved) {
			switch (action) {
			case NOTIFY_DO_NOTHING:
				m_notifications.at(i)->resolved = true;
				break;
			case NOTIFY_START_WORKOUT:
				appUserModel()->actualMesoModel()->startTodaysWorkout();
				m_notifications.at(i)->resolved = true;
				break;
			}
		}
	}
}

void OSInterface::removeNotification(notificationData *data)
{
	m_AndroidNotification->cancelNotification(data->id);
	if (data->action == NOTIFY_START_WORKOUT) {
		if (data->resolved) { //Send a new notification with an innocuous greeting message.
			data->resolved = false;
			data->message = m_workoutDoneMessage;
			data->action = NOTIFY_DO_NOTHING;
			m_AndroidNotification->sendNotification(data);
			return;
		}
	}
	m_notifications.removeOne(data);
	delete data;
}

void OSInterface::checkPendingIntents() const
{
	const QJniObject &activity{QNativeInterface::QAndroidApplication::context()};
	if (activity.isValid()) {

		activity.callStaticMethod<void>(
			"org/vivenciasoftware/TrainingPlanner/TPActivity",
			"checkPendingIntents",
			"()V",
			activity.object());
		return;
	}
#ifndef QT_NO_DEBUG
	qDebug() << "OSInterface::checkPendingIntents() -> Activity not valid"_L1;
#endif
}

/*
  *As default we're going the Java - way with one simple JNI call (recommended)
  *if altImpl is true we're going the pure JNI way
  *HINT: we don't use altImpl anymore
 *
  *If a requestId was set we want to get the Activity Result back (recommended)
  *We need the Request Id and Result Id to control our workflow
*/
bool OSInterface::shareFile(const QString &filePath, const int requestId, const QString &title, const QString &mimeType) const
{
	/*setExportFileName("app_logo.png");
	if (!QFile::exists(exportFileName())) {
		QFile::copy(":/images/app_logo.png", exportFileName());
		QFile::setPermissions(exportFileName(), QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
	}
	sendFile(exportFileName(), tr("Send file"), u"image/png"_s, 10);*/

	const QJniObject &jsPath{QJniObject::fromString(filePath)};
	const QJniObject &jsTitle{QJniObject::fromString(title.isEmpty() ? tr("Send file") : title )};
	const QJniObject &jsMimeType{QJniObject::fromString(mimeType.isEmpty() ? "text/plain"_L1 : mimeType)};
	const jboolean ok{QJniObject::callStaticMethod<jboolean>(
		"org/vivenciasoftware/TrainingPlanner/QShareUtils",
		"sendFile",
		"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
		jsPath.object<jstring>(), jsTitle.object<jstring>(), jsMimeType.object<jstring>(), requestId)};

#ifndef QT_NO_DEBUG
	if (!ok)
		qDebug() << "OSInterface::OSInterface::sendFile() -> Unable to resolve activity from Java"_L1;
#endif
	return ok;
}

void OSInterface::openURL(const QString &address) const
{
	if (!address.isEmpty()) {
		QString url;
		if (!address.startsWith("http"_L1))
			url = std::move("https://"_L1 % address);
		else
			url = address;

		const QJniObject &jsPath{QJniObject::fromString(url)};
		const jboolean ok{QJniObject::callStaticMethod<jboolean>(
			"org/vivenciasoftware/TrainingPlanner/QShareUtils",
			"openURL",
			"(Ljava/lang/String;)Z",
			jsPath.object<jstring>())};
#ifndef QT_NO_DEBUG
		if (!ok)
			qDebug() << "OSInterface::OSInterface::androidOpenURL() -> Unable to open the address: "_L1 << address;
#endif
	}
}

bool OSInterface::sendMail(const QString &address, const QString &subject, const QString &attachment) const
{
	const QString &attachment_file{attachment.isEmpty() ? QString() : "file://"_L1 + attachment};
	const QJniObject &jsAddress{QJniObject::fromString(address)};
	const QJniObject &jsSubject{QJniObject::fromString(subject)};
	const QJniObject &jsAttach{QJniObject::fromString(attachment_file)};
	const jboolean ok{QJniObject::callStaticMethod<jboolean>(
		"org/vivenciasoftware/TrainingPlanner/QShareUtils",
		"sendEmail",
		"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
		jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>())};

	if (ok && appUserModel()->email(0).contains("gmail.com"_L1)) {
		const QString &gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(appUserModel()->email(0), address, subject));
		openURL(gmailURL);
	}
	return ok;
}

bool OSInterface::viewExternalFile(const QString &filePath) const
{
	const QString &filename{appUtils()->getCorrectPath(filePath)};
	if (!appUtils()->canReadFile(filename))
		return false;
	const QString &localFile{appSettings()->localAppFilesDir() + "tempfile"_L1 + filename.last(4)};
	static_cast<void>(QFile::remove(localFile));
	if (!appUtils()->copyFile(filename, localFile)) {
		qDebug() << "could not copy:  " << filename << "    to   " << localFile;
		return false;
	}

	const QJniObject &jsPath{QJniObject::fromString(localFile)};
	const QJniObject &jsTitle{QJniObject::fromString(tr("View file with..."))};
	const jboolean ok{QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
															 "viewFile",
															 "(Ljava/lang/String;Ljava/lang/String;)Z",
															 jsPath.object<jstring>(), jsTitle.object<jstring>())};
#ifndef QT_NO_DEBUG
	if (!ok)
		qDebug() << "OSInterface::OSInterface::androidOpenURL() -> Unable to resolve view activity from Java"_L1;
#endif
	return ok;
}

QString OSInterface::readFileFromAndroidFileDialog(const QString &android_uri) const
{
	if (android_uri.startsWith("//com"_L1))
	{
		const QString &properFilename{"content:"_L1 + android_uri};
		const QString &localFile{appSettings()->localAppFilesDir() + "tempfile"_L1};
		static_cast<void>(QFile::remove(localFile));
		return appUtils()->copyFile(properFilename, localFile) ? properFilename : QString();
	}
	// else: uri is not a uri it's already been translated by QShareUtils via android's open with or share
	return android_uri;
}

void OSInterface::startAppNotifications()
{
	m_notificationsTimer = new QTimer{this};
	m_notificationsTimer->setInterval(30*60*1000); //every 30min
	m_notificationsTimer->callOnTimeout([this] () { checkNotificationsStatus(); } );
	m_notificationsTimer->start();
	m_bTodaysWorkoutFinishedConnected = false;
	checkWorkouts();
}

void OSInterface::checkNotificationsStatus()
{
	const QDateTime now{std::move(QDateTime::currentDateTime())};
	short action;
	for (auto i{m_notifications.count()-1}; i >= 0; --i) {
		if (m_notifications.at(i)->expiration == now) {
			action = m_notifications.at(i)->action;
			removeNotification(m_notifications.at(i));
			switch (action) {
			case NOTIFY_DO_NOTHING:
				break;
			case NOTIFY_START_WORKOUT:
				//Workout for the -previous- day was not concluded(neither was the notification activated). Start a new one for the day
				checkWorkouts();
				break;
			}
		}
	}
}

void OSInterface::checkWorkouts()
{
	/*if (appMesoModel()->count() > 0) {
		DBMesoCalendarTable *calTable{new DBMesoCalendarTable{appThreadManager()->dbFilesPath()}};
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty()) {
			notificationData *data{new notificationData{}};
			data->title = std::move("TrainingPlanner "_L1) + data->start_time.toString("dd/MM - hh:mm"_L1);
			const QString &splitLetter{dayInfoList.at(2)};
			if (splitLetter != "R"_L1) //day is training day {
				if (dayInfoList.at(3) == '1') //day is completed {
					data->message = m_workoutDoneMessage;
					data->action = NOTIFY_DO_NOTHING;
				}
				else {
					data->message = std::move(tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter);
					data->action = NOTIFY_START_WORKOUT;
					if (!m_bTodaysWorkoutFinishedConnected) {
						connect(appMesoModel(), &DBMesocyclesModel::todaysWorkoutFinished, this, [this,data] () {
							data->resolved = true;
							removeNotification(data);
						});
						m_bTodaysWorkoutFinishedConnected = true;
					}
				}
			}
			else {
				data->message = std::move(tr("Enjoy your day of rest from workouts!"));
				data->action = NOTIFY_DO_NOTHING;
			}
			data->expiration = std::move(QDateTime(QDate::currentDate(), QTime(23, 59, 59)));
			m_notifications.append(data);
			m_AndroidNotification->sendNotification(data);
		}
		delete calTable;
	}*/
}

extern "C"
{

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileUrlReceived(JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	appOsInterface()->setFileUrlReceived(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileReceivedAndSaved(JNIEnv *env,
																									jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	appOsInterface()->setFileReceivedAndSaved(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_fireActivityResult(JNIEnv *env, jobject obj,
																							   jint requestCode, jint resultCode)
{
	Q_UNUSED (obj)
	Q_UNUSED (env)
	appOsInterface()->onActivityResult(requestCode, resultCode);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_notificationActionReceived(JNIEnv *env,
																									   jobject obj, jint action, jint id)
{
	Q_UNUSED (obj)
	//const char *actionStr = env->GetStringUTFChars(action, NULL);
	appOsInterface()->execNotification(static_cast<short>(action), static_cast<short>(id));
	//env->ReleaseStringUTFChars(action, actionStr);
	return;
}
} //extern "C"

#endif //Q_OS_ANDROID

#ifndef Q_OS_ANDROID
#ifdef Q_OS_LINUX
void OSInterface::restartApp()
{
	char *args[2] = {nullptr, nullptr};
	const QString &argv0{qApp->arguments().at(0)};
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}

void OSInterface::openURL(const QString &address) const
{
	if (!address.isEmpty()) {
		auto *__restrict proc{new QProcess{}};
		proc->startDetached("xdg-open"_L1, {address});
		delete proc;
	}
}

void OSInterface::sendMail(const QString &address, const QString &subject, const QString &attachment_file) const
{
	const QStringList &args{QStringList{6} << std::move("--utf8"_L1) << std::move("--subject"_L1) <<
							std::move(QChar{'\''} % subject % QChar{'\''}) << std::move("--attach"_L1) << attachment_file <<
							std::move(QChar{'\''} % address % QChar{'\''})};
	auto *__restrict proc{new QProcess};
	proc->start("xdg-email"_L1, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exit_code, QProcess::ExitStatus) {
		if (exit_code != 0) {
			if (appUserModel()->email(0).contains("gmail.com"_L1)) {
				const QString &gmailURL{u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(
					appUserModel()->email(0), address, subject)};
				openURL(gmailURL);
			}
		}
		proc->deleteLater();
	});
}

void OSInterface::viewExternalFile(const QString &filename) const
{
	const QString &_filename{appUtils()->getCorrectPath(filename)};
	if (!appUtils()->canReadFile(_filename))
		return;
	openURL(_filename);
}
#endif //Q_OS_LINUX
#endif //Q_OS_ANDROID

QString OSInterface::deviceID() const
{
	return QSysInfo::machineUniqueId();
}

void OSInterface::startMessagingApp(const QString &phone, const QString &appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	for (const auto &it: phone) {
		if (it.isDigit())
			phoneNumbers += it;
	}
	QString address;
	if (appname.contains("Whats"_L1))
		address = std::move("https://wa.me/"_L1 % phoneNumbers);
	else
		address = std::move("https://t.me/+"_L1 % phoneNumbers);
	openURL(address);
}

void OSInterface::setNetStatus(uint messages_index, bool success, QString &&message)
{
	short on_bit{0}, off_bit{0};
	switch (messages_index) {
	case interfaceMessage:
		on_bit = success ? HAS_INTERFACE : NO_INTERFACE_RUNNING;
		off_bit = success ? NO_INTERFACE_RUNNING : HAS_INTERFACE;
		break;
	case internetMessage:
		on_bit = success ? HAS_INTERNET : NO_INTERNET_ACCESS;
		off_bit = success ? NO_INTERNET_ACCESS : HAS_INTERNET;
		break;
	case serverMessage:
		on_bit = success ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE;
		off_bit = success ? SERVER_UNREACHABLE : SERVER_UP_AND_RUNNING;
		break;
	}
	setBit(m_networkStatus, on_bit);
	unSetBit(m_networkStatus, off_bit);
	m_currentNetworkStatus[messages_index] = success;
	m_connectionMessages[messages_index] = std::forward<QString>(message);
	emit connectionStatusChanged();
	appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(appUtils()->string_strings(
			{QString{}, m_connectionMessages.at(messages_index)}, record_separator)), Qt::AlignTop|Qt::AlignHCenter,
																std::move(success ? "set-completed"_L1 : "error"_L1));
}

void OSInterface::localServerProcessResult(const uint online_status, const QString &additional_message)
{
	const bool online{online_status == TP_RET_CODE_SUCCESS};
	if (!m_currentNetworkStatus[serverMessage].has_value() || m_currentNetworkStatus[serverMessage].value() != online) {
		QString message{online ? tr("Connected to server ") : tr("Server unreachable")};
		if (online)
			message += std::move('(' + appSettings()->serverAddress() % ':' % appSettings()->serverPort() % ')' % additional_message);
		else
			message += additional_message;
		setNetStatus(serverMessage, online, std::move(message));
	}
}

#ifdef LOCAL_TPSERVER
void OSInterface::getAvailableAddresses()
{
	QHash<int,QString> interface_addresses;
	const QList<QNetworkInterface> &interfaces{QNetworkInterface::allInterfaces()};
	for (const auto &interface : interfaces) {
		if (interface.isValid() && interface.flags() & QNetworkInterface::IsRunning) {
			const QList<QNetworkAddressEntry> &addresses{interface.addressEntries()};
			for (const auto &address : addresses) {
				if (!address.ip().isNull() && tpScanNetwork::ping(address.ip().toString())) {
					if (interface.type() != QNetworkInterface::Loopback)
						interface_addresses.insert(interface.index(), address.ip().toString() % ':' % appSettings()->serverPort());
				}
			}
		}
	}
	emit serverAddressesFetched(interface_addresses);
}

void OSInterface::setWorkingNetInterface(const int interface_index)
{
	QString message{tr("Network interface: ")};
	if (interface_index != 0) {
		bool correct_interface{false};
		const QList<QNetworkInterface> &interfaces{QNetworkInterface::allInterfaces()};
		for (const auto &interface : interfaces) {
			correct_interface = interface.index() == interface_index;
			if (!correct_interface) {
				const QList<QNetworkAddressEntry> &addresses{interface.addressEntries()};
				for (const auto &address : addresses) {
					if ((correct_interface = !address.ip().isNull() && address.ip().toString() == appSettings()->serverAddress()))
						break;
				}
			}
			if (correct_interface) {
				switch (interface.type()) {
				case QNetworkInterface::Loopback:	message += "Loopback"_L1;	break;
				case QNetworkInterface::Virtual:	message += "Virtual"_L1;	break;
				case QNetworkInterface::Ethernet:	message += "Ethernet"_L1;	break;
				case QNetworkInterface::Wifi:		message += "WiFi"_L1;		break;
				default:							message += "Unknown"_L1;	break;
				}
				message += '(' % interface.name() % "@ "_L1 % appSettings()->serverAddress() % ':' % appSettings()->serverPort();
			}
		}
	} else {
		switch (appOnlineServices()->serverStatus()) {
		case TP_RET_CODE_SERVER_NOT_RUNNING:
#ifdef LOCAL_TPSERVER
#ifdef TPSERVER_MACHINE
			return; //nginx is not yet initialized. Ignore serverStatus for now
#else
			if (internetOK()) {//probably nginx is not yet initialized, just print a message on console for debugging purposes
				qDebug() << "Querying the server returned Bad Gateway, which might indicate that nginx is not running,"
							"since we have internet"_L1;
				return;
			} else {
				message += tr("Error: cannot reach the TP Server because we don't have internet access");
			}
#endif
#else
			if (internetOK())
				message += tr("Error: The TP Server is unreachable at the moment");
			else
				message += tr("Error: cannot reach the TP Server because we don't have internet access");
#endif
			break;
		case TP_RET_CODE_SERVER_PAUSED:
			message += tr("Error: The TP Server is currently under maintenance");
			break;
		case TP_RET_CODE_SERVER_UNREACHABLE: //Some other error. Report only on the console if there is internet
			if (internetOK())
				qDebug() << "Could not communicate with the server. Unkown error."_L1;
			return;
		}
	}
	setNetStatus(interfaceMessage, interface_index > 0, std::move(message));
}

#ifdef TPSERVER_MACHINE
void OSInterface::startLocalServerProcess()
{
	if (!m_severScriptProc) {
		m_severScriptProc = new QProcess{this};
		connect(m_severScriptProc, &QProcess::finished, this, [this] (int exit_code, QProcess::ExitStatus exit_status) {
			if (exit_status != QProcess::NormalExit) {
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
					appUtils()->string_strings({"Linux TP Server"_L1, "Error executing init_script("_L1
													% QString::number(exit_code) % ')'}, record_separator)));
			} else {
				serverProcessFinished(m_severScriptProc, exit_code);
			}
			m_severScriptProc->close();
			m_commandQueue.removeFirst();
			if (!m_commandQueue.isEmpty()) {
				startLocalServerProcess();
			} else {
				delete m_severScriptProc;
				m_severScriptProc = nullptr;
			}
		});
	} else {
		if (m_severScriptProc->state() != QProcess::NotRunning)
			return;
	}
	m_severScriptProc->start(tp_server_config_script, m_commandQueue.constFirst(), QIODeviceBase::ReadOnly);
}

void OSInterface::serverProcessFinished(QProcess *proc, const int exit_code)
{
	switch (exit_code) {
	case TPSERVER_ERROR:
	case TPSERVER_PAUSED_FAILED:
		localServerProcessResult(TP_RET_CODE_SERVER_UNREACHABLE, proc->readAllStandardOutput()
															% "\nReturn code("_L1 % QUOTE(exit_code) % ')');
		break;
	case TPSERVER_NGINX_ERROR:
		commandLocalServer("start"_L1, true, "Start server service?"_L1);
		break;
	case TPSERVER_PHPFPM_ERROR:
		commandLocalServer("restart"_L1, true, "Restart server service?"_L1);
		break;
	case TPSERVER_CONFIG_ERROR:
		commandLocalServer("setup"_L1, true, "Setup server?"_L1);
		break;
	case TPSERVER_PAUSED:
	case TPSERVER_PAUSED_LOCALHOST:
		commandLocalServer("pause"_L1, true, "Unpause server?"_L1);
		break;
	default: { //TPSERVER_OK or TPSERVER_OK_LOCALHOST
		QString address{std::move(proc->readAllStandardOutput())};
		const auto address_start{address.indexOf('(') + 1};
		const auto port_end{address.indexOf(')', address_start + 1) - 1};
		address.slice(address_start, port_end - address_start + 1);
		emit serverAddressesFetched(QHash<int,QString>{std::pair<int,QString>{-1, address}});
		}
		break;
	}
}

OSInterface::clsRetCode OSInterface::commandLocalServer(const QString &command, const bool as_su,
																					const QString &title)
{
	if (std::find_if(m_commandQueue.cbegin(), m_commandQueue.cend(), [command] (const QStringList &args) {
			return command == args.constFirst();
	}) != m_commandQueue.cend())
		return CLS_ERROR_ALREADY_QUEUED;
	if (as_su) {
		static bool waiting_for_password{false};
		QLatin1StringView seed{command.toLatin1()};
		const int requestid{appUtils()->generateUniqueId(seed)};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appItemManager(), &QmlItemManager::passwordAcquired, this, [=,this]
								(const bool proceed, const int request_id, const QString &passwd) mutable {
			qDebug() << "######  OSInterface::passwordAcquired, proceed = " << proceed << ", request_id = "
					 << request_id << ", requestid = " << requestid << ", passwd = " << passwd;
			if (request_id == requestid) {
				disconnect(*conn);
				if (proceed) {
					m_commandQueue.append({command, "-p="_L1 % passwd});
					startLocalServerProcess();
				} else {
					appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, std::move(
					appUtils()->string_strings({title, "Operation canceled by the user"_L1}, record_separator)));
				}
				waiting_for_password = false;
			}
		});
		if (!waiting_for_password) {
			waiting_for_password = true;
			appItemManager()->showPasswordDialog(requestid, appItemManager()->appHomePage(), title,
													"Your system user password is required"_L1);
			return CLS_OK_WAITING_FOR_PASSWORD;
		} else {
			return CLS_ERROR_WAITING_FOR_PASSWORD;
		}
	} else {
		m_commandQueue.append({command});
		startLocalServerProcess();
		return m_commandQueue.count() == 1 ? CLS_OK_STARTED : CLS_OK_QUEUED;
	}
}
#endif //TPSERVER_MACHINE
#endif //LOCAL_TPSERVER
