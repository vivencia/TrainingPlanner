#include "websocketserver.h"

#include "tpchat.h"
#include "tpmessagesmanager.h"
#include "tponlineservices.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtWebSockets>

WSServer *WSServer::app_ws_server{nullptr};

using namespace QLiterals;

static inline QString getIdentifier(QWebSocket *peer)
{
	return peer->peerAddress().toString() % ':' % QString::number(peer->peerPort());
}

WSServer::WSServer(const QString &id, QObject *parent)
	: QObject{parent}, m_id{id}, m_pWebSocketServer{new QWebSocketServer{id, QWebSocketServer::NonSecureMode, this}}
{
	app_ws_server = this;
	m_port = std::move(m_id.last(5));
}

WSServer::~WSServer()
{
	m_pWebSocketServer->close();
}

void WSServer::setServerStatus(const bool enabled)
{
	if (!enabled && m_pWebSocketServer->isListening()) {
		m_pWebSocketServer->close();
		emit wsServerStatusChanged(enabled);
	}
	else if (enabled && !m_pWebSocketServer->isListening())
		setupWSServer();
}

void WSServer::connectToPeer(QObject *handler, const int handle, const QString &userid, int n_attempts)
{
	if (isConnectionOK(userid)) {
		if (!m_handlers.value(userid).contains(handler))
			m_handlers[userid].append(handler);
		return;
	}
	const QLatin1String seed{"connectToPeer" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &WSServer::gotPeerAddress, this, [=,this,&n_attempts] (const int request_id, const QString &address) {
		if (request_id == requestid) {
			disconnect(*conn);
			if (address.contains("not logged"_L1)) {
				qDebug() << "****** WebSocket error: peer not logged: "_L1 << address;
				emit _connectionAttemptResult(false, userid);
			}
			else {
				QWebSocket *peer{new QWebSocket(m_id, QWebSocketProtocol::VersionLatest, this)};
				connect(peer, &QWebSocket::connected, this, [=,this] () {
					qDebug() << "****** WebSocket connected to "_L1 << userid;
					connect(peer, &QWebSocket::textMessageReceived, this, [this] (const QString &message) { wsTextMessageReceived(message); });
					connect(peer, &QWebSocket::binaryMessageReceived, this, [this] (const QByteArray &data) { wsBinaryMessageReceived(data); });
					connect(peer, &QWebSocket::disconnected, this, [this,peer,userid] () {
						peer->disconnect();
						peer->deleteLater();
						m_peersSockets.remove(userid);
						m_handlers.remove(userid);
					});
					m_peersSockets.insert(userid, peer);
					QList<QObject*> local_handlers{TPUtils::MH_UNKOWN, nullptr};
					local_handlers[handle] = handler;
					m_handlers.insert(userid, local_handlers);
					emit _connectionAttemptResult(true, userid);
				});
				// Handle errors (e.g., server not found, connection refused)
				QObject::connect(peer, &QWebSocket::errorOccurred, this, [=,this,&n_attempts] (QAbstractSocket::SocketError error) {
					auto err_func = [this,peer,userid,error] () -> void {
						qDebug() << "****** WebSocket error: "_L1 << error << " " << peer->errorString() << " "_L1 << peer->peerAddress();
						peer->deleteLater();
						emit _connectionAttemptResult(false, userid);
					};
					switch (error) {
					case QAbstractSocket::ConnectionRefusedError:
					case QAbstractSocket::RemoteHostClosedError:
						peer->close();
						if (--n_attempts > 0)
							connectToPeer(handler, handle, userid, n_attempts);
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

bool WSServer::isConnectionOK(const QString &userid, const bool try_to_connect) const
{
	QWebSocket *peer{m_peersSockets.value(userid)};
	const bool conn_ok{peer && peer->isValid()};
	if (!conn_ok && try_to_connect) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &WSServer::_connectionAttemptResult, this, [this,conn,userid]
															(const bool established, const QString &user_id) {
			if (userid == user_id) {
				disconnect(*conn);
				emit const_cast<WSServer*>(this)->connectionAttemptResult(established, user_id);
			}
		});
		appWSServer()->connectToPeer(appMessagesManager(), TPUtils::MH_TPMESSAGES_MANAGER, userid);
	}
	return conn_ok;
}

bool WSServer::sendTextMessage(const QString &encoded_message)
{
	QWebSocket *peer{m_peersSockets.value(appUtils()->encodedMessageFieldValue(encoded_message, TPUtils::EF_RECEIVER))};
	if (peer && peer->isValid())
		return peer->sendTextMessage(encoded_message);
	return false;
}

bool WSServer::sendBinaryMessage(const TPFilePath &local_filename, const TPFilePath &target_filename, const bool remove_local_file)
{
	bool sent_ok{false};
	const QString &receiver_id{local_filename.targetUser()};
	QWebSocket *peer{m_peersSockets.value(receiver_id)};
	if (peer && peer->isValid()) {
		QByteArray data{std::move(QString{TPUtils::filetransfer_prefix % target_filename.relativeFilePath() % binary_file_separator}.toUtf8())};
		data.append(std::move(appUtils()->readBinaryFile(local_filename.toString())));
		sent_ok = peer->sendBinaryMessage(data) == data.size();
		if (sent_ok && remove_local_file)
			static_cast<void>(QFile::remove(local_filename.fileName()));
	}
	return sent_ok;
}

void WSServer::wsTextMessageReceived(const QString &message)
{
	const TPUtils::MessageHandlers handler_id{appUtils()->messagePrefixToMessageHandler(message)};
	if (handler_id != TPUtils::MH_UNKOWN) {
		const QString &remote_user{appUtils()->encodedMessageFieldValue(message, TPUtils::EF_SENDER)};
		const QList<QObject*> local_peers{m_handlers.value(remote_user)};
		if (!local_peers.isEmpty()) {
			QObject *handler{local_peers.at(static_cast<int>(handler_id))};
			if (handler) {
				switch (handler_id) {
				case TPUtils::MH_DIRECT_FILE_TRANSFER:
					appMessagesManager()->textMessageReceived(message);
					break;
				case TPUtils::MH_TPCHAT:
					qobject_cast<TPChat*>(handler)->processChatMessage(message);
					break;
				default:
					Q_UNREACHABLE();
				}
			}
		}
	}
}

void WSServer::wsBinaryMessageReceived(QByteArray data)
{
	QString meta_data{data.first(data.indexOf(binary_file_separator.toLatin1()))};
	data.remove(0, meta_data.length() + 1);
	meta_data.remove(0, TPUtils::filetransfer_prefix.length());
	TPFilePath new_tpfilepath(meta_data);
	const auto bytes_written{appUtils()->writeBinaryFile(new_tpfilepath.toString(), data)};
	emit fileReceived(new_tpfilepath, bytes_written == data.size());
}

void WSServer::onNewConnection()
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

void WSServer::queryPeerAddress(const int requestid, const QString &userid)
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

void WSServer::setupWSServer()
{
	connect(m_pWebSocketServer, &QWebSocketServer::serverError, this, [](QWebSocketProtocol::CloseCode code) {
		qDebug() << "WebSocket Server error: "_L1 << code;
	});
	if (m_pWebSocketServer->listen(QHostAddress{appSettings()->serverAddress()}, m_port.toUShort())) {
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  WebSocket Chat Server listening on : "_L1 <<
					m_pWebSocketServer->serverAddress().toString() % u':' % QString::number(m_pWebSocketServer->serverPort());
		#endif
		emit wsServerStatusChanged(true);
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
	}
	else {
		qDebug() << "Error starting websocket server: "_L1 << m_pWebSocketServer->errorString();
		emit wsServerStatusChanged(false);
	}
}
