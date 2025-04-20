#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class homePageMesoModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:
	explicit homePageMesoModel(QObject *parent = nullptr);
	Q_INVOKABLE inline uint count() const { return m_modeldata2.count(); }
	Q_INVOKABLE inline uint mesoRow(const uint row) const { return m_mesoModelRows.at(row); }
	Q_INVOKABLE inline int findMesoIdx(const uint meso_idx) const { return m_mesoModelRows.indexOf(meso_idx); }

	void appendData(const QStringList& modeldata, const uint mesoModelRow);
	void removeRow(const uint row);

	inline void removeData(const QStringList &modeldata)
	{
		const qsizetype row{m_modeldata2.indexOf(&modeldata)};
		removeRow(row);
	}
	inline void removeData(const uint mesorow)
	{
		const qsizetype row{m_mesoModelRows.indexOf(mesorow)};
		removeRow(row);
	}

	inline void emitDataChanged(const uint meso_idx, const int role)
	{
		const int row{findMesoIdx(meso_idx)};
		if (row >= 0)
			emit dataChanged(index(row, 0), index(row, 0), QList<int>{} << role);
	}

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline virtual int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();

private:
	QList<const QStringList*> m_modeldata2;
	QList<uint> m_mesoModelRows;
	QHash<int, QByteArray> m_roleNames;
};

