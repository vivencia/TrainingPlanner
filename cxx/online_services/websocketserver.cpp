#include "websocketserver.h"

#include "tponlineservices.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtWebSockets>

ChatWSServer *ChatWSServer::app_ws_server{nullptr};

using namespace Qt::Literals::StringLiterals;

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

void ChatWSServer::connectToPeer(const QString &userid, int n_attempts)
{
	const QLatin1String seed{"connectToPeer" % userid.toLatin1()};
	const int requestid{appUtils()->generateUniqueId(seed)};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &ChatWSServer::gotPeerAddress, [this,conn,requestid,userid,&n_attempts] (const int request_id, const QString &address)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (address.contains("not logged"_L1))
			{
				qDebug() << "****** WebSocket error: " << address;
				emit wsConnectionToClientPeerConcluded(false, userid, nullptr);
			}
			else
			{
				QWebSocket *peer{new QWebSocket(m_id, QWebSocketProtocol::Version13, this)};
				connect(peer, &QWebSocket::connected, [=,this] () {
					qDebug() << "****** WebSocket connected to " << userid;
					emit wsConnectionToClientPeerConcluded(true, userid, peer);
				});
				// Handle errors (e.g., server not found, connection refused)
				QObject::connect(peer, &QWebSocket::errorOccurred, [=,this,&n_attempts] (QAbstractSocket::SocketError error)
				{
					auto err_func = [this,peer,userid,error] () -> void {
						qDebug() << "****** WebSocket error: " << error << " " << peer->errorString() << " " << peer->peerAddress();
						emit wsConnectionToClientPeerConcluded(false, userid, nullptr);
					};
					switch (error)
					{
						case QAbstractSocket::ConnectionRefusedError:
						case QAbstractSocket::RemoteHostClosedError:
							peer->close();
							if (--n_attempts > 0)
								connectToPeer(userid, n_attempts);
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
		emit wsConnectionToClientPeerConcluded(true, id, p_socket);

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
