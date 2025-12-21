#include "tcpserver.h"

#include "tponlineservices.h"

#include <QtWebSockets>

using namespace Qt::Literals::StringLiterals;

static inline QString getIdentifier(QWebSocket *peer)
{
	return peer->peerAddress().toString() % ':' % QString::number(peer->peerPort());
}

ChatServer::ChatServer(const QString &id, const QString &address, quint16 port, QObject *parent) :
	QObject{parent}, m_id{id},
	m_pWebSocketServer{new QWebSocketServer{id, QWebSocketServer::NonSecureMode, this}}
{
	connect(m_pWebSocketServer, &QWebSocketServer::originAuthenticationRequired, this, [](QWebSocketCorsAuthenticator *authenticator) {
		qDebug() << "Origin authentication requested. Origin:" << authenticator->origin();  // This will log the incoming Origin (or empty if missing)
		authenticator->setAllowed(true);  // Accept any origin
	});
	connect(m_pWebSocketServer, &QWebSocketServer::serverError, this, [](QWebSocketProtocol::CloseCode code) {
		qDebug() << "Server error:" << code;
	});
	if (m_pWebSocketServer->listen(QHostAddress{address}, port))
	{
		qDebug() << "--------------  Chat Server listening on : " <<
						m_pWebSocketServer->serverAddress().toString() + ':' + QString::number(m_pWebSocketServer->serverPort());
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &ChatServer::onNewConnection);
	}
	else
		 qDebug() << "Error starting server:" << m_pWebSocketServer->errorString();
	appOnlineServices()->webSocketsClientRegistration(id.toInt(), id, address, QString::number(port));
}

ChatServer::~ChatServer()
{

	m_pWebSocketServer->close();
}
//! [constructor]

//! [onNewConnection]
void ChatServer::onNewConnection()
{
	auto pSocket = m_pWebSocketServer->nextPendingConnection();
	if (pSocket)
	{
		qDebug() << "--------------  " << getIdentifier(pSocket) << " connected!";
		pSocket->setParent(this);
		connect(pSocket, &QWebSocket::textMessageReceived, this, &ChatServer::processMessage);
		connect(pSocket, &QWebSocket::disconnected, this, &ChatServer::socketDisconnected);
		m_clients << pSocket;
		 // Optional: Send welcome message
		pSocket->sendTextMessage("Connected to "_L1 + m_id);
	}
}
//! [onNewConnection]

//! [processMessage]
void ChatServer::processMessage(const QString &message)
{
	qDebug() << "--------------  Received: " << message;
	//QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());

}
//! [processMessage]

//! [socketDisconnected]
void ChatServer::socketDisconnected()
{
	QWebSocket *socket{qobject_cast<QWebSocket*>(sender())};
	if (socket)
	{
		qDebug() << "--------------  " << getIdentifier(socket) << " disconnected!";
		m_clients.removeAll(socket);
		socket->deleteLater();
	}
}
