#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(QString)

class ChatServer : public QObject
{

Q_OBJECT

public:
	explicit ChatServer(const QString &id, const QString &address, quint16 port, QObject *parent = nullptr);
	~ChatServer() override;

private slots:
	void onNewConnection();
	void processMessage(const QString &message);
	void socketDisconnected();

private:
	QWebSocketServer *m_pWebSocketServer;
	QList<QWebSocket *> m_clients;
	QString m_id;
};

#endif // TCPSERVER_H
