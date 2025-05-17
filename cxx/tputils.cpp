#include "tputils.h"

#include "dbexercisesmodel.h"
#include "tpglobals.h"
#include "tpsettings.h"

#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QLocale>
#include <QStandardPaths>

#include <ranges>

TPUtils* TPUtils::app_utils{nullptr};

TPUtils::TPUtils(QObject *parent)
	: QObject{parent}, m_appLocale{nullptr},
		m_localAppFilesDir{std::move(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)) + QLatin1Char('/')}
{
	app_utils = this;
}

int TPUtils::generateUniqueId(const QLatin1StringView &seed) const
{
	if (seed.isEmpty())
		return QTime::currentTime().msecsSinceStartOfDay();
	else
	{
		int n{0};
		int shift{2};
		auto itr{seed.constBegin()};
		const auto itr_end{seed.constEnd()};
		const int shifter{qFloor(QTime::currentTime().msec()/100)};
		do {
			n += static_cast<int>(*itr);
			if (--shift == 0)
			{
				n <<= shifter;
				shift = 2;
			}
		} while (++itr != itr_end);
		return n;
	}
}

int TPUtils::idFromString(const QString &string_id) const
{
	const int id{std::accumulate(string_id.cbegin(), string_id.cend(), 0, [] (int sum, const QChar &chr) {
		return sum + static_cast<int>(chr.toLatin1());
	})};
	return id;
}

QString TPUtils::getCorrectPath(const QUrl &url) const
{
	QString path{url.toString(QUrl::PrettyDecoded|QUrl::PreferLocalFile|QUrl::RemoveScheme)};
	if (path.startsWith("file://"_L1))
		path.remove(0, 7);
	return path;
}

int TPUtils::getFileType(const QString &filename) const
{
	#ifdef Q_OS_ANDROID
		return filename.contains("video%"_L1) ? 1 : (filename.contains("image%"_L1) ? 0 : -1);
	#else
		if (filename.endsWith(".mp4"_L1) || filename.endsWith(".mkv"_L1) || filename.endsWith(".mov"_L1))
			return 1;
		else return (filename.endsWith(".png"_L1) || filename.endsWith(".jpg"_L1)) ? 0 : -1;
	#endif
}

bool TPUtils::canReadFile(const QString &filename) const
{
	const QFileInfo file{filename};
	if (file.isFile())
		return file.isReadable();
	return false;
}

QString TPUtils::getFilePath(const QString &filename) const
{
	const qsizetype slash_idx{filename.lastIndexOf('/')};
	if (slash_idx > 0)
		return filename.left(slash_idx + 1); //include the trainling '/'
	return filename;
}

QString TPUtils::getLastDirInPath(const QString &filename) const
{
	const QString &filePath{getFilePath(filename)};
	QString ret;
	for (const auto &chr : filePath | std::views::reverse)
	{
		if (chr != '/')
			ret += chr;
		else
		{
			if (ret.length() > 0)
				break;
		}
	}
	return ret;
}

QString TPUtils::getFileName(const QString &filename, const bool without_extension) const
{
	QString f_name;
	const qsizetype slash_idx{filename.lastIndexOf('/')};
	if (slash_idx > 0)
		f_name = std::move(filename.right(filename.length() - slash_idx -1));

	if (without_extension)
	{
		const qsizetype dot_idx{slash_idx > 0 ? f_name.lastIndexOf('.') : filename.lastIndexOf('.')};
		if (dot_idx > 0)
			return slash_idx > 0 ? f_name.left(dot_idx) : filename.left(dot_idx);
	}

	return slash_idx > 0 ? f_name : filename;
}

void TPUtils::copyToClipBoard(const QString &text) const
{
	qApp->clipboard()->setText(text);
}

bool TPUtils::mkdir(const QString &fileOrDir) const
{
	const QFileInfo fi{fileOrDir};
	const QString &path{(!fi.exists() || fi.isFile()) ? getFilePath(fileOrDir) : fileOrDir};
	QDir fs_dir{path};
	if (!fs_dir.exists())
		return fs_dir.mkpath(path);
	return true;
}

