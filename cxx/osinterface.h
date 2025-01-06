#ifndef OSINTERFACE_H
#define OSINTERFACE_H

#include <QObject>
#include <QFile>

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"
#include <QDate>
#include <QTime>
#include <jni.h>

QT_FORWARD_DECLARE_CLASS(TPAndroidNotification)

using namespace Qt::Literals::StringLiterals;

struct notificationData {
	short id;
	short action;
	bool resolved;
	QDateTime start_time;
	QDateTime expiration;
	QString message;
	QString title;

	explicit inline notificationData(): id{0}, action{0}, resolved{false},
			start_time{QDate::currentDate(), QTime::currentTime()}, title{"TrainingPlanner"_L1} {}
};
#endif

QT_FORWARD_DECLARE_CLASS(TPListModel)

class OSInterface : public QObject
{

Q_OBJECT

public:
	explicit OSInterface(QObject* parent = nullptr);
	inline ~OSInterface()
	{
	#ifdef Q_OS_ANDROID
		delete m_AndroidNotification;
	#endif
	}

	inline const QString& appDataFilesPath() const { return m_appDataFilesPath; }
	inline void initialCheck()
	{
		#ifdef Q_OS_ANDROID
			checkPendingIntents();
			startAppNotifications();
		#else
			processArguments();
		#endif
	}

#ifdef Q_OS_ANDROID
	void setFileUrlReceived(const QString& url) const;
	void setFileReceivedAndSaved(const QString& url) const;
	bool checkFileExists(const QString& url) const;
	void onActivityResult(int requestCode, int resultCode);
	void execNotification(const short action, const short id);
	void removeNotification(notificationData* data);

	void checkPendingIntents() const;
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const;
	void androidOpenURL(const QString& address) const;
	bool androidSendMail(const QString& address, const QString& subject, const QString& attachment) const;
	bool viewFile(const QString& filePath, const QString& title) const;
	QString readFileFromAndroidFileDialog(const QString& android_uri) const;
	void startAppNotifications();
	void checkNotificationsStatus();
	void checkWorkouts();
#else
	void processArguments() const;
	Q_INVOKABLE void restartApp();
#endif

	void shareFile(const QString& fileName) const;
	Q_INVOKABLE void openURL(const QString& address) const;
	Q_INVOKABLE void startChatApp(const QString& phone, const QString& appname) const;
	Q_INVOKABLE void sendMail(const QString& address, const QString& subject, const QString& attachment_file) const;
	Q_INVOKABLE void viewExternalFile(const QString& filename) const;

signals:
#ifdef Q_OS_ANDROID
	void activityFinishedResult(const int requestCode, const int resultCode);
#endif
	void appSuspended();
	void appResumed();

public slots:
	void aboutToExit();

private:
	QString m_appDataFilesPath;

#ifdef Q_OS_ANDROID
	TPAndroidNotification* m_AndroidNotification;
	bool mb_appSuspended;
	QList<notificationData*> m_notifications;
#endif

	static OSInterface* app_os_interface;
	friend OSInterface* appOsInterface();
};

inline OSInterface* appOsInterface() { return OSInterface::app_os_interface; }
#endif // OSINTERFACE_H
