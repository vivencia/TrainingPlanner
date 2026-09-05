#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QQuickItem>

#include "dbmesocyclesmodel.h"

class HomePageMesoModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(HomePageMesoModel)
QML_UNCREATABLE("")

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(uint viewIndex READ viewIndex CONSTANT FINAL)
Q_PROPERTY(QString viewTitle READ viewTitle CONSTANT FINAL)
Q_PROPERTY(QString backgroundColor READ backgroundColor CONSTANT FINAL)
Q_PROPERTY(int type READ type CONSTANT FINAL)
Q_PROPERTY(bool canHaveTodaysWorkout READ canHaveTodaysWorkout NOTIFY canHaveTodaysWorkoutChanged FINAL)
Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

public:
	Q_DISABLE_COPY_MOVE(HomePageMesoModel)
	explicit HomePageMesoModel(DBMesocyclesModel *meso_model, const DBMesocyclesModel::MesoType type, const uint view_index);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif
	inline uint count() const { return m_mesoModelRows.count(); }
	inline uint viewIndex() const { return m_viewIndex; }
	inline int type() const { return static_cast<int>(m_type); }
	bool canHaveTodaysWorkout() const;
	inline int currentIndex() const { return m_curIndex; }
	void setCurrentIndex(const int new_index);
	inline void setCurrentIndexViaMesoIdx(const int meso_idx)
	{
		setCurrentIndex(findLocalIdx(meso_idx));
	}
	Q_INVOKABLE inline int currentMesoIdx() const
	{
		return (m_curIndex >= 0 && m_curIndex < m_mesoModelRows.count()) ? m_mesoModelRows.at(m_curIndex) : -1;
	}

	QString viewTitle() const {
		switch (m_type) {
		case DBMesocyclesModel::MT_MESO_FROM_COACH: return tr("My Coaches's Programs");
		case DBMesocyclesModel::MT_MESO_FOR_SELF: return tr("My Own Programs");
		case DBMesocyclesModel::MT_MESO_FOR_CLIENT: return tr("My Clients' Programs");
		default: Q_UNREACHABLE();
		}
	}
	QString backgroundColor() const;

	Q_INVOKABLE inline DBMesocyclesModel *mesoModel() const { return m_mesoModel; }
	Q_INVOKABLE void showOptionsMenu(QQuickItem *tpbutton, const int meso_idx);

	void appendMesoIdx(const uint meso_idx);
	void removeMesoIdx(const uint meso_idx);

	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }
	QVariant data(const QModelIndex &index, int role) const override final;
	inline virtual int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }

signals:
	void countChanged();
	void currentIndexChanged();
	void canHaveTodaysWorkoutChanged();

private:
	QList<uint> m_mesoModelRows;
	QHash<int, QByteArray> m_roleNames;
	DBMesocyclesModel *m_mesoModel;
	int m_curIndex{-1};
	uint m_viewIndex;
	DBMesocyclesModel::MesoType m_type;

	inline int findLocalIdx(const uint meso_idx) const { return m_mesoModelRows.indexOf(meso_idx); }
};
