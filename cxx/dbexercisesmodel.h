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

public:
	explicit DBExercisesModel(QObject* parent = nullptr, const bool bMainExercisesModel = true);
	void fillColumnNames();

	inline const QString& id(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_ID); }
	inline const int _id(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_ID).toInt(); }
	inline void setId(const uint index, const QString& new_id) { m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_ID] = new_id; }

	Q_INVOKABLE inline QString mainName(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_MAINNAME); }
	Q_INVOKABLE inline void setMainName(const uint index, const QString& new_name)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_MAINNAME] = new_name;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString subName(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_SUBNAME); }
	Q_INVOKABLE inline void setSubName(const uint index, const QString& new_name)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_SUBNAME] = new_name;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString muscularGroup(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_MUSCULARGROUP); }
	Q_INVOKABLE inline void setMuscularGroup(const uint index, const QString& new_group)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_MUSCULARGROUP] = new_group;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString setsNumber(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_SETSNUMBER); }
	Q_INVOKABLE inline void setSetsNumber(const uint index, const QString& new_nsets)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_SETSNUMBER] = new_nsets;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString repsNumber(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_REPSNUMBER); }
	Q_INVOKABLE inline void setRepsNumber(const uint index, const QString& new_nreps)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_REPSNUMBER] = new_nreps;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString weight(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_WEIGHT); }
	Q_INVOKABLE inline void setWeight(const uint index, const QString& new_weight)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_WEIGHT] = new_weight;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString weightUnit(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_WEIGHTUNIT); }
	Q_INVOKABLE inline void setWeightUnit(const uint index, const QString& new_unit)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_WEIGHTUNIT] = new_unit;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString mediaPath(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_MEDIAPATH); }
	Q_INVOKABLE inline void setMediaPath(const uint index, const QString& new_path)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_MEDIAPATH] = new_path;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline uint actualIndex(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_ACTUALINDEX).toUInt(); }
	Q_INVOKABLE inline void setActualIndex(const uint index, const uint new_index)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_ACTUALINDEX] = QString::number(new_index);
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline bool isSelected(const uint index) const { return m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_SELECTED) == Qt::StringLiterals::operator""_L1("1", 1); }
	Q_INVOKABLE inline void setSelected(const uint index, const bool selected)
	{
		m_modeldata[m_indexProxy.at(index)][EXERCISES_COL_SELECTED] = selected ? Qt::StringLiterals::operator""_L1("1", 1) : Qt::StringLiterals::operator""_L1("0", 1);
		addModifiedIndex(index);
	}

	inline uint count() const override { return m_indexProxy.count(); }
	Q_INVOKABLE void newExercise(const QString& name = QString(), const QString& subname = QString(), const QString& muscular_group = QString());
	Q_INVOKABLE void removeExercise(const uint index);
	Q_INVOKABLE void setFilter(const QString& filter);
	Q_INVOKABLE void search(const QString& search_term);
	Q_INVOKABLE QString getFilter() const { return m_filterString; }

	Q_INVOKABLE void clearSelectedEntries();
	Q_INVOKABLE bool manageSelectedEntries(uint index, const uint max_selected = 1);
	Q_INVOKABLE QString selectedEntriesValue(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index).real_index).at(field); }

	inline const QString& selectedEntriesValue_fast(const uint index, const uint field) const { return m_modeldata.at(m_selectedEntries.at(index).real_index).at(field); }
	inline uint selectedEntriesCount() const { return m_selectedEntries.count(); }

	inline void clearModifiedIndices() { m_modifiedIndices.clear(); }
	inline void addModifiedIndex(const uint index)
	{
		m_modifiedIndices[index]++;
		if (m_modifiedIndices.value(index) == 5)
			emit exerciseChanged(index);
	}

	inline uint modifiedIndicesCount() const { return m_modifiedIndices.count(); }
	inline uint modifiedIndex(const uint pos) const
	{
		uint ret_pos(0);
		QMap<uint,uint>::const_iterator itr{m_modifiedIndices.constBegin()};
		const QMap<uint,uint>::const_iterator& itr_end{m_modifiedIndices.constEnd()};
		while (itr != itr_end)
		{
			if (pos == ret_pos)
				break;
			++itr;
			++ret_pos;
		}
		return itr.value();
	}

	void setLastID(uint exercisesTableLastId) { m_exercisesTableLastId = exercisesTableLastId; }
	inline int lastID() const { return m_exercisesTableLastId; }
	bool collectExportData();

	inline QStringList& lastRow() { return m_modeldata.last(); }
	void appendList(const QStringList& list);
	void appendList(QStringList&& list);
	void clear() override;
	QString makeTransactionStatementForDataBase(const uint index) const;

	inline void resetPrivateData() override { clearSelectedEntries(); }
	int importFromFile(const QString& filename) override final;
	bool updateFromModel(const TPListModel* const model) override final;

	int columnCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return numberOfFields(); }
	inline int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex& index, int role) const override final;
	[[maybe_unused]] bool setData(const QModelIndex& index, const QVariant &value, int role) override final;

signals:
	void exerciseChanged(const uint index);

private:
	typedef struct st_SelEntry{
		uint real_index;
		uint view_index;

		explicit inline st_SelEntry() : real_index(0), view_index(0) {}
	} selectedEntry;

	QList<uint> m_filteredIndices;
	QList<uint> m_indexProxy;
	QMap<uint,uint> m_modifiedIndices;
	QList<selectedEntry> m_selectedEntries;
	uint m_selectedEntryToReplace;
	int m_exercisesTableLastId;
	bool m_bFilterApplied;

	static DBExercisesModel* app_exercises_model;
	friend DBExercisesModel* appExercisesModel();
};

inline DBExercisesModel* appExercisesModel() { return DBExercisesModel::app_exercises_model; }
#endif // DBEXERCISESMODEL_H
