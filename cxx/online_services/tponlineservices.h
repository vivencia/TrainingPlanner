#pragma once

#include <QObject>
#include <QNetworkAccessManager>

QT_FORWARD_DECLARE_CLASS(QNetworkReply)
QT_FORWARD_DECLARE_CLASS(QFile)

/**
 * @brief The TPOnlineServices class is a front-end to the file url_parser.php.
 */

class TPOnlineServices : public QObject
{

Q_OBJECT

public:
	inline explicit TPOnlineServices(QObject *parent = nullptr) : QObject{parent}
#ifndef Q_OS_ANDROID
	, m_useLocalHost{false}
#endif
	{
		if (!_appOnlineServices)
			_appOnlineServices = this;
		m_networkManager = new QNetworkAccessManager{this};
	}
	inline ~TPOnlineServices() { delete m_networkManager; }

#ifndef Q_OS_ANDROID
	inline void setUseLocalHost(const bool use_localhost) { m_useLocalHost = use_localhost; }
#else
	void checkServer();
#endif
	void checkOnlineUser(const int requestid, const QString &query, const QString &passwd);
	void getOnlineUserData(const int requestid, const QString &user_id);
	void checkUser(const int requestid, const QString &username, const QString &passwd);
	void registerUser(const int requestid, const QString &username, const QString &passwd);
	void updateOnlineUserInfo(const int requestid, const QString &username, const QString &passwd);
	void removeUser(const int requestid, const QString &username);
	void changePassword(const int requestid, const QString &username, const QString &old_passwd, const QString &new_passwd);

	void addDevice(const int requestid, const QString &username, const QString &passwd, const QString &device_id);
	void delDevice(const int requestid, const QString &username, const QString &passwd, const QString &device_id);
	void getDevicesList(const int requestid, const QString &username, const QString &passwd);
	void addOrRemoveCoach(const int requestid, const QString &username, const QString &passwd, const bool bAdd);
	void getOnlineCoachesList(const int requestid, const QString &username, const QString &passwd);
	void sendRequestToCoach(const int requestid, const QString &username, const QString &passwd, const QString& coach_net_name);
	void checkClientsRequests(const int requestid, const QString &username, const QString &passwd);
	void removeClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client);
	void acceptClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client);
	void rejectClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client);
	void checkCoachesAnswers(const int requestid, const QString &username, const QString &passwd);
	void removeCoachAnwers(const int requestid, const QString &username, const QString &passwd, const QString &coach);
	void acceptCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach);
	void rejectCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach);
	void checkCurrentClients(const int requestid, const QString &username, const QString &passwd);
	void removeClientFromCoach(const int requestid, const QString &username, const QString &passwd, const QString &client);
	void checkCurrentCoaches(const int requestid, const QString &username, const QString &passwd);
	void removeCoachFromClient(const int requestid, const QString &username, const QString &passwd, const QString &coach);
	void executeCommands(const int requestid, const QString &username, const QString &passwd, const QString &subdir, const bool delete_cmdfile = true);

	void sendFile(const int requestid, const QString &username, const QString &passwd, QFile *file, const QString &subdir = QString{},
					const QString &targetUser = QString{}, const bool b_internal_signal_only = false);
	/**
	 * @brief listFiles
	 * @param requestid An unique integer value that will be emitted with the signal networkListReceived to stabilish a chain of calls
	 * @param username
	 * @param passwd
	 * @param only_new If true, will compare the modification time stamps of both the local and the remote file that matches pattern and only include the file if the remote file is newer than the local file
	 * @param pattern If not set, matches all files within username/subdir or targetUser/subdir
	 * @param subdir
	 * @param targetUser
	 */
	void listFiles(const int requestid, const QString &username, const QString &passwd, const bool only_new = true,
					const bool include_ctime = false, const QString &pattern = QString{}, const QString &subdir = QString{}, const QString &targetUser = QString{});
	void removeFile(const int requestid, const QString &username, const QString &passwd, const QString &filename, const QString &subdir = QString{},
					const QString &targetUser = QString{});
	void getFile(const int requestid, const QString &username, const QString &passwd, const QString &filename,
				const QString &subdir = {}, const QString &targetUser = QString{}, const QString &localFilePath = QString{});
	void getCmdFile(const int requestid, const QString &username, const QString &passwd, const QString &filename,
				const QString &subdir = QString{}, const bool delete_cmd = false);

	bool remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const;
	bool localFileUpToDate(const QString &onlineDate, const QString &localFile) const;

signals:
	void networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);
	void _networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);
	void networkListReceived(const int request_id, const int ret_code, const QStringList &ret_list);
	void fileReceived(const int request_id, const int ret_code, const QString& filename, const QByteArray &contents);
	void serverOnline(const bool online);

private:
	QString makeCommandURL(const QString &username, const QString &passwd = QString{}, const QString &option1 = QString{},
								const QString &value1 = QString{}, const QString &option2 = QString{}, const QString &value2 = QString{},
								const QString &option3 = QString{}, const QString &value3 = QString{}
								);
	void makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only = false);
	void handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only = false);
	void uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only = false);

	QNetworkAccessManager *m_networkManager;
#ifndef Q_OS_ANDROID
	bool m_useLocalHost;
#endif

	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
