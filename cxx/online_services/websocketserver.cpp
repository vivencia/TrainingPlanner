#include "websocketserver.h"

#include "tponlineservices.h"
#include "../tputils.h"

#include <QtWebSockets>

ChatWSServer *ChatWSServer::app_ws_server{nullptr};

using namespace Qt::Literals::StringLiterals;

static inline QString getIdentifier(QWebSocket *peer)
{
	return peer->peerAddress().toString() % ':' % QString::number(peer->peerPort());
}

ChatWSServer::ChatWSServer(const QString &id, const QString &address, QObject *parent)
	: QObject{parent}, m_id{id}, m_tpServerSocket{nullptr},
							m_pWebSocketServer{new QWebSocketServer{id, QWebSocketServer::NonSecureMode, this}}
{
	if (app_ws_server)
		return;

	app_ws_server = this;
	connect(m_pWebSocketServer, &QWebSocketServer::serverError, this, [](QWebSocketProtocol::CloseCode code) {
		qDebug() << "WebSocket Server error:" << code;
	});
	m_port = std::move(id.last(5));
	if (m_pWebSocketServer->listen(QHostAddress{address}, m_port.toUShort()))
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  Chat Server listening on : " <<
						m_pWebSocketServer->serverAddress().toString() + ':' + QString::number(m_pWebSocketServer->serverPort());
		#endif
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &ChatWSServer::onNewConnection);
	}
	else
		 qDebug() << "Error starting server:" << m_pWebSocketServer->errorString();
	appOnlineServices()->webSocketsClientRegistration(m_id, m_port);
}

ChatWSServer::~ChatWSServer()
{
	m_pWebSocketServer->close();
}

int ChatWSServer::queryPeerAddress(const QString &userid)
{
	if (m_tpServerSocket)
	{
		const QLatin1String seed{"queryPeerAddress" % userid.toLatin1()};
		const int request_id{appUtils()->generateUniqueId(seed)};
		m_tpServerSocket->sendTextMessage(appUtils()->string_strings({
											QString::number(request_id), "peer_address"_L1, userid}, set_separator));
		return request_id;
	}
	return -1;
}

void ChatWSServer::connectToPeer(const QString &id, const QString &address)
{
	QWebSocket *this_client{new QWebSocket(m_id, QWebSocketProtocol::Version13, this)};
	connect(this_client, &QWebSocket::connected, [=,this] () {
		qDebug() << "WebSocket connected to " << id;
		emit wsConnectionToClientPeerConcluded(true, id, qobject_cast<QWebSocket*>(sender()));
	});

	#ifndef QT_NO_DEBUG
	// Handle errors (e.g., server not found, connection refused)
	QObject::connect(this_client, &QWebSocket::errorOccurred, [=,this] (QAbstractSocket::SocketError error) {
		qDebug() << "WebSocket error: " << error << " " << this_client->errorString();
		emit wsConnectionToClientPeerConcluded(false, id, nullptr);
	});
	#endif
	this_client->open(QUrl{"ws://"_L1 % address % ':' % id.last(5)});
}

void ChatWSServer::onNewConnection()
{
	auto p_socket{m_pWebSocketServer->nextPendingConnection()};
	if (p_socket)
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  " << getIdentifier(p_socket) << " connected!";
		#endif
		if (p_socket->origin().isEmpty()) //TPServer connection
		{
			p_socket->setParent(this);
			connect(p_socket, &QWebSocket::textMessageReceived, this, &ChatWSServer::processMessage);
			connect(p_socket, &QWebSocket::disconnected, this, &ChatWSServer::socketDisconnected);
			m_tpServerSocket = p_socket;
		}
		else //result of connectToPeer() from another user. Now we are not the server in the connection. They are.
		{
			const QString &id{p_socket->origin()};
			m_peersSockets.insert(id, p_socket);
			emit wsConnectionToServerPeerConcluded(true, id, p_socket);

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
}

void ChatWSServer::processMessage(const QString &message)
{
	QWebSocket *socket{qobject_cast<QWebSocket*>(sender())};
	if (socket == m_tpServerSocket)
	{
		bool ok{false};
		const int requestid{appUtils()->getCompositeValue(0, message, record_separator).toInt(&ok)};
		if (ok)
		{
			const QString &reply{appUtils()->getCompositeValue(1, message, record_separator)};
			emit tpServerReply(requestid, reply);
		}
	}
}

void ChatWSServer::socketDisconnected()
{
	QWebSocket *socket{qobject_cast<QWebSocket*>(sender())};
	if (socket == m_tpServerSocket)
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  " << getIdentifier(socket) << " disconnected!";
		#endif
		m_tpServerSocket->deleteLater();
		m_tpServerSocket = nullptr;
	}
}
