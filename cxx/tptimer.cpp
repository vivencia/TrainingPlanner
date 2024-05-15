#include <QSoundEffect>

#include "tptimer.h"
#include "runcommands.h"

static int warningIdx(-1);

TPTimer::TPTimer(QObject* parent)
	: QTimer{parent}, m_hours(0), m_minutes(0), m_seconds(0), m_totalSeconds(0), mb_stopWatch(true),
		mb_timerForward(true), mb_paused(false), m_alarmSound(nullptr)
{
	connect(this, &QTimer::timeout, this, &TPTimer::calcTime);
	m_pausedTime.setHMS(0, 0, 0);
	m_timeOfPause.setHMS(0, 0 ,0);
	m_originalStartTime = u"00:00:00"_qs;
}

TPTimer::~TPTimer()
{
	if (m_alarmSound)
		delete m_alarmSound;
}

void TPTimer::setRunCommandsObject(RunCommands *runCmd)
{
	connect(runCmd, &RunCommands::appResumed, this, &TPTimer::correctTimer);
}

void TPTimer::prepareTimer(const QString& strStartTime)
{
	m_originalStartTime = strStartTime;
	prepareFromString();
	emit hoursChanged();
	emit minutesChanged();
	emit secondsChanged();
	emit pausedChanged();
	emit timerForwardChanged();
}

void TPTimer::startTimer()
{
	if (!mb_paused)
	{
		m_elapsedTime.setHMS(0, 0, 0);
		m_initialTime.setHMS(m_hours, m_minutes, m_seconds);
		m_timeOfDay = QTime::currentTime();
	}
	else
	{
		mb_paused = false;
		emit pausedChanged();
		calculateTimeBetweenTimes(m_timeOfPause, QTime::currentTime());
		m_pausedTime = m_pausedTime.addSecs(totalSecs(m_elapsedTime));
	}
	start();
}

void TPTimer::stopTimer()
{
	stop();
	stopAlarmSound();
	calculateElapsedTime();
	mb_paused = false;
	emit pausedChanged();
}

void TPTimer::pauseTimer()
{
	if (!isActive())
		return;
	stopAlarmSound();
	mb_paused = true;
	m_timeOfPause = QTime::currentTime();
	stop();
	emit pausedChanged();
}

void TPTimer::resetTimer(const bool start)
{
	stop();
	stopAlarmSound();
	prepareFromString();
	if (start)
		startTimer();
	else
	{
		emit hoursChanged();
		emit minutesChanged();
		emit secondsChanged();
		emit pausedChanged();
	}
}

QString TPTimer::strHours() const
{
	QString ret(QString::number(m_hours));
	if (m_hours < 10)
		ret.prepend('0');
	return ret;
}

void TPTimer::setStrHours(const QString& str_hours)
{
	bool bOK(true);
	const uint hours(str_hours.toUInt(&bOK));
	if (bOK)
	{
		m_hours = hours;
		calcTotalSecs();
		m_originalStartTime.replace(0, 2, str_hours);
		emit hoursChanged();
	}
}

QString TPTimer::strMinutes() const
{
	QString ret(QString::number(m_minutes));
	if (m_minutes < 10)
		ret.prepend('0');
	return ret;
}

void TPTimer::setStrMinutes(const QString& str_minutes)
{
	bool bOK(true);
	const uint minutes(str_minutes.toUInt(&bOK));
	if (bOK)
	{
		m_minutes = minutes;
		calcTotalSecs();
		m_originalStartTime.replace(3, 2, str_minutes);
		emit minutesChanged();
	}
}

QString TPTimer::strSeconds() const
{
	QString ret(QString::number(m_seconds));
	if (m_seconds < 10)
		ret.prepend('0');
	return ret;
}

void TPTimer::setStrSeconds(const QString& str_seconds)
{
	bool bOK(true);
	const uint seconds(str_seconds.toUInt(&bOK));
	if (bOK)
	{
		m_seconds = seconds;
		calcTotalSecs();
		m_originalStartTime.replace(6, 2, str_seconds);
		emit secondsChanged();
	}
}

void TPTimer::setAlarmSoundFile(const QString& soundFileName)
{
	if (!m_alarmSound)
	{
		m_alarmSound = new QSoundEffect(this);
		m_alarmSound->setSource(soundFileName);
		m_alarmSound->setLoopCount(1);
		m_alarmSound->setVolume(0.25f);
	}
}

