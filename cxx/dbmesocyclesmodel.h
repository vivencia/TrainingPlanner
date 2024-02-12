#ifndef DBMESOCYCLESMODEL_H
#define DBMESOCYCLESMODEL_H

#include "tplistmodel.h"

class DBMesocyclesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	// Define the role names to be used
	enum RoleNames {
		mesoIdRole = Qt::UserRole,
		mesoNameRole = Qt::UserRole+1,
		mesoStartDateRole = Qt::UserRole+2,
		mesoEndDateRole = Qt::UserRole+3,
		mesoNoteRole = Qt::UserRole+4,
		mesoWeeksRole = Qt::UserRole+5,
		mesoSplitRole = Qt::UserRole+6,
		mesoDrugsRole = Qt::UserRole+7,
		realMesoRole = Qt::UserRole+8,
	};

	explicit DBMesocyclesModel(QObject *parent = 0);

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 8; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

};

#endif // DBMESOCYCLESMODEL_H
