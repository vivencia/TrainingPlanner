#include "dbexerciseslistmodel.h"

#include "dbexerciseslisttable.h"
#include "return_codes.h"
#include "tputils.h"
#include "translationclass.h"

#include <QFile>

#include <ranges>
#include <utility>

DBExercisesListModel *DBExercisesListModel::app_exercises_list{nullptr};
constexpr uint fieldsNumberInDatabase{EXERCISES_LIST_COL_FROMAPPLIST + 1}; //FromAppList is the last savable field + the Id field

DBExercisesListModel::DBExercisesListModel(QObject *parent)
	: QAbstractListModel{parent}, m_selectedEntryToReplace{0}, m_muscularFilterApplied{false}, m_searchFilterApplied{false}
{
	app_exercises_list = this;

	m_roleNames[exerciseIdRole]		=	std::move("exerciseId");
	m_roleNames[mainNameRole]		=	std::move("mainName");
	m_roleNames[subNameRole]		=	std::move("subName");
	m_roleNames[muscularGroupRole]	=	std::move("muscularGroup");
	m_roleNames[mediaPathRole]		=	std::move("mediaPath");
	m_roleNames[fromListRole]		=	std::move("fromList");
	m_roleNames[actualIndexRole]	=	std::move("actualIndex");
	m_roleNames[selectedRole]		=	std::move("selected");

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesListModel::labelsChanged);

	//Value is hardcoded based on the most current exercises list
	m_exercisesData.reserve(304);
	m_searchFilteredIndices.reserve(304);
	m_muscularFilteredIndices.reserve(304);

	m_dbModelInterface = new DBModelInterfaceExercisesList;
	m_db = new DBExercisesListTable{m_dbModelInterface, getExercisesListVersion()};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
}

