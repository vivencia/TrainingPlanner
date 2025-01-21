#include "tponlineservices.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace Qt::Literals::StringLiterals;

TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

static const QLatin1StringView root_user{"admin"};
static const QLatin1StringView root_passwd{"admin"};
static const QLatin1StringView uri_paramether_upload{"upload"};
static const QLatin1StringView uri_paramether_user{"user"};
static const QLatin1StringView uri_paramether_passwd{"password"};
static const QLatin1StringView uri_paramether_file{"file"};
static const QLatin1StringView uri_checkuser{"checkuser"};
static const QLatin1StringView uri_adduser{"adduser"};
static const QLatin1StringView uri_deluser{"deluser"};
static const QLatin1StringView uri_alteruser{"moduser"};

inline QString makeDownloadURI(const QString& user, const QString& passwd, const QString& file)
{
	return "http://127.0.0.1/trainingplanner/?"_L1 + uri_paramether_user + '=' + user + '&' + uri_paramether_passwd + '=' + passwd + '&' +
		uri_paramether_file + '=' + file;
}

inline QString makeUploadURI(const QString& user, const QString& passwd, const QString& file)
{
	return "http://127.0.0.1/trainingplanner/?"_L1 + uri_paramether_upload + '=' + user + '&' + uri_paramether_passwd + '=' + passwd + '&' +
		uri_paramether_file + '=' + file;
}

inline QString makeCommandURI(const QString& command, const QString& parameter, const QString &parameter2 = QString{},
								const QString &parameter3 = QString{}, const QString& passwd = QString{})
{
	QString ret{"http://127.0.0.1/trainingplanner/?"_L1 + command + '=' + parameter};
	if (!parameter2.isEmpty())
		ret += '&' + parameter2 + '=' + parameter3;
	if (!passwd.isEmpty())
		ret += '&' + uri_paramether_passwd + '=' + passwd;
	return ret;
}

void TPOnlineServices::createRootUser()
{
	const QUrl url{std::move(makeCommandURI(uri_adduser, root_user, QString{}, QString{}, root_passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::checkUser(const QString &username, const QString &passwd)
{
	const QUrl url{std::move(makeCommandURI(uri_checkuser, QString{}, QString{}, username, passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::registerUser(const QString& username, const QString& passwd)
{
	const QUrl url{std::move(makeCommandURI(uri_adduser, username, QString{}, QString{}, passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::removeUser(const QString &username)
{
	const QUrl url{std::move(makeCommandURI(uri_deluser, username))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::alterUser(const QString &old_username, const QString &new_username, const QString &new_passwd)
{
	const QUrl url{std::move(makeCommandURI(uri_alteruser, old_username, uri_paramether_user, new_username, new_passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::handleServerRequestReply(QNetworkReply *reply)
{
	if (!reply)
		return;
	QString replyString{std::move(QString::fromUtf8(reply->readAll()))};
	if (reply->error())
		replyString += " ***** "_L1 + std::move(reply->errorString());
	qDebug() << replyString;
	qsizetype ret_idx{replyString.indexOf("Return code: ")};
	int ret_code = -100;
	if (ret_idx != -1)
	{
		replyString.remove(ret_idx, replyString.length() - ret_idx);
		ret_idx += 13;
		ret_code = replyString.sliced(ret_idx, replyString.indexOf(' ', ret_idx + 1)).toInt();
	}
	emit networkRequestProcessed(ret_code, replyString.simplified());
}
