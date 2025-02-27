#include "tponlineservices.h"

#include "../tpglobals.h"
#include "../tputils.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace Qt::Literals::StringLiterals;

TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

static const QLatin1StringView root_user{"admin"};
static const QLatin1StringView root_passwd{"admin"};
static const QLatin1StringView url_paramether_user{"user"};
static const QLatin1StringView url_paramether_passwd{"password"};

inline QString makeCommandURL(const QString& option, const QString& value, const QString& passwd = QString{}, const QString &option2 = QString{},
								const QString &value2 = QString{}, const QString &option3 = QString{}, const QString &value3 = QString{}
								//, const QString &option4 = QString{}, const QString &value4 = QString{}
								)
{
	QString ret{"http://127.0.0.1/trainingplanner/?"_L1 + option + '=' + value};
	if (!passwd.isEmpty())
		ret += '&' + url_paramether_passwd + '=' + passwd;
	if (!option2.isEmpty())
	{
		ret += '&' + option2 + '=';
		if (!value2.isEmpty())
			ret += value2;
	}
	if (!option3.isEmpty())
	{
		ret += '&' + option3 + '=';
		if (!value3.isEmpty())
			ret += value3;
	}
	/*if (!option4.isEmpty())
		ret += '&' + option4;
	if (!value4.isEmpty())
		ret += '=' + value4;*/
	LOG_MESSAGE(ret)
	return ret;
}

/* The network_status param is not used, but it's carried from the caller to the signal handler. When using local, unnamed lambdas that would
	not be necessary, but functions connected to the serverOnline signal might be called several times before a response is obtained, so we use
	Qt::UniqueConnection which cannot be used with a lambda
*/
void TPOnlineServices::checkServer(int network_status)
{
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{QUrl{"http://127.0.0.1/trainingplanner"_L1}})};
	connect(reply, &QNetworkReply::finished, this, [this,reply,network_status]() {
		bool server_ok{false};
		if (reply)
		{
			reply->deleteLater();
			const QString &replyString{reply->readAll()};
			server_ok = replyString.contains("Welcome to the TrainingPlanner"_L1);
		}
		emit serverOnline(server_ok, network_status);
	});
}

void TPOnlineServices::checkOnlineUser(const QString &query, const QString &passwd)
{
	const QUrl &url{makeCommandURL("onlineuser"_L1, query, passwd)};
	makeNetworkRequest(url);
}

void TPOnlineServices::getOnlineUserData(const QString &user_id)
{
	const QUrl &url{makeCommandURL("onlinedata"_L1, user_id)};
	makeNetworkRequest(url);
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

void TPOnlineServices::updateOnlineUserInfo(const QString &username, const QString &passwd, QFile *file)
{
	connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,username] (const int ret_code, const QString &ret_string) {
		if (ret_code == 0)
		{
			const QUrl &url{makeCommandURL("alteronlineuser"_L1, username)};
			makeNetworkRequest(url);
		}
		else
			emit networkRequestProcessed(ret_code, ret_string);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	sendFile(username, passwd, file, QString{}, true);
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

void TPOnlineServices::checkClientsRequests(const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "listclientsrequests"_L1)};
	makeNetworkRequest(url);
}

void TPOnlineServices::removeClientRequest(const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "deletecoachrequest"_L1, client)};
	makeNetworkRequest(url);
}

void TPOnlineServices::acceptClientRequest(const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "acceptclientrequest"_L1, client)};
	makeNetworkRequest(url);
}

void TPOnlineServices::rejectClientRequest(const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "rejectclientrequest"_L1, client)};
	makeNetworkRequest(url);
}

void TPOnlineServices::checkCoachesAnswers(const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "listcoachesanswers"_L1)};
	makeNetworkRequest(url);
}

void TPOnlineServices::sendFile(const QString &username, const QString &passwd, QFile *file, const QString &targetUser, const bool b_internal_signal_only)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "upload"_L1, targetUser.isEmpty() ? username : targetUser)};
	uploadFile(url, file, b_internal_signal_only);
}

void TPOnlineServices::getFile(const QString &username, const QString &passwd, const QString &file, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "file"_L1, file, !targetUser.isEmpty() ? "fromuser"_L1 : QString{}, targetUser)};
	makeNetworkRequest(url);
}

