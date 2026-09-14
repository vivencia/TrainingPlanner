#include "tplogs.h"

#include "tputils.h"

enum LogRoleNames {
	createRole(logOrigin, Qt::UserRole)
	createRole(logTitle, Qt::UserRole + 1)
	createRole(logMessage, Qt::UserRole + 2)
	createRole(logTime, Qt::UserRole + 3)
	createRole(logTooltip, Qt::UserRole + 4)
};

TPLogs::TPLogs(LogType type, const QString &category, QObject *parent)
	: QAbstractListModel{parent}, m_type{type}, m_category{category}
{
	roleToString(logOrigin)
	roleToString(logTitle)
	roleToString(logMessage)
	roleToString(logTime)
	roleToString(logTooltip)
}

QString TPLogs::d_time(const int row, const bool full_time) const
{
	if (m_entries.isEmpty())
		return QString{};
	const st_LogEntry *l_entry{nullptr};
	l_entry = row <= -1 ? &m_entries.constLast() : &m_entries.at(row);
	return appUtils()->formatDateTime(l_entry->d_time, full_time ? static_cast<int>(TPUtils::DTF_LOCALE)
																 : static_cast<int>(TPUtils::TF_QML_DISPLAY_COMPLETE)
											   , QLatin1Char{' '}).chopped(1);
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

void TPLogs::removeSelected()
{
	QList<int> indexes;
	int index{0};
	for (const auto &entry : std::as_const(m_entries)) {
		if (entry.selected)
			indexes.append(index);
		++index;
	}
	if (!indexes.isEmpty())
		remove(indexes);
}

QVariant TPLogs::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_entries.count()) {
		switch (role) {
		case logOriginRole: return origin(row);
		case logTitleRole: return title(row);
		case logMessageRole: return message(row);
		case logTimeRole: return d_time(row);
		case logTooltipRole: return condensedLog_fancy(row);
		default: break;
		}
	}
	return QVariant{};
}

void TPLogs::remove(const QList<int> &entries)
{
	beginRemoveRows(QModelIndex{}, entries.constFirst(), entries.constLast());
	for (const auto entry : entries | std::views::reverse) {
		m_entries.removeAt(entry);
		emit entryRemoved(entry);
	}
	endRemoveRows();
}
