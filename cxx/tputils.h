#ifndef TPUTILS_H
#define TPUTILS_H

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QUrl>

class TPUtils : public QObject
{

Q_OBJECT

public:
	explicit TPUtils(QObject* parent = nullptr);
	inline ~TPUtils() { delete m_appLocale; }

	Q_INVOKABLE const QString getCorrectPath(const QUrl& url) const;
	Q_INVOKABLE int getFileType(const QString& filename) const;
	Q_INVOKABLE inline QString getFileName(const QString& filepath) const { return QFileInfo(filepath).fileName(); };
	Q_INVOKABLE void copyToClipBoard(const QString& text) const;
	Q_INVOKABLE bool canReadFile(const QString& filename) const;

	inline QLocale* appLocale() const { return m_appLocale; }

	Q_INVOKABLE QString formatDate(const QDate& date) const;
	Q_INVOKABLE QString formatTodayDate() const;
	QDate getDateFromStrDate(const QString& strDate) const;
	Q_INVOKABLE uint calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const;
	Q_INVOKABLE QDate getMesoStartDate(const QDate& lastMesoEndDate) const;
	Q_INVOKABLE QDate createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const;
	Q_INVOKABLE QDate getDayBefore(const QDate& date) const { return date.addDays(-1); }
	Q_INVOKABLE bool areDatesTheSame(const QDate& date1, const QDate& date2) const { return date1 == date2; }

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
	Q_INVOKABLE QTime timeFromStrTime(const QString& strTime) const { return QTime::fromString(strTime, u"hh:mm"_qs); }
	Q_INVOKABLE QDateTime getCurrentTime() const { return QDateTime(QDate::currentDate(), QTime::currentTime()); }
	Q_INVOKABLE QString calculateTimeDifference_str(const QString& strTimeInit, const QString& strTimeFinal) const;
	Q_INVOKABLE QTime calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal) const;

	Q_INVOKABLE QString getCompositeValue(const uint idx, const QString& compositeString, const char chr_sep = 31) const;
	void setCompositeValue(const uint idx, const QString& newValue, QString& compositeString, const char chr_sep = 31) const;
	Q_INVOKABLE QString setCompositeValue_QML(const uint idx, const QString& newValue, const QString& compositeString) const
	{
		QString cpString(compositeString);
		setCompositeValue(idx, newValue, cpString);
		return cpString;
	}
	bool stringsAreSimiliar(const QString& string1, const QString& string2) const;
	Q_INVOKABLE QString setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const;

	void setAppLocale(const QString& localeStr);

	inline int splitLetterToIndex(const QString& strletter) const { return splitLetterToIndex(strletter.at(0)); }
	inline int splitLetterToIndex(const QChar& letter) const { return static_cast<int>(letter.cell()) - static_cast<int>('A'); }
	inline int splitLetterToMesoSplitIndex(const QString& strletter) const { return splitLetterToIndex(strletter.at(0)) + 2; }
	inline int splitLetterToMesoSplitIndex(const QChar& letter) const { return splitLetterToIndex(letter) + 2; }

signals:
	void appSuspended();
	void appResumed();

private:
	QLocale* m_appLocale;

	bool mb_appSuspended;

	inline QString addToTime(const QTime& origTime, const uint hours, const uint mins) const
	{
		const QTime& newTime(origTime.addSecs(mins*60 + hours*3600));
		return newTime.toString(u"hh:mm"_qs);
	}

	static TPUtils* app_utils;
	friend TPUtils* appUtils();
	friend class TPAppControl;
};

inline TPUtils* appUtils() { return TPUtils::app_utils; }

#endif // TPUTILS_H
