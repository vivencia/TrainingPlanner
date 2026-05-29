#pragma once

#include "../tpfilepath.h"

#include <QObject>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(TPMessagesManager)
QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class ChatWSServer : public QObject
{

Q_OBJECT

public:
	explicit ChatWSServer(const QString &id, QObject *parent = nullptr);
	~ChatWSServer() override;
	void setServerStatus(const bool enabled);

	void connectToPeer(QObject *local_peer, const int handle, const QString &userid, int n_attempts = 5);
	bool isConnectionOK(const QString &userid) const;
	inline const QString &port() const { return m_port; }
	inline bool hasPeers() const { return !m_peersSockets.isEmpty(); }

	//A binary message will make use TPUtils::BINARY_FILE_INFO_FIELDS, but also a text message, i.e. the final transmitted message
	//will contain a data string made of the same enum fields to carry all the information it needs
	bool sendTextMessage(const int handle, const QString &sender_id, const QString &receiver_id, const QString &message,
																				const TPFilePath &filename = TPFilePath{});
	bool sendBinaryMessage(const int handle, const TPFilePath &filename, const QString &extra_info = QString{},
																					const bool remove_local_file = false);

signals:
	void connectionAttemptResult(const bool established, const QString &userid);
	void wsServerStatusChanged(const bool enabled);
	void gotPeerAddress(const int request_id, const QString &address);
	void textMessageReceived(const uint use, const QString &userid, const QString &message);
	void binaryMessageReceived(const uint use, const QString &userid, const QByteArray &data);
	void fileReceived(const QByteArray &data);

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
