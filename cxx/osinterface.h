#ifndef OSINTERFACE_H
#define OSINTERFACE_H

#include <QObject>
#include <QFile>

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"
#include <jni.h>

class TPAndroidNotification;
#endif

class TPListModel;

class OSInterface : public QObject
{

Q_OBJECT

public:
	explicit OSInterface(QObject* parent = nullptr);
	inline OSInterface(const OSInterface& other)
		: QObject{other.parent()}, m_appDataFilesPath(other.m_appDataFilesPath) {}
	inline ~OSInterface()
	{
	#ifdef Q_OS_ANDROID
		delete m_AndroidNotification;
	#endif
	}

	Q_INVOKABLE void exitApp();

	inline const QString& appDataFilesPath() const { return m_appDataFilesPath; }
	inline void initialCheck() const
	{
		#ifdef Q_OS_ANDROID
			checkPendingIntents();
		#else
			processArguments();
		#endif
	}

#ifdef Q_OS_ANDROID
	void setFileUrlReceived(const QString& url) const;
	void setFileReceivedAndSaved(const QString& url) const;
	bool checkFileExists(const QString& url) const;
	void onActivityResult(int requestCode, int resultCode);
	void startNotificationAction(const QString& action);

	void checkPendingIntents() const;
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const;
	void androidOpenURL(const QString& address) const;
	bool androidSendMail(const QString& address, const QString& subject, const QString& attachment) const;
	bool viewFile(const QString& filePath, const QString& title) const;
	QString readFileFromAndroidFileDialog(const QString& android_uri) const;
	void appStartUpNotifications();
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

public slots:
	void aboutToExit();

private:
	QString m_appDataFilesPath;

#ifdef Q_OS_ANDROID
	TPAndroidNotification* m_AndroidNotification;
#endif

	static OSInterface* app_os_interface;
	friend OSInterface* appOsInterface();
};
Q_DECLARE_METATYPE(OSInterface*)

inline OSInterface* appOsInterface() { return OSInterface::app_os_interface; }
#endif // OSINTERFACE_H
