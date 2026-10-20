#pragma once

#include "tplistmodel.h"

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)

class HomePageMesoModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(HomePageMesoModel)
QML_UNCREATABLE("")

Q_PROPERTY(QString viewTitle READ viewTitle CONSTANT FINAL)
Q_PROPERTY(QString backgroundColor READ backgroundColor CONSTANT FINAL)
Q_PROPERTY(int type READ type CONSTANT FINAL)
Q_PROPERTY(int viewIndex READ viewIndex CONSTANT FINAL)
Q_PROPERTY(bool canHaveTodaysWorkout READ canHaveTodaysWorkout NOTIFY canHaveTodaysWorkoutChanged FINAL)

public:
	explicit HomePageMesoModel(DBMesocyclesModel *meso_model, const int type, const int view_index);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif
	inline int type() const { return static_cast<int>(m_type); }
	inline int viewIndex() const { return m_viewIndex; }
	bool canHaveTodaysWorkout() const;
	inline void setCurrentIndexViaMesoIdx(const int meso_idx)
	{
		setCurrentRowViaRealRow(meso_idx);
	}
	Q_INVOKABLE inline int currentMesoIdx() const
	{
		return realRow(currentRow());
	}

	QString viewTitle() const;
	QString backgroundColor() const;
	void appendNewMeso() {
		beginInsertRows(QModelIndex{}, count(), count());
		endInsertRows();
	}

	Q_INVOKABLE inline DBMesocyclesModel *mesoModel() const { return m_mesoModel; }
	Q_INVOKABLE void showOptionsMenu(QQuickItem *tpbutton, const int meso_idx);

	//--------------- Base Class overrides -------------------
	void setFieldsNames() override final;
	const QString &dataValue(const uint real_row, const uint column) const override final;
	QVariant data(const uint role, const uint row, const int column = -1) const override final;
	//--------------- Base Class overrides -------------------

signals:
	void canHaveTodaysWorkoutChanged();

private:
	DBMesocyclesModel *m_mesoModel;
	int m_type, m_viewIndex;
	QString m_dummy;

	void addFilters();
};
