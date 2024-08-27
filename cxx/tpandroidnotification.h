#ifndef TPANDROIDNOTIFICATION_H
#define TPANDROIDNOTIFICATION_H

#include <qglobal.h>

#ifdef Q_OS_ANDROID
#include <QObject>

class TPAndroidNotification : public QObject
{
Q_OBJECT

public:
	explicit TPAndroidNotification(QObject* parent = nullptr);
	void sendNotification(const QString& title, const QString& message);
};
#endif

#endif // TPANDROIDNOTIFICATION_H
