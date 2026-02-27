#pragma once

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QQmlEngine>
#include <QSize>
#include <QUrl>

namespace QLiterals = Qt::Literals::StringLiterals;
using namespace QLiterals;

constexpr QLatin1Char exercises_separator{28}; // \034 0x1c
constexpr QLatin1Char comp_exercises_separator{29}; // \035 0x1d
constexpr QLatin1Char record_separator{30}; // \036 0x1e
constexpr QLatin1Char set_separator{31}; // \037 0x1f
constexpr QLatin1Char binary_file_initial_separator{record_separator};
constexpr QLatin1Char binary_file_separator{set_separator};

constexpr QLatin1Char fancy_record_separator1{'|'};
constexpr QLatin1Char fancy_record_separator2{';'};
constexpr QLatin1StringView comp_exercise_fancy_separator{" + "_L1};

static const QString STR_ONE{'1'};
static const QString STR_ZERO{'0'};

// Helper macros for token pasting
#define PASTE_HELPER(a, b) a##b
#define PASTE(a, b) PASTE_HELPER(a, b)

#define createRole(roleName, enumField) \
	PASTE(roleName, Role) = Qt::UserRole + static_cast<uint8_t>(enumField),

#define STRINGIFY(x) #x
// Macro to wrap the parameter with quotes
#define QUOTE(x) STRINGIFY(x)
#define roleToString(roleName) \
	m_roleNames[PASTE(roleName, Role)] = std::move(QUOTE(roleName));

class TPUtils : public QObject
{

Q_OBJECT
QML_ELEMENT

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

	enum FILE_TYPE {
		FT_TP_USER_PROFILE	= 1U << 0,
		FT_TP_PROGRAM		= 1U << 1,
		FT_TP_WORKOUT_A		= 1U << 2,
		FT_TP_WORKOUT_B		= 1U << 3,
		FT_TP_WORKOUT_C		= 1U << 4,
		FT_TP_WORKOUT_D		= 1U << 5,
		FT_TP_WORKOUT_E		= 1U << 6,
		FT_TP_WORKOUT_F		= 1U << 7,
		FT_TP_EXERCISES		= 1U << 8,
		FT_TP_FORMATTED		= 1U << 9,
		FT_IMAGE			= 1U << 10,
		FT_VIDEO			= 1U << 11,
		FT_PDF				= 1U << 12,
		FT_TEXT				= 1U << 13,
		FT_OPEN_DOCUMENT	= 1U << 14,
		FT_MS_DOCUMENT		= 1U << 15,
		FT_OTHER			= 1U << 30,
		FT_UNKNOWN			= 1U << 31,
	};
	Q_ENUM(FILE_TYPE);

	enum BINARY_FILE_INFO_FIELDS {
		BFIF_LOCAL_HANDLER_ID,
		BFIF_SUBDIR_PLUS_FILENAME,
		BFIF_SENDERID,
		BFIF_RECEIVERID,
		BFIF_MESONAME,
		BFIF_SPLITLETTER,
		BFIF_TOTAL_FIELDS
	};

	const QString exercisesListFileIdentifier{QLiterals::operator""_L1("0x01", 4)};
	const QString mesoFileIdentifier{QLiterals::operator""_L1("0x02", 4)};
	const QString splitFileIdentifier{QLiterals::operator""_L1("0x03", 4)};
	const QString workoutFileIdentifier{QLiterals::operator""_L1("0x05", 4)};
	const QString userFileIdentifier{QLiterals::operator""_L1("0x06", 4)};

	static constexpr QLatin1StringView STR_START_EXPORT{"##%%"_L1};
	static constexpr QLatin1StringView STR_START_FORMATTED_EXPORT{"####"_L1};
	static constexpr QLatin1StringView STR_END_EXPORT{"##!!"_L1};
	static constexpr QLatin1StringView STR_END_FORMATTED_EXPORT{"##$$"_L1};

	static constexpr QLatin1StringView previewImagesSubDir{"temp-img/"};

	explicit TPUtils(QObject *parent = nullptr);
	inline ~TPUtils() { delete m_appLocale; }

	int generateUniqueId(const QLatin1StringView &seed = QLatin1StringView{}) const;
	int idFromString(const QString &string_id) const; //not unique
	Q_INVOKABLE int generateRandomNumber(const int min, const int max) const;

	Q_INVOKABLE FILE_TYPE getFileType(const QString &filename) const;
	FILE_TYPE getTPFileType(const QString &filename, std::optional<bool> &formatted) const;
	QString getFileTypeIcon(const QString &filename, const QSize &preferred_size = QSize{}) const;
	QString getImagePreviewFile(const QString &image_filename, const QSize &preferred_size = QSize{}) const;
	Q_INVOKABLE void viewOrOpenFile(const QString &filename, const QVariant &extra_info = QVariant{});

