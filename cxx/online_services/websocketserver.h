#pragma once

#include <QObject>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class ChatWSServer : public QObject
{

Q_OBJECT

public:
	explicit ChatWSServer(const QString &id, const QString &address, QObject *parent = nullptr);
	~ChatWSServer() override;

	int queryPeerAddress(const QString &userid);
	void connectToPeer(const QString &id, const QString &address);
	inline const QString &port() const { return m_port; }
	inline bool hasPeers() const { return !m_peersSockets.isEmpty(); }
	inline QWebSocket *peerSocket(const QString &id) const { return m_peersSockets.value(id); }

signals:
	void tpServerReply(const int request_id, const QString &reply);
	void wsConnectionToClientPeerConcluded(const bool success, const QString &id, QWebSocket *peer);
	void wsConnectionToServerPeerConcluded(const bool success, const QString &id, QWebSocket *peer);

private slots:
	void onNewConnection();
	void processMessage(const QString &message);
	void socketDisconnected();

private:
	QWebSocketServer *m_pWebSocketServer;
	QWebSocket* m_tpServerSocket;
	QHash<QString,QWebSocket*> m_peersSockets;
	QString m_id, m_port;

	static ChatWSServer *app_ws_server;
	friend ChatWSServer *appWSServer();
};

inline ChatWSServer *appWSServer() { return ChatWSServer::app_ws_server; }
