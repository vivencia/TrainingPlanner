#include "dbmesosplitmodel.h"
#include "tpglobals.h"
#include "dbtrainingdaymodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "tputils.h"

#include <utility>

DBMesoSplitModel::DBMesoSplitModel(QObject* parent, const bool bComplete, const uint meso_idx)
	: TPListModel(parent, static_cast<int>(meso_idx)), m_nextAddedExercisePos(2), mb_Complete(bComplete)
{
	setObjectName(DBMesoSplitObjectName);
	m_tableId = MESOSPLIT_TABLE_ID;
	m_exportName = std::move(tr("Exercises Program"));

	if (mb_Complete)
	{
		m_fieldCount = COMPLETE_MESOSPLIT_TOTAL_COLS;
		m_modeldata.reserve(COMPLETE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.reserve(COMPLETE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.append(std::move(tr("Exercise name: ")));
		mColumnNames.append(std::move(tr("Number of sets: ")));
		mColumnNames.append(std::move(tr("Set instructions: ")));
		mColumnNames.append(std::move(tr("Set type: ")));
		mColumnNames.append(std::move(tr("Number of subsets: ")));
		mColumnNames.append(std::move(tr("Reps: ")));
		mColumnNames.append(std::move(tr("Weight: ")));
		mColumnNames.append(QString()); //MESOSPLIT_COL_WORKINGSET
	}
	else
	{
		m_fieldCount = SIMPLE_MESOSPLIT_TOTAL_COLS;
		m_modeldata.reserve(SIMPLE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.reserve(SIMPLE_MESOSPLIT_TOTAL_COLS);
		mColumnNames.append(QString()); //MESOSPLIT_COL_ID
		mColumnNames.append(QString()); //MESOSPLIT_COL_MESOID
		mColumnNames.append(std::move(tr("Split A: ")));
		mColumnNames.append(std::move(tr("Split B: ")));
		mColumnNames.append(std::move(tr("Split C: ")));
		mColumnNames.append(std::move(tr("Split D: ")));
		mColumnNames.append(std::move(tr("Split E: ")));
		mColumnNames.append(std::move(tr("Split F: ")));
	}
}

void DBMesoSplitModel::convertFromTDayModel(const DBTrainingDayModel* const tDayModel)
{
	m_modeldata.clear();
	QStringList exerciseInfo(COMPLETE_MESOSPLIT_TOTAL_COLS);
	exerciseInfo[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;

	for (uint i(0); i < tDayModel->m_ExerciseData.count(); ++i)
	{
		exerciseInfo[MESOSPLIT_COL_EXERCISENAME] = tDayModel->_exerciseName(i);
		exerciseInfo[MESOSPLIT_COL_SETSNUMBER] = tDayModel->_setsNumber(i);
		exerciseInfo[MESOSPLIT_COL_NOTES] = tDayModel->setsNotes(i);
		exerciseInfo[MESOSPLIT_COL_SETTYPE] = tDayModel->setsTypes(i);
		exerciseInfo[MESOSPLIT_COL_SUBSETSNUMBER] = tDayModel->setsSubSets(i);
		exerciseInfo[MESOSPLIT_COL_REPSNUMBER] = tDayModel->setsReps(i);
		exerciseInfo[MESOSPLIT_COL_WEIGHT] = tDayModel->setsWeight(i);
		m_modeldata.append(exerciseInfo);
	}
	setReady(true);
}

void DBMesoSplitModel::setModified(const uint row, const uint field)
{
	if (isExerciseNew(row))
	{
		unSetBit(m_exerciseIsNew[row], field);
		if (isExerciseNew(row))
			return;
	}
	emit splitChanged(row, field);
}

void DBMesoSplitModel::addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight)
{
	appendList(QStringList() << exercise_name << sets << " "_L1 << QString::number(settype) << STR_ZERO << reps << weight << STR_ZERO);
	setCurrentRow(count() - 1);
	uchar newExerciseRequiredFields(0);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_EXERCISENAME);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_SETSNUMBER);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_SETTYPE);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_REPSNUMBER);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_WEIGHT);
	m_exerciseIsNew.append(newExerciseRequiredFields);
}

