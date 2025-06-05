#include "osinterface.h"

#include "dbusermodel.h"
#include "dbinterface.h"
#include "qmlitemmanager.h"
#include "tputils.h"
#include "online_services/tponlineservices.h"

#ifdef Q_OS_ANDROID
#include "dbmesocyclesmodel.h"
#include "dbmesocalendartable.h"
#include "tpandroidnotification.h"

#include <QJniObject>
#include <qnativeinterface.h>
#if QT_VERSION == QT_VERSION_CHECK(6, 8, 2)
	#include <QtCore/6.8.2/QtCore/private/qandroidextras_p.h>
#else
	#include <QtCore/6.8.0/QtCore/private/qandroidextras_p.h>
#endif

//"(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;"
// String f(Context, Uri)

const QString &workoutDoneMessage{qApp->tr("Your training routine seems to go well. Workout for the day is concluded")};

#define NOTIFY_DO_NOTHING 0xA
#define MESOCYCLE_NOTIFICATION 0x14
#define SPLIT_NOTIFICATION 0x1E
#define CALENDAR_NOTIFICATION 0x28
#define NOTIFY_START_WORKOUT 0x32
#define USER_NOTIFICATION 0x3C

#else

#ifdef Q_OS_LINUX
#include <QProcess>
static const QString &tp_server_config_script{"/var/www/html/trainingplanner/scripts/init_script.sh"_L1};
static bool can_display_message{true};

extern "C"
{
	#include <unistd.h>
}
#endif
#endif

#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>

OSInterface *OSInterface::app_os_interface(nullptr);
constexpr uint CONNECTION_CHECK_TIMEOUT{10*60*1000};
constexpr uint CONNECTION_ERR_TIMEOUT{20*1000};

OSInterface::OSInterface(QObject *parent)
	: QObject{parent}, m_networkStatus{0}, m_bchecking_ic{false}
{
	app_os_interface = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));
	connect(appOnlineServices(), &TPOnlineServices::serverOnline, this, &OSInterface::checkServerResponseSlot);

	m_checkConnectionTimer = new QTimer{this};
	m_checkConnectionTimer->setInterval(CONNECTION_CHECK_TIMEOUT);
	m_checkConnectionTimer->callOnTimeout([this] () { checkInternetConnection(); });
	m_checkConnectionTimer->start();
	checkInternetConnection();

#ifdef Q_OS_ANDROID
	const QJniObject &context(QNativeInterface::QAndroidApplication::context());

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

	mb_appSuspended = false;
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
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

void OSInterface::checkInternetConnection()
{
	if (m_bchecking_ic)
		return;
	m_bchecking_ic = true;
	int network_status{0};
    QTcpSocket checkConnectionSocket;
    checkConnectionSocket.connectToHost("google.com"_L1, 443); // 443 for HTTPS or use Port 80 for HTTP
    checkConnectionSocket.waitForConnected(2000);

    bool isConnected{checkConnectionSocket.state() == QTcpSocket::ConnectedState};
    checkConnectionSocket.close();
#ifndef QT_NO_DEBUG
	if (!isConnected)
	{
		//When debugging, and using the local server as online server, ignore if the internet is not working
		setBit(network_status, HAS_INTERNET);
		unSetBit(network_status, NO_INTERNET_ACCESS);
		appOnlineServices()->checkServer();
		isConnected = true;
	}
#else
    setBit(network_status, isConnected ? HAS_INTERNET : NO_INTERNET_ACCESS);
	unSetBit(network_status, !isConnected ? HAS_INTERNET : NO_INTERNET_ACCESS);
#endif

	if (isConnected)
		appOnlineServices()->checkServer();
	else
	{
		setBit(network_status, SERVER_UNREACHABLE);
		unSetBit(network_status, SERVER_UP_AND_RUNNING);
	}
	setNetworkStatus(network_status, 0);
}

void OSInterface::setNetworkStatus(int new_internetstatus, int new_serverstatus)
{
	if (new_internetstatus != 0)
	{
		const bool has_internet_now{isBitSet(new_internetstatus, HAS_INTERNET)};
		if (internetOK() != has_internet_now)
		{
			setBit(m_networkStatus, has_internet_now ? HAS_INTERNET : NO_INTERNET_ACCESS);
			unSetBit(m_networkStatus, has_internet_now ? NO_INTERNET_ACCESS : HAS_INTERNET);
			if (!has_internet_now)
			{
				setBit(m_networkStatus, SERVER_UNREACHABLE);
				unSetBit(m_networkStatus, SERVER_UP_AND_RUNNING);
			}
			m_checkConnectionTimer->setInterval(has_internet_now ? CONNECTION_CHECK_TIMEOUT : CONNECTION_ERR_TIMEOUT); //When network is out, check more frequently
			emit internetStatusChanged(internetOK());
		}
	}
	if (new_serverstatus != 0)
	{
		const bool server_online_now{isBitSet(new_serverstatus, SERVER_UP_AND_RUNNING)};
		if (tpServerOK() != server_online_now)
		{
			setBit(m_networkStatus, server_online_now ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE);
			unSetBit(m_networkStatus, server_online_now ? SERVER_UNREACHABLE : SERVER_UP_AND_RUNNING);
			emit serverStatusChanged(server_online_now);
		}
		m_checkConnectionTimer->setInterval(server_online_now ? CONNECTION_CHECK_TIMEOUT : CONNECTION_ERR_TIMEOUT); //When network is out, check more frequently
	}
}

