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

	void sendMessageToInterlocutor(const QString &id, const QString &message) const;
	void connectToPeer(const QString &id, const QString &address);
	inline const QString &port() const { return m_port; }

signals:
	void connectionEstabilished(QWebSocket *peer);
	void lostConnection(QWebSocket *peer);

private slots:
	void onNewConnection();
	void processMessage(const QString &message);
	void socketDisconnected();

private:
	QWebSocketServer *m_pWebSocketServer;
	QHash<QString, QWebSocket*> m_interlocutorSockets;
	QString m_id, m_port;

	void connectPeer(QWebSocket *peer);
};
