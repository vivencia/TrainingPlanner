#include "tptimer.h"
#include "tputils.h"

#include <QSoundEffect>

TPTimer::TPTimer(QObject* parent)
	: QTimer{parent}, m_hours(0), m_minutes(0), m_seconds(0), m_totalSeconds(0), mb_stopWatch(true),
		mb_timerForward(true), mb_paused(false), m_alarmSound(nullptr)
{
	connect(this, &QTimer::timeout, this, &TPTimer::calcTime);
	connect(appUtils(), &TPUtils::appResumed, this, &TPTimer::correctTimer);
	m_pausedTime.setHMS(0, 0, 0);
	m_timeOfPause.setHMS(0, 0 ,0);
	m_originalStartTime = u"00:00:00"_s;
	setInterval(1000);
}

TPTimer::~TPTimer()
{
	if (m_alarmSound)
		delete m_alarmSound;
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
	emit progressValueChanged();
}

void TPTimer::startTimer(const QString& initialTimeOfDay)
{
	if (!mb_paused)
	{
		m_elapsedTime.setHMS(0, 0, 0);
		m_initialTime.setHMS(m_hours, m_minutes, m_seconds);
		if (initialTimeOfDay.contains('-'))
			m_timeOfDay = std::move(QTime::currentTime());
		else
			m_timeOfDay = std::move(appUtils()->timeFromStrTime(initialTimeOfDay));
	}
	else
	{
		mb_paused = false;
		emit pausedChanged();
		calculateTimeBetweenTimes(m_timeOfPause, QTime::currentTime());
		m_pausedTime = m_pausedTime.addSecs(totalSecs(m_elapsedTime));
		mb_pausedTimePositive = true;
	}
	start();
}

void TPTimer::stopTimer()
{
	if (isActive())
	{
		stop();
		stopAlarmSound();
	}
	if (mb_paused)
	{
		calculateElapsedTime();
		mb_paused = false;
		mb_pausedTimePositive = false;
		m_pausedTime.setHMS(0, 0, 0);
		emit pausedChanged();
	}
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
		startTimer(QString());
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

void TPTimer::setStrHours(QString& str_hours)
{
	if (str_hours.length() == 1)
		str_hours.prepend('0');
	prepareTimer(m_originalStartTime.replace(0, 2, str_hours));
}

QString TPTimer::strMinutes() const
{
	QString ret(QString::number(m_minutes));
	if (m_minutes < 10)
		ret.prepend('0');
	return ret;
}

void TPTimer::setStrMinutes(QString& str_minutes)
{
	if (str_minutes.length() == 1)
		str_minutes.prepend('0');
	prepareTimer(m_originalStartTime.replace(3, 2, str_minutes));
}

QString TPTimer::strSeconds() const
{
	QString ret(QString::number(m_seconds));
	if (m_seconds < 10)
		ret.prepend('0');
	return ret;
}

void TPTimer::setStrSeconds(QString& str_seconds)
{
	if (str_seconds.length() == 1)
		str_seconds.prepend('0');
	prepareTimer(m_originalStartTime.replace(6, 2, str_seconds));
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
		if (m_originalStartTime.length() < 2)
			m_originalStartTime.append(u"0:00:00"_s);
		m_hours = m_originalStartTime.first(2).toUInt();
		if (m_originalStartTime.length() < 5)
			m_originalStartTime.append(u":00:00"_s);
		m_minutes = m_originalStartTime.sliced(3, 2).toUInt();
		if (m_originalStartTime.length() < 8)
			m_originalStartTime.append(u":00"_s);
		m_seconds = m_originalStartTime.last(2).toUInt();
		mb_timerForward = mb_stopWatch;
	}
	else
	{
		m_hours = m_minutes = m_seconds = 0;
		if (mb_stopWatch)
			mb_timerForward = true;
	}
	calcTotalSecs();
	m_progressValue = !mb_stopWatch ? m_totalSeconds : 0;
	mb_paused = false;
	mb_pausedTimePositive = false;
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
		--min;
		sec += 60;
	}
	if (min < 0)
	{
		--hour;
		min += 60;
	}
	m_elapsedTime.setHMS(hour, min, sec);
	return m_elapsedTime;
}

void TPTimer::calculateElapsedTime()
{
	if (mb_pausedTimePositive)
	{
		calculateTimeBetweenTimes(m_timeOfPause, QTime::currentTime());
		m_pausedTime = m_pausedTime.addSecs(totalSecs(m_elapsedTime));
	}
	if (stopWatch())
		m_elapsedTime.setHMS(m_hours, m_minutes, m_seconds);
	else
	{
		if (!mb_timerForward)
			calculateTimeBetweenTimes(QTime(m_hours, m_minutes, m_seconds), m_initialTime);
		else
			m_elapsedTime = m_initialTime.addSecs(m_totalSeconds);
		m_totalSeconds = totalSecs(m_elapsedTime);
		emit totalSecondsChanged();
	}
	if (mb_pausedTimePositive)
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
				++m_hours;
				emit hoursChanged();
			}
			++m_minutes;
			emit minutesChanged();
		}
		++m_seconds;
		++m_progressValue;
		emit progressValueChanged();
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
				--m_hours;
				emit hoursChanged();
				emit minutesChanged();
			}
			else
			{
				if (m_hours == 0)
				{
					mWarningIdx = mMinutesWarnings.indexOf(m_minutes);
					if (mWarningIdx != -1)
					{
						mMinutesWarnings.remove(mWarningIdx);
						emit timeWarning(QString::number(m_minutes), true);
						if (m_alarmSound)
							m_alarmSound->play();
					}
				}
			}
			--m_minutes;
			emit minutesChanged();
			m_seconds = 60;
		}
		--m_seconds;
		--m_progressValue;
		emit progressValueChanged();
		emit secondsChanged();
		if (m_minutes == 0)
		{
			mWarningIdx = mSecondsWarnings.indexOf(m_seconds);
			if (mWarningIdx != -1)
			{
				mSecondsWarnings.remove(mWarningIdx);
				emit timeWarning(QString::number(m_seconds), false);
				if (m_alarmSound)
					m_alarmSound->play();
			}
		}
	}
}

void TPTimer::correctTimer()
{
	if (isActive() && !paused())
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
}