QString DBExercisesListModel::muscularGroup(const int index) const
{
	if (index < 0)
		return QString{};

	const QStringList &groups{m_exercisesData.at(m_searchFilterApplied ? m_searchFilteredIndices.at(index) :
											(m_muscularFilterApplied ? m_muscularFilteredIndices.at(index) : index)).
																		at(EXERCISES_LIST_COL_MUSCULARGROUP).split(',')};
	QString translatedGroups;
	for (const auto &group : groups)
	{
		if (group == "quadriceps")
			translatedGroups += std::move(tr("Quadriceps")) % fancy_record_separator1;
		else if (group == "hamstrings")
			translatedGroups += std::move(tr("Hamstrings")) % fancy_record_separator1;
		else if (group == "glutes")
			translatedGroups += std::move(tr("Glutes")) % fancy_record_separator1;
		else if (group == "calves")
			translatedGroups += std::move(tr("Calves")) % fancy_record_separator1;
		else if (group == "upper back")
			translatedGroups += std::move(tr("Upper Back")) % fancy_record_separator1;
		else if (group == "middle back")
			translatedGroups += std::move(tr("Middle Back")) % fancy_record_separator1;
		else if (group == "lower back")
			translatedGroups += std::move(tr("Lower Back")) % fancy_record_separator1;
		else if (group == "biceps")
			translatedGroups += std::move(tr("Biceps")) % fancy_record_separator1;
		else if (group == "triceps")
			translatedGroups += std::move(tr("Triceps")) % fancy_record_separator1;
		else if (group == "fore arms")
			translatedGroups += std::move(tr("Forearms")) % fancy_record_separator1;
		else if (group == "upper chest")
			translatedGroups += std::move(tr("Upper Chest")) % fancy_record_separator1;
		else if (group == "middle chest")
			translatedGroups += std::move(tr("Middle Chest")) % fancy_record_separator1;
		else if (group == "lower chest")
			translatedGroups += std::move(tr("Lower Chest")) % fancy_record_separator1;
		else if (group == "front delts")
			translatedGroups += std::move(tr("Front Delts")) % fancy_record_separator1;
		else if (group == "lateral delts")
			translatedGroups += std::move(tr("Lateral Delts")) % fancy_record_separator1;
		else if (group == "rear delts")
			translatedGroups += std::move(tr("Rear Delts")) % fancy_record_separator1;
		else if (group == "traps")
			translatedGroups += std::move(tr("Traps")) % fancy_record_separator1;
		else if (group == "abs")
			translatedGroups += std::move(tr("Abs")) % fancy_record_separator1;
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

void DBExercisesListModel::newExerciseFromList(QString &&name, QString &&subname, QString &&muscular_group)
{
	appendList(std::move(QStringList{} <<
				std::move(QString::number(m_exercisesData.last().at(EXERCISES_LIST_COL_ID).toUInt() + 1)) <<
				std::move(name) << std::move(subname) << std::move(muscular_group) << QString{} << std::move("1"_L1) <<
				std::move(QString::number(m_exercisesData.count())) << std::move("0"_L1)));
}

void DBExercisesListModel::newExercise(const QString &name, const QString &subname, const QString &muscular_group)
{
	uint last_id{m_exercisesData.last().at(EXERCISES_LIST_COL_ID).toUInt()};
	if (last_id < 1000)
		last_id = 1000;
	appendList(std::move(QStringList{} << std::move(QString::number(last_id + 1)) << std::move(name) <<
				std::move(subname) << std::move(muscular_group) << QString{} << std::move("0"_L1) <<
				std::move(QString::number(m_exercisesData.count())) << std::move("0"_L1)));
}

void DBExercisesListModel::removeExercise(const uint index)
{
	m_dbModelInterface->setRemovalInfo(actualIndex(index), QList<uint>{1 , EXERCISES_LIST_COL_ID});
	appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);

	beginRemoveRows(QModelIndex{}, index, index);
	const uint actual_index{actualIndex(index)};
	m_exercisesData.remove(actual_index);
	if (m_searchFilterApplied)
		m_searchFilteredIndices.remove(index);
	else
		m_muscularFilteredIndices.remove(index);
	if (m_currentRow >= index)
		setCurrentRow(m_currentRow > 0 ? m_currentRow - 1 : 0);
	emit countChanged();
	endRemoveRows();
}

//Do not use the helper functions in .h file to get values. Look directly into the data set
void DBExercisesListModel::setFilter(const QString &filter)
{
	if (!filter.isEmpty())
	{									
		QStringList words_list{std::move(filter.split(fancy_record_separator1, Qt::SkipEmptyParts, Qt::CaseInsensitive))};
		const qsizetype modelCount{m_searchFilterApplied ? m_searchFilteredIndices.count() : m_exercisesData.count()};
		for (uint i{0}; i < modelCount; ++i)
		{
			const uint index{m_searchFilterApplied ? m_searchFilteredIndices.at(i) :
										(m_muscularFilterApplied ? m_muscularFilteredIndices.at(i) : i)};
			const QString &subject{m_exercisesData.at(index).at(EXERCISES_LIST_COL_MUSCULARGROUP)};
			for (const auto &word : std::as_const(words_list))
			{
				if (subject.contains(word, Qt::CaseInsensitive))
				{
					if (!m_muscularFilterApplied)
					{
						beginResetModel();
						clearSelectedEntries();
						m_filterString = filter;
						m_muscularFilterApplied = true;
						emit countChanged();
						endResetModel();
					}
					m_muscularFilteredIndices.append(m_exercisesData.at(index).at(EXERCISES_LIST_COL_ACTUALINDEX).toUInt());
					if (!m_searchFilterApplied)
					{
						beginInsertRows(QModelIndex{}, m_muscularFilteredIndices.count() - 1, m_muscularFilteredIndices.count() - 1);
						emit countChanged();
						endInsertRows();
					}
				}
			}
		}
		if (m_muscularFilterApplied && m_searchFilterApplied)
		{
			beginResetModel();
			clearSelectedEntries();
			m_searchFilteredIndices.clear();
			m_searchFilteredIndices = m_muscularFilteredIndices;
			emit countChanged();
			endResetModel();
		}
	}
	else
	{
		if (m_muscularFilterApplied)
		{
			m_muscularFilterApplied = false;
			m_muscularFilteredIndices.clear();
			if (!m_searchFilterApplied)
			{
				beginResetModel();
				endResetModel();
				emit countChanged();
			}
			else
			{
				const QString search_term{m_searchString};
				resetSearchModel();
				search(search_term);
			}
		}
	}
}

//Do not use the helper functions in .h file to get values. Look directly into the data set
void DBExercisesListModel::search(const QString &search_term)
{
	bool found{false};
	if (search_term.length() >= 3)
	{
		qsizetype modelCount{count()};
		const QStringList &words_list{appUtils()->stripDiacriticsFromString(search_term).split(' ', Qt::SkipEmptyParts)};
		const bool look_in_searched_indices{(search_term.length() >= m_searchString.length()) && m_searchFilterApplied};

		for (uint i{0}; i < static_cast<uint>(modelCount); ++i)
		{
			const uint index{look_in_searched_indices ? m_searchFilteredIndices.at(i) :
										(m_muscularFilterApplied ? m_muscularFilteredIndices.at(i) : i)};
			const QString &subject{m_exercisesData.at(index).at(EXERCISES_LIST_COL_MAINNAME) + ' ' +
								m_exercisesData.at(index).at(EXERCISES_LIST_COL_SUBNAME)};
			if (containsAllWords(subject, words_list))
			{
				found = true;
				if (search_term.length() < m_searchString.length())
				{
					const uint insert_idx{m_exercisesData.at(index).at(EXERCISES_LIST_COL_ACTUALINDEX).toUInt()};
					if (m_searchFilteredIndices.indexOf(insert_idx) == -1)
					{
						uint i{0};
						for (; i < count(); ++i)
						{
							if (actualIndex(i) > insert_idx)
								break;
						}
						beginInsertRows(QModelIndex{}, i, i);
						m_searchFilteredIndices.insert(i, insert_idx);
						endInsertRows();
					}
					else
						break;
				}
				m_searchString = search_term;
				if (!look_in_searched_indices)
				{
					beginResetModel();
					m_searchFilterApplied = true;
					emit countChanged();
					endResetModel();
					beginInsertRows(QModelIndex{}, m_searchFilteredIndices.count(), m_searchFilteredIndices.count());
					m_searchFilteredIndices.append(m_exercisesData.at(index).at(EXERCISES_LIST_COL_ACTUALINDEX).toUInt());
					endInsertRows();
					emit countChanged();
				}
				else
				{
					if (m_searchFilteredIndices.indexOf(m_exercisesData.at(index).at(EXERCISES_LIST_COL_ACTUALINDEX).toUInt()) != -1)
						continue;
				}
			}
			else
			{
				if (look_in_searched_indices)
				{
					beginRemoveRows(QModelIndex{}, i, i);
					m_searchFilteredIndices.remove(i);
					--modelCount;
					--i;
					endRemoveRows();
					emit countChanged();
				}
			}
		}
	}
	if (!found)
	{
		if (m_searchFilterApplied)
		{
			resetSearchModel();
			if (m_muscularFilterApplied)
			{
				m_muscularFilteredIndices.clear();
				setFilter(m_filterString);
			}
		}
	}
}

void DBExercisesListModel::clearSelectedEntries()
{
	for (uint i{0}; i < m_selectedEntries.count(); ++i)
	{
		m_exercisesData[m_selectedEntries.at(i).real_index][EXERCISES_LIST_COL_SELECTED] = '0';
		emit dataChanged(index(m_selectedEntries.at(i).view_index, 0),
				index(m_selectedEntries.at(i).view_index, 0), QList<int>{} << selectedRole);
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
	const uint real_item_pos{actualIndex(item_pos)};
	entry.real_index = real_item_pos;
	entry.view_index = item_pos;

	int idx{-1};
	for (uint i{0}; i < m_selectedEntries.count(); ++i)
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
			m_exercisesData[m_selectedEntries.at(m_selectedEntryToReplace).real_index][EXERCISES_LIST_COL_SELECTED] = '0';
			emit dataChanged(index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0),
					index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0), QList<int>{} << selectedRole);
			m_selectedEntries[m_selectedEntryToReplace].real_index = real_item_pos;
			m_selectedEntries[m_selectedEntryToReplace].view_index = item_pos;
			m_selectedEntryToReplace++;
		}
		else
		{
			for (uint i{0}; i <= max_selected; ++i)
			{
				m_exercisesData[m_selectedEntries.at(0).real_index][EXERCISES_LIST_COL_SELECTED] = '0';
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
		m_exercisesData[m_selectedEntries.at(idx).real_index][EXERCISES_LIST_COL_SELECTED] = '0';
		emit dataChanged(index(m_selectedEntries.at(idx).view_index, 0),
					index(m_selectedEntries.at(idx).view_index, 0), QList<int>{1, selectedRole});
		m_selectedEntries.remove(idx, 1);
		return false;
	}
	m_exercisesData[real_item_pos][EXERCISES_LIST_COL_SELECTED] = '1';
	emit dataChanged(index(item_pos, 0), index(item_pos, 0), QList<int>{1, selectedRole});
	return true;
}

