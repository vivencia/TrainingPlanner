#include "tponlineservices.h"

#include "scan_network.h"
#include "../tpglobals.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

using namespace Qt::Literals::StringLiterals;

TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

constexpr QLatin1StringView server_address{"http://%1:8080/trainingplanner/"_L1};
const QLatin1StringView root_user{"admin"};
const QLatin1StringView root_passwd{"admin"};
const QLatin1StringView base_ip{"192.168.10."_L1};

void TPOnlineServices::scanNetwork()
{
	const int requestid{appUtils()->generateUniqueId("scanNetwork"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	if (!appSettings()->serverAddress().isEmpty())
	{
		*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid]
									(const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				connect(this, &TPOnlineServices::_serverOnline, this, [this] (const uint online_status, const QString &address)
				{
					switch (online_status)
					{
						case 0: break; //online
						case 1: //Bad Gateway or connection refused or some other error received. Scan local network for a new server address
							appSettings()->setServerAddress(QString{});
							scanNetwork();
						break;
						case 2: break; //server paused
					}
					emit serverOnline(online_status);
				}, Qt::SingleShotConnection);
				checkServerResponse(ret_code, ret_string, appSettings()->serverAddress());
			}
		});
		makeNetworkRequest(requestid, server_address.arg(appSettings()->serverAddress()), true);
	}
	else
	{
		tpScanNetwork *tsn{new tpScanNetwork{base_ip}};
		QThread *thread{new QThread{}};
		tsn->moveToThread(thread);

		connect(tsn, &tpScanNetwork::addressReachable, this, [this,conn,requestid] (const QString &ip)
		{
			if (ip == "None"_L1)
			{
				emit serverOnline(1);
				return;
			}
			*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid,ip]
									(const int request_id, const int ret_code, const QString &ret_string)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					checkServerResponse(ret_code, ret_string, ip);
				}
			});
			makeNetworkRequest(requestid, server_address.arg(ip), true);
		}, Qt::QueuedConnection);

		auto conn2{std::make_shared<QMetaObject::Connection>()};
		*conn2 = connect(this, &TPOnlineServices::_serverOnline, this, [this,conn2,thread,tsn] (const uint online_status, const QString &address)
		{
			if (online_status == 0)
			{
				disconnect(*conn2);
				appSettings()->setServerAddress(address);
				thread->disconnect();
				thread->requestInterruption();
				#ifndef Q_OS_ANDROID
				m_useLocalHost = false;
				#endif
			}
			emit serverOnline(online_status);
		});

		connect(thread, &QThread::finished, thread, [this,conn2,tsn,thread] ()
		{
			disconnect(*conn2);
			tsn->deleteLater();
			thread->deleteLater();

			//If thread is finished it means no server ip was found, on linux test machine, try localhost
			#ifndef Q_OS_ANDROID
			m_useLocalHost = true;
			appSettings()->setServerAddress("localhost"_L1);
			scanNetwork();
			#endif
		});
		connect(thread, &QThread::started, tsn, [tsn] () { tsn->scan(base_ip); });
		thread->start();
	}
}

