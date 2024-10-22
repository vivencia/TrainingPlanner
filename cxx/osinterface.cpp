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
	#if QT_VERSION == QT_VERSION_CHECK(6, 8, 0)
		#include <QtCore/6.8.0/QtCore/private/qandroidextras_p.h>
	#else
		#include <QtCore/6.7.2/QtCore/private/qandroidextras_p.h>
	#endif
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
	m_appDataFilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/"_qs;
}

void OSInterface::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
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
	if (!address.startsWith(u"http"_qs))
		url = u"https://" + address;
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
	const QString& attachment_file(attachment.isEmpty() ? QString() : u"file://"_qs + attachment);
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

void OSInterface::appStartUpNotifications()
{
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &TPUtils::appResumed, this, &OSInterface::checkPendingIntents);
	connect(this, &OSInterface::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QmlManager()->displayActivityResultMessage(requestCode, resultCode);
	});

	m_AndroidNotification = new TPAndroidNotification(this);
	if (appMesoModel()->count() > 0)
	{
		DBMesoCalendarTable* calTable{new DBMesoCalendarTable(appDBInterface()->dbFilesPath())};
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			QString message;
			const QString& splitLetter{dayInfoList.at(2)};
			if (splitLetter != u"R"_qs) //day is training day
			{
				if (dayInfoList.at(3) == STR_ONE) //day is completed
					message = tr("Your training routine seems to go well. Workout for the day is concluded");
				else
					message = tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter;
			}
			else
				message = tr("Enjoy your day of rest from workouts!");
			m_AndroidNotification->sendNotification(u"Training Planner"_qs, message, WORKOUT_NOTIFICATION);
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

void OSInterface::startNotificationAction(const QString& action)
{
	if (action.toUInt() == WORKOUT_NOTIFICATION)
		appItemManager()->getTrainingDayPage(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate());
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
						JNIEnv *env, jobject obj, jstring action)
{
	Q_UNUSED (obj)
	const char *actionStr = env->GetStringUTFChars(action, NULL);
	appOsInterface()->startNotificationAction(actionStr);
	env->ReleaseStringUTFChars(action, actionStr);
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
	exitApp();
}
#endif //Q_OS_ANDROID

void OSInterface::shareFile(const QString& fileName) const
{
	#ifdef Q_OS_ANDROID
	/*setExportFileName("app_logo.png");
	if (!QFile::exists(exportFileName()))
	{
		QFile::copy(":/images/app_logo.png", exportFileName());
		QFile::setPermissions(exportFileName(), QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
	}
	sendFile(exportFileName(), tr("Send file"), u"image/png"_qs, 10);*/
	sendFile(fileName, tr("Send file"), u"text/plain"_qs, 10);
	#endif
}
void OSInterface::openURL(const QString& address) const
{
	#ifdef Q_OS_ANDROID
	androidOpenURL(address);
	#else
	auto* __restrict proc(new QProcess());
	proc->startDetached(u"xdg-open"_qs, QStringList() << address);
	delete proc;
	#endif
}

void OSInterface::startChatApp(const QString& phone, const QString& appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	QString::const_iterator itr(phone.constBegin());
	const QString::const_iterator& itr_end(phone.constEnd());
	do {
		if ((*itr).isDigit())
			phoneNumbers += *itr;
	} while (++itr != itr_end);

	QString address;
	if (appname.contains(u"Whats"_qs))
		address = u"https://wa.me/"_qs + phoneNumbers;
	else
		address = u"https://t.me/+"_qs + phoneNumbers;

	openURL(address);
}

void OSInterface::sendMail(const QString& address, const QString& subject, const QString& attachment_file) const
{
	#ifdef Q_OS_ANDROID
	if (!androidSendMail(address, subject, attachment_file))
	{
		if (appUserModel()->email(0).contains(u"gmail.com"_qs))
		{
			const QString& gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_qs.arg(appUserModel()->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList& args (QStringList() <<
		u"--utf8"_qs << u"--subject"_qs << QChar('\'') + subject + QChar('\'') << u"--attach"_qs << attachment_file <<
			QChar('\'') + address + QChar('\''));
	auto* __restrict proc(new QProcess ());
	proc->start(u"xdg-email"_qs, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exitCode, QProcess::ExitStatus)
	{
		if (exitCode != 0)
		{
			if (appUserModel()->email(0).contains(u"gmail.com"_qs))
			{
				const QString& gmailURL(u"https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3"_qs.arg(appUserModel()->email(0), address, subject));
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
	const QString& localFile{m_appDataFilesPath + u"tempfile"_qs + filename.last(4)};
	static_cast<void>(QFile::remove(localFile));
	if (QFile::copy(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "coud not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}
