#ifndef TPONLINESERVICES_H
#define TPONLINESERVICES_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QNetworkReply)

class TPOnlineServices : public QObject
{

Q_OBJECT

public:
	explicit TPOnlineServices(QObject *parent = nullptr);

private:
	void handleServerRequestReply(QNetworkReply *reply);
};

#endif // TPONLINESERVICES_H
