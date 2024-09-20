#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "tpappcontrol.h"
#include "tputils.h"

static const QLatin1Char fancy_record_separator2(';');

DBMesoSplitModel::DBMesoSplitModel(QObject* parent, const bool bComplete, const int meso_idx)
	: TPListModel(parent, meso_idx), m_nextAddedExercisePos(2), mb_Complete(bComplete)
{
	setObjectName(DBMesoSplitObjectName);
	m_tableId = MESOSPLIT_TABLE_ID;
	m_exportName = tr("Exercises Program");

	if (mb_Complete)
	{
		m_modeldata.reserve(COMPLETE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.reserve(COMPLETE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.append(tr("Exercise name: "));
		mColumnNames.append(tr("Number of sets: "));
		mColumnNames.append(tr("Set instructions: "));
		mColumnNames.append(tr("Set type: "));
		mColumnNames.append(tr("Number of subsets: "));
		mColumnNames.append(tr("Reps: "));
		mColumnNames.append(tr("Weight: "));
		mColumnNames.append(QString()); //MESOSPLIT_COL_WORKINGSET
	}
	else
	{
		m_modeldata.reserve(SIMPLE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.reserve(SIMPLE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.append(QString()); //MESOSPLIT_COL_ID
		mColumnNames.append(QString()); //MESOSPLIT_COL_MESOID
		mColumnNames.append(tr("Split A: "));
		mColumnNames.append(tr("Split B: "));
		mColumnNames.append(tr("Split C: "));
		mColumnNames.append(tr("Split D: "));
		mColumnNames.append(tr("Split E: "));
		mColumnNames.append(tr("Split F: "));
	}
}

void DBMesoSplitModel::convertFromTDayModel(const DBTrainingDayModel* const tDayModel)
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
	appUtils()->setCompositeValue(workingSet(row), new_setsnotes, m_modeldata[row][MESOSPLIT_COL_NOTES], record_separator2.toLatin1());
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
	return appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SETTYPE), record_separator2.toLatin1()).toUInt();
}

void DBMesoSplitModel::setSetType(const uint row, const uint new_type)
{
	appUtils()->setCompositeValue(workingSet(row), QString::number(new_type), m_modeldata[row][MESOSPLIT_COL_SETTYPE], record_separator2.toLatin1());
	setModified(true);
	emit setTypeChanged();
}

QString DBMesoSplitModel::setsSubsets(const uint row) const
{
	return appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_SUBSETSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsSubsets(const uint row, const QString& new_setssubsets)
{
	appUtils()->setCompositeValue(workingSet(row), new_setssubsets, m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER], record_separator2.toLatin1());
	setModified(true);
}

QString DBMesoSplitModel::setsReps(const uint row) const
{
	return appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsReps(const uint row, const QString& new_setsreps)
{
	appUtils()->setCompositeValue(workingSet(row), new_setsreps, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], record_separator2.toLatin1());
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
	return appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT), record_separator2.toLatin1());
}

void DBMesoSplitModel::setSetsWeight(const uint row, const QString& new_setsweight)
{
	appUtils()->setCompositeValue(workingSet(row), new_setsweight, m_modeldata[row][MESOSPLIT_COL_WEIGHT], record_separator2.toLatin1());
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

void DBMesoSplitModel::changeExercise(const DBExercisesModel* const model)
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

static void muscularGroupSimplified(QString& muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList words(muscularGroup.split(' '));

	if ( words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator itr_end(words.end());
		muscularGroup.clear();

		do
		{
			if(static_cast<QString>(*itr).length() < 3)
				continue;
			if (!muscularGroup.isEmpty())
				muscularGroup.append(' ');
			muscularGroup.append(static_cast<QString>(*itr).toLower());
			if (muscularGroup.endsWith('s', Qt::CaseInsensitive) )
				muscularGroup.chop(1);
			muscularGroup.remove('.');
			muscularGroup.remove('(');
			muscularGroup.remove(')');
		} while (++itr != itr_end);
	}
}

QString DBMesoSplitModel::findSwappableModel() const
{
	QString muscularGroup1(appMesoModel()->getMuscularGroup(mesoIdx(), splitLetter()));
	if (!muscularGroup1.isEmpty())
	{
		muscularGroupSimplified(muscularGroup1);
		QString muscularGroup2;
		const QString mesoSplit(appMesoModel()->getFast(mesoIdx(), MESOCYCLES_COL_SPLIT));
		QString::const_iterator itr(mesoSplit.constBegin());
		const QString::const_iterator itr_end(mesoSplit.constEnd());

		do {
			if (static_cast<QChar>(*itr) == QChar('R'))
				continue;
			else if (static_cast<QChar>(*itr) == splitLetter().at(0))
				continue;

			muscularGroup2 = appMesoModel()->getMuscularGroup(mesoIdx(), *itr);
			if (!muscularGroup2.isEmpty())
			{
				muscularGroupSimplified(muscularGroup2);
				if (appUtils()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
					return QString(*itr);
			}
		} while (++itr != itr_end);
	}
	return QString();
}

bool DBMesoSplitModel::exportToFile(const QString& filename, const bool, const bool) const
{
	QFile* outFile{new QFile(filename)};
	const bool bOK(outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text));
	if (bOK)
	{
		const QString strHeader(u"## "_qs + exportName() + u"\n\n"_qs);
		outFile->write(strHeader.toUtf8().constData());

		QString value;
		uint nsets(0);
		QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
		const QList<QStringList>::const_iterator itr_end(m_modeldata.constEnd());

		while (itr != itr_end)
		{
			for (uint i(0); i < (*itr).count(); ++i)
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
								value.append(appUtils()->getCompositeValue(x, (*itr).at(i), record_separator2.toLatin1()) + fancy_record_separator2);
								value.replace(subrecord_separator, '|');
							}
							else
								value.append(formatFieldToExport(i, appUtils()->getCompositeValue(x, (*itr).at(i), record_separator2.toLatin1())) + fancy_record_separator2);
						}
						outFile->write(value.replace(record_separator2, fancy_record_separator2).toUtf8().constData());
						outFile->write("\n", 1);
					}
				}
			}
			outFile->write("\n", 1);
			++itr;
		}
		outFile->write(STR_END_EXPORT.toUtf8().constData());
		outFile->close();
	}
	delete outFile;
	return bOK;
}

