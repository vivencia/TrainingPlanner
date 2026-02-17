#pragma once

#include <QObject>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class ChatWSServer : public QObject
{

Q_OBJECT

public:

	enum WS_USES {
		WS_TPCHAT,
		WS_TPMESSAGESMANAGER,
		WS_TOTALUSES
	};

	explicit ChatWSServer(const QString &id, QObject *parent = nullptr);
	~ChatWSServer() override;
	void setServerStatus(const bool enabled);

	void connectToPeer(QObject *local_peer, const WS_USES use, const QString &userid, int n_attempts = 5);
	bool isConnectionOK(const QString &userid) const;
	inline const QString &port() const { return m_port; }
	inline bool hasPeers() const { return !m_peersSockets.isEmpty(); }

	bool sendTextMessage(const WS_USES use, const QString &sender_id, const QString &receiver_id, const QString &message);
	bool sendBinaryMessage(const WS_USES use, const QString &sender_id, const QString &receiver_id, QByteArray &&data);

signals:
	void wsServerStatusChanged(const bool enabled);
	void gotPeerAddress(const int request_id, const QString &address);
	void textMessageReceived(const uint use, const QString &userid, const QString &message);
	void binaryMessageReceived(const uint use, const QString &userid, const QByteArray &data);

private slots:
	void wsTextMessageReceived(QString message);
	void wsBinaryMessageReceived(QByteArray data);
	void onNewConnection();

private:
	QWebSocketServer *m_pWebSocketServer;
	QHash<QString,QWebSocket*> m_peersSockets;
	QHash<QString,QList<QObject*>> m_localPeers;
	QString m_id, m_port;

	void queryPeerAddress(const int requestid, const QString &userid);
	void setupWSServer();

	static ChatWSServer *app_ws_server;
	friend ChatWSServer *appWSServer();
};

inline ChatWSServer *appWSServer() { return ChatWSServer::app_ws_server; }
