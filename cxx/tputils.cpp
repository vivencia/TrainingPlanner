#include "tputils.h"
#include "tpglobals.h"
#include "tpsettings.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>

TPUtils* TPUtils::app_utils(nullptr);

TPUtils::TPUtils(QObject* parent)
	: QObject{parent}, m_appLocale(nullptr), mb_appSuspended(false)
{
	app_utils = this;
	connect(qApp, &QGuiApplication::applicationStateChanged, this, [&] (Qt::ApplicationState state) {
		if (state == Qt::ApplicationSuspended)
		{
			mb_appSuspended = true;
			emit appSuspended();
		}
		else if (state == Qt::ApplicationActive)
		{
			if (mb_appSuspended)
			{
				emit appResumed();
				mb_appSuspended = false;
			}
		}
	});
}

const QString TPUtils::getCorrectPath(const QUrl& url) const
{
	QString path(url.toString(QUrl::PrettyDecoded|QUrl::PreferLocalFile|QUrl::RemoveScheme));
	if (path.startsWith(u"file://"_s))
		path.remove(0, 7);
	return path;
}

int TPUtils::getFileType(const QString& filename) const
{
	#ifdef Q_OS_ANDROID
		return filename.contains(u"video%"_s) ? 1 : (filename.contains(u"image%"_s) ? 0 : -1);
	#else
		if (filename.endsWith(u".mp4"_s) || filename.endsWith(u".mkv"_s) || filename.endsWith(u".mov"_s))
			return 1;
		else return (filename.endsWith(u".png"_s) || filename.endsWith(u".jpg"_s)) ? 0 : -1;
	#endif
}

void TPUtils::copyToClipBoard(const QString& text) const
{
	qApp->clipboard()->setText(text);
}

bool TPUtils::canReadFile(const QString& filename) const
{
	const QFileInfo file(filename);
	if (file.isFile())
		return file.isReadable();
	return false;
}

QString TPUtils::formatDate(const QDate& date) const
{
	return m_appLocale->toString(date, u"ddd d/M/yyyy"_s);
}

QString TPUtils::formatTodayDate() const
{
	const QDate& today(QDate::currentDate());
	return m_appLocale->toString(today, u"ddd d/M/yyyy"_s);
}

QDate TPUtils::getDateFromStrDate(const QString& strDate) const
{
	const QStringView& strdate(strDate);
	//if (appLocale->name() == u"pt_BR"_s)
	//{
		const int spaceIdx(strdate.indexOf(' '));
		const int fSlashIdx(strdate.indexOf('/'));
		const int fSlashIdx2 = strdate.indexOf('/', fSlashIdx+1);
		const uint day(strdate.sliced(spaceIdx+1, fSlashIdx-spaceIdx-1).toUInt());
		const uint month(strdate.sliced(fSlashIdx+1, fSlashIdx2-fSlashIdx-1).toUInt());
		const uint year(strdate.last(4).toUInt());
		const QDate date(year, month, day);
		return date;
	/*}
	else
	{
		static const QString months[12] = {u"Jan"_s,u"Feb"_s,u"Mar"_s,u"Apr"_s,u"May"_s,
		u"Jun"_s,u"Jul"_s,u"Aug"_s,u"Sep"_s,u"Oct"_s,u"Nov"_s,u"Dez"_s };
		const QStringView strMonth(strdate.sliced(4, 3));
		uint i(0);
		for(; i < 12; ++ i)
		{
			if (months[i] == strMonth) break;
		}
		const uint month(i);
		const uint year(strdate.last(4).toUInt());

		const int spaceIdx(strdate.indexOf(' '));
		const int spaceIdx2(strdate.indexOf(' ', spaceIdx+1));
		const uint day(strdate.sliced(spaceIdx+1, spaceIdx2-spaceIdx-1).toUInt());
		const QDate date(year, month, day);
		return date;
	}*/
}

uint TPUtils::calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const
{
	uint n(0);
	const uint week1(date1.weekNumber());
	const uint week2(date2.weekNumber());
	//Every 6 years we have a 53 week year
	if (week2 < week1)
	{
		const uint totalWeeksInYear (QDate::currentDate().year() != 2026 ? 52 : 53);
		n = (totalWeeksInYear - week1) + week2;
	}
	else
		n = week2 - week1;
	return n+1; //+1 include current week
}

QDate TPUtils::getMesoStartDate(const QDate& lastMesoEndDate) const
{
	const uint daysToNextMonday[7] = { 7, 6, 5, 4, 3, 2, 1 };
	const QDate& date(lastMesoEndDate);
	return date.addDays(daysToNextMonday[date.dayOfWeek()-1]);
}

QDate TPUtils::createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const
{
	QDate newDate(date);
	if (days > 0)
		newDate = newDate.addDays(days);
	if (months > 0)
		newDate = newDate.addMonths(months);
	if (years > 0)
		newDate = newDate.addYears(years);
	//qDebug() << "createFutureDate: in " << date.toString("d 'de' MMMM 'de' yyyy") << "  out  " << newDate.toString("d 'de' MMMM 'de' yyyy");
	return newDate;
}

