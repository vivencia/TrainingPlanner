#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbexercisesmodel.h"
#include "runcommands.h"

DBMesoSplitModel::DBMesoSplitModel(QObject *parent, const bool bComplete)
	: TPListModel(parent), m_nextAddedExercisePos(2)
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

QString DBMesoSplitModel::muscularGroup() const
{
	return m_muscularGroup;
}

void DBMesoSplitModel::setMuscularGroup(const QString& muscularGroup)
{
	m_muscularGroup = muscularGroup;
	setModified(true);
	emit muscularGroupChanged();
}

QString DBMesoSplitModel::splitLetter() const
{
	return QString(m_splitLetter);
}

void DBMesoSplitModel::setSplitLetter(const QChar& splitLetter)
{
	m_splitLetter = splitLetter;
	setModified(true);
	emit splitLetterChanged();
}

const QString DBMesoSplitModel::exerciseName(const uint row)
{
	QString name(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME));
	return name.replace(subrecord_separator, QStringLiteral(" + "));
}

void DBMesoSplitModel::setExerciseName(const uint row, const QString& new_name)
{
	m_modeldata[row][MESOSPLIT_COL_EXERCISENAME] = new_name;
	setModified(true);
	emit exerciseNameChanged();
}

QString DBMesoSplitModel::exerciseName1(const uint row) const
{
	const int idx(static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).indexOf(subrecord_separator));
	return idx != -1 ? QStringLiteral("2: ") + static_cast<QString>(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME)).left(idx) :
			m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).isEmpty() ? tr("1: Add exercise ...") : m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME);
}

void DBMesoSplitModel::setExerciseName1(const uint row, const QString& new_name)
{
	replaceCompositeValue(row, MESOSPLIT_COL_EXERCISENAME, 1, new_name);
	emit exerciseNameChanged();
}

QString DBMesoSplitModel::exerciseName2(const uint row) const
{
	const int idx(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).indexOf(subrecord_separator));
	return idx != -1 ? QStringLiteral("2: ") + m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).sliced(idx+1) : tr("2: Add exercise ...");
}

void DBMesoSplitModel::setExerciseName2(const uint row, const QString& new_name)
{
	replaceCompositeValue(row, MESOSPLIT_COL_EXERCISENAME, 2, new_name);
	emit exerciseNameChanged();
}

void DBMesoSplitModel::addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight)
{
	appendList(QStringList() << exercise_name << QString::number(settype) << sets << u"0"_qs << reps << weight << u"0"_qs << u" "_qs << u"0"_qs);
	setCurrentRow(count() - 1);
}

uint DBMesoSplitModel::setType(const uint row) const
{
	return runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SETTYPE), record_separator2.toLatin1()).toUInt();
}

void DBMesoSplitModel::setSetType(const uint row, const uint new_type)
{
	static_cast<void>(runCmd()->setCompositeValue(getWorkingSet(row), QString::number(new_type), m_modeldata[row][MESOSPLIT_COL_SETTYPE], record_separator2.toLatin1()));
	setModified(true);
	emit setTypeChanged();
}

uint DBMesoSplitModel::setsNumber(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_SETSNUMBER).toUInt();
}

void DBMesoSplitModel::setSetsNumber(const uint row, const uint new_setsnumber)
{
	m_modeldata[row][MESOSPLIT_COL_SETSNUMBER] = QString::number(new_setsnumber);
	setModified(true);
}

uint DBMesoSplitModel::workingSet(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_WORKINGSET).toUInt();
}

void DBMesoSplitModel::setWorkingSet(const uint row, const uint new_workingset)
{
	m_modeldata[row][MESOSPLIT_COL_WORKINGSET] = QString::number(new_workingset);
	emit workingSetChanged();
}

QString DBMesoSplitModel::setsSubsets(const uint row) const
{
	return runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SUBSETSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsSubsets(const uint row, const QString& new_setssubsets)
{
	static_cast<void>(runCmd()->setCompositeValue(getWorkingSet(row), new_setssubsets, m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER], record_separator2.toLatin1()));
	setModified(true);
}

QString DBMesoSplitModel::setsReps(const uint row) const
{
	return runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsReps(const uint row, const QString& new_setsreps)
{
	static_cast<void>(runCmd()->setCompositeValue(getWorkingSet(row), new_setsreps, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], record_separator2.toLatin1()));
	setModified(true);
}

QString DBMesoSplitModel::setsReps1(const uint row) const
{
	const QString value(runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? value.left(idx) : value;
}

void DBMesoSplitModel::setSetsReps1(const uint row, const QString& new_setsreps)
{
	replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 1, new_setsreps);
}

