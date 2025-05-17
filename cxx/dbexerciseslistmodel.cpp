#include "dbexerciseslistmodel.h"

#include "tputils.h"
#include "tpglobals.h"
#include "translationclass.h"

#include <QFile>

#include <ranges>
#include <utility>

DBExercisesListModel *DBExercisesListModel::app_exercises_model(nullptr);
constexpr short fieldsNumberInDatabase{EXERCISES_LIST_COL_WEIGHT+1}; //Weight is the last savable field + the Id field

DBExercisesListModel::DBExercisesListModel(QObject *parent, const bool bMainExercisesModel)
	: QAbstractListModel{parent}, m_selectedEntryToReplace{0}, m_exercisesTableLastId{-1}, m_bFilterApplied{false}
{
	if (bMainExercisesModel)
	{
		app_exercises_model = this;

		m_roleNames[exerciseIdRole] = std::move("exerciseId");
		m_roleNames[mainNameRole] = std::move("mainName");
		m_roleNames[subNameRole] = std::move("subName");
		m_roleNames[muscularGroupRole] = std::move("muscularGroup");
		m_roleNames[nSetsRole] = std::move("nSets");
		m_roleNames[nRepsRole] = std::move("nReps");
		m_roleNames[nWeightRole] = std::move("nWeight");
		m_roleNames[uWeightRole] = std::move("uWeight");
		m_roleNames[mediaPathRole] = std::move("mediaPath");
		m_roleNames[fromListRole] = std::move("fromList");
		m_roleNames[actualIndexRole] = std::move("actualIndex");
		m_roleNames[selectedRole] = std::move("selected");

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesListModel::labelsChanged);

		//Value is hardcoded based on the most current exercises list
		m_exercisesData.reserve(305);
		m_indexProxy.reserve(305);
		m_filteredIndices.reserve(305);
	}
}

QString DBExercisesListModel::muscularGroup(const uint index) const
{
	const QStringList &groups{m_exercisesData.at(m_indexProxy.at(index)).at(EXERCISES_LIST_COL_MUSCULARGROUP).split(',')};
	QString translatedGroups;
	for (uint i{0}; i < groups.count(); ++i)
	{
		const QString &group{groups.at(i)};
		if (group == "quadriceps")
			translatedGroups += std::move(tr("Quadriceps")) + ", "_L1;
		else if (group == "hamstrings")
			translatedGroups += std::move(tr("Hamstrings")) + ", "_L1;
		else if (group == "glutes")
			translatedGroups += std::move(tr("Glutes")) + ", "_L1;
		else if (group == "calves")
			translatedGroups += std::move(tr("Calves")) + ", "_L1;
		else if (group == "upper back")
			translatedGroups += std::move(tr("Upper Back")) + ", "_L1;
		else if (group == "middle back")
			translatedGroups += std::move(tr("Middle Back")) + ", "_L1;
		else if (group == "lower back")
			translatedGroups += std::move(tr("Lower Back")) + ", "_L1;
		else if (group == "biceps")
			translatedGroups += std::move(tr("Biceps")) + ", "_L1;
		else if (group == "triceps")
			translatedGroups += std::move(tr("Triceps")) + ", "_L1;
		else if (group == "fore arms")
			translatedGroups += std::move(tr("Forearms")) + ", "_L1;
		else if (group == "upper chest")
			translatedGroups += std::move(tr("Upper Chest")) + ", "_L1;
		else if (group == "middle chest")
			translatedGroups += std::move(tr("Middle Chest")) + ", "_L1;
		else if (group == "lower chest")
			translatedGroups += std::move(tr("Lower Chest")) + ", "_L1;
		else if (group == "front delts")
			translatedGroups += std::move(tr("Front Delts")) + ", "_L1;
		else if (group == "lateral delts")
			translatedGroups += std::move(tr("Lateral Delts")) + ", "_L1;
		else if (group == "rear delts")
			translatedGroups += std::move(tr("Rear Delts")) + ", "_L1;
		else if (group == "traps")
			translatedGroups += std::move(tr("Traps")) + ", "_L1;
		else if (group == "abs")
			translatedGroups += std::move(tr("Abs")) + ", "_L1;
	}
	translatedGroups.chop(2);
	return translatedGroups;
}

