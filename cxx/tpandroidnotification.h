#ifndef TPANDROIDNOTIFICATION_H
#define TPANDROIDNOTIFICATION_H

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
	explicit TPAndroidNotification(QObject* parent = nullptr);
	~TPAndroidNotification();
	uint sendNotification(notificationData* data);
	void cancelNotification(const uint id);

private:
	QMap<uint, QList<uint>> m_Ids;
};

#endif

#endif // TPANDROIDNOTIFICATION_H