void TPOnlineServices::checkOnlineUser(const int requestid, const QString &query, const QString &passwd)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "onlineuser"_L1, query, "userpassword"_L1, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getOnlineUserData(const int requestid, const QString &user_id)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "onlinedata"_L1, user_id)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkUser(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "checkuser"_L1, username, "userpassword"_L1, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::registerUser(const int requestid, const QString& username, const QString& passwd)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "adduser"_L1, username, "userpassword"_L1, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::updateOnlineUserInfo(const int requestid, const QString &username, const QString &passwd)
{
	//user.data must be sent to the server prior to calling this function
	const QUrl &url{makeCommandURL(root_user, root_passwd, "alteronlineuser"_L1, username, "userpassword"_L1, passwd)};
	makeNetworkRequest(requestid, url, false);
}

void TPOnlineServices::removeUser(const int requestid, const QString &username)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "deluser"_L1, username)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::changePassword(const int requestid, const QString &username, const QString &old_passwd, const QString &new_passwd)
{
	const QUrl &url{makeCommandURL(root_user, root_passwd, "changepassword"_L1, username, "oldpassword"_L1, old_passwd, "newpassword"_L1, new_passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::addDevice(const int requestid, const QString &username, const QString &passwd, const QString &device_id)
{
	const QUrl &url{makeCommandURL(username, passwd, "adddevice"_L1, device_id)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getDevicesList(const int requestid, const QString &username, const QString &passwd)
{
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			QStringList devices_list;
			if (ret_code == 0)
			{
				const QStringList &remote_devices_list{ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts)};
				for (const auto &device : remote_devices_list)
					devices_list.append(std::move(device));
			}
			emit networkListReceived(request_id, ret_code, devices_list);
		}
	});
	const QUrl &url{makeCommandURL(username, passwd, "getdeviceslist"_L1)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::delDevice(const int requestid, const QString &username, const QString &passwd, const QString &device_id)
{
	const QUrl &url{makeCommandURL(username, passwd, "deldevice"_L1, device_id)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::addOrRemoveCoach(const int requestid, const QString &username, const QString &passwd, const bool bAdd)
{
	const QUrl &url{makeCommandURL(username, passwd, bAdd ? "addcoach"_L1 : "delcoach"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getOnlineCoachesList(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "getonlinecoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendRequestToCoach(const int requestid, const QString &username, const QString &passwd, const QString& coach_net_name)
{
	const QUrl &url{makeCommandURL(username, passwd, "requestcoach"_L1, coach_net_name)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkClientsRequests(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "listclientsrequests"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(username, passwd, "deleteclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(username, passwd, "acceptclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectClientRequest(const int requestid, const QString &username, const QString &passwd, const QString &client)
{
	const QUrl &url{makeCommandURL(username, passwd, "rejectclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCoachesAnswers(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "listcoachesanswers"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachAnwers(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(username, passwd, "deletecoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(username, passwd, "acceptcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectCoachAnswer(const int requestid, const QString &username, const QString &passwd, const QString &coach)
{
	const QUrl &url{makeCommandURL(username, passwd, "rejectcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentClients(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "getclients"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientFromCoach(const int requestid, const QString &username, const QString &passwd,
												const QString &client)
{
	const QUrl &url{makeCommandURL(username, passwd, "removecurclient"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentCoaches(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "getcoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachFromClient(const int requestid, const QString &username, const QString &passwd,
												const QString &coach)
{
	const QUrl &url{makeCommandURL(username, passwd, "removecurcoach"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::executeCommands(const int requestid, const QString &username, const QString &passwd,
											const QString &subdir, const bool delete_cmdfile)
{
	const QUrl &url{makeCommandURL(username, passwd, "runcmds"_L1, subdir, "delete", delete_cmdfile ? "1"_L1 : "0"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendFile(const int requestid, const QString &username, const QString &passwd, QFile *file,
									const QString &subdir, const QString &targetUser, const bool b_internal_signal_only)
{
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
							(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				if (remoteFileUpToDate(ret_string, file->fileName())) //remote file is up to date. Don't send anything
				{
					emit networkRequestProcessed(requestid, 0, tr("File on the online server already up to date"));
					return;
				}
			}
			const QUrl &url{makeCommandURL(username, passwd, "upload"_L1, subdir, "targetuser", targetUser.isEmpty() ? username : targetUser)};
			uploadFile(requestid, url, file, b_internal_signal_only);
		}
	});
	const QUrl &url{makeCommandURL(username, passwd, "checkfilectime"_L1, appUtils()->getFileName(file->fileName()),
						"subdir"_L1, subdir, "fromuser"_L1, targetUser)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::listFiles(const int requestid, const QString &username, const QString &passwd, const bool only_new,
						const bool include_ctime, const QString &pattern, const QString &subdir, const QString &targetUser)
{
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			QStringList new_files;
			if (ret_code == 0)
			{
				const QString &localDir{appUtils()->localAppFilesDir() + targetUser + '/' + subdir + '/'};
				const QStringList &remote_files_list{ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts)};
				for (uint i{0}; i < remote_files_list.count(); i += 2)
				{
					QString filename{std::move(remote_files_list.at(i))};
					QString online_date{std::move(remote_files_list.at(i+1))};
					if (only_new)
					{
						if (localFileUpToDate(online_date, localDir + filename))
							continue;
					}
					new_files.append(std::move(filename));
					if (include_ctime)
						new_files.append(std::move(online_date));
				}
			}
			emit networkListReceived(request_id, ret_code, new_files);
		}
	});
	const QUrl &url{makeCommandURL(username, passwd, "listfiles"_L1, subdir, "fromuser"_L1, targetUser, "pattern"_L1, pattern)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::removeFile(const int requestid, const QString &username, const QString &passwd, const QString &filename,
										const QString &subdir, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(username, passwd, "delfile"_L1, filename, "subdir"_L1, subdir, "fromuser"_L1, targetUser)};
	makeNetworkRequest(requestid, url, true);
}

bool TPOnlineServices::remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const
{
	QFileInfo fi{localFile};
	if (fi.exists())
	{
		const QDateTime &c_time{fi.lastModified()};
		const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(onlineDate)};
		return online_ctime >= c_time;
	}
	return true;
}

bool TPOnlineServices::localFileUpToDate(const QString &onlineDate, const QString &localFile) const
{
	QFileInfo fi{localFile};
	if (fi.exists())
	{
		const QDateTime &c_time{fi.lastModified()};
		const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(onlineDate)};
		return c_time >= online_ctime;
	}
	return false;
}

void TPOnlineServices::getFile(const int requestid, const QString &username, const QString &passwd, const QString &filename, const QString &subdir,
									const QString &targetUser, const QString &localFilePath)
{
	if (!localFilePath.isEmpty())
	{
		QFileInfo fi{localFilePath};
		if (fi.isFile() && fi.isWritable())
		{
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid,username,passwd,filename,targetUser,localFilePath]
							(const int request_id, const int ret_code, const QString &ret_string)
			{
				if (request_id == requestid)
				{
					disconnect(*conn);
					if (ret_code == 0)
					{
						if (localFileUpToDate(ret_string, localFilePath)) //local file is up to date. Use it
							emit fileReceived(request_id, 1, ret_string, QByteArray{});
						else
							getFile(requestid, username, passwd, filename, targetUser);
					}
				}
			});
			const QUrl &url{makeCommandURL(username, passwd, "checkfilectime"_L1, filename.lastIndexOf('.') > 0 ?
							filename : appUtils()->getFileName(localFilePath), "subdir"_L1, subdir, "fromuser"_L1, targetUser)};
			makeNetworkRequest(requestid, url, true);
			return;
		}
	}
	const QUrl &url{makeCommandURL(username, passwd, filename.lastIndexOf('.') > 0 ?
				(filename.endsWith(".txt"_L1) ? "file"_L1 : "getbinfile"_L1) : "getbinfile"_L1, filename, "subdir"_L1, subdir, "fromuser"_L1, targetUser)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getCmdFile(const int requestid, const QString &username, const QString &passwd,
										const QString &filename, const QString &subdir, const bool delete_cmd)
{
	const QUrl &url{makeCommandURL(username, passwd, "downloadcmd"_L1, filename, "subdir"_L1, subdir, "delete"_L1, delete_cmd ? "1"_L1 : "0"_L1)};
	makeNetworkRequest(requestid, url);
}

QString TPOnlineServices::makeCommandURL(const QString &username, const QString &passwd, const QString &option1,
								const QString &value1, const QString &option2, const QString &value2,
								const QString &option3, const QString &value3
							)
{
	QString ret{server_address.arg(appSettings()->serverAddress()) + "?user="_L1 + username + "&password="_L1 + passwd};
	if (!option1.isEmpty())
	{
		ret += '&' + option1 + '=';
		if (!value1.isEmpty())
			ret += value1;
	}
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
	return ret;
}

void TPOnlineServices::makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only)
{
	LOG_MESSAGE(url.toDisplayString() + " * "_L1 + QString::number(requestid))
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
		handleServerRequestReply(requestid, reply, b_internal_signal_only);
	}, Qt::SingleShotConnection);
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
							QVariant("multipart/form-data; name=\"file\"; filename=\""_L1 + file->fileName() + "\""_L1));
							//QVariant("multipart/form-data; name=\"file\"; filename=\""_L1 + file->fileName() + "\"_L1"));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"_L1));
        filePart.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
        filePart.setBodyDevice(file);

		// Prepare the multipart data
		QHttpMultiPart *multiPart{new QHttpMultiPart{QHttpMultiPart::FormDataType, this}};
		multiPart->append(filePart);
		//file->setParent(multiPart); // MultiPart will manage file deletion

		LOG_MESSAGE(url.toDisplayString() + " * "_L1 + QString::number(requestid))

		// Send the request
		QNetworkReply *reply{m_networkManager->post(request, multiPart)};
		connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
			handleServerRequestReply(requestid, reply, b_internal_signal_only);
		});
		multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
	}
}

void TPOnlineServices::checkServerResponse(const int ret_code, const QString &ret_string, const QString &address)
{
	qDebug() << "server response: " << ret_string << "  " << address;
	uint online_status{1};
	if (ret_string.contains("Welcome to the TrainingPlanner"_L1))
		online_status = 0;
	else if (ret_string.contains("server paused"_L1))
		online_status = 2;
	emit _serverOnline(online_status, address);
}
