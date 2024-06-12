#include "urihandler.h"

#ifdef Q_OS_ANDROID

#include "dbmanager.h"

#include <jni.h>

URIHandler* URIHandler::s_instance(nullptr);

URIHandler::URIHandler(DbManager* appDB, QObject* parent)
		: QObject(parent), m_appDB(appDB)
{
	if (!URIHandler::s_instance)
		URIHandler::s_instance = this;
}

void URIHandler::setFileUrlReceived(const QString &url) const
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
		m_appDB->openRequestedFile(androidUrl);
	else
		MSG_OUT("setFileUrlReceived: FILE does NOT exist ")
}

void URIHandler::setFileReceivedAndSaved(const QString& url) const
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
		//emit fileReceivedAndSaved(androidUrl)
		;
	else
		MSG_OUT("setFileReceivedAndSaved: FILE does NOT exist ")
}

bool URIHandler::checkFileExists(const QString& url) const
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

#ifdef __cplusplus
}
#endif

#endif //Q_OS_ANDROID
