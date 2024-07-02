#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbexercisesmodel.h"

DBMesoSplitModel::DBMesoSplitModel(QObject *parent, const bool bComplete)
	: TPListModel{parent}, m_nextAddedExercisePos(2)
{
	m_tableId = MESOSPLIT_TABLE_ID;
	setObjectName(DBMesoSplitObjectName);

	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[exerciseNameRole] = "exerciseName";
	m_roleNames[exerciseName1Role] = "exerciseName1";
	m_roleNames[exerciseName2Role] = "exerciseName2";
	m_roleNames[setTypeRole] = "setType";
	m_roleNames[setsNumberRole] = "setsNumber";
	m_roleNames[setsSubsetsRole] = "setsSubsets";
	m_roleNames[setsRepsRole] = "setsReps";
	m_roleNames[setsWeightRole] = "setsWeight";
	m_roleNames[setsReps1Role] = "setsReps1";
	m_roleNames[setsWeight1Role] = "setsWeight1";
	m_roleNames[setsReps2Role] = "setsReps2";
	m_roleNames[setsWeight2Role] = "setsWeight2";
	m_roleNames[setsDropSetRole] = "setsDropSet";
	m_roleNames[setsNotesRole] = "setsNotes";

	mb_Complete = bComplete;
	if (bComplete)
	{
		mColumnNames.reserve(MESOSPLIT_COL_NOTES+1);
		mColumnNames.append(tr("Exercise name: "));
		mColumnNames.append(tr("Set type: "));
		mColumnNames.append(tr("Number of sets: "));
		mColumnNames.append(tr("Number of subsets: "));
		mColumnNames.append(tr("Baseline number of reps: "));
		mColumnNames.append(tr("Baseline weight: "));
		mColumnNames.append(tr("Last set is a Drop Set: "));
		mColumnNames.append(tr("Set instructions: "));
	}
	else
	{
		mColumnNames.reserve(8);
		mColumnNames.append(QString()); //id
		mColumnNames.append(QString()); //meso id
		mColumnNames.append(tr("Split A: "));
		mColumnNames.append(tr("Split B: "));
		mColumnNames.append(tr("Split C: "));
		mColumnNames.append(tr("Split D: "));
		mColumnNames.append(tr("Split E: "));
		mColumnNames.append(tr("Split F: "));
	}
}

void DBMesoSplitModel::convertFromTDayModel(DBTrainingDayModel* tDayModel)
{
	m_modeldata.clear();
	m_indexProxy.clear();
	QStringList exerciseInfo;
	QString repsOrweight;
	for (uint i(0); i < tDayModel->m_ExerciseData.count(); ++i)
	{
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->name); //MESOSPLIT_COL_EXERCISENAME
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->type.at(0)); //MESOSPLIT_COL_SETTYPE
		exerciseInfo.append(QString::number(tDayModel->m_ExerciseData.at(i)->nsets)); //MESOSPLIT_COL_SETSNUMBER
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->subsets.at(0)); //MESOSPLIT_COL_SUBSETSNUMBER

		//DBTrainingDayModel can handle composite sets that end with subrecord_separator. DBMesoSplitModel cannot
		repsOrweight = tDayModel->m_ExerciseData.at(i)->reps.at(0);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);
		exerciseInfo.append(repsOrweight); //MESOSPLIT_COL_REPSNUMBER
		repsOrweight = tDayModel->m_ExerciseData.at(i)->weight.at(0);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);
		exerciseInfo.append(repsOrweight); //MESOSPLIT_COL_WEIGHT
		if (tDayModel->setType(tDayModel->m_ExerciseData.at(i)->nsets - 1, i) == SET_TYPE_DROP)
			exerciseInfo.append(u"1"_qs); //MESOSPLIT_COL_DROPSET
		else
			exerciseInfo.append(u"0"_qs); //MESOSPLIT_COL_DROPSET
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->notes.at(0)); //MESOSPLIT_COL_NOTES
		m_modeldata.append(exerciseInfo);
		m_indexProxy.append(i);
		exerciseInfo.clear();
	}
	setReady(true);
}