bool DBExercisesListModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i{count() - 1}; i > 0; --i)
	{
		if (_id(i) >= 1000)
			m_exportRows.append(i);
	}
	return m_exportRows.count() > 0;
}

void DBExercisesListModel::appendList(const QStringList &list, const bool save_to_database)
{
	if (save_to_database)
		//m_dbModelInterface->setAllFieldsModified(m_exercisesData.count(), list.count());
		m_dbModelInterface->setModified(m_exercisesData.count(), 0);
	beginInsertRows(QModelIndex{}, m_exercisesData.count(), m_exercisesData.count() + list.count() - 1);
	m_exercisesData.append(list);
	emit countChanged();
	emit hasExercisesChanged();
	endInsertRows();
}

void DBExercisesListModel::appendList(QStringList &&list, const bool save_to_database)
{
	if (save_to_database)
		//m_dbModelInterface->setAllFieldsModified(m_exercisesData.count(), list.count());
		m_dbModelInterface->setModified(m_exercisesData.count(), 0);
	beginInsertRows(QModelIndex{}, m_exercisesData.count(), m_exercisesData.count() + list.count() - 1);
	m_exercisesData.append(std::move(list));
	emit countChanged();
	emit hasExercisesChanged();
	endInsertRows();
}

void DBExercisesListModel::clear()
{
	beginResetModel();
	m_muscularFilteredIndices.clear();
	m_searchFilteredIndices.clear();
	clearSelectedEntries();
	m_exercisesData.clear();
	m_exportRows.clear();
	emit countChanged();
	emit hasExercisesChanged();
	endResetModel();
}

