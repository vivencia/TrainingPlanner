#ifndef TPTIMER_H
#define TPTIMER_H

#include <QObject>
#include <QTimer>
#include <QQmlEngine>

class RunCommands;

class TPTimer : public QTimer
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int hours READ hours WRITE setHours NOTIFY hoursChanged FINAL)
Q_PROPERTY(int minutes READ minutes WRITE setMinutes NOTIFY minutesChanged FINAL)
Q_PROPERTY(int seconds READ seconds WRITE setSeconds NOTIFY secondsChanged FINAL)
Q_PROPERTY(bool stopWatch READ stopWatch WRITE setStopWatch NOTIFY stopWatchChanged FINAL)

public:
	explicit TPTimer(QObject* parent, RunCommands *runCmd);

	void prepareTimer(const QString& strStartTime);
	void startTimer();
	void stopTimer();
	const QTime& calculateTimeBetweenTimes(const QTime& time1, const QTime& time2);

	inline int hours() const { return m_hours; }
	inline void setHours(const int n_hours) { m_hours = n_hours; emit hoursChanged(); }
	inline int minutes() const { return m_minutes; }
	inline void setMinutes(const int n_minutes) { m_minutes = n_minutes; emit minutesChanged(); }
	inline int seconds() const { return m_seconds; }
	inline void setSeconds(const int n_seconds) { m_seconds = n_seconds; emit secondsChanged(); }
	inline bool stopWatch() const { return mb_stopWatch; }
	inline void setStopWatch(const bool forward_timer) { mb_stopWatch = mb_timerForward = forward_timer; emit stopWatchChanged(); }

	template<class... Args>
	void setMinutesWarnings(Args... mins);
	template<class... Args>
	void setSecondsWarnings(Args... secs);

	inline int totalSecs() const { return m_seconds + m_minutes*60 + m_hours*3600; }
	inline const QTime& elapsedTime() const { return m_elapsedTime; }
	inline const QTime& initialTime() const { return m_initialTime; }

signals:
	void hoursChanged();
	void minutesChanged();
	void secondsChanged();
	void stopWatchChanged();
	void timeWarning(QString remaingMinutes, bool bminutes);

private:
	int m_hours, m_minutes, m_seconds;
	uint mTimeWarnings;
	bool mb_stopWatch, mb_timerForward;
	QTime m_elapsedTime;
	QTime m_initialTime;
	QTime m_timeOfDay;
	QList<int> mMinutesWarnings;
	QList<int> mSecondsWarnings;

	void calcTime();
	void correctTimer();
};

template<class... Args>
void TPTimer::setMinutesWarnings(Args... mins)
{
	mTimeWarnings = 0;
	const auto list = {mins...};
	mMinutesWarnings = list;
}

template<class... Args>
void TPTimer::setSecondsWarnings(Args... secs)
{
	mTimeWarnings = 0;
	const auto list = {secs...};
	mSecondsWarnings = list;
}

#endif // TPTIMER_H
