#include "runcommands.h"

#include <QSettings>
#include <QLocale>
#include <QClipboard>
#include <QGuiApplication>

RunCommands::RunCommands( QSettings* settings, QObject *parent )
	: QObject(parent), m_appSettings(settings), m_workoutTimer(nullptr), mb_appSuspended(false)
{
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

const QString RunCommands::formatDate(const QDate& date) const
{
	if (date.isValid())
	{
		if (m_appSettings->value("appLocale").toString() == u"pt_BR"_qs)
		{
			QLocale locale(QStringLiteral("pt_BR"));
			return locale.toString(date, u"ddd d/M/yyyy"_qs);
		}
		return date.toString(Qt::TextDate);
	}
	return QString();
}

const QString RunCommands::formatTodayDate() const
{
	const QDate today(QDate::currentDate());
	if (m_appSettings->value("appLocale").toString() == u"pt_BR"_qs)
	{
		QLocale locale(QStringLiteral("pt_BR"));
		return locale.toString(today, u"ddd d/M/yyyy"_qs);
	}
	return today.toString(Qt::TextDate);
}

QDate RunCommands::getDateFromStrDate(const QString& strDate) const
{
	const QStringView strdate(strDate);
	if (m_appSettings->value("appLocale").toString() == u"pt_BR"_qs)
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

const QDateTime RunCommands::calculateTimeDifference(const QString& strTimeInit, const QString& strTimeFinal)
{
	int hour(strTimeFinal.left(2).toInt() - strTimeInit.left(2).toInt());
	int min (strTimeFinal.right(2).toInt() - strTimeInit.right(2).toInt());

	if (min < 0)
	{
		hour--;
		min += 60;
	}
	QDateTime timeDiff;
	timeDiff.setTime(QTime(hour, min, 0));
	return timeDiff;
}

void RunCommands::prepareWorkoutTimer(const QString& strStartTime)
{
	if (!m_workoutTimer)
	{
		m_workoutTimer = new QTimer(this);
		m_workoutTimer->setInterval(1000);
		connect(this, &RunCommands::appResumed, this, &RunCommands::correctTimer);
		connect(m_workoutTimer, &QTimer::timeout, this, &RunCommands::calcTime);
	}
	m_hours = strStartTime.left(2).toUInt();
	m_mins = strStartTime.mid(3, 2).toUInt();
	m_secs = strStartTime.right(2).toUInt();
	mb_timerForward = (m_hours == m_mins == m_secs == 0);
	mSessionLength.setHMS(m_hours, m_mins, m_secs);
	mTimeWarnings = 0;
}

void RunCommands::startWorkoutTimer()
{
	if (!m_workoutTimer)
		prepareWorkoutTimer();
	mSessionStartTime = QTime::currentTime();
	m_workoutTimer->start();
	emit timerRunningChanged();
}

void RunCommands::stopWorkoutTimer()
{
	m_workoutTimer->stop();
	emit timerRunningChanged();
	if (mb_timerForward)
	{
		if (mSessionLength.hour() != 0 && mSessionLength.minute() != 0 && mSessionLength.second() != 0)
			mElapsedTime = mSessionLength.addSecs(m_secs + m_mins*60 + m_hours*3600);
		else
			mElapsedTime.setHMS(m_hours, m_mins, m_secs);
	}
	else
	{
		mElapsedTime.setHMS(m_hours, m_mins, m_secs);
		calculateTimeBetweenTimes(mElapsedTime, mSessionLength);
	}
	emit workoutTimerTriggered(mElapsedTime.hour(), mElapsedTime.minute(), mElapsedTime.second());
}

void RunCommands::calcTime()
{
	if (mb_timerForward)
	{
		if (m_secs == 59)
		{
			m_secs = 0;
			if (m_mins == 59)
			{
				m_mins = 0;
				m_hours++;
			}
			m_mins++;
		}
		m_secs++;
	}
	else
	{
		if (m_secs == 0)
		{
			if (m_mins == 0)
			{
				if (m_hours == 0)
				{
					mb_timerForward = true;
					return;
				}
				m_mins = 59;
				m_hours--;
			}
			else
			{
				if (m_hours == 0)
				{
					switch (m_mins)
					{
						case 15:
							if (mTimeWarnings == 0)
							{
								mTimeWarnings = 1;
								emit timeWarning(u"15"_qs, true);
							}
							break;
						case 5:
							if (mTimeWarnings < 2)
							{
								mTimeWarnings = 2;
								emit timeWarning(u"5"_qs, true);
							}
						break;
						case 0:
							if (mTimeWarnings < 3)
							{
								mTimeWarnings = 3;
								emit timeWarning(QString::number(m_secs), false);
							}
							break;
					}
					m_secs = 59;
					m_mins--;
				}
				m_secs--;
			}
		}
	}
	emit workoutTimerTriggered(m_hours, m_mins, m_secs);
}

void RunCommands::correctTimer()
{
	calculateTimeBetweenTimes(mSessionStartTime, QTime::currentTime());
	if (mb_timerForward)
		calculateTimeBetweenTimes(QTime(0, 0, 0), mElapsedTime);
	else
		calculateTimeBetweenTimes(mElapsedTime, mSessionLength);
	m_hours = mElapsedTime.hour();
	m_mins = mElapsedTime.minute();
	m_secs = mElapsedTime.second();
}

void RunCommands::calculateTimeBetweenTimes(const QTime& time1, const QTime& time2)
{
	int hour(time2.hour() - time1.hour());
	int min (time2.minute() - time1.minute());
	int sec(time2.second() - time1.second());

	if (sec < 0)
	{
		min--;
		sec += 60;
	}
	if (min < 0)
	{
		hour--;
		min += 60;
	}

	mElapsedTime.setHMS(hour, min, sec);
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

QString RunCommands::setCompositeValue(const uint idx, const QString& newValue, QString& compositeString) const
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
			return compositeString += newValue;
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
		sep_pos = compositeString.indexOf(QLatin1Char(31), last_sep_pos);
	} while(sep_pos != -1);
	while (++n_seps < idx)
		compositeString += subrecord_separator;
	return compositeString += newValue;
}
