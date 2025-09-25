#pragma once

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;

constexpr QLatin1Char record_separator(28);
constexpr QLatin1Char exercises_separator(29);
constexpr QLatin1Char comp_exercise_separator(30);
constexpr QLatin1Char set_separator(31);
constexpr QLatin1Char fancy_record_separator1('|');
constexpr QLatin1Char fancy_record_separator2(';');
constexpr QLatin1StringView comp_exercise_fancy_separator(" + "_L1);

class TPUtils : public QObject
{

Q_OBJECT

public:
	enum DATE_FORMAT {
		DF_QML_DISPLAY,
		DF_LOCALE,
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

	const QString exercisesListFileIdentifier{Qt::Literals::StringLiterals::operator""_L1("0x01", 4)};
	const QString mesoFileIdentifier{Qt::Literals::StringLiterals::operator""_L1("0x02", 4)};
	const QString splitFileIdentifier{Qt::Literals::StringLiterals::operator""_L1("0x03", 4)};
	const QString workoutFileIdentifier{Qt::Literals::StringLiterals::operator""_L1("0x05", 4)};
	const QString userFileIdentifier{Qt::Literals::StringLiterals::operator""_L1("0x06", 4)};

	const QString STR_START_EXPORT{"##%%"_L1};
	const QString STR_START_FORMATTED_EXPORT{"####"_L1};
	const QString STR_END_EXPORT{"##!!"_L1};
	const QString STR_END_FORMATTED_EXPORT{"##$$"_L1};

	explicit inline TPUtils(QObject *parent = nullptr): QObject{parent}, m_appLocale{nullptr}
	{
		app_utils = this;
	}
	inline ~TPUtils() { delete m_appLocale; }

	int generateUniqueId(const QLatin1StringView &seed = QLatin1StringView{}) const;
	int idFromString(const QString &string_id) const; //not unique
	Q_INVOKABLE int generateRandomNumber(const int min, const int max) const;

	Q_INVOKABLE QString getCorrectPath(const QUrl &url) const;
	Q_INVOKABLE int getFileType(const QString &filename) const;
	Q_INVOKABLE bool canReadFile(const QString &filename) const;
	QString getFilePath(const QString &filename) const;
	QString getLastDirInPath(const QString &filename) const;
	//Returns the filename or the last directory in path if path does not include a file
	QString getFileName(const QString &filename, const bool without_extension = false) const;
	QString getFileExtension(const QString &filename, const bool include_dot = false) const;
	bool mkdir(const QString &fileOrDir) const;
	bool rename(const QString &source_file_or_dir, const QString &dest_file_or_dir, const bool overwrite) const;
	bool copyFile(const QString &srcFile, const QString &dstFileOrDir, const bool createPath = true) const;
	QFile *openFile(const QString &filename, QIODeviceBase::OpenMode flags) const;
	void scanDir(const QString &path, QFileInfoList &results, const QString &match = QString{}, const bool follow_tree = false) const;
	bool scanFile(const QString &filename, std::optional<bool> &formatted, uint &fileContents) const;
	bool writeDataToFile(QFile *out_file,
							const QString &identifier,
							const QList<QStringList> &data,
							const QList<uint> &export_rows = QList<uint>{},
							const bool use_real_id = true) const;

	bool writeDataToFormattedFile(QFile *out_file,
								  const QString &identifier,
								  const QList<QStringList> &data,
								  const QList<std::function<QString(void)>> &field_description,
								  const std::function<QString(const uint field, const QString &value)> &formatToExport = nullptr,
								  const QList<uint> &export_rows = QList<uint>{},
								  const QString &header = QString{} ) const;

	int readDataFromFile(QFile *in_file,
							QList<QStringList> &data,
							const uint field_count,
							const QString &identifier,
							const int row = -1) const;

	int readDataFromFormattedFile(QFile *in_file,
								  QList<QStringList> &data,
								  const uint field_count,
								  const QString &identifier,
								  const std::function<QString(const uint field, const QString &value)> &formatToImport = nullptr) const;

	QFile *createServerCmdFile(const QString &dir, const uint cmd_order, const std::initializer_list<QString> &command_parts) const;
	bool executeCmdFile(const QString &cmd_file, const QString &success_message, const bool remove_file = true) const;

	Q_INVOKABLE void copyToClipBoard(const QString &text) const;

