#pragma once

#include "dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

#define EXERCISES_LIST_COL_ID 0
#define EXERCISES_LIST_COL_MAINNAME 1
#define EXERCISES_LIST_COL_SUBNAME 2
#define EXERCISES_LIST_COL_MUSCULARGROUP 3
#define EXERCISES_LIST_COL_MEDIAPATH 4
#define EXERCISES_LIST_COL_FROMAPPLIST 5
#define EXERCISES_LIST_COL_ACTUALINDEX 6
#define EXERCISES_LIST_COL_SELECTED 7
#define EXERCISES_TOTAL_COLS EXERCISES_LIST_COL_SELECTED+1

QT_FORWARD_DECLARE_CLASS(DBExercisesListTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercisesList);
QT_FORWARD_DECLARE_CLASS(QFile)

class DBExercisesListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
Q_PROPERTY(bool hasExercises READ hasExercises NOTIFY hasExercisesChanged FINAL)

Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelsChanged)
Q_PROPERTY(QString exerciseSpecificsLabel READ exerciseSpecificsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString muscularGroupsLabel READ muscularGroupsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString mediaLabel READ mediaLabel NOTIFY labelsChanged)

enum RoleNames {
	exerciseIdRole		= Qt::UserRole,
	mainNameRole		= Qt::UserRole + EXERCISES_LIST_COL_MAINNAME,
	subNameRole			= Qt::UserRole + EXERCISES_LIST_COL_SUBNAME,
	muscularGroupRole	= Qt::UserRole + EXERCISES_LIST_COL_MUSCULARGROUP,
	mediaPathRole		= Qt::UserRole + EXERCISES_LIST_COL_MEDIAPATH,
	fromListRole		= Qt::UserRole + EXERCISES_LIST_COL_FROMAPPLIST,
	actualIndexRole		= Qt::UserRole + EXERCISES_LIST_COL_ACTUALINDEX,
	selectedRole		= Qt::UserRole + EXERCISES_LIST_COL_SELECTED
};

public:
	explicit DBExercisesListModel(QObject *parent = nullptr);

	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString exerciseSpecificsLabel() const { return tr("Specifics: "); }
	inline QString muscularGroupsLabel() const { return tr("Muscular Groups: "); }
	inline QString mediaLabel() const { return tr("Descriptive media: "); }

	inline QString id(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), exerciseIdRole).toString();
	}
	inline const int _id(const uint index) const
	{
		return id(index).toUInt();
	}
	inline void setId(const uint index, const QString &new_id)
	{
		setData(QAbstractListModel::index(index, 0), new_id, exerciseIdRole);
	}

	Q_INVOKABLE inline QString mainName(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), mainNameRole).toString();
	}
	Q_INVOKABLE inline void setMainName(const uint index, const QString &new_name)
	{
		setData(QAbstractListModel::index(index, 0), new_name, mainNameRole);
	}

	Q_INVOKABLE inline QString subName(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), subNameRole).toString();
	}
	Q_INVOKABLE inline void setSubName(const uint index, const QString &new_subname)
	{
		setData(QAbstractListModel::index(index, 0), new_subname, subNameRole);
	}

	Q_INVOKABLE QString muscularGroup(const int index) const;
	Q_INVOKABLE inline void setMuscularGroup(const uint index, const QString &new_group)
	{
		setData(QAbstractListModel::index(index, 0), new_group, muscularGroupRole);
	}

	Q_INVOKABLE inline QString mediaPath(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), mediaPathRole).toString();
	}
	Q_INVOKABLE inline void setMediaPath(const uint index, const QString &new_path)
	{
		setData(QAbstractListModel::index(index, 0), new_path, mediaPathRole);
	}

	Q_INVOKABLE inline uint actualIndex(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), actualIndexRole).toUInt();
	}
	Q_INVOKABLE inline void setActualIndex(const uint index, const uint new_index)
	{
		setData(QAbstractListModel::index(index, 0), QString::number(new_index), actualIndexRole);
	}

	Q_INVOKABLE inline bool isSelected(const uint index) const
	{
		return data(QAbstractListModel::index(index, 0), selectedRole).toUInt() == 1;
	}
	Q_INVOKABLE inline void setSelected(const uint index, const bool selected)
	{
		setData(QAbstractListModel::index(index, 0), selected,  selectedRole);
	}

	inline uint count() const
	{
		if (m_searchFilterApplied)
			return m_searchFilteredIndices.count();
		else
		{
			if (m_muscularFilterApplied)
				return m_muscularFilteredIndices.count();
			else
				return m_exercisesData.count();
		}
	}

	inline int currentRow() const { return m_currentRow; }
	void setCurrentRow(const int row);

	Q_INVOKABLE inline int currentRealRow() const
	{
		if (m_currentRow == -1)
			return -1;
		if (m_searchFilterApplied)
			return m_exercisesData.at(m_searchFilteredIndices.at(m_currentRow)).at(EXERCISES_LIST_COL_SELECTED).toUInt();
		else
		{
			if (m_muscularFilterApplied)
				return m_exercisesData.at(m_muscularFilteredIndices.at(m_currentRow)).at(EXERCISES_LIST_COL_SELECTED).toUInt();
			else
				return m_currentRow;
		}
	}

	bool hasExercises() const { return !m_exercisesData.isEmpty(); };

	void newExerciseFromList(QString &&name, QString &&subname, QString &&muscular_group);
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

	bool collectExportData();

	inline QStringList &lastRow() { return m_exercisesData.last(); }
	void appendList(const QStringList &list, const bool save_to_database = false);
	void appendList(QStringList &&list, const bool save_to_database = false);
	void clear();

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
	void hasExercisesChanged();

private:
	typedef struct st_SelEntry
	{
		uint real_index;
		uint view_index;

		explicit inline st_SelEntry() : real_index{0}, view_index{0} {}
	} selectedEntry;

	QList<QStringList> m_exercisesData;
	QList<uint> m_muscularFilteredIndices;
	QList<uint> m_searchFilteredIndices;
	QList<uint> m_exportRows;
	QHash<int, QByteArray> m_roleNames;
	QList<selectedEntry> m_selectedEntries;
	QString m_filterString, m_searchString;
	uint m_selectedEntryToReplace;
	int m_currentRow;
	bool m_muscularFilterApplied, m_searchFilterApplied;
	DBModelInterfaceExercisesList *m_dbModelInterface;
	DBExercisesListTable *m_db;

	QString untranslatedMuscularGroup(const QString &translated_group) const;
	void resetSearchModel();
	QString getExercisesListVersion() const;

	static DBExercisesListModel *app_exercises_list;
	friend DBExercisesListModel *appExercisesList();
	friend class DBModelInterfaceExercisesList;
};

inline DBExercisesListModel *appExercisesList() { return DBExercisesListModel::app_exercises_list; }

class DBModelInterfaceExercisesList : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceExercisesList() : DBModelInterface{appExercisesList()} {}
	inline const QList<QStringList> &modelData() const { return appExercisesList()->m_exercisesData; }
	inline QList<QStringList> &modelData() { return appExercisesList()->m_exercisesData; }
};