int DBExercisesListModel::exportToFile(const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, false, true, false, true);
		if (!out_file)
			return TP_RET_CODE_OPEN_WRITE_FAILED;
	}

	const bool ret{appUtils()->writeDataToFile(out_file, appUtils()->exercisesListFileIdentifier, m_exercisesData, m_exportRows)};
	out_file->close();
	return ret ? TP_RET_CODE_EXPORT_OK : TP_RET_CODE_EXPORT_FAILED;
}

int DBExercisesListModel::exportToFormattedFile(const QString &filename, QFile *out_file) const
{
	if (!out_file)
	{
		out_file = appUtils()->openFile(filename, false, true, false, true);
		if (!out_file)
			return TP_RET_CODE_OPEN_CREATE_FAILED;
	}

	QList<std::function<QString(void)>> field_description{QList<std::function<QString(void)>>{} <<
											nullptr <<
											[this] () { return exerciseNameLabel(); } <<
											[this] () { return exerciseSpecificsLabel(); } <<
											[this] () { return muscularGroupsLabel(); } <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr
	};

	int ret{TP_RET_CODE_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					appUtils()->exercisesListFileIdentifier,
					m_exercisesData,
					field_description,
					nullptr,
					m_exportRows,
					QString{tr("Exercises List") + "\n\n"_L1})
	)
		ret = TP_RET_CODE_EXPORT_OK;
	return ret;
}

