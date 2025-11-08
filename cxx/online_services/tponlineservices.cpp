#include "tponlineservices.h"

#include "scan_network.h"
#include "../osinterface.h"
#include "../tpsettings.h"
#include "../tputils.h"
#include "../return_codes.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

using namespace Qt::Literals::StringLiterals;

TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

constexpr QLatin1StringView server_address{"http://%1:8080/trainingplanner/"_L1};
constexpr QLatin1StringView root_user{"admin"};
constexpr QLatin1StringView root_passwd{"admin"};

void TPOnlineServices::scanNetwork(const QString &last_working_address, const bool assume_working)
{
	if (m_scanning)
		return;
	#ifndef QT_NO_DEBUG
	qDebug() << "scanNetwork() -> starting";
	#endif
	m_scanning = true;
	const int requestid{appUtils()->generateUniqueId("scanNetwork"_L1)};
	auto conn{std::make_shared<QMetaObject::Connection>()};

	if (assume_working && !last_working_address.isEmpty())
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "scanNetwork() -> appSettings()->serverAddress() " << last_working_address;
		#endif
		*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid,last_working_address]
									(const int request_id, const int ret_code, const QString &ret_string)
		{
			if (request_id == requestid)
			{
				disconnect(*conn);
				connect(this, &TPOnlineServices::_serverResponse, this, [this,last_working_address] (const uint online_status, const QString &address)
				{
					m_scanning = false;
					#ifndef QT_NO_DEBUG
					qDebug() << "scanNetwork() 1-> _serverResponse online_status = " << online_status << " , address = " << address;
					#endif
					switch (online_status)
					{
						case TP_RET_CODE_SUCCESS: break; //online
						case TP_RET_CODE_SERVER_UNREACHABLE: //Bad Gateway, socket operation timed out, connection refused or some other error received. Scan local network for a new server address
							appSettings()->setServerAddress(QString{});
							scanNetwork(last_working_address, false);
						break;
						case TP_RET_CODE_SERVER_PAUSED: break;
					}
					emit serverOnline(online_status);
				}, Qt::SingleShotConnection);
				checkServerResponse(ret_code, ret_string, last_working_address);
			}
		});
		makeNetworkRequest(requestid, server_address.arg(last_working_address), true);
	}
	else
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "scanNetwork() -> starting scan on separate thread";
		#endif
		tpScanNetwork *tsn{new tpScanNetwork{}};
		QThread *thread{new QThread{}};
		tsn->moveToThread(thread);

		connect(tsn, &tpScanNetwork::addressReachable, this, [this,conn,requestid] (const QString &ip)
		{
			#ifndef QT_NO_DEBUG
			qDebug() << "scanNetwork() -> addressReachable = " << ip;
			#endif
			if (ip == "None"_L1)
			{
				emit serverOnline(TP_RET_CODE_SERVER_UNREACHABLE);
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
		*conn2 = connect(this, &TPOnlineServices::_serverResponse, this, [this,conn2,thread]
													(const uint online_status, const QString &address)
		{
			#ifndef QT_NO_DEBUG
			qDebug() << "scanNetwork() 2-> _serverResponse online_status = " << online_status << " , address = " << address;
			#endif
			if (online_status == TP_RET_CODE_SUCCESS)
			{
				disconnect(*conn2);
				if (m_scanning)
				{
					m_scanning = false;
					appSettings()->setServerAddress(address);
					thread->requestInterruption();
					#ifndef Q_OS_ANDROID
					m_useLocalHost = false;
					#endif
				}
			}
			emit serverOnline(online_status);
		});

		connect(thread, &QThread::finished, thread, [this,conn2,tsn,thread] ()
		{
			#ifndef QT_NO_DEBUG
			qDebug() << "scanNetwork() -> thread finished";
			#endif
			m_scanning = false;
			thread->disconnect();
			tsn->deleteLater();
			thread->deleteLater();
		});
		connect(thread, &QThread::started, tsn, [tsn,last_working_address] () { tsn->scan(last_working_address); });
		m_scanning = true;
		thread->start();
	}
}

#ifndef Q_OS_ANDROID
void TPOnlineServices::getAllUsers(const int requestid)
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid]
									(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			QStringList users;
			if (ret_code == TP_RET_CODE_SUCCESS)
				users = std::move(ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts));
			emit networkListReceived(request_id, ret_code, users);
		}
	});
	const QUrl &url{makeCommandURL(root_user, root_passwd, "allusers"_L1)};
	makeNetworkRequest(requestid, url, true);
}
#endif

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
			if (ret_code == TP_RET_CODE_SUCCESS)
				devices_list = std::move(ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts));
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
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
							(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				if (remoteFileUpToDate(ret_string, file->fileName())) //remote file is up to date. Don't send anything
				{
					emit networkRequestProcessed(requestid, TP_RET_CODE_NO_CHANGES_SUCCESS, tr("File on the online server already up to date"));
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
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			QStringList new_files;
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				const QString &localDir{appSettings()->localAppFilesDir() + targetUser + '/' + subdir + '/'};
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

void TPOnlineServices::listDirs(const int requestid, const QString &username, const QString &passwd, const QString &pattern,
								const QString &subdir, const QString &targetUser, const bool include_dot_dir)
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			QStringList directories;
			if (ret_code == TP_RET_CODE_SUCCESS)
				directories = std::move(ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts));
			if (include_dot_dir)
				directories.prepend(std::move(QString{'.'}));
			emit networkListReceived(request_id, ret_code, directories);
		}
	});
	const QUrl &url{makeCommandURL(username, passwd, "listdirs"_L1, subdir, "fromuser"_L1, targetUser, "pattern"_L1, pattern)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::removeFile(const int requestid, const QString &username, const QString &passwd, const QString &filename,
										const QString &subdir, const QString &targetUser)
{
	const QUrl &url{makeCommandURL(username, passwd, "delfile"_L1, filename, "subdir"_L1, subdir, "fromuser"_L1, targetUser)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::getFile(const int requestid, const QString &username, const QString &passwd, const QString &filename, const QString &subdir,
									const QString &targetUser, const QString &localFilePath)
{
	bool check_ctime_first{false};
	if (!localFilePath.isEmpty())
	{
		QFileInfo fi{localFilePath};
		check_ctime_first = (fi.isFile() && fi.isWritable());
	}
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
							(const int request_id, const int ret_code, const QString &ret_string, const QByteArray &contents)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS)
			{
				if (check_ctime_first)
				{
					if (!localFileUpToDate(ret_string, localFilePath)) //if local file is up to date, we 'll use it
						getFile(requestid, username, passwd, filename, subdir, targetUser);
					else
					{
						emit fileReceived(request_id, TP_RET_CODE_NO_CHANGES_SUCCESS, ret_string, contents);
						return;
					}
				}
			}
			emit fileReceived(request_id, ret_code, ret_string, contents);
		}
	});

	QUrl url{};
	if (check_ctime_first)
	{
		url = std::move(makeCommandURL(username, passwd, "checkfilectime"_L1, filename.lastIndexOf('.') > 0 ?
						filename : appUtils()->getFileName(localFilePath), "subdir"_L1, subdir, "fromuser"_L1, targetUser));
	}
	else
	{
		url = std::move(makeCommandURL(username, passwd, filename.lastIndexOf('.') > 0 ?
				(filename.endsWith(".txt"_L1) || filename.endsWith(".ini"_L1) ? "file"_L1 :
				"getbinfile"_L1) : "getbinfile"_L1, filename, "subdir"_L1, subdir, "fromuser"_L1, targetUser));
	}
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::getCmdFile(const int requestid, const QString &username, const QString &passwd,
										const QString &filename, const QString &subdir)
{
	const QUrl &url{makeCommandURL(username, passwd, "downloadcmd"_L1, filename, "subdir"_L1, subdir, "deviceid"_L1, appOsInterface()->deviceID())};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::checkMessages(const int requestid, const QString &username, const QString &passwd)
{
	const QUrl &url{makeCommandURL(username, passwd, "getnewmessages"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendMessage(const int requestid, const QString &username, const QString &passwd, const QString &receiver, const QString &encoded_message)
{
	const QUrl &url{makeCommandURL(username, passwd, "sendmessage"_L1, receiver, "message"_L1, encoded_message)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::chatMessageReceived(const int requestid, const QString &username, const QString &passwd, const QString &sender, const QString &msgid)
{
	const QUrl &url{makeCommandURL(username, passwd, "messagereceived"_L1, sender, "messageid"_L1, msgid)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::chatMessageRead(const int requestid, const QString &username, const QString &passwd, const QString &sender, const QString &msgid)
{
	const QUrl &url{makeCommandURL(username, passwd, "messageread"_L1, sender, "messageid"_L1, msgid)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeMessage(const int requestid, const QString &username, const QString &passwd, const QString &receiver, const QString &msgid)
{
	const QUrl &url{makeCommandURL(username, passwd, "removemessage"_L1, receiver, "messageid"_L1, msgid)};
	makeNetworkRequest(requestid, url);
}

QString TPOnlineServices::makeCommandURL(const QString &username, const QString &passwd, const QString &option1,
								const QString &value1, const QString &option2, const QString &value2,
								const QString &option3, const QString &value3)
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
	#ifndef QT_NO_DEBUG
	qDebug() << url.toDisplayString() << " * "_L1  << QString::number(requestid);
	#endif
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
		handleServerRequestReply(requestid, reply, b_internal_signal_only);
	}, Qt::SingleShotConnection);
}

void TPOnlineServices::handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only)
{
	int ret_code{-100};
	QString reply_string;
	QByteArray file_contents;

	if (reply)
	{
		reply->deleteLater();
		const QHttpHeaders &headers{reply->headers()};
		if (headers.contains("Content-Type"_L1))
		{
			const QString &fileType{headers.value("Content-Type"_L1).toByteArray()};
			if (fileType.contains("application/octet-stream"_L1) || fileType.contains("text/plain"_L1))
			{
				file_contents = std::move(reply->readAll());
				const qsizetype filename_sep_idx{file_contents.indexOf("%%")};
				if (filename_sep_idx >= 2)
				{
					ret_code = 0;
					reply_string = std::move(file_contents.sliced(0, filename_sep_idx));
					static_cast<void>(file_contents.slice(filename_sep_idx + 2, file_contents.size() - filename_sep_idx - 2));
				}
				else
				{
					reply_string = std::move(tr("Error downloading file"));
					ret_code = 2;
				}
			}
			else //Only text replies, including text files
			{
				reply_string = std::move(QString::fromUtf8(reply->readAll()));
				if (reply->error())
					reply_string += " ***** "_L1 + std::move(reply->errorString());
				#ifndef QT_NO_DEBUG
				qDebug() << reply_string << " * "_L1 << QString::number(requestid);
				#endif
				//Slice off "Return code: "
				const qsizetype ret_code_idx{reply_string.indexOf(':')};
				if (ret_code_idx >= 1)
				{
					ret_code = reply_string.sliced(0, ret_code_idx).toInt();
					static_cast<void>(reply_string.remove(0, ret_code_idx + 1));
				}
				reply_string = std::move(reply_string.trimmed());
			}
		}
		else
			reply_string = std::move(tr("Http headers missing \"Content-Type\""));
	}
	else
		reply_string = std::move(tr("No network reply"));
	if (!b_internal_signal_only)
	{
		if (file_contents.isEmpty())
			emit networkRequestProcessed(requestid, ret_code, reply_string);
		else
			emit fileReceived(requestid, ret_code, reply_string, file_contents);
	}
	else
		emit _networkRequestProcessed(requestid, ret_code, reply_string, file_contents);
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
		#ifndef QT_NO_QDEBUG
		qDebug() << url.toDisplayString() << " * "_L1 << QString::number(requestid);
		#endif
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
	#ifndef QT_NO_DEBUG
	qDebug() << "checkServerResponse() ret_code = " << ret_code << " , ret_string = " << ret_string << " , address = " << address;
	#endif
	uint online_status{TP_RET_CODE_SERVER_UNREACHABLE};
	if (ret_string.contains("Welcome to the TrainingPlanner"_L1))
		online_status = TP_RET_CODE_SUCCESS;
	else if (ret_string.contains("server paused"_L1))
		online_status = TP_RET_CODE_SERVER_PAUSED;
	emit _serverResponse(online_status, address);
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
