#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)

class HomePageMesoModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:
	explicit HomePageMesoModel(DBMesocyclesModel *meso_model = nullptr);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif
	Q_INVOKABLE inline uint count() const { return m_mesoModelRows.count(); }
	Q_INVOKABLE inline uint mesoRow(const uint row) const { return row < m_mesoModelRows.count() ? m_mesoModelRows.at(row) : row; }
	Q_INVOKABLE inline int findMesoIdx(const uint meso_idx) const { return m_mesoModelRows.indexOf(meso_idx); }

	void appendData(const uint mesoModelRow);
	void removeRow(const uint row);
	inline int indexOf(const uint mesorow) const { return m_mesoModelRows.indexOf(mesorow); }

	inline void removeData(const uint mesorow)
	{
		const qsizetype row{m_mesoModelRows.indexOf(mesorow)};
		removeRow(row);
	}

	inline void emitDataChanged(const uint meso_idx, const int role)
	{
		const int row{findMesoIdx(meso_idx)};
		if (row >= 0)
			emit dataChanged(index(row, 0), index(row, 0), role >= 0 ? QList<int>{} << role : QList<int>{});
	}

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();

private:
	QList<uint> m_mesoModelRows;
	QHash<int, QByteArray> m_roleNames;
	DBMesocyclesModel *m_mesoModel;
};