void TPTimer::stopAlarmSound()
{
	if (m_alarmSound)
		m_alarmSound->stop();
}

void TPTimer::setAlarmSoundLoops(const uint nloops)
{
	if (m_alarmSound)
		m_alarmSound->setLoopCount(nloops);
}

void TPTimer::prepareFromString()
{
	if (!m_originalStartTime.isEmpty())
	{
		m_hours = m_originalStartTime.left(2).toUInt();
		m_minutes = m_originalStartTime.mid(3, 2).toUInt();
		m_seconds = m_originalStartTime.right(2).toUInt();
		mb_timerForward = mb_stopWatch;
	}
	else
	{
		m_hours = m_minutes = m_seconds = 0;
		if (mb_stopWatch)
			mb_timerForward = true;
		m_totalSeconds = 0;
	}
	mb_paused = false;
	m_pausedTime.setHMS(0, 0, 0);
	m_timeOfPause.setHMS(0, 0 ,0);
}

const QTime& TPTimer::calculateTimeBetweenTimes(const QTime& time1, const QTime& time2)
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
	m_elapsedTime.setHMS(hour, min, sec);
	return m_elapsedTime;
}

void TPTimer::calculateElapsedTime()
{
	if (!m_pausedTime.isNull())
	{
		calculateTimeBetweenTimes(m_timeOfPause, QTime::currentTime());
		m_pausedTime = m_pausedTime.addSecs(totalSecs(m_elapsedTime));
	}
	if (stopWatch())
		m_elapsedTime.setHMS(m_hours, m_minutes, m_seconds);
	else
	{
		if (!mb_timerForward)
			calculateTimeBetweenTimes(QTime(m_hours, m_seconds, m_minutes), m_initialTime);
		else
			m_elapsedTime = m_initialTime.addSecs(m_totalSeconds);
		m_totalSeconds = totalSecs(m_elapsedTime);
		emit totalSecondsChanged();
	}
	if (!m_pausedTime.isNull())
		m_elapsedTime = m_elapsedTime.addMSecs(-totalSecs(m_pausedTime));
}

void TPTimer::calcTime()
{
	if (mb_timerForward)
	{
		if (m_seconds == 59)
		{
			m_seconds = -1;
			if (m_minutes == 59)
			{
				m_minutes = -1;
				m_hours++;
				emit hoursChanged();
			}
			m_minutes++;
			emit minutesChanged();
		}
		m_seconds++;
		emit secondsChanged();
	}
	else
	{
		if (m_seconds == 0)
		{
			if (m_minutes == 0)
			{
				if (m_hours == 0)
				{
					mb_timerForward = true;
					emit timerForwardChanged();
					return;
				}
				m_minutes = 60;
				m_hours--;
				emit hoursChanged();
				emit minutesChanged();
			}
			else
			{
				if (m_hours == 0)
				{
					warningIdx = mMinutesWarnings.indexOf(m_minutes);
					if (warningIdx != -1)
					{
						mMinutesWarnings.remove(warningIdx);
						emit timeWarning(QString::number(m_minutes), true);
						if (m_alarmSound)
							m_alarmSound->play();
					}
				}
			}
			m_minutes--;
			emit minutesChanged();
			m_seconds = 60;
		}
		m_seconds--;
		emit secondsChanged();
		if (m_minutes == 0)
		{
			warningIdx = mSecondsWarnings.indexOf(m_seconds);
			if (warningIdx != -1)
			{
				mSecondsWarnings.remove(warningIdx);
				emit timeWarning(QString::number(m_seconds), false);
				if (m_alarmSound)
					m_alarmSound->play();
			}
		}
	}
}

void TPTimer::correctTimer()
{
	static_cast<void>(calculateTimeBetweenTimes(m_timeOfDay, QTime::currentTime()));
	if (mb_timerForward)
		static_cast<void>(calculateTimeBetweenTimes(m_initialTime, m_elapsedTime));
	else
		static_cast<void>(calculateTimeBetweenTimes(m_elapsedTime, m_initialTime));
	m_hours = m_elapsedTime.hour();
	emit hoursChanged();
	m_minutes = m_elapsedTime.minute();
	emit minutesChanged();
	m_seconds = m_elapsedTime.second();
	emit secondsChanged();
}
