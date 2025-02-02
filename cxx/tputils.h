#ifndef TPUTILS_H
#define TPUTILS_H

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;

class TPUtils : public QObject
{

Q_OBJECT

public:
	enum DATE_FORMAT {
		DF_QML_DISPLAY,
		DF_CATALOG,
		DF_DATABASE
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
	QDate getDateFromStrDate(const QString &strDate) const;
	uint calculateNumberOfWeeks(const QDate &date1, const QDate &date2) const;
	QDate getNextMonday(const QDate &fromDate) const;
	QDate createDate(const QDate &fromDate, const int years, const int months, const int days) const;
	Q_INVOKABLE inline QDate getDayBefore(const QDate &date) const { return date.addDays(-1); }
	int daysInMonth(const int month, const int year) const;

	Q_INVOKABLE inline QString formatTime(const QTime &time, const bool use_hours = false, const bool use_secs = false) const
	{
		return time.toString((use_hours ? "hh:mm"_L1 : "mm"_L1) + (use_secs ? ":ss"_L1 : ""_L1));
	}

	inline QString currentFormattedTimeString() const
	{
		QString strTime{std::move(QTime::currentTime().toString("hh  mm"_L1))};
		strTime.insert(6, std::move("min"_L1));
		strTime.insert(3, std::move(tr("and")));
		strTime.insert(2, std::move("hs"_L1));
		return strTime;
	}

	Q_INVOKABLE QString getCurrentTimeString(const bool use_secs = false) const { return !use_secs ?
					QTime::currentTime().toString("hh:mm"_L1) : QTime::currentTime().toString("hh:mm:ss"_L1); }
	Q_INVOKABLE QString addTimeToStrTime(const QString &strTime, const int addmins, const int addsecs) const;
	Q_INVOKABLE inline QString getHourFromCurrentTime() const { return getHourOrMinutesFromStrTime(QTime::currentTime().toString("hh:mm"_L1)); }
	Q_INVOKABLE inline QString getMinutesFromCurrentTime() const { return getMinutesOrSeconsFromStrTime(QTime::currentTime().toString("hh:mm"_L1)); }
	Q_INVOKABLE QString getHourOrMinutesFromStrTime(const QString &strTime) const;
	Q_INVOKABLE QString getMinutesOrSeconsFromStrTime(const QString &strTime) const;
	QTime timeFromStrTime(const QString &strTime) const { return QTime::fromString(strTime, "hh:mm"_L1); }
	Q_INVOKABLE QTime getCurrentTime() const { return QTime::currentTime(); }
	QString calculateTimeDifference_str(const QString &strTimeInit, const QString &strTimeFinal) const;
	QTime calculateTimeDifference(const QString &strTimeInit, const QString &strTimeFinal) const;

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