void DBMesoSplitModel::addSet(const uint row)
{
	uint nsets(setsNumber(row));
	if (nsets < 10)
	{
		++nsets;
		setSetsNumber(row, nsets);
	}
}

void DBMesoSplitModel::delSet(const uint row)
{
	uint nsets(setsNumber(row));
	if (nsets > 1)
	{
		--nsets;
		setSetsNumber(row, nsets);
	}
}

const QString DBMesoSplitModel::exerciseName(const int row)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		QString name(_exerciseName(static_cast<uint>(row)));
		return name.replace(comp_exercise_separator, comp_exercise_fancy_separator);
	}
	return QString();
}

void DBMesoSplitModel::setExerciseName(const uint row, const QString& new_name)
{
	QString name{new_name};
	m_modeldata[row][MESOSPLIT_COL_EXERCISENAME] = std::move(name.replace(comp_exercise_fancy_separator, QString(comp_exercise_separator)));
	emit exerciseNameChanged();
	setModified(row, MESOSPLIT_COL_EXERCISENAME);
}

QString DBMesoSplitModel::exerciseName1(const uint row) const
{
	const int idx(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).indexOf(comp_exercise_separator));
	return idx != -1 ? "2: "_L1 + m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).first(idx) :
						m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).isEmpty() ?
						tr("1: Add exercise ...") : m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME);
}

void DBMesoSplitModel::setExerciseName1(const uint row, const QString& new_name)
{
	replaceCompositeValue(row, 0, MESOSPLIT_COL_EXERCISENAME, 1, new_name);
	emit exerciseNameChanged();
}

QString DBMesoSplitModel::exerciseName2(const uint row) const
{
	const int idx(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).indexOf(comp_exercise_separator));
	return idx != -1 ? "2: "_L1 + m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).sliced(idx+1) :
						tr("2: Add exercise ...");
}

void DBMesoSplitModel::setExerciseName2(const uint row, const QString& new_name)
{
	replaceCompositeValue(row, 0, MESOSPLIT_COL_EXERCISENAME, 2, new_name);
	emit exerciseNameChanged();
}

inline uint DBMesoSplitModel::setsNumber(const int row) const
{
	return (row >= 0 && row < m_modeldata.count()) ? _setsNumber(static_cast<uint>(row)).toUInt() : 0;
}

void DBMesoSplitModel::setSetsNumber(const uint row, const uint new_setsnumber)
{
	m_modeldata[row][MESOSPLIT_COL_SETSNUMBER] = QString::number(new_setsnumber);
	setModified(row, MESOSPLIT_COL_SETSNUMBER);
}

QString DBMesoSplitModel::setsNotes(const int row) const
{
	return (row >= 0 && row < m_modeldata.count()) ? _setsNotes(static_cast<uint>(row)) : QString();
}

void DBMesoSplitModel::setSetsNotes(const uint row, const QString& new_setsnotes)
{
	m_modeldata[row][MESOSPLIT_COL_NOTES] = new_setsnotes;
	setModified(row, MESOSPLIT_COL_NOTES);
}

uint DBMesoSplitModel::setType(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsTypes(static_cast<uint>(row)), set_separator).toUInt() : 0;
}

void DBMesoSplitModel::setSetType(const uint row, const uint set_number, const uint new_type)
{
	appUtils()->setCompositeValue(set_number, QString::number(new_type), m_modeldata[row][MESOSPLIT_COL_SETTYPE], set_separator);
	setModified(row, MESOSPLIT_COL_SETTYPE);
	emit setTypeChanged();
}

QString DBMesoSplitModel::setSubsets(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsSubSets(row), set_separator) : STR_ZERO;
}

void DBMesoSplitModel::setSetsSubsets(const uint row, const uint set_number, const QString& new_setssubsets)
{
	appUtils()->setCompositeValue(set_number, new_setssubsets, m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER], set_separator);
}

QString DBMesoSplitModel::setReps(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsReps(row), set_separator) : QString();
}

void DBMesoSplitModel::setSetReps(const uint row, const uint set_number, const QString& new_setsreps)
{
	appUtils()->setCompositeValue(set_number, new_setsreps, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], set_separator);
	setModified(row, MESOSPLIT_COL_REPSNUMBER);
}

QString DBMesoSplitModel::setReps1(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 1);
}

