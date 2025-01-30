#ifndef OSINTERFACE_H
#define OSINTERFACE_H

#include "tpglobals.h"

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

	explicit inline notificationData(): id{0}, action{0}, resolved{false}, start_time{QDate::currentDate(), QTime::currentTime()} {}
};
#endif

QT_FORWARD_DECLARE_CLASS(TPListModel)

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

Q_PROPERTY(bool internetOK READ internetOK NOTIFY networkStatusChanged FINAL)
Q_PROPERTY(bool tpServerOK READ tpServerOK NOTIFY networkStatusChanged FINAL)

public:
	explicit OSInterface(QObject* parent = nullptr);
	inline ~OSInterface()
	{
	#ifdef Q_OS_ANDROID
		delete m_AndroidNotification;
	#endif
	}

	void checkInternetConnection();
	inline bool internetOK() const { return isBitSet(m_networkStatus, HAS_INTERNET); }
	inline bool tpServerOK() const { return isBitSet(m_networkStatus, SERVER_UP_AND_RUNNING); }
	inline int networkStatus() const { return m_networkStatus; }
	inline void setNetworkStatus(int new_status) {m_networkStatus = new_status; emit networkStatusChanged(); }

	inline const QString& appDataFilesPath() const { return m_appDataFilesPath; }
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
	#ifdef Q_OS_LINUX
		QString executeAndCaptureOutput(const QString &program, QStringList &arguments, const bool b_asRoot = false, int *exitCode = nullptr);
		void configureLocalServer(bool second_pass = false);
		void processArguments() const;
		Q_INVOKABLE void restartApp();
	#endif
#endif

	void checkOnlineResources();
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
	void networkStatusChanged();

public slots:
	void aboutToExit();

private:
	QString m_appDataFilesPath;
	int m_networkStatus;

#ifdef Q_OS_ANDROID
	TPAndroidNotification* m_AndroidNotification;
	bool mb_appSuspended, m_bTodaysWorkoutFinishedConnected;
	QList<notificationData*> m_notifications;
#endif

	static OSInterface* app_os_interface;
	friend OSInterface* appOsInterface();
};

inline OSInterface* appOsInterface() { return OSInterface::app_os_interface; }
#endif // OSINTERFACE_H
