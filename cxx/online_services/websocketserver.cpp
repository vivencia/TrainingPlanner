#include "websocketserver.h"

#include "tpmessagesmanager.h"
#include "tponlineservices.h"

#include <QtWebSockets>

using namespace Qt::Literals::StringLiterals;

static inline QString getIdentifier(QWebSocket *peer)
{
	return peer->peerAddress().toString() % ':' % QString::number(peer->peerPort());
}

ChatWSServer::ChatWSServer(const QString &id, const QString &address, QObject *parent)
	: QObject{parent}, m_id{id}, m_pWebSocketServer{new QWebSocketServer{id, QWebSocketServer::NonSecureMode, this}}
{
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

void ChatWSServer::sendMessageToInterlocutor(const QString &id, const QString &message) const
{
	QWebSocket* socket{m_interlocutorSockets.value(id)};
	if (socket)
		socket->sendTextMessage(message);
}

void ChatWSServer::connectToPeer(const QString &id, const QString &address)
{
	QWebSocket *this_client{new QWebSocket(m_id, QWebSocketProtocol::Version13, this)};
	connect(this_client, &QWebSocket::connected, [this,id] () {
		qDebug() << "WebSocket connected to " << id;
		connectPeer(qobject_cast<QWebSocket*>(sender()));
	});

	#ifndef QT_NO_DEBUG
	// Handle errors (e.g., server not found, connection refused)
	QObject::connect(this_client, &QWebSocket::errorOccurred, [&](QAbstractSocket::SocketError error) {
		qDebug() << "WebSocket error: " << error << " " << this_client->errorString();
	});
	#endif
	this_client->open(QUrl{"ws://"_L1 % address % ':' % id.last(5)});
}

void ChatWSServer::connectPeer(QWebSocket *peer)
{
	connect(peer, &QWebSocket::textMessageReceived, this, &ChatWSServer::processMessage);
	connect(peer, &QWebSocket::disconnected, this, &ChatWSServer::socketDisconnected);
	QString id{std::move(peer->origin())};
	if (id.isEmpty())
		id = std::move("TPServer");
	m_interlocutorSockets.insert(id, peer);
	emit connectionEstabilished(peer);
}

void ChatWSServer::onNewConnection()
{
	auto p_socket{m_pWebSocketServer->nextPendingConnection()};
	if (p_socket)
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  " << getIdentifier(p_socket) << " connected!";
		#endif
		p_socket->setParent(this);
		connectPeer(p_socket);
	}
}

void ChatWSServer::processMessage(const QString &message)
{
	QWebSocket *socket{qobject_cast<QWebSocket*>(sender())};
	if (socket)
		appMessagesManager()->processWebSocketMessage(socket->origin(), message);
}

void ChatWSServer::socketDisconnected()
{
	QWebSocket *socket{qobject_cast<QWebSocket*>(sender())};
	if (socket)
	{
		emit lostConnection(socket);
		#ifndef QT_NO_DEBUG
		qDebug() << "--------------  " << getIdentifier(socket) << " disconnected!";
		#endif
		socket->deleteLater();
	}
}