void DBExercisesListModel::setCurrentRow(const int row)
{
	if (m_currentRow != row)
	{
		m_currentRow = row;
		emit currentRowChanged();
	}
}

void DBExercisesListModel::newExercise(const QString &name, const QString &subname, const QString &muscular_group)
{
	setLastID(lastID() + 1);
	appendList(std::move(QStringList{} << std::move(QString::number(lastID())) << std::move(name) << std::move(subname) << std::move(muscular_group) <<
		std::move("3"_L1) << std::move("12"_L1) << std::move("20"_L1) << std::move("(kg)"_L1) << std::move("qrc:/images/no_image.jpg"_L1) <<
			STR_ZERO << std::move(QString::number(m_exercisesData.count())) << STR_ZERO));
}

void DBExercisesListModel::removeExercise(const uint index)
{
	beginRemoveRows(QModelIndex{}, index, index);
	m_exercisesData.remove(index);
	m_indexProxy.remove(index);
	if (!m_bFilterApplied)
		m_indexProxy.remove(index);
	else
	{
		const int proxy_index(m_indexProxy.indexOf(index));
		if (proxy_index >= 0)
		{
			m_indexProxy.remove(proxy_index);
			for(uint i(proxy_index); i < m_indexProxy.count(); ++i)
				m_indexProxy[i] = i-1;
		}
	}
	if (m_currentRow >= index)
		setCurrentRow(m_currentRow > 0 ? m_currentRow - 1 : 0);
	emit countChanged();
	endRemoveRows();
}

void DBExercisesListModel::setFilter(const QString &filter)
{
	beginRemoveRows(QModelIndex{}, 0, count()-1);
	m_indexProxy.clear();
	endRemoveRows();

	if (!filter.isEmpty())
	{
		uint idx{0};
		for (const auto &exercise : std::as_const(m_exercisesData))
		{
			const QString &subject{exercise.at(EXERCISES_LIST_COL_MUSCULARGROUP)};
			const QStringList &words_list{filter.split(fancy_record_separator1, Qt::SkipEmptyParts, Qt::CaseInsensitive)};
			for (const auto &word : words_list)
			{
				if (subject.contains(word, Qt::CaseInsensitive))
				{
					beginInsertRows(QModelIndex{}, count(), count());
					m_indexProxy.append(idx);
					endInsertRows();
					m_filteredIndices.append(idx);
					break;
				}
			}
			++idx;
		}
		m_bFilterApplied = m_filteredIndices.count() != m_exercisesData.count();
	}
	else
	{
		if (m_bFilterApplied)
		{
			m_bFilterApplied = false;
			beginInsertRows(QModelIndex{}, 0, m_exercisesData.count());
			for (uint i {0}; i < m_exercisesData.count(); ++i)
				m_indexProxy.append(i);
			endInsertRows();
		}
	}
}

