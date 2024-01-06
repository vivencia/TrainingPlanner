#include "runcommands.h"

#include <QImageReader>
#include <QFile>
RunCommands::~RunCommands()
{
	this->QObject::~QObject();
}

const QString RunCommands::getCorrectPath(const QUrl& url)
{
	qDebug() << "input url:  " << url;
	qDebug() << "output string:  " << url.toString(QUrl::PreferLocalFile);
	#ifdef Q_OS_ANDROID
	return url.toString(QUrl::PreferLocalFile);
	#else
	return url.toString();
	#endif
}

int RunCommands::getFileType( const QString& filename )
{
	qDebug() << "getFileType:   " << filename;
	#ifdef Q_OS_ANDROID
		if ( filename.contains("video%", Qt::CaseInsensitive))
			return 1;
		else if ( filename.contains("image%", Qt::CaseInsensitive))
			return 0;
		else return -1;
	#else
		if ( filename.endsWith(".mp4", Qt::CaseInsensitive) ||
			 filename.endsWith(".mkv", Qt::CaseInsensitive) ||
			 filename.endsWith(".mov", Qt::CaseInsensitive) )
			return 1;
		else if ( filename.endsWith(".png", Qt::CaseInsensitive) ||
				 filename.endsWith(".jpg", Qt::CaseInsensitive) )
			return 0;
		else
			return -1;
	#endif
}
