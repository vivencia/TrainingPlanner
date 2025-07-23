#pragma once

#include <QObject>
#include <QNetworkAccessManager>

QT_FORWARD_DECLARE_CLASS(QNetworkReply)
QT_FORWARD_DECLARE_CLASS(QFile)

class TPOnlineServices : public QObject
{

Q_OBJECT

public:
	inline explicit TPOnlineServices(QObject *parent = nullptr) : QObject{parent}
	{
		if (!_appOnlineServices)
			_appOnlineServices = this;
		m_networkManager = new QNetworkAccessManager{this};
	}

	inline ~TPOnlineServices() { delete m_networkManager; }

	void checkServer();
	void checkOnlineUser(const int requestid, const QString &query, const QString &passwd);
	void getOnlineUserData(const int requestid, const QString &user_id);
	void checkUser(const int requestid, const QString &username, const QString &passwd);
	void registerUser(const int requestid, const QString &username, const QString &passwd);
	void updateOnlineUserInfo(const int requestid, const QString &username, const QString &passwd, QFile *file);
	void removeUser(const int requestid, const QString &username);
	void changePassword(const int requestid, const QString &username, const QString &old_passwd, const QString &new_passwd);

	void addOrRemoveCoach(const int requestid, const QString &username, const QString &passwd, const bool bAdd);
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

	void sendFile(const int requestid, const QString &username, const QString &passwd, QFile *file, const QString &subdir = QString{},
					const QString &targetUser = QString{}, const bool b_internal_signal_only = false);
	void listFiles(const int requestid, const QString &username, const QString &passwd, const QString &subdir = QString{},
					const QString &targetUser = QString{});
	void removeFile(const int requestid, const QString &username, const QString &passwd, const QString &filename, const QString &subdir = QString{},
					const QString &targetUser = QString{});

	/**
	 * @brief getFile
	 * @param username
	 * @param passwd
	 * @param filename Must contain an extension when c_time is not null. And, conversely, must not contain an extension when c_time is valid
	 * @param targetUser
	 * @param c_time
	 */
	void getFile(const int requestid, const QString &username, const QString &passwd, const QString &filename, const QString &subdir = {},
					const QString &targetUser = QString{}, const QString &localFilePath = QString{});
	void getOnlineCoachesList(const int requestid, const QString &username, const QString &passwd);

signals:
	void networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);
	void _networkRequestProcessed(const int request_id, const int ret_code, const QString &ret_string);
	void networkListReceived(const int request_id, const int ret_code, const QStringList &ret_list);
	void fileReceived(const int request_id, const int ret_code, const QString& filename, const QByteArray &contents);
	void serverOnline(const bool online);

private:
	void makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only = false);
	void handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only = false);
	void uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only = false);
	bool remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const;
	bool localFileUpToDate(const QString &onlineDate, const QString &localFile) const;

	QNetworkAccessManager *m_networkManager;

	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
