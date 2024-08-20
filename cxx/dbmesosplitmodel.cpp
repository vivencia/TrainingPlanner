#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbexercisesmodel.h"
#include "runcommands.h"

static const QLatin1Char fancy_record_separator2(';');

DBMesoSplitModel::DBMesoSplitModel(QObject *parent, const bool bComplete)
	: TPListModel(parent), m_nextAddedExercisePos(2)
{
	m_tableId = MESOSPLIT_TABLE_ID;
	setObjectName(DBMesoSplitObjectName);

	mb_Complete = bComplete;
	if (bComplete)
	{
		mColumnNames.reserve(MESOSPLIT_COL_WORKINGSET);
		mColumnNames.append(tr("Exercise name: "));
		mColumnNames.append(tr("Number of sets: "));
		mColumnNames.append(tr("Set instructions: "));
		mColumnNames.append(tr("Set type: "));
		mColumnNames.append(tr("Number of subsets: "));
		mColumnNames.append(tr("Reps: "));
		mColumnNames.append(tr("Weight: "));
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
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->type.join(subrecord_separator)); //MESOSPLIT_COL_SETTYPE
		exerciseInfo.append(QString::number(tDayModel->m_ExerciseData.at(i)->nsets)); //MESOSPLIT_COL_SETSNUMBER
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->subsets.join(subrecord_separator)); //MESOSPLIT_COL_SUBSETSNUMBER

		//DBTrainingDayModel can handle composite sets that end with subrecord_separator. DBMesoSplitModel cannot
		repsOrweight = tDayModel->m_ExerciseData.at(i)->reps.join(subrecord_separator);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);
		exerciseInfo.append(repsOrweight); //MESOSPLIT_COL_REPSNUMBER
		repsOrweight = tDayModel->m_ExerciseData.at(i)->weight.join(subrecord_separator);
		if (repsOrweight.endsWith(subrecord_separator))
			repsOrweight.chop(1);
		exerciseInfo.append(repsOrweight); //MESOSPLIT_COL_WEIGHT
		exerciseInfo.append(tDayModel->m_ExerciseData.at(i)->notes.join(subrecord_separator)); //MESOSPLIT_COL_NOTES
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

const QString DBMesoSplitModel::exerciseName(const int row)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		QString name(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME));
		return name.replace(subrecord_separator, u" + "_qs);
	}
	return QString();
}

void DBMesoSplitModel::setExerciseName(const uint row, const QString& new_name)
{
	QString name(new_name);
	m_modeldata[row][MESOSPLIT_COL_EXERCISENAME] = name.replace(u" + "_qs, QString(subrecord_separator));
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
	appendList(QStringList() << exercise_name << sets << u" "_qs << QString::number(settype) << u"0"_qs << reps << weight << u"0"_qs);
	setCurrentRow(count() - 1);
}

QString DBMesoSplitModel::setsNotes(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_NOTES);
}

void DBMesoSplitModel::setSetsNotes(const uint row, const QString& new_setsnotes)
{
	runCmd()->setCompositeValue(workingSet(row), new_setsnotes, m_modeldata[row][MESOSPLIT_COL_NOTES], record_separator2.toLatin1());
	setModified(true);
}

uint DBMesoSplitModel::setsNumber(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_SETSNUMBER).toUInt();
}

void DBMesoSplitModel::setSetsNumber(const uint row, const uint new_setsnumber)
{
	m_modeldata[row][MESOSPLIT_COL_SETSNUMBER] = QString::number(new_setsnumber);
	setModified(true);
	emit nbrSetsChanged();
}

void DBMesoSplitModel::addSet(const uint row)
{
	uint nsets(m_modeldata.at(row).at(MESOSPLIT_COL_SETSNUMBER).toUInt());
	if (nsets < 10)
	{
		++nsets;
		setSetsNumber(row, nsets);
	}
}

void DBMesoSplitModel::delSet(const uint row)
{
	uint nsets(m_modeldata.at(row).at(MESOSPLIT_COL_SETSNUMBER).toUInt());
	if (nsets > 1)
	{
		--nsets;
		setSetsNumber(row, nsets);
	}
}

uint DBMesoSplitModel::setType(const uint row) const
{
	return runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SETTYPE), record_separator2.toLatin1()).toUInt();
}

void DBMesoSplitModel::setSetType(const uint row, const uint new_type)
{
	runCmd()->setCompositeValue(workingSet(row), QString::number(new_type), m_modeldata[row][MESOSPLIT_COL_SETTYPE], record_separator2.toLatin1());
	setModified(true);
	emit setTypeChanged();
}

QString DBMesoSplitModel::setsSubsets(const uint row) const
{
	return runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SUBSETSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsSubsets(const uint row, const QString& new_setssubsets)
{
	runCmd()->setCompositeValue(workingSet(row), new_setssubsets, m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER], record_separator2.toLatin1());
	setModified(true);
}

QString DBMesoSplitModel::setsReps(const uint row) const
{
	return runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsReps(const uint row, const QString& new_setsreps)
{
	runCmd()->setCompositeValue(workingSet(row), new_setsreps, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], record_separator2.toLatin1());
	setModified(true);
}

QString DBMesoSplitModel::setsReps1(const uint row) const
{
	return getFromCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 1);
}

void DBMesoSplitModel::setSetsReps1(const uint row, const QString& new_setsreps)
{
	replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 1, new_setsreps);
}

QString DBMesoSplitModel::setsReps2(const uint row) const
{
	return getFromCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 2);
}