//Only for a complete meso split
bool DBMesoSplitModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return false;
	}

	char buf[512];
	qint64 lineLength(0);
	uint col(1);
	QString value;
	bool bexpect_extrainfo(false);

	QStringList modeldata(COMPLETE_MESOSPLIT_TOTAL_COLS);
	modeldata[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (bexpect_extrainfo)
					{
						bexpect_extrainfo = !importExtraInfo(buf);
						continue;
					}
					value = buf;
					value.remove(0, value.indexOf(':') + 2);
					if (!isFieldFormatSpecial(col))
					{
						switch(col)
						{
							case MESOSPLIT_COL_EXERCISENAME:
								modeldata[col] = value.replace('|', subrecord_separator);
							break;
							case MESOSPLIT_COL_SETSNUMBER:
							case MESOSPLIT_COL_NOTES:
								modeldata[col] = value;
							break;
							default:
								modeldata[col] = value.replace('|', subrecord_separator).replace(fancy_record_separator2, record_separator2) + record_separator2;
						}
					}
					else
						modeldata[col] = formatFieldToImport(col, value);
					col++;
					if (col == MESOSPLIT_COL_WORKINGSET)
					{
						m_modeldata.append(modeldata);
						col = 0;
					}
				}
				else
					bexpect_extrainfo = true;
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	return modeldata.count() > 1;
}

bool DBMesoSplitModel::updateFromModel(const TPListModel* const model)
{
	clear();
	QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
	const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
	do {
		appendList((*lst_itr));
	} while (++lst_itr != lst_itrend);
	setSplitLetter(static_cast<DBMesoSplitModel* const>(const_cast<TPListModel*>(model))->splitLetter());
	setMuscularGroup(static_cast<DBMesoSplitModel* const>(const_cast<TPListModel*>(model))->muscularGroup());
	setMesoIdx(static_cast<DBMesoSplitModel* const>(const_cast<TPListModel*>(model))->mesoIdx());
	setImportMode(true);
	return true;
}

const QString DBMesoSplitModel::exportExtraInfo() const
{
	return mb_Complete ? tr("Split: ") + m_splitLetter + u" - "_qs + m_muscularGroup : QString();
}

QString DBMesoSplitModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	if (field == MESOSPLIT_COL_SETTYPE)
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
	else
		return QString();
}

QString DBMesoSplitModel::formatFieldToImport(const uint field, const QString& fieldValue) const
{
	QString retStr;
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		if (!fieldValue.isEmpty())
		{
			QString setTypeStr;
			const uint n(fieldValue.count(fancy_record_separator2));
			for (uint i(0); i <= n; ++i)
			{
				setTypeStr = appUtils()->getCompositeValue(i, fieldValue, fancy_record_separator2.toLatin1());
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
	}
	return retStr;
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
	return false;
}

QString DBMesoSplitModel::getFromCompositeValue(const uint row, const uint column, const uint pos) const
{
	const QString value(appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(column), record_separator2.toLatin1()));
	const int idx(value.indexOf(subrecord_separator));
	return idx != -1 ? pos == 1 ? value.left(idx) : value.sliced(idx+1) : value;
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value)
{
	QString fieldValue(appUtils()->getCompositeValue(workingSet(row), m_modeldata.at(row).at(column), record_separator2.toLatin1()));

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
	appUtils()->setCompositeValue(workingSet(row), fieldValue, m_modeldata[row][column], record_separator2.toLatin1());
	setModified(true);
}
