#include "backupclass.h"

#include <QFileInfo>
#include <QDate>
#include <QFile>
#include <QDir>

static const QString backupsListFileName( QStringLiteral( "tp_backups.bk" ) );

BackupClass::BackupClass( const QString &dbFileName , const QString &privateAppDir )
	: QObject (nullptr), mDBFile (dbFileName), mb_CanDoBackup (false)
{

	QDir backupsDir( privateAppDir );
	QFileInfoList backupFilesList ( backupsDir.entryInfoList(QStringList(), QDir::Dirs|QDir::Executable|QDir::NoDotDot|QDir::Writable) );
	if (!backupFilesList.isEmpty()) //can write to directory
	{
		mSavePath = privateAppDir + QStringLiteral("/DBBackups/");
		backupsDir.mkdir(QStringLiteral("DBBackups"));
		connect(this, &BackupClass::backupOperationResult, this, &BackupClass::addBackupFileToList);
		mb_CanDoBackup = true;
	}
}

bool BackupClass::doBackUp( const QString& fileName )
{
	if (!mb_CanDoBackup)
		return false;

	QString strBackupFileName;
	if (!fileName.isEmpty())
		strBackupFileName = fileName;
	else
		strBackupFileName = mSavePath + mDBFile + '_' + QDate::currentDate().toString("yyyyMMdd");

	QFile outFile( mDBFile );
	bool ret ( outFile.copy( strBackupFileName ) );
	if (ret)
	{
		QFile backupsListFile( mSavePath + backupsListFileName );
		bool bAddFile = true;
		if ( backupsListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
		{
			char buf[1024];
			qint64 lineLength;
			QString line;
			do
			{
				lineLength = backupsListFile.readLine( buf, sizeof(buf) );
				if (lineLength < 0) break;
				line = buf;
				if (line == strBackupFileName) {
					bAddFile = true;
					break;
				}
			} while (true);
			backupsListFile.close();
		}
		if ( bAddFile ) {
			if ( backupsListFile.open( QIODeviceBase::Append|QIODeviceBase::Text ) ) {
				ret = backupsListFile.write( strBackupFileName.toLatin1() ) > 0;
				backupsListFile.close();
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
	if (!mb_CanDoBackup)
		return false;

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
	if (!m_backupsList.isEmpty())
		return m_backupsList;

	QFile backupsList( mSavePath + backupsListFileName );
	if ( backupsList.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[1024];
		qint64 lineLength;
		do
		{
			lineLength = backupsList.readLine( buf, sizeof(buf) );
			if (lineLength < 0) break;
			m_backupsList.append( QString( buf ) );
		} while( !backupsList.atEnd() );
	}
	return m_backupsList;
}

void BackupClass::checkIfDBFileIsMissing()
{
	return;
	QFileInfo fInfo( mDBFile );
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
		if ( m_backupsList.isEmpty() )
			getBackupsList();
		m_backupsList.append(backupFilename);
	}
}
