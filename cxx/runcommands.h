#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QQuickImageProvider>

class RunCommands : public QObject
{

Q_OBJECT
QML_ELEMENT

public:
	explicit RunCommands( QObject *parent = nullptr ) : QObject(parent) {}
	Q_INVOKABLE const QString getCorrectPath( const QUrl& url );
	Q_INVOKABLE int getFileType( const QString& filename );
	float getExercisesListVersion();
	bool getExercisesList(QStringList& exercisesList);
	QString searchForDatabaseFile( const QString& baseDir );
	QString getAppDir(const QString& dbFile);
};

#endif // RUNCOMMANDS_H
