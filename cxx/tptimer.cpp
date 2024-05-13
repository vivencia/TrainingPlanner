#include "tptimer.h"
#include "runcommands.h"

TPTimer::TPTimer(QObject* parent, RunCommands* runCmd)
	: QTimer{parent}, mb_stopWatch(true), mb_timerForward(true)
{
	connect(this, &QTimer::timeout, this, &TPTimer::calcTime);
	connect(runCmd, &RunCommands::appResumed, this, &TPTimer::correctTimer);
}

void TPTimer::prepareTimer(const QString& strStartTime)
{
	m_hours = strStartTime.left(2).toUInt();
	m_minutes = strStartTime.mid(3, 2).toUInt();
	m_seconds = strStartTime.right(2).toUInt();
	mb_timerForward = (m_hours == m_minutes == m_seconds == 0);
	mTimeWarnings = 0;
}

void TPTimer::startTimer()
{
	m_elapsedTime.setHMS(0, 0, 0);
	m_initialTime.setHMS(m_hours, m_minutes, m_seconds);
	m_timeOfDay = QTime::currentTime();
	start();
}

void TPTimer::stopTimer()
{
	stop();
	if (stopWatch())
		m_elapsedTime.setHMS(m_hours, m_minutes, m_seconds);
	else
	{
		if (!mb_timerForward)
			calculateTimeBetweenTimes(QTime(m_hours, m_seconds, m_minutes), m_initialTime);
		else
			m_elapsedTime = m_initialTime.addSecs(totalSecs());
	}
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
					return;
				}
				m_minutes = 59;
				m_hours--;
				emit hoursChanged();
				emit minutesChanged();
			}
			else
			{
				if (m_hours == 0)
				{
					if (!mMinutesWarnings.isEmpty())
					{
						if (m_minutes == mMinutesWarnings.at(mTimeWarnings))
						{
							mTimeWarnings++;
							emit timeWarning(QString::number(m_minutes), true);
						}
					}
					else if (!mSecondsWarnings.isEmpty())
					{
						if (m_seconds == mSecondsWarnings.at(mTimeWarnings))
						{
							mTimeWarnings++;
							emit timeWarning(QString::number(m_seconds), false);
						}
					}
					m_seconds = 60;
					m_minutes--;
					emit minutesChanged();
				}
				m_seconds--;
				emit secondsChanged();
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
