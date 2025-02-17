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
	enum DATE_FORMAT {
		DF_QML_DISPLAY,
		DF_CATALOG,
		DF_DATABASE,
		DF_ONLINE
	};

	enum TIME_FORMAT {
		TF_QML_DISPLAY_COMPLETE,
		TF_QML_DISPLAY_NO_SEC,
		TF_QML_DISPLAY_NO_HOUR,
		TF_FANCY,
		TF_FANCY_SECS,
		TF_ONLINE
	};

	explicit inline TPUtils(QObject *parent = nullptr) : QObject{parent}, m_appLocale{nullptr} { app_utils = this; }
	inline ~TPUtils() { delete m_appLocale; }

	Q_INVOKABLE const QString getCorrectPath(const QUrl& url) const;
	Q_INVOKABLE int getFileType(const QString &filename) const;
	Q_INVOKABLE inline QString getFileName(const QString &filepath) const { return QFileInfo(filepath).fileName(); };
	Q_INVOKABLE void copyToClipBoard(const QString &text) const;
	Q_INVOKABLE bool canReadFile(const QString &filename) const;

	inline QLocale* appLocale() const { return m_appLocale; }

	Q_INVOKABLE QString formatDate(const QDate &date, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	inline QString formatTodayDate() const { return formatDate(QDate::currentDate()); }
	QDate getDateFromDateString(const QString &strdate, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	uint calculateNumberOfWeeks(const QDate &date1, const QDate &date2) const;
	QDate getNextMonday(const QDate &fromDate) const;
	QDate createDate(const QDate &fromDate, const int years, const int months, const int days) const;
	Q_INVOKABLE inline QDate getDayBefore(const QDate &date) const { return date.addDays(-1); }
	int daysInMonth(const int month, const int year) const;

	Q_INVOKABLE QString formatTime(const QTime &time, const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const;
	QTime getTimeFromTimeString(const QString &strtime, const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const;
	Q_INVOKABLE inline QString getCurrentTimeString(const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const { return formatTime(QTime::currentTime(), format); }
	Q_INVOKABLE QString addTimeToStrTime(const QString &strTime, const int addmins, const int addsecs) const;
	Q_INVOKABLE inline QString getHourFromCurrentTime() const
	{
		return getHourFromStrTime(formatTime(QTime::currentTime(), TF_QML_DISPLAY_NO_SEC));
	}
	Q_INVOKABLE inline QString getMinutesFromCurrentTime() const
	{
		return getMinutesFromStrTime(formatTime(QTime::currentTime(), TF_QML_DISPLAY_NO_SEC));
	}
	Q_INVOKABLE QString getHourFromStrTime(const QString &strTime, const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const;
	Q_INVOKABLE QString getMinutesFromStrTime(const QString &strTime, const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const;
	Q_INVOKABLE QTime getCurrentTime() const { return QTime::currentTime(); }
	inline QString calculateTimeDifference_str(const QString &strTimeInit, const QString &strTimeFinal) const
	{
		return formatTime(calculateTimeDifference(strTimeInit, strTimeFinal), TF_QML_DISPLAY_COMPLETE);
	}

	QTime calculateTimeDifference(const QString &strTimeInit, const QString &strTimeFinal) const;

	QDateTime getDateTimeFromOnlineString(const QString &datetime) const;

	QString makeCompositeValue(const QString &defaultValue, const uint n_fields, const QLatin1Char &chr_sep) const;
	QString makeDoubleCompositeValue(const QString &defaultValue, const uint n_fields1, const uint n_fields2,
										const QLatin1Char &chr_sep1, const QLatin1Char &chr_sep2) const;
	QString getCompositeValue(const uint idx, const QString &compositeString, const QLatin1Char &chr_sep) const;
	void setCompositeValue(const uint idx, const QString &newValue, QString &compositeString, const QLatin1Char &chr_sep) const;
	void removeFieldFromCompositeValue(const uint idx, QString &compositeString, const QLatin1Char &chr_sep) const;
	inline QString string_strings( const std::initializer_list<QString> &strings, const QLatin1Char &chr_sep) const
	{
		QString ret;
		for (QString i : strings)
			ret += i + chr_sep;
		return ret;
	}

	bool stringsAreSimiliar(const QString &string1, const QString &string2) const;
	QString stripDiacriticsFromString(const QString &src) const;

	Q_INVOKABLE QString setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const;

	void setAppLocale(const QString &localeStr, const bool bWriteConfig);
	inline const QString &strLocale() const { return m_strLocale; }

	inline uint splitLetterToIndex(const QString &strletter) const { return splitLetterToIndex(strletter.at(0)); }
	inline uint splitLetterToIndex(const QChar &letter) const { return static_cast<int>(letter.cell()) - static_cast<int>('A'); }
	inline uint splitLetterToMesoSplitIndex(const QString &strletter) const { return splitLetterToIndex(strletter.at(0)) + 2; }
	inline uint splitLetterToMesoSplitIndex(const QChar &letter) const { return splitLetterToIndex(letter) + 2; }

private:
	QLocale *m_appLocale;
	QString m_strLocale;

	static TPUtils *app_utils;
	friend TPUtils *appUtils();
};

inline TPUtils *appUtils() { return TPUtils::app_utils; }

#endif // TPUTILS_H
