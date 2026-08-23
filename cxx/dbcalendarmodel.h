#pragma once

#include "dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceCalendar);

class DBCalendarModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(DBCalendarModel)
QML_UNCREATABLE("")

Q_PROPERTY(int nMonths READ nMonths NOTIFY nMonthsChanged FINAL)
Q_PROPERTY(int currentDay READ currentDay WRITE setCurrentDay NOTIFY currentDayChanged FINAL)
Q_PROPERTY(QDate currentDate READ currentDate WRITE setCurrentDate NOTIFY currentDayChanged FINAL)
Q_PROPERTY(QDate startDate READ startDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QDate endDate READ endDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QString workoutNumber READ workoutNumber NOTIFY workoutNumberChanged FINAL)
Q_PROPERTY(QString splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged FINAL)
Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged FINAL)

public:

	enum MesoCalendarFields {
		FLD_MESOID,
		FLD_WORKOUTID,
		FLD_DATE,
		FLD_WORKOUTNUMBER,
		FLD_SPLITLETTER,
		FLD_TIMEIN,
		FLD_TIMEOUT,
		FLD_LOCATION,
		FLD_NOTES,
		FLD_WORKOUT_COMPLETED,
		MESOCALENDAR_N_FIELDS
	};

	explicit DBCalendarModel(DBMesocyclesModel *parent, DBMesoCalendarTable *db, const uint meso_idx);
	DBModelInterfaceCalendar *dbModelInterface() const { return m_dbmic; }
	inline void setNMonths(const uint n_months) { m_nMonths = n_months; }

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; }
	QDate firstDateOfEachMonth(const uint index) const;
	const QString &mesoId() const;
	int calendarDay(const QDate &date) const;

	Q_INVOKABLE int getIndexFromDate(const QDate &date) const;
	Q_INVOKABLE inline uint month(const uint index) const { return firstDateOfEachMonth(index).month() - 1; }
	Q_INVOKABLE inline uint year(const uint index) const { return firstDateOfEachMonth(index).year(); }
	Q_INVOKABLE QDate date(const uint calendar_day) const;

	inline int nMonths() const { return m_nMonths; }
	inline int currentDay() const { return m_curDay; }
	inline void setCurrentDay(const uint new_day)
	{
		if (new_day != m_curDay) {
			m_curDay = new_day;
			setCurrentDate(date(m_curDay));
			emit currentDayChanged();
		}
	}
	inline QDate currentDate() const { return m_curDate; }
	inline void setCurrentDate(const QDate &new_date)
	{
		if (new_date != m_curDate) {
			m_curDate = new_date;
			setCurrentDay(calendarDay(m_curDate));
			emit currentDayChanged();
		}
	}
	inline QDate startDate() const { return m_startDate; }
	inline QDate endDate() const { return m_startDate.addDays(m_nCaldays); }

	inline bool isPartOfMeso() const { return m_curDay >= 0 && m_curDay < m_nCaldays; }
	Q_INVOKABLE inline bool isPartOfMeso(const QDate &date) const { return calendarDay(date) != -1; }
	inline bool isWorkoutDay() const { return !workoutNumber().isEmpty(); }
	bool isWorkoutDay(const int calendar_day);
	Q_INVOKABLE inline bool isWorkoutDay(const QDate &date) const { return !workoutNumber(date).isEmpty(); }
	Q_INVOKABLE QString workoutNumber(const QDate &date) const;
	QString workoutNumber() const;
	Q_INVOKABLE QString splitLetter(const QDate &date) const;
	void setSplitLetter(const QDate &date, const QString &new_splitletter);
	QString splitLetter() const;
	void setSplitLetter(const QString &new_splitletter);
	Q_INVOKABLE inline int splitLetterToIndex() const
	{
		const QChar &splitletter{splitLetter().at(0)};
		return splitletter != 'R' ? static_cast<int>(splitletter.cell()) - static_cast<int>('A') : 6;
	}
	Q_INVOKABLE QString dayEntryLabel(const QDate &date) const;
	Q_INVOKABLE inline bool selectable(const QDate &date) const
	{
		return m_selectable.contains(date) ? m_selectable.value(date) : true;
	}
	inline void setSelectable(const QDate &date, const bool selectable)
	{
		m_selectable[date] = selectable;
	}
	//Sets all the dates on the QList to selectable and the other dates on the calendar to !selectable
	void setSelectable(QList<QDate> &&dates, const bool selectable);
	//This model has the ability to display two sets of dates: one custom(via setSelectable),
	//the other the entire program calendar(from m_startDate to m_startDate + m_nCalDays
	//changeSelectableDates() can alternate between the two
	inline void setAllDatesSelectable(const bool selectable) { setSelectable(QList<QDate>{}, !selectable); }

	QString location(const int calendar_day) const;
	QString location() const;
	void setLocation(const QString &new_location);
	QString notes() const;
	void setNotes(const QString &new_notes);
	QTime timeIn() const;
	void setTimeIn(const QTime &new_timein);
	QTime timeOut() const;
	void setTimeOut(const QTime &new_timeout);
	Q_INVOKABLE bool completed_by_date(const QDate &date) const;
	bool completed() const;
	void setCompleted(const bool completed);

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return nMonths(); }

public slots:
	void changeSelectableDates(const bool use_selectable_list);

signals:
	void nMonthsChanged();
	void currentDayChanged();
	void startDateChanged();
	void endDateChanged();
	void workoutNumberChanged();
	void splitLetterChanged(const QDate &cal_date = QDate{});
	void completedChanged(const QDate &cal_date = QDate{}, const bool completed = true);
	void calendarLoaded(const bool success);

private:
	DBMesocyclesModel *m_mesoModel{nullptr};
	uint m_mesoIdx;
	int m_nMonths{-1}, m_curDay{-1}, m_nCaldays{-1};
	QHash<QDate,bool> m_selectable;
	QList<QDate> m_selectableList;
	QHash<int, QByteArray> m_roleNames;
	DBMesoCalendarTable *m_db{nullptr};
	DBModelInterfaceCalendar *m_dbmic{nullptr};
	QDate m_curDate, m_startDate;

	inline QString dayInfo(const int calendar_day, const uint field) const;
	inline void setDayInfo(const int calendar_day, const uint field, const QString &new_value);
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

