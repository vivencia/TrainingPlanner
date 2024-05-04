#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include "tplistmodel.h"

class DBExercisesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:	
	// Define the role names to be used
	enum RoleNames {
		exerciseIdRole = Qt::UserRole,
		mainNameRole = Qt::UserRole+1,
		subNameRole = Qt::UserRole+2,
		muscularGroupRole = Qt::UserRole+3,
		nSetsRole = Qt::UserRole+4,
		nRepsRole = Qt::UserRole+5,
		nWeightRole = Qt::UserRole+6,
		uWeightRole = Qt::UserRole+7,
		mediaPathRole = Qt::UserRole+8,
		actualIndexRole = Qt::UserRole+9
	};

	explicit DBExercisesModel(QObject *parent = 0);
	Q_INVOKABLE void manageSelectedEntries(const uint index, const uint operation);
	Q_INVOKABLE QString selectedEntriesValues(const uint field) const;
	Q_INVOKABLE QList<uint> selectedEntries() const { return m_selectedEntries; }

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 10; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
	QList<uint> m_selectedEntries;
};

#endif // DBEXERCISESMODEL_H
