#pragma once

#include <QObject>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class ChatWSServer : public QObject
{

Q_OBJECT

public:
	explicit ChatWSServer(const QString &id, QObject *parent = nullptr);
	~ChatWSServer() override;
	void setServerStatus(const bool enabled);

	void connectToPeer(const QString &userid);
	inline const QString &port() const { return m_port; }
	inline bool hasPeers() const { return !m_peersSockets.isEmpty(); }
	inline QWebSocket *peerSocket(const QString &id) const { return m_peersSockets.value(id); }

signals:
	void wsServerStatusChanged(const bool enabled);
	void gotPeerAddress(const int request_id, const QString &address);
	void wsConnectionToClientPeerConcluded(const bool success, const QString &userid, QWebSocket *peer);

private slots:
	void onNewConnection();

private:
	QWebSocketServer *m_pWebSocketServer;
	QHash<QString,QWebSocket*> m_peersSockets;
	QString m_id, m_port;

	void queryPeerAddress(const int requestid, const QString &userid);
	void setupWSServer();

	static ChatWSServer *app_ws_server;
	friend ChatWSServer *appWSServer();
};

inline ChatWSServer *appWSServer() { return ChatWSServer::app_ws_server; }
