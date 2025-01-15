#include "tponlineservices.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace Qt::Literals::StringLiterals;

static const QLatin1StringView root_user{"admin"};
static const QLatin1StringView root_passwd{"admin"};
static const QLatin1StringView uri_paramether_user{"user"};
static const QLatin1StringView uri_paramether_passwd{"password"};
static const QLatin1StringView uri_paramether_file{"file"};

inline QString makeURI(const QString& user, const QString& passwd, const QString& file)
{
	return "http://127.0.0.1/trainingplanner/?"_L1 + uri_paramether_user + '=' + user + '&' + uri_paramether_passwd + '=' + passwd + '&' +
		uri_paramether_file + '=' + file;
}

TPOnlineServices::TPOnlineServices(QObject *parent)
	: QObject{parent}
{
	const QUrl url{makeURI(root_user, root_passwd, "complete_program.txt")};
	QNetworkAccessManager *networkManager = new QNetworkAccessManager{this};

	QNetworkReply* reply{networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::handleServerRequestReply(QNetworkReply *reply)
{
	if (!reply)
		return;
	if (!reply->error())
		qDebug() << QString::fromUtf8(reply->readAll());
	else
		qDebug() << reply->errorString();
}
