#include "tponlineservices.h"

#include "scan_network.h"
#include "websocketserver.h"
#include "../dbusermodel.h"
#include "../qmlitemmanager.h"
#include "../osinterface.h"
#include "../tpdatabasetable.h"
#include "../tpfilepath.h"
#include "../tpsettings.h"
#include "../tputils.h"
#include "../return_codes.h"
#include "../tpkeychain/tpkeychain.h"

#include <QFile>
#include <QHash>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

using namespace Qt::StringLiterals;
TPOnlineServices* TPOnlineServices::_appOnlineServices{nullptr};

constexpr uint file_upload_max_size{8*1024*1024};
constexpr QLatin1StringView server_address{"http://%1:%2/trainingplanner/"_L1};
static const QString &root_user{"admin"_L1};
static const QString &root_passwd{"admin"_L1};

TPOnlineServices::TPOnlineServices(QObject *parent) : QObject{parent}, m_onlineStatus{TP_RET_CODE_UNKNOWN_ERROR}
{
	_appOnlineServices = this;
	m_networkManager = new QNetworkAccessManager{this};
}

void TPOnlineServices::testServerConnection(const QString &address, const QString &port, const int requestid)
{
	if (checkRequestPool(requestid, "testServerConnection()"_L1))
		return;
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
									(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			uint8_t online_status{TP_RET_CODE_SERVER_UNREACHABLE};
			if (ret_string.contains("Welcome to the TrainingPlanner"_L1))
				online_status = TP_RET_CODE_SUCCESS;
			else if (ret_string.contains("server paused"_L1, Qt::CaseInsensitive))
				online_status = TP_RET_CODE_SERVER_PAUSED;
			else if (ret_string.contains("bad gateway"_L1, Qt::CaseInsensitive))
				online_status = TP_RET_CODE_SERVER_NOT_RUNNING;

			if (m_onlineStatus != online_status) {
				switch (online_status) {
				case TP_RET_CODE_SUCCESS:
					if (appSettings()->serverAddress() != address)
						appSettings()->setServerAddress(address);
					if (appSettings()->serverPort() != port)
						appSettings()->setServerPort(port);
					m_serverAddress = std::move(server_address.arg(address, port));
					if (m_hasCredentials)
						emit onlineServicesReady();
					break;
				case TP_RET_CODE_SERVER_PAUSED:
					break;
				case TP_RET_CODE_SERVER_UNREACHABLE:
				case TP_RET_CODE_SERVER_NOT_RUNNING:
					if (request_id == -1) {
						appSettings()->setServerAddress(QString{});
						QTimer::singleShot(5000, this, [this] () -> void { connectToServer(); });
					} else {
						//when searching for a viable interface address, ignore unsuccessfull tests after a successfull one
						if (m_onlineStatus == TP_RET_CODE_SUCCESS && request_id >= 100)
							return;
					}
					break;
				default: Q_UNREACHABLE();
				}
				appOsInterface()->setWorkingNetInterface(online_status == TP_RET_CODE_SUCCESS ? request_id > 0 ? request_id - 100 : -1 : 0);
				m_onlineStatus = online_status;
				emit serverStatusChanged(m_onlineStatus, address, request_id);
			}
		}
	});
	makeNetworkRequest(requestid, server_address.arg(address, port), true);
}

void TPOnlineServices::connectToServer()
{
#ifdef LOCAL_TPSERVER
	const QString &address{appSettings()->serverAddress()};
	if (address.isEmpty()) {
		connect(appOsInterface(), &OSInterface::serverAddressesFetched, this, [this] (const QHash<int,QString> &addresses) {
			QHash<int,QString>::const_iterator itr{addresses.constBegin()};
			const QHash<int,QString>::const_iterator itr_end{addresses.constEnd()};
			while (itr != itr_end) {
				const int requestid{itr.key() == -1 ? -1 : 100 + itr.key()};
				const int port_sep(itr.value().indexOf(':'));
				testServerConnection(itr.value().first(port_sep), itr.value().last(itr.value().length() - port_sep - 1), requestid);
				++itr;
			}
		});
#ifdef TPSERVER_MACHINE
		appOsInterface()->checkLocalServer();
#else
		appOsInterface()->getAvailableAddresses();
#endif
		return;
	} else {
		if (tpScanNetwork::ping(address)) {
			testServerConnection(address, appSettings()->serverPort());
		} else {
			appSettings()->setServerAddress(QString{});
			connectToServer();
		}
	}
#else
	testServerConnection("www.tpserver.com"_L1, 0);
#endif
}