bool TPUtils::copyFile(const QString &srcFile, const QString &dstFileOrDir, const bool createPath) const
{
	if (QFile::exists(srcFile))
	{
		if (createPath)
		{
			const QString &filepath{getFilePath(dstFileOrDir)};
			if (!mkdir(filepath))
				return false;
		}
		const QFileInfo fi{dstFileOrDir};
		const QString &dstFile{fi.isDir() ? dstFileOrDir + getFileName(srcFile) : dstFileOrDir};
		return QFile::copy(srcFile, dstFile);
	}
	return false;
}

QFile *TPUtils::openFile(const QString &filename, QIODeviceBase::OpenMode flags) const
{
	const bool exists{QFile::exists(filename)};
	if (!exists && (flags & QIODeviceBase::ReadOnly))
		return nullptr;
	if (mkdir(filename))
	{
		QFile *file{new QFile{filename}};
		if (file->open(flags))
		{
			file->deleteLater();
			return file;
		}
		delete file;
	}
	return nullptr;
}

bool TPUtils::scanFile(const QString &filename, std::optional<bool> &formatted, uint &file_contents) const
{
	QFile *in_file{openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (!in_file)
		return false;

	formatted = std::nullopt;
	file_contents = 0;

	qint64 lineLength{0};
	char buf[128];
	QString split_info;
	const char *raw_data_file{STR_START_EXPORT.toUtf8().constData()};
	const char *raw_data_file_end{STR_END_EXPORT.toUtf8().constData()};
	const char *formatted_data_file{STR_START_FORMATTED_EXPORT.toUtf8().constData()};
	const char *formatted_data_file_end{STR_END_FORMATTED_EXPORT.toUtf8().constData()};
	const char *split_label{DBExercisesModel::splitLabel().toUtf8().constData()};
	const uint split_label_length{static_cast<uint>(DBExercisesModel::splitLabel().length())};

	while ((lineLength = in_file->readLine(buf, sizeof(buf))) != -1)
	{
		if (lineLength >= 8)
		{
			if(strncmp(buf, "##", 2) == 0)
			{
				if(strncmp(buf, raw_data_file_end, 4) == 0)
					continue;
				else if(strncmp(buf, formatted_data_file_end, 4) == 0)
					continue;
				else if (strstr(buf, raw_data_file) != NULL)
					*formatted = false;
				else if (strstr(buf, formatted_data_file) != NULL)
					*formatted = true;
				else if (strstr(buf, exercisesListFileIdentifier.toLatin1().constData()) != NULL)
					setBit(file_contents, IFC_EXERCISES);
				else if (strstr(buf, mesoFileIdentifier.toLatin1().constData()) != NULL)
					setBit(file_contents, IFC_MESO);
				else if (strstr(buf, splitFileIdentifier.toLatin1().constData()) != NULL)
					setBit(file_contents, IFC_MESOSPLIT);
				else if (strstr(buf, workoutFileIdentifier.toLatin1().constData()) != NULL)
					setBit(file_contents, IFC_WORKOUT);
				else if (strstr(buf, userFileIdentifier.toLatin1().constData()) != NULL)
					setBit(file_contents, IFC_USER);

				if (isBitSet(file_contents, IFC_MESOSPLIT) || isBitSet(file_contents, IFC_WORKOUT))
				{
					if(strncmp(buf, split_label, split_label_length) == 0)
					{
						split_info = buf;
						const int idx{static_cast<int>(split_info.indexOf(DBExercisesModel::splitLabel()) + split_label_length + 2)};
						switch (split_info.sliced(idx, 1).at(0).toLatin1())
						{
							case 'A': setBit(file_contents, IFC_MESOSPLIT_A); break;
							case 'B': setBit(file_contents, IFC_MESOSPLIT_B); break;
							case 'C': setBit(file_contents, IFC_MESOSPLIT_C); break;
							case 'D': setBit(file_contents, IFC_MESOSPLIT_D); break;
							case 'E': setBit(file_contents, IFC_MESOSPLIT_E); break;
							case 'F': setBit(file_contents, IFC_MESOSPLIT_F); break;
						}
					}
				}
			}
		}
	}
	in_file->close();
	delete in_file;
	return true;
}

bool TPUtils::writeDataToFile(QFile *out_file,
								const QString &identifier,
								const QList<QStringList> &data,
								const QList<uint> &export_rows,
								const bool use_real_id) const
{
	if (!out_file || !out_file->isOpen())
		return false;

	out_file->write(QString{STR_START_EXPORT + identifier + '\n'}.toUtf8().constData());

	if (export_rows.isEmpty())
	{
		for (const auto &modeldata : data)
		{
			uint i{0};
			if (!use_real_id)
			{
				out_file->write("-1\n", 3);
				i = 1;
			}
			for (; i < modeldata.count(); ++i)
			{
				out_file->write(modeldata.at(i).toUtf8().constData());
				out_file->write("\n", 1);
			}
			out_file->write(STR_END_EXPORT.toUtf8().constData());
		}
	}
	else
	{
		for (uint x{0}; x < export_rows.count(); ++x)
		{
			uint i{0};
			for (const auto &modeldata : data.at(export_rows.at(x)))
			{
				if (i == 0 && !use_real_id)
				{
					out_file->write("-1\n", 3);
					i = 1;
				}
				else
				{
					out_file->write(modeldata.toUtf8().constData());
					out_file->write("\n", 1);
				}
			}
			out_file->write(STR_END_EXPORT.toUtf8().constData());
		}
	}
	out_file->flush();
	return true;
}

bool TPUtils::writeDataToFormattedFile(QFile *out_file,
								const QString &identifier,
								const QList<QStringList> &data,
								const QList<std::function<QString(void)>> &field_description,
								const std::function<QString(const uint field, const QString &value)> &formatToExport,
								const QList<uint> &export_rows,
								const QString &header) const
{
	if (!out_file || !out_file->isOpen())
		return false;

	QString first_line{std::move(STR_START_FORMATTED_EXPORT + identifier)};
	if (!header.isEmpty())
		first_line += std::forward<QString>("  "_L1 + header);
	first_line += std::forward<QString>(std::move("\n\n"_L1));
	out_file->write(first_line.toUtf8().constData());

	if (export_rows.isEmpty())
	{
		for (const auto &modeldata : data)
		{
			for (uint i{0}; i < modeldata.count(); ++i)
			{
				if (field_description.at(i) != nullptr)
				{
					out_file->write(field_description.at(i)().toUtf8().constData());
					if (formatToExport == nullptr)
						out_file->write(modeldata.at(i).toUtf8().constData());
					else
						out_file->write(formatToExport(i, modeldata.at(i)).toUtf8().constData());
					out_file->write("\n", 1);
				}
			}
			out_file->write(STR_END_FORMATTED_EXPORT.toUtf8().constData());
		}
	}
	else
	{
		for (uint x{0}; x < export_rows.count(); ++x)
		{
			for (const auto &modeldata : data.at(export_rows.at(x)))
			{
				uint i{0};
				if (field_description.at(i) != nullptr)
				{
					out_file->write(field_description.at(i)().toUtf8().constData());
					if (formatToExport == nullptr)
						out_file->write(modeldata.toUtf8().constData());
					else
						out_file->write(formatToExport(i, modeldata).toUtf8().constData());
					out_file->write("\n", 1);
				}
				++i;
			}
			out_file->write(STR_END_FORMATTED_EXPORT.toUtf8().constData());
		}
	}
	out_file->flush();
	return true;
}

int TPUtils::readDataFromFile(QFile *in_file,
								QList<QStringList> &data,
								const uint field_count,
								const QString &identifier,
								const int row) const
{
	if (!in_file || !in_file->isOpen())
		return -1;

	const qsizetype prevCount{data.count()};
	QStringList data_read{field_count};
	bool identifier_found{false};
	const char *identifier_in_file{QString{STR_START_EXPORT + identifier}.toUtf8().constData()};
	char buf[512];

	while (in_file->readLine(buf, sizeof(buf)) != -1)
	{
		if (strstr(buf, STR_START_EXPORT.toUtf8().constData()) != NULL)
		{
			if (!identifier_found)
				identifier_found = strstr(buf, identifier_in_file) != NULL;
			else //Found the beginning of another data set
			{
				if (strstr(buf, identifier_in_file) == NULL) //Data set of a different type, rewind and return
				{
					in_file->seek(in_file->pos()-strlen(buf));
					break;
				}
			}
		}
		else
		{
			if (strstr(buf, STR_END_EXPORT.toUtf8().constData()) != NULL)
			{
				if (data_read.count() >= field_count)
				{
					if (data_read.count() > field_count)
						data_read.resize(field_count);
					if (row == -1)
						data.append(std::move(data_read));
					else if (row < data.count())
						data.replace(row, std::move(data_read));
				}
			}
			else
				data_read.append(std::move(QString{buf}.chopped(1)));
		}

	}
	return identifier_found ? data.count() - prevCount : APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE;
}

int TPUtils::readDataFromFormattedFile(QFile *in_file,
										QList<QStringList> &data,
										const uint field_count,
										const QString &identifier,
										const std::function<QString(const uint field, const QString &value)> &formatToImport) const
{
	if (!in_file || !in_file->isOpen())
		return -1;

	bool identifier_found{false};
	const char *identifier_in_file{QString{STR_START_FORMATTED_EXPORT + identifier}.toLatin1().constData()};
	char buf[512];
	QString value;
	uint field{1}; //skip ID
	int line_length{0};
	QStringList data_read{field_count};

	while ((line_length = in_file->readLine(buf, sizeof(buf))) != -1)
	{
		if (line_length < 5)
			continue;

		if (strstr(buf, STR_START_FORMATTED_EXPORT.toUtf8().constData()) != NULL)
		{
			if (!identifier_found)
				identifier_found = strstr(buf, identifier_in_file) != NULL;
			else //Found the beginning of another data set
			{
				if (strstr(buf, identifier_in_file) == NULL) //Data set of a different type, rewind and return
				{
					in_file->seek(in_file->pos()-strlen(buf));
					break;
				}
			}
		}
		else
		{
			if (strstr(buf, STR_END_FORMATTED_EXPORT.toUtf8().constData()) != NULL)
			{
				data.append(std::move(data_read));
				field = 1;
			}
			else
			{
				if (field < field_count)
				{
					value = buf;
					value = std::move(value.remove(0, value.indexOf(':') + 2).simplified());
					if (formatToImport == nullptr)
						data_read[field] = std::move(value);
					else
						data_read[field] = std::move(formatToImport(field, value));
					++field;
				}
			}
		}
	}
	return identifier_found ? field : APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE;
}

void TPUtils::scanDir(const QString &path, QFileInfoList &results, const QString &match, const bool follow_tree) const
{
	QDir dir{path};
	if (dir.isReadable())
	{
		results.append(std::move(dir.entryInfoList(QStringList{match}, QDir::Files|QDir::NoDotAndDotDot)));
		if (follow_tree)
		{
			const QStringList &subdirs{dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)};
			for (const auto &subdir: subdirs)
				scanDir(path + subdir, results, match, true);
		}
	}
}

