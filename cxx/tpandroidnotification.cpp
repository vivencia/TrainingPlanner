#include "tpandroidnotification.h"

#ifdef Q_OS_ANDROID
#include "tplistmodel.h"

#include <QtCore/qjniobject.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/private/qandroidextras_p.h>

using namespace Qt::StringLiterals;

TPAndroidNotification::TPAndroidNotification(QObject* parent)
	: QObject{parent}
{
	if (QNativeInterface::QAndroidApplication::sdkVersion() >= __ANDROID_API_T__)
	{
		const auto notificationPermission = "android.permission.POST_NOTIFICATIONS"_L1;
		auto requestResult = QtAndroidPrivate::requestPermission(notificationPermission);
		if (requestResult.result() != QtAndroidPrivate::Authorized)
		{
			qWarning() << "Failed to acquire permission to post notifications "
						  "(required for Android 13+)";
		}
	}
}

TPAndroidNotification::~TPAndroidNotification()
{
	return;
	QMapIterator<uint, QList<uint>> it(m_Ids);
	it.toFront();
	while (it.hasNext())
	{
		it.next();
		for (uint i(0); i < it.value().count(); ++i)
			cancelNotification(it.value().at(i));
	}
}

uint TPAndroidNotification::sendNotification(const QString& title, const QString& message, const uint table_id)
{
	const uint id(m_Ids.value(table_id).isEmpty() ? 0 : m_Ids.value(table_id).constLast() + 1);

	QJniObject jtitle = QJniObject::fromString(title);
	QJniObject jmessage = QJniObject::fromString(message);
	QJniObject jaction = QJniObject::fromString(QString::number(table_id));
	QJniObject::callStaticMethod<void>(
					"org/vivenciasoftware/TrainingPlanner/NotificationClient",
					"notify",
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V",
					jtitle.object<jstring>(), jmessage.object<jstring>(), jaction.object<jstring>(), id);
	m_Ids[table_id].append(id);
	return id;
}

void TPAndroidNotification::cancelNotification(const uint id)
{
	QJniObject::callStaticMethod<void>(
					"org/vivenciasoftware/TrainingPlanner/NotificationClient",
					"cancelNotify",
					"(I)V",
					id);
}
#endif
