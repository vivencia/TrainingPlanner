#ifndef TPTIMER_H
#define TPTIMER_H

#include <QObject>
#include <QTimer>
#include <QQmlEngine>

class RunCommands;
class QSoundEffect;

class TPTimer : public QTimer
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int hours READ hours WRITE setHours NOTIFY hoursChanged FINAL)
Q_PROPERTY(int minutes READ minutes WRITE setMinutes NOTIFY minutesChanged FINAL)
Q_PROPERTY(int seconds READ seconds WRITE setSeconds NOTIFY secondsChanged FINAL)
Q_PROPERTY(QString strHours READ strHours WRITE setStrHours NOTIFY hoursChanged FINAL)
Q_PROPERTY(QString strMinutes READ strMinutes WRITE setStrMinutes NOTIFY minutesChanged FINAL)
Q_PROPERTY(QString strSeconds READ strSeconds WRITE setStrSeconds NOTIFY secondsChanged FINAL)
Q_PROPERTY(bool stopWatch READ stopWatch WRITE setStopWatch NOTIFY stopWatchChanged FINAL)
Q_PROPERTY(QString alarmSoundFile READ alarmSoundFile WRITE setAlarmSoundFile FINAL)
Q_PROPERTY(int totalSeconds READ totalSeconds NOTIFY totalSecondsChanged FINAL)

public:
	explicit TPTimer(QObject* parent = nullptr);
	virtual ~TPTimer();

	Q_INVOKABLE void setRunCommandsObject(RunCommands *runCmd);
	Q_INVOKABLE void prepareTimer(const QString& strStartTime);
	Q_INVOKABLE void startTimer();
	Q_INVOKABLE void stopTimer();

	inline int hours() const { return m_hours; }
	inline void setHours(const int n_hours) { m_hours = n_hours; emit hoursChanged(); }
	QString strHours() const;
	void setStrHours(const QString& str_hours);

	inline int minutes() const { return m_minutes; }
	inline void setMinutes(const int n_minutes) { m_minutes = n_minutes; emit minutesChanged(); }
	QString strMinutes() const;
	void setStrMinutes(const QString& str_minutes);

	inline int seconds() const { return m_seconds; }
	inline void setSeconds(const int n_seconds) { m_seconds = n_seconds; emit secondsChanged(); }
	QString strSeconds() const;
	void setStrSeconds(const QString& str_seconds);

	inline bool stopWatch() const { return mb_stopWatch; }
	inline void setStopWatch(const bool forward_timer) { mb_stopWatch = mb_timerForward = forward_timer; emit stopWatchChanged(); }
	inline const QString& alarmSoundFile() const { return m_alarmSoundFile; }
	void setAlarmSoundFile(const QString& soundFileName);

	Q_INVOKABLE void stopAlarmSound();
	Q_INVOKABLE void setAlarmSoundLoops(const uint nloops);
	Q_INVOKABLE inline void addWarningAtMinute(const uint minute) { mMinutesWarnings.append(minute); }
	Q_INVOKABLE inline void addWarningAtSecond(const uint second) { mSecondsWarnings.append(second); }

	inline int totalSeconds() const { return m_totalSeconds; }
	Q_INVOKABLE inline QDateTime elapsedTime() const { return QDateTime(QDate::currentDate(), m_elapsedTime); }
	inline const QTime& initialTime() const { return m_initialTime; }

signals:
	void hoursChanged();
	void minutesChanged();
	void secondsChanged();
	void stopWatchChanged();
	void timeWarning(QString remaingMinutes, bool bminutes);
	void totalSecondsChanged();

private:
	int m_hours, m_minutes, m_seconds;
	int m_totalSeconds;
	bool mb_stopWatch, mb_timerForward;
	QString m_alarmSoundFile;
	QTime m_elapsedTime;
	QTime m_initialTime;
	QTime m_timeOfDay;
	QList<int> mMinutesWarnings;
	QList<int> mSecondsWarnings;
	QSoundEffect* m_alarmSound;

	const QTime& calculateTimeBetweenTimes(const QTime& time1, const QTime& time2);
	void calcTime();
	void correctTimer();
};
#endif // TPTIMER_H
