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
	Q_INVOKABLE bool isPartOfMeso(const QDate &date) const;
	Q_INVOKABLE bool isPartOfMeso(const int year, const int month, const int day) const { return isPartOfMeso(QDate{year, month-1, day}); }
	Q_INVOKABLE QString workoutNumber(const QDate &date) const;
	Q_INVOKABLE inline QString workoutNumber(const int year, const int month, const int day) const { return workoutNumber(QDate{year, month-1, day}); }
	Q_INVOKABLE QString splitLetter(const QDate &date) const;
	Q_INVOKABLE inline QString splitLetter(const int year, const int month, const int day) const { return splitLetter(QDate{year, month-1, day}); }
	Q_INVOKABLE void setSplitLetter(const int year, const int month, const int day, const QString &new_splitletter);
	Q_INVOKABLE QTime timeIn(const int year, const int month, const int day) const;
	Q_INVOKABLE void setTimeIn(const int year, const int month, const int day, const QTime &new_timein);
	Q_INVOKABLE QTime timeOut(const int year, const int month, const int day) const;
	Q_INVOKABLE void setTimeOut(const int year, const int month, const int day, const QTime &new_timeout);
	Q_INVOKABLE QString location(const int year, const int month, const int day) const;
	Q_INVOKABLE void setLocation(const int year, const int month, const int day, const QString &new_location);
	Q_INVOKABLE QString notes(const int year, const int month, const int day) const;
	Q_INVOKABLE void setNotes(const int year, const int month, const int day, const QString &new_notes);
	Q_INVOKABLE bool completed(const int year, const int month, const int day) const;
	Q_INVOKABLE void setCompleted(const int year, const int month, const int day, const bool completed);

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
