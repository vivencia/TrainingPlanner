#include "runcommands.h"

#include <QSettings>
#include <QLocale>
#include <QClipboard>
#include <QGuiApplication>

RunCommands* RunCommands::app_runcmd(nullptr);

RunCommands::RunCommands( QSettings* settings, QObject *parent )
	: QObject(parent), m_appSettings(settings), m_appLocale(nullptr), mb_appSuspended(false)
{
	app_runcmd = this;
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

const QString RunCommands::getCorrectPath(const QUrl& url)
{
	#ifdef DEBUG
	qDebug() << "input url:  " << url;
	qDebug() << "output string:  " << url.toString(QUrl::PreferLocalFile);
	#endif
	//#ifdef Q_OS_ANDROID
	return url.toString(QUrl::PreferLocalFile);
	//#else
	//return url.toString();
	//#endif
}

int RunCommands::getFileType( const QString& filename )
{
	#ifdef Q_OS_ANDROID
		if ( filename.contains(QStringLiteral("video%"), Qt::CaseInsensitive))
			return 1;
		else if ( filename.contains(QStringLiteral("image%"), Qt::CaseInsensitive))
			return 0;
		else return -1;
	#else
		if ( filename.endsWith(QStringLiteral(".mp4"), Qt::CaseInsensitive) ||
			 filename.endsWith(QStringLiteral(".mkv"), Qt::CaseInsensitive) ||
			 filename.endsWith(QStringLiteral(".mov"), Qt::CaseInsensitive) )
			return 1;
		else if ( filename.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
				 filename.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) )
			return 0;
		else
			return -1;
	#endif
}

QString RunCommands::getAppDir(const QString& dbFile)
{
	if (!dbFile.isEmpty())
	{
		const int idx (dbFile.indexOf(QStringLiteral("Planner")));
		if (idx > 1)
			m_appPrivateDir = dbFile.left(dbFile.indexOf('/', idx + 1) + 1);
	}
	return m_appPrivateDir;
}

void RunCommands::copyToClipBoard(const QString& text) const
{
	qApp->clipboard()->setText(text);
}

QString RunCommands::formatDate(const QDate& date) const
{
	return m_appLocale->toString(date, u"ddd d/M/yyyy"_qs);
}

QString RunCommands::formatTodayDate() const
{
	const QDate today(QDate::currentDate());
	return m_appLocale->toString(today, u"ddd d/M/yyyy"_qs);
}

QDate RunCommands::getDateFromStrDate(const QString& strDate) const
{
	const QStringView strdate(strDate);
	//if (appLocale->name() == u"pt_BR"_qs)
	//{
		const int spaceIdx(strdate.indexOf(' '));
		const int fSlashIdx(strdate.indexOf('/'));
		const int fSlashIdx2 = strdate.indexOf('/', fSlashIdx+1);
		const uint day(strdate.mid(spaceIdx+1, fSlashIdx-spaceIdx-1).toUInt());
		const uint month(strdate.mid(fSlashIdx+1, fSlashIdx2-fSlashIdx-1).toUInt());
		const uint year(strdate.right(4).toUInt());
		const QDate date(year, month, day);
		return date;
	/*}
	else
	{
		static const QString months[12] = {u"Jan"_qs,u"Feb"_qs,u"Mar"_qs,u"Apr"_qs,u"May"_qs,
		u"Jun"_qs,u"Jul"_qs,u"Aug"_qs,u"Sep"_qs,u"Oct"_qs,u"Nov"_qs,u"Dez"_qs };
		const QStringView strMonth(strdate.mid(4, 3));
		uint i(0);
		for(; i < 12; ++ i)
		{
			if (months[i] == strMonth) break;
		}
		const uint month(i);
		const uint year(strdate.right(4).toUInt());

		const int spaceIdx(strdate.indexOf(' '));
		const int spaceIdx2(strdate.indexOf(' ', spaceIdx+1));
		const uint day(strdate.mid(spaceIdx+1, spaceIdx2-spaceIdx-1).toUInt());
		const QDate date(year, month, day);
		return date;
	}*/
}

uint RunCommands::calculateNumberOfWeeks(const QDate& date1, const QDate& date2) const
{
	uint n(0);
	const uint week1(date1.weekNumber());
	const uint week2(date2.weekNumber());
	//Every 6 years we have a 53 week year
	if ( week2 < week1 ) {
		const uint totalWeeksInYear (QDate::currentDate().year() != 2026 ? 52 : 53);
		n = (totalWeeksInYear - week1) + week2;
	}
	else
		n = week2 - week1;
	return n+1; //+1 include current week
}

QDate RunCommands::getMesoStartDate(const QDate& lastMesoEndDate) const
{
	const uint daysToNextMonday[7] = { 7, 6, 5, 4, 3, 2, 1 };
	const QDate date (lastMesoEndDate);
	return date.addDays(daysToNextMonday[date.dayOfWeek()-1]);
}

