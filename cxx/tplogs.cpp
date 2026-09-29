#include "tplogs.h"

#include "tputils.h"

enum LogRoleNames {
	createRole(logOrigin, TPLogs::LF_ORIGIN)
	createRole(logTitle, TPLogs::LF_TITLE)
	createRole(logMessage, TPLogs::LF_MESSAGE)
	createRole(logTime, TPLogs::LF_TIME)
	createRole(logTooltip, TPLogs::LF_TOOLTIP)
};

TPLogs::TPLogs(LogType type, const QString &category, QObject *parent)
	: TPListModel{parent, LF_N_FIELDS}, m_type{type}, m_category{category}
{
	//setSortField(LF_MESSAGE);
	//setSortDirection(SORT_DOWN);
	setFieldsNames();
	roleToString(logOrigin)
	roleToString(logTitle)
	roleToString(logMessage)
	roleToString(logTime)
	roleToString(logTooltip)
}

const QString &TPLogs::d_time(int row, const bool full_time) const
{
	if (m_entries.isEmpty())
		return m_dummy;
	if (row == -1)
		row = m_entries.count() - 1;
	if (m_entries.at(row).m_time.isEmpty()) {
		m_entries[row].m_time = std::move(appUtils()->formatDateTime(m_entries.at(row).d_time, full_time
														? static_cast<int>(TPUtils::DTF_LOCALE)
														: static_cast<int>(TPUtils::TF_QML_DISPLAY_COMPLETE)
														, QLatin1Char{' '}).chopped(1));
	}
	return m_entries.at(row).m_time;
}

void TPLogs::appendLog(const QString &origin, QString &&title, QString &&message)
{
	beginInsertRows(QModelIndex{}, count(), count());
	st_LogEntry new_entry;
	new_entry.origin = origin;
	new_entry.title = std::forward<QString>(title);
	new_entry.message = std::forward<QString>(message);
	new_entry.d_time = std::move(QDateTime::currentDateTime());
	m_entries.append(std::move(new_entry));
	endInsertRows();
	emit countChanged();
}

void TPLogs::copyLog(const int entry)
{
	if (entry >= 0 && entry < m_entries.count())
		appUtils()->copyToClipboard(condensedLog(entry));
}

void TPLogs::setFieldsNames()
{
	m_fieldsNames.append(std::move(tr("Header")));
	m_fieldsNames.append(std::move(tr("Message")));
	m_fieldsNames.append(std::move(tr("Time")));
}

QVariant TPLogs::data(const uint role, const uint row, const int column) const
{
	switch (role) {
		case logOriginRole: return origin(row);
		case logTitleRole: return title(row);
		case logMessageRole: return message(row);
		case logTimeRole: return d_time(row);
		case logTooltipRole: return condensedLog_fancy(row);
		default: break;
	}
	return QVariant{};
}

const QString &TPLogs::_d_time(const uint row) const
{
	if (m_entries.isEmpty())
		return m_dummy;
	if (m_entries.at(row).m_ptime.isEmpty()) {
		m_entries[row].m_ptime = std::move(appUtils()->formatDateTime(m_entries.at(row).d_time,
						static_cast<int>(TPUtils::DF_DATABASE)|static_cast<int>(TPUtils::TF_DATABASE), QLatin1Char{0}));
	}
	return m_entries.at(row).m_ptime;
}

void TPLogs::remove(const QList<int> &entries)
{
	if (!entries.isEmpty()) {
		beginRemoveRows(QModelIndex{}, entries.constFirst(), entries.constLast());
		for (const auto entry : entries | std::views::reverse) {
			m_entries.removeAt(entry);
			emit entryRemoved(entry);
		}
		endRemoveRows();
	}
}