QString TPUtils::formatDate(const QDate &date, const DATE_FORMAT format) const
{
	switch (format)
	{
		case DF_QML_DISPLAY:
			return m_appLocale->toString(date, "ddd d/M/yyyy"_L1);
		break;
		case DF_LOCALE:
			return m_appLocale->toString(date, QLocale::ShortFormat);
		case DF_CATALOG:
			return QString::number(date.year()) + QString::number(date.month()) + QString::number(date.day());
		break;
		case DF_DATABASE:
			return QString::number(date.toJulianDay());
		break;
		case DF_ONLINE:
			return QString::number(date.year()).right(2) + QString::number(date.month()) + QString::number(date.day());
		break;
	}
	return QString{};
}

QDate TPUtils::getDateFromDateString(const QString &strdate, const DATE_FORMAT format) const
{
	if (strdate.isEmpty())
		return QDate{};

	int day{0}, month{0}, year{0};
	switch (format)
	{
		case DF_QML_DISPLAY:
			{
				const qsizetype spaceIdx{strdate.indexOf(' ')};
				const qsizetype fSlashIdx{strdate.indexOf('/')};
				const qsizetype fSlashIdx2{strdate.indexOf('/', fSlashIdx+1)};
				day = strdate.sliced(spaceIdx+1, fSlashIdx-spaceIdx-1).toInt();
				month = strdate.sliced(fSlashIdx+1, fSlashIdx2-fSlashIdx-1).toInt();
				year = strdate.last(4).toInt();
			}
		break;
		case DF_LOCALE:

		break;
		case DF_CATALOG:
			year = strdate.first(4).toInt();
			month = strdate.sliced(4, 2).toInt();
			day = strdate.sliced(6, 2).toInt();
		break;
		case DF_DATABASE:
			return QDate::fromJulianDay(strdate.toLongLong());
		break;
		case DF_ONLINE:
			year = strdate.first(2).toInt() + 2000;
			month = strdate.sliced(2, 2).toInt();
			day = strdate.sliced(4, 2).toInt();
		break;
	}
	return QDate{year, month, day};
}