QString DBMesoSplitModel::setsReps2(const uint row) const
{
	const QString value(runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? value.sliced(idx+1) : value;
}

void DBMesoSplitModel::setSetsReps2(const uint row, const QString& new_setsreps)
{
	replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 2, new_setsreps);
}

QString DBMesoSplitModel::setsWeight(const uint row) const
{
	return runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsWeight(const uint row, const QString& new_setsweight)
{
	static_cast<void>(runCmd()->setCompositeValue(getWorkingSet(row), new_setsweight, m_modeldata[row][MESOSPLIT_COL_WEIGHT], record_separator2.toLatin1()));
	setModified(true);
}

QString DBMesoSplitModel::setsWeight1(const uint row) const
{
	const QString value(runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? value.left(idx) : value;
}

void DBMesoSplitModel::setSetsWeight1(const uint row, const QString& new_setsweight)
{
	replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 1, new_setsweight);
}

QString DBMesoSplitModel::setsWeight2(const uint row) const
{
	const QString value(runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? value.sliced(idx+1) : value;
}

void DBMesoSplitModel::setSetsWeight2(const uint row, const QString& new_setsweight)
{
	replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 2, new_setsweight);
}

bool DBMesoSplitModel::setsDropSet(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_DROPSET) == u"1"_qs;
}

void DBMesoSplitModel::setSetsDropSet(const uint row, const bool bDropSet)
{
	m_modeldata[row][MESOSPLIT_COL_DROPSET] = bDropSet ? u"1"_qs : u"0"_qs;
	setModified(true);
}

QString DBMesoSplitModel::setsNotes(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_NOTES);
}

void DBMesoSplitModel::setSetsNotes(const uint row, const QString& new_setsnotes)
{
	setData(index(currentRow(), MESOSPLIT_COL_NOTES), new_setsnotes, setsNotesRole);
	setModified(true);
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value)
{
	QString fieldValue(runCmd()->getCompositeValue(getWorkingSet(row), m_modeldata.at(row).at(column), record_separator2.toLatin1()));

	const int idx(fieldValue.indexOf(subrecord_separator));
	if (idx == -1)
		fieldValue = pos == 1 ? value : fieldValue += subrecord_separator + value;
	else
	{
		if (pos == 1)
			fieldValue.replace(0, idx, value);
		else
		{
			fieldValue.truncate(idx+1);
			fieldValue.append(value);
		}
	}
	static_cast<void>(runCmd()->setCompositeValue(getWorkingSet(row), fieldValue, m_modeldata[row][column], record_separator2.toLatin1()));
	setModified(true);
}

void DBMesoSplitModel::changeExercise(DBExercisesModel *model)
{
	QString name, reps, weight;
	const uint nSel(model->selectedEntriesCount());

	if (nSel == 1)
	{
		name = model->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_qs +
				model->selectedEntriesValue_fast(0, EXERCISES_COL_SUBNAME);
		reps = model->selectedEntriesValue(0, EXERCISES_COL_REPSNUMBER);
		weight = model->selectedEntriesValue(0, EXERCISES_COL_WEIGHT);
	}
	else
	{
		for (uint i(0); i < nSel; ++i)
		{
			name += model->selectedEntriesValue_fast(i, EXERCISES_COL_MAINNAME) + u" - "_qs +
					model->selectedEntriesValue_fast(i, EXERCISES_COL_SUBNAME) + subrecord_separator;
			reps += model->selectedEntriesValue(i, EXERCISES_COL_REPSNUMBER) + subrecord_separator;
			weight += model->selectedEntriesValue(i, EXERCISES_COL_WEIGHT) + subrecord_separator;
		}
		name.chop(1);
		reps.chop(1);
		weight.chop(1);
	}

	setExerciseName(currentRow(), name);
	setSetsReps(currentRow(), reps);
	setSetsWeight(currentRow(), weight);
	setSetsNumber(currentRow(), model->selectedEntriesValue(0, EXERCISES_COL_SETSNUMBER).toUInt());
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
	int valueLen(0);

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
				valueLen = inData.length() - sep_idx - 2;
				if (valueLen <= 0)
					modeldata.append(u" "_qs); //QString::split(Qt::SkipEmptyParts) will yield an empty QStringList if we dont provide at least one character
				else
				{
					if (!isFieldFormatSpecial(col))
						modeldata.append(inData.right(valueLen).replace('|', subrecord_separator));
					else
						modeldata.append(formatFieldToImport(inData.right(valueLen), col));
				}
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

bool DBMesoSplitModel::updateFromModel(const TPListModel* model)
{
	if (model->count() > 0)
	{
		clear();
		QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
		do {
			appendList((*lst_itr));
		} while (++lst_itr != lst_itrend);
		setSplitLetter(static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model))->splitLetter());
		setMuscularGroup(static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model))->muscularGroup());
		return true;
	}
	return false;
}
