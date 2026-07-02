#pragma once

#include "../tpfilepath.h"

#include <QObject>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(TPMessagesManager)
QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject
{

Q_OBJECT

public:
	explicit WSServer(const QString &id, QObject *parent = nullptr);
	~WSServer() override;
	void setServerStatus(const bool enabled);

	void connectToPeer(QObject *handler, const int handle, const QString &userid, int n_attempts = 5);
	bool isConnectionOK(const QString &userid, const bool try_to_connect = false) const;
	inline const QString &port() const { return m_port; }
	inline bool hasPeers() const { return !m_peersSockets.isEmpty(); }

	//A binary message will make use TPUtils::BINARY_FILE_INFO_FIELDS, but also a text message, i.e. the final transmitted message
	//will contain a data string made of the same enum fields to carry all the information it needs
	bool sendTextMessage(const QString &encoded_message);
	bool sendBinaryMessage(const TPFilePath &local_filename, const TPFilePath &target_filename,
																			const bool remove_local_file = false);

signals:
	void connectionAttemptResult(const bool established, const QString &userid);
	void _connectionAttemptResult(const bool established, const QString &userid);
	void wsServerStatusChanged(const bool enabled);
	void gotPeerAddress(const int request_id, const QString &address);
	void textMessageReceived(const uint use, const QString &userid, const QString &message);
	void binaryMessageReceived(const uint use, const QString &userid, const QByteArray &data);
	void fileReceived(const TPFilePath &tp_filepath, const bool success);

private slots:
	void wsTextMessageReceived(const QString &message);
	void wsBinaryMessageReceived(QByteArray data);
	void onNewConnection();

private:
	QWebSocketServer *m_pWebSocketServer;
	QHash<QString,QWebSocket*> m_peersSockets;
	QHash<QString,QList<QObject*>> m_handlers;
	QString m_id, m_port;

	void queryPeerAddress(const int requestid, const QString &userid);
	void setupWSServer();

	static WSServer *app_ws_server;
	friend WSServer *appWSServer();
};

inline WSServer *appWSServer() { return WSServer::app_ws_server; }