uint TPUtils::calculateNumberOfWeeks(const QDate &date1, const QDate &date2) const
{
	uint n{0};
	const int week1{date1.weekNumber()};
	const int week2{date2.weekNumber()};
	//Every 6 years we have a 53 week year
	if (week2 < week1)
	{
		const int totalWeeksInYear{QDate::currentDate().year() != 2026 ? 52 : 53};
		n = (totalWeeksInYear - week1) + week2;
	}
	else
		n = week2 - week1;
	return n+1; //+1 include current week
}

uint TPUtils::calculateNumberOfMonths(const QDate &date1, const QDate &date2) const
{
	int n_months{0};
	if (date1.year() == date2.year())
	{
		n_months = date2.month() - date1.month() + 1;
		if (n_months < 0)
			n_months *= -1;
	}
	else
	{
		if (date2.year() > date1.year())
		{
			n_months = date2.month();
			n_months += 12 - date1.month() + 1;
		}
		else
		{
			n_months = date1.month();
			n_months += 12 - date2.month() + 1;
		}
	}
	return n_months;
}

QDate TPUtils::getNextMonday(const QDate &fromDate) const
{
	constexpr uint daysToNextMonday[7]{ 7, 6, 5, 4, 3, 2, 1 };
	return fromDate.addDays(daysToNextMonday[fromDate.dayOfWeek()-1]);
}

