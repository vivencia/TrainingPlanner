#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QQuickImageProvider>

class RunCommands : public QObject
{

Q_OBJECT
Q_PROPERTY(QString dbFileName READ getDBFileName NOTIFY dbFileNameChanged)
Q_PROPERTY(QString appPrivateDir READ getAppPrivateDir NOTIFY appPrivateDirChanged)

public:
	explicit RunCommands( QObject *parent = nullptr ) : QObject(parent) {}
	Q_INVOKABLE const QString getCorrectPath( const QUrl& url );
	Q_INVOKABLE int getFileType( const QString& filename );
	const QString getExercisesListVersion();
	Q_INVOKABLE QStringList getExercisesList();
	Q_INVOKABLE QString searchForDatabaseFile( const QString& baseDir );
	QString getAppDir(const QString& dbFile);

	inline QString getDBFileName() const { return m_dbFileName; }
	inline QString getAppPrivateDir() const { return m_appPrivateDir; }

signals:
	void dbFileNameChanged();
	void appPrivateDirChanged();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
};

#endif // RUNCOMMANDS_H
