#include "dbexerciseslistmodel.h"

#include "dbexerciseslisttable.h"
#include "return_codes.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"

#include <QFile>

#include <ranges>
#include <utility>

DBExercisesListModel *DBExercisesListModel::app_exercises_list{nullptr};
constexpr uint8_t fieldsNumberInDatabase{EXERCISES_LIST_FIELD_FROMAPPLIST + 1}; //FromAppList is the last savable field + the Id field

enum RoleNames {
	createRole(exerciseId, EXERCISES_LIST_FIELD_ID)
	createRole(mainName, EXERCISES_LIST_FIELD_MAINNAME)
	createRole(subName, EXERCISES_LIST_FIELD_SUBNAME)
	createRole(muscularGroup, EXERCISES_LIST_FIELD_MUSCULARGROUP)
	createRole(mediaPath, EXERCISES_LIST_FIELD_MEDIAPATH)
	createRole(fromList, EXERCISES_LIST_FIELD_FROMAPPLIST)
	createRole(actualIndex, EXERCISES_LIST_FIELD_ACTUALINDEX)
	createRole(selected, EXERCISES_LIST_FIELD_SELECTED)
};

DBExercisesListModel::DBExercisesListModel(QObject *parent)
	: QAbstractListModel{parent}, m_selectedEntryToReplace{0}, m_muscularFilterApplied{false}, m_searchFilterApplied{false}
{
	app_exercises_list = this;
	roleToString(exerciseId)
	roleToString(mainName)
	roleToString(subName)
	roleToString(muscularGroup)
	roleToString(mediaPath)
	roleToString(fromList)
	roleToString(actualIndex)
	roleToString(selected)

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesListModel::labelsChanged);

	//Value is hardcoded based on the most current exercises list
	m_exercisesData.reserve(306);
	m_searchFilteredIndices.reserve(306);
	m_muscularFilteredIndices.reserve(306);
}

void DBExercisesListModel::initExercisesList()
{
	readExercisesList();
	m_dbModelInterface = new DBModelInterfaceExercisesList;
	m_db = new DBExercisesListTable{m_dbModelInterface};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
}

QString DBExercisesListModel::id(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), exerciseIdRole).toString();
}

void DBExercisesListModel::setId(const uint index, const QString &new_id)
{
	setData(QAbstractListModel::index(index, 0), new_id, exerciseIdRole);
}

QString DBExercisesListModel::mainName(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), mainNameRole).toString();
}

void DBExercisesListModel::setMainName(const uint index, const QString &new_name)
{
	setData(QAbstractListModel::index(index, 0), new_name, mainNameRole);
}

QString DBExercisesListModel::subName(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), subNameRole).toString();
}

void DBExercisesListModel::setSubName(const uint index, const QString &new_subname)
{
	setData(QAbstractListModel::index(index, 0), new_subname, subNameRole);
}

QString DBExercisesListModel::muscularGroup(const int index) const
{
	if (index < 0)
		return QString{};

	return m_exercisesData.at(m_searchFilterApplied ? m_searchFilteredIndices.at(index) :
							(m_muscularFilterApplied ? m_muscularFilteredIndices.at(index) : index)).at(EXERCISES_LIST_FIELD_MUSCULARGROUP);
}

void DBExercisesListModel::setMuscularGroup(const uint index, const QString &new_group)
{
	setData(QAbstractListModel::index(index, 0), new_group, muscularGroupRole);
}

QString DBExercisesListModel::mediaPath(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), mediaPathRole).toString();
}

void DBExercisesListModel::setMediaPath(const uint index, const QString &new_path)
{
	setData(QAbstractListModel::index(index, 0), new_path, mediaPathRole);
}

uint DBExercisesListModel::actualIndex(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), actualIndexRole).toUInt();
}

void DBExercisesListModel::setActualIndex(const uint index, const uint new_index)
{
	setData(QAbstractListModel::index(index, 0), QString::number(new_index), actualIndexRole);
}

bool DBExercisesListModel::isSelected(const uint index) const
{
	return data(QAbstractListModel::index(index, 0), selectedRole).toUInt() == 1;
}

void DBExercisesListModel::setSelected(const uint index, const bool selected)
{
	setData(QAbstractListModel::index(index, 0), selected,  selectedRole);
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
	appendList(std::move(QStringList{} <<std::move("-1"_L1) << name << subname << muscular_group << QString{} << std::move("0"_L1) <<
																	std::move(QString::number(m_exercisesData.count())) << std::move("0"_L1)));
}