void DBMesoSplitModel::setSetsReps2(const uint row, const QString& new_setsreps)
{
	replaceCompositeValue(row, MESOSPLIT_COL_REPSNUMBER, 2, new_setsreps);
}

QString DBMesoSplitModel::setsWeight(const uint row) const
{
	return runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsWeight(const uint row, const QString& new_setsweight)
{
	runCmd()->setCompositeValue(workingSet(row), new_setsweight, m_modeldata[row][MESOSPLIT_COL_WEIGHT], record_separator2.toLatin1());
	setModified(true);
}

QString DBMesoSplitModel::setsWeight1(const uint row) const
{
	return getFromCompositeValue(row, MESOSPLIT_COL_WEIGHT, 1);
}

void DBMesoSplitModel::setSetsWeight1(const uint row, const QString& new_setsweight)
{
	replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 1, new_setsweight);
}

QString DBMesoSplitModel::setsWeight2(const uint row) const
{
	return getFromCompositeValue(row, MESOSPLIT_COL_WEIGHT, 2);
}

void DBMesoSplitModel::setSetsWeight2(const uint row, const QString& new_setsweight)
{
	replaceCompositeValue(row, MESOSPLIT_COL_WEIGHT, 2, new_setsweight);
}

void DBMesoSplitModel::setWorkingSet(const uint row, const uint new_workingset, const bool emitSignal)
{
	m_modeldata[row][MESOSPLIT_COL_WORKINGSET] = QString::number(new_workingset);
	if (emitSignal)
		emit workingSetChanged();
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

QString DBMesoSplitModel::formatFieldToExport(const QString& fieldValue)
{
	if (fieldValue.isEmpty())
		return tr("Regular");
	switch (fieldValue.at(0).toLatin1())
	{
		default: return tr("Regular"); break;
		case '1': return tr("Pyramid"); break;
		case '2': return tr("Drop Set"); break;
		case '3': return tr("Cluster Set"); break;
		case '4': return tr("Giant Set"); break;
		case '5': return tr("Myo Reps"); break;
		case '6': return tr("Inverted Pyramid"); break;
	}
}

QString DBMesoSplitModel::formatFieldToImport(const QString& fieldValue)
{
	QString retStr;
	if (!fieldValue.isEmpty())
	{
		QString setTypeStr;
		const uint n(fieldValue.count(fancy_record_separator2));
		for (uint i(0); i <= n; ++i)
		{
			setTypeStr = runCmd()->getCompositeValue(i, fieldValue, fancy_record_separator2.toLatin1());
			if (setTypeStr == tr("Regular"))
				retStr.append(u"0"_qs + record_separator2);
			else if (setTypeStr == tr("Pyramid"))
				retStr.append(u"1"_qs + record_separator2);
			else if (setTypeStr == tr("Drop Set"))
				retStr.append(u"2"_qs + record_separator2);
			else if (setTypeStr == tr("Cluster Set"))
				retStr.append(u"3"_qs + record_separator2);
			else if (setTypeStr == tr("Giant Set"))
				retStr.append(u"4"_qs + record_separator2);
			else if (setTypeStr == tr("Myo Reps"))
				retStr.append(u"5"_qs + record_separator2);
			else if (setTypeStr == tr("Inverted Pyramid"))
				retStr.append(u"6"_qs + record_separator2);
			else
				retStr.append(u"0"_qs + record_separator2);
		}
	}
	else
		retStr = u"0"_qs + record_separator2;
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
	uint nsets(0);
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
						value.clear();
						nsets = (*itr).at(MESOSPLIT_COL_SETSNUMBER).toUInt();
						for (uint x(0); x < nsets; ++x)
						{
							if (!isFieldFormatSpecial(i))
							{
								value.append(runCmd()->getCompositeValue(x, (*itr).at(i), record_separator2.toLatin1()) + fancy_record_separator2);
								value.replace(subrecord_separator, '|');
							}
							else
								value.append(formatFieldToExport(runCmd()->getCompositeValue(x, (*itr).at(i), record_separator2.toLatin1())) + fancy_record_separator2);
						}
						outFile->write(value.replace(record_separator2, fancy_record_separator2).toUtf8().constData());
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
				modeldata.append(u"0"_qs); //MESOSPLIT_COL_WORKINGSET
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
					modeldata.append(u" "_qs); //QString::split(Qt::SkipEmptyParts) will yield an empty QStringList if we don't provide at least one character
				else
				{
					if (!isFieldFormatSpecial(col))
					{
						switch(col)
						{
							case MESOSPLIT_COL_EXERCISENAME:
								modeldata.append(inData.right(valueLen).replace('|', subrecord_separator));
							break;
							case MESOSPLIT_COL_SETSNUMBER:
							case MESOSPLIT_COL_NOTES:
								modeldata.append(inData.right(valueLen));
							break;
							default:
								modeldata.append(inData.right(valueLen).replace('|', subrecord_separator).replace(fancy_record_separator2, record_separator2) + record_separator2);
						}
					}
					else
						modeldata.append(formatFieldToImport(inData.right(valueLen)));
				}
				//qDebug() << mColumnNames.at(col) << ":  " << modeldata.at(modeldata.count()-1); //When enabled, import worked on Android. Before, it was not working
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

QString DBMesoSplitModel::getFromCompositeValue(const uint row, const uint column, const uint pos) const
{
	const QString value(runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(column), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? pos == 1 ? value.left(idx) : value.sliced(idx+1) : value;
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value)
{
	QString fieldValue(runCmd()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(column), record_separator2.toLatin1()));

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
	runCmd()->setCompositeValue(workingSet(row), fieldValue, m_modeldata[row][column], record_separator2.toLatin1());
	setModified(true);
}
