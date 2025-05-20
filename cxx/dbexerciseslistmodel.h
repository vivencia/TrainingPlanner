#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#define EXERCISES_LIST_COL_ID 0
#define EXERCISES_LIST_COL_MAINNAME 1
#define EXERCISES_LIST_COL_SUBNAME 2
#define EXERCISES_LIST_COL_MUSCULARGROUP 3
#define EXERCISES_LIST_COL_SETSNUMBER 4
#define EXERCISES_LIST_COL_REPSNUMBER 5
#define EXERCISES_LIST_COL_WEIGHT 6
#define EXERCISES_LIST_COL_WEIGHTUNIT 7
#define EXERCISES_LIST_COL_MEDIAPATH 8
#define EXERCISES_LIST_COL_FROMAPPLIST 9
#define EXERCISES_LIST_COL_ACTUALINDEX 10
#define EXERCISES_LIST_COL_SELECTED 11
#define EXERCISES_TOTAL_COLS EXERCISES_LIST_COL_SELECTED+1

QT_FORWARD_DECLARE_CLASS(QFile)

class DBExercisesListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelsChanged)
Q_PROPERTY(QString exerciseSpecificsLabel READ exerciseSpecificsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString muscularGroupsLabel READ muscularGroupsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString setsLabel READ setsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString repsLabel READ repsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString weightLabel READ weightLabel NOTIFY labelsChanged)
Q_PROPERTY(QString mediaLabel READ mediaLabel NOTIFY labelsChanged)

enum RoleNames {
	exerciseIdRole = Qt::UserRole,
	mainNameRole = Qt::UserRole+EXERCISES_LIST_COL_MAINNAME,
	subNameRole = Qt::UserRole+EXERCISES_LIST_COL_SUBNAME,
	muscularGroupRole = Qt::UserRole+EXERCISES_LIST_COL_MUSCULARGROUP,
	nSetsRole = Qt::UserRole+EXERCISES_LIST_COL_SETSNUMBER,
	nRepsRole = Qt::UserRole+EXERCISES_LIST_COL_REPSNUMBER,
	nWeightRole = Qt::UserRole+EXERCISES_LIST_COL_WEIGHT,
	uWeightRole = Qt::UserRole+EXERCISES_LIST_COL_WEIGHTUNIT,
	mediaPathRole = Qt::UserRole+EXERCISES_LIST_COL_MEDIAPATH,
	fromListRole = Qt::UserRole+EXERCISES_LIST_COL_FROMAPPLIST,
	actualIndexRole = Qt::UserRole+EXERCISES_LIST_COL_ACTUALINDEX,
	selectedRole = Qt::UserRole+EXERCISES_LIST_COL_SELECTED
};

