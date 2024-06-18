#ifndef RUNCOMMANDS_H
#define RUNCOMMANDS_H

#include "tptimer.h"

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QDateTime>

class QSettings;
class QFileDialog;

static QLocale appLocale;
static QString androidUrl;

class RunCommands : public QObject
{

Q_OBJECT

public:
	explicit RunCommands( QSettings* settings, QObject *parent = nullptr );
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

	Q_INVOKABLE QString formatTime(const QDateTime& time, const bool use_hours = false, const bool use_secs = false) const
	{ return time.toString((use_hours ? u"hh:mm"_qs : u"mm"_qs) + (use_secs ? u":ss"_qs : u""_qs)); }
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
	Q_INVOKABLE QString calculateTimeDifference_str(const QString& strTimeInit, const QString& strTimeFinal) const;
	const QTime calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal) const;

	Q_INVOKABLE QString getCompositeValue(const uint idx, const QString& compositeString) const;
	Q_INVOKABLE QString setCompositeValue(const uint idx, const QString newValue, QString compositeString) const;
	bool stringsAreSimiliar(const QString& string1, const QString& string2) const;

	Q_INVOKABLE QString setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const;

signals:
	void appSuspended();
	void appResumed();

private:
	QString m_dbFileName;
	QString m_appPrivateDir;
	QSettings* m_appSettings;

	bool mb_appSuspended;

	inline QString addToTime(const QTime& origTime, const uint hours, const uint mins) const
	{
		const QTime newTime(origTime.addSecs(mins*60 + hours*3600));
		return newTime.toString(u"hh:mm"_qs);
	}
};

extern void setAppLocale(const QString& localeStr);

#endif // RUNCOMMANDS_H
