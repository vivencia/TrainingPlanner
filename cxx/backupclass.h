#ifndef BACKUPCLASS_H
#define BACKUPCLASS_H

#include <QObject>
#include <QQmlEngine>
#include <QStringList>

class BackupClass : public QObject
{

Q_OBJECT
QML_ELEMENT
Q_PROPERTY( QStringList allBackupsList READ getBackupsList NOTIFY backupsListChanged )

public:
	explicit BackupClass( const QString &dbFilePath );
	Q_INVOKABLE bool doBackUp( const QString& fileName = QString() );
	Q_INVOKABLE bool doRestore( const QString& fileName );
	QStringList getBackupsList();
	void checkIfDBFileIsMissing();

signals:
	void backupOperationResult( bool result, const QString backupFileName );
	void restoreOperationResult( bool result );
	void backupsListChanged();

private:
	void addBackupFileToList( bool result, const QString& backupFilename );
	QString mDBFile;
	QString mSavePath;
	QString mDBFileName;
	QStringList mBackupFiles;
};

#endif // BACKUPCLASS_H