QVariant DBMesoSplitModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case exerciseNameRole:
				return static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).replace(subrecord_separator, QStringLiteral(" + "));
			case exerciseName1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).indexOf(subrecord_separator));
				return idx != -1 ? QStringLiteral("2: ") + static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).left(idx) :
					m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).isEmpty() ? tr("1: Add exercise ...") : m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME);
			}
			case exerciseName2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).indexOf(subrecord_separator));
				return idx != -1 ? QStringLiteral("2: ") + static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).sliced(idx+1) : tr("2: Add exercise ...");
			}
			case setsNumberRole:
			case setsSubsetsRole:
			case setsRepsRole:
			case setsWeightRole:
			case setsNotesRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case setsDropSetRole:
				return m_modeldata.at(row).at(MESOSPLIT_COL_DROPSET) == u"1"_qs;
			case setsReps1Role:
			case setsWeight1Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-4)).left(idx) : m_modeldata.at(row).at(role-Qt::UserRole-3);
			}
			case setsReps2Role:
			case setsWeight2Role:
			{
				const int idx(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-6)).indexOf(subrecord_separator));
				return idx != -1 ? static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole-6)).sliced(idx+1) : m_modeldata.at(row).at(role-Qt::UserRole-5);
			}
			case setTypeRole:
				return static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_SETTYPE)).toUInt();
		}
	}
	return QVariant();
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value)
{
	const int idx(static_cast<QString>(m_modeldata.at(row).at(column)).indexOf(subrecord_separator));
	if (idx == -1)
		m_modeldata[row][column] = pos == 1 ? value : m_modeldata[row][column] += subrecord_separator + value;
	else
	{
		if (pos == 1)
			m_modeldata[row][column].replace(0, idx, value);
		else
		{
			m_modeldata[row][column].truncate(idx+1);
			m_modeldata[row][column].append(value);
		}
	}
}

bool DBMesoSplitModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case exerciseNameRole:
			case setsRepsRole:
			case setsWeightRole:
			case setsNumberRole:
			case setTypeRole:
			case setsSubsetsRole:
			case setsNotesRole:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
			break;
			case setsDropSetRole:
				m_modeldata[row][MESOSPLIT_COL_DROPSET] = value.toBool() ? u"1"_qs : u"0"_qs;
				emit setsDropSetChanged();
			break;
			case exerciseName1Role:
				replaceCompositeValue(row, MESOSPLIT_COL_EXERCISENAME, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
			break;
			case exerciseName2Role:
				replaceCompositeValue(row, MESOSPLIT_COL_EXERCISENAME, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << exerciseNameRole);
			break;
			case setsReps1Role:
				replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << setsRepsRole);
			case setsReps2Role:
				replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << setsRepsRole);
			break;
			case setsWeight1Role:
				replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 1, value.toString());
				emit dataChanged(index, index, QList<int>() << setsWeightRole);
			case setsWeight2Role:
				replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 2, value.toString());
				emit dataChanged(index, index, QList<int>() << setsWeightRole);
			break;
			default: return false;
		}
		emit dataChanged(index, index, QList<int>() << role);
		setModified(true);
		return true;
	}
	return false;
}

void DBMesoSplitModel::changeExercise(DBExercisesModel *model)
{
	QString name, sets, reps, weight;
	const uint nSel(model->selectedEntriesCount());

	if (nSel == 1)
	{
		name = model->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_qs +
				model->selectedEntriesValue_fast(0, EXERCISES_COL_SUBNAME);
		sets = model->selectedEntriesValue(0, EXERCISES_COL_SETSNUMBER);
		reps = model->selectedEntriesValue(0, EXERCISES_COL_REPSNUMBER);
		weight = model->selectedEntriesValue(0, EXERCISES_COL_WEIGHT);
	}
	else
	{
		for (uint i(0); i < nSel; ++i)
		{
			name += model->selectedEntriesValue_fast(i, EXERCISES_COL_MAINNAME) + u" - "_qs +
					model->selectedEntriesValue_fast(i, EXERCISES_COL_SUBNAME) + subrecord_separator;
			sets += model->selectedEntriesValue(i, EXERCISES_COL_SETSNUMBER) + subrecord_separator;
			reps += model->selectedEntriesValue(i, EXERCISES_COL_REPSNUMBER) + subrecord_separator;
			weight += model->selectedEntriesValue(i, EXERCISES_COL_WEIGHT) + subrecord_separator;
		}
		name.chop(1);
		sets.chop(1);
		reps.chop(1);
		weight.chop(1);
	}

	setExerciseName(name);
	setSetsReps(reps);
	setSetsWeight(weight);
	setSetsNumber(sets);
}

QString DBMesoSplitModel::formatFieldToExport(const QString& fieldValue, const uint field)
{
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		switch (fieldValue.at(0).toLatin1())
		{
			case '0': return tr("Regular"); break;
			case '1': return tr("Pyramid"); break;
			case '2': return tr("Drop Set"); break;
			case '3': return tr("Cluster Set"); break;
			case '4': return tr("Giant Set"); break;
			case '5': return tr("Myo Reps"); break;
			case '6': return tr("Inverted Pyramid"); break;
			default: return QString(); break;
		}
	}
	else
		return fieldValue == u"1"_qs ? tr("Yes") : tr("No");
}

