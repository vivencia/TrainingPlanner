#include "osinterface.h"

#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "online_services/tponlineservices.h"

#ifdef Q_OS_ANDROID
#include "dbmesocyclesmodel.h"
#include "tpandroidnotification.h"

#include <QJniObject>
#include <qnativeinterface.h>
#include <QtCore/6.9.1/QtCore/private/qandroidextras_p.h>

//"(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;"
// String f(Context, Uri)

static const QString &workoutDoneMessage{qApp->tr("Your training routine seems to go well. Workout for the day is concluded")};

#define NOTIFY_DO_NOTHING 0xA
#define MESOCYCLE_NOTIFICATION 0x14
#define SPLIT_NOTIFICATION 0x1E
#define CALENDAR_NOTIFICATION 0x28
#define NOTIFY_START_WORKOUT 0x32
#define USER_NOTIFICATION 0x3C

#else

#ifdef Q_OS_LINUX
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

constexpr int TPSERVER_OK				{0};
constexpr int TPSERVER_ERROR			{1};
constexpr int TPSERVER_NGINX_ERROR		{2};
constexpr int TPSERVER_PHPFPM_ERROR		{3};
constexpr int TPSERVER_CONFIG_ERROR		{4};
constexpr int TPSERVER_OK_LOCALHOST		{5};
constexpr int TPSERVER_PAUSED			{6};
constexpr int TPSERVER_PAUSED_LOCALHOST	{7};
constexpr int TPSERVER_PAUSED_FAILED	{8};

#endif //Q_OS_LINUX
#endif //Q_OS_ANDROID

#include <QFileInfo>
#include <QGuiApplication>
#include <QNetworkInterface>
#include <QProcess>
#include <QSysInfo>
#include <QTcpSocket>
#include <QTimer>

OSInterface *OSInterface::app_os_interface{nullptr};
#ifndef QT_NO_DEBUG
//When testing, poll more frequently
constexpr int CONNECTION_CHECK_TIMEOUT{60*1000};
constexpr int CONNECTION_ERR_TIMEOUT{10*1000};
#else
constexpr int CONNECTION_CHECK_TIMEOUT{10*60*1000};
constexpr int CONNECTION_ERR_TIMEOUT{20*1000};
#endif

enum connectMessagesIndex {
	interfaceMessage = 0,
	internetMessage = 1,
	serverMessage = 2,
};

OSInterface::OSInterface(QObject *parent)
	: QObject{parent}, m_networkStatus{0}
{
	app_os_interface = this;
	m_connectionMessages.resize(3);

	m_checkConnectionTimer = new QTimer{this};
	m_checkConnectionTimer->callOnTimeout([this] () { checkNetworkInterfaces(); });
	/*connect(appOnlineServices(), &TPOnlineServices::serverOnline, this, [this] (const uint online_status)
	{
		onlineServicesResponse(online_status);
	});*/
	connect(qApp, &QCoreApplication::aboutToQuit, this, [this] () {
		appOnlineServices()->userLogout(111111);
	});
	checkNetworkInterfaces();

#ifdef Q_OS_ANDROID
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
		appItemManager()->displayActivityResultMessage(requestCode, resultCode);
	});

	connect(qApp, &QGuiApplication::applicationStateChanged, this, [&] (Qt::ApplicationState state) {
		if (state == Qt::ApplicationSuspended)
		{
			mb_appSuspended = true;
			emit appSuspended();
		}
		else if (state == Qt::ApplicationActive)
		{
			if (mb_appSuspended)
			{
				emit appResumed();
				mb_appSuspended = false;
			}
		}
	});

	m_AndroidNotification = new TPAndroidNotification{this};
#endif
}