void DBMesoSplitModel::setSetReps1(const uint row, const uint set_number, const QString& new_setsreps)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 1, new_setsreps);
}

QString DBMesoSplitModel::setReps2(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 2);
}

void DBMesoSplitModel::setSetReps2(const uint row, const uint set_number, const QString& new_setsreps)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 2, new_setsreps);
}

QString DBMesoSplitModel::setWeight(const int row, const uint set_number) const
{
	return appUtils()->getCompositeValue(set_number, _setsWeights(row), set_separator);
}

void DBMesoSplitModel::setSetWeight(const uint row, const uint set_number, const QString& new_setsweight)
{
	appUtils()->setCompositeValue(set_number, new_setsweight, m_modeldata[row][MESOSPLIT_COL_WEIGHT], set_separator);
	setModified(row, MESOSPLIT_COL_WEIGHT);
}

QString DBMesoSplitModel::setWeight1(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 1);
}

void DBMesoSplitModel::setSetWeight1(const uint row, const uint set_number, const QString& new_setsweight)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 1, new_setsweight);
}

QString DBMesoSplitModel::setWeight2(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 2);
}

void DBMesoSplitModel::setSetWeight2(const uint row, const uint set_number, const QString& new_setsweight)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 2, new_setsweight);
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
		name = model->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_s +
				model->selectedEntriesValue_fast(0, EXERCISES_COL_SUBNAME);
		reps = model->selectedEntriesValue(0, EXERCISES_COL_REPSNUMBER);
		weight = model->selectedEntriesValue(0, EXERCISES_COL_WEIGHT);
	}
	else
	{
		for (uint i(0); i < nSel; ++i)
		{
			name += model->selectedEntriesValue_fast(i, EXERCISES_COL_MAINNAME) + u" - "_s +
					model->selectedEntriesValue_fast(i, EXERCISES_COL_SUBNAME) + comp_exercise_separator;
			reps += model->selectedEntriesValue(i, EXERCISES_COL_REPSNUMBER) + comp_exercise_separator;
			weight += model->selectedEntriesValue(i, EXERCISES_COL_WEIGHT) + comp_exercise_separator;
		}
		name.chop(1);
		reps.chop(1);
		weight.chop(1);
	}

	setExerciseName(currentRow(), name);
	setSetReps(currentRow(), workingSet(), reps);
	setSetWeight(currentRow(), workingSet(), weight);
	setSetsNumber(currentRow(), model->selectedEntriesValue(0, EXERCISES_COL_SETSNUMBER).toUInt());
}

