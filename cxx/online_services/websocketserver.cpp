#include "websocketserver.h"

#include "tpchat.h"
#include "tpmessagesmanager.h"
#include "tponlineservices.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtWebSockets>

ChatWSServer *ChatWSServer::app_ws_server{nullptr};

using namespace QLiterals;

static inline QString getIdentifier(QWebSocket *peer)
{
	return peer->peerAddress().toString() % ':' % QString::number(peer->peerPort());
}

ChatWSServer::ChatWSServer(const QString &id, QObject *parent)
	: QObject{parent}, m_id{id}, m_pWebSocketServer{new QWebSocketServer{id, QWebSocketServer::NonSecureMode, this}}
{
	app_ws_server = this;
	m_port = std::move(m_id.last(5));
}

ChatWSServer::~ChatWSServer()
{
	m_pWebSocketServer->close();
}

void ChatWSServer::setServerStatus(const bool enabled)
{
	if (!enabled && m_pWebSocketServer->isListening()) {
		m_pWebSocketServer->close();
		emit wsServerStatusChanged(enabled);
	}
	else if (enabled && !m_pWebSocketServer->isListening())
		setupWSServer();
}

void ChatWSServer::connectToPeer(QObject *local_peer, const int handle, const QString &userid, int n_attempts)
{
	if (isConnectionOK(userid)) {
		if (!m_localPeers.value(userid).contains(local_peer))
			m_localPeers[userid].append(local_peer);
		return;
	}
	const QLatin1String seed{"connectToPeer" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &ChatWSServer::gotPeerAddress, this, [=,this,&n_attempts] (const int request_id, const QString &address) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (address.contains("not logged"_L1)) {
				qDebug() << "****** WebSocket error: peer not logged: " << address;
				emit connectionAttemptResult(false, userid);
			}
			else {
				QWebSocket *peer{new QWebSocket(m_id, QWebSocketProtocol::Version13, this)};
				connect(peer, &QWebSocket::connected, this, [=,this] () {
					qDebug() << "****** WebSocket connected to " << userid;
					connect(peer, &QWebSocket::textMessageReceived, this, [this] (const QString &message) { wsTextMessageReceived(message); });
					connect(peer, &QWebSocket::binaryMessageReceived, this, [this] (const QByteArray &data) { wsBinaryMessageReceived(data); });
					connect(peer, &QWebSocket::disconnected, this, [this,peer,userid] () {
						peer->disconnect();
						peer->deleteLater();
						m_peersSockets.remove(userid);
						m_localPeers.remove(userid);
					});
					m_peersSockets.insert(userid, peer);
					QList<QObject*> local_peers{TPUtils::SFM_TOTAL_NUMBER_OF_METHODS, nullptr};
					local_peers[handle] = local_peer;
					m_localPeers.insert(userid, local_peers);
					emit connectionAttemptResult(true, userid);
				});
				// Handle errors (e.g., server not found, connection refused)
				QObject::connect(peer, &QWebSocket::errorOccurred, this, [=,this,&n_attempts] (QAbstractSocket::SocketError error) {
					auto err_func = [this,peer,userid,error] () -> void {
						qDebug() << "****** WebSocket error: " << error << " " << peer->errorString() << " " << peer->peerAddress();
					};
					switch (error) {
						case QAbstractSocket::ConnectionRefusedError:
						case QAbstractSocket::RemoteHostClosedError:
							peer->close();
							if (--n_attempts > 0)
								connectToPeer(local_peer, handle, userid, n_attempts);
							else
								err_func();
						break;
						default:
							err_func();
						break;
					}
				});
				peer->open(QUrl{"ws://"_L1 % address});
			}
		}
	});
	queryPeerAddress(requestid, userid);
}

bool ChatWSServer::isConnectionOK(const QString &userid) const
{
	QWebSocket *peer{m_peersSockets.value(userid)};
	return peer && peer->isValid();
}

bool ChatWSServer::sendTextMessage(const int handle, const QString &sender_id, const QString &receiver_id,
																		const QString &message, const TPFilePath &filename)
{
	QWebSocket *peer{m_peersSockets.value(receiver_id)};
	if (peer && peer->isValid())
		return peer->sendTextMessage(appUtils()->makeBinaryFileMetaInfo(
			handle, sender_id, receiver_id, filename.subdirs(), filename.fileName(), QChar{}, message)) > message.length();
	return false;
}

bool ChatWSServer::sendBinaryMessage(const int handle, const TPFilePath &filename, const QString &extra_info,
																							const bool remove_local_file)
{
	const QString &receiver_id{filename.targetUser()};
	const QString &info_fields{appUtils()->makeBinaryFileMetaInfo(handle, filename.ownerUser(), receiver_id,
															filename.subdirs(), filename.fileName(), QChar{}, extra_info)};
	QByteArray data{std::move(appUtils()->readBinaryFile(filename.toString(), info_fields))};
	QWebSocket *peer{m_peersSockets.value(receiver_id)};
	if (peer && peer->isValid()) {
		if (peer->sendBinaryMessage(data) == data.size() + info_fields.size()) {
			if (remove_local_file)
				QFile::remove(filename.fileName());
			return true;
		}
	}
	return false;
}

