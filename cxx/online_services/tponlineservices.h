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
	explicit TPOnlineServices(QObject *parent = nullptr);
	inline ~TPOnlineServices() { delete m_networkManager; }

	//void scanNetwork(const QString &last_working_address, const bool assume_working = true);
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
	void addDevice(const int requestid, const QString &device_id);
	void delDevice(const int requestid, const QString &device_id);
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
	void executeCommands(const int requestid, const QString &subdir, const bool delete_cmdfile = true);

	void sendFile(const int requestid, QFile *file, const QString &subdir = QString{},
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
	void listFiles(const int requestid, const bool only_new = true, const bool include_ctime = false,
					const QString &pattern = QString{}, const QString &subdir = QString{}, const QString &targetUser = QString{});
	void listDirs(const int requestid, const QString &pattern = QString{}, const QString &subdir = QString{},
												const QString &targetUser = QString{}, const bool include_dot_dir = false);
	void removeFile(const int requestid, const QString &filename, const QString &subdir = QString{},
																					const QString &targetUser = QString{});
	void getFile(const int requestid, const QString &filename, const QString &subdir = {}, const QString &targetUser = QString{},
																						const QString &localFilePath = QString{});
	void getCmdFile(const int requestid, const QString &filename, const QString &subdir = QString{});

	void checkMessages(const int requestid);
	void sendMessage(const int requestid, const QString &receiver, const QString &encoded_message);
	void chatMessageWork(const int requestid, const QString &recipient, const QString &msgid, const QLatin1StringView &work);
	void chatMessageWorkAcknowledged(const int requestid, const QString &recipient, const QString &msgid, const QLatin1StringView &work);

public slots:
	void storeCredentials();

signals:
	void onlineServicesReady();
	void networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);
	void _networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string,
									const QByteArray &contents = QByteArray{});
	void networkListReceived(const int request_id, const int ret_code, const QStringList &ret_list);
	void fileReceived(const int request_id, const int ret_code, const QString& filename, const QByteArray &contents);
	//void serverOnline(const uint online_status);
	void _serverResponse(const uint online_status, const QString &address);

private:
	QString makeCommandURL(const bool admin,
						   const QLatin1StringView &option1 = QLatin1StringView{}, const QString &value1 = QString{},
						   const QLatin1StringView &option2 = QLatin1StringView{}, const QString &value2 = QString{},
						   const QLatin1StringView &option3 = QLatin1StringView{}, const QString &value3 = QString{});
	void makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only = false);
	void handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only = false);
	void uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only = false);
	//void checkServerResponse(const int ret_code, const QString &ret_string, const QString &address);
	bool remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const;
	bool localFileUpToDate(const QString &onlineDate, const QString &localFile) const;

	QNetworkAccessManager *m_networkManager;
	bool m_scanning;
	QString m_userid, m_passwd;
	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