	Q_INVOKABLE inline QString monthName(const uint qml_month) const { return _months_names.at(qml_month); }
	Q_INVOKABLE inline QString dayName(const uint week_day) const { return _days_names.at(week_day); }
	Q_INVOKABLE QString formatDate(const QDate &date, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	inline QString formatTodayDate(const DATE_FORMAT format = DF_QML_DISPLAY) const { return std::move(formatDate(QDate::currentDate())); }
	QDate getDateFromDateString(const QString &strdate, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	uint calculateNumberOfWeeks(const QDate &date1, const QDate &date2) const;
	//The returned value contains the number of months in between the dates plus the starting month
	inline uint calculateNumberOfMonths(const QString &date1, const QString &date2) const
	{
		return calculateNumberOfMonths(getDateFromDateString(date1, DF_DATABASE), getDateFromDateString(date2, DF_DATABASE));
	}
	uint calculateNumberOfMonths(const QDate &date1, const QDate &date2) const;
	QDate getNextSunday(const QDate &fromDate) const;
	QDate getNextMonday(const QDate &fromDate) const;
	QDate createDate(const QDate &fromDate, const int years, const int months, const int days) const;
	Q_INVOKABLE inline QDate getDayBefore(const QDate &date) const { return date.addDays(-1); }
	Q_INVOKABLE inline QDate yesterday() const { return QDate::currentDate().addDays(-1); }
	Q_INVOKABLE inline QDate tomorrow() const { return QDate::currentDate().addDays(1); }
	Q_INVOKABLE inline QDate today() const { return QDate::currentDate(); }
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

	QTime calculateTimeDifference(const QTime &start_time, const QTime &end_time) const;
	QTime calculateTimeDifference(const QString &strTimeInit, const QString &strTimeFinal) const;
	QDateTime getDateTimeFromOnlineString(const QString &datetime) const;

	QString makeCompositeValue(const QString &defaultValue, const uint n_fields, const QLatin1Char &chr_sep) const;
	QString makeDoubleCompositeValue(const QString &defaultValue, const uint n_fields1, const uint n_fields2,
										const QLatin1Char &chr_sep1, const QLatin1Char &chr_sep2) const;
	QString getCompositeValue(const uint idx, const QString &compositeString, const QLatin1Char &chr_sep) const;
	QString lastValueInComposite(const QString &compositeString, const QLatin1Char &chr_sep) const;
	void setCompositeValue(const uint idx, const QString &newValue, QString &compositeString, const QLatin1Char &chr_sep) const;
	void removeFieldFromCompositeValue(const uint idx, QString &compositeString, const QLatin1Char &chr_sep) const;
	int fieldOfValue(const QString &value, const QString &compositeString, const QLatin1Char &chr_sep) const;
	QString subSetOfCompositeValue(const QString &value, const uint from, const uint n, const QLatin1Char &chr_sep) const;
	inline QString string_strings(const std::initializer_list<QString> &strings, const QLatin1Char &chr_sep) const
	{
		QString ret;
		for (QString i : strings)
			ret += i + chr_sep;
		return ret;
	}
	inline uint nFieldsInCompositeString(const QString &compositeString, const QLatin1Char &chr_sep) const { return compositeString.count(chr_sep); }

	bool stringsAreSimiliar(const QString &string1, const QString &string2) const;
	QString stripDiacriticsFromString(const QString &src) const;

	Q_INVOKABLE QString setTypeOperation(const uint settype, const bool bIncrease, QString strValue, const bool seconds = false) const;

	inline QLocale *appLocale() const { return m_appLocale; }
	void setAppLocale(const QString &locale_str);

	inline uint splitLetterToIndex(const QString &strletter) const { return splitLetterToIndex(strletter.at(0)); }
	inline uint splitLetterToIndex(const QChar &letter) const { return static_cast<int>(letter.cell()) - static_cast<int>('A'); }
	inline uint splitLetterToMesoSplitIndex(const QString &strletter) const { return splitLetterToIndex(strletter.at(0)) + 2; }
	inline uint splitLetterToMesoSplitIndex(const QChar &letter) const { return splitLetterToIndex(letter) + 2; }

private:
	QLocale *m_appLocale;

	static QStringList _months_names;
	static QStringList _days_names;
	static TPUtils *app_utils;
	friend TPUtils *appUtils();
};

template <typename T>
inline void setBit(T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var |= (2 << (bit - 1));
	else
		var |= 1;
}

template <typename T>
inline void unSetBit(T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var &= ~(2 << (bit - 1));
	else
		var &= ~1;
}

template <typename T>
inline bool isBitSet(const T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		return static_cast<bool>(var & (2 << (bit - 1)));
	else
		return static_cast<bool>(var & 1);
}

inline bool containsAllWords(const QString &mainString, const QStringList &wordSet) {
	for (const auto &word : wordSet) {
		if (!mainString.contains(word, Qt::CaseInsensitive))
			return false; // Word not found in the main string
	}
	return true; // All words found in the main string
}

inline TPUtils *appUtils() { return TPUtils::app_utils; }
