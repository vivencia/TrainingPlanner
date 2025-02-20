#ifndef TPONLINESERVICES_H
#define TPONLINESERVICES_H

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

	void checkServer(int network_status);
	void checkOnlineUser(const QString &query, const QString &passwd);
	void getOnlineUserData(const QString &user_id);
	void checkUser(const QString &username, const QString &passwd);
	void registerUser(const QString &username, const QString &passwd);
	void updateOnlineUserInfo(const QString &username, const QString &passwd, QFile *file);
	void removeUser(const QString &username);
	void alterUser(const QString &old_username, const QString &new_username, const QString &new_passwd);
	void addOrRemoveCoach(const QString &username, const QString &passwd, const bool bAdd);
	void sendRequestToCoach(const QString &username, const QString &passwd, const QString& coach_net_name);
	void checkClientsRequests(const QString &username, const QString &passwd);

	void sendFile(const QString &username, const QString &passwd, QFile *file,
					const QString &targetUser = QString{}, const bool b_internal_signal_only = false);
	void getFile(const QString &username, const QString &passwd, const QString &file, const QString &targetUser = QString{});

	/**
	 * @brief getBinFile
	 * @param username
	 * @param passwd
	 * @param filename Must contain an extension when c_time is not null. And, conversely, must not contain an extension when c_time is valid
	 * @param targetUser
	 * @param c_time
	 */
	void getBinFile(const QString &username, const QString &passwd, const QString &filename,
					const QString &targetUser, const QDateTime &c_time);
	void getCoachesList(const QString &username, const QString &passwd);

signals:
	void networkRequestProcessed(const int ret_code, const QString &ret_string);
	void _networkRequestProcessed(const int ret_code, const QString &ret_string);
	void fileReceived(const int ret_code, const QString& filename, const QByteArray &contents);
	void serverOnline(const bool online, int network_status);

private:
	void makeNetworkRequest(const QUrl &url, const bool b_internal_signal_only = false);
	void handleServerRequestReply(QNetworkReply *reply, const bool b_internal_signal_only = false);
	void uploadFile(const QUrl &url, QFile *file, const bool b_internal_signal_only = false);

	QNetworkAccessManager *m_networkManager;

	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
#endif // TPONLINESERVICES_H