QString DBMesoSplitModel::formatFieldToImport(const QString& fieldValue, const uint field)
{
	QString retStr;
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		if (fieldValue == tr("Regular"))
			retStr = u"0"_qs;
		else if (fieldValue == tr("Pyramid"))
			retStr = u"1"_qs;
		else if (fieldValue == tr("Drop Set"))
			retStr = u"2"_qs;
		else if (fieldValue == tr("Cluster Set"))
			retStr = u"3"_qs;
		else if (fieldValue == tr("Giant Set"))
			retStr = u"4"_qs;
		else if (fieldValue == tr("Myo Reps"))
			retStr = u"5"_qs;
		else if (fieldValue == tr("Inverted Pyramid"))
			retStr = u"6"_qs;
		else
			retStr = u"0"_qs;
	}
	else
		retStr = fieldValue == tr("Yes") ? u"1"_qs : u"0"_qs;
	return retStr;
}

void DBMesoSplitModel::exportToText(QFile* outFile, const bool bFancy) const
{
	QString strHeader;
	if (bFancy)
		strHeader = u"##"_qs + objectName() + u"\n\n"_qs;
	else
		strHeader = u"##0x0"_qs + QString::number(m_tableId) + u"\n"_qs;

	outFile->write(strHeader.toUtf8().constData());
	outFile->write(exportExtraInfo().toUtf8().constData());
	if (bFancy)
		outFile->write("\n\n", 2);
	else
		outFile->write("\n", 1);

	QString value;
	QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
	const QList<QStringList>::const_iterator itr_end(m_modeldata.constEnd());

	while (itr != itr_end)
	{
		for (uint i(0); i < (*itr).count(); ++i)
		{
			if (bFancy)
			{
				if (i < mColumnNames.count())
				{
					if (!mColumnNames.at(i).isEmpty())
					{
						outFile->write(mColumnNames.at(i).toUtf8().constData());
						if (!isFieldFormatSpecial(i))
							value = (*itr).at(i);
						else
							value = formatFieldToExport((*itr).at(i), i);
						outFile->write(value.replace(subrecord_separator, '|').toUtf8().constData());
						outFile->write("\n", 1);
					}
				}
			}
			else
			{
				outFile->write((*itr).at(i).toUtf8().constData());
				outFile->write(QByteArray(1, record_separator.toLatin1()), 1);
			}
		}
		if (bFancy)
			outFile->write("\n", 1);
		else
			outFile->write(QByteArray(1, record_separator2.toLatin1()), 1);
		++itr;
	}

	if (bFancy)
		outFile->write(tr("##End##\n").toUtf8().constData());
	else
		outFile->write("##end##");
}

const QString DBMesoSplitModel::exportExtraInfo() const
{
	return mb_Complete ? tr("Split: ") + m_splitLetter + u" - "_qs + m_muscularGroup : QString();
}

bool DBMesoSplitModel::importExtraInfo(const QString& extrainfo)
{
	int idx(extrainfo.indexOf(':'));
	if (idx != -1)
	{
		setSplitLetter(extrainfo.mid(idx+2, 1));
		idx = extrainfo.indexOf('-', idx+1);
		if (idx != -1)
		{
			setMuscularGroup(extrainfo.mid(idx+2, extrainfo.length() - idx - 3));
			mb_Complete = true;
			m_extraInfo.append(u"1"_qs); //Just any value, so that TPList::importFromFancyText knows we have extra info
			return true;
		}
	}
	mb_Complete = false;
	return true;
}

bool DBMesoSplitModel::importFromFancyText(QFile* inFile, QString& inData)
{
	char buf[256];
	QStringList modeldata;
	int sep_idx(-1);
	uint col(0);

	if (m_extraInfo.isEmpty())
	{
		inData.chop(1);
		int sep_idx(inData.indexOf(':'));
		if (sep_idx != -1)
		{
			modeldata.append(u"-1"_qs); //id
			modeldata.append(u"-1"_qs); //meso id
			modeldata.append(inData.right(inData.length() - sep_idx - 2).replace('|', subrecord_separator));
		}
		else
			return false;
	}

	while (inFile->readLine(buf, sizeof(buf)) != -1) {
		inData = buf;
		inData.chop(1);
		if (inData.isEmpty())
		{
			if (!modeldata.isEmpty())
			{
				appendList(modeldata);
				modeldata.clear();
				col = 0;
			}
		}
		else
		{
			sep_idx = inData.indexOf(':');
			if (sep_idx != -1)
			{
				if (!isFieldFormatSpecial(col))
					modeldata.append(inData.right(inData.length() - sep_idx - 2).replace('|', subrecord_separator));
				else
					modeldata.append(formatFieldToImport(inData.right(inData.length() - sep_idx - 2), col));
				col++;
			}
			else
			{
				if (inData.contains(u"##"_qs))
					break;
			}
		}
	}
	return count() > 0;
}

void DBMesoSplitModel::updateFromModel(TPListModel* model)
{
	if (model->count() > 0)
	{
		clear();
		QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
		do {
			appendList((*lst_itr));
		} while (++lst_itr != lst_itrend);
		setSplitLetter(static_cast<DBMesoSplitModel*>(model)->splitLetter());
		setMuscularGroup(static_cast<DBMesoSplitModel*>(model)->muscularGroup());
	}
}
