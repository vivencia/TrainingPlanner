#pragma once

#include "tputils.h"

#include <QAbstractListModel>
#include <qqml.h>

class CalendarMethods : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(CalendarMethods)

Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged FINAL)

public:

	enum Methods {
		Today,
		Tomorrow,
		Yesterday,
		NextMonth,
		PrevMonth,
		NextWeek,
		PrevWeek,
		TwoWeeksAhead,
		TwoWeeksBefore,
		FifteenDaysAhead,
		FifteenDaysAgo,
		ThirtyDays,
		ThirtyDaysAgo,
		TwoMonthsAhead,
		TwoMonthsAgo,
		SixtyDaysAhead,
		SixtyDaysAgo,
		MethodsCount
	};
	Q_ENUM(Methods)

	enum RoleNames {
		createRole(label, 0)
		createRole(method_function, 1)
	};

	struct st_methodData {
		QString methodName;
		std::function<QDate(void)> methodFunciton;
	};

	inline explicit CalendarMethods(QObject *parent = nullptr) : QAbstractListModel{parent}
	{
		roleToString(label)
		roleToString(method_function)
	}

	inline const QDate &date() const { return m_date; }
	inline void setDate(const QDate &new_date)
	{
		if (m_date != new_date) {
			m_date = new_date;
			createMethods();
			emit dateChanged();
		}
	}

	Q_INVOKABLE inline QDate resultDate(const int method_index) const { return m_calMethods.at(method_index).methodFunciton(); }

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final
	{
		const auto method{index.row()};
		switch (role) {
		case labelRole: return m_calMethods.at(method).methodName;
		}
		return QVariant{};
	}

	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return MethodsCount; }

signals:
	void dateChanged();

private:
	QDate m_date;
	QHash<int, QByteArray> m_roleNames;
	QList<st_methodData> m_calMethods;

	void createMethods()
	{
		m_calMethods = std::move(QList<st_methodData>{
			st_methodData{m_date == QDate::currentDate() ? tr("Today") :
									appUtils()->formatDate(m_date, TPUtils::DF_LOCALE), [this] () -> QDate { return m_date; }},
			st_methodData{m_date == QDate::currentDate() ? tr("Tomorrow"): tr("The next day"),
																				[this] () -> QDate { return m_date.addDays(1); }},
			st_methodData{m_date == QDate::currentDate() ? tr("Yesterday") : tr("The previous days"),
																				[this] () -> QDate { return m_date.addDays(-1); }},
			st_methodData{tr("Next week"), [this] () -> QDate { return m_date.addDays(7); }},
			st_methodData{tr("Previous week"), [this] () -> QDate { return m_date.addDays(-7); }},
			st_methodData{tr("Two weeks ahead"), [this] () -> QDate { return m_date.addDays(14); }},
			st_methodData{tr("Two week before"), [this] () -> QDate { return m_date.addDays(-14); }},
			st_methodData{tr("15 days ahead"), [this] () -> QDate { return m_date.addDays(15); }},
			st_methodData{tr("15 days before"), [this] () -> QDate { return m_date.addDays(-15); }},
			st_methodData{tr("Next month"), [this] () -> QDate { return m_date.addMonths(1); }},
			st_methodData{tr("Previous month"), [this] () -> QDate { return m_date.addMonths(-1); }},
			st_methodData{tr("30 days ahead"), [this] () -> QDate { return m_date.addDays(30); }},
			st_methodData{tr("30 days before"), [this] () -> QDate { return m_date.addDays(-30); }},
			st_methodData{tr("Two months ahead"), [this] () -> QDate { return m_date.addMonths(2); }},
			st_methodData{tr("Two months before"), [this] () -> QDate { return m_date.addMonths(-2); }},
			st_methodData{tr("60 days ahead"), [this] () -> QDate { return m_date.addDays(60); }},
			st_methodData{tr("60 days before"), [this] () -> QDate { return m_date.addDays(-60); }},
		});
	}
};
