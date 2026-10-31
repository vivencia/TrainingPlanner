#pragma once

#include "tplistmodel.h"
#include "dbmodelinterface.h"
#include "qml_singleton.h"

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(DBExercisesListTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercisesList);
QT_FORWARD_DECLARE_CLASS(TPFilePath)

class DBExercisesListModel : public TPListModel
{

Q_OBJECT

Q_PROPERTY(QString exerciseNameLabel READ exerciseNameLabel NOTIFY labelsChanged)
Q_PROPERTY(QString exerciseSpecificsLabel READ exerciseSpecificsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString muscularGroupsLabel READ muscularGroupsLabel NOTIFY labelsChanged)
Q_PROPERTY(QString mediaLabel READ mediaLabel NOTIFY labelsChanged)

public:
	enum ExerciseListFields {
		ID,
		MAINNAME,
		SUBNAME,
		MUSCULARGROUP,
		MEDIAPATH,
		N_FIELDS
	};

	Q_DISABLE_COPY_MOVE(DBExercisesListModel)
	explicit DBExercisesListModel(QObject *parent = nullptr);
	Q_INVOKABLE void initExercisesList();

	inline QString exerciseNameLabel() const { return tr("Exercise: "); }
	inline QString exerciseSpecificsLabel() const { return tr("Specifics: "); }
	inline QString muscularGroupsLabel() const { return tr("Muscular Groups: "); }
	inline QString mediaLabel() const { return tr("Descriptive media: "); }
	std::shared_ptr<TPFilePath> suggestedName() const;

	Q_INVOKABLE inline QString mainName() const
	{
		return exerciseData(MAINNAME, currentRow());
	}
	Q_INVOKABLE void setMainName(const uint index, const QString &new_name);
	Q_INVOKABLE inline QString subName() const
	{
		return exerciseData(SUBNAME, currentRow());
	}
	Q_INVOKABLE void setSubName(const uint index, const QString &new_subname);
	Q_INVOKABLE inline QString muscularGroup() const
	{
		return exerciseData(MUSCULARGROUP, currentRow());
	}
	Q_INVOKABLE void setMuscularGroup(const uint index, const QString &new_group);
	Q_INVOKABLE inline QString mediaPath() const
	{
		return exerciseData(MEDIAPATH, currentRow());
	}
	Q_INVOKABLE void setMediaPath(const uint index, const QString &new_path);

	Q_INVOKABLE void newExercise(const QString &name = QString{}, const QString &subname = QString{},
																	const QString &muscular_group = QString{});

	inline void setMuscularGroupFilter(const QString &filter) { m_mgf = filter; }

	bool collectExportData();

	inline QStringList &lastRow() { return m_exercisesData.last(); }
	void appendList(const QStringList &list);
	void clear();

	int exportToFile(const TPFilePath &filename) const;
	int exportToFormattedFile(const TPFilePath &filename) const;
	int importFromFile(const TPFilePath &filename);
	int importFromFormattedFile(const TPFilePath &filename);
	int newExerciseFromFile(const TPFilePath &filename, const std::optional<bool> &file_formatted = std::nullopt);

	inline const QString &exerciseData(const ExerciseListFields field, int index = -1) const
	{
		if (index == -1) {
			if ((index = currentRow()) < 0)
				return m_strEmpty;
		} else if (index >= m_exercisesData.count()) {
			return m_strEmpty;
		}
		return m_exercisesData.at(index).at(field);
	}

	//--------------- Base Class overrides -------------------
	void setFieldsNames() override final;
	inline const QString &dataValue(const uint real_row, const uint column) const override final
	{
		return exerciseData(static_cast<ExerciseListFields>(column), real_row);
	}
	QVariant data(const uint role, const uint row, const int column = -1) const override final;
	//--------------- Base Class overrides -------------------

signals:
	void labelsChanged();

private:
	QList<QStringList> m_exercisesData;
	QList<uint> m_exportRows;
	DBModelInterfaceExercisesList *m_dbModelInterface{nullptr};
	DBExercisesListTable *m_db{nullptr};
	QString m_strEmpty, m_mgf;

	void addFilters();
	void readExercisesList();

	static DBExercisesListModel *_app_exercises_list;
	friend DBExercisesListModel *appExercisesList();
	friend class DBModelInterfaceExercisesList;
};
DECLARE_QML_NAMED_SINGLETON(DBExercisesListModel, AppExercisesList)

inline DBExercisesListModel *appExercisesList() { return DBExercisesListModel::_app_exercises_list; }

class DBModelInterfaceExercisesList : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceExercisesList() : DBModelInterface{appExercisesList()} {}
	inline const QList<QStringList> &modelData() const { return appExercisesList()->m_exercisesData; }
	inline QList<QStringList> &modelData() { return appExercisesList()->m_exercisesData; }
};

