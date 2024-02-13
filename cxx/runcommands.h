#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QDate>

class QSettings;

class RunCommands : public QObject
{

Q_OBJECT
Q_PROPERTY(QString dbFileName READ getDBFileName NOTIFY dbFileNameChanged)
Q_PROPERTY(QString appPrivateDir READ getAppPrivateDir NOTIFY appPrivateDirChanged)

public:
	explicit RunCommands( QSettings* settings, QObject *parent = nullptr ) : QObject(parent), m_appSettings(settings) {}
	Q_INVOKABLE const QString getCorrectPath( const QUrl& url );
	Q_INVOKABLE int getFileType( const QString& filename );
	Q_INVOKABLE QString searchForDatabaseFile( const QString& baseDir );
	QString getAppDir(const QString& dbFile);

	inline QString getDBFileName() const { return m_dbFileName; }
	inline QString getAppPrivateDir() const { return m_appPrivateDir; }

	Q_INVOKABLE const QString formatDate(const QDate& date) const;
	Q_INVOKABLE inline uint calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const
	{
		return calculateNumberOfWeeks(date1.weekNumber(), date2.weekNumber());
	}
	Q_INVOKABLE uint calculateNumberOfWeeks(const uint week1, const uint week2) const;
	Q_INVOKABLE QDate getMesoStartDate(const QDate& lastMesoEndDate) const;
	Q_INVOKABLE QDate createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const;

signals:
	void dbFileNameChanged();
	void appPrivateDirChanged();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
	QSettings* m_appSettings;
};

#endif // RUNCOMMANDS_H