QDate RunCommands::createFutureDate(const QDate& date, const uint years, const uint months, const uint days) const
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

QString RunCommands::addTimeToStrTime(const QString& strTime, const int addmins, const int addsecs) const
{
	int secs(QStringView{strTime}.mid(3, 2).toUInt());
	int mins(QStringView{strTime}.left(2).toUInt());

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
	QString ret(mins <=9 ? QChar('0') + QString::number(mins) : QString::number(mins));
	ret += QChar(':') + (secs <=9 ? QChar('0') + QString::number(secs) : QString::number(secs));
	return ret;
}

QString RunCommands::formatFutureTime(const QDateTime& addTime) const
{
	const QTime time(addTime.time());
	return addToTime(QTime::currentTime(), time.hour(), time.minute());
}

QString RunCommands::addToTime(const QString& origTime, const uint hours, const uint mins) const
{
	const QTime time(origTime.left(2).toUInt(), origTime.right(2).toUInt());
	return addToTime(time, hours, mins);
}

QString RunCommands::getHourOrMinutesFromStrTime(const QString& strTime) const
{
	const int idx(strTime.indexOf(':'));
	return idx > 1 ? strTime.left(idx) : QString();
}

QString RunCommands::getMinutesOrSeconsFromStrTime(const QString& strTime) const
{
	const int idx(strTime.indexOf(':'));
	return idx > 1 ? strTime.mid(idx+1) : QString();
}

QString RunCommands::calculateTimeDifference_str(const QString& strTimeInit, const QString& strTimeFinal) const
{
	const QTime time(calculateTimeDifference(strTimeInit, strTimeFinal));
	return time.toString(u"hh:mm:ss");
}

const QTime RunCommands::calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal) const
{
	int hour(strTimeFinal.left(2).toInt() - strTimeInit.left(2).toInt());
	int min (strTimeFinal.right(2).toInt() - strTimeInit.right(2).toInt());

	if (min < 0)
	{
		hour--;
		min += 60;
	}
	return QTime(hour, min, 0);
}

QString RunCommands::getCompositeValue(const uint idx, const QString& compositeString) const
{
	QString::const_iterator itr(compositeString.constBegin());
	const QString::const_iterator itr_end(compositeString.constEnd());
	uint n_seps(0);
	int chr_pos(0);
	uint last_sep_pos(0);

	while (itr != itr_end)
	{
		if ((*itr).toLatin1() == char(31))
		{
			if (n_seps == idx)
				return compositeString.mid(last_sep_pos, chr_pos);
			++n_seps;
			last_sep_pos += chr_pos + 1;
			chr_pos = -1;
		}
		++chr_pos;
		++itr;
	}
	return compositeString.mid(last_sep_pos, chr_pos);
}

QString RunCommands::setCompositeValue(const uint idx, const QString newValue, QString compositeString) const
{
	static const QLatin1Char subrecord_separator(31);
	int sep_pos(compositeString.indexOf(subrecord_separator));
	int n_seps(-1);

	if (sep_pos == -1)
	{
		if (idx == 0)
			return compositeString = newValue;
		else
		{
			while (++n_seps < idx)
				compositeString += subrecord_separator;
			return compositeString += newValue + subrecord_separator;
		}
	}

	uint last_sep_pos(0);
	do {
		++n_seps;
		if (idx == n_seps)
		{
			compositeString.remove(last_sep_pos, sep_pos - last_sep_pos);
			compositeString.insert(last_sep_pos, newValue);
			return compositeString;
		}
		last_sep_pos = sep_pos + 1;
		sep_pos = compositeString.indexOf(subrecord_separator, last_sep_pos);
	} while(sep_pos != -1);
	while (++n_seps < idx)
		compositeString += subrecord_separator;
	return compositeString += newValue + subrecord_separator;
}

