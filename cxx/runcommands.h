#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QDateTime>

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
	Q_INVOKABLE uint calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const
	{
		return calculateNumberOfWeeks(date1.weekNumber(), date2.weekNumber());
	}
	Q_INVOKABLE uint calculateNumberOfWeeks(const uint week1, const uint week2) const;
	Q_INVOKABLE QDate getMesoStartDate(const QDate& lastMesoEndDate) const;
	Q_INVOKABLE QDate createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const;
	Q_INVOKABLE QDate getDayBefore(const QDate& date) const;

	Q_INVOKABLE QString intTimeToStrTime(const uint time) const;
	Q_INVOKABLE QString getStrHourFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString getStrMinFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString formatTime(const QDateTime& time) const;
	Q_INVOKABLE QString addTimeToStrTime(const QString& strTime, const int addmins, const int addsecs) const;
	Q_INVOKABLE QString formatFutureTime(const QDateTime& time, const uint hours, const uint mins) const;
	Q_INVOKABLE QString formatFutureTime(const QDateTime& time, const QDateTime& addTime) const;
	Q_INVOKABLE QString getHourOrMinutesFromStrTime(const QString& strTime) const;
	Q_INVOKABLE QString getMinutesOrSeconsFromStrTime(const QString& strTime) const;
	Q_INVOKABLE QDateTime calculateTimeBetweenTimes(const QString& strTime1, const QString& strTime2) const;

signals:
	void dbFileNameChanged();
	void appPrivateDirChanged();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
	QSettings* m_appSettings;
};

#endif // RUNCOMMANDS_H
