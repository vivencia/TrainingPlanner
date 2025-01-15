#include "osinterface.h"
#include "tputils.h"
#include "dbusermodel.h"
#include "dbinterface.h"
#include "qmlitemmanager.h"

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"
#include "qmlitemmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbinterface.h"
#include "dbmesocalendartable.h"

#include <QJniObject>
#include <qnativeinterface.h>
	#if QT_VERSION == QT_VERSION_CHECK(6, 8, 1)
		#include <QtCore/6.8.1/QtCore/private/qandroidextras_p.h>
	#else
		#include <QtCore/6.8.0/QtCore/private/qandroidextras_p.h>
	#endif

//"(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;"
// String f(Context, Uri)

const QString& workoutDoneMessage{qApp->tr("Your training routine seems to go well. Workout for the day is concluded")};

#define NOTIFY_DO_NOTHING 0xA
#define MESOCYCLE_NOTIFICATION 0x14
#define SPLIT_NOTIFICATION 0x1E
#define CALENDAR_NOTIFICATION 0x28
#define NOTIFY_START_WORKOUT 0x32
#define USER_NOTIFICATION 0x3C

#else
#include <QProcess>
extern "C"
{
	#include <unistd.h>
}
#endif

#include <QFileInfo>
#include <QGuiApplication>
#include <QStandardPaths>

OSInterface* OSInterface::app_os_interface(nullptr);

OSInterface::OSInterface(QObject* parent)
	: QObject{parent}
{
	app_os_interface = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));
	m_appDataFilesPath = std::move(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)) + '/';

#ifdef Q_OS_ANDROID
	const QJniObject& context(QNativeInterface::QAndroidApplication::context());

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

void OSInterface::aboutToExit()
{
	appDBInterface()->cleanUpThreads();
}

#ifdef Q_OS_ANDROID
void OSInterface::checkPendingIntents() const
{
	const QJniObject& activity = QNativeInterface::QAndroidApplication::context();
	if(activity.isValid())
	{
		activity.callMethod<void>("checkPendingIntents","()V");
		return;
	}
	DEFINE_SOURCE_LOCATION
	ERROR_MESSAGE("Activity not valid", QString())
}