bool RunCommands::stringsAreSimiliar(const QString& string1, const QString& string2) const
{
	const QStringList words2(string2.split(' '));
	QStringList::const_iterator itr(words2.begin());
	const QStringList::const_iterator itr_end(words2.end());
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

QString RunCommands::setTypeOperation(const uint settype, const bool bIncrease, QString strValue) const
{
	strValue.replace('.', ',');
	strValue.replace('-', u""_qs);
	strValue.replace('E', u""_qs);
	strValue = strValue.trimmed();

	float result(m_appLocale->toFloat(strValue));
	switch (settype)
		case 0: //SetInputField.Type.WeightType
		{
			if (strValue.contains('.') || strValue.contains(','))
			{
				if (bIncrease)
					result += strValue.endsWith('5') ? 2.5 : 5.0;
				else
					result -= strValue.endsWith('5') ? 2.5 : 5.0;
			}
			else
			{
				if (result < 40)
				{
					if (bIncrease)
					{
						if (strValue.endsWith('4') || strValue.endsWith('9'))
							++result;
						else
							result += 2;
					}
					else
					{
						if (strValue.endsWith('6') || strValue.endsWith('1'))
							--result;
						else
							result -= 2;
					}
				}
				else
				{
					if (bIncrease)
						result += 5;
					else
						result -= 5;
				}
			}
			if (bIncrease)
			{
				if (result > 999.99)
					result = 999.99;
			}
			else
			{
				if (result < 0)
					result = 0;
			}
			strValue = QString::number(result, 'f', 2);
			if (strValue.right(2) != u"50"_qs)
				strValue.chop(3);
			return strValue;
		break;

		case 1: //SetInputField.Type.RepType
			if (strValue.contains('.') || strValue.contains(','))
			{
				if (bIncrease)
					result += 0.5;
				else
					result -= 0.5;
			}
			else
			{
				if (bIncrease)
					++result;
				else
					--result;
			}
			if (bIncrease)
			{
				if (result > 100)
					result = 100;
			}
			else
			{
				if (result < 0)
					result = 0;
			}
			return QString::number(static_cast<uint>(result));
		break;

		case 2: //SetInputField.Type.TimeType
		{
			if (bIncrease)
			{
				result = strValue.right(2).toUInt();
				if (result >= 55)
				{
					++result;
					if (result >= 60)
						result = 0;
				}
				else
					result += 5;
				strValue = strValue.left(3) + (result < 10 ? u"0"_qs : u""_qs) + QString::number(static_cast<uint>(result));
			}
			else
			{
				result = strValue.left(2).toUInt();
				if (result > 55)
					--result;
				else if (result <= 5)
					--result;
				else
					result -= 5;
				if (result < 0)
					result = 0;
				strValue = (result < 10 ? u"0"_qs : u""_qs) + QString::number(static_cast<uint>(result)) + strValue.right(3);
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

void RunCommands::setAppLocale(const QString& localeStr, const bool bChangeConfig)
{
	if (m_appLocale)
		delete m_appLocale;

	const QString strLanguage(localeStr.left(2));
	const QString strTerritory(localeStr.right(2));
	QLocale::Language language;
	QLocale::Territory territory;

	if (strLanguage == u"pt"_qs)
		language = QLocale::Portuguese;
	else if (strLanguage == u"de"_qs)
		language = QLocale::German;
	else
		language = QLocale::English;

	if (strTerritory == u"BR"_qs)
		territory = QLocale::Brazil;
	else if (strTerritory == u"DE"_qs)
		territory = QLocale::Germany;
	else
		territory = QLocale::UnitedStates;

	m_appLocale = new QLocale(language, territory);
	m_appLocale->setNumberOptions(QLocale::IncludeTrailingZeroesAfterDot);
}

void RunCommands::populateSettingsWithDefaultValue()
{
	if (m_appSettings->childKeys().isEmpty() || m_appSettings->value("appLocale").toString().isEmpty())
	{
		const QString localeStr(QLocale::system().name());
		setAppLocale(localeStr);
		m_appSettings->setValue("appLocale", localeStr);
		m_appSettings->setValue("appVersion", TP_APP_VERSION);
		m_appSettings->setValue("weightUnit", u"(kg)"_qs);
		m_appSettings->setValue("themeStyle", u"Material"_qs);
		m_appSettings->setValue("colorScheme", u"Blue"_qs);
		m_appSettings->setValue("primaryDarkColor", u"#1976D2"_qs);
		m_appSettings->setValue("primaryColor", u"#25b5f3"_qs);
		m_appSettings->setValue("primaryLightColor", u"#BBDEFB"_qs);
		m_appSettings->setValue("paneBackgroundColor", u"#1976d2"_qs);
		m_appSettings->setValue("entrySelectedColor", u"#6495ed"_qs);
		m_appSettings->setValue("exercisesListVersion", u"0"_qs);
		m_appSettings->setValue("backupFolder", u""_qs);
		m_appSettings->setValue("fontColor", u"white"_qs);
		m_appSettings->setValue("disabledFontColor", u"lightgray"_qs);
		m_appSettings->setValue("iconFolder", u"white/"_qs);
		m_appSettings->setValue("fontSize", 12);
		m_appSettings->setValue("fontSizeLists", 8);
		m_appSettings->setValue("fontSizeText", 10);
		m_appSettings->setValue("fontSizeTitle", 18);
		m_appSettings->setValue("lastViewedMesoId", 0);
		m_appSettings->setValue("alwaysAskConfirmation", true);
		m_appSettings->setValue("firstTime", true);
		m_appSettings->sync();
	}
	else
		setAppLocale(m_appSettings->value("appLocale").toString());
}
