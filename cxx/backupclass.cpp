#include "backupclass.h"
#include <QFileInfo>
#include <QDate>
#include <QFile>
#include <QDir>

static const QString backupsListFileName( QStringLiteral( "tp_backups.bk" ) );

BackupClass::BackupClass( const QString &dbFilePath )
	: QObject ( nullptr )
{
	QDir backupsDir( dbFilePath + QStringLiteral("/Databases/") );
	QFileInfoList backupFilesList ( backupsDir.entryInfoList( QStringList() << QStringLiteral("*.sqlite") ) );
	if ( !backupFilesList.isEmpty() )
		mDBFileName = backupFilesList.at( 0 ).fileName();
	mDBFile = backupFilesList.at( 0 ).filePath();
	backupsDir.cdUp();
	mSavePath = backupsDir.path() + '/';
	connect(this, &BackupClass::backupOperationResult, this, &BackupClass::addBackupFileToList);
}

bool BackupClass::doBackUp( const QString& fileName )
{
	QString strBackupFileName;
	if (!fileName.isEmpty())
		strBackupFileName = fileName;
	else
		strBackupFileName = mSavePath + mDBFileName + '_' + QDate::currentDate().toString("yyyyMMdd");

	QFile outFile( mDBFile );
	bool ret ( outFile.copy( strBackupFileName ) );
	if (ret)
	{
		QFile backupsList( mSavePath + backupsListFileName );
		bool bAddFile = true;
		if ( backupsList.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
		{
			char buf[1024];
			qint64 lineLength;
			QString line;
			do
			{
				lineLength = backupsList.readLine( buf, sizeof(buf) );
				if (lineLength < 0) break;
				line = buf;
				if (line == strBackupFileName) {
					bAddFile = true;
					break;
				}
			} while (true);
			backupsList.close();
		}
		if ( bAddFile ) {
			if ( backupsList.open( QIODeviceBase::Append|QIODeviceBase::Text ) ) {
				ret = backupsList.write( strBackupFileName.toLatin1() ) > 0;
				backupsList.close();
				if ( ret )
					emit backupsListChanged();
			}
		}
		else
			ret = false;
	}
	emit backupOperationResult( ret, outFile.fileName() );
	return ret;
}

bool BackupClass::doRestore( const QString &fileName )
{
	QFileInfo fInfo( fileName );
	bool ret ( fInfo.isReadable() );
	if ( ret )
	{
		QFile outFile( mDBFile );
		ret = outFile.remove();
		if ( ret ) {
			ret = outFile.copy( fileName );
		}
	}
	emit restoreOperationResult( ret );
	return ret;
}

QStringList BackupClass::getBackupsList()
{
	if (!mBackupFiles.isEmpty())
		return mBackupFiles;

	QFile backupsList( mSavePath + backupsListFileName );
	if ( backupsList.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[1024];
		qint64 lineLength;
		do
		{
			lineLength = backupsList.readLine( buf, sizeof(buf) );
			if (lineLength < 0) break;
			mBackupFiles.append( QString( buf ) );
		} while( !backupsList.atEnd() );
	}
	return mBackupFiles;
}

void BackupClass::checkIfDBFileIsMissing()
{
	return;
	QFileInfo fInfo( mDBFile );
	//First time ever the program is running or, most likely, some QGuiApplication setting was changed
	//like version or organization. If the name was changed, then this will not work
	if ( !fInfo.exists() )
	{
		//Try to restore
		QDir dir(mSavePath);
		QStringList backupFilesList ( dir.entryList( QDir::Files, QDir::Time ) );
		if ( !backupFilesList.isEmpty() )
		{
			doRestore(backupFilesList.at (0)); //Most recent
		}
	}
}

void BackupClass::addBackupFileToList( bool result, const QString& backupFilename )
{
	if ( result )
	{
		if ( mBackupFiles.isEmpty() )
			getBackupsList();
		mBackupFiles.append(backupFilename);
	}
}
