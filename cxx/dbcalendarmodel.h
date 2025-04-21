#pragma once

#include <QAbstractItemModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesoCalendarModel);
class DBCalendarModel : public QAbstractItemModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:
	explicit DBCalendarModel(DBMesoCalendarModel *parent, const uint meso_idx, const uint n_months);
	inline uint count() const { return m_nmonths; }


	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int) override final;
	inline virtual int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();

private:
	DBMesoCalendarModel* m_calendarManager;
	uint m_mesoIdx, m_nmonths;
	QHash<int, QByteArray> m_roleNames;
};
