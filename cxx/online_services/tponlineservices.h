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

	void checkUser(const QString &username, const QString &passwd);
	void registerUser(const QString &username, const QString &passwd);
	void removeUser(const QString &username);
	void alterUser(const QString &old_username, const QString &new_username, const QString &new_passwd);
	void addOrRemoveCoach(const QString &username, const QString &passwd, const bool bAdd);

	void sendFile(const QString &username, const QString &passwd, QFile *file);
	void getFile(const QString &username, const QString &passwd, const QString &file);

signals:
	void networkRequestProcessed(const int ret_code, const QString &ret_string);

private:
	void makeNetworkRequest(const QUrl &url);
	void handleServerRequestReply(QNetworkReply *reply);
	void uploadFile(const QUrl &url, QFile *file);

	QNetworkAccessManager *m_networkManager;

	static TPOnlineServices* _appOnlineServices;
	friend TPOnlineServices* appOnlineServices();
};

inline TPOnlineServices* appOnlineServices() { return TPOnlineServices::_appOnlineServices; }
#endif // TPONLINESERVICES_H