QDate TPUtils::createDate(const QDate &fromDate, const int years, const int months, const int days) const
{
	QDate newDate{fromDate};
	newDate = std::move(newDate.addDays(days));
	newDate = std::move(newDate.addMonths(months));
	newDate = std::move(newDate.addYears(years));
	return newDate;
}

int TPUtils::daysInMonth(const int month, const int year) const
{
	switch (month)
	{
		case 3: case 5: case 8: case 10: return 30;
		case 1: return year % 4 == 0 ? 29 : 28;
		default: return 31;
	}
}

QString TPUtils::formatTime(const QTime &time, const TIME_FORMAT format) const
{
	switch (format)
	{
		case TF_QML_DISPLAY_COMPLETE:
			return time.toString("hh:mm:ss"_L1);
		break;
		case TF_QML_DISPLAY_NO_SEC:
			return time.toString("hh:mm"_L1);
		break;
		case TF_QML_DISPLAY_NO_HOUR:
			return time.toString("mm:ss"_L1);
		break;
		case TF_FANCY:
		{
			QString strTime{std::move(time.toString("hh  mm"_L1))};
			strTime.insert(6, std::move("min"_L1));
			strTime.insert(3, std::move(tr("and")));
			strTime.insert(2, std::move("hs"_L1));
		}
		case TF_FANCY_SECS:
		{
			QString strTime{std::move(time.toString("hh, mm  ss"_L1))};
			strTime.insert(10, std::move("secs"));
			strTime.insert(7, std::move(tr("and")));
			strTime.insert(6, std::move("min"_L1));
			strTime.insert(2, std::move("hs"_L1));
		}
		break;
		case TF_ONLINE:
			return time.toString("hhmmss"_L1);
		break;
	}
	return QString{};
}

QTime TPUtils::getTimeFromTimeString(const QString &strtime, const TIME_FORMAT format) const
{
	int hour{0}, min{0}, sec{0};
	switch (format)
	{
		case TF_QML_DISPLAY_COMPLETE:
			hour = strtime.first(2).toInt();
			min = strtime.sliced(3, 2).toInt();
			sec = strtime.last(2).toInt();
		break;
		case TF_QML_DISPLAY_NO_SEC:
			hour = strtime.first(2).toInt();
			min = strtime.last(2).toInt();
		break;
		case TF_QML_DISPLAY_NO_HOUR:
			min = strtime.first(2).toInt();
			sec = strtime.last(2).toInt();
		break;
		case TF_FANCY:
			hour = strtime.first(2).toInt();
			min = strtime.sliced(6, 2).toInt();
		break;
		case TF_FANCY_SECS:
			hour = strtime.first(2).toInt();
			min = strtime.sliced(6, 2).toInt();
			sec = strtime.sliced(strtime.length() - 6, 2).toInt();
		break;
		case TF_ONLINE:
			hour = strtime.first(2).toInt();
			min = strtime.sliced(2, 2).toInt();
			sec = strtime.last(2).toInt();
		break;
	}
	return QTime{hour, min, sec};
}

