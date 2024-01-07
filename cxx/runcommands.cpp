#include "runcommands.h"

#include <QFileInfo>
#include <QFile>

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

bool RunCommands::updateExercisesList()
{
	QFileInfo fi("exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("qrc:/exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("qrc://exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("qrc:exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("/exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("./exerciseslist.lst");
	qDebug() << fi.exists();
	fi.setFile("qrc://images/black/time.png");
	qDebug() << fi.exists();

	fi.setFile(QUrl("exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc:/exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc://exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc:exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("/exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("./exerciseslist.lst").toString());
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc://images/black/time.png").toString());
	qDebug() << fi.exists();

	fi.setFile(QUrl("exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc:/exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc://exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc:exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("/exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("./exerciseslist.lst").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();
	fi.setFile(QUrl("qrc://images/black/time.png").toString(QUrl::PreferLocalFile));
	qDebug() << fi.exists();

	QFile exercisesList( "qrc://exerciseslist.lst" );
	if ( exercisesList.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[1024];
		qint64 lineLength;
		QString line;
		do
		{
			lineLength = exercisesList.readLine( buf, sizeof(buf) );
			if (lineLength < 0) break;
			line = buf;
			const QStringList exerciseInfo ( line.split(';') );
			for (int i = 0; i < exerciseInfo.length(); ++i)
				qDebug() << i << ":  " <<exerciseInfo.at(i);
		} while (true);
		exercisesList.close();
		return true;
	}
	return false;
}
