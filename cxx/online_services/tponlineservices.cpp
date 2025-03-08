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

void TPOnlineServices::checkOnlineUser(const int requestid, const QString &query, const QString &passwd)
{
	const QUrl &url{makeCommandURL("onlineuser"_L1, query, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getOnlineUserData(const int requestid, const QString &user_id)
{
	const QUrl &url{makeCommandURL("onlinedata"_L1, user_id)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkUser(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL("checkuser"_L1, username, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::registerUser(const int requestid, const QString& username, const QString& passwd)
{
	const QUrl &url{makeCommandURL("adduser"_L1, username, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::updateOnlineUserInfo(const int requestid, const QString &username, const QString &passwd, QFile *file)
{
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid,username]
					(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				const QUrl &url{makeCommandURL("alteronlineuser"_L1, username)};
				makeNetworkRequest(requestid, url, false);
			}
			else
				emit networkRequestProcessed(request_id, ret_code, ret_string);
		}
	});
	sendFile(requestid, username, passwd, file, QString{}, true);
}

void TPOnlineServices::removeUser(const int requestid, const QString &username)
{
	const QUrl &url{makeCommandURL("deluser"_L1, username)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::alterUser(const int requestid, const QString &old_username, const QString &new_username, const QString &new_passwd)
{
	const QUrl &url{makeCommandURL("moduser"_L1, old_username, new_passwd, "newuser"_L1, new_username)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::addOrRemoveCoach(const int requestid, const QString &username, const QString &passwd, const bool bAdd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, bAdd ? "addcoach"_L1 : "delcoach"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendRequestToCoach(const int requestid, const QString &username, const QString &passwd, const QString& coach_net_name)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "requestcoach"_L1, coach_net_name)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkClientsRequests(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "listclientsrequests"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "deleteclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "acceptclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "rejectclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCoachesAnswers(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "listcoachesanswers"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachAnwers(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "deletecoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "acceptcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "rejectcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentClients(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getclients"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientFromCoach(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "removecurclient"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentCoaches(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getcoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachFromClient(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "removecurcoach"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendFile(const int requestid, const QString &username, const QString &passwd, QFile *file, const QString &targetUser, const bool b_internal_signal_only)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "upload"_L1, targetUser.isEmpty() ? username : targetUser)};
	uploadFile(requestid, url, file, b_internal_signal_only);
}

void TPOnlineServices::getFile(const int requestid, const QString &username, const QString &passwd, const QString &filename,
									const QString &targetUser, const QString &localFilePath)
{
	if (!localFilePath.isEmpty())
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid,username,passwd,filename,targetUser,localFilePath]
				(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				QString filename_without_extension;
				const qsizetype dot_idx{filename.lastIndexOf('.')};
				if (dot_idx > 0)
					filename_without_extension = std::move(filename.left(dot_idx));
				if (ret_code == 0)
				{
					QFileInfo fi{localFilePath};
					QDateTime c_time;
					if (fi.exists())
						c_time = std::move(fi.birthTime());

					const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(ret_string)};
					if (online_ctime > c_time) //online file is newer. Download it
						getFile(requestid, username, passwd, dot_idx > 0 ? filename_without_extension : filename, targetUser, QString{});
					else //local file is up to date. Use it
						emit fileReceived(request_id, 1, ret_string, QByteArray{});
				}
				else
				{
					if (dot_idx > 0)
					{
						//filename is not on server. Try to download the same file with different extension because the owner might have uploaded
						//a different file, i.e. a jpeg avatar intead of a png; an odt resume instead of a pdf. Not applicable to .txt files
						const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getbinfile"_L1, filename_without_extension,
							!targetUser.isEmpty() ? "fromuser"_L1 : QString{}, targetUser)};
						makeNetworkRequest(requestid, url, false);
					}
					else //Error. Nothing we can do
						emit fileReceived(request_id, 2, ret_string, QByteArray{});
				}
			}
		});
		const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "checkfilectime"_L1, filename,
							!targetUser.isEmpty() ? "fromuser"_L1 : QString{}, targetUser)};
		makeNetworkRequest(requestid, url, true);
	}
	else
	{
		const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, filename.endsWith(".txt"_L1) ? "file"_L1 : "getbinfile"_L1, filename,
							!targetUser.isEmpty() ? "fromuser"_L1 : QString{}, targetUser)};
		makeNetworkRequest(requestid, url);
	}
}

void TPOnlineServices::getOnlineCoachesList(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(url_paramether_user, username, passwd, "getonlinecoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only)
{
	LOG_MESSAGE(url.toDisplayString() + " * "_L1 + QString::number(requestid))
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
		handleServerRequestReply(requestid, reply, b_internal_signal_only);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void TPOnlineServices::handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only)
{
	int ret_code = -100;
	QString replyString;
	if (reply)
	{
		reply->deleteLater();

		const QHttpHeaders &headers{reply->headers()};
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
					emit fileReceived(requestid, 0, filename, data.sliced(filename_sep_idx + 2, data.size() - filename_sep_idx - 2));
					return;
				}
				emit fileReceived(requestid, 2, "Error downloading file: "_L1, data);
				LOG_MESSAGE("Error downloading file: "_L1 + QString::fromUtf8(data));
				return;
			}
		}
		//Only text replies, including text files
		replyString = std::move(QString::fromUtf8(reply->readAll()));
		if (reply->error())
			replyString += " ***** "_L1 + std::move(reply->errorString());
		LOG_MESSAGE(replyString + " * "_L1 + QString::number(requestid))
		//Slice off "Return code: "
		const qsizetype ret_code_idx{replyString.indexOf("Return code: "_L1) + 13};
		if (ret_code_idx >= 13)
		{
			ret_code = replyString.sliced(ret_code_idx, replyString.indexOf(' ', ret_code_idx) - ret_code_idx).toInt();
			static_cast<void>(replyString.remove(0, replyString.indexOf(' ', ret_code_idx) + 1));
		}
	}
	if (!b_internal_signal_only)
		emit networkRequestProcessed(requestid, ret_code, replyString.trimmed());
	else
		emit _networkRequestProcessed(requestid, ret_code, replyString.trimmed());
}

//curl -X POST -F file=@/home/guilherme/Documents/Fase_de_transição_-_Completo.txt "http://127.0.0.1/trainingplanner/?user=uc_guilherme_fortunato&upload&password=Guilherme_Fortunato"
void TPOnlineServices::uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only)
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
		connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
			handleServerRequestReply(requestid, reply, b_internal_signal_only);
		});
		multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
	}
}
