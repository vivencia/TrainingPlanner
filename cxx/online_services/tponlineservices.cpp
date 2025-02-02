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
static const QLatin1StringView url_paramether_passwd{"password"};
static const QLatin1StringView url_paramether_file{"file"};

inline QString makeCommandURL(const QString& option, const QString& value, const QString& passwd = QString{}, const QString &option2 = QString{},
								const QString &value2 = QString{}, const QString &option3 = QString{}, const QString &value3 = QString{})
{
	QString ret{"http://127.0.0.1/trainingplanner/?"_L1 + option + '=' + value};
	if (!passwd.isEmpty())
		ret += '&' + url_paramether_passwd + '=' + passwd;
	if (!option2.isEmpty())
		ret += '&' + option2;
	if (!value2.isEmpty())
		ret += '=' + value2;
	if (!option3.isEmpty())
		ret += '&' + option3;
	if (!value3.isEmpty())
		ret += '=' + value3;
	LOG_MESSAGE(ret)
	return ret;
}

void TPOnlineServices::checkServer()
{
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{QUrl{"http://127.0.0.1/trainingplanner"_L1}})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		bool server_ok{false};
		if (reply)
		{
			reply->deleteLater();
			const QString &replyString{reply->readAll()};
			server_ok = replyString.contains("Welcome to the TrainingPlanner"_L1);
		}
		emit serverOnline(server_ok);
	});
}

void TPOnlineServices::checkUser(const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL("checkuser"_L1, username, passwd)};
	makeNetworkRequest(url);
}

void TPOnlineServices::registerUser(const QString& username, const QString& passwd)
{
	const QUrl &url{makeCommandURL("adduser"_L1, username, passwd)};
	makeNetworkRequest(url);
}

void TPOnlineServices::removeUser(const QString &username)
{
	const QUrl &url{makeCommandURL("deluser"_L1, username)};
	makeNetworkRequest(url);
}

void TPOnlineServices::alterUser(const QString &old_username, const QString &new_username, const QString &new_passwd)
{
	const QUrl &url{makeCommandURL("moduser"_L1, old_username, new_passwd, "newuser"_L1, new_username)};
	makeNetworkRequest(url);
}

void TPOnlineServices::addOrRemoveCoach(const QString &username, const QString &passwd, const bool bAdd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, bAdd ? "addcoach"_L1 : "delcoach"_L1)};
	makeNetworkRequest(url);
}

void TPOnlineServices::sendRequestToCoach(const QString &username, const QString &passwd, const QString& coach_net_name)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "requestcoach"_L1, coach_net_name)};
	makeNetworkRequest(url);
}

void TPOnlineServices::sendFile(const QString &username, const QString &passwd, QFile *file, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "upload"_L1, targetUser)};
	uploadFile(url, file);
}

void TPOnlineServices::getFile(const QString &username, const QString &passwd, const QString &file, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "file"_L1, file, !targetUser.isEmpty() ? "fromuser"_L1 : QString{}, targetUser)};
	makeNetworkRequest(url);
}

void TPOnlineServices::getBinFile(const QString &username, const QString &passwd, const QString &filename_without_extension, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getbinfile"_L1, filename_without_extension, "fromuser"_L1, targetUser)};
	makeNetworkRequest(url);
}

void TPOnlineServices::getCoachesList(const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getcoaches"_L1)};
	makeNetworkRequest(url);
}

void TPOnlineServices::makeNetworkRequest(const QUrl &url)
{
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
}

void TPOnlineServices::handleServerRequestReply(QNetworkReply *reply)
{
	int ret_code = -100;
	QString replyString;
	if (reply)
	{
		reply->deleteLater();

		const QHttpHeaders& headers{reply->headers()};
		if (headers.contains("Content-Type"_L1))
		{
			const QString &fileType{headers.value("Content-Type"_L1).toByteArray()};
			if (fileType.contains("application/octet-stream"_L1))
			{
				QByteArray data{std::move(reply->readAll())};
				const qsizetype ret_code_idx{data.indexOf("Return code: ") + 13};
				if (ret_code_idx >= 13)
				{
					ret_code = replyString.sliced(ret_code_idx, data.indexOf(' ', ret_code_idx) - ret_code_idx).toInt();
					if (ret_code == 0)
					{
						const qsizetype filename_sep_idx{data.indexOf("##", ret_code_idx) + 2};
						if (filename_sep_idx >= 2)
						{
							const qsizetype filename_sep_idx2{data.indexOf("##", filename_sep_idx)};
							const QString filename{std::move(data.sliced(filename_sep_idx, filename_sep_idx2 - filename_sep_idx))};
							static_cast<void>(data.remove(0, filename_sep_idx2 + 2));
							emit binaryFileReceived(ret_code, filename, data);
							return;
						}
					}
				}
				emit binaryFileReceived(ret_code, "Error downloading bynary file: "_L1, data);
				LOG_MESSAGE("Error downloading bynary file: "_L1 + QString::fromUtf8(data));
				return;
			}
		}
		//Only text replies, including text files
		replyString = std::move(QString::fromUtf8(reply->readAll()));
		if (reply->error())
			replyString += " ***** "_L1 + std::move(reply->errorString());
		LOG_MESSAGE(replyString);
		//Slice off "Return code: "
		const qsizetype ret_code_idx{replyString.indexOf("Return code: "_L1) + 13};
		if (ret_code_idx >= 13)
		{
			ret_code = replyString.sliced(ret_code_idx, replyString.indexOf(' ', ret_code_idx) - ret_code_idx).toInt();
			static_cast<void>(replyString.remove(0, replyString.indexOf(' ', ret_code_idx) + 1));
		}
	}
	emit networkRequestProcessed(ret_code, replyString.simplified());
}

//curl -X POST -F file=@/home/guilherme/Documents/Fase_de_transição_-_Completo.txt "http://127.0.0.1/trainingplanner/?user=uc_guilherme_fortunato&upload&password=Guilherme_Fortunato"
void TPOnlineServices::uploadFile(const QUrl &url, QFile *file)
{
	if ( file->isOpen())
	{
		QNetworkRequest request{url};

		// Add the file as a part
		QHttpPart filePart;
		filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant("multipart/form-data; name=\"file\"; filename=\"" + file->fileName() + "\""));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        filePart.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
        filePart.setBodyDevice(file);

		// Prepare the multipart data
		QHttpMultiPart *multiPart{new QHttpMultiPart{QHttpMultiPart::FormDataType, this}};
		multiPart->append(filePart);
		file->setParent(multiPart); // MultiPart will manage file deletion

		// Send the request
		QNetworkReply *reply{m_networkManager->post(request, multiPart)};
		connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleServerRequestReply(reply); });
		multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
	}
}
