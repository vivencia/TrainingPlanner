#include "tponlineservices.h"

#include "../tpglobals.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace Qt::Literals::StringLiterals;

TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

static const QLatin1StringView root_user{"admin"};
static const QLatin1StringView root_passwd{"admin"};
static const QLatin1StringView url_paramether_upload{"upload"};
static const QLatin1StringView url_paramether_user{"user"};
static const QLatin1StringView url_paramether_newuser{"newuser"};
static const QLatin1StringView url_paramether_passwd{"password"};
static const QLatin1StringView url_paramether_file{"file"};
static const QLatin1StringView url_checkuser{"checkuser"};
static const QLatin1StringView url_adduser{"adduser"};
static const QLatin1StringView url_deluser{"deluser"};
static const QLatin1StringView url_alteruser{"moduser"};

inline QString makeDownloadURL(const QString& user, const QString& passwd, const QString& file)
{
	return "http://127.0.0.1/trainingplanner/?"_L1 + url_paramether_user + '=' + user + '&' + url_paramether_passwd + '=' + passwd + '&' +
		url_paramether_file + '=' + file;
}

inline QString makeUploadURL(const QString& user, const QString& passwd, const QString& file)
{
	return "http://127.0.0.1/trainingplanner/?"_L1 + url_paramether_upload + '=' + user + '&' + url_paramether_passwd + '=' + passwd + '&' +
		url_paramether_file + '=' + file;
}

inline QString makeCommandURL(const QString& command, const QString& parameter, const QString &parameter2 = QString{},
								const QString &parameter3 = QString{}, const QString& passwd = QString{})
{
	QString ret{"http://127.0.0.1/trainingplanner/?"_L1 + command + '=' + parameter};
	if (!parameter2.isEmpty())
		ret += '&' + parameter2;
	if (!parameter3.isEmpty())
		ret += '=' + parameter3;
	if (!passwd.isEmpty())
		ret += '&' + url_paramether_passwd + '=' + passwd;
	return ret;
}

void TPOnlineServices::checkUser(const QString &username, const QString &passwd)
{
	const QUrl url{std::move(makeCommandURL(url_checkuser, username, QString{}, QString{}, passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::registerUser(const QString& username, const QString& passwd)
{
	const QUrl url{std::move(makeCommandURL(url_adduser, username, QString{}, QString{}, passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::removeUser(const QString &username)
{
	const QUrl url{std::move(makeCommandURL(url_deluser, username))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::alterUser(const QString &old_username, const QString &new_username, const QString &new_passwd)
{
	const QUrl url{std::move(makeCommandURL(url_alteruser, old_username, url_paramether_newuser, new_username, new_passwd))};
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::sendFile(const QString &username, const QString &passwd, QFile *file)
{
	const QUrl url{std::move(makeCommandURL(url_paramether_user, username, "upload"_L1, "", passwd))};
	uploadFile(url, file);
}

void TPOnlineServices::handleServerRequestReply(QNetworkReply *reply)
{
	int ret_code = -100;
	QString replyString;
	if (reply)
	{
		reply->deleteLater();
		replyString = std::move(QString::fromUtf8(reply->readAll()));
		if (reply->error())
			replyString += " ***** "_L1 + std::move(reply->errorString());
		LOG_MESSAGE(replyString);
		//Slice off "Return code: "
		const qsizetype ret_code_idx{replyString.indexOf("Return code: ") + 13};
		ret_code = replyString.sliced(ret_code_idx, replyString.indexOf(' ', ret_code_idx) - ret_code_idx).toInt();
		static_cast<void>(replyString.remove(0, replyString.indexOf(' ', ret_code_idx) + 1));
	}
	emit networkRequestProcessed(ret_code, replyString.simplified());
}

//curl -X POST -F file=@/home/guilherme/Documents/Fase_de_transição_-_Completo.txt "http://127.0.0.1/trainingplanner/?user=uc_guilherme_fortunato&upload&password=Guilherme_Fortunato"
void TPOnlineServices::uploadFile(const QUrl &url, QFile *file)
{
	QNetworkRequest request{url};
	// Prepare the multipart data
	QHttpMultiPart *multiPart{new QHttpMultiPart{QHttpMultiPart::FormDataType, this}};
	// Add the file as a part
	QHttpPart filePart;
	filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant("form-data; name=\"uploaded_file\"; file=\""_L1 + file->fileName() + "\""_L1));
	filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"_L1));
	filePart.setBodyDevice(file);
	file->setParent(multiPart); // Let multipart manage the file's lifecycle
	multiPart->append(filePart);

	// Send the request
	QNetworkReply *reply{m_networkManager->post(request, multiPart)};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
	multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
}