	Q_INVOKABLE QString getCorrectPath(const QUrl &url) const;
	Q_INVOKABLE bool canReadFile(const QString &filename) const;
	QString getFilePath(const QString &filename) const;
	QString getNthDirInPath(const QString &filename, int nth_dir = -1, int n_dirs = 1) const;
	void removeNthDirFromPath(QString &path, int nth_dir);
	//Returns the filename or the last directory in path if path does not include a file
	QString getFileName(const QString &filename, const bool without_extension = false) const;
	QString getFileExtension(const QString &filename, const bool include_dot = false, const QString &default_ext = QString{}) const;
	//Only for files stored in the app's private directory
	QString getSubDir(const QString &filename) const;

	/**
	 * @brief fileRecentlyModified
	 * @param filename Local filename(including path)
	 * @param threshold: Number of minutes past from now since filename was last modified to be considered borderline for recentness.
	 * @return false if the file was modified more than threshold minutes; less or equal to threshold, true
	 */
	bool fileRecentlyModified(const QString &filename, const int threshold = 30) const;

	bool mkdir(const QString &fileOrDir) const;
	bool rename(const QString &source_file_or_dir, const QString &dest_file_or_dir, const bool overwrite) const;
	bool copyFile(const QString &srcFile, const QString &dstFileOrDir, const bool createPath = true, const bool remove_source = false) const;
	QFile *openFile(const QString &filename, const bool read = true, const bool write = false, const bool append = false,
							const bool overwrite = false, const bool text = true) const;
	void scanDir(const QString &path, QFileInfoList &results, const QString &match = QString{}, const bool follow_tree = false) const;
	void rmDir(const QString &path) const;

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

	QByteArray readBinaryFile(const QString &filename, const QString &extra_info = QString{}) const &;
	void writeBinaryFile(const QString &filename, const QByteArray &data, const bool strip_extra_info) const;
	void insertOrModifyBinaryFileField(QByteArray &data, BINARY_FILE_INFO_FIELDS field, const QString &info) const;
	QString binaryFileExtraFieldValue(const QByteArray &data, BINARY_FILE_INFO_FIELDS field) const;

	Q_INVOKABLE void copyToClipboard(const QString &text) const;
	Q_INVOKABLE QString pasteFromClipboard() const;

	Q_INVOKABLE inline QString monthName(const uint qml_month) const { return _months_names.at(qml_month); }
	Q_INVOKABLE inline QString dayName(const uint week_day) const { return _days_names.at(week_day); }
	Q_INVOKABLE QString formatDate(const QDate &date, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	inline QString formatTodayDate(const DATE_FORMAT format = DF_QML_DISPLAY) const { return std::move(formatDate(QDate::currentDate())); }
	QDate dateFromString(const QString &strdate, const DATE_FORMAT format = DF_QML_DISPLAY) const;
	uint calculateNumberOfWeeks(const QDate &date1, const QDate &date2) const;
	//Returns the number of months in between the dates plus one(the starting month)
	inline uint calculateNumberOfMonths(const QString &date1, const QString &date2) const
	{
		return calculateNumberOfMonths(dateFromString(date1, DF_DATABASE), dateFromString(date2, DF_DATABASE));
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
	QTime timeFromString(const QString &strtime, const TIME_FORMAT format = TF_QML_DISPLAY_NO_SEC) const;
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
	QString getCompositeValue(const uint idx, const QString &compositeString, const QLatin1Char &chr_sep) const;
	QString lastValueInComposite(const QString &compositeString, const QLatin1Char &chr_sep) const;
	void setCompositeValue(const int idx, const QString &newValue, QString &compositeString, const QLatin1Char &chr_sep) const;
	bool removeFieldFromCompositeValue(const uint idx, QString &compositeString, const QLatin1Char &chr_sep) const;
	int fieldOfValue(const QString &value, const QString &compositeString, const QLatin1Char &chr_sep) const;
	QString subSetOfCompositeValue(const QString &value, const uint from, const uint n, const QLatin1Char &chr_sep) const;
	inline QString string_strings(const std::initializer_list<QString> &strings, const QLatin1Char &chr_sep) const
	{
		return std::accumulate(strings.begin(), strings.end(), QString{}, [chr_sep] (QString &&final_str, QString str) {
			return final_str.append(str % chr_sep);
		});
	}
	inline uint nFieldsInCompositeString(const QString &compositeString, const QLatin1Char &chr_sep) const { return compositeString.count(chr_sep); }

	double similarityBetweenStrings(const QString &string1, const QString &string2) const;
	QString stripDiacriticsFromString(const QString &src) const;
	Q_INVOKABLE QString stripInvalidCharacters(const QString &string) const;
	bool containsAllWords(const QString &mainString, const QStringList &wordSet, const bool precise = false);

	Q_INVOKABLE QString setTypeOperation(const uint settype, const bool increase, QString str_value, const bool seconds = false) const;

	inline QLocale *appLocale() const { return m_appLocale; }
	void setAppLocale(const QString &locale_str);
	inline QString newDBTemporaryId() const { return QString::number(m_lowestTempId--); }

signals:
	void tpFileOpenRequest(uint32_t tp_filetype, const QString &filename, const bool formatted = false, const QVariant &extra_info = QVariant{});

private:
	QLocale *m_appLocale;
	mutable int16_t m_lowestTempId;
	QStringList _months_names;
	QStringList _days_names;

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

inline TPUtils *appUtils() { return TPUtils::app_utils; }