void OSInterface::aboutToExit()
{
	appDBInterface()->cleanUpThreads();
}

void OSInterface::checkServerResponseSlot(const bool online)
{
#ifdef Q_OS_LINUX
	#ifndef Q_OS_ANDROID
	if (!online)
	{
		if (can_display_message)
		{
			can_display_message = false;
			checkLocalServer();
		}
	}
	else
	{
		can_display_message = true;
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, "TrainingPlanner App"_L1 + record_separator + "Connected online!"_L1);
	}
	#endif
	#else
	if (!online)
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, "TrainingPlanner App"_L1 + record_separator + "Server unreachable!"_L1);
	else
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, "TrainingPlanner App"_L1 + record_separator + "Connected online!"_L1);
#endif
	int server_status{0};
	setBit(server_status, online ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE);
	unSetBit(server_status, !online ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE);
	setNetworkStatus(0, server_status);
	m_bchecking_ic = false;
}

#ifdef Q_OS_ANDROID
void OSInterface::checkPendingIntents() const
{
	const QJniObject &activity = QNativeInterface::QAndroidApplication::context();
	if(activity.isValid())
	{
		activity.callMethod<void>("checkPendingIntents","()V");
		return;
	}
	DEFINE_SOURCE_LOCATION
	ERROR_MESSAGE("Activity not valid", QString())
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
	const QJniObject &jsPath = QJniObject::fromString(filePath);
	const QJniObject &jsTitle = QJniObject::fromString(title);
	const QJniObject &jsMimeType = QJniObject::fromString(mimeType);
	const jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendFile",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>(), jsMimeType.object<jstring>(), requestId);
	if(!ok)
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("Unable to resolve activity from Java", QString())
		return false;
	}
	return true;
}

void OSInterface::androidOpenURL(const QString &address) const
{
	QString url;
	if (!address.startsWith("http"_L1))
		url = std::move("https://"_L1 + address);
	else
		url = address;

	const QJniObject &jsPath = QJniObject::fromString(url);
	const jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"openURL",
													"(Ljava/lang/String;)Z",
													jsPath.object<jstring>());
	if(!ok)
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("Unable to open the address: ", address)
	}
}

bool OSInterface::androidSendMail(const QString &address, const QString &subject, const QString &attachment) const
{
	const QString &attachment_file{attachment.isEmpty() ? QString() : "file://"_L1 + attachment};
	const QJniObject &jsAddress = QJniObject::fromString(address);
	const QJniObject &jsSubject = QJniObject::fromString(subject);
	const QJniObject &jsAttach = QJniObject::fromString(attachment_file);
	const jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendEmail",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
													jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>());
	return ok;
}

bool OSInterface::viewFile(const QString &filePath, const QString &title) const
{
	const QJniObject &jsPath = QJniObject::fromString(filePath);
	const QJniObject &jsTitle = QJniObject::fromString(title);
	const jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"viewFile",
													"(Ljava/lang/String;Ljava/lang/String;)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>());
	if(!ok)
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("Unable to resolve view activity from Java", QString())
	}
	return ok;
}

QString OSInterface::readFileFromAndroidFileDialog(const QString &android_uri) const
{
	if (android_uri.startsWith("//com"_L1))
	{
		const QString &properFilename{"content:"_L1 + android_uri};
		const QString &localFile{appUtils()->localAppFilesDir() + "tempfile"_L1};
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
	if (appMesoModel()->count() > 0)
	{
		DBMesoCalendarTable *calTable{new DBMesoCalendarTable{appDBInterface()->dbFilesPath()}};
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			notificationData *data{new notificationData{}};
			data->title = std::move("TrainingPlanner "_L1) + data->start_time.toString("dd/MM - hh:mm"_L1);
			const QString &splitLetter{dayInfoList.at(2)};
			if (splitLetter != "R"_L1) //day is training day
			{

				if (dayInfoList.at(3) == STR_ONE) //day is completed
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
	}
}

void OSInterface::setFileUrlReceived(const QString &url) const
{
	const QString &androidUrl{appUtils()->getCorrectPath(url)};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("FILE does NOT exist ", url)
	}
}

void OSInterface::setFileReceivedAndSaved(const QString &url) const
{
	const QString &androidUrl{appUtils()->getCorrectPath(url)};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("FILE does NOT exist ", url)
	}
}

