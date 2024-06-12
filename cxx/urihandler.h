#ifndef URIHANDLER_H
#define URIHANDLER_H

#include <qglobal.h>

#ifdef Q_OS_ANDROID

#include <QObject>
#include <QFileInfo>

class DbManager;

class URIHandler : public QObject
{

Q_OBJECT

public:
	explicit URIHandler(DbManager* appDB, QObject* parent = nullptr);

	void setFileUrlReceived(const QString &url) const;
	void setFileReceivedAndSaved(const QString& url) const;
	bool checkFileExists(const QString& url) const;

signals:
	void openURLRequest(const QString& url);
	void fileReceivedAndSaved(const QString& url);

private:
	DbManager* m_appDB;
	static URIHandler* s_instance;

	friend URIHandler* handlerInstance();
};

inline URIHandler* handlerInstance() { return URIHandler::s_instance; }
#endif

#endif // URIHANDLER_H
