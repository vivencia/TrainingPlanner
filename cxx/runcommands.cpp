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

void RunCommands::getExercisesListVersion()
{
	m_exercisesListVersion = QStringLiteral("0");
	QFile exercisesListFile( QStringLiteral(":/extras/exerciseslist.lst") );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[20] = { 0 };
		qint64 lineLength;
		QString line;
		lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
		if (lineLength < 0) return 0;
		line = buf;
		if (line.startsWith(QStringLiteral("#Vers")))
			m_exercisesListVersion = line.split(';').at(1).trimmed();
		exercisesListFile.close();
	}
}

QStringList RunCommands::getExercisesList()
{
	QStringList exercisesList;
	QFile exercisesListFile( QStringLiteral(":/extras/exerciseslist.lst") );
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
		const int idx (dbFile.indexOf(QStringLiteral("Planner")));
		if (idx > 1)
			m_appPrivateDir = dbFile.left(dbFile.indexOf('/', idx + 1) + 1);
	}
	return m_appPrivateDir;
}
