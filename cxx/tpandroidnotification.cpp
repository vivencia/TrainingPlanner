#include "tpandroidnotification.h"

#ifdef Q_OS_ANDROID
#include <QtCore/qjniobject.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/private/qandroidextras_p.h>

using namespace Qt::StringLiterals;

TPAndroidNotification::TPAndroidNotification(QObject* parent)
	: QObject{parent}
{
	if (QNativeInterface::QAndroidApplication::sdkVersion() >= __ANDROID_API_T__) {
		const auto notificationPermission = "android.permission.POST_NOTIFICATIONS"_L1;
		auto requestResult = QtAndroidPrivate::requestPermission(notificationPermission);
		if (requestResult.result() != QtAndroidPrivate::Authorized) {
			qWarning() << "Failed to acquire permission to post notifications "
						  "(required for Android 13+)";
		}
	}
}

void TPAndroidNotification::sendNotification(const QString& title, const QString& message)
{
	QJniObject jtitle = QJniObject::fromString(title);
	QJniObject jmessage = QJniObject::fromString(message);
	QJniObject::callStaticMethod<void>(
					"org/vivenciasoftware/TrainingPlanner/NotificationClient",
					"notify",
					"(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
					QNativeInterface::QAndroidApplication::context(),
					jtitle.object<jstring>(), jmessage.object<jstring>());
}
#endif