QString TPUtils::addTimeToStrTime(const QString &strTime, const int addmins, const int addsecs) const
{
	int secs{QStringView{strTime}.sliced(3, 2).toInt()};
	int mins{QStringView{strTime}.first(2).toInt()};

	secs += addsecs;
	if (secs > 59)
	{
		secs -= 60;
		mins++;
	}
	else if (secs < 0)
	{
		secs += 60;
		mins--;
	}
	mins += addmins;
	if (mins < 0)
	{
		mins = 0;
		secs = 0;
	}
	const QString &ret{(mins <= 9 ? STR_ZERO + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? STR_ZERO + QString::number(secs) : QString::number(secs))};
	return ret;
}

QString TPUtils::getHourFromStrTime(const QString &strTime, const TIME_FORMAT format) const
{
	switch (format)
	{
		case TF_QML_DISPLAY_COMPLETE:
		case TF_QML_DISPLAY_NO_SEC:
		case TF_ONLINE:
		case TF_FANCY:
		case TF_FANCY_SECS:
			return strTime.left(2);
		break;
		case TF_QML_DISPLAY_NO_HOUR:
		break;
	}
	return QString{};
}

QString TPUtils::getMinutesFromStrTime(const QString &strTime, const TIME_FORMAT format) const
{
	switch (format)
	{
		case TF_QML_DISPLAY_COMPLETE:
			return strTime.sliced(3, 2);
		case TF_QML_DISPLAY_NO_SEC:
			return strTime.right(2);
		case TF_QML_DISPLAY_NO_HOUR:
			return strTime.left(2);
		case TF_ONLINE:
			return strTime.sliced(2, 2);
		case TF_FANCY:
			return strTime.sliced(strTime.length()-5, 2);
		case TF_FANCY_SECS:
			return strTime.sliced(4, 2);
	}
	return QString{};
}

QTime TPUtils::calculateTimeDifference(const QString &strTimeInit, const QString &strTimeFinal) const
{
	int hour{strTimeFinal.first(2).toInt() - strTimeInit.first(2).toInt()};
	int min {strTimeFinal.last(2).toInt() - strTimeInit.last(2).toInt()};

	if (min < 0)
	{
		hour--;
		min += 60;
	}
	return QTime{hour, min, 0};
}

QDateTime TPUtils::getDateTimeFromOnlineString(const QString &datetime) const
{
	const QDate &date{getDateFromDateString(datetime.right(6), DF_ONLINE)};
	const QTime &time{getTimeFromTimeString(datetime.left(6), TF_ONLINE)};
	return QDateTime{date, time};
}

QString TPUtils::makeCompositeValue(const QString &defaultValue, const uint n_fields, const QLatin1Char &chr_sep) const
{
	QString comp;
	for(uint i{0}; i < n_fields; ++i)
		comp += defaultValue + chr_sep;
	return comp;
}

QString TPUtils::makeDoubleCompositeValue(const QString &defaultValue, const uint n_fields1, const uint n_fields2,
												const QLatin1Char &chr_sep1, const QLatin1Char &chr_sep2) const
{
	QString comp1{std::move(makeCompositeValue(defaultValue, n_fields1, chr_sep1))};
	return makeCompositeValue(comp1, n_fields2, chr_sep2);
}

QString TPUtils::getCompositeValue(const uint idx, const QString &compositeString, const QLatin1Char &chr_sep) const
{
	int n_seps{-1};
	int chr_pos{0};
	uint last_sep_pos{0};

	for (const auto &chr : compositeString)
	{
		if (chr.toLatin1() == chr_sep)
		{
			if (++n_seps == idx)
				return compositeString.sliced(last_sep_pos, chr_pos);
			last_sep_pos += chr_pos + 1;
			chr_pos = -1;
		}
		++chr_pos;
	}
	return idx == 0 ? compositeString : QString{};
}

QString TPUtils::lastValueInComposite(const QString &compositeString, const QLatin1Char &chr_sep) const
{
	const int last_field{static_cast<int>(nFieldsInCompositeString(compositeString, chr_sep) - 1)};
	if (last_field >= 0)
		return getCompositeValue(last_field, compositeString, chr_sep);
	else
		return compositeString;
}

