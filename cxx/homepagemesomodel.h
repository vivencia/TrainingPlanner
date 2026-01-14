#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)

class HomePageMesoModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(bool ownMesosModel READ ownMesosModel CONSTANT FINAL)
Q_PROPERTY(bool canHaveTodaysWorkout READ canHaveTodaysWorkout NOTIFY canHaveTodaysWorkoutChanged FINAL)
Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

public:
	explicit HomePageMesoModel(DBMesocyclesModel *meso_model, const bool own_mesos);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif
	inline uint count() const { return m_mesoModelRows.count(); }
	inline bool ownMesosModel() const { return m_ownMesos; }
	bool canHaveTodaysWorkout() const;
	inline int currentIndex() const { return m_curIndex; }
	void setCurrentIndex(const int new_index);
	inline void setCurrentIndexViaMesoIdx(const int meso_idx)
	{
		setCurrentIndex(findLocalIdx(meso_idx));
	}
	inline int currentMesoIdx() const { return m_curIndex < m_mesoModelRows.count() ? m_mesoModelRows.at(m_curIndex) : -1; }

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
	int m_curIndex;
	bool m_ownMesos;

	inline int findLocalIdx(const uint meso_idx) const { return m_mesoModelRows.indexOf(meso_idx); }
};
