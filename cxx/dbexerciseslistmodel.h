#pragma once

#include "dbmodelinterface.h"

#include <QAbstractListModel>
#include <QQmlEngine>

enum ExerciseListFields {
	EXERCISES_LIST_FIELD_ID,
	EXERCISES_LIST_FIELD_MAINNAME,
	EXERCISES_LIST_FIELD_SUBNAME,
	EXERCISES_LIST_FIELD_MUSCULARGROUP,
	EXERCISES_LIST_FIELD_MEDIAPATH,
	EXERCISES_LIST_FIELD_FROMAPPLIST,
	EXERCISES_LIST_FIELD_ACTUALINDEX,
	EXERCISES_LIST_FIELD_SELECTED,
	EXERCISES_TOTAL_COLS
};

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

public:
	explicit DBExercisesListModel(QObject *parent = nullptr);
	void initExercisesList();

	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString exerciseSpecificsLabel() const { return tr("Specifics: "); }
	inline QString muscularGroupsLabel() const { return tr("Muscular Groups: "); }
	inline QString mediaLabel() const { return tr("Descriptive media: "); }

	inline const int _id(const uint index) const { return id(index).toUInt(); }
	Q_INVOKABLE QString id(const uint index) const;
	Q_INVOKABLE void setId(const uint index, const QString &new_id);
	Q_INVOKABLE QString mainName(const uint index) const;
	Q_INVOKABLE void setMainName(const uint index, const QString &new_name);
	Q_INVOKABLE QString subName(const uint index) const;
	Q_INVOKABLE void setSubName(const uint index, const QString &new_subname);
	Q_INVOKABLE QString muscularGroup(const int index) const;
	Q_INVOKABLE void setMuscularGroup(const uint index, const QString &new_group);
	Q_INVOKABLE QString mediaPath(const uint index) const;
	Q_INVOKABLE void setMediaPath(const uint index, const QString &new_path);
	Q_INVOKABLE uint actualIndex(const uint index) const;
	Q_INVOKABLE void setActualIndex(const uint index, const uint new_index);
	Q_INVOKABLE bool isSelected(const uint index) const;
	Q_INVOKABLE void setSelected(const uint index, const bool selected);

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
			return m_exercisesData.at(m_searchFilteredIndices.at(m_currentRow)).at(EXERCISES_LIST_FIELD_SELECTED).toUInt();
		else
		{
			if (m_muscularFilterApplied)
				return m_exercisesData.at(m_muscularFilteredIndices.at(m_currentRow)).at(EXERCISES_LIST_FIELD_SELECTED).toUInt();
			else
				return m_currentRow;
		}
	}

	bool hasExercises() const { return !m_exercisesData.isEmpty(); };

	Q_INVOKABLE void newExercise(const QString &name = QString{}, const QString &subname = QString{},
																	const QString &muscular_group = QString{});
	Q_INVOKABLE void removeExercise(const uint index);

	Q_INVOKABLE void setFilter(const QString &filter);
	Q_INVOKABLE void search(const QString &search_term);
	Q_INVOKABLE inline QString filter() const { return m_filterString; }

	Q_INVOKABLE void clearSelectedEntries();
	Q_INVOKABLE bool manageSelectedEntries(uint index, const uint max_selected = 1);
	inline QString selectedEntriesValue(const uint index, const uint field) const
	{
		return data(QAbstractListModel::index(index), Qt::UserRole + field).toString();
	}
	inline uint selectedEntriesCount() const { return m_selectedEntries.count(); }

	bool collectExportData();

	inline QStringList &lastRow() { return m_exercisesData.last(); }
	void appendList(const QStringList &list);
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
	QList<QStringList> m_exercisesData;
	QList<uint> m_muscularFilteredIndices;
	QList<uint> m_searchFilteredIndices;
	QList<uint> m_exportRows;
	QHash<int, QByteArray> m_roleNames;
	QList<uint> m_selectedEntries;
	QString m_filterString, m_searchString;
	uint m_exercisesListCount, m_selectedEntryToReplace;
	int m_currentRow;
	bool m_muscularFilterApplied, m_searchFilterApplied;
	DBModelInterfaceExercisesList *m_dbModelInterface;
	DBExercisesListTable *m_db;

	void readExercisesList();
	void resetSearchModel();

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