int DBExercisesListModel::importFromFile(const QString& filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	beginInsertRows(QModelIndex{}, count(), count());
	int ret{appUtils()->readDataFromFile(in_file, m_exercisesData, EXERCISES_TOTAL_COLS, appUtils()->exercisesListFileIdentifier)};
	if (ret != TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
	{
		emit countChanged();
		ret = TP_RET_CODE_IMPORT_OK;
	}
	else
		ret = TP_RET_CODE_IMPORT_FAILED;
	endInsertRows();
	in_file->close();
	return ret;
}

int DBExercisesListModel::importFromFormattedFile(const QString &filename, QFile *in_file)
{
	if (!in_file)
	{
		in_file = appUtils()->openFile(filename);
		if (!in_file)
			return TP_RET_CODE_OPEN_READ_FAILED;
	}

	const uint first_imported_idx{count()};

	beginInsertRows(QModelIndex{}, count(), count());
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
			data[0] = std::move(QString::number(m_exercisesData.last().at(EXERCISES_LIST_COL_ID).toUInt() + 1));
			data.append(std::move("(kg)"_L1));
			data.append(std::move("0"_L1));
			data.append(std::move(QString::number(actual_index++)));
			data.append(std::move("0"_L1));
		}
		ret = TP_RET_CODE_IMPORT_OK;
		emit countChanged();
	}
	else
		ret = TP_RET_CODE_IMPORT_FAILED;
	endInsertRows();
	in_file->close();
	return ret;
}

int DBExercisesListModel::newExerciseFromFile(const QString &filename, const std::optional<bool> &file_formatted)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value())
	{
		if (file_formatted.value())
			import_result = importFromFormattedFile(filename);
		else
			import_result = importFromFile(filename);
	}
	else
	{
		import_result = importFromFile(filename);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(filename);
	}
	return import_result;
}

QVariant DBExercisesListModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_exercisesData.count())
	{
		switch (role) {
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case mediaPathRole:
			case actualIndexRole:
				if (m_searchFilterApplied)
					return m_exercisesData.at(m_searchFilteredIndices.at(row)).at(role-Qt::UserRole);
				else
				{
					if (m_muscularFilterApplied)
						return m_exercisesData.at(m_muscularFilteredIndices.at(row)).at(role-Qt::UserRole);
					else
						return m_exercisesData.at(row).at(role-Qt::UserRole);
				}
			break;
			case fromListRole:
			case selectedRole:
				if (m_searchFilterApplied)
					return m_exercisesData.at(m_searchFilteredIndices.at(row)).at(role-Qt::UserRole) == '1';
				else
				{
					if (m_muscularFilterApplied)
						return m_exercisesData.at(m_muscularFilteredIndices.at(row)).at(role-Qt::UserRole) == '1';
					else
						return m_exercisesData.at(row).at(role-Qt::UserRole) == '1';
				}
			break;
		}
	}
	return QVariant{};
}

