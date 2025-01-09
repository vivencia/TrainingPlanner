#include "tpandroidnotification.h"

#ifdef Q_OS_ANDROID
#include "osinterface.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qjniobject.h>
#include <QtCore/private/qandroidextras_p.h>

using namespace Qt::Literals::StringLiterals;

TPAndroidNotification::TPAndroidNotification(QObject* parent)
	: QObject{parent}
{
	if (QNativeInterface::QAndroidApplication::sdkVersion() >= __ANDROID_API_T__)
	{
		auto requestResult{QtAndroidPrivate::requestPermission("android.permission.POST_NOTIFICATIONS"_L1)};
		if (requestResult.result() != QtAndroidPrivate::Authorized)
			qWarning() << "Failed to acquire permission to post notifications (required for Android 13+)";
	}
}

void TPAndroidNotification::sendNotification(notificationData* data)
{
	const QJniObject& jtitle{QJniObject::fromString(data->title)};
	const QJniObject& jmessage{QJniObject::fromString(data->message)};

	const jint ok = QJniObject::callStaticMethod<jint>(
					"org/vivenciasoftware/TrainingPlanner/NotificationClient",
					"notify",
					"(Ljava/lang/String;Ljava/lang/String;I)I",
					jtitle.object<jstring>(), jmessage.object<jstring>(), static_cast<int>(data->action));
	data->id = static_cast<short>(ok);
}

void TPAndroidNotification::cancelNotification(const short id)
{
	QJniObject::callStaticMethod<void>(
					"org/vivenciasoftware/TrainingPlanner/NotificationClient",
					"cancelNotify",
					"(I)V",
					static_cast<int>(id));
}
#endif
