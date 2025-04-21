#pragma once

#include <QAbstractItemModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarModel)

class DBCalendarModel : public QAbstractItemModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:
	explicit DBCalendarModel(DBMesoCalendarModel *parent, const uint meso_idx, const uint n_months);
	inline uint count() const { return m_nmonths; }

	Q_INVOKABLE QString workoutNumber(const int year, const int month, const int day) const;
	Q_INVOKABLE QString splitLetter(const int year, const int month, const int day) const;
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
	inline virtual int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();

private:
	DBMesoCalendarModel* m_calendarManager;
	uint m_mesoIdx, m_nmonths;
	QHash<int, QByteArray> m_roleNames;
};