static void muscularGroupSimplified(QString& muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList& words(muscularGroup.split(' '));

	if (words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator& itr_end(words.end());
		muscularGroup.clear();

		do
		{
			if((*itr).length() < 3)
				continue;
			if (!muscularGroup.isEmpty())
				muscularGroup.append(' ');
			muscularGroup.append((*itr).toLower());
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
	QString muscularGroup1(appMesoModel()->muscularGroup(mesoIdx(), _splitLetter()));
	if (!muscularGroup1.isEmpty())
	{
		muscularGroupSimplified(muscularGroup1);
		QString muscularGroup2;
		const QString& mesoSplit(appMesoModel()->split(mesoIdx()));
		QString::const_iterator itr(mesoSplit.constBegin());
		const QString::const_iterator& itr_end(mesoSplit.constEnd());

		do {
			if ((*itr) == QChar('R'))
				continue;
			else if ((*itr) == splitLetter().at(0))
				continue;

			muscularGroup2 = appMesoModel()->muscularGroup(mesoIdx(), *itr);
			if (!muscularGroup2.isEmpty())
			{
				muscularGroupSimplified(muscularGroup2);
				if (appUtils()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
					return static_cast<QString>(*itr);
			}
		} while (++itr != itr_end);
	}
	return QString();
}

int DBMesoSplitModel::exportToFile(const QString& filename, const bool, const bool) const
{
	QFile* outFile{new QFile(filename)};
	const bool bOK(outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text));
	if (bOK)
	{
		const QString& strHeader(u"## "_s + exportName() + u"\n\n"_s);
		outFile->write(strHeader.toUtf8().constData());

		QString value;
		QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
		const QList<QStringList>::const_iterator& itr_end(m_modeldata.constEnd());

		while (itr != itr_end)
		{
			for (uint i(0); i < (*itr).count(); ++i)
			{
				if (i < mColumnNames.count())
				{
					if (!mColumnNames.at(i).isEmpty())
					{
						outFile->write(mColumnNames.at(i).toUtf8().constData());
						if (!isFieldFormatSpecial(i))
							value = (*itr).at(i);
						else
							value = formatFieldToExport(i, (*itr).at(i));
						value.replace(set_separator, fancy_record_separator2);
						value.replace(comp_exercise_separator, comp_exercise_fancy_separator);
						outFile->write(value.toUtf8().constData());
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
	return bOK ? APPWINDOW_MSG_EXPORT_OK : APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;
}

//Only for a complete meso split
int DBMesoSplitModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
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
						modeldata[col] = value;
					else
						modeldata[col] = formatFieldToImport(col, value);
					modeldata[col].replace(fancy_record_separator2, set_separator);
					modeldata[col].replace(comp_exercise_fancy_separator, QChar(comp_exercise_separator));
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
	return m_modeldata.count() > 1 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBMesoSplitModel::updateFromModel(const TPListModel* const model)
{
	clear();
	QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
	const QList<QStringList>::const_iterator& lst_itrend(model->m_modeldata.constEnd());
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
	return mb_Complete ? tr("Split: ") + m_splitLetter + u" - "_s + m_muscularGroup : QString();
}

QString DBMesoSplitModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		QString ret{fieldValue};
		ret.replace(u"0"_s, tr("Regular"));
		ret.replace(u"1"_s, tr("Pyramid"));
		ret.replace(u"2"_s, tr("Drop Set"));
		ret.replace(u"3"_s, tr("Cluster Set"));
		ret.replace(u"4"_s, tr("Giant Set"));
		ret.replace(u"5"_s, tr("Myo Reps"));
		ret.replace(u"6"_s, tr("Inverted Pyramid"));
		return ret;
	}
	return QString();
}

QString DBMesoSplitModel::formatFieldToImport(const uint field, const QString& fieldValue) const
{
	QString ret;
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		if (!fieldValue.isEmpty())
		{
			ret = fieldValue;
			ret.replace(tr("Regular"), u"0"_s);
			ret.replace(tr("Pyramid"), u"1"_s);
			ret.replace(tr("Drop Set"), u"2"_s);
			ret.replace(tr("Cluster Set"), u"3"_s);
			ret.replace(tr("Giant Set"), u"4"_s);
			ret.replace(tr("Myo Reps"), u"5"_s);
			ret.replace(tr("Inverted Pyramid"), u"6"_s);

		}
		else
			ret = u"0"_s;
	}
	return ret;
}

bool DBMesoSplitModel::importExtraInfo(const QString& extrainfo)
{
	int idx(extrainfo.indexOf(':'));
	if (idx != -1)
	{
		setSplitLetter(extrainfo.sliced(idx+2, 1));
		idx = extrainfo.indexOf('-', idx+1);
		if (idx != -1)
		{
			setMuscularGroup(extrainfo.sliced(idx+2, extrainfo.length() - idx - 3));
			mb_Complete = true;
			return true;
		}
	}
	return false;
}

QString DBMesoSplitModel::getFromCompositeValue(const uint row, const uint set_number, const uint field, const uint pos) const
{
	const QString& value(appUtils()->getCompositeValue(set_number, m_modeldata.at(row).at(field), comp_exercise_separator));
	const int idx(value.indexOf(comp_exercise_separator));
	return idx != -1 ? pos == 1 ? value.first(idx) : value.sliced(idx+1) : value;
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint set_number, const uint field, const uint pos, const QString& value)
{
	QString fieldValue{std::move(appUtils()->getCompositeValue(set_number, m_modeldata.at(row).at(field), comp_exercise_separator))};
	const int idx(fieldValue.indexOf(comp_exercise_separator));
	if (idx == -1)
	{
		if (pos == 1)
			fieldValue = value;
		else
			fieldValue += comp_exercise_separator + value;
	}
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
	appUtils()->setCompositeValue(set_number, fieldValue, m_modeldata[row][field], comp_exercise_separator);
	setModified(row, field);
}
