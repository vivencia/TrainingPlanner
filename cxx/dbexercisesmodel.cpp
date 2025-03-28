#include "dbexercisesmodel.h"
#include "tputils.h"
#include "tpglobals.h"

#include <QFile>

#include <utility>

DBExercisesModel* DBExercisesModel::app_exercises_model(nullptr);

DBExercisesModel::DBExercisesModel(QObject* parent, const bool bMainExercisesModel)
	: TPListModel{parent},
		m_selectedEntryToReplace(0), m_exercisesTableLastId(-1), m_bFilterApplied(false)
{
	setObjectName(DBExercisesObjectName);
	m_tableId = EXERCISES_TABLE_ID;
	m_fieldCount = EXERCISES_TOTAL_COLS;

	if (bMainExercisesModel)
	{
		app_exercises_model = this;
		m_exportName = std::move(tr("Exercises List"));

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

		mColumnNames.reserve(EXERCISES_TOTAL_COLS);
		for(uint i(EXERCISES_COL_ID); i < EXERCISES_TOTAL_COLS; ++i)
			mColumnNames.append(QString());
		fillColumnNames();

		//Value is hardcoded based on the most current exercises list
		m_modeldata.reserve(305);
		m_indexProxy.reserve(305);
		m_filteredIndices.reserve(305);
	}
}

void DBExercisesModel::fillColumnNames()
{
	mColumnNames[EXERCISES_COL_MAINNAME] = std::move(tr("Exercise: "));
	mColumnNames[EXERCISES_COL_SUBNAME] = std::move(tr("Specifics: "));
	mColumnNames[EXERCISES_COL_MUSCULARGROUP] = std::move(tr("Muscular Group: "));
	mColumnNames[EXERCISES_COL_SETSNUMBER] = std::move(tr("Sets: "));
	mColumnNames[EXERCISES_COL_REPSNUMBER] = std::move(tr("Reps: "));
	mColumnNames[EXERCISES_COL_WEIGHT] = std::move(tr("Weight: "));
	mColumnNames[EXERCISES_COL_MEDIAPATH] = std::move(tr("Descriptive media: "));
}

