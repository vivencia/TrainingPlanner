#pragma once

#include "tplistmodel.h"

#include <QDateTime>
#include <QQmlEngine>

using namespace Qt::Literals;

class TPLogs : public TPListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPLogs)
QML_UNCREATABLE("")

Q_PROPERTY(LogType type READ type CONSTANT FINAL)
Q_PROPERTY(QString categoryName READ categoryName CONSTANT FINAL)

public:
	enum LogType {
		LT_MESSAGES,
		LT_INFO,
		LT_DEBUG
	};
	Q_ENUM(LogType)

	enum LogFields {
		LF_ORIGIN,
		LF_TITLE,
		LF_MESSAGE,
		LF_TIME,
		LF_TOOLTIP,
		LF_N_FIELDS
	};

	Q_DISABLE_COPY_MOVE(TPLogs)
	explicit TPLogs(LogType type, const QString &category, QObject *parent = nullptr);

	inline int lastEntry() const { return m_entries.count() - 1; }
	inline LogType type() const { return m_type; }
	inline QString categoryName() const { return m_category; }
	inline const QString &origin(const int row = -1) const
	{
		return row <= -1 ? (!m_entries.isEmpty() ? m_entries.constLast().origin : m_dummy) : m_entries.at(row).origin;
	}
	inline const QString &title(const int row = -1) const
	{
		return row <= -1 ? (!m_entries.isEmpty() ? m_entries.constLast().title : m_dummy) : m_entries.at(row).title;
	}
	inline const QString &message(const int row = -1) const
	{
		return row <= -1 ? (!m_entries.isEmpty() ? m_entries.constLast().message : m_dummy) : m_entries.at(row).message;
	}
	const QString &d_time(int row = -1, const bool full_time = true) const;

	void appendLog(const QString &origin, QString &&title, QString &&message);
	Q_INVOKABLE void copyLog(const int entry);

	inline QString condensedLog(const uint entry) const
	{
		return title(entry) % '[' % d_time(entry) % "] "_L1 % message(entry);
	}
	inline const QString &condensedLog_fancy(const uint entry) const
	{
		if (m_entries.at(entry).m_tooltip.isEmpty())
			m_entries[entry].m_tooltip = std::move("<b>"_L1 % title(entry) % "</b>["_L1 % d_time(entry) % "]<br><mark>"_L1
																					% message(entry) % "</mark>"_L1);
		return m_entries.at(entry).m_tooltip;
	}

	void setFieldsNames() override final;
	inline const QString &dataValue(const uint real_row, const uint column) const override final
	{
		switch (column) {
		case LF_ORIGIN: return origin(real_row);
		case LF_TITLE: return title(real_row);
		case LF_MESSAGE: return message(real_row);
		case LF_TIME: return _d_time(real_row);
		case LF_TOOLTIP: return condensedLog_fancy(real_row);
		default: Q_UNREACHABLE_RETURN(m_dummy);
		}
	}

protected:
	QVariant data(const uint role, const uint row, const int column = -1) const override final;

private:
	LogType m_type;
	QString m_category, m_origin, m_dummy;

	struct st_LogEntry {
		QString origin, title, message, m_time, m_tooltip, m_ptime;
		QDateTime d_time;
	};

	mutable QList<st_LogEntry> m_entries;

	void addFilters();
	const QString &_d_time(const uint row) const;
};