#ifdef Q_OS_ANDROID
void OSInterface::checkPendingIntents() const
{
	const QJniObject &activity{QNativeInterface::QAndroidApplication::context()};
	if (activity.isValid())
	{

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
bool OSInterface::sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) const
{
	const QJniObject &jsPath{QJniObject::fromString(filePath)};
	const QJniObject &jsTitle{QJniObject::fromString(title)};
	const QJniObject &jsMimeType{QJniObject::fromString(mimeType)};
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

void OSInterface::androidOpenURL(const QString &address) const
{
	QString url;
	if (!address.startsWith("http"_L1))
		url = std::move("https://"_L1 + address);
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

bool OSInterface::androidSendMail(const QString &address, const QString &subject, const QString &attachment) const
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
	return ok;
}

bool OSInterface::viewFile(const QString &filePath, const QString &title) const
{
	const QJniObject &jsPath{QJniObject::fromString(filePath)};
	const QJniObject &jsTitle{QJniObject::fromString(title)};
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
	for (qsizetype i{m_notifications.count()-1}; i >= 0; --i)
	{
		if (m_notifications.at(i)->expiration == now)
		{
			action = m_notifications.at(i)->action;
			removeNotification(m_notifications.at(i));
			switch (action)
			{
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

	/*if (appMesoModel()->count() > 0)
	{
		DBMesoCalendarTable *calTable{new DBMesoCalendarTable{appThreadManager()->dbFilesPath()}};
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			notificationData *data{new notificationData{}};
			data->title = std::move("TrainingPlanner "_L1) + data->start_time.toString("dd/MM - hh:mm"_L1);
			const QString &splitLetter{dayInfoList.at(2)};
			if (splitLetter != "R"_L1) //day is training day
			{

				if (dayInfoList.at(3) == '1') //day is completed
				{
					data->message = workoutDoneMessage;
					data->action = NOTIFY_DO_NOTHING;
				}
				else
				{
					data->message = std::move(tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter);
					data->action = NOTIFY_START_WORKOUT;
					if (!m_bTodaysWorkoutFinishedConnected)
					{
						connect(appMesoModel(), &DBMesocyclesModel::todaysWorkoutFinished, this, [this,data] () {
							data->resolved = true;
							removeNotification(data);
						});
						m_bTodaysWorkoutFinishedConnected = true;
					}
				}
			}
			else
			{
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

void OSInterface::setFileUrlReceived(const QString &url) const
{
	const QString &androidUrl{appUtils()->getCorrectPath(url)};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_FILE_NOT_FOUND, url);
}

void OSInterface::setFileReceivedAndSaved(const QString &url) const
{
	const QString &androidUrl{appUtils()->getCorrectPath(url)};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_FILE_NOT_FOUND, url);
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
	for (qsizetype i{0}; i < m_notifications.count(); ++i)
	{
		if (m_notifications.at(i)->id == id && !m_notifications.at(i)->resolved)
		{
			switch (action)
			{
				case NOTIFY_DO_NOTHING:
					m_notifications.at(i)->resolved = true;
				break;
				case NOTIFY_START_WORKOUT:
					appMesoModel()->todaysWorkout();
					m_notifications.at(i)->resolved = true;
				break;
			}
		}
	}
}

void OSInterface::removeNotification(notificationData *data)
{
	m_AndroidNotification->cancelNotification(data->id);
	if (data->action == NOTIFY_START_WORKOUT)
	{
		if (data->resolved) //Send a new notification with an innocuous greeting message.
		{
			data->resolved = false;
			data->message = workoutDoneMessage;
			data->action = NOTIFY_DO_NOTHING;
			m_AndroidNotification->sendNotification(data);
			return;
		}
	}
	m_notifications.removeOne(data);
	delete data;
}

extern "C"
{

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileUrlReceived(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	appOsInterface()->setFileUrlReceived(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileReceivedAndSaved(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	appOsInterface()->setFileReceivedAndSaved(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_fireActivityResult(
						JNIEnv *env, jobject obj, jint requestCode, jint resultCode)
{
	Q_UNUSED (obj)
	Q_UNUSED (env)
	appOsInterface()->onActivityResult(requestCode, resultCode);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_notificationActionReceived(
						JNIEnv *env, jobject obj, jint action, jint id)
{
	Q_UNUSED (obj)
	//const char *actionStr = env->GetStringUTFChars(action, NULL);
	appOsInterface()->execNotification(static_cast<short>(action), static_cast<short>(id));
	//env->ReleaseStringUTFChars(action, actionStr);
	return;
}
} //extern "C"

#else

void OSInterface::serverProcessFinished(QProcess *proc, const int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitStatus != QProcess::NormalExit)
	{
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, appUtils()->string_strings(
				{"Linux TP Server"_L1, "Error executing init_script"_L1}, record_separator));
		return;
	}

	switch (exitCode)
	{
		case TPSERVER_OK:
			//appOnlineServices()->scanNetwork(appSettings()->serverAddress());
			onlineServicesResponse(TPSERVER_OK);
		break;
		case TPSERVER_OK_LOCALHOST:
			appSettings()->setServerAddress("localhost"_L1);
			onlineServicesResponse(TPSERVER_OK);
		break;
		case TPSERVER_ERROR:
		case TPSERVER_NGINX_ERROR:
		case TPSERVER_PAUSED_FAILED:
			onlineServicesResponse(exitCode, proc->readAllStandardOutput() % "\nReturn code("_L1 % QString::number(exitCode) % ')');
		break;
		case TPSERVER_PHPFPM_ERROR:
			commandLocalServer("Start server service?"_L1, "start"_L1);
		break;

		case TPSERVER_CONFIG_ERROR:
			commandLocalServer("Setup server?"_L1, "setup"_L1);
		break;

		case TPSERVER_PAUSED:
		case TPSERVER_PAUSED_LOCALHOST:
			commandLocalServer("Unpause server?"_L1, "pause"_L1);
		break;
	}
	proc->close();
	proc->deleteLater();
}

void OSInterface::checkLocalServer()
{
	QProcess *check_server_proc{new QProcess{this}};
	connect(check_server_proc, &QProcess::finished, this, [this,check_server_proc] (int exitCode, QProcess::ExitStatus exitStatus) {
		serverProcessFinished(check_server_proc, exitCode, exitStatus);
	});
	check_server_proc->start(tp_server_config_script, {"status"_L1}, QIODeviceBase::ReadOnly);
}

void OSInterface::commandLocalServer(const QString &message, const QString &command)
{
	connect(appItemManager(), &QmlItemManager::qmlPasswordDialogClosed, this, [this,message,command] (int resultCode, const QString &password) {
		if (resultCode == 0)
		{
			QProcess *server_script_proc{new QProcess{this}};
			connect(server_script_proc, &QProcess::finished, this, [this,server_script_proc] (int exitCode, QProcess::ExitStatus exitStatus) {
				serverProcessFinished(server_script_proc, exitCode, exitStatus);
			});
			server_script_proc->start(tp_server_config_script , {command, "-p="_L1 + password}, QIODeviceBase::ReadOnly);
		}
		else
			appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
						{message, "Operation canceled by the user"_L1}, record_separator));
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appItemManager()->getPasswordDialog(message, "Your user password is required"_L1);
}

void OSInterface::processArguments() const
{
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1)
	{
		QString filename;
		for (const auto &arg : args)
			filename += arg + ' ';
		filename.chop(1);
		const QFileInfo file{filename};
		if (file.isFile())
			appItemManager()->openRequestedFile(appUtils()->getCorrectPath(filename));
	}
}

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
#endif //Q_OS_ANDROID

QString OSInterface::deviceID() const
{
	return QSysInfo::machineUniqueId();
}

void OSInterface::shareFile(const QString &fileName) const
{
	#ifdef Q_OS_ANDROID
	/*setExportFileName("app_logo.png");
	if (!QFile::exists(exportFileName()))
	{
		QFile::copy(":/images/app_logo.png", exportFileName());
		QFile::setPermissions(exportFileName(), QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
	}
	sendFile(exportFileName(), tr("Send file"), u"image/png"_s, 10);*/
	sendFile(fileName, tr("Send file"), "text/plain"_L1, 10);
	#endif
}
void OSInterface::openURL(const QString &address) const
{
	if (!address.isEmpty())
	{
		#ifdef Q_OS_ANDROID
		androidOpenURL(address);
		#else
		auto *__restrict proc{new QProcess{}};
		proc->startDetached("xdg-open"_L1, QStringList{} << address);
		delete proc;
		#endif
	}
}

void OSInterface::startMessagingApp(const QString &phone, const QString &appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	for (const auto &it: phone)
	{
		if (it.isDigit())
			phoneNumbers += it;
	}
	QString address;
	if (appname.contains("Whats"_L1))
		address = std::move("https://wa.me/"_L1 + phoneNumbers);
	else
		address = std::move("https://t.me/+"_L1 + phoneNumbers);

	openURL(address);
}

void OSInterface::sendMail(const QString &address, const QString &subject, const QString &attachment_file) const
{
	#ifdef Q_OS_ANDROID
	if (!androidSendMail(address, subject, attachment_file))
	{
		if (appUserModel()->email(0).contains("gmail.com"_L1))
		{
			const QString &gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(appUserModel()->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList &args{QStringList{6} << "--utf8"_L1 << "--subject"_L1 << QChar{'\''} + subject + QChar{'\''} <<
						"--attach"_L1 << attachment_file << QChar{'\''} + address + QChar{'\''}};
	auto *__restrict proc{new QProcess};
	proc->start("xdg-email"_L1, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exitCode, QProcess::ExitStatus)
	{
		if (exitCode != 0)
		{
			if (appUserModel()->email(0).contains("gmail.com"_L1))
			{
				const QString &gmailURL{u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(appUserModel()->email(0), address, subject)};
				openURL(gmailURL);
			}
		}
		proc->deleteLater();
	});
	#endif
}

void OSInterface::viewExternalFile(const QString &filename) const
{
	if (!appUtils()->canReadFile(appUtils()->getCorrectPath(filename)))
		return;
	#ifdef Q_OS_ANDROID
	const QString &localFile{appSettings()->localAppFilesDir() + "tempfile"_L1 + filename.last(4)};
	static_cast<void>(QFile::remove(localFile));
	if (appUtils()->copyFile(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "could not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}



void OSInterface::setNetStatus(uint messages_index, bool success, QString &&message)
{
	short on_bit{0}, off_bit{0};
	switch (messages_index)
	{
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
	setConnectionMessage(messages_index, std::move(message));
	emit tpServerStatusChanged(success);
}

void OSInterface::checkNetworkInterfaces()
{
	const QNetworkInterface *running_interface{nullptr};
	const QList<QNetworkInterface> &interfaces{QNetworkInterface::allInterfaces()};
	for (const auto &interface : interfaces)
	{
		if (interface.flags() & QNetworkInterface::IsRunning)
		{
			if (interface.name() == "lo"_L1)
			{
				#ifndef Q_OS_ANDROID
				running_interface = &interface;
				#endif
			}
			else
			{
				const QList<QNetworkAddressEntry> &addresses{interface.addressEntries()};
				for (const auto &address : addresses)
				{
					if (!address.ip().isNull())
					{
						running_interface = &interface;
						break;
					}
				}
			}
		}
	}
	const bool success{running_interface ? running_interface->isValid() : false};
	if (!m_currentNetworkStatus[interfaceMessage].has_value() || m_currentNetworkStatus[interfaceMessage].value() != success)
	{
		QString message{tr("Network interface: ")};
		if (success)
		{
			switch (running_interface->type())
			{
				case QNetworkInterface::Loopback: message += "Loopback"_L1; break;
				case QNetworkInterface::Virtual: message += "Virtual"_L1; break;
				case QNetworkInterface::Ethernet:  message += "Ethernet"_L1; break;
				case QNetworkInterface::Wifi: message += "WiFi"_L1; break;
				default: message += "Unknown"_L1; break;
			}
			m_localIPAddress = running_interface->addressEntries().constFirst().ip().toString();
			message += '(' % running_interface->name() % ')';
			if (appSettings()->serverAddress() != m_localIPAddress)
				appSettings()->setServerAddress(m_localIPAddress);
		}
		else
			message += tr("This device does not have access to any network interface or the app does not have permission to access them");
		setNetStatus(interfaceMessage, success, std::move(message));
	}
	checkInternetConnection();
}

void OSInterface::checkInternetConnection()
{
	bool is_connected{false};
	if (networkInterfaceOK())
	{
		QTcpSocket checkConnectionSocket;
		checkConnectionSocket.connectToHost("google.com"_L1, 443); // 443 for HTTPS or use Port 80 for HTTP
		checkConnectionSocket.waitForConnected(2000);
		is_connected = checkConnectionSocket.state() == QTcpSocket::ConnectedState;
		checkConnectionSocket.close();
	}
	if (!m_currentNetworkStatus[internetMessage].has_value() || m_currentNetworkStatus[internetMessage].value() != is_connected)
	{
		setNetStatus(internetMessage, is_connected,
			std::move(is_connected ? tr("Device is connected to the internet") : tr("Device is not connected to the internet")));
		emit internetStatusChanged();
	}

	#ifndef Q_OS_ANDROID
	checkLocalServer();
	#else
	appOnlineServices()->scanNetwork(appSettings()->serverAddress());
	#endif
}

void OSInterface::setConnectionMessage(int msg_idx, QString &&message)
{
	m_connectionMessages[msg_idx] = std::move(message);
	emit connectionMessageChanged();
}

void OSInterface::onlineServicesResponse(const uint online_status, const QString &additional_message)
{
	const bool online{online_status == TP_RET_CODE_SUCCESS};
	if (!m_currentNetworkStatus[serverMessage].has_value() || m_currentNetworkStatus[serverMessage].value() != online)
	{
		QString message{online ? tr("Connected to server ") : tr("Server unreachable")};
		if (online)
			message += '(' + appSettings()->serverAddress() % ')' % additional_message;
		else
			message += additional_message;
		setNetStatus(serverMessage, online, std::move(message));
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, appUtils()->string_strings(
					{"Linux TP Server"_L1, connectionMessage()}, record_separator),
					online_status == TP_RET_CODE_SUCCESS ? "set-completed" : "error");
	}
	m_checkConnectionTimer->start(online ? CONNECTION_CHECK_TIMEOUT : CONNECTION_ERR_TIMEOUT); //When network is out, check more frequently)
}
