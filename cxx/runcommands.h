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
	QDate getDateFromStrDate(const QString& strDate) const;

	Q_INVOKABLE uint calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const;
	Q_INVOKABLE QDate getMesoStartDate(const QDate& lastMesoEndDate) const;
	Q_INVOKABLE QDate createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const;
	Q_INVOKABLE QDate getDayBefore(const QDate& date) const { return date.addDays(-1); }

	Q_INVOKABLE QString intTimeToStrTime(const uint time) const;
	Q_INVOKABLE QString getStrHourFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString getStrMinFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString formatTime(const QDateTime& time) const { return time.toString(u"hh:mm"_qs); }
	Q_INVOKABLE QString getCurrentTimeString() const { return QTime::currentTime().toString(u"hh:mm"_qs); }
	Q_INVOKABLE QString addTimeToStrTime(const QString& strTime, const int addmins, const int addsecs) const;
	Q_INVOKABLE QString formatFutureTime(const uint hours, const uint mins) const { return addToTime(QTime::currentTime(), hours, mins); }
	Q_INVOKABLE QString formatFutureTime(const QDateTime& addTime) const;
	Q_INVOKABLE QString addToTime(const QString& origTime, const uint hours, const uint mins) const;
	Q_INVOKABLE QString getHourOrMinutesFromStrTime(const QString& strTime) const;
	Q_INVOKABLE QString getHourFromCurrentTime() const { return getHourOrMinutesFromStrTime(QTime::currentTime().toString(u"hh:mm"_qs)); }
	Q_INVOKABLE QString getMinutesFromCurrentTime() const { return getMinutesOrSeconsFromStrTime(QTime::currentTime().toString(u"hh:mm"_qs)); }
	Q_INVOKABLE QString getMinutesOrSeconsFromStrTime(const QString& strTime) const;
	Q_INVOKABLE QDateTime timeFromStrTime(const QString& strTime) const { return QDateTime(QDate::currentDate(), QTime::fromString(strTime, u"hh:mm"_qs)); }
	Q_INVOKABLE QDateTime getCurrentTime() const { return QDateTime(QDate::currentDate(), QTime::currentTime()); }
	Q_INVOKABLE QDateTime calculateTimeBetweenTimes(const QString& strTime1, const QString& strTime2) const;
	Q_INVOKABLE QDateTime calculateTimeRemaing(const QString& strFinalTime) const { return calculateTimeBetweenTimes(QTime::currentTime().toString(u"hh:mm"_qs), strFinalTime); }
	Q_INVOKABLE QDateTime updateTimer(const QDateTime& timeOfSuspension, const QDateTime& currentTimer, const bool bTimer) const;

signals:
	void dbFileNameChanged();
	void appPrivateDirChanged();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
	QSettings* m_appSettings;

	inline QString addToTime(const QTime& origTime, const uint hours, const uint mins) const
	{
		const QTime newTime(origTime.addSecs(mins*60 + hours*3600));
		return newTime.toString(u"hh:mm"_qs);
	}
};

#endif // RUNCOMMANDS_H