public:
	explicit DBExercisesListModel(QObject *parent = nullptr, const bool bMainExercisesModel = true);
	void fillColumnNames();

	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString exerciseSpecificsLabel() const { return tr("Specifics: "); }
	inline QString muscularGroupsLabel() const { return tr("Muscular Groups: "); }
	inline QString setsLabel() const { return tr("Sets: "); }
	inline QString repsLabel() const { return tr("Reps: "); }
	inline QString weightLabel() const { return tr("Weight: "); }
	inline QString mediaLabel() const { return tr("Descriptive media: "); }

	inline const QString &id(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_ID); }
	inline const int _id(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_ID).toInt(); }
	inline void setId(const uint index, const QString &new_id) { m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_ID] = new_id; }

	Q_INVOKABLE inline QString mainName(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_MAINNAME); }
	Q_INVOKABLE inline void setMainName(const uint index, const QString &new_name)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_MAINNAME] = new_name;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString subName(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_SUBNAME); }
	Q_INVOKABLE inline void setSubName(const uint index, const QString &new_name)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_SUBNAME] = new_name;
		addModifiedIndex(index);
	}

	Q_INVOKABLE QString muscularGroup(const uint index) const;
	Q_INVOKABLE inline void setMuscularGroup(const uint index, const QString &new_group)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_MUSCULARGROUP] = new_group;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString setsNumber(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_SETSNUMBER); }
	Q_INVOKABLE inline void setSetsNumber(const uint index, const QString &new_nsets)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_SETSNUMBER] = new_nsets;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString repsNumber(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_REPSNUMBER); }
	Q_INVOKABLE inline void setRepsNumber(const uint index, const QString &new_nreps)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_REPSNUMBER] = new_nreps;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString weight(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_WEIGHT); }
	Q_INVOKABLE inline void setWeight(const uint index, const QString &new_weight)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_WEIGHT] = new_weight;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString weightUnit(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_WEIGHTUNIT); }
	Q_INVOKABLE inline void setWeightUnit(const uint index, const QString &new_unit)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_WEIGHTUNIT] = new_unit;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline QString mediaPath(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_MEDIAPATH); }
	Q_INVOKABLE inline void setMediaPath(const uint index, const QString &new_path)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_MEDIAPATH] = new_path;
		addModifiedIndex(index);
	}

	Q_INVOKABLE inline uint actualIndex(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_ACTUALINDEX).toUInt(); }
	Q_INVOKABLE inline void setActualIndex(const uint index, const uint new_index)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_ACTUALINDEX] = QString::number(new_index);
	}

	Q_INVOKABLE inline bool isSelected(const uint index) const { return m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_SELECTED) == Qt::StringLiterals::operator""_L1("1", 1); }
	Q_INVOKABLE inline void setSelected(const uint index, const bool selected)
	{
		m_exercisesData[m_indexProxy.at(index)][EXERCISES_LIST_COL_SELECTED] = selected ? Qt::StringLiterals::operator""_L1("1", 1) : Qt::StringLiterals::operator""_L1("0", 1);
	}

	inline uint count() const { return m_indexProxy.count(); }
	inline int currentRow() const { return m_currentRow; }
	void setCurrentRow(const int row);

	Q_INVOKABLE void newExercise(const QString &name = QString{}, const QString &subname = QString{},
											const QString &muscular_group = QString{});
	Q_INVOKABLE void removeExercise(const uint index);
	Q_INVOKABLE void setFilter(const QString &filter);
	Q_INVOKABLE void search(const QString &search_term);
	Q_INVOKABLE QString getFilter() const { return m_filterString; }

	Q_INVOKABLE void clearSelectedEntries();
	Q_INVOKABLE bool manageSelectedEntries(uint index, const uint max_selected = 1);
	Q_INVOKABLE QString selectedEntriesValue(const uint index, const uint field) const { return m_exercisesData.at(m_selectedEntries.at(index).real_index).at(field); }

	inline const QString &selectedEntriesValue_fast(const uint index, const uint field) const { return m_exercisesData.at(m_selectedEntries.at(index).real_index).at(field); }
	inline uint selectedEntriesCount() const { return m_selectedEntries.count(); }

	inline void clearModifiedIndices() { m_modifiedIndices.clear(); }
	inline void addModifiedIndex(const uint index)
	{
		if (!m_modifiedIndices.contains(index))
			m_modifiedIndices.append(index);
	}

	inline uint modifiedIndicesCount() const { return m_modifiedIndices.count(); }
	inline uint modifiedIndex(const uint pos) const { return m_modifiedIndices.at(pos); }

	void setLastID(uint exercisesTableLastId) { m_exercisesTableLastId = exercisesTableLastId; }
	inline int lastID() const { return m_exercisesTableLastId; }
	bool collectExportData();

	inline QStringList &lastRow() { return m_exercisesData.last(); }
	void appendList(const QStringList &list);
	void appendList(QStringList &&list);
	void clear();
	QString makeTransactionStatementForDataBase(const uint index) const;

	int exportToFile(const QString &filename, QFile *out_file = nullptr) const;
	int exportToFormattedFile(const QString &filename, QFile *out_file = nullptr) const;
	int importFromFile(const QString &filename, QFile *in_file = nullptr);
	int importFromFormattedFile(const QString &filename, QFile *in_file = nullptr);
	int newExerciseFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex &index, int role) const override final;
	[[maybe_unused]] bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void labelsChanged();
	void currentRowChanged();

private:
	typedef struct st_SelEntry{
		uint real_index;
		uint view_index;

		explicit inline st_SelEntry() : real_index{0}, view_index{0} {}
	} selectedEntry;

	QList<QStringList> m_exercisesData;
	QList<uint> m_filteredIndices;
	QList<uint> m_indexProxy;
	QList<uint> m_modifiedIndices;
	QList<uint> m_exportRows;
	QHash<int, QByteArray> m_roleNames;
	QList<selectedEntry> m_selectedEntries;
	QString m_filterString;
	uint m_selectedEntryToReplace;
	int m_exercisesTableLastId, m_currentRow;
	bool m_bFilterApplied;

	static DBExercisesListModel *app_exercises_list;
	friend DBExercisesListModel *appExercisesList();
};

inline DBExercisesListModel *appExercisesList() { return DBExercisesListModel::app_exercises_list; }