bool DBExercisesListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	bool data_set{false};
	int row{index.row()};
	if (row >= 0)
	{
		const int field{role - Qt::UserRole};
		switch (role)
		{
			case exerciseIdRole:
			case mainNameRole:
			case subNameRole:
			case muscularGroupRole:
			case mediaPathRole:
			case actualIndexRole:
				if (m_searchFilterApplied)
					m_exercisesData[m_searchFilteredIndices.at(row)][field] = std::move(value.toString());
				else
				{
					if (m_muscularFilterApplied)
						m_exercisesData[m_muscularFilteredIndices.at(row)][field] = std::move(value.toString());
					else
						m_exercisesData[row][field] = std::move(value.toString());
				}
				data_set = true;
			break;
			case fromListRole:
			case selectedRole:
				if (m_searchFilterApplied)
					m_exercisesData[m_searchFilteredIndices.at(row)][field] = value.toBool() ? '1' : '0';
				else
				{
					if (m_muscularFilterApplied)
						m_exercisesData[m_muscularFilteredIndices.at(row)][field] = value.toBool() ? '1' : '0';
					else
						m_exercisesData[row][field] = value.toBool() ? '1' : '0';
				}
				data_set = true;
			break;
		}
		if (data_set)
		{
			emit dataChanged(index, index, QList<int>{} << role);
			if (m_searchFilterApplied)
				row = m_exercisesData.at(m_searchFilteredIndices.at(row)).at(EXERCISES_LIST_COL_ACTUALINDEX).toInt();
			else
			{
				if (m_muscularFilterApplied)
					row = m_exercisesData.at(m_muscularFilteredIndices.at(row)).at(EXERCISES_LIST_COL_ACTUALINDEX).toInt();
				else
					row = m_exercisesData.at(m_searchFilteredIndices.at(row)).at(EXERCISES_LIST_COL_ACTUALINDEX).toInt();
					row = m_exercisesData.at(row).at(EXERCISES_LIST_COL_ACTUALINDEX).toInt();
			}
			m_dbModelInterface->setModified(row, field);
		}
	}
	return data_set;
}

QString DBExercisesListModel::untranslatedMuscularGroup(const QString &translated_group) const
{
	if (translated_group == tr("Quadriceps"))
		return "quadriceps"_L1;
	if (translated_group == tr("Hamstrings"))
		return "hamstrings"_L1;
	if (translated_group == tr("Glutes"))
		return "glutes"_L1;
	if (translated_group == tr("Calves"))
		return "calves"_L1;
	if (translated_group == tr("Upper Back"))
		return "upper back"_L1;
	if (translated_group == tr("Middle Back"))
		return "middle back"_L1;
	if (translated_group == tr("Lower Back"))
		return "lower back"_L1;
	if (translated_group == tr("Biceps"))
		return "biceps"_L1;
	if (translated_group == tr("Triceps"))
		return "triceps"_L1;
	if (translated_group == tr("Forearms"))
		return "fore arms"_L1;
	if (translated_group == tr("Upper Chest"))
		return "upper chest"_L1;
	if (translated_group == tr("Middle Chest"))
		return "middle chest"_L1;
	if (translated_group == tr("Lower Chest"))
		return "lower chest"_L1;
	if (translated_group == tr("Front Delts"))
		return "front delts"_L1;
	if (translated_group == tr("Lateral Delts"))
		return "lateral delts"_L1;
	if (translated_group == tr("Rear Delts"))
		return "rear delts"_L1;
	if (translated_group == tr("Traps"))
		return "traps"_L1;
	if (translated_group == tr("Abs"))
		return "abs"_L1;
	return QString {};
}

void DBExercisesListModel::resetSearchModel()
{
	beginResetModel();
	clearSelectedEntries();
	m_searchFilterApplied = false;
	m_searchFilteredIndices.clear();
	m_searchString.clear();
	endResetModel();
}

QString DBExercisesListModel::getExercisesListVersion() const
{
	QString version{std::move("0"_L1)};
	QFile exercisesListFile{":/extras/exerciseslist.lst"_L1};
	if (exercisesListFile.open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		char buf[20]{0};
		qint64 lineLength;
		lineLength = exercisesListFile.readLine(buf, sizeof(buf));
		if (lineLength > 0)
		{
			version = buf;
			if (version.startsWith("#Vers"_L1))
				version = std::move(version.split(';').at(1).trimmed());
		}
		exercisesListFile.close();
	}
	return version;
}