/*
 * As default we're going the Java - way with one simple JNI call (recommended)
 * if altImpl is true we're going the pure JNI way
 * HINT: we don't use altImpl anymore
 *
 * If a requestId was set we want to get the Activity Result back (recommended)
 * We need the Request Id and Result Id to control our workflow
*/
bool OSInterface::sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const
{
	const QJniObject& jsPath = QJniObject::fromString(filePath);
	const QJniObject& jsTitle = QJniObject::fromString(title);
	const QJniObject& jsMimeType = QJniObject::fromString(mimeType);
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

void OSInterface::androidOpenURL(const QString& address) const
{
	QString url;
	if (!address.startsWith("http"_L1))
		url = "https://"_L1 + address;
	else
		url = address;

	const QJniObject& jsPath = QJniObject::fromString(url);
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

bool OSInterface::androidSendMail(const QString& address, const QString& subject, const QString& attachment) const
{
	const QString& attachment_file(attachment.isEmpty() ? QString() : "file://"_L1 + attachment);
	const QJniObject& jsAddress = QJniObject::fromString(address);
	const QJniObject& jsSubject = QJniObject::fromString(subject);
	const QJniObject& jsAttach = QJniObject::fromString(attachment_file);
	const jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendEmail",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
													jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>());
	return ok;
}

bool OSInterface::viewFile(const QString& filePath, const QString& title) const
{
	const QJniObject& jsPath = QJniObject::fromString(filePath);
	const QJniObject& jsTitle = QJniObject::fromString(title);
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

QString OSInterface::readFileFromAndroidFileDialog(const QString& android_uri) const
{
	if (android_uri.startsWith("//com"_L1))
	{
		QString properFilename;
		properFilename = "content:"_L1 + android_uri;
		const QString& localFile{m_appDataFilesPath + "tempfile"_L1};
		static_cast<void>(QFile::remove(localFile));
		return QFile::copy(properFilename, localFile) ? properFilename : QString();
	}
	// else: uri is not a uri it's already been translated by QShareUtils via android's open with or share
	return android_uri;
}

void OSInterface::startAppNotifications()
{
	QTimer notificationsTimer{this};
	notificationsTimer.setInterval(1000*30); //every 30min
	notificationsTimer.callOnTimeout([this] () { checkNotificationsStatus(); } );
	notificationsTimer.start();
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
		DBMesoCalendarTable* calTable{new DBMesoCalendarTable{appDBInterface()->dbFilesPath()}};
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			notificationData* data{new notificationData{}};
			data->title = std::move("TrainingPlanner "_L1) + data->start_time.toString("dd/MM - hh:mm"_L1);
			const QString& splitLetter{dayInfoList.at(2)};
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

void OSInterface::setFileUrlReceived(const QString& url) const
{
	const QString& androidUrl{appUtils()->getCorrectPath(url)};
	if (QFileInfo::exists(androidUrl))
		appItemManager()->openRequestedFile(androidUrl);
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("FILE does NOT exist ", url)
	}
}

void OSInterface::setFileReceivedAndSaved(const QString& url) const
{
	const QString& androidUrl{appUtils()->getCorrectPath(url)};
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

void OSInterface::removeNotification(notificationData* data)
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
	const char* urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	appOsInterface()->setFileUrlReceived(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileReceivedAndSaved(
						JNIEnv *env, jobject obj, jstring url)
{
	const char* urlStr = env->GetStringUTFChars(url, NULL);
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
void OSInterface::processArguments() const
{
	const QStringList& args(qApp->arguments());
	if (args.count() > 1)
	{
		QString filename;
		for (uint i(1); i < args.count(); ++i)
			filename += args.at(i) + ' ';
		filename.chop(1);
		const QFileInfo file{filename};
		if (file.isFile())
			appItemManager()->openRequestedFile(appUtils()->getCorrectPath(filename));
	}
}

void OSInterface::restartApp()
{
	char* args[2] = { nullptr, nullptr };
	const QString& argv0{qApp->arguments().at(0)};
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}
#endif //Q_OS_ANDROID

#include "online_services/tponlineservices.h"

void OSInterface::checkOnlineResources()
{
	//TPOnlineServices *tos = new TPOnlineServices{this};
}

void OSInterface::shareFile(const QString& fileName) const
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
void OSInterface::openURL(const QString& address) const
{
	#ifdef Q_OS_ANDROID
	androidOpenURL(address);
	#else
	auto* __restrict proc(new QProcess());
	proc->startDetached("xdg-open"_L1, QStringList() << address);
	delete proc;
	#endif
}

void OSInterface::startChatApp(const QString& phone, const QString& appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	QString::const_iterator itr{phone.constBegin()};
	const QString::const_iterator& itr_end{phone.constEnd()};
	do {
		if ((*itr).isDigit())
			phoneNumbers += *itr;
	} while (++itr != itr_end);

	QString address;
	if (appname.contains("Whats"_L1))
		address = std::move("https://wa.me/"_L1 + phoneNumbers);
	else
		address = std::move("https://t.me/+"_L1 + phoneNumbers);

	openURL(address);
}

void OSInterface::sendMail(const QString& address, const QString& subject, const QString& attachment_file) const
{
	#ifdef Q_OS_ANDROID
	if (!androidSendMail(address, subject, attachment_file))
	{
		if (appUserModel()->email(0).contains("gmail.com"_L1))
		{
			const QString& gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(appUserModel()->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList& args (QStringList() <<
		"--utf8"_L1 << "--subject"_L1 << QChar('\'') + subject + QChar('\'') << "--attach"_L1 << attachment_file <<
			QChar('\'') + address + QChar('\''));
	auto* __restrict proc(new QProcess ());
	proc->start("xdg-email"_L1, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exitCode, QProcess::ExitStatus)
	{
		if (exitCode != 0)
		{
			if (appUserModel()->email(0).contains("gmail.com"_L1))
			{
				const QString& gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_s.arg(appUserModel()->email(0), address, subject));
				openURL(gmailURL);
			}
		}
		proc->deleteLater();
	});
	#endif
}

void OSInterface::viewExternalFile(const QString& filename) const
{
	if (!appUtils()->canReadFile(appUtils()->getCorrectPath(filename)))
		return;
	#ifdef Q_OS_ANDROID
	const QString& localFile{m_appDataFilesPath + "tempfile"_L1 + filename.last(4)};
	static_cast<void>(QFile::remove(localFile));
	if (QFile::copy(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "could not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}
