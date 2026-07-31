#pragma once

#include "qml_singleton.h"

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
#else
#ifdef Q_OS_LINUX
#ifdef LOCAL_TPSERVER
#include <QProcess>
#define TPSERVER_MACHINE //comment to test/debug android scenarios on the development machine
#endif //LOCAL_TPSERVER
#endif //Q_OS_LINUX
#endif //Q_OS_ANDROID

QT_FORWARD_DECLARE_CLASS(TPListModel)

class OSInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool internetOK READ internetOK NOTIFY internetStatusChanged FINAL)
Q_PROPERTY(QString connectionMessage READ connectionMessage NOTIFY connectionStatusChanged FINAL)

static constexpr short HAS_INTERFACE			{0};
static constexpr short NO_INTERFACE_RUNNING		{1};
static constexpr short HAS_INTERNET				{2};
static constexpr short NO_INTERNET_ACCESS		{3};
static constexpr short SERVER_UP_AND_RUNNING	{4};
static constexpr short SERVER_UNREACHABLE		{5};

public:
	explicit OSInterface(QObject *parent = nullptr);

	void checkInternetConnection();
#ifdef LOCAL_TPSERVER
	void getAvailableAddresses();
	void setWorkingNetInterface(const int interface_index);
#ifdef TPSERVER_MACHINE
	enum clsRetCode {
		CLS_OK_STARTED, //command will start immediately
		CLS_OK_QUEUED, //command will start as soon as the previous command finishes
		CLS_OK_WAITING_FOR_PASSWORD, //command will start or enter queue after the user enters their password
		CLS_ERROR_ALREADY_QUEUED, //command will not start because the first argument is already executing or queued
		CLS_ERROR_WAITING_FOR_PASSWORD, //it may be a different command, but as long as we are waiting for the password, nothing will be done
	};
	clsRetCode commandLocalServer(const QString &command, const bool as_su = false, const QString &title = QString{});
#endif
#endif

	inline bool networkInterfaceOK() const
	{
		#ifndef QT_NO_DEBUG
		const bool interface_ok{isBitSet(m_networkStatus, HAS_INTERFACE)};
		return interface_ok;
		#else
		return isBitSet(m_networkStatus, HAS_INTERFACE);
		#endif
	}

	inline bool internetOK() const
	{
#ifndef QT_NO_DEBUG
		const bool internet_ok{isBitSet(m_networkStatus, HAS_INTERNET)};
		return internet_ok;
#else
		return isBitSet(m_networkStatus, HAS_INTERNET);
#endif
	}

	Q_INVOKABLE inline QString connectionMessage() const { return m_connectionMessages.join('\n'); }
	inline int networkStatus() const { return m_networkStatus; }

	inline void initialCheck()
	{
#ifdef Q_OS_ANDROID
		checkPendingIntents();
		startAppNotifications();
#endif
	}

#ifndef Q_OS_ANDROID
	#ifdef Q_OS_LINUX
		Q_INVOKABLE void restartApp();
		Q_INVOKABLE void sendMail(const QString &address, const QString &subject, const QString &attachment_file) const;
		Q_INVOKABLE void viewExternalFile(const QString &filename) const;
		Q_INVOKABLE void openURL(const QString &address) const;
	#endif
#endif

	QString deviceID() const;
	Q_INVOKABLE void startMessagingApp(const QString &phone, const QString &appname) const;

#ifdef Q_OS_ANDROID
	void initAndroidInterface();
	void setFileUrlReceived(const QString &url) const;
	void setFileReceivedAndSaved(const QString &url) const;
	bool checkFileExists(const QString &url) const;
	void onActivityResult(int requestCode, int resultCode);
	void execNotification(const short action, const short id);
	void removeNotification(notificationData *data);

	void checkPendingIntents() const;
	bool shareFile(const QString &filePath, const int requestId, const QString &title = QString{}, const QString &mimeType = QString{}) const;
	Q_INVOKABLE void openURL(const QString &address) const;
	Q_INVOKABLE void sendMail(const QString &address, const QString &subject, const QString &attachment_file) const;
	Q_INVOKABLE void viewExternalFile(const QString &filename) const;
	QString readFileFromAndroidFileDialog(const QString &android_uri) const;
	void startAppNotifications();
	void checkNotificationsStatus();
	void checkWorkouts();
#endif

signals:
#ifdef Q_OS_ANDROID
	void activityFinishedResult(const int requestCode, const int resultCode);
#endif
	void appSuspended();
	void appResumed();
	void internetStatusChanged();
	void connectionStatusChanged();
#ifdef LOCAL_TPSERVER
	void serverAddressesFetched(const QHash<int,QString> &addresses);
#endif

private:
	int m_networkStatus{0};
	QStringList m_connectionMessages;
	std::optional<bool> m_currentNetworkStatus[3];

#ifdef Q_OS_ANDROID
	TPAndroidNotification *m_AndroidNotification;
	bool mb_appSuspended, m_bTodaysWorkoutFinishedConnected;
	QList<notificationData*> m_notifications;
	QTimer *m_notificationsTimer;
	QString m_workoutDoneMessage
#endif

	void setNetStatus(uint messages_index, bool success, QString &&message);
	void localServerProcessResult(const uint online_status, const QString &additional_message = QString{});

#ifdef TPSERVER_MACHINE
	QProcess *m_severScriptProc{nullptr};
	QList<QStringList> m_commandQueue;
	void startLocalServerProcess();
	void serverProcessFinished(QProcess *proc, const int exit_code);
#endif //TPSERVER_MACHINE

	static OSInterface *_app_os_interface;
	friend OSInterface *appOsInterface();
};

DECLARE_QML_NAMED_SINGLETON(OSInterface, AppOsInterface)
inline OSInterface *appOsInterface() { return OSInterface::_app_os_interface; }
