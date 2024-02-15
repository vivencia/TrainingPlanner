#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	// Define the role names to be used
	enum RoleNames {
		mesoSplitIdRole = Qt::UserRole,
		mesoSplitMesoIdRole = Qt::UserRole+1,
		splitARole = Qt::UserRole+2,
		splitBRole = Qt::UserRole+3,
		splitCRole = Qt::UserRole+4,
		splitDRole = Qt::UserRole+5,
		splitERole = Qt::UserRole+6,
		splitFRole = Qt::UserRole+7
	};

	explicit DBMesoSplitModel(QObject *parent = nullptr);

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 8; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};

#endif // DBMESOSPLITMODEL_H