void DBExercisesListModel::search(const QString &search_term)
{
	if (search_term.length() >= 3)
	{
		bool bFound{false};
		const qsizetype modelCount{m_filteredIndices.isEmpty() ? m_exercisesData.count() : m_filteredIndices.count()};

		for (uint i{0}; i < modelCount; ++i)
		{
			const uint idx{m_filteredIndices.isEmpty() ? i : m_filteredIndices.at(i)};
			const QString &subject{m_exercisesData.at(idx).at(EXERCISES_LIST_COL_MAINNAME) +
						' ' + m_exercisesData.at(idx).at(EXERCISES_LIST_COL_SUBNAME)};
			const QStringList &words_list{appUtils()->stripDiacriticsFromString(search_term).split(' ', Qt::SkipEmptyParts, Qt::CaseInsensitive)};

			for (const auto &word : words_list)
			{
				if (subject.contains(word, Qt::CaseInsensitive))
				{
					if (!bFound)
					{
						bFound = true;
						beginRemoveRows(QModelIndex{}, 0, count()-1);
						m_indexProxy.clear();
						endRemoveRows();
						clearSelectedEntries();
						setCurrentRow(-1);
						m_bFilterApplied = true;
					}
					beginInsertRows(QModelIndex{}, count(), count());
					m_indexProxy.append(idx);
					endInsertRows();
				}
			}
		}
		if (!bFound)
		{
			beginRemoveRows(QModelIndex{}, 0, count()-1);
			m_indexProxy.clear();
			endRemoveRows();
			if (m_filteredIndices.isEmpty())
				m_bFilterApplied = false;
		}
	}
	else
	{
		if (m_bFilterApplied)
		{
			bool indexProxyModified{false};
			if (m_filteredIndices.isEmpty())
			{
				m_bFilterApplied = false;
				indexProxyModified = m_indexProxy.count() < m_exercisesData.count();
			}
			else
				indexProxyModified = m_indexProxy.count() != m_filteredIndices.count();

			if (indexProxyModified)
			{
				beginRemoveRows(QModelIndex{}, 0, count()-1);
				m_indexProxy.clear();
				endRemoveRows();
				if (!m_bFilterApplied)
				{
					beginInsertRows(QModelIndex{}, 0, m_exercisesData.count());
					for (uint i {0}; i < m_exercisesData.count(); ++i)
						m_indexProxy.append(i);
				}
				else
				{
					beginInsertRows(QModelIndex{}, 0, m_filteredIndices.count());
					for (uint i {0}; i < m_filteredIndices.count(); ++i)
						m_indexProxy.append(m_filteredIndices.at(i));
				}
				endInsertRows();
			}
		}
	}
}

void DBExercisesListModel::clearSelectedEntries()
{
	for (uint i{0}; i < m_selectedEntries.count(); ++i)
	{
		m_exercisesData[m_selectedEntries.at(i).real_index][EXERCISES_LIST_COL_SELECTED] = STR_ZERO;
		emit dataChanged(index(m_selectedEntries.at(i).view_index, 0),
				index(m_selectedEntries.at(i).view_index, 0), QList<int>() << selectedRole);
	}
	setCurrentRow(-1);
	m_selectedEntries.clear();
	m_selectedEntryToReplace = 0;
}

