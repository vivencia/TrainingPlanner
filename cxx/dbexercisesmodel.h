#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include "tplistmodel.h"

class DBExercisesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT
Q_PROPERTY(bool entryIsSelected READ entryIsSelected NOTIFY entryIsSelectedChanged)

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
	Q_INVOKABLE bool entryIsSelected() const { return m_selectedEntries.contains(currentRow()); }
	Q_INVOKABLE void clearSelectedEntries() { m_selectedEntries.clear(); m_selectedEntryToReplace = 0; }
	Q_INVOKABLE int manageSelectedEntries(const uint index, const uint max_selected = 1);
	Q_INVOKABLE QString selectedEntriesValue(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index)).at(field); }
	inline const QString& selectedEntriesValue_fast(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index)).at(field); }
	inline uint selectedEntriesCount() const { return m_selectedEntries.count(); }

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 10; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

signals:
	void entryIsSelectedChanged();

private:
	QList<uint> m_selectedEntries;
	uint m_selectedEntryToReplace;
};

#endif // DBEXERCISESMODEL_H
