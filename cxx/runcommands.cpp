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
			return locale.toString(date, QStringLiteral("ddd d/M/yy"));
		}
		return date.toString(Qt::TextDate);
	}
	return QString();
}

uint RunCommands::calculateNumberOfWeeks(const uint week1, const uint week2) const
{
	uint n(0);
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

QString RunCommands::getStrHourFromTime(const QTime& time) const
{
	const int hour(time.hour());
	QString ret(QString::number(hour));
	if (hour < 10)
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

QString RunCommands::getStrMinFromTime(const QTime& time) const
{
	const int min(time.minute());
	QString ret(QString::number(min));
	if (min < 10)
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

QString RunCommands::formatFutureTime(const QDateTime& time, const uint hours, const uint mins) const
{
	QDateTime newTime(time.addSecs(mins*60 + hours*3600));
	return newTime.toString(QStringLiteral("hh:mm"));
}

QString RunCommands::formatFutureTime(const QDateTime& time, const QTime& addTime) const
{
	QDateTime newTime(time.addSecs(addTime.minute()*60 + addTime.hour()*3600));
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

QTime RunCommands::calculateTimeBetweenTimes(const QString& strTime1, const QString& strTime2) const
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
	return time2;
}
