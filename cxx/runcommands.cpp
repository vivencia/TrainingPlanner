#include "runcommands.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>

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
	#ifdef Q_OS_ANDROID
		if ( filename.contains(QStringLiteral("video%"), Qt::CaseInsensitive))
			return 1;
		else if ( filename.contains(QStringLiteral("image%"), Qt::CaseInsensitive))
			return 0;
		else return -1;
	#else
		if ( filename.endsWith(QStringLiteral(".mp4"), Qt::CaseInsensitive) ||
			 filename.endsWith(QStringLiteral(".mkv"), Qt::CaseInsensitive) ||
			 filename.endsWith(QStringLiteral(".mov"), Qt::CaseInsensitive) )
			return 1;
		else if ( filename.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
				 filename.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) )
			return 0;
		else
			return -1;
	#endif
}

QString RunCommands::searchForDatabaseFile( const QString& baseDir)
{
	QDir root (baseDir);
	root.setFilter(QDir::AllEntries);
	QFileInfoList list = root.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		if (list.at(i).fileName() == "." || list.at(i).fileName() == "..")
			continue;
		if (list.at(i).isDir())
		{
			return searchForDatabaseFile(list.at(i).filePath());
		}
		else
		{
			if (list.at(i).fileName().endsWith(QStringLiteral(".sqlite")))
			{
				m_dbFileName = list.at(i).filePath();
				break;
			}
		}
	}
	return m_dbFileName;
}

QString RunCommands::getAppDir(const QString& dbFile)
{
	if (!dbFile.isEmpty())
	{
		const int idx (dbFile.indexOf(QStringLiteral("Planner")));
		if (idx > 1)
			m_appPrivateDir = dbFile.left(dbFile.indexOf('/', idx + 1) + 1);
	}
	return m_appPrivateDir;
}
