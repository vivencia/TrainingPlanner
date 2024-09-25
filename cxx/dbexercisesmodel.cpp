#include "dbexercisesmodel.h"
#include "tpglobals.h"

#include <QFile>

DBExercisesModel* DBExercisesModel::app_exercises_model(nullptr);

DBExercisesModel::DBExercisesModel(QObject* parent)
	: TPListModel{parent}, m_selectedEntryToReplace(0)
{
	if (!app_exercises_model)
		app_exercises_model = this;

	setObjectName(DBExercisesObjectName);
	m_tableId = EXERCISES_TABLE_ID;
	m_fieldCount = EXERCISES_TOTAL_COLS;
	m_exportName = tr("Exercises List");

	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseIdRole] = "exerciseId";
	m_roleNames[mainNameRole] = "mainName";
	m_roleNames[subNameRole] = "subName";
	m_roleNames[muscularGroupRole] = "muscularGroup";
	m_roleNames[nSetsRole] = "nSets";
	m_roleNames[nRepsRole] = "nReps";
	m_roleNames[nWeightRole] = "nWeight";
	m_roleNames[uWeightRole] = "uWeight";
	m_roleNames[mediaPathRole] = "mediaPath";
	m_roleNames[fromListRole] = "fromList";
	m_roleNames[actualIndexRole] = "actualIndex";
	m_roleNames[selectedRole] = "selected";

	filterSearch_Field1 = 3; //First look for muscularGroup
	filterSearch_Field2 = 1; //Then look for mainName

	mColumnNames.reserve(EXERCISES_COL_MEDIAPATH+1);
	mColumnNames.append(QString()); //EXERCISES_COL_ID
	mColumnNames.append(tr("Exercise: "));
	mColumnNames.append(tr("Specifics: "));
	mColumnNames.append(tr("Muscular Group: "));
	mColumnNames.append(tr("Sets: "));
	mColumnNames.append(tr("Reps: "));
	mColumnNames.append(tr("Weight: "));
	mColumnNames.append(QString()); //EXERCISES_COL_WEIGHTUNIT
	mColumnNames.append(tr("Descriptive media: "));
}

bool DBExercisesModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i(count() - 1); i > 0; --i)
	{
		if (getFast(i, 0).toUInt() >= 1000)
			m_exportRows.append(i);
	}
	return m_exportRows.count() > 0;
}

void DBExercisesModel::clear()
{
	clearSelectedEntries();
	TPListModel::clear();
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
						modeldata[EXERCISES_COL_WEIGHTUNIT] = u"(kg)"_qs;
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
	return modeldata.count() > 1 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
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
				return !m_bFilterApplied ? bool(m_modeldata.at(row).at(role-Qt::UserRole) == u"1"_qs) :
					bool(m_modeldata.at(m_indexProxy.at(row)).at(role-Qt::UserRole) == u"1"_qs);
		}
	}
	return QVariant();
}

bool DBExercisesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
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
					m_modeldata[row][role-Qt::UserRole] = value.toString();
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = value.toString();
				emit dataChanged(index, index, QList<int>() << role);
				return true;

			case fromListRole:
			case selectedRole:
				if (!m_bFilterApplied)
					m_modeldata[row][role-Qt::UserRole] = value.toBool() ? u"1"_qs : u"0"_qs;
				else
					m_modeldata[m_indexProxy.at(row)][role-Qt::UserRole] = value.toBool() ? u"1"_qs : u"0"_qs;
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}

void DBExercisesModel::clearSelectedEntries()
{
	for (uint i(0); i < m_selectedEntries.count(); ++i)
	{
		m_modeldata[m_selectedEntries.at(i).real_index][EXERCISES_COL_SELECTED] = u"0"_qs;
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
			m_modeldata[m_selectedEntries.at(m_selectedEntryToReplace).real_index][EXERCISES_COL_SELECTED] = u"0"_qs;
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
				m_modeldata[m_selectedEntries.at(0).real_index][EXERCISES_COL_SELECTED] = u"0"_qs;
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
		m_modeldata[m_selectedEntries.at(idx).real_index][EXERCISES_COL_SELECTED] = u"0"_qs;
			emit dataChanged(index(m_selectedEntries.at(idx).view_index, 0),
					index(m_selectedEntries.at(idx).view_index, 0), QList<int>() << selectedRole);
		m_selectedEntries.remove(idx, 1);
		return false;
	}
	m_modeldata[real_item_pos][EXERCISES_COL_SELECTED] = u"1"_qs;
	emit dataChanged(index(item_pos, 0), index(item_pos, 0), QList<int>() << selectedRole);
	return true;
}