void TPOnlineServices::getBinFile(const QString &username, const QString &passwd, const QString &filename,
									const QString &targetUser, const QDateTime& c_time)
{
	if (!c_time.isNull())
	{
		connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,username,passwd,filename,targetUser,c_time]
				(const int ret_code, const QString &ret_string) {
			QString filename_without_extension;
			const qsizetype dot_idx{filename.lastIndexOf('.')};
			if (dot_idx > 0)
				filename_without_extension = std::move(filename.left(dot_idx));
			if (ret_code == 0)
			{
				const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(ret_string)};
				if (online_ctime > c_time) //online file is newer. Download it
					getBinFile(username, passwd, dot_idx > 0 ? filename_without_extension : filename, targetUser, QDateTime{});
				else //local file is up to date. Use it
					emit fileReceived(1, ret_string, QByteArray{});
			}
			else
			{
				if (dot_idx > 0)
				{
					//filename is not on server. Try to download the same file with different extension because the owner might have uploaded
					//a different file, i.e. a jpeg avatar intead of a png; an odt resume instead of a pdf
					const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getbinfile"_L1, filename_without_extension, "fromuser"_L1, targetUser)};
					makeNetworkRequest(url);
				}
				else //Error. Nothing we can do
					emit fileReceived(2, ret_string, QByteArray{});
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "checkfilectime"_L1, filename, "fromuser"_L1, targetUser)};
		makeNetworkRequest(url, true);
	}
	else
	{
		const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getbinfile"_L1, filename, "fromuser"_L1, targetUser)};
		makeNetworkRequest(url);
	}
}

void TPOnlineServices::getCoachesList(const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getcoaches"_L1)};
	makeNetworkRequest(url);
}

void TPOnlineServices::makeNetworkRequest(const QUrl &url, const bool b_internal_signal_only)
{
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this,reply,b_internal_signal_only]() { handleServerRequestReply(reply, b_internal_signal_only); });
}

void TPOnlineServices::handleServerRequestReply(QNetworkReply *reply, const bool b_internal_signal_only)
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
			if (fileType.contains("application/octet-stream"_L1) || fileType.contains("text/plain"_L1))
			{
				QByteArray data{std::move(reply->readAll())};
				const qsizetype filename_sep_idx{data.indexOf("%%")};
				if (filename_sep_idx >= 2)
				{
					const QString filename{std::move(data.sliced(0, filename_sep_idx))};
					emit fileReceived(0, filename, data.sliced(filename_sep_idx + 2, data.size() - filename_sep_idx - 2));
					return;
				}
				emit fileReceived(1, "Error downloading file: "_L1, data);
				LOG_MESSAGE("Error downloading file: "_L1 + QString::fromUtf8(data));
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
	if (!b_internal_signal_only)
		emit networkRequestProcessed(ret_code, replyString.trimmed());
	else
		emit _networkRequestProcessed(ret_code, replyString.trimmed());
}

//curl -X POST -F file=@/home/guilherme/Documents/Fase_de_transição_-_Completo.txt "http://127.0.0.1/trainingplanner/?user=uc_guilherme_fortunato&upload&password=Guilherme_Fortunato"
void TPOnlineServices::uploadFile(const QUrl &url, QFile *file, const bool b_internal_signal_only)
{
	if (file->isOpen())
	{
		QNetworkRequest request{url};

		// Add the file as a part
		QHttpPart filePart;
		filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant("multipart/form-data; name=\"file\"; filename=\""_L1 + file->fileName() + "\"_L1"));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"_L1));
        filePart.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
        filePart.setBodyDevice(file);

		// Prepare the multipart data
		QHttpMultiPart *multiPart{new QHttpMultiPart{QHttpMultiPart::FormDataType, this}};
		multiPart->append(filePart);
		//file->setParent(multiPart); // MultiPart will manage file deletion

		// Send the request
		QNetworkReply *reply{m_networkManager->post(request, multiPart)};
		connect(reply, &QNetworkReply::finished, this, [this,reply,b_internal_signal_only]() { handleServerRequestReply(reply, b_internal_signal_only); });
		multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
	}
}