QString DBExercisesModel::muscularGroup(const uint index) const
{
	const QStringList& groups{m_modeldata.at(m_indexProxy.at(index)).at(EXERCISES_COL_MUSCULARGROUP).split(',')};
	QString translatedGroups;
	for (uint i{0}; i < groups.count(); ++i)
	{
		const QString& group{groups.at(i)};
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

void DBExercisesModel::newExercise(const QString& name, const QString& subname, const QString& muscular_group)
{
	setLastID(lastID() + 1);
	appendList(std::move(QStringList() << std::move(QString::number(lastID())) << std::move(name) << std::move(subname) << std::move(muscular_group) <<
		std::move("3"_L1) << std::move("12"_L1) << std::move("20"_L1) << std::move("(kg)"_L1) << std::move("qrc:/images/no_image.jpg"_L1) <<
			STR_ZERO << std::move(QString::number(m_modeldata.count())) << STR_ZERO));
}

void DBExercisesModel::removeExercise(const uint index)
{
	beginRemoveRows(QModelIndex{}, index, index);
	m_modeldata.remove(index);
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

void DBExercisesModel::setFilter(const QString& filter)
{
	beginRemoveRows(QModelIndex{}, 0, count()-1);
	m_indexProxy.clear();
	endRemoveRows();

	if (!filter.isEmpty())
	{
		uint idx(0);
		QList<QStringList>::const_iterator lst_itr(m_modeldata.constBegin());
		const QList<QStringList>::const_iterator& lst_itrend(m_modeldata.constEnd());
		for (; lst_itr != lst_itrend; ++lst_itr, ++idx)
		{
			const QString& subject{(*lst_itr).at(EXERCISES_COL_MUSCULARGROUP)};
			const QStringList& words_list{filter.split(fancy_record_separator1, Qt::SkipEmptyParts, Qt::CaseInsensitive)};
			for (uint i{0}; i < words_list.count(); ++i)
			{
				if (subject.contains(words_list.at(i), Qt::CaseInsensitive))
				{
					beginInsertRows(QModelIndex{}, count(), count());
					m_indexProxy.append(idx);
					endInsertRows();
					m_filteredIndices.append(idx);
					break;
				}
			}
		}
		m_bFilterApplied = m_filteredIndices.count() != m_modeldata.count();
	}
	else
	{
		if (m_bFilterApplied)
		{
			m_bFilterApplied = false;
			beginInsertRows(QModelIndex{}, 0, m_modeldata.count());
			for (uint i (0); i < m_modeldata.count(); ++i)
				m_indexProxy.append(i);
			endInsertRows();
		}
	}
}

void DBExercisesModel::search(const QString& search_term)
{
	if (search_term.length() >= 3)
	{
		bool bFound{false};
		const qsizetype modelCount{m_filteredIndices.isEmpty() ? m_modeldata.count() : m_filteredIndices.count()};

		for (uint i{0}; i < modelCount; ++i)
		{
			const uint idx{m_filteredIndices.isEmpty() ? i : m_filteredIndices.at(i)};
			const QString& subject{m_modeldata.at(idx).at(EXERCISES_COL_MAINNAME) +
						' ' + m_modeldata.at(idx).at(EXERCISES_COL_SUBNAME)};
			const QStringList& words_list{appUtils()->stripDiacriticsFromString(search_term).split(' ', Qt::SkipEmptyParts, Qt::CaseInsensitive)};

			for (uint x{0}; x < words_list.count(); ++x)
			{
				if (subject.contains(words_list.at(x), Qt::CaseInsensitive))
				{
					if (!bFound)
					{
						bFound = true;
						beginRemoveRows(QModelIndex{}, 0, count()-1);
						m_indexProxy.clear();
						endRemoveRows();
						resetPrivateData();
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
			bool indexProxyModified(false);
			if (m_filteredIndices.isEmpty())
			{
				m_bFilterApplied = false;
				indexProxyModified = m_indexProxy.count() < m_modeldata.count();
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
					beginInsertRows(QModelIndex{}, 0, m_modeldata.count());
					for (uint i {0}; i < m_modeldata.count(); ++i)
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

void DBExercisesModel::clearSelectedEntries()
{
	for (uint i(0); i < m_selectedEntries.count(); ++i)
	{
		m_modeldata[m_selectedEntries.at(i).real_index][EXERCISES_COL_SELECTED] = STR_ZERO;
		emit dataChanged(index(m_selectedEntries.at(i).view_index, 0),
				index(m_selectedEntries.at(i).view_index, 0), QList<int>() << selectedRole);
	}
	setCurrentRow(-1);
	m_selectedEntries.clear();
	m_selectedEntryToReplace = 0;
}

//Returns true if an item is added to the list of selected entries. False if the item is already in the list(the item then gets removed)
//When an item is added, it becomes selected. When an item is removed, it becomes deselected
bool DBExercisesModel::manageSelectedEntries(const uint item_pos, const uint max_selected)
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
			m_modeldata[m_selectedEntries.at(m_selectedEntryToReplace).real_index][EXERCISES_COL_SELECTED] = STR_ZERO;
			emit dataChanged(index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0),
					index(m_selectedEntries.at(m_selectedEntryToReplace).view_index, 0), QList<int>() << selectedRole);
			m_selectedEntries[m_selectedEntryToReplace].real_index = real_item_pos;
			m_selectedEntries[m_selectedEntryToReplace].view_index = item_pos;
			m_selectedEntryToReplace++;
		}
		else
		{
			for (uint i(0); i <= max_selected; ++i)
			{
				m_modeldata[m_selectedEntries.at(0).real_index][EXERCISES_COL_SELECTED] = STR_ZERO;
				emit dataChanged(index(m_selectedEntries.at(0).view_index, 0),
					index(m_selectedEntries.at(0).view_index, 0), QList<int>() << selectedRole);
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
		m_modeldata[m_selectedEntries.at(idx).real_index][EXERCISES_COL_SELECTED] = STR_ZERO;
		emit dataChanged(index(m_selectedEntries.at(idx).view_index, 0),
					index(m_selectedEntries.at(idx).view_index, 0), QList<int>() << selectedRole);
		m_selectedEntries.remove(idx, 1);
		return false;
	}
	m_modeldata[real_item_pos][EXERCISES_COL_SELECTED] = STR_ONE;
	emit dataChanged(index(item_pos, 0), index(item_pos, 0), QList<int>() << selectedRole);
	return true;
}

bool DBExercisesModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i(count() - 1); i > 0; --i)
	{
		if (_id(i) >= 1000)
			m_exportRows.append(i);
	}
	return m_exportRows.count() > 0;
}

void DBExercisesModel::appendList(const QStringList& list)
{
	TPListModel::appendList(list);
	m_indexProxy.append(m_modeldata.count() - 1);
}

void DBExercisesModel::appendList(QStringList&& list)
{
	TPListModel::appendList(list);
	m_indexProxy.append(m_modeldata.count() - 1);
}

void DBExercisesModel::clear()
{
	m_indexProxy.clear();
	clearSelectedEntries();
	TPListModel::clear();
}

QString DBExercisesModel::makeTransactionStatementForDataBase(const uint index) const
{
	QString statement{'(' + id(index)};
	for (uint i(1); i <= EXERCISES_COL_MEDIAPATH; ++i)
		statement += ",\'"_L1 + m_modeldata.at(index).at(i) + '\'';
	statement += ',' + STR_ONE + "),"_L1; //EXERCISES_COL_FROMAPPLIST
	return statement;
}

int DBExercisesModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile{filename}};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList modeldata{EXERCISES_TOTAL_COLS};
	uint col{EXERCISES_COL_MAINNAME};
	QString value;
	uint n_items{0};
	const qsizetype databaseLastIndex{appExercisesModel()->m_modeldata.count()};
	const QString tableIdStr{"0x000"_L1 + QString::number(EXERCISES_TABLE_ID)};
	bool bFoundModelInfo(false);

	char buf[256];
	qint64 lineLength{0};
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (!bFoundModelInfo)
					bFoundModelInfo = strstr(buf, tableIdStr.toLatin1().constData()) != NULL;
				else
				{
					if (col <= EXERCISES_COL_MEDIAPATH)
					{
						if (col != EXERCISES_COL_WEIGHTUNIT)
						{
							value = buf;
							modeldata[col] = std::move(value.remove(0, value.indexOf(':') + 2).simplified());
						}
						++col;
					}
					else
					{
						++n_items;
						modeldata[EXERCISES_COL_ID] = std::move(QString::number(m_exercisesTableLastId + n_items));
						modeldata[EXERCISES_COL_WEIGHTUNIT] = std::move("(kg)"_L1);
						modeldata[EXERCISES_COL_FROMAPPLIST] = STR_ZERO;
						modeldata[EXERCISES_COL_ACTUALINDEX] = std::move(QString::number(databaseLastIndex + n_items));
						modeldata[EXERCISES_COL_SELECTED] = STR_ZERO;
						m_modeldata.append(modeldata);
						col = 0;
					}
				}
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	return m_modeldata.count() > 1 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBExercisesModel::updateFromModel(TPListModel* model)
{
	QList<QStringList>::iterator lst_itr{model->m_modeldata.begin()};
	const QList<QStringList>::const_iterator& lst_itrend{model->m_modeldata.constEnd()};
	qsizetype lastIndex{m_modeldata.count()};
	do {
		appendList(std::move((*lst_itr)));
		addModifiedIndex(lastIndex++);
	} while (++lst_itr != lst_itrend);
	return true;
}

QVariant DBExercisesModel::data(const QModelIndex& index, int role) const
{
	const int row{index.row()};
	if(row >= 0 && row < m_modeldata.count())
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
					//MSG_OUT("NO filter: DBExercisesModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_modeldata.at(row).at(role-Qt::UserRole))
					return m_modeldata.at(row).at(role-Qt::UserRole);
				}
				else
				{
					//MSG_OUT("Filter: DBExercisesModel::data(" << index.row() << "," << index.column() << ") role: " << role << " = " << m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole))
					return m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole);
				}
			case fromListRole:
			case selectedRole:
				return !m_bFilterApplied ? bool(m_modeldata.at(row).at(role-Qt::UserRole) == STR_ONE) :
					bool(m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole) == STR_ONE);
		}
	}
	return QVariant();
}

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_modeldata.count())
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
					m_modeldata[row][field] = std::move(value.toString());
				else
					m_modeldata[m_indexProxy.at(row)][field] = std::move(value.toString());
				emit dataChanged(index, index, QList<int>() << role);
				return true;

			case fromListRole:
			case selectedRole:
				if (!m_bFilterApplied)
					m_modeldata[row][field] = value.toBool() ? STR_ONE : STR_ZERO;
				else
					m_modeldata[m_indexProxy.at(row)][field] = value.toBool() ? STR_ONE : STR_ZERO;
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}
