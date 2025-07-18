#pragma once

#ifndef QT_NO_DEBUG
#include "tputils.h"
#endif

#include <QObject>
#include <QFile>

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"
#include <QDate>
#include <QTime>
#include <jni.h>

//QT_FORWARD_DECLARE_CLASS(TPAndroidNotification)

using namespace Qt::Literals::StringLiterals;

struct notificationData {
	short id;
	short action;
	bool resolved;
	QDateTime start_time;
	QDateTime expiration;
	QString message;
	QString title;

	explicit inline notificationData(): id{0}, action{0}, resolved{false}, start_time{QDate::currentDate(), QTime::currentTime()} {}
};
#endif

QT_FORWARD_DECLARE_CLASS(TPListModel)
QT_FORWARD_DECLARE_CLASS(QTimer);

enum SERVER_STATUS {
	SERVER_UP_AND_RUNNING = 0,
	SERVER_UNREACHABLE = 1
};

enum INTERNET_STATUS {
	HAS_INTERNET = 2,
	NO_INTERNET_ACCESS = 3
};

class OSInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool internetOK READ internetOK NOTIFY internetStatusChanged FINAL)
Q_PROPERTY(bool tpServerOK READ tpServerOK NOTIFY serverStatusChanged FINAL)

public:
	explicit OSInterface(QObject *parent = nullptr);
	inline ~OSInterface()
	{
	#ifdef Q_OS_ANDROID
		delete m_AndroidNotification;
	#endif
	}

	inline bool internetConnectionCheckInPlace() const { return m_bchecking_ic; }
	void checkInternetConnection();
	inline bool internetOK() const
	{
#ifndef QT_NO_DEBUG
		const bool internet_ok{isBitSet(m_networkStatus, HAS_INTERNET)};
		return internet_ok;
#else
		return isBitSet(m_networkStatus, HAS_INTERNET);
#endif
	}

	inline bool tpServerOK() const
	{
#ifndef QT_NO_DEBUG
		const bool server_ok{isBitSet(m_networkStatus, SERVER_UP_AND_RUNNING)};
		return server_ok;
#else
		return isBitSet(m_networkStatus, SERVER_UP_AND_RUNNING);
#endif
	}

	inline int networkStatus() const { return m_networkStatus; }
	void setNetworkStatus(int new_internetstatus, int new_serverstatus);

	inline void initialCheck()
	{
		#ifdef Q_OS_ANDROID
			checkPendingIntents();
			startAppNotifications();
		#else
			#ifdef Q_OS_LINUX
				processArguments();
			#endif
		#endif
		checkOnlineResources();
	}

#ifdef Q_OS_ANDROID
	void setFileUrlReceived(const QString &url) const;
	void setFileReceivedAndSaved(const QString &url) const;
	bool checkFileExists(const QString &url) const;
	void onActivityResult(int requestCode, int resultCode);
	void execNotification(const short action, const short id);
	void removeNotification(notificationData *data);

	void checkPendingIntents() const;
	bool sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId) const;
	void androidOpenURL(const QString &address) const;
	bool androidSendMail(const QString &address, const QString &subject, const QString &attachment) const;
	bool viewFile(const QString &filePath, const QString &title) const;
	QString readFileFromAndroidFileDialog(const QString &android_uri) const;
	void startAppNotifications();
	void checkNotificationsStatus();
	void checkWorkouts();
#else
	#ifdef Q_OS_LINUX
		QString executeAndCaptureOutput(const QString &program, QStringList &arguments, const bool b_asRoot = false, int *exitCode = nullptr);
		void checkLocalServer();
		void commandLocalServer(const QString &command);
		void processArguments() const;
		Q_INVOKABLE void restartApp();
	#endif
#endif

	void checkOnlineResources();
	void shareFile(const QString &fileName) const;
	Q_INVOKABLE void openURL(const QString &address) const;
	Q_INVOKABLE void startChatApp(const QString &phone, const QString &appname) const;
	Q_INVOKABLE void sendMail(const QString &address, const QString &subject, const QString &attachment_file) const;
	Q_INVOKABLE void viewExternalFile(const QString &filename) const;

signals:
#ifdef Q_OS_ANDROID
	void activityFinishedResult(const int requestCode, const int resultCode);
#endif
	void appAboutToExit();
	void appSuspended();
	void appResumed();
	void internetStatusChanged(const bool connected);
	void serverStatusChanged(const bool online);

public slots:
	void aboutToExit();
	void checkServerResponseSlot(const bool online);

private:
	int m_networkStatus;
	QTimer *m_checkConnectionTimer;
	bool m_bchecking_ic;

#ifdef Q_OS_ANDROID
	TPAndroidNotification *m_AndroidNotification;
	bool mb_appSuspended, m_bTodaysWorkoutFinishedConnected;
	QList<notificationData*> m_notifications;
	QTimer *m_notificationsTimer;
#endif

	static OSInterface *app_os_interface;
	friend OSInterface *appOsInterface();
};

inline OSInterface *appOsInterface() { return OSInterface::app_os_interface; }
