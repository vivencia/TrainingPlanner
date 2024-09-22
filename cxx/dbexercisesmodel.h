#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include "tplistmodel.h"

#define EXERCISES_COL_ID 0
#define EXERCISES_COL_MAINNAME 1
#define EXERCISES_COL_SUBNAME 2
#define EXERCISES_COL_MUSCULARGROUP 3
#define EXERCISES_COL_SETSNUMBER 4
#define EXERCISES_COL_REPSNUMBER 5
#define EXERCISES_COL_WEIGHT 6
#define EXERCISES_COL_WEIGHTUNIT 7
#define EXERCISES_COL_MEDIAPATH 8
#define EXERCISES_COL_FROMAPPLIST 9
#define EXERCISES_COL_ACTUALINDEX 10
#define EXERCISES_COL_SELECTED 11
#define EXERCISES_TOTAL_COLS EXERCISES_COL_SELECTED+1

class DBExercisesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:	
	// Define the role names to be used
	enum RoleNames {
		exerciseIdRole = Qt::UserRole,
		mainNameRole = Qt::UserRole+EXERCISES_COL_MAINNAME,
		subNameRole = Qt::UserRole+EXERCISES_COL_SUBNAME,
		muscularGroupRole = Qt::UserRole+EXERCISES_COL_MUSCULARGROUP,
		nSetsRole = Qt::UserRole+EXERCISES_COL_SETSNUMBER,
		nRepsRole = Qt::UserRole+EXERCISES_COL_REPSNUMBER,
		nWeightRole = Qt::UserRole+EXERCISES_COL_WEIGHT,
		uWeightRole = Qt::UserRole+EXERCISES_COL_WEIGHTUNIT,
		mediaPathRole = Qt::UserRole+EXERCISES_COL_MEDIAPATH,
		fromListRole = Qt::UserRole+EXERCISES_COL_FROMAPPLIST,
		actualIndexRole = Qt::UserRole+EXERCISES_COL_ACTUALINDEX,
		selectedRole = Qt::UserRole+EXERCISES_COL_SELECTED
	};

	explicit DBExercisesModel(QObject* parent = nullptr);

	Q_INVOKABLE inline void setMainName(const QString& new_name) { set(currentRow(), EXERCISES_COL_MAINNAME, new_name); addModifiedIndex(currentRow()); }
	Q_INVOKABLE inline void setSubNameName(const QString& new_name) { set(currentRow(), EXERCISES_COL_SUBNAME, new_name); addModifiedIndex(currentRow()); }
	Q_INVOKABLE inline void setMuscularGroup(const QString& new_group) { set(currentRow(), EXERCISES_COL_MUSCULARGROUP, new_group); addModifiedIndex(currentRow()); }
	Q_INVOKABLE inline void setSetsNumber(const QString& new_nsets) { set(currentRow(), EXERCISES_COL_SETSNUMBER, new_nsets); addModifiedIndex(currentRow()); }
	Q_INVOKABLE inline void setRepsNumber(const QString& new_nreps) { set(currentRow(), EXERCISES_COL_REPSNUMBER, new_nreps); addModifiedIndex(currentRow()); }
	Q_INVOKABLE inline void setWeight(const QString& new_weight) { set(currentRow(), EXERCISES_COL_WEIGHT, new_weight); addModifiedIndex(currentRow()); }

	Q_INVOKABLE void clearSelectedEntries();
	Q_INVOKABLE bool manageSelectedEntries(uint index, const uint max_selected = 1);
	Q_INVOKABLE QString selectedEntriesValue(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index).real_index).at(field); }
	inline const QString& selectedEntriesValue_fast(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index).real_index).at(field); }
	inline uint selectedEntriesCount() const { return m_selectedEntries.count(); }
	void setLastID(uint exercisesTableLastId) { m_exercisesTableLastId = exercisesTableLastId; }
	inline uint lastID() const { return m_exercisesTableLastId; }
	bool collectExportData();

	Q_INVOKABLE virtual void clear() override;
	inline virtual void resetPrivateData() override { clearSelectedEntries(); }
	virtual int importFromFile(const QString& filename) override;

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 10; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
	typedef struct {
		uint real_index;
		uint view_index;
	} selectedEntry;

	QList<selectedEntry> m_selectedEntries;
	uint m_selectedEntryToReplace;
	uint m_exercisesTableLastId;
};

#endif // DBEXERCISESMODEL_H
