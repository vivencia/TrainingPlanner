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

const QString RunCommands::getExercisesListVersion()
{
	QFile exercisesListFile( ":/extras/exerciseslist.lst" );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[20] = { 0 };
		qint64 lineLength;
		QString line;
		lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
		if (lineLength < 0) return 0;
		line = buf;
		if (line.startsWith(QStringLiteral("#Vers"))) {
			exercisesListFile.close();
			return line.split(';').at(1).trimmed();
		}
		exercisesListFile.close();
	}
	return "0";
}

QStringList RunCommands::getExercisesList()
{
	QStringList exercisesList;
	QFile exercisesListFile( ":/extras/exerciseslist.lst" );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[1024];
		qint64 lineLength;
		do
		{
			lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
			if (lineLength < 0) continue;
			exercisesList.append(buf);
		} while (!exercisesListFile.atEnd());
		exercisesListFile.close();
	}
	return exercisesList;
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
		const int idx (dbFile.indexOf("Planner"));
		if (idx > 1)
			m_appPrivateDir = dbFile.left(dbFile.indexOf('/', idx + 1) + 1);
	}
	return m_appPrivateDir;
}
