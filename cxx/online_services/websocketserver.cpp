#include "websocketserver.h"

#include "tpchat.h"
#include "tpmessagesmanager.h"
#include "tponlineservices.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtWebSockets>

ChatWSServer *ChatWSServer::app_ws_server{nullptr};

using namespace QLiterals;

enum messageUseSortingFields {
	MUSF_USE_TYPE,
	MUSF_LOCAL_USERID,
	MUSF_REMOTE_USERID,
};

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
	if (!enabled && m_pWebSocketServer->isListening())
	{
		m_pWebSocketServer->close();
		emit wsServerStatusChanged(enabled);
	}
	else if (enabled && !m_pWebSocketServer->isListening())
		setupWSServer();
}

void ChatWSServer::connectToPeer(QObject *local_peer, const WS_USES use, const QString &userid, int n_attempts)
{
	if (isConnectionOK(userid))
	{
		if (!m_localPeers.value(userid).contains(local_peer))
			m_localPeers.value(userid).append(local_peer);
		return;
	}
	const QLatin1String seed{"connectToPeer" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &ChatWSServer::gotPeerAddress, [=,this,&n_attempts] (const int request_id, const QString &address)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (address.contains("not logged"_L1))
				qDebug() << "****** WebSocket error: peer not logged: " << address;
			else
			{
				QWebSocket *peer{new QWebSocket(m_id, QWebSocketProtocol::Version13, this)};
				connect(peer, &QWebSocket::connected, [=,this] () {
					qDebug() << "****** WebSocket connected to " << userid;
					connect(peer, &QWebSocket::textMessageReceived, [this] (const QString &message) { wsTextMessageReceived(message); });
					connect(peer, &QWebSocket::binaryMessageReceived, [this] (const QByteArray &data) { wsBinaryMessageReceived(data); });
					connect(peer, &QWebSocket::disconnected, this, [this,peer,userid] () {
						peer->disconnect();
						peer->deleteLater();
						m_peersSockets.remove(userid);
						m_localPeers.remove(userid);
					});
					m_peersSockets.insert(userid, peer);
					QList<QObject*> local_peers{WS_TOTALUSES, nullptr};
					local_peers[use] = local_peer;
					m_localPeers.insert(userid, local_peers);
				});
				// Handle errors (e.g., server not found, connection refused)
				QObject::connect(peer, &QWebSocket::errorOccurred, [=,this,&n_attempts] (QAbstractSocket::SocketError error)
				{
					auto err_func = [this,peer,userid,error] () -> void {
						qDebug() << "****** WebSocket error: " << error << " " << peer->errorString() << " " << peer->peerAddress();
					};
					switch (error)
					{
						case QAbstractSocket::ConnectionRefusedError:
						case QAbstractSocket::RemoteHostClosedError:
							peer->close();
							if (--n_attempts > 0)
								connectToPeer(local_peer, use, userid, n_attempts);
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

bool ChatWSServer::sendTextMessage(const WS_USES use, const QString &sender_id, const QString &receiver_id, const QString &message)
{
	QWebSocket *peer{m_peersSockets.value(receiver_id)};
	if (peer && peer->isValid())
		return peer->sendTextMessage(appUtils()->string_strings({QString::number(use), sender_id, receiver_id, message},
																							comp_exercises_separator)) > message.length();
	return false;
}

bool ChatWSServer::sendBinaryMessage(const WS_USES use, const QString &sender_id, const QString &receiver_id, QByteArray &&data)
{
	QWebSocket *peer{m_peersSockets.value(sender_id)};
	if (peer && peer->isValid())
	{
		data.append(appUtils()->string_strings({QString::number(use), sender_id, receiver_id}, comp_exercises_separator).toLatin1());
		return peer->sendBinaryMessage(data) == data.size();
	}
	return false;
}

void ChatWSServer::wsTextMessageReceived(QString message)
{
	bool ok{false};
	const WS_USES use{static_cast<WS_USES>(appUtils()->getCompositeValue(MUSF_USE_TYPE, message, comp_exercises_separator).toUInt(&ok))};
	if (ok)
	{
		const QString &remote_user{appUtils()->getCompositeValue(MUSF_LOCAL_USERID, message, comp_exercises_separator)};
		const QList<QObject*> local_peers{m_localPeers.value(remote_user)};
		if (!local_peers.isEmpty())
		{
			QObject *local_peer{local_peers.at(use)};
			if (local_peer)
			{
				message.remove(0, (remote_user.length() * 2) + 4); // use = 1 char, separators = 3 chars(3 fields)
				switch (use)
				{
					case WS_TPCHAT:
						qobject_cast<TPChat*>(local_peer)->processWebSocketTextMessage(std::move(message));
					break;
					case WS_TPMESSAGESMANAGER:
						qobject_cast<TPMessagesManager*>(local_peer)->addMessage();
					break;
					default: return;
				}
			}
		}
	}
}

void ChatWSServer::wsBinaryMessageReceived(QByteArray data)
{
	bool ok{false};
	const WS_USES use{static_cast<WS_USES>(appUtils()->getCompositeValue(MUSF_USE_TYPE, data, comp_exercises_separator).toUInt(&ok))};
	if (ok)
	{
		const QString &remote_user{appUtils()->getCompositeValue(MUSF_LOCAL_USERID, data, comp_exercises_separator)};
		const QList<QObject*> local_peers{m_localPeers.value(remote_user)};
		if (!local_peers.isEmpty())
		{
			QObject *local_peer{local_peers.at(use)};
			if (local_peer)
			{
				QString filename{std::move(data.last(data.length() - data.lastIndexOf(record_separator.toLatin1())))};
				data.remove(0, (remote_user.length() * 2) + 4); // use = 1 char, separators = 3 chars(3 fields)
				data.chop(filename.length());
				filename.removeFirst();
				switch (use)
				{
					case WS_TPCHAT:
						qobject_cast<TPChat*>(local_peer)->processWebSocketBinaryMessage(filename, std::move(data));
					break;
					case WS_TPMESSAGESMANAGER:
						qobject_cast<TPMessagesManager*>(local_peer)->addMessage();
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
	if (p_socket)
	{
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
			if (!(*itr)->isValid())
			{
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
														(const int request_id, const int ret_code, const QString &ret_string)
	{
		if (request_id == requestid)
		{
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
	if (m_pWebSocketServer->listen(QHostAddress{appSettings()->serverAddress()}, m_port.toUShort()))
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  WebSocket Chat Server listening on : " <<
						m_pWebSocketServer->serverAddress().toString() + ':' + QString::number(m_pWebSocketServer->serverPort());
		#endif
		emit wsServerStatusChanged(true);
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &ChatWSServer::onNewConnection);
	}
	else
	{
		qDebug() << "Error starting websocket server:" << m_pWebSocketServer->errorString();
		emit wsServerStatusChanged(false);
	}
}