void DBExercisesListModel::removeExercise(const uint index)
{
	m_dbModelInterface->setRemovalInfo(actualIndex(index), QList<uint>{1 , EXERCISES_LIST_FIELD_ID});
	appThreadManager()->queueAction(m_db, ThreadManager::DeleteRecords);

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
		if (m_muscularFilterApplied)
		{
			if (m_filterString != filter)
				setFilter(QString{});
			else
				return;
		}

		QStringList words_list{std::move(filter.split(fancy_record_separator1, Qt::SkipEmptyParts, Qt::CaseInsensitive))};
		const auto modelCount{m_searchFilterApplied ? m_searchFilteredIndices.count() : m_exercisesData.count()};
		for (uint i{0}; i < modelCount; ++i)
		{
			const uint index{m_searchFilterApplied ? m_searchFilteredIndices.at(i) : i};
			const QString &subject{m_exercisesData.at(index).at(EXERCISES_LIST_FIELD_MUSCULARGROUP)};
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
					}
					m_muscularFilteredIndices.append(index);
					break;
				}
			}
		}
		if (m_muscularFilterApplied)
		{
			if (m_searchFilterApplied)
			{
				beginResetModel();
				m_searchFilteredIndices.clear();
				m_searchFilteredIndices = m_muscularFilteredIndices;
			}
			endResetModel();
			emit countChanged();
		}
	}
	else
	{
		if (m_muscularFilterApplied)
		{
			m_muscularFilterApplied = false;
			m_muscularFilteredIndices.clear();
			m_filterString.clear();
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
			const QString &subject{m_exercisesData.at(index).at(EXERCISES_LIST_FIELD_MAINNAME) % ' ' %
								m_exercisesData.at(index).at(EXERCISES_LIST_FIELD_SUBNAME)};
			if (appUtils()->containsAllWords(subject, words_list, false))
			{
				found = true;
				if (search_term.length() < m_searchString.length())
				{
					const uint insert_idx{m_exercisesData.at(index).at(EXERCISES_LIST_FIELD_ACTUALINDEX).toUInt()};
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
					m_searchFilteredIndices.append(index);
					endResetModel();
					emit countChanged();
				}
				else
				{
					if (m_searchFilteredIndices.indexOf(m_exercisesData.at(index).at(EXERCISES_LIST_FIELD_ACTUALINDEX).toUInt()) != -1)
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
		if (!found)
			m_searchFilterApplied = false;
	}
	else if (!m_searchFilteredIndices.isEmpty())
		resetSearchModel();
}

void DBExercisesListModel::clearSelectedEntries()
{
	for (const auto idx : std::as_const(m_selectedEntries))
		setData(index(idx), false, selectedRole);
	setCurrentRow(-1);
	m_selectedEntries.clear();
	m_selectedEntryToReplace = 0;
}

//Returns true if an item is added to the list of selected entries. False if the item is already in the list(the item then gets removed)
//When an item is added, it becomes selected. When an item is removed, it becomes deselected
bool DBExercisesListModel::manageSelectedEntries(const uint item_pos, const uint max_selected)
{
	if (m_selectedEntries.contains(item_pos))
		return false;
	if (m_selectedEntries.count() == max_selected)
	{
		setData(index(m_selectedEntries.at(m_selectedEntryToReplace)), false, selectedRole);
		m_selectedEntries[m_selectedEntryToReplace] = item_pos;
		if (++m_selectedEntryToReplace == max_selected)
			m_selectedEntryToReplace = 0;
	}
	else
		m_selectedEntries.append(item_pos);
	setData(index(item_pos), true, selectedRole);
	return true;
}

bool DBExercisesListModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i{m_exercisesListCount}; i < m_exercisesData.count(); ++i)
		m_exportRows.append(i);
	return m_exportRows.count() > 0;
}

void DBExercisesListModel::appendList(const QStringList &list)
{
	beginInsertRows(QModelIndex{}, m_exercisesData.count(), m_exercisesData.count() + list.count() - 1);
	m_dbModelInterface->setModified(m_exercisesData.count(), 0);
	m_exercisesData.append(list);
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
			data[0] = std::move(QString::number(m_exercisesData.last().at(EXERCISES_LIST_FIELD_ID).toUInt() + 1));
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
			emit dataChanged(index, index, QList<int>{role});
			if (m_searchFilterApplied)
				row = m_exercisesData.at(m_searchFilteredIndices.at(row)).at(EXERCISES_LIST_FIELD_ACTUALINDEX).toInt();
			else
			{
				if (m_muscularFilterApplied)
					row = m_exercisesData.at(m_muscularFilteredIndices.at(row)).at(EXERCISES_LIST_FIELD_ACTUALINDEX).toInt();
				else
					row = m_exercisesData.at(row).at(EXERCISES_LIST_FIELD_ACTUALINDEX).toInt();
			}
			m_dbModelInterface->setModified(row, field);
		}
	}
	return data_set;
}

void DBExercisesListModel::readExercisesList()
{
	const QString &list_filename{":/extras/exercises_%1.lst"_L1};
	QFile *file{appUtils()->openFile(list_filename.arg(appSettings()->userLocale()))};
	if (!file)
		file = appUtils()->openFile(list_filename.arg("en_US"_L1));
	if (file)
	{
		beginResetModel();
		QString line{256, QChar{0}}, version{};
		QTextStream stream{file};
		stream.readLineInto(&line);
		while (stream.readLineInto(&line))
		{
			QStringList fields{std::forward<QStringList>(line.split(';'))};
			m_exercisesData.append(std::move(QStringList{} <<
			std::move("-1"_L1) << std::move(fields[0]) << std::move(fields[1]) << std::move(fields[2]) << QString{} << std::move("1"_L1) <<
				std::move(QString::number(m_exercisesData.count())) << std::move("0"_L1)));
		}
		m_exercisesListCount = m_exercisesData.count();
		emit countChanged();
		emit hasExercisesChanged();
		endResetModel();
		file->close();
		delete file;
	}
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