//Returns true if an item is added to the list of selected entries. False if the item is already in the list(the item then gets removed)
//When an item is added, it becomes selected. When an item is removed, it becomes deselected
bool DBExercisesListModel::manageSelectedEntries(const uint item_pos, const uint max_selected)
{
	selectedEntry entry;
	uint real_item_pos(item_pos);
	if (m_bFilterApplied)
		real_item_pos = m_indexProxy.at(item_pos);
	entry.real_index = real_item_pos;
	entry.view_index = item_pos;

	int idx(-1);
	for (uint i(0); i < m_selectedEntries.count(); ++i)
	{
		if (m_selectedEntries.at(i).real_index == real_item_pos)
		{
			if (max_selected == 1) //Item is double clicked. Do not deselect it
				return false;
			idx = i;
			break;
		}
	}

	if (idx == -1)
	{
		if (m_selectedEntries.count() < max_selected)
			m_selectedEntries.append(entry);
		else if (m_selectedEntries.count() == max_selected)
		{
			if (m_selectedEntryToReplace > max_selected - 1)
				m_selectedEntryToReplace = 0;
			m_exercisesData[m_selectedEntries.at(m_selectedEntryToReplace).real_index][EXERCISES_LIST_COL_SELECTED] = STR_ZERO;
			emit dataChanged(index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0),
					index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0), QList<int>{} << selectedRole);
			m_selectedEntries[m_selectedEntryToReplace].real_index = real_item_pos;
			m_selectedEntries[m_selectedEntryToReplace].view_index = item_pos;
			m_selectedEntryToReplace++;
		}
		else
		{
			for (uint i(0); i <= max_selected; ++i)
			{
				m_exercisesData[m_selectedEntries.at(0).real_index][EXERCISES_LIST_COL_SELECTED] = STR_ZERO;
				emit dataChanged(index(m_selectedEntries.at(0).view_index, 0),
					index(m_selectedEntries.at(0).view_index, 0), QList<int>{} << selectedRole);
				if (m_selectedEntries.count() > 1)
					m_selectedEntries.remove(0, 1);
			}
			m_selectedEntries[0].real_index = real_item_pos;
			m_selectedEntries[0].view_index = item_pos;
		}
	}
	else
	{
		if (m_selectedEntryToReplace == idx)
		{
			++m_selectedEntryToReplace;
			if (m_selectedEntryToReplace > max_selected - 1)
				m_selectedEntryToReplace = 0;
		}
		m_exercisesData[m_selectedEntries.at(idx).real_index][EXERCISES_LIST_COL_SELECTED] = STR_ZERO;
		emit dataChanged(index(m_selectedEntries.at(idx).view_index, 0),
					index(m_selectedEntries.at(idx).view_index, 0), QList<int>() << selectedRole);
		m_selectedEntries.remove(idx, 1);
		return false;
	}
	m_exercisesData[real_item_pos][EXERCISES_LIST_COL_SELECTED] = STR_ONE;
	emit dataChanged(index(item_pos, 0), index(item_pos, 0), QList<int>() << selectedRole);
	return true;
}

bool DBExercisesListModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i(count() - 1); i > 0; --i)
	{
		if (_id(i) >= 1000)
			m_exportRows.append(i);
	}
	return m_exportRows.count() > 0;
}

void DBExercisesListModel::appendList(const QStringList &list)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_indexProxy.append(m_exercisesData.count());
	m_exercisesData.append(list);
	emit countChanged();
	endInsertRows();

}

void DBExercisesListModel::appendList(QStringList &&list)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_indexProxy.append(m_exercisesData.count());
	m_exercisesData.append(std::move(list));
	emit countChanged();
	endInsertRows();
}

void DBExercisesListModel::clear()
{
	m_indexProxy.clear();
	clearSelectedEntries();
	beginResetModel();
	m_exercisesData.clear();
	m_exportRows.clear();
	emit countChanged();
	endResetModel();
}

QString DBExercisesListModel::makeTransactionStatementForDataBase(const uint index) const
{
	QString statement{'(' + id(index)};
	for (uint i{1}; i <= EXERCISES_LIST_COL_MEDIAPATH; ++i)
		statement += ",\'"_L1 + m_exercisesData.at(index).at(i) + '\'';
	statement += ',' + STR_ONE + "),"_L1; //EXERCISES_LIST_COL_FROMAPPLIST
	return statement;
}

int DBExercisesListModel::exportToFile(const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text);
		if (!out_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	const bool ret{appUtils()->writeDataToFile(out_file, appUtils()->exercisesListFileIdentifier, m_exercisesData, m_exportRows)};
	out_file->close();
	return ret ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_EXPORT_FAILED;
}

int DBExercisesListModel::exportToFormattedFile(const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = {appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
		if (!out_file)
			return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;
	}

	QList<std::function<QString(void)>> field_description{QList<std::function<QString(void)>>{} <<
											nullptr <<
											[this] () { return exerciseNameLabel(); } <<
											[this] () { return exerciseSpecificsLabel(); } <<
											[this] () { return muscularGroupsLabel(); } <<
											[this] () { return setsLabel(); } <<
											[this] () { return repsLabel(); } <<
											[this] () { return weightLabel(); } <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr
	};

	int ret{APPWINDOW_MSG_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					appUtils()->exercisesListFileIdentifier,
					m_exercisesData,
					field_description,
					nullptr,
					m_exportRows,
					QString{tr("Exercises List") + "\n\n"_L1})
	)
		ret = APPWINDOW_MSG_EXPORT_OK;
	return ret;
}

int DBExercisesListModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	int ret{appUtils()->readDataFromFile(in_file, m_exercisesData, EXERCISES_TOTAL_COLS, appUtils()->exercisesListFileIdentifier)};
	if (ret != APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE)
		ret = APPWINDOW_MSG_IMPORT_OK;
	else
		ret = APPWINDOW_MSG_IMPORT_FAILED;
	in_file->close();
	return ret;
}

int DBExercisesListModel::importFromFormattedFile(const QString &filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text);
		if (!in_file)
			return APPWINDOW_MSG_OPEN_FAILED;
	}

	const uint first_imported_idx{count()};

	int ret{appUtils()->readDataFromFormattedFile(in_file,
												m_exercisesData,
												fieldsNumberInDatabase,
												appUtils()->exercisesListFileIdentifier,
												nullptr)
	};

	if (ret > 0)
	{
		uint actual_index{first_imported_idx};
		for (auto &&data : m_exercisesData | std::views::drop(first_imported_idx) )
		{
			data[0] = std::move(QString::number(m_exercisesTableLastId++));
			data.append(std::move("(kg)"_L1));
			data.append(STR_ZERO);
			data.append(std::move(QString::number(actual_index++)));
			data.append(STR_ZERO);
		}
		ret = APPWINDOW_MSG_IMPORT_OK;
	}
	else
		ret = APPWINDOW_MSG_IMPORT_FAILED;
	in_file->close();
	return ret;
}

QVariant DBExercisesListModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if(row >= 0  &&row < m_exercisesData.count())
	{
		switch(role) {
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case nSetsRole:
			case nRepsRole:
			case nWeightRole:
			case uWeightRole:
			case mediaPathRole:
			case actualIndexRole:
				if (!m_bFilterApplied)
				{
					//MSG_OUT("NO filter: DBExercisesListModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_exercisesData.at(row).at(role-Qt::UserRole))
					return m_exercisesData.at(row).at(role-Qt::UserRole);
				}
				else
				{
					//MSG_OUT("Filter: DBExercisesListModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_exercisesData.at(m_indexProxy.at(row)).at(role-Qt::UserRole))
					return m_exercisesData.at(m_indexProxy.at(row)).at(role-Qt::UserRole);
				}
			case fromListRole:
			case selectedRole:
				return !m_bFilterApplied ? bool(m_exercisesData.at(row).at(role-Qt::UserRole) == STR_ONE) :
					bool(m_exercisesData.at(m_indexProxy.at(row)).at(role-Qt::UserRole) == STR_ONE);
		}
	}
	return QVariant();
}

bool DBExercisesListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0  &&row < m_exercisesData.count())
	{
		const int field{role-Qt::UserRole};
		switch (role) {
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case nSetsRole:
			case nRepsRole:
			case nWeightRole:
			case uWeightRole:
			case mediaPathRole:
			case actualIndexRole:
				if (!m_bFilterApplied)
					m_exercisesData[row][field] = std::move(value.toString());
				else
					m_exercisesData[m_indexProxy.at(row)][field] = std::move(value.toString());
				emit dataChanged(index, index, QList<int>() << role);
				return true;

			case fromListRole:
			case selectedRole:
				if (!m_bFilterApplied)
					m_exercisesData[row][field] = value.toBool() ? STR_ONE : STR_ZERO;
				else
					m_exercisesData[m_indexProxy.at(row)][field] = value.toBool() ? STR_ONE : STR_ZERO;
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}