void TPUtils::setCompositeValue(const uint idx, const QString &newValue, QString &compositeString, const QLatin1Char &chr_sep) const
{
	qsizetype sep_pos{compositeString.indexOf(chr_sep)};
	qsizetype n_seps{-1};

	if (sep_pos == -1)
	{
		if (idx == 0)
			compositeString = newValue;
		else
		{
			while (++n_seps < idx)
				compositeString += chr_sep;
			compositeString += newValue + chr_sep;
		}
		return;
	}

	qsizetype last_sep_pos{0};
	do {
		++n_seps;
		if (idx == n_seps)
		{
			compositeString.remove(last_sep_pos, sep_pos - last_sep_pos);
			compositeString.insert(last_sep_pos, newValue);
			return;
		}
		last_sep_pos = sep_pos + 1;
		sep_pos = compositeString.indexOf(chr_sep, last_sep_pos);
	} while(sep_pos != -1);
	while (++n_seps < idx)
		compositeString += chr_sep;
	compositeString += newValue + chr_sep;
}

void TPUtils::removeFieldFromCompositeValue(const uint idx, QString &compositeString, const QLatin1Char &chr_sep) const
{
	qsizetype sep_pos{compositeString.indexOf(chr_sep)};
	qsizetype n_seps{-1}, del_pos_1{0}, del_pos_2{-1};
	do {
		++n_seps;
		if (n_seps == idx)
		{
			del_pos_2 = sep_pos;
			break;
		}
		del_pos_1 = sep_pos + 1;
		sep_pos = compositeString.indexOf(chr_sep, sep_pos + 1);
	} while(sep_pos != -1);
	compositeString.remove(del_pos_1, del_pos_2 - del_pos_1 + 1);
}

int TPUtils::fieldOfValue(const QString &value, const QString &compositeString, const QLatin1Char &chr_sep) const
{
	qsizetype sep_pos{compositeString.indexOf(chr_sep)};
	if (sep_pos != -1)
	{
		qsizetype value_start{sep_pos+1};
		qsizetype value_end{compositeString.indexOf(chr_sep, value_start)};
		if (value_end != -1 && value_end != value_start)
		{
			int idx{0};
			do {
				const QString &word{compositeString.sliced(value_start, value_end-value_start)};
				if (word == value)
					return idx;
				sep_pos = compositeString.indexOf(chr_sep, value_end+1);
				if (sep_pos != -1)
				{
					++idx;
					value_start = sep_pos+1;
					value_end = compositeString.indexOf(chr_sep, value_start);
				}
			} while (sep_pos != -1);
		}
	}
	return -1;
}

QString TPUtils::subSetOfCompositeValue(const QString &value, const uint from, const uint n, const QLatin1Char &chr_sep) const
{
	int n_seps{-1};
	int chr_pos{0};
	uint last_sep_pos{0};
	QString ret;

	for (const auto &chr : value)
	{
		if (chr.toLatin1() == chr_sep)
		{
			if (n_seps <= from)
				ret += value.sliced(last_sep_pos, chr_pos + 1);
			if (++n_seps >= n)
				break;
			last_sep_pos += chr_pos + 1;
		}
		++chr_pos;
	}
	return ret.isEmpty() ? value + chr_sep : ret;
}

bool TPUtils::stringsAreSimiliar(const QString &string1, const QString &string2) const
{
	const QStringList& words2{string2.split(' ')};
	QStringList::const_iterator itr{words2.begin()};
	const QStringList::const_iterator& itr_end{words2.end()};
	uint matches{0};
	uint nwords{0};
	do
	{
		nwords++;
		if (string1.contains(*itr))
			matches++;
	} while (++itr != itr_end);
	if (matches > 0)
		return (nwords/matches >= 0.8);
	return false;
}

QString TPUtils::stripDiacriticsFromString(const QString &src) const
{
	QString filtered;
    for (qsizetype i{0}; i < src.length(); ++i)
	{
		if (src.at(i).decompositionTag() != QChar::NoDecomposition)
			filtered.push_back(src.at(i).decomposition().at(0));
		else
			filtered.push_back(src.at(i));
	}
	return filtered;
}

