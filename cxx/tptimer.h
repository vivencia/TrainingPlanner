#ifndef TPTIMER_H
#define TPTIMER_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class QSoundEffect;

class TPTimer : public QTimer
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint hours READ hours WRITE setHours NOTIFY hoursChanged FINAL)
Q_PROPERTY(uint minutes READ minutes WRITE setMinutes NOTIFY minutesChanged FINAL)
Q_PROPERTY(uint seconds READ seconds WRITE setSeconds NOTIFY secondsChanged FINAL)
Q_PROPERTY(QString strHours READ strHours WRITE setStrHours NOTIFY hoursChanged FINAL)
Q_PROPERTY(QString strMinutes READ strMinutes WRITE setStrMinutes NOTIFY minutesChanged FINAL)
Q_PROPERTY(QString strSeconds READ strSeconds WRITE setStrSeconds NOTIFY secondsChanged FINAL)
Q_PROPERTY(bool stopWatch READ stopWatch WRITE setStopWatch NOTIFY stopWatchChanged FINAL)
Q_PROPERTY(bool timerForward READ timerForward NOTIFY timerForwardChanged FINAL)
Q_PROPERTY(QString alarmSoundFile READ alarmSoundFile WRITE setAlarmSoundFile FINAL)
Q_PROPERTY(uint totalSeconds READ totalSeconds NOTIFY totalSecondsChanged FINAL)
Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged FINAL)
Q_PROPERTY(uint progressValue READ progressValue NOTIFY progressValueChanged FINAL)

public:
	explicit TPTimer(QObject* parent = nullptr);
	virtual ~TPTimer();

	Q_INVOKABLE void prepareTimer(const QString& strStartTime = QString());
	Q_INVOKABLE void startTimer(const QString& initialTimeOfDay = QString());
	Q_INVOKABLE void stopTimer();
	Q_INVOKABLE void pauseTimer();
	Q_INVOKABLE void resetTimer(const bool start);

	inline int hours() const { return m_hours; }
	inline void setHours(const uint n_hours) { m_hours = n_hours; emit hoursChanged(); }
	QString strHours() const;
	void setStrHours(QString& str_hours);

	inline int minutes() const { return m_minutes; }
	inline void setMinutes(const uint n_minutes) { m_minutes = n_minutes; emit minutesChanged(); }
	QString strMinutes() const;
	void setStrMinutes(QString& str_minutes);

	inline int seconds() const { return m_seconds; }
	inline void setSeconds(const uint n_seconds) { m_seconds = n_seconds; emit secondsChanged(); }
	QString strSeconds() const;
	void setStrSeconds(QString& str_seconds);

	inline bool stopWatch() const { return mb_stopWatch; }
	inline bool timerForward() const { return mb_timerForward; }
	inline void setStopWatch(const bool forward_timer) { mb_stopWatch = mb_timerForward = forward_timer; emit stopWatchChanged(); }
	inline const QString& alarmSoundFile() const { return m_alarmSoundFile; }
	void setAlarmSoundFile(const QString& soundFileName);

	Q_INVOKABLE void stopAlarmSound();
	Q_INVOKABLE void setAlarmSoundLoops(const uint nloops);
	Q_INVOKABLE inline void addWarningAtMinute(const uint minute) { mMinutesWarnings.append(minute); }
	Q_INVOKABLE inline void addWarningAtSecond(const uint second) { mSecondsWarnings.append(second); }

	Q_INVOKABLE inline uint orignalHours() const { return m_displayStartingTime.first(2).toUInt(); }
	Q_INVOKABLE inline uint orignalMinutes() const { return m_displayStartingTime.sliced(3, 2).toUInt(); }
	Q_INVOKABLE inline uint orignalSeconds() const { return m_displayStartingTime.last(2).toUInt(); }

	inline uint totalSeconds() const { return m_totalSeconds; }
	inline bool paused() const { return mb_paused; }
	inline uint progressValue() const { return m_progressValue; }
	inline const QTime& elapsedTime() const { return m_elapsedTime; }
	Q_INVOKABLE QTime currentElapsedTime() { calculateElapsedTime(); return m_elapsedTime; }
	inline const QTime& initialTime() const { return m_initialTime; }

signals:
	void hoursChanged();
	void minutesChanged();
	void secondsChanged();
	void stopWatchChanged();
	void timerForwardChanged();
	void timeWarning(QString remaingTime, bool bminutes);
	void totalSecondsChanged();
	void pausedChanged();
	void progressValueChanged();

private:
	uint m_hours, m_minutes, m_seconds;
	uint m_totalSeconds, m_progressValue;
	int mWarningIdx;
	bool mb_stopWatch, mb_timerForward, mb_paused, mb_pausedTimePositive;
	QString m_alarmSoundFile, m_displayStartingTime;
	QTime m_elapsedTime;
	QTime m_initialTime;
	QTime m_timeOfDay;
	QTime m_pausedTime;
	QTime m_timeOfPause;
	QList<int> mMinutesWarnings;
	QList<int> mSecondsWarnings;
	QSoundEffect* m_alarmSound;

	inline uint totalSecs(const QTime& time) const { return time.second() + time.minute()*60 + time.hour()*3600; }
	inline void calcTotalSecs() { m_totalSeconds = m_seconds + m_minutes*60 + m_hours*3600; emit totalSecondsChanged(); }
	void prepareFromString();
	const QTime& calculateTimeBetweenTimes(const QTime& time1, const QTime& time2);
	void calculateElapsedTime();
	void calcTime();
	void correctTimer();
};

#endif // TPTIMER_H
