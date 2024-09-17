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
public:
	OSInterface();
	~OSInterface();
	Q_INVOKABLE void exitApp();

	inline const QString& appDataFilesPath() const { return m_appDataFilesPath; }
#ifndef Q_OS_ANDROID
	Q_INVOKABLE void processArguments();
	Q_INVOKABLE void restartApp();
#else
	void setFileUrlReceived(const QString& url) const;
	void setFileReceivedAndSaved(const QString& url) const;
	bool checkFileExists(const QString& url) const;
	void onActivityResult(int requestCode, int resultCode);
	void startNotificationAction(const QString& action);

	Q_INVOKABLE void checkPendingIntents() const;
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const;
	void androidOpenURL(const QString& address) const;
	bool androidSendMail(const QString& address, const QString& subject, const QString& attachment) const;
	bool viewFile(const QString& filePath, const QString& title) const;
	void appStartUpNotifications();
#endif

	void setExportFileName(const QString& filename) { m_exportFileName = m_appDataFilesPath + filename;}
	inline const QString& exportFileName() const { return m_exportFileName; }
	void openRequestedFile(const QString& filename);
	bool exportToFile(const TPListModel* model, const QString& filename, const bool bShare, QFile* outFile = nullptr) const;
	Q_INVOKABLE int importFromFile(QString filename, QFile* inFile = nullptr);
	bool importFromModel(TPListModel* model);

	Q_INVOKABLE void saveFileDialogClosed(QString finalFileName, bool bResultOK);
	Q_INVOKABLE int parseFile(QString filename);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo);
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
	QString m_exportFileName, m_appDataFilesPath;

#ifdef Q_OS_ANDROID
	TPAndroidNotification* m_AndroidNotification;
#endif
};

#endif // OSINTERFACE_H
