#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QQmlEngine>

using namespace Qt::Literals;

class TPLogs : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPLogs)
QML_UNCREATABLE("")

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(LogType type READ type CONSTANT FINAL)
Q_PROPERTY(QString categoryName READ categoryName CONSTANT FINAL)

public:
	enum LogType {
		LT_MESSAGES,
		LT_INFO,
		LT_DEBUG
	};
	Q_ENUM(LogType)

	Q_DISABLE_COPY_MOVE(TPLogs)
	explicit TPLogs(LogType type, const QString &category, QObject *parent = nullptr);

	inline int lastEntry() const { return m_entries.count() - 1; }
	inline uint count() const { return m_entries.count(); }
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
	QString d_time(const int row = -1, const bool full_time = true) const;

	void appendLog(const QString &origin, QString &&title, QString &&message);
	Q_INVOKABLE inline void removeEntry(const int entry){
		if (entry >= 0 && entry < m_entries.count())
			remove({entry});
	}

	Q_INVOKABLE void copyLog(const int entry);
	Q_INVOKABLE void removeSelected();
	Q_INVOKABLE void setSelected(const int entry, const bool selected)
	{
		m_entries[entry].selected = selected;
	}
	Q_INVOKABLE bool isSelected(const int entry) const
	{
		if (entry >= 0 && entry < m_entries.count())
			return m_entries.at(entry).selected;
		return false;
	}

	inline QString condensedLog(const uint entry) const
	{
		return title(entry) % '[' % d_time(entry) % "] "_L1 % message(entry);
	}
	inline QString condensedLog_fancy(const uint entry) const
	{
		return "<b>"_L1 % title(entry) % "</b>["_L1 % d_time(entry) % "]<br><mark>"_L1 % message(entry) % "</mark>"_L1;
	}

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();
	void selectedChanged();
	void entryRemoved(const int entry);

private:
	LogType m_type;
	QString m_dummy, m_category, m_origin;
	QHash<int, QByteArray> m_roleNames;

	struct st_LogEntry {
		QString origin, title, message;
		QDateTime d_time;
		bool selected;
	};

	QList<st_LogEntry> m_entries;

	void remove(const QList<int> &entries);
};

