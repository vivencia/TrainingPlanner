#ifndef QMLSETENTRY_H
#define QMLSETENTRY_H

#include <QObject>

#define SET_MODE_UNDEFINED 0
#define SET_MODE_START_REST 1
#define SET_MODE_START_EXERCISE 2
#define SET_MODE_SET_COMPLETED 3

class QQuickItem;
class DBTrainingDayModel;

class QmlSetEntry : public QObject
{

Q_OBJECT

Q_PROPERTY(uint type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(uint number READ number WRITE setMode NOTIFY numberChanged FINAL)
Q_PROPERTY(uint mode READ mode WRITE setMode NOTIFY modeChanged FINAL)
Q_PROPERTY(QString strNumber READ strNumber NOTIFY strNumberChanged FINAL)
Q_PROPERTY(QString modeLabel READ modeLabel NOTIFY modeLabelChanged FINAL)
Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged FINAL)
Q_PROPERTY(bool lastSet READ lastSet WRITE setLastSet NOTIFY lastSetChanged FINAL)
Q_PROPERTY(bool finishButtonEnabled READ finishButtonEnabled WRITE setFinishButtonEnabled NOTIFY finishButtonEnabledChanged FINAL)
Q_PROPERTY(bool trackRestTime READ trackRestTime WRITE setTrackRestTime NOTIFY trackRestTimeChanged FINAL)
Q_PROPERTY(bool autoRestTime READ autoRestTime WRITE setAutoRestTime NOTIFY autoRestTimeChanged FINAL)
Q_PROPERTY(bool current READ current WRITE setCurrent NOTIFY currentChanged FINAL)

public:
	inline explicit QmlSetEntry(QObject* parent, DBTrainingDayModel* tDayModel)
		: QObject{parent}, m_tDayModel(tDayModel) {}

	inline const QQuickItem* setEntry() const { return m_setEntry; }
	inline QQuickItem* setEntry() { return m_setEntry; }
	inline void setSetEntry(QQuickItem* item) { m_setEntry = item; }

	inline const uint type() const { return m_type; }
	void setType(const uint new_value);

	inline const uint number() const { return m_number; }
	inline void setNumber(const uint new_value) { m_number = new_value; emit numberChanged(); }
	inline const QString strNumber() const { return QString::number(m_number + 1); }

	inline QString modeLabel() const
	{
		QString ret;
		switch (mode())
		{
			case SET_MODE_UNDEFINED: ret = tr("Set completed?"); break;
			case SET_MODE_START_REST: ret = tr("Start rest"); break;
			case SET_MODE_START_EXERCISE: ret = tr("Begin exercise"); break;
			case SET_MODE_SET_COMPLETED: break;
		}
		return ret;
	}

	inline const uint mode() const { return m_mode; }
	inline void setMode(const uint new_value) { m_mode = new_value; emit modeChanged(); emit modeLabelChanged(); }

	inline const bool completed() const { return m_bCompleted; }
	void setCompleted(const bool new_value);

	inline const bool lastSet() const { return m_bLastSet; }
	void setLastSet(const bool new_value);

	inline const bool finishButtonEnabled () const { return m_bFinishButtonEnabled; }
	void setFinishButtonEnabled(const bool new_value);

	inline const bool trackRestTime() const { return m_bTrackRestTime; }
	void setTrackRestTime(const bool new_value);

	inline const bool autoRestTime() const { return m_bAutoRestTime; }
	void setAutoRestTime(const bool new_value);

	inline const bool current() const { return m_bCurrent; }
	void setCurrent(const bool new_value);

signals:
	void typeChanged();
	void numberChanged();
	void modeChanged();
	void strNumberChanged();
	void modeLabelChanged();
	void completedChanged();
	void lastSetChanged();
	void finishButtonEnabledChanged();
	void trackRestTimeChanged();
	void autoRestTimeChanged();
	void currentChanged();

private:
	DBTrainingDayModel* m_tDayModel;
	QQuickItem* m_setEntry;
	uint m_type, m_number, m_mode;
	bool m_bCompleted, m_bLastSet, m_bFinishButtonEnabled, m_bTrackRestTime, m_bAutoRestTime, m_bCurrent;
};

#endif // QMLSETENTRY_H
