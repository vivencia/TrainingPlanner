#include "dbexercisesmodel.h"
#include "tpglobals.h"

#include <QFile>

#include <utility>

DBExercisesModel* DBExercisesModel::app_exercises_model(nullptr);

DBExercisesModel::DBExercisesModel(QObject* parent)
	: TPListModel{parent},
		m_selectedEntryToReplace(0), m_exercisesTableLastId(-1), m_bFilterApplied(false)
{
	if (!app_exercises_model)
		app_exercises_model = this;

	setObjectName(DBExercisesObjectName);
	m_tableId = EXERCISES_TABLE_ID;
	m_fieldCount = EXERCISES_TOTAL_COLS;
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
	for(uint i(0); i < EXERCISES_TOTAL_COLS; ++i)
		mColumnNames.append(QString());
	fillColumnNames();
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

void DBExercisesModel::newExercise(const QString& name, const QString& subname, const QString& muscular_group)
{
	setLastID(lastID() + 1);
	appendList(QStringList() << std::move(QString::number(lastID())) << std::move(name) << std::move(subname) << std::move(muscular_group) <<
		std::move("3"_L1) << std::move("12"_L1) << std::move("20"_L1) << std::move("(kg)"_L1) << std::move("qrc:/images/no_image.jpg"_L1) <<
			STR_ZERO << std::move(QString::number(m_modeldata.count())) << STR_ZERO);
}

void DBExercisesModel::removeExercise(const uint index)
{
	beginRemoveRows(QModelIndex(), index, index);
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

void DBExercisesModel::setFilter(const QString& filter, const bool resetSelection)
{
	if (filter.length() >= 3)
	{
		uint idx(0);
		bool bFirst(true);

		QList<QStringList>::const_iterator lst_itr(m_modeldata.constBegin());
		const QList<QStringList>::const_iterator& lst_itrend(m_modeldata.constEnd());
		for(; lst_itr != lst_itrend; ++lst_itr, ++idx)
		{
			const QString& subject{(*lst_itr).at(EXERCISES_COL_MAINNAME) + ' ' +
						(*lst_itr).at(EXERCISES_COL_SUBNAME) + ' ' +
						(*lst_itr).at(EXERCISES_COL_MUSCULARGROUP)};
			const QStringList& words_list{filter.split(' ')};
			bool bFound = true;
			for(uint i(0); i < words_list.count(); ++i)
			{
				if (!subject.contains(words_list.at(i), Qt::CaseInsensitive))
				{
					bFound = false;
					break;
				}
			}
			if (bFound)
			{
				if (bFirst)
				{
					bFirst = false;
					beginRemoveRows(QModelIndex(), 0, count()-1);
					m_indexProxy.clear();
					if (resetSelection)
					{
						resetPrivateData();
						setCurrentRow(-1);
					}
					endRemoveRows();
				}
				beginInsertRows(QModelIndex(), count(), count());
				m_indexProxy.append(idx);
				endInsertRows();
			}
		}
		m_bFilterApplied = m_indexProxy.count() != m_modeldata.count();
	}
	else
	{
		if (m_bFilterApplied)
		{
			m_bFilterApplied = false;
			beginRemoveRows(QModelIndex(), 0, count()-1);
			m_indexProxy.clear();
			if (resetSelection)
			{
				resetPrivateData();
				setCurrentRow(-1);
			}
			endRemoveRows();
			beginInsertRows(QModelIndex(), 0, m_modeldata.count());
			for( uint i (0); i < m_modeldata.count(); ++i )
				m_indexProxy.append(i);
			endInsertRows();
		}
	}
}

void DBExercisesModel::makeFilterString(const QString& text)
{
	m_filterString = text;
	m_filterString = m_filterString.replace(',', ' ').simplified();
	const QStringList& words(m_filterString.split(' '));

	if (words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator& itr_end(words.end());
		m_filterString.clear();

		do
		{
			if((*itr).length() < 3)
				continue;
			if (!m_filterString.isEmpty())
				m_filterString.append('|');
			m_filterString.append((*itr).toLower());
			if (m_filterString.endsWith('s', Qt::CaseInsensitive) )
				m_filterString.chop(1);
			m_filterString.remove('.');
			m_filterString.remove('(');
			m_filterString.remove(')');
		} while (++itr != itr_end);
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
		real_item_pos = m_modeldata.at(m_indexProxy.at(item_pos)).at(10).toUInt();
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
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList modeldata(EXERCISES_TOTAL_COLS);
	uint col(1);
	QString value;
	uint n_items(0);
	const uint databaseLastIndex(appExercisesModel()->m_modeldata.count());

	char buf[256];
	qint64 lineLength(0);
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (col <= EXERCISES_COL_MEDIAPATH)
					{
						if (col != EXERCISES_COL_WEIGHTUNIT)
						{
							value = buf;
							modeldata[col] = value.remove(0, value.indexOf(':') + 2);
						}
						++col;
					}
					else
					{
						++n_items;
						modeldata[EXERCISES_COL_ID] = QString::number(m_exercisesTableLastId + n_items);
						modeldata[EXERCISES_COL_WEIGHTUNIT] = "(kg)"_L1;
						modeldata[EXERCISES_COL_FROMAPPLIST] = STR_ZERO;
						modeldata[EXERCISES_COL_ACTUALINDEX] = QString::number(databaseLastIndex + n_items);
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

bool DBExercisesModel::updateFromModel(const TPListModel* const model)
{
	QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
	const QList<QStringList>::const_iterator& lst_itrend(model->m_modeldata.constEnd());
	uint lastIndex(m_modeldata.count());
	do {
		appendList((*lst_itr));
		m_modifiedIndices[lastIndex]++;
	} while (++lst_itr != lst_itrend);
	return true;
}

QVariant DBExercisesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
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

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
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
					m_modeldata[row][role-Qt::UserRole] = std::move(value.toString());
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = std::move(value.toString());
				emit dataChanged(index, index, QList<int>() << role);
				return true;

			case fromListRole:
			case selectedRole:
				if (!m_bFilterApplied)
					m_modeldata[row][role-Qt::UserRole] = value.toBool() ? STR_ONE : STR_ZERO;
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = value.toBool() ? STR_ONE : STR_ZERO;
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}
