#include "runcommands.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QLocale>

const QString RunCommands::getCorrectPath(const QUrl& url)
{
	#ifdef DEBUG
	qDebug() << "input url:  " << url;
	qDebug() << "output string:  " << url.toString(QUrl::PreferLocalFile);
	#endif
	#ifdef Q_OS_ANDROID
	return url.toString(QUrl::PreferLocalFile);
	#else
	return url.toString();
	#endif
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

QString RunCommands::searchForDatabaseFile( const QString& baseDir)
{
	QDir root (baseDir);
	root.setFilter(QDir::AllEntries);
	QFileInfoList list = root.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		if (list.at(i).fileName() == "." || list.at(i).fileName() == "..")
			continue;
		if (list.at(i).isDir())
		{
			return searchForDatabaseFile(list.at(i).filePath());
		}
		else
		{
			if (list.at(i).fileName().endsWith(QStringLiteral(".sqlite")))
			{
				m_dbFileName = list.at(i).filePath();
				break;
			}
		}
	}
	return m_dbFileName;
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

const QString RunCommands::formatDate(const QDate& date) const
{
	if (date.isValid())
	{
		if (m_appSettings->value("appLocale").toString() == QStringLiteral("pt_BR"))
		{
			QLocale locale(QStringLiteral("pt_BR"));
			return locale.toString(date, QStringLiteral("ddd d/M/yyyy"));
		}
		return date.toString(Qt::TextDate);
	}
	return QString();
}

QDate RunCommands::getDateFromStrDate(const QString& strDate) const
{
	const QStringView strdate(strDate);
	if (m_appSettings->value("appLocale").toString() == QStringLiteral("pt_BR"))
	{
		const int spaceIdx(strdate.indexOf(' '));
		const int fSlashIdx(strdate.indexOf('/'));
		const int fSlashIdx2 = strdate.indexOf('/', fSlashIdx+1);
		const uint day(strdate.mid(spaceIdx+1, fSlashIdx-spaceIdx-1).toUInt());
		const uint month(strdate.mid(fSlashIdx+1, fSlashIdx2-fSlashIdx-1).toUInt());
		const uint year(strdate.right(4).toUInt());
		const QDate date(year, month, day);
		return date;
	}
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
	}
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
	return date.addDays(daysToNextMonday[date.dayOfWeek()]);
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

QDate RunCommands::getDayBefore(const QDate& date) const
{
	return date.addDays(-1);
}

QString RunCommands::intTimeToStrTime(const uint time) const
{
	QString ret(QString::number(time));
	if (time < 10)
		ret.prepend('0');
	return ret;
}

QString RunCommands::getStrHourFromTime(const QDateTime& time) const
{
	const int hour(time.time().hour());
	QString ret(QString::number(hour));
	if (hour < 10)
		ret.prepend('0');
	return ret;
}

QString RunCommands::getStrMinFromTime(const QDateTime& time) const
{
	const int min(time.time().minute());
	QString ret(QString::number(min));
	if (min < 10)
		ret.prepend('0');
	return ret;
}

QString RunCommands::formatTime(const QDateTime& time) const
{
	return time.toString(QStringLiteral("hh:mm"));
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

QString RunCommands::formatFutureTime(const QDateTime& time, const uint hours, const uint mins) const
{
	QDateTime newTime(time.addSecs(mins*60 + hours*3600));
	return newTime.toString(QStringLiteral("hh:mm"));
}

QString RunCommands::formatFutureTime(const QDateTime& time, const QDateTime& addTime) const
{
	QTime addtime(addTime.time());
	QTime newTime(time.addSecs(addtime.minute()*60 + addtime.hour()*3600).time());
	return newTime.toString(QStringLiteral("hh:mm"));
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

QDateTime RunCommands::calculateTimeBetweenTimes(const QString& strTime1, const QString& strTime2) const
{
	const QTime time1(QTime::fromString(strTime1, QStringLiteral("hh:mm")));
	QTime time2(QTime::fromString(strTime2, QStringLiteral("hh:mm")));

	int hour(time2.hour() - time1.hour());
	int min (time2.minute() - time1.minute());
	if (min < 0) {
		hour--;
		min += 60;
	}

	time2.setHMS(hour, min, 0);
	return QDateTime(QDate(), time2);
}
