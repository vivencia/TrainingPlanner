#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)

class DBCalendarModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY modelChanged)

public:
	explicit DBCalendarModel(DBMesoCalendarManager *parent, const uint meso_idx);
	inline uint count() const { return m_nmonths; }
	inline void setNMonths(const uint new_nmonths) { m_nmonths = new_nmonths; emit modelChanged(); }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	QDate firstDateOfEachMonth(const uint index) const;

	Q_INVOKABLE inline uint month(const uint index) const { return firstDateOfEachMonth(index).month(); }
	Q_INVOKABLE inline uint year(const uint index) const { return firstDateOfEachMonth(index).year(); }
	Q_INVOKABLE QDate date(const uint calendar_day) const;
	Q_INVOKABLE bool isPartOfMeso(const QDate &date) const;
	Q_INVOKABLE bool isWorkoutDay(const QDate &date) const;
	Q_INVOKABLE bool isWorkoutDay(const uint calendar_day) const;
	Q_INVOKABLE QString workoutNumber(const QDate &date) const;
	Q_INVOKABLE QString workoutNumber(const uint calendar_day) const;
	Q_INVOKABLE QString splitLetter(const QDate &date) const;
	Q_INVOKABLE QString splitLetter(const uint calendar_day) const;
	Q_INVOKABLE void setSplitLetter(const QDate &date, const QString &new_splitletter);
	Q_INVOKABLE void setSplitLetter(const uint calendar_day, const QString &new_splitletter);
	Q_INVOKABLE QTime timeIn(const QDate &date) const;
	Q_INVOKABLE QTime timeIn(const uint calendar_day) const;
	Q_INVOKABLE void setTimeIn(const QDate &date, const QTime &new_timein);
	Q_INVOKABLE void setTimeIn(const uint calendar_day, const QTime &new_timein);
	Q_INVOKABLE QTime timeOut(const QDate &date) const;
	Q_INVOKABLE QTime timeOut(const uint calendar_day) const;
	Q_INVOKABLE void setTimeOut(const QDate &date, const QTime &new_timeout);
	Q_INVOKABLE void setTimeOut(const uint calendar_day, const QTime &new_timeout);
	Q_INVOKABLE QString location(const QDate &date) const;
	Q_INVOKABLE QString location(const uint calendar_day) const;
	Q_INVOKABLE void setLocation(const QDate &date, const QString &new_location);
	Q_INVOKABLE void setLocation(const uint calendar_day, const QString &new_location);
	Q_INVOKABLE QString notes(const QDate &date) const;
	Q_INVOKABLE QString notes(const uint calendar_day) const;
	Q_INVOKABLE void setNotes(const QDate &date, const QString &new_notes);
	Q_INVOKABLE void setNotes(const uint calendar_day, const QString &new_notes);
	Q_INVOKABLE bool completed(const QDate &date) const;
	Q_INVOKABLE bool completed(const uint calendar_day) const;
	Q_INVOKABLE void setCompleted(const QDate &date, const bool completed);
	Q_INVOKABLE void setCompleted(const uint calendar_day, const bool completed);

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void modelChanged();
	void workoutNumberChanged(const QDate &date);
	void splitLetterChanged(const QDate &date);
	void completedChanged(const QDate &date);

private:
	DBMesoCalendarManager* m_calendarManager;
	uint m_mesoIdx, m_nmonths;
	QHash<int, QByteArray> m_roleNames;
};