QString TPUtils::setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const
{
	strValue.replace('.', ',');
	strValue.replace('-', ""_L1);
	strValue.replace('E', ""_L1);
	strValue = strValue.trimmed();
	const char rightmostDigit{!strValue.isEmpty() ? strValue.at(strValue.length()-1).toLatin1() : '0'};

	float result{m_appLocale->toFloat(strValue)};
	switch (settype)
		case 0: //SetInputField.Type.WeightType
		{
			if (strValue.contains('.') || strValue.contains(','))
			{
				if (bIncrease)
					result += rightmostDigit == '5' ? 2.5 : 5.0;
				else
					result -= rightmostDigit == '5' ? 2.5 : 5.0;
			}
			else
			{
				if (result < 40)
				{
					switch (rightmostDigit)
					{
						case '0':
						case '2':
						case '6':
						case '8':
							bIncrease ? result += 2 : result -= 2;
						break;
						case '1':
						case '3':
						case '4':
						case '5':
						case '7':
						case '9':
							bIncrease ? ++result : --result;
						break;
					}
				}
				else
				{
					int paddingValue{0};
					switch (rightmostDigit)
					{
						case '0':
							bIncrease ? paddingValue = 5 : paddingValue = -5; break;
						case '1':
						case '6':
							bIncrease ? paddingValue = 4 : paddingValue = -1; break;
						case '2':
						case '7':
							bIncrease ? paddingValue = 3 : paddingValue = -2; break;
						case '3':
						case '8':
							bIncrease ? paddingValue = 2 : paddingValue = -3; break;
						case '4':
						case '9':
							bIncrease ? paddingValue = 1 : paddingValue = -4; break;
						case '5':
							bIncrease ? paddingValue = 5 : paddingValue = -5; break;
					}
					result += paddingValue;
				}
			}
			if (result > 999.99)
				result = 999.99;
			else if (result < 0)
				result = 0;

			strValue = std::move(QString::number(result, 'f', 2));
			if (strValue.last(2) != "50"_L1)
				strValue.chop(3);
			return strValue;
		break;

		case 1: //SetInputField.Type.RepType
			if (strValue.contains('.') || strValue.contains(','))
				bIncrease ? result += 0.5 : result -= 0.5;
			else
				bIncrease ? ++result : --result;

			if (result > 100)
				result = 100;
			else if (result < 0)
				result = 0;
			return QString::number(static_cast<uint>(result));
		break;

		case 2: //SetInputField.Type.TimeType
		{
			if (bIncrease)
			{
				result = strValue.last(2).toUInt();
				if (result >= 55)
				{
					++result;
					if (result >= 60)
						result = 0;
				}
				else
					result += 5;
				strValue = std::move(strValue.first(3) + (result < 10 ? "0"_L1 : ""_L1) + QString::number(static_cast<uint>(result)));
			}
			else
			{
				result = strValue.first(2).toUInt();
				if (result > 55)
					--result;
				else if (result <= 5)
					--result;
				else
					result -= 5;
				if (result < 0)
					result = 0;
				strValue = std::move((result < 10 ? "0"_L1 : ""_L1) + QString::number(static_cast<uint>(result)) + strValue.last(3));
			}
			return strValue;
		}
		break;

		case 3: //SetInputField.Type.SetType
			if (bIncrease)
			{
				++result;
				if (result > 9)
					result = 9;
			}
			else
			{
				--result;
				if (result < 0)
					result = 0;
			}
			return QString::number(static_cast<uint>(result));
		break;

		default: return QString{};
	}
}

void TPUtils::setAppLocale(const QString &localeStr, const bool bWriteConfig)
{
	if (m_appLocale)
		delete m_appLocale;

	const QString &strLanguage{localeStr.first(2)};
	const QString &strTerritory{localeStr.last(2)};
	QLocale::Language language;
	QLocale::Territory territory;

	if (strLanguage == "pt"_L1)
		language = QLocale::Portuguese;
	else if (strLanguage == "de"_L1)
		language = QLocale::German;
	else
		language = QLocale::English;

	if (strTerritory == "BR"_L1)
		territory = QLocale::Brazil;
	else if (strTerritory == "DE"_L1)
		territory = QLocale::Germany;
	else
		territory = QLocale::UnitedStates;

	m_strLocale = localeStr;
	m_appLocale = new QLocale{language, territory};
	m_appLocale->setNumberOptions(QLocale::IncludeTrailingZeroesAfterDot);
	if (bWriteConfig)
		appSettings()->setAppLocale(localeStr);
}