void TPOnlineServices::storeCredentials()
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,conn]
										(const bool ok, const QString &key, const QString &value) {
		if (key == appUserModel()->userId(0)) {
			disconnect(*conn);
			if (ok) {
				m_userid = key;
				m_passwd = value;
				m_hasCredentials = true;
				if (m_onlineStatus == TP_RET_CODE_SUCCESS)
					emit onlineServicesReady();
			}
		}
	});
	appKeyChain()->readKey(appUserModel()->userId(0));
}

#ifndef Q_OS_ANDROID
void TPOnlineServices::getAllUsers(const int requestid)
{
	if (checkRequestPool(requestid, "getAllUsers()"_L1))
		return;
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [this,conn,requestid]
												(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			QStringList users;
			if (ret_code == TP_RET_CODE_SUCCESS)
				users = std::move(ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts));
			emit networkListReceived(request_id, ret_code, users);
		}
	});
	const QUrl url{makeCommandURL(true, "allusers"_L1)};
	makeNetworkRequest(requestid, url, true);
}
#endif

void TPOnlineServices::checkUserAccount(const int requestid, const QString &query, const QString &passwd)
{
	const QUrl url{makeCommandURL(true, "checkaccount"_L1, query, "userpassword"_L1, passwd)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getOnlineUserData(const int requestid, const QString &user_id)
{
	const QUrl url{makeCommandURL(true, "onlinedata"_L1, user_id)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::userLogin(const int requestid)
{
	const QUrl url{makeCommandURL(false, "login"_L1, appWSServer()->port(), "device"_L1, appOsInterface()->deviceID())};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::userLogout(const int requestid)
{
	if (appUserModel()->mainUserLoggedIn()) {
		const QUrl url{makeCommandURL(false, "logout"_L1, QString{}, "device"_L1, appOsInterface()->deviceID())};
		makeNetworkRequest(requestid, url);
	}
}

void TPOnlineServices::registerUser(const int requestid)
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,requestid,conn]
												(const bool ok, const QString &key, const QString &value) {
		if (key == appUserModel()->userId(0)) {
			disconnect(*conn);
			if (ok) {
				const QUrl url{makeCommandURL(true, "adduser"_L1, key, "userpassword"_L1, value)};
				makeNetworkRequest(requestid, url);
			}
		}
	});
	appKeyChain()->readKey(appUserModel()->userId(0));
}

void TPOnlineServices::removeUser(const int requestid, const QString &userid)
{
	const QUrl url{makeCommandURL(true, "deluser"_L1, userid)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getPeerAddress(const int requestid, const QString &userid)
{
	const QUrl url{makeCommandURL(false, "getpeeraddress"_L1, userid)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::getDevicesList(const int requestid)
{
	if (checkRequestPool(requestid, "getDevicesList()"_L1))
		return;
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			QStringList devices_list;
			if (ret_code == TP_RET_CODE_SUCCESS)
				devices_list = std::move(ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts));
			emit networkListReceived(request_id, ret_code, devices_list);
		}
	});
	const QUrl url{makeCommandURL(false, "getdeviceslist"_L1)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::addOrRemoveCoach(const int requestid, const bool bAdd)
{
	const QUrl url{makeCommandURL(false, bAdd ? "addcoach"_L1 : "delcoach"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::changePassword(const int requestid, const QString &old_passwd, const QString &new_passwd)
{
	const QUrl url{makeCommandURL(false, "changepassword"_L1, old_passwd, "newpassword"_L1, new_passwd)};
	makeNetworkRequest(requestid, url, false);
}

void TPOnlineServices::getOnlineCoachesList(const int requestid)
{
	const QUrl url{makeCommandURL(false, "getonlinecoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendRequestToCoach(const int requestid, const QString& coach_net_name)
{
	const QUrl url{makeCommandURL(false, "requestcoach"_L1, coach_net_name)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkClientsRequests(const int requestid)
{
	const QUrl url{makeCommandURL(false, "listclientsrequests"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientRequest(const int requestid, const QString &client)
{
	const QUrl url{makeCommandURL(false, "deleteclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptClientRequest(const int requestid, const QString &client)
{
	const QUrl url{makeCommandURL(false, "acceptclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectClientRequest(const int requestid, const QString &client)
{
	const QUrl url{makeCommandURL(false, "rejectclientrequest"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCoachesAnswers(const int requestid)
{
	const QUrl url{makeCommandURL(false, "listcoachesanswers"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachAnwers(const int requestid, const QString &coach)
{
	const QUrl url{makeCommandURL(false, "deletecoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::acceptCoachAnswer(const int requestid, const QString &coach)
{
	const QUrl url{makeCommandURL(false, "acceptcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::rejectCoachAnswer(const int requestid, const QString &coach)
{
	const QUrl url{makeCommandURL(false, "rejectcoachanswer"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentClients(const int requestid)
{
	const QUrl url{makeCommandURL(false, "getclients"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeClientFromCoach(const int requestid,
												const QString &client)
{
	const QUrl url{makeCommandURL(false, "removecurclient"_L1, client)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkCurrentCoaches(const int requestid)
{
	const QUrl url{makeCommandURL(false, "getcoaches"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeCoachFromClient(const int requestid, const QString &coach)
{
	const QUrl url{makeCommandURL(false, "removecurcoach"_L1, coach)};
	makeNetworkRequest(requestid, url);
}

std::pair<TPBool,int> TPOnlineServices::sendFileToServer(const TPFilePath &tp_filename, const bool remove_local_file)
{
	QFileInfo fi{tp_filename.toString()};
	if (fi.size() > file_upload_max_size) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_ERROR, std::move(
			appUtils()->string_strings({ tr("Cannot upload file"), tr("Maximum file size allowed: 8MB")}, record_separator)));
		return {TPBool{}, TP_RET_CODE_FILE_TOO_BIG};
	}
	int requestid{serverCommandStarter(tp_filename.generateUniqueId(), std::move(tr("File upload: ") % tp_filename.fileName()))};
	if (requestid < 0 || requestid > (TP_RET_CODE_DEFERRED_ACTION + 100)) {
		QFile *upload_file{appUtils()->openFile(tp_filename.toString(), true, false, false, false, false)};
		if (upload_file) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(this, &TPOnlineServices::networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid) {
					disconnect(*conn);
					emit fileUploaded(requestid, ret_code);
					upload_file->close();
					if (remove_local_file)
						QFile::remove(upload_file->fileName());
					delete upload_file;
				}
			});
			sendFile(requestid, tp_filename, upload_file);
			return {TPBool{true}, requestid};
		} else {
			requestid = TP_RET_CODE_OPEN_CREATE_FAILED;
		}
	}
	return {TPBool{}, requestid};
}

/*tp_filename must be OK and point to a *local* file path(existing or not), i.e. the destination file path of the
 * downloaded file. ownerUser() will be prepended to subDir(), and targetUser() is the $target_user argument in
 * url_parser.h, that is, the "owner user" in the server's app directory
*/
std::pair<TPBool,int> TPOnlineServices::downloadFileFromServer(const TPFilePath &tp_filename)
{
	if (appUtils()->fileRecentlyModified(tp_filename.toString(), 30))
		return {TPBool{true}, TP_RET_CODE_NO_CHANGES_SUCCESS};
	const int requestid{serverCommandStarter(tp_filename.generateUniqueId(), std::move(tr("File download: ") % tp_filename.fileName()))};
	if (requestid < 0 || requestid > (TP_RET_CODE_DEFERRED_ACTION + 100)) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &TPOnlineServices::fileReceived, this, [=,this]
					(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
			if (request_id == requestid) {
				disconnect(*conn);
				bool success{!contents.isEmpty()};
				static_cast<void>(appUtils()->mkdir(tp_filename.toString()));
				switch (ret_code) {
				case TP_RET_CODE_SUCCESS:
					if (success) {
						QFile *local_file{new QFile{tp_filename.toString(), this}};
						if (!local_file->exists() || local_file->remove()) {
							if (local_file->open(QIODeviceBase::WriteOnly)) {
								local_file->write(contents);
								local_file->close();
							}
						}
						delete local_file;
					}
					break;
				case TP_RET_CODE_NO_CHANGES_SUCCESS: //online file and local file are the same
					success = true;
					break;
				default: //some error
					success = false;
				}
				if (!success)
					qCritical() << "Error! " << filename << '(' << QString::number(ret_code) << ") "_L1 % contents;
				emit fileDownloaded(requestid, ret_code, tp_filename);
			}
		});
		getFile(requestid, tp_filename);
		return {TPBool{true}, requestid};
	} else {
		return {TPBool{}, requestid};
	}
}

void TPOnlineServices::removeFileFromServer(const TPFilePath &tp_filename)
{
	const int requestid{serverCommandStarter(tp_filename.generateUniqueId(),
											 std::move(tr("File removal: ") % tp_filename.fileName()))};
	if (requestid < 0 || requestid > (TP_RET_CODE_DEFERRED_ACTION + 100))
		removeFile(requestid, tp_filename);
}

std::pair<TPBool,int> TPOnlineServices::listFilesOrDirs(const bool files, const bool dirs, const bool admin,
			const QString &target_user, const QString &subdir, const QString &pattern, const bool recursive)
{
	QLatin1StringView v{QString{(files ? "listfiles"_L1 : "listdirs"_L1) % (admin ? root_user : m_userid)}.toLatin1().constData()};
	const int requestid{serverCommandStarter(appUtils()->generateUniqueId(v), std::move(tr("Get list: ") % subdir))};
	if (requestid < 0 || requestid > (TP_RET_CODE_DEFERRED_ACTION + 100)) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
										(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid) {
				disconnect(*conn);
				QStringList files_list;
				if (ret_code == TP_RET_CODE_SUCCESS)
					parseReceivedFilesList(files_list, ret_string);
				emit networkListReceived(request_id, ret_code, files_list);
			}
		});
		const QUrl url{makeCommandURL(admin
									, files ? "listfiles"_L1 : QString{}
									, dirs ? "listdirs"_L1 : QString{}
									, "owner"_L1, admin ? target_user : m_userid
									, "subdir"_L1, subdir
									, "pattern"_L1, pattern
									, recursive ? "recursive"_L1 : QString{})};
		makeNetworkRequest(requestid, url, true);
		return std::pair<TPBool,int>{true, requestid};
	}
	return std::pair<TPBool,int>{};
}

void TPOnlineServices::sendCmdFileToServer(const QString &cmd_filename)
{
	TPFilePathPtr tp_filename{TPFilePath::newTPFilePath(cmd_filename)};
	const auto res{sendFileToServer(*tp_filename, true)};
	if (res.first()) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &TPOnlineServices::fileUploaded, this, [=,this] (const uint requestid, const int ret_code) {
			if (res.second == requestid) {
				disconnect(*conn);
				if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
					executeCommands(requestid, tp_filename->subdirs());
			}
		});
	}
}

void TPOnlineServices::downloadCmdFilesFromServer()
{
	std::pair<TPBool,int> res{listFilesOrDirs(true, false, false, QString{}, TPDatabaseTable::cmdsSubDir,
																			TPDatabaseTable::cmd_file_extension)};
	if (!res.first)
		return;
	const int request_id{res.second};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::networkListReceived, this, [this,conn,request_id]
									(const int requestid, const int ret_code, const QStringList &files_list) {
		if (requestid == request_id) {
			disconnect(*conn);
			if (files_list.isEmpty())
				return;

			TPFilePath tp_filename;
			for (const auto &file : std::as_const(files_list)) {
				tp_filename = file;
				const auto res{downloadFileFromServer(tp_filename)};
				auto parseCmd = [this] (const QString &cmd_file) {
					TPDatabaseTable::parseCmdFile(cmd_file);
					QFile::remove(cmd_file);
				};
				if (res.first) {
					if (res.second == TP_RET_CODE_NO_CHANGES_SUCCESS) {
						parseCmd(tp_filename.toString());
					} else {
						auto conn2{std::make_shared<QMetaObject::Connection>()};
						*conn2 = connect(this, &TPOnlineServices::fileDownloaded, this, [this,conn2,res,parseCmd]
										(const int ret_code, const uint requestid, const TPFilePath &local_file_name) {
							if (res.second == requestid) {
								disconnect(*conn2);
								if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
									parseCmd(local_file_name.toString());
							}
						});
					}
				}
			}
		}
	});
}

void TPOnlineServices::executeCommands(const int requestid, const QString &subdir)
{
	const QUrl url{makeCommandURL(false, "runcmds"_L1, subdir)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkTPMessages(const int requestid)
{
	if (checkRequestPool(requestid, "checkTPMessages()"_L1))
		return;
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
												(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			const QStringList &messages{ret_string.split('\n')};
			emit networkListReceived(request_id, ret_code, messages);
		}
	});
	const QUrl url{makeCommandURL(false, "gettpmessages"_L1)};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::sendTPMessage(const int requestid, const QString &message, const QString &target_user)
{
	const QUrl url{makeCommandURL(false, "sendtpmessage"_L1, target_user, "message"_L1, message)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeTPMessage(const int requestid, const QString &message)
{
	const QUrl url{makeCommandURL(false, "removetpmessage"_L1, message)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::checkChatMessages(const int requestid)
{
	const QUrl url{makeCommandURL(false, "getnewchatmessages"_L1)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::sendChatMessage(const int requestid, const QString &receiver, const QString &encoded_message)
{
	const QUrl url{makeCommandURL(false, "sendchatmessage"_L1, receiver, "message"_L1, encoded_message)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::removeChatMessage(const int requestid, const QString &receiver, const QString &encoded_message)
{
	const QUrl url{makeCommandURL(false, "removechatmessage"_L1, receiver, "message"_L1, encoded_message)};
	makeNetworkRequest(requestid, url);
}

void TPOnlineServices::recheckNewChatMessages()
{
	const QUrl url{makeCommandURL(false, "forcegetnewmessages"_L1)};
	static_cast<void>(m_networkManager->get(QNetworkRequest{url}));
}

inline bool TPOnlineServices::canConnectToServer() const { return m_onlineStatus == TP_RET_CODE_SUCCESS; }

int TPOnlineServices::serverCommandStarter(int requestid, QString &&command_description) const
{
	if (!canConnectToServer()) {
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_SERVER_UNREACHABLE, std::forward<QString>(command_description));
		requestid = TP_RET_CODE_SERVER_UNREACHABLE;
	} else {
		if (!appUserModel()->mainUserLoggedIn())
			requestid = TP_RET_CODE_USER_OFFLINE;
	}
	if (checkRequestPool(requestid, QLatin1StringView{command_description.toLocal8Bit().data()}))
		requestid = TP_RET_CODE_REQUESTID_IN_USE;
	return requestid;
}

QString TPOnlineServices::makeCommandURL(const bool admin, auto && ... args)
{
	const QString *userid, *password;
	if (!admin) {
		userid = &m_userid;
		password = &m_passwd;
	} else {
		userid = &root_user;
		password = &root_passwd;
	}

	/*if constexpr(1 > (sizeof... (args))) {
		return{};
	}*/
	// transform arguments parameter pack to tuple
	auto tup_list = std::make_tuple( args ... );
	// the type of the whole tuple instance as an added bonus
	using arg_list_type = decltype(tup_list);
	// how many elements in the tuple(should be the same as sizeof...(args)
	//constexpr std::size_t tuple_size_v{std::tuple_size<arg_list_type>::value};
	QString url{std::move(m_serverAddress % "?user="_L1 % *userid % "&password="_L1 % *password)};
	auto tup_value = [&url]<std::size_t... I>(arg_list_type&& tup, std::index_sequence<I...>) {
		( [&] {
			const auto &arg{std::get<I>(tup)};
			//typeid(arg).name();
			if (!arg.isEmpty()) {
				if (url.endsWith('='))
					url += arg;
				else
					url += '&' % arg % '=';
			}
		}(), ...);
	};
	tup_value(std::forward<arg_list_type>(tup_list),
			  std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<arg_list_type>>>{});
	return url;
}

void TPOnlineServices::makeNetworkRequest(const int requestid, const QUrl &url, const bool b_internal_signal_only)
{
	if (checkRequestPool(requestid, QLatin1StringView{QString{"makeNetworkRequest("_L1 % url.toString() % ')'}
																							.toLocal8Bit().data()}))
		return;
	#ifndef QT_NO_DEBUG
	qInfo() << url.toDisplayString() << " * "_L1  << QString::number(requestid);
	#endif
	setRequestToPool(requestid, true);
	QNetworkReply *reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
		handleServerRequestReply(requestid, reply, b_internal_signal_only);
	}, Qt::SingleShotConnection);
}

void TPOnlineServices::handleServerRequestReply(const int requestid, QNetworkReply *reply, const bool b_internal_signal_only)
{
	int ret_code{TP_RET_CODE_UNKNOWN_ERROR};
	QString reply_string;
	QByteArray file_contents;

	if (reply && reply->error() == QNetworkReply::NoError) {
		reply->deleteLater();
		const QHttpHeaders &headers{reply->headers()};
		if (headers.contains("Content-Type"_L1)) {
			const QString &fileType{headers.value("Content-Type"_L1).toByteArray()};
			if (fileType.contains("application/octet-stream"_L1) || fileType.contains("text/plain"_L1)) { //file download replies
				file_contents = std::move(reply->readAll());
				const qsizetype filename_sep_idx{file_contents.indexOf("^^"_L1)};
				if (filename_sep_idx >= 2) {
					reply_string = std::move(file_contents.sliced(0, filename_sep_idx));
					static_cast<void>(file_contents.slice(filename_sep_idx + 2, file_contents.size() - filename_sep_idx - 2));
					ret_code = TP_RET_CODE_SUCCESS;
				} else {
					reply_string = std::move(tr("Error downloading file"));
					ret_code = TP_RET_CODE_DOWNLOAD_FAILED;
				}
			} else { //Only-text replies
				reply_string = std::move(QString::fromUtf8(reply->readAll()));
				if (reply->error())
					reply_string += " ***** "_L1 + std::move(reply->errorString());
				#ifndef QT_NO_DEBUG
				qInfo() << reply_string << " * "_L1 << QString::number(requestid);
				#endif
				//Slice off "Return code: "
				const qsizetype ret_code_idx{reply_string.indexOf(':')};
				if (ret_code_idx >= 1) {
					ret_code = reply_string.sliced(0, ret_code_idx).toInt();
					static_cast<void>(reply_string.remove(0, ret_code_idx + 1));
				} else {
					ret_code = TP_RET_CODE_INVALID_REQUEST_METHOD;
				}
				reply_string = std::move(reply_string.trimmed());
			}
		} else {
			reply_string = std::move(tr("Http headers missing \"Content-Type\""));
			connectToServer(); //disconnected from server? why? Ttry to reconnect
		}
	} else {
		reply_string = std::move(tr("No network reply"));
		if (reply)
			reply_string += " - "_L1 % reply->errorString();
		connectToServer(); //disconnected from server? why? Ttry to reconnect
	}
	setRequestToPool(requestid, false);
	if (!b_internal_signal_only) {
		if (file_contents.isEmpty())
			emit networkRequestProcessed(requestid, ret_code, reply_string);
		else
			emit fileReceived(requestid, ret_code, reply_string, file_contents);
	} else {
		emit _networkRequestProcessed(requestid, ret_code, reply_string, file_contents);
	}
}

void TPOnlineServices::sendFile(const int requestid, const TPFilePath &tp_filename, QFile *file)
{
	if (checkRequestPool(requestid, QLatin1StringView{QString{"sendFile("_L1 % tp_filename.fileName() % ')'}
																						.toLocal8Bit().data()}))
		return;
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
										(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				if (remoteFileUpToDate(ret_string, file->fileName())) { //remote file is up to date. Don't send anything
					emit networkRequestProcessed(requestid, TP_RET_CODE_NO_CHANGES_SUCCESS,
												 tr("File on the online server already up to date"));
					return;
				}
			}
			const QUrl url{makeCommandURL(false,
										   "upload"_L1, tp_filename.fileName(),
										   "owner"_L1, tp_filename.targetUser(),
										   "target"_L1, tp_filename.ownerUser(),
										   "subdirs"_L1, tp_filename.subdirs())};
			uploadFile(requestid, url, file, true);
		}
	});
	//(tp_filename.targetUser().isEmpty() ?
	//	 QString{} : tp_filename.ownerUser()) % u'/' % tp_filename.subdirs(), tp_filename.targetUser()

	const QUrl url{makeCommandURL(false,
									"checkfilectime"_L1, tp_filename.fileName(),
									"owner"_L1, tp_filename.targetUser(),
									"target"_L1, tp_filename.ownerUser(),
									"subdirs"_L1, tp_filename.subdirs())};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::getFile(const int requestid, const TPFilePath &tp_filename, bool check_ctime_first)
{
	if (checkRequestPool(requestid, QLatin1StringView{QString{"getFile("_L1 % tp_filename.fileName() % ')'}
																						.toLocal8Bit().data()}))
		return;
	if (check_ctime_first) {
		QFileInfo fi{tp_filename.toString()};
		check_ctime_first = (fi.isFile() && fi.isWritable());
	}
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &TPOnlineServices::_networkRequestProcessed, this, [=,this]
					(const int request_id, const int ret_code, const QString &ret_string, const QByteArray &contents) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (ret_code == TP_RET_CODE_SUCCESS) {
				if (check_ctime_first) {
					if (!localFileUpToDate(ret_string, tp_filename.toString())) {
						getFile(requestid, tp_filename, false);
						return;
					} else { //if local file is up to date, we'll use it
						emit fileReceived(request_id, TP_RET_CODE_NO_CHANGES_SUCCESS, ret_string, contents);
						return;
					}
				}
			}
			emit fileReceived(request_id, ret_code, ret_string, contents);
		}
	});
	const QUrl url{makeCommandURL(false,
									check_ctime_first ? "get_file"_L1 : "checkfilectime"_L1, tp_filename.fileName(),
									"owner"_L1, tp_filename.targetUser(),
									"target"_L1, tp_filename.ownerUser(),
									"subdirs"_L1, tp_filename.subdirs())};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::removeFile(const int requestid, const TPFilePath &tp_filename)
{
	const QUrl url{makeCommandURL(false, "delfile"_L1, tp_filename.fileName(),
								   "owner"_L1, tp_filename.targetUser(),
								   "target"_L1, tp_filename.ownerUser(),
								   "subdirs"_L1, tp_filename.subdirs())};
	makeNetworkRequest(requestid, url, true);
}

void TPOnlineServices::getCmdFile(const int requestid, const QString &filename, const QString &subdir)
{
	const QUrl url{makeCommandURL(false, "downloadcmd"_L1, filename, "subdir"_L1, subdir, "deviceid"_L1, appOsInterface()->deviceID())};
	makeNetworkRequest(requestid, url, true);
}

//curl -X POST -F file=@/home/guilherme/Documents/Fase_de_transição_-_Completo.txt "http://127.0.0.1/trainingplanner/?user=uc_guilherme_fortunato&upload&password=Guilherme_Fortunato"
void TPOnlineServices::uploadFile(const int requestid, const QUrl &url, QFile *file, const bool b_internal_signal_only)
{
	if (file->isOpen()) {
		QNetworkRequest request{url};
		// Add the file as a part
		QHttpPart filePart;
		filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
							QVariant{"multipart/form-data; name=\"file\"; filename=\""_L1 % file->fileName() % "\""_L1});
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"_L1));
        filePart.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
        filePart.setBodyDevice(file);

		// Prepare the multipart data
		QHttpMultiPart *multiPart{new QHttpMultiPart{QHttpMultiPart::FormDataType, this}};
		multiPart->append(filePart);
		//file->setParent(multiPart); // MultiPart will manage file deletion
		#ifndef QT_NO_QDEBUG
		qInfo() << url.toDisplayString() << " * "_L1 << QString::number(requestid);
		#endif
		// Send the request
		QNetworkReply *reply{m_networkManager->post(request, multiPart)};
		connect(reply, &QNetworkReply::finished, this, [this,requestid,reply,b_internal_signal_only]() {
			handleServerRequestReply(requestid, reply, b_internal_signal_only);
		}, Qt::SingleShotConnection);
		multiPart->setParent(reply); // Let the reply manage the multipart's lifecycle
	}
}

void TPOnlineServices::parseReceivedFilesList(QStringList &files, const QString &ret_string)
{
	TPFilePath local_file;
	const QStringList &remote_files_list{ret_string.split(fancy_record_separator1, Qt::SkipEmptyParts)};
	for (uint i{0}; i < remote_files_list.count(); i += 2) {
		local_file = remote_files_list.at(i);
		const QString &online_date{remote_files_list.at(i + 1)};
		if (online_date.length() > 5) {
			if (localFileUpToDate(online_date, local_file.toString()))
				continue;
		}
		files.append(std::move(local_file.toString()));
	}
}

bool TPOnlineServices::remoteFileUpToDate(const QString &onlineDate, const QString &localFile) const
{
	QFileInfo fi{localFile};
	if (fi.exists()) {
		const QDateTime &c_time{fi.lastModified()};
		const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(onlineDate)};
		return online_ctime >= c_time;
	}
	return true;
}

bool TPOnlineServices::localFileUpToDate(const QString &onlineDate, const QString &localFile) const
{
	QFileInfo fi{localFile};
	if (fi.exists()) {
		const QDateTime &c_time{fi.lastModified()};
		const QDateTime &online_ctime{appUtils()->getDateTimeFromOnlineString(onlineDate)};
		return c_time >= online_ctime;
	}
	return false;
}
