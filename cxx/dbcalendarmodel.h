#pragma once

#include "dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar);

class DBCalendarModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBCalendarModel(DBMesoCalendarManager *parent, DBMesoCalendarTable *db, const uint meso_idx);
	DBModelInterfaceCalendar *dbModelInterface() const { return m_dbModelInterface; }
	inline void setNMonths(const uint n_months) { m_nmonths = n_months; }

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	QDate firstDateOfEachMonth(const uint index) const;
	const QString &mesoId() const;

	Q_INVOKABLE int getIndexFromDate(const QDate &date) const;
	Q_INVOKABLE inline uint count() const { return m_nmonths; }
	Q_INVOKABLE inline uint month(const uint index) const { return firstDateOfEachMonth(index).month() - 1; }
	Q_INVOKABLE inline uint year(const uint index) const { return firstDateOfEachMonth(index).year(); }
	Q_INVOKABLE QDate date(const uint calendar_day) const;
	Q_INVOKABLE bool isPartOfMeso(const QDate &date) const;
	Q_INVOKABLE bool isWorkoutDay(const QDate &date) const;
	Q_INVOKABLE bool isWorkoutDay(const uint calendar_day) const;

	Q_INVOKABLE QString dayText(const QDate &date) const;
	QString workoutNumber(const QDate &date) const;
	QString workoutNumber(const uint calendar_day) const;
	QString splitLetter(const QDate &date) const;
	QString splitLetter(const uint calendar_day) const;
	void setSplitLetter(const QDate &date, const QString &new_splitletter);
	void setSplitLetter(const uint calendar_day, const QString &new_splitletter);
	QTime timeIn(const QDate &date) const;
	QTime timeIn(const uint calendar_day) const;
	void setTimeIn(const QDate &date, const QTime &new_timein);
	void setTimeIn(const uint calendar_day, const QTime &new_timein);
	QTime timeOut(const QDate &date) const;
	QTime timeOut(const uint calendar_day) const;
	void setTimeOut(const QDate &date, const QTime &new_timeout);
	void setTimeOut(const uint calendar_day, const QTime &new_timeout);
	QString location(const QDate &date) const;
	QString location(const uint calendar_day) const;
	void setLocation(const QDate &date, const QString &new_location);
	void setLocation(const uint calendar_day, const QString &new_location);
	QString notes(const QDate &date) const;
	QString notes(const uint calendar_day) const;
	void setNotes(const QDate &date, const QString &new_notes);
	void setNotes(const uint calendar_day, const QString &new_notes);
	Q_INVOKABLE bool completed(const QDate &date) const;
	bool completed(const uint calendar_day) const;
	void setCompleted(const QDate &date, const bool completed);
	void setCompleted(const uint calendar_day, const bool completed);

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void workoutNumberChanged(const QDate &date);
	void splitLetterChanged(const QDate &date);
	void completedChanged(const QDate &date);
	void calendarLoaded(const bool success);

private:
	DBMesoCalendarManager *m_calendarManager;
	uint m_mesoIdx, m_nmonths, m_ncaldays;
	QHash<int, QByteArray> m_roleNames;

	DBMesoCalendarTable *m_db;
	DBModelInterfaceCalendar *m_dbModelInterface;
};

class DBModelInterfaceCalendar : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceCalendar(DBCalendarModel *model) : DBModelInterface{model} {}
	inline const QList<QStringList> &modelData() const { return m_modelData; }
	inline QList<QStringList> &modelData() { return m_modelData; }

private:
	QList<QStringList> m_modelData;
};