void ChatWSServer::wsTextMessageReceived(QString message)
{
	bool ok{false};
	const TPUtils::SEND_FILE_METHOD use{static_cast<TPUtils::SEND_FILE_METHOD>(appUtils()->binaryFileMetaInfoFieldValue(
																			message, TPUtils::BFIF_HANDLE).toUInt(&ok))};
	if (ok) {
		const QString &local_user{appUtils()->binaryFileMetaInfoFieldValue(message, TPUtils::BFIF_RECEIVERID)};
		const QList<QObject*> local_peers{m_localPeers.value(local_user)};
		const QString &remote_user{appUtils()->binaryFileMetaInfoFieldValue(message, TPUtils::BFIF_SENDERID)};
		if (!local_peers.isEmpty()) {
			QObject *local_peer{local_peers.at(use)};
			TPFilePath filename{appUtils()->binaryFileMetaInfoFieldValue(message, TPUtils::BFIF_FILEPATH)};
			if (local_peer) {
				switch (use) {
				case TPUtils::SFM_TPCHAT:
					qobject_cast<TPChat*>(local_peer)->processWebSocketTextMessage(message);
					break;
				case TPUtils::SFM_TPMESSAGESMANAGER: {
					qobject_cast<TPMessagesManager*>(local_peer)->textMesssageReceived(
								appUtils()->binaryFileMetaInfoFieldValue(message, TPUtils::BFIF_EXTRAINFO), filename);
					break;
				}
				case TPUtils::SFM_FILETRANSFER:
					filename.setOwnerUser(local_user);
					filename.setTargetUser(remote_user);
					sendBinaryMessage(TPUtils::SFM_FILETRANSFER, filename);
					break;
				default: return;
				}
			}
		}
	}
}

void ChatWSServer::wsBinaryMessageReceived(QByteArray data)
{
	const QString &data_meta_info{appUtils()->getBinaryFileMetaInfo(data)};
	if (data_meta_info.isEmpty()) {
		emit fileReceived(QByteArray{});
		return;
	}
	bool ok{false};
	const TPUtils::SEND_FILE_METHOD use{static_cast<TPUtils::SEND_FILE_METHOD>(appUtils()->binaryFileMetaInfoFieldValue(
																		data_meta_info, TPUtils::BFIF_HANDLE).toUInt(&ok))};
	if (ok) {
		const QString &local_user{appUtils()->binaryFileMetaInfoFieldValue(data_meta_info, TPUtils::BFIF_RECEIVERID)};
		const QList<QObject*> local_peers{m_localPeers.value(local_user)};
		if (!local_peers.isEmpty()) {
			QObject *local_peer{local_peers.at(use)};
			if (local_peer) {
				switch (use) {
				case TPUtils::SFM_TPCHAT:
					qobject_cast<TPChat*>(local_peer)->processWebSocketBinaryMessage(data, data_meta_info);
					break;
				case TPUtils::SFM_TPMESSAGESMANAGER:
					qobject_cast<TPMessagesManager*>(local_peer)->binaryFileReceived(data, data_meta_info);
					break;
				case TPUtils::SFM_FILETRANSFER:
					emit fileReceived(data);
					break;
				default: return;
				}
			}
		}
	}
}

void ChatWSServer::onNewConnection()
{
	auto p_socket{m_pWebSocketServer->nextPendingConnection()};
	if (p_socket) {
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  " << getIdentifier(p_socket) << " connected!";
		#endif
		const QString &id{p_socket->origin()};
		m_peersSockets.insert(id, p_socket);
		p_socket->setParent(this);

		//Remove all closed or otherwise invalid connections
		QHash<QString,QWebSocket*>::iterator itr{m_peersSockets.begin()};
		const QHash<QString,QWebSocket*>::iterator itr_end{m_peersSockets.end()};
		do {
			if (!(*itr)->isValid()) {
				m_peersSockets.remove(itr.key());
				delete (*itr);
			}
		} while (++itr != itr_end);
	}
}

void ChatWSServer::queryPeerAddress(const int requestid, const QString &userid)
{
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
													(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid) {
			disconnect(*conn);
			emit gotPeerAddress(requestid, ret_string);
		}
	});
	appOnlineServices()->getPeerAddress(requestid, userid);
}

void ChatWSServer::setupWSServer()
{
	connect(m_pWebSocketServer, &QWebSocketServer::serverError, this, [](QWebSocketProtocol::CloseCode code) {
		qDebug() << "WebSocket Server error:" << code;
	});
	if (m_pWebSocketServer->listen(QHostAddress{appSettings()->serverAddress()}, m_port.toUShort())) {
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  WebSocket Chat Server listening on : " <<
					m_pWebSocketServer->serverAddress().toString() + ':' + QString::number(m_pWebSocketServer->serverPort());
		#endif
		emit wsServerStatusChanged(true);
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &ChatWSServer::onNewConnection);
	}
	else {
		qDebug() << "Error starting websocket server:" << m_pWebSocketServer->errorString();
		emit wsServerStatusChanged(false);
	}
}
