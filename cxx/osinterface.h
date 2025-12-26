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
#else
#include <QProcess>
#endif

QT_FORWARD_DECLARE_CLASS(TPListModel)
QT_FORWARD_DECLARE_CLASS(QTimer);
QT_FORWARD_DECLARE_CLASS(QNetworkInterface)

class OSInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool internetOK READ internetOK NOTIFY internetStatusChanged FINAL)
Q_PROPERTY(bool tpServerOK READ tpServerOK NOTIFY tpServerStatusChanged FINAL)
Q_PROPERTY(QString connectionMessage READ connectionMessage NOTIFY connectionMessageChanged FINAL)

static constexpr short HAS_INTERFACE			{0};
static constexpr short NO_INTERFACE_RUNNING		{1};
static constexpr short HAS_INTERNET				{2};
static constexpr short NO_INTERNET_ACCESS		{3};
static constexpr short SERVER_UP_AND_RUNNING	{4};
static constexpr short SERVER_UNREACHABLE		{5};

public:
	explicit OSInterface(QObject *parent = nullptr);
	inline ~OSInterface()
	{
	#ifdef Q_OS_ANDROID
		delete m_AndroidNotification;
	#endif
	}

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

	inline bool tpServerOK() const
	{
#ifndef QT_NO_DEBUG
		const bool server_ok{isBitSet(m_networkStatus, SERVER_UP_AND_RUNNING)};
		return server_ok;
#else
		return isBitSet(m_networkStatus, SERVER_UP_AND_RUNNING);
#endif
	}
	QString connectionMessage() const { return m_connectionMessages.join('\n'); }

	inline int networkStatus() const { return m_networkStatus; }

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
		void serverProcessFinished(QProcess *proc, const int exitCode, QProcess::ExitStatus exitStatus);
		void checkLocalServer();
		void commandLocalServer(const QString &message, const QString &command);
		void processArguments() const;
		Q_INVOKABLE void restartApp();
	#endif
#endif

	QString deviceID() const;
	void shareFile(const QString &fileName) const;
	Q_INVOKABLE void openURL(const QString &address) const;
	Q_INVOKABLE void startMessagingApp(const QString &phone, const QString &appname) const;
	Q_INVOKABLE void sendMail(const QString &address, const QString &subject, const QString &attachment_file) const;
	Q_INVOKABLE void viewExternalFile(const QString &filename) const;

signals:
#ifdef Q_OS_ANDROID
	void activityFinishedResult(const int requestCode, const int resultCode);
#endif
	void appSuspended();
	void appResumed();
	void internetStatusChanged();
	void tpServerStatusChanged(const bool online);
	void connectionMessageChanged();

private:
	int m_networkStatus;
	QTimer *m_checkConnectionTimer;
	QStringList m_connectionMessages;
	QString m_localIPAddress;
	std::optional<bool> m_currentNetworkStatus[3];

#ifndef Q_OS_ANDROID
	const QNetworkInterface *m_currentNetInterface;
#else
	TPAndroidNotification *m_AndroidNotification;
	bool mb_appSuspended, m_bTodaysWorkoutFinishedConnected;
	QList<notificationData*> m_notifications;
	QTimer *m_notificationsTimer;
#endif

	void setNetStatus(uint messages_index, bool success, QString &&message);
	void checkNetworkInterfaces();
	void checkInternetConnection();
	void setConnectionMessage(int msg_idx, QString &&message);
	void onlineServicesResponse(const uint online_status, const QString &additional_message = QString{});
	static OSInterface *app_os_interface;
	friend OSInterface *appOsInterface();
};

inline OSInterface *appOsInterface() { return OSInterface::app_os_interface; }
