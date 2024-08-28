#ifndef TPANDROIDNOTIFICATION_H
#define TPANDROIDNOTIFICATION_H

#include <qglobal.h>

#ifdef Q_OS_ANDROID
#include <QObject>
#include <QMap>
#include <QList>

#define GENERAL_TP_NOTIFICATION 0xA
#define MESOCYCLE_NOTIFICATION 0x14
#define SPLIT_NOTIFICATION 0x1E
#define CALENDAR_NOTIFICATION 0x28
#define WORKOUT_NOTIFICATION 0x32
#define USER_NOTIFICATION 0x3C

class TPAndroidNotification : public QObject
{
Q_OBJECT

public:
	explicit TPAndroidNotification(QObject* parent = nullptr);
	~TPAndroidNotification();
	uint sendNotification(const QString& title, const QString& message, const uint table_id = 0);
	void cancelNotification(const uint id);

private:
	QMap<uint, QList<uint>> m_Ids;
};

#endif

#endif // TPANDROIDNOTIFICATION_H