QString TPUtils::addTimeToStrTime(const QString& strTime, const int addmins, const int addsecs) const
{
	int secs(QStringView{strTime}.sliced(3, 2).toUInt());
	int mins(QStringView{strTime}.first(2).toUInt());

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
	const QString& ret((mins <= 9 ? STR_ZERO + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? STR_ZERO + QString::number(secs) : QString::number(secs)));
	return ret;
}

QString TPUtils::addToTime(const QString& origTime, const uint hours, const uint mins) const
{
	const QTime time(origTime.first(2).toUInt(), origTime.last(2).toUInt());
	return addToTime(time, hours, mins);
}

QString TPUtils::getHourOrMinutesFromStrTime(const QString& strTime) const
{
	const int idx(strTime.indexOf(':'));
	return idx > 1 ? strTime.first(idx) : QString();
}

QString TPUtils::getMinutesOrSeconsFromStrTime(const QString& strTime) const
{
	const int idx(strTime.indexOf(':'));
	return idx > 1 ? strTime.sliced(idx+1) : QString();
}

QString TPUtils::calculateTimeDifference_str(const QString& strTimeInit, const QString& strTimeFinal) const
{
	const QTime& time(calculateTimeDifference(strTimeInit, strTimeFinal));
	return time.toString(u"hh:mm:ss");
}

QTime TPUtils::calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal) const
{
	int hour(strTimeFinal.first(2).toInt() - strTimeInit.first(2).toInt());
	int min (strTimeFinal.last(2).toInt() - strTimeInit.last(2).toInt());

	if (min < 0)
	{
		hour--;
		min += 60;
	}
	return QTime(hour, min, 0);
}

QString TPUtils::getCompositeValue(const uint idx, const QString& compositeString, const QLatin1Char& chr_sep) const
{
	QString::const_iterator itr(compositeString.constBegin());
	const QString::const_iterator& itr_end(compositeString.constEnd());
	uint n_seps(0);
	int chr_pos(0);
	uint last_sep_pos(0);

	while (itr != itr_end)
	{
		if ((*itr).toLatin1() == chr_sep)
		{
			if (n_seps == idx)
				return compositeString.sliced(last_sep_pos, chr_pos);
			++n_seps;
			last_sep_pos += chr_pos + 1;
			chr_pos = -1;
		}
		++chr_pos;
		++itr;
	}
	return compositeString.sliced(last_sep_pos, chr_pos);
}

void TPUtils::setCompositeValue(const uint idx, const QString& newValue, QString& compositeString, const QLatin1Char& chr_sep) const
{
	int sep_pos(compositeString.indexOf(chr_sep));
	int n_seps(-1);

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

	uint last_sep_pos(0);
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

bool TPUtils::stringsAreSimiliar(const QString& string1, const QString& string2) const
{
	const QStringList& words2(string2.split(' '));
	QStringList::const_iterator itr(words2.begin());
	const QStringList::const_iterator& itr_end(words2.end());
	uint matches(0);
	uint nwords(0);
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

QString TPUtils::setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const
{
	strValue.replace('.', ',');
	strValue.replace('-', u""_s);
	strValue.replace('E', u""_s);
	strValue = strValue.trimmed();
	const char rightmostDigit(!strValue.isEmpty() ? strValue.at(strValue.length()-1).toLatin1() : '0');

	float result(m_appLocale->toFloat(strValue));
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
					int paddingValue(0);
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

			strValue = QString::number(result, 'f', 2);
			if (strValue.last(2) != u"50"_s)
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
				strValue = strValue.first(3) + (result < 10 ? u"0"_s : u""_s) + QString::number(static_cast<uint>(result));
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
				strValue = (result < 10 ? u"0"_s : u""_s) + QString::number(static_cast<uint>(result)) + strValue.last(3);
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

		default: return QString();
	}
}

void TPUtils::setAppLocale(const QString& localeStr, const bool bWriteConfig)
{
	if (m_appLocale)
		delete m_appLocale;

	const QString& strLanguage(localeStr.first(2));
	const QString& strTerritory(localeStr.last(2));
	QLocale::Language language;
	QLocale::Territory territory;

	if (strLanguage == u"pt"_s)
		language = QLocale::Portuguese;
	else if (strLanguage == u"de"_s)
		language = QLocale::German;
	else
		language = QLocale::English;

	if (strTerritory == u"BR"_s)
		territory = QLocale::Brazil;
	else if (strTerritory == u"DE"_s)
		territory = QLocale::Germany;
	else
		territory = QLocale::UnitedStates;

	m_strLocale = localeStr;
	m_appLocale = new QLocale(language, territory);
	m_appLocale->setNumberOptions(QLocale::IncludeTrailingZeroesAfterDot);
	if (bWriteConfig)
		appSettings()->setAppLocale(localeStr);
}
