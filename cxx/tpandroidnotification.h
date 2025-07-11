#pragma once

#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QList>
#include <QMap>
#include <QObject>

QT_FORWARD_DECLARE_STRUCT(notificationData);

class TPAndroidNotification : public QObject
{
Q_OBJECT

public:
	explicit TPAndroidNotification(QObject *parent = nullptr);
	void sendNotification(notificationData *data);
	void cancelNotification(const short id);
};

#endif
