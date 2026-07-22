#pragma once

#include "../tpbool.h"

#include <QObject>
#include <QNetworkAccessManager>

QT_FORWARD_DECLARE_CLASS(TPFilePath)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)
QT_FORWARD_DECLARE_CLASS(QFile)

/**
 * @brief The TPOnlineServices class is a front-end to the file url_parser.php.
 */

class TPOnlineServices : public QObject
{

Q_OBJECT

public:
	explicit TPOnlineServices(QObject *parent = nullptr);
	inline ~TPOnlineServices() { delete m_networkManager; }

	inline uint8_t serverStatus() const { return m_onlineStatus; }
	void connectToServer();

#ifndef Q_OS_ANDROID
	void getAllUsers(const int requestid);
#endif
	void checkUserAccount(const int requestid, const QString &query, const QString &passwd);
	void getOnlineUserData(const int requestid, const QString &user_id);
	void userLogin(const int requestid);
	void userLogout(const int requestid);
	void registerUser(const int requestid);
	void removeUser(const int requestid, const QString &userid);

	void getPeerAddress(const int requestid, const QString &userid);
	void getDevicesList(const int requestid);
	void changePassword(const int requestid, const QString &old_passwd, const QString &new_passwd);
	void addOrRemoveCoach(const int requestid, const bool bAdd);
	void getOnlineCoachesList(const int requestid);
	void sendRequestToCoach(const int requestid, const QString& coach_net_name);
	void checkClientsRequests(const int requestid);
	void removeClientRequest(const int requestid, const QString &client);
	void acceptClientRequest(const int requestid, const QString &client);
	void rejectClientRequest(const int requestid, const QString &client);
	void checkCoachesAnswers(const int requestid);
	void removeCoachAnwers(const int requestid, const QString &coach);
	void acceptCoachAnswer(const int requestid, const QString &coach);
	void rejectCoachAnswer(const int requestid, const QString &coach);
	void checkCurrentClients(const int requestid);
	void removeClientFromCoach(const int requestid, const QString &client);
	void checkCurrentCoaches(const int requestid);
	void removeCoachFromClient(const int requestid, const QString &coach);

	int sendFileToServer(const TPFilePath &tp_filename, const bool remove_local_file = false);
	int downloadFileFromServer(const TPFilePath &tp_filename);
	void removeFileFromServer(const TPFilePath &tp_filename);
	std::pair<TPBool,int> listFilesOrDirs(const bool files = true, const bool dirs = false, const bool admin = false,
						const QString &target_user = QString{}, const QString &subdir = QString{},
						const QString &pattern = QString{}, const bool recursive = false);
	void sendCmdFileToServer(const QString &cmd_filename);
	void downloadCmdFilesFromServer();
	void executeCommands(const int requestid, const QString &subdir);

	void checkTPMessages(const int requestid);
	void sendTPMessage(const int requestid, const QString &message, const QString &target_user);
	void removeTPMessage(const int requestid, const QString &message);

	void checkChatMessages(const int requestid);
	void sendChatMessage(const int requestid, const QString &receiver, const QString &encoded_message);
	void removeChatMessage(const int requestid, const QString &receiver, const QString &encoded_message);
	void recheckNewChatMessages();

public slots:
	void storeCredentials();

signals:
	void onlineServicesReady();
	void fileDownloaded(const uint requestid, const int ret_code, const TPFilePath &tp_filepath);
	void fileUploaded(const uint requestid, const int ret_code);
	void networkListReceived(const int request_id, const int ret_code, const QStringList &ret_list);
	void serverStatusChanged(const uint online_status, const QString &server_address, const int request_id);
	void networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);

	void _networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string,
																	const QByteArray &contents = QByteArray{});
	void fileReceived(const int request_id, const int ret_code, const QString& filename, const QByteArray &contents);

private:
	bool canConnectToServer() const;
	int serverCommandStarter(int requestid, QString &&command_description = QString{}) const;
#ifdef LOCAL_TPSERVER
	void testServerConnection(const QString &address, const QString &port, const int requestid = -1);
#endif
	QString makeCommandURL(const bool admin, auto && ... args);
	void makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only = false);
	void handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only = false);
	void sendFile(const int requestid, const TPFilePath &tp_filename, QFile *file);
	void getFile(const int requestid, const TPFilePath &tp_filename, bool check_ctime_first = true);
	void removeFile(const int requestid, const TPFilePath &tp_filename);
	void getCmdFile(const int requestid, const QString &filename, const QString &subdir = QString{});
	void uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only = false);
	void parseReceivedFilesList(QStringList &files, const QString &ret_string);
	bool remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const;
	bool localFileUpToDate(const QString &onlineDate, const QString &localFile) const;
	inline bool checkRequestPool(const int requestid, const QLatin1StringView &method) const
	{
		if (requestid == -1)
			return false;
#ifndef QT_NO_QDEBUG
		if (m_requestsPool.value(requestid))
			qDebug() << method << Qt::StringLiterals::operator""_L1(": ATENTION! Request Id already in use: ", 37) << requestid;
#endif
		return m_requestsPool.value(requestid);
	}
	inline void setRequestToPool(const int requestid, const bool in_use) { m_requestsPool[requestid] = in_use; }

	QNetworkAccessManager *m_networkManager{nullptr};
	bool m_hasCredentials{false};
	uint8_t m_onlineStatus{255};
	QString m_userid, m_passwd, m_serverAddress;
	QHash<int,TPBool> m_requestsPool;
	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
