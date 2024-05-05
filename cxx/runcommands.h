#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QDateTime>
#include <QTimer>

class QSettings;
class QFileDialog;

class RunCommands : public QObject
{

Q_OBJECT

Q_PROPERTY(bool timerRunning READ timerRunning NOTIFY timerRunningChanged FINAL)

public:
	explicit RunCommands( QSettings* settings, QObject *parent = nullptr );
	~RunCommands() { if (m_workoutTimer) delete m_workoutTimer; }
	Q_INVOKABLE const QString getCorrectPath( const QUrl& url );
	Q_INVOKABLE int getFileType( const QString& filename );
	QString getAppDir(const QString& dbFile);
	Q_INVOKABLE void copyToClipBoard(const QString& text) const;

	inline QString getDBFileName() const { return m_dbFileName; }
	inline QString getAppPrivateDir() const { return m_appPrivateDir; }

	Q_INVOKABLE const QString formatDate(const QDate& date) const;
	Q_INVOKABLE const QString formatTodayDate() const;
	QDate getDateFromStrDate(const QString& strDate) const;
	Q_INVOKABLE uint calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const;
	Q_INVOKABLE QDate getMesoStartDate(const QDate& lastMesoEndDate) const;
	Q_INVOKABLE QDate createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const;
	Q_INVOKABLE QDate getDayBefore(const QDate& date) const { return date.addDays(-1); }

	Q_INVOKABLE QString intTimeToStrTime(const uint time) const;
	Q_INVOKABLE QString getStrHourFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString getStrMinFromTime(const QDateTime& time) const;
	Q_INVOKABLE QString formatTime(const QDateTime& time, const bool use_secs = false) const { return time.toString(!use_secs ? u"hh:mm"_qs : u"hh:mm:ss"_qs); }
	Q_INVOKABLE QString getCurrentTimeString(const bool use_secs = false) const { return !use_secs ?
					QTime::currentTime().toString(u"hh:mm"_qs) : QTime::currentTime().toString(u"hh:mm:ss"_qs); }
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

	const QTime calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal);
	bool timerRunning() const { return m_workoutTimer ? m_workoutTimer->isActive() : false; }
	Q_INVOKABLE void prepareWorkoutTimer(const QString& strStartTime = u"00:00:00"_qs);
	Q_INVOKABLE void startWorkoutTimer();
	Q_INVOKABLE void stopWorkoutTimer();

	Q_INVOKABLE QString getCompositeValue(const uint idx, const QString& compositeString) const;
	Q_INVOKABLE QString setCompositeValue(const uint idx, const QString& newValue, QString& compositeString) const;

signals:
	void appSuspended();
	void appResumed();
	void workoutTimerTriggered(const uint hours, const uint mins, const uint secs);
	void timeWarning(QString remaingMinutes, bool bminutes);
	void timerRunningChanged();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
	QSettings* m_appSettings;

	QTimer* m_workoutTimer;
	uint m_hours, m_mins, m_secs;
	uint mTimeWarnings;
	bool mb_timerForward;
	QTime mSessionStartTime;
	QTime mElapsedTime;
	QTime mSessionLength;

	bool mb_appSuspended;

	inline QString addToTime(const QTime& origTime, const uint hours, const uint mins) const
	{
		const QTime newTime(origTime.addSecs(mins*60 + hours*3600));
		return newTime.toString(u"hh:mm"_qs);
	}

	void calcTime();
	void calculateTimeBetweenTimes(const QTime& time1, const QTime& time2);
	void correctTimer();
};

#endif // RUNCOMMANDS_H
