#include "osinterface.h"
#include "tpappcontrol.h"
#include "tputils.h"
#include "dbinterface.h"
#include "dbusermodel.h"

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"

#include <QJniObject>
#include <QtGlobal>
#include <qnativeinterface.h>
	#if QT_VERSION == QT_VERSION_CHECK(6, 7, 2)
		#include <QtCore/6.7.2/QtCore/private/qandroidextras_p.h>
	#else
		#include <QtCore/6.6.3/QtCore/private/qandroidextras_p.h>
	#endif
#else
#include <QProcess>
extern "C"
{
	#include <unistd.h>
}
#endif

#include <QGuiApplication>
#include <QFileInfo>
#include <QStandardPaths>

OSInterface::OSInterface(QObject* parent)
	: QObject{parent}
{
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
	appControl()->cleanUp();
}

#ifdef Q_OS_ANDROID
void OSInterface::checkPendingIntents() const
{
	QJniObject activity = QNativeInterface::QAndroidApplication::context();
	if(activity.isValid())
	{
		activity.callMethod<void>("checkPendingIntents","()V");
		return;
	}
	MSG_OUT("checkPendingIntents: Activity not valid")
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
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	QJniObject jsMimeType = QJniObject::fromString(mimeType);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendFile",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>(), jsMimeType.object<jstring>(), requestId);
	if(!ok)
	{
		MSG_OUT("Unable to resolve activity from Java")
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

	QJniObject jsPath = QJniObject::fromString(url);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"openURL",
													"(Ljava/lang/String;)Z",
													jsPath.object<jstring>());
	if(!ok)
		MSG_OUT("Unable to open the address: " << address)
}

bool OSInterface::androidSendMail(const QString& address, const QString& subject, const QString& attachment) const
{
	const QString attachment_file(attachment.isEmpty() ? QString() : u"file://" + attachment);
	QJniObject jsAddress = QJniObject::fromString(address);
	QJniObject jsSubject = QJniObject::fromString(subject);
	QJniObject jsAttach = QJniObject::fromString(attachment_file);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendEmail",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
													jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>());
	return ok;
}

bool OSInterface::viewFile(const QString& filePath, const QString& title) const
{
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"viewFile",
													"(Ljava/lang/String;Ljava/lang/String;)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>());
	if(!ok)
	{
		MSG_OUT("Unable to resolve view activity from Java")
		return false;
	}
	return true;
}

void OSInterface::appStartUpNotifications()
{
	m_AndroidNotification = new TPAndroidNotification(this);
	if (appMesoModel()->count() > 0)
	{
		DBMesoCalendarTable* calTable(new DBMesoCalendarTable(m_DBFilePath));
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			if (dayInfoList.at(0).toUInt() == m_currentMesoManager->mesoId())
			{
				QString message;
				const QString splitLetter(dayInfoList.at(2));
				if (splitLetter != u"R"_qs) //day is training day
				{
					if (dayInfoList.at(3) == u"1"_qs) //day is completed
						message = tr("Your training routine seems to go well. Workout for the day is concluded");
					else
						message = tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter;
				}
				else
					message = tr("Enjoy your day of rest from workouts!");
				m_AndroidNotification->sendNotification(u"Training Planner"_qs, message, WORKOUT_NOTIFICATION);
			}
		}
		delete calTable;
	}
}

void OSInterface::setFileUrlReceived(const QString &url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("setFileUrlReceived: we got an empty URL");
		return;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
		appControl()->openRequestedFile(androidUrl);
	else
		MSG_OUT("setFileUrlReceived: FILE does NOT exist ")
}

void OSInterface::setFileReceivedAndSaved(const QString& url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("setFileReceivedAndSaved: we got an empty URL");
		return;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
		appControl()->openRequestedFile(androidUrl);
	else
		MSG_OUT("setFileReceivedAndSaved: FILE does NOT exist ")
}

bool OSInterface::checkFileExists(const QString& url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("checkFileExists: we got an empty URL");
		return false;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
	{
		MSG_OUT("Yep: the File exists for Qt");
		return true;
	}
	else
	{
		MSG_OUT("Uuups: FILE does NOT exist ");
		return false;
	}
}

void OSInterface::onActivityResult(int requestCode, int resultCode)
{
	// we're getting RESULT_OK only if edit is done
	if (resultCode == -1)
		MSG_OUT("Send Activity Result OK")
	else if (resultCode == 0)
		MSG_OUT("Send Activity Result Canceled")
	else
		MSG_OUT("Send Activity wrong result code: " << resultCode << " from request: " << requestCode)
	emit activityFinishedResult(requestCode, resultCode);
}

void OSInterface::startNotificationAction(const QString& action)
{
	if (action.toUInt() == WORKOUT_NOTIFICATION)
		m_appDB->getTrainingDay(QDate::currentDate());
}

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileUrlReceived(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	handlerInstance()->setFileUrlReceived(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileReceivedAndSaved(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	handlerInstance()->setFileReceivedAndSaved(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT bool JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_checkFileExists(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	bool exists = handlerInstance()->checkFileExists(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return exists;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_fireActivityResult(
						JNIEnv *env, jobject obj, jint requestCode, jint resultCode)
{
	Q_UNUSED (obj)
	Q_UNUSED (env)
	handlerInstance()->onActivityResult(requestCode, resultCode);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_notificationActionReceived(
						JNIEnv *env, jobject obj, jstring action)
{
	Q_UNUSED (obj)
	const char *actionStr = env->GetStringUTFChars(action, NULL);
	handlerInstance()->startNotificationAction(actionStr);
	env->ReleaseStringUTFChars(action, actionStr);
	return;
}

#ifdef __cplusplus
}
#endif

#else
void OSInterface::processArguments()
{
	const QStringList args(qApp->arguments());
	if (args.count() > 1)
	{
		QString filename;
		for (uint i(1); i < args.count(); ++i)
			filename += args.at(i) + ' ';
		filename.chop(1);
		const QFileInfo file(filename);
		if (file.isFile())
			appControl()->openRequestedFile(filename);
	}
}

void OSInterface::restartApp()
{
	char* args[2] = { nullptr, nullptr };
	const QString argv0(qApp->arguments().at(0));
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	exitApp();
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
	const QString::const_iterator itr_end(phone.constEnd());
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
			const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(appUserModel()->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList args (QStringList() <<
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
				const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(appUserModel()->email(0), address, subject));
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
	const QString localFile(mAppDataFilesPath + u"tempfile"_qs + filename.right(4));
	static_cast<void>(QFile::remove(localFile));
	if (QFile::copy(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "coud not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}

#endif //Q_OS_ANDROID