void OSInterface::onActivityResult(int requestCode, int resultCode)
{
	// we're getting RESULT_OK only if edit is done
	if (resultCode == -1)
		LOG_MESSAGE("Send Activity Result OK")
	else if (resultCode == 0)
		LOG_MESSAGE("Send Activity Result Canceled")
	else
		LOG_MESSAGE("Send Activity wrong result code: " << resultCode << " from request: " << requestCode)
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

QString OSInterface::executeAndCaptureOutput(const QString &program, QStringList &arguments, const bool b_asRoot, int *exitCode)
{
	auto *__restrict proc{new QProcess()};
	QString app;

	if (b_asRoot)
	{
		arguments.prepend(program);
		app = std::move("/usr/bin/pkexec"_L1);
	}
	else
		app = program;

	proc->start(app, arguments);
	proc->waitForFinished(10000);
	QString strOutput{std::move(proc->readAllStandardOutput().constData())};
	if (strOutput.isEmpty())
		strOutput = std::move(proc->readAllStandardError().constData());
	if (exitCode != nullptr)
		*exitCode = proc->exitCode();

	delete proc;
	return strOutput;
}

void OSInterface::checkLocalServer()
{
	QProcess *check_server_proc{new QProcess{this}};
	connect(check_server_proc, &QProcess::finished, this, [this,check_server_proc] (int exitCode, QProcess::ExitStatus exitStatus) {
		check_server_proc->deleteLater();
		if (exitStatus != QProcess::NormalExit)
		{
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, "Linux TP Server"_L1 +
						record_separator + "Could not run init_script test"_L1);
			return;
		}

		switch (exitCode)
		{
			case 0:
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, "Linux TP Server"_L1 +
						record_separator + "Up and running!"_L1);
			break;
			case 1:
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, "Linux TP Server"_L1 +
						record_separator + check_server_proc->readAllStandardOutput());
			break;
			case 2: commandLocalServer("start"_L1); break;
			case 3: commandLocalServer("setup"_L1); break;
		}
	});
	check_server_proc->start(tp_server_config_script, {"test"_L1}, QIODeviceBase::ReadOnly);
}

void OSInterface::commandLocalServer(const QString &command)
{
	connect(appItemManager(), &QmlItemManager::qmlPasswordDialogClosed, this, [this,command] (int resultCode, const QString &password) {
		if (resultCode == 0)
		{
			QProcess *server_script_proc{new QProcess{this}};
			connect(server_script_proc, &QProcess::finished, this, [this,server_script_proc] (int exitCode, QProcess::ExitStatus exitStatus) {
				if (exitStatus != QProcess::NormalExit)
					exitCode = 10;
				int server_status{0};
				setBit(server_status, exitCode == 0 ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE);
				unSetBit(server_status, exitCode != 0 ? SERVER_UP_AND_RUNNING : SERVER_UNREACHABLE);
				setNetworkStatus(0, server_status);
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, "Linux TP Server"_L1 + record_separator +
						server_script_proc->readAllStandardOutput() + "\nReturn code("_L1 + QString::number(exitCode) + ')');
				server_script_proc->deleteLater();
			});
			server_script_proc->start(tp_server_config_script , {command, "-p="_L1 + password}, QIODeviceBase::ReadOnly);
		}
		else
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, "Linux TP Server"_L1 + record_separator +
				"Operation canceled by the user"_L1);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appItemManager()->getPasswordDialog("Administrator's rights needed"_L1,
							"In order to setup and/or start the local HTTP server, you need to provide your user's password"_L1);
}

void OSInterface::processArguments() const
{
	const QStringList &args{qApp->arguments()};
	if (args.count() > 1)
	{
		QString filename;
		for (uint i{1}; i < args.count(); ++i)
			filename += args.at(i) + ' ';
		filename.chop(1);
		const QFileInfo file{filename};
		if (file.isFile())
			appItemManager()->openRequestedFile(appUtils()->getCorrectPath(filename));
	}
}

void OSInterface::restartApp()
{
	char *args[2] = { nullptr, nullptr };
	const QString &argv0{qApp->arguments().at(0)};
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size())  *sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}
#endif //Q_OS_ANDROID

void OSInterface::checkOnlineResources()
{
//TODO
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

void OSInterface::startChatApp(const QString &phone, const QString &appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	for (const auto it: phone)
	{
		if (it.isDigit())
			phoneNumbers += it;
	}
	/*QString::const_iterator itr{phone.constBegin()};
	const QString::const_iterator &itr_end{phone.constEnd()};
	do {
		if ((*itr).isDigit())
			phoneNumbers += *itr;
	} while (++itr != itr_end);*/

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
	const QStringList &args (QStringList{} <<
		"--utf8"_L1 << "--subject"_L1 << QChar{'\''} + subject + QChar{'\''} << "--attach"_L1 << attachment_file <<
			QChar{'\''} + address + QChar{'\''});
	auto *__restrict proc(new QProcess ());
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
	const QString &localFile{appUtils()->localAppFilesDir() + "tempfile"_L1 + filename.last(4)};
	static_cast<void>(QFile::remove(localFile));
	if (appUtils()->copyFile(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "could not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}
