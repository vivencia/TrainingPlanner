#include "dbmesosplitmodel.h"

#include "dbmesocyclesmodel.h"
#include "dbworkoutmodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "translationclass.h"

#include <utility>

DBMesoSplitModel::DBMesoSplitModel(QObject *parent, const bool bComplete, const uint meso_idx)
	: TPListModel(parent, static_cast<int>(meso_idx)), m_nextAddedExercisePos(2), mb_Complete(bComplete)
{
	setObjectName(DBMesoSplitObjectName);
	m_tableId = MESOSPLIT_TABLE_ID;
	m_exportName = std::move(tr("Exercises Program"));
	const uint totalCols(mb_Complete ? COMPLETE_MESOSPLIT_TOTAL_COLS : SIMPLE_MESOSPLIT_TOTAL_COLS);
	m_fieldCount = totalCols;
	m_modeldata.reserve(totalCols);
	mColumnNames.reserve(totalCols);
	for(uint i(0); i < totalCols; ++i)
		mColumnNames.append(QString{});
	fillColumnNames();

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		fillColumnNames();
		emit labelsChanged();
	});
}

void DBMesoSplitModel::convertFromTDayModel(const DBTrainingDayModel *const tDayModel)
{
	m_modeldata.clear();
	for (uint i(0); i < tDayModel->m_ExerciseData.count(); ++i)
	{
		QStringList exerciseInfo(COMPLETE_MESOSPLIT_TOTAL_COLS);
		exerciseInfo[MESOSPLIT_COL_EXERCISENAME] = tDayModel->exerciseName(i);
		exerciseInfo[MESOSPLIT_COL_SETSNUMBER] = std::move(QString::number(tDayModel->setsNumber(i)));
		exerciseInfo[MESOSPLIT_COL_NOTES] = std::move(tDayModel->setsNotes(i));
		exerciseInfo[MESOSPLIT_COL_SETTYPE] = std::move(tDayModel->setsTypes(i));
		exerciseInfo[MESOSPLIT_COL_SUBSETSNUMBER] = std::move(tDayModel->setsSubSets(i));
		exerciseInfo[MESOSPLIT_COL_REPSNUMBER] = std::move(tDayModel->setsReps(i));
		exerciseInfo[MESOSPLIT_COL_WEIGHT] = std::move(tDayModel->setsWeight(i));
		exerciseInfo[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;
		appendList(std::move(exerciseInfo));
		m_exerciseIsNew.append(0);
	}
	setReady(true);
}

void DBMesoSplitModel::addExerciseFromDatabase(QStringList &&exercise_info)
{
	appendList(std::move(exercise_info));
	m_exerciseIsNew.append(0);
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

bool DBMesoSplitModel::isFieldUserModified(const uint row, const uint field) const
{
	return !isBitSet(m_exerciseIsNew.at(row), field);
}

void DBMesoSplitModel::appendExercise()
{
	appendList(std::move(QStringList{} << std::move(tr("Choose exercise...")) << STR_ZERO << " "_L1 << STR_MINUS_ONE <<
				STR_ZERO << "10"_L1 << "20"_L1 << STR_ZERO));
	setCurrentRow(count() - 1);
	uchar newExerciseRequiredFields{0};
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_EXERCISENAME);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_SETSNUMBER);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_SETTYPE);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_REPSNUMBER);
	setBit(newExerciseRequiredFields, MESOSPLIT_COL_WEIGHT);
	m_exerciseIsNew.append(newExerciseRequiredFields);
}

void DBMesoSplitModel::addSet(const uint row)
{
	uint nsets{setsNumber(row)};
	if (nsets < 10)
	{
		const uint referenceSet{nsets > 0 ? nsets-1 : 0};
		QString exercise1data{std::move(appUtils()->getCompositeValue(0, _setsReps(row), comp_exercise_separator))};
		appUtils()->setCompositeValue(nsets, setReps1(row, referenceSet), exercise1data, set_separator);

		QString exercise2data{std::move(appUtils()->getCompositeValue(1, _setsReps(row), comp_exercise_separator))};
		if (!exercise2data.isEmpty())
		{
			appUtils()->setCompositeValue(nsets, setReps2(row, referenceSet), exercise2data, set_separator);
			appUtils()->setCompositeValue(0, exercise1data, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], comp_exercise_separator);
			appUtils()->setCompositeValue(1, exercise2data, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], comp_exercise_separator);
		}
		else
			m_modeldata[row][MESOSPLIT_COL_REPSNUMBER] = std::move(exercise1data);

		exercise1data = std::move(appUtils()->getCompositeValue(0, _setsWeights(row), comp_exercise_separator));
		appUtils()->setCompositeValue(nsets, setWeight1(row, referenceSet), exercise1data, set_separator);

		exercise2data = std::move(appUtils()->getCompositeValue(1, _setsWeights(row), comp_exercise_separator));
		if (!exercise2data.isEmpty())
		{
			appUtils()->setCompositeValue(nsets, setWeight2(row, referenceSet), exercise2data, set_separator);
			appUtils()->setCompositeValue(0, exercise1data, m_modeldata[row][MESOSPLIT_COL_WEIGHT], comp_exercise_separator);
			appUtils()->setCompositeValue(1, exercise2data, m_modeldata[row][MESOSPLIT_COL_WEIGHT], comp_exercise_separator);
		}
		else
			m_modeldata[row][MESOSPLIT_COL_WEIGHT] = std::move(exercise1data);

		exercise1data = std::move(appUtils()->getCompositeValue(0, _setsTypes(row), comp_exercise_separator));
		appUtils()->setCompositeValue(nsets, QString::number(setType(row, referenceSet)), exercise1data, set_separator);
		m_modeldata[row][MESOSPLIT_COL_SETTYPE] = std::move(exercise1data);

		exercise1data = std::move(appUtils()->getCompositeValue(0, _setsSubSets(row), comp_exercise_separator));
		appUtils()->setCompositeValue(nsets, setSubsets(row, referenceSet), exercise1data, set_separator);
		m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER] = std::move(exercise1data);

		setWorkingSet(row, nsets);
		++nsets;
		setSetsNumber(row, nsets);
	}
}

void DBMesoSplitModel::delSet(const uint row)
{
	uint nsets{setsNumber(row)};
	if (nsets > 1)
	{
		QString exercise1data{std::move(appUtils()->getCompositeValue(0, _setsReps(row), comp_exercise_separator))};
		appUtils()->removeFieldFromCompositeValue(nsets, exercise1data, set_separator);
		QString exercise2data{std::move(appUtils()->getCompositeValue(1, _setsReps(row), comp_exercise_separator))};
		appUtils()->removeFieldFromCompositeValue(nsets, exercise2data, set_separator);
		appUtils()->setCompositeValue(0, exercise1data, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], comp_exercise_separator);
		appUtils()->setCompositeValue(1, exercise2data, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], comp_exercise_separator);

		exercise1data = std::move(appUtils()->getCompositeValue(0, _setsWeights(row), comp_exercise_separator));
		appUtils()->removeFieldFromCompositeValue(nsets, exercise1data, set_separator);
		exercise2data = std::move(appUtils()->getCompositeValue(1, _setsWeights(row), comp_exercise_separator));
		appUtils()->removeFieldFromCompositeValue(nsets, exercise2data, set_separator);
		appUtils()->setCompositeValue(0, exercise1data, m_modeldata[row][MESOSPLIT_COL_WEIGHT], comp_exercise_separator);
		appUtils()->setCompositeValue(1, exercise2data, m_modeldata[row][MESOSPLIT_COL_WEIGHT], comp_exercise_separator);

		--nsets;
		if (workingSet(row) == nsets)
			setWorkingSet(row, nsets-1);
		setSetsNumber(row, nsets);
	}
}

QString DBMesoSplitModel::exerciseName(const int row, const bool b_raw) const
{
	if (row >= 0 && row < m_modeldata.count())
	{
		if (!b_raw)
		{
			QString name{_exerciseName(static_cast<uint>(row))};
			if (name.endsWith(comp_exercise_separator))
				name.chop(1);
			return name.replace(comp_exercise_separator, comp_exercise_fancy_separator);
		}
		else
			return _exerciseName(row);
	}
	return QString{};
}

void DBMesoSplitModel::setExerciseName(const uint row, const QString &new_name)
{
	QString name{new_name};
	m_modeldata[row][MESOSPLIT_COL_EXERCISENAME] = std::move(name.replace(comp_exercise_fancy_separator, QString(comp_exercise_separator)));
	emit exerciseNameChanged(row);
	setModified(row, MESOSPLIT_COL_EXERCISENAME);
}

QString DBMesoSplitModel::exerciseName1(const uint row) const
{
	const int idx(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).indexOf(comp_exercise_separator));
	return idx != -1 ? "2: "_L1 + std::move(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).first(idx)) :
						m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).isEmpty() ?
						std::move(tr("1: Add exercise ...")) : m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME);
}

void DBMesoSplitModel::setExerciseName1(const uint row, const QString &new_name)
{
	replaceCompositeValue(row, 0, MESOSPLIT_COL_EXERCISENAME, 0, new_name);
	emit exerciseNameChanged(row);
}

QString DBMesoSplitModel::exerciseName2(const uint row) const
{
	const int idx(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).indexOf(comp_exercise_separator));
	return idx != -1 ? "2: "_L1 + std::move(m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).sliced(idx+1)) :
						std::move(tr("2: Add exercise ..."));
}

void DBMesoSplitModel::setExerciseName2(const uint row, const QString &new_name)
{
	replaceCompositeValue(row, 0, MESOSPLIT_COL_EXERCISENAME, 1, new_name);
	emit exerciseNameChanged(row);
}

bool DBMesoSplitModel::exerciseIsComposite(const uint row) const
{
	return m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME).contains(comp_exercise_separator);
}

uint DBMesoSplitModel::setsNumber(const int row) const
{
	return (row >= 0 && row < m_modeldata.count()) ? _setsNumber(static_cast<uint>(row)).toUInt() : 0;
}

void DBMesoSplitModel::setSetsNumber(const uint row, const uint new_setsnumber)
{
	m_modeldata[row][MESOSPLIT_COL_SETSNUMBER] = QString::number(new_setsnumber);
	emit setsNumberChanged(row);
	setModified(row, MESOSPLIT_COL_SETSNUMBER);
}

QString DBMesoSplitModel::setsNotes(const int row) const
{
	return (row >= 0 && row < m_modeldata.count()) ? _setsNotes(static_cast<uint>(row)) : QString{};
}

void DBMesoSplitModel::setSetsNotes(const uint row, const QString &new_setsnotes)
{
	m_modeldata[row][MESOSPLIT_COL_NOTES] = new_setsnotes;
	setModified(row, MESOSPLIT_COL_NOTES);
}

int DBMesoSplitModel::setType(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsTypes(static_cast<uint>(row)), set_separator).toInt() : -1;
}

void DBMesoSplitModel::setSetType(const uint row, const uint set_number, const uint new_type, const bool b_emit_modified)
{
	appUtils()->setCompositeValue(set_number, QString::number(new_type), m_modeldata[row][MESOSPLIT_COL_SETTYPE], set_separator);
	setModified(row, MESOSPLIT_COL_SETTYPE);
	if (b_emit_modified)
		emit setTypeChanged(row);
	if (exerciseName(row).isEmpty())
		setExerciseName(row, new_type != SET_TYPE_GIANT ? tr("Choose exercise...") : tr("Choose exercises..."));
}

QString DBMesoSplitModel::setSubsets(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsSubSets(row), set_separator) : STR_ZERO;
}

void DBMesoSplitModel::setSetsSubsets(const uint row, const uint set_number, const QString &new_setssubsets)
{
	appUtils()->setCompositeValue(set_number, new_setssubsets, m_modeldata[row][MESOSPLIT_COL_SUBSETSNUMBER], set_separator);
}

QString DBMesoSplitModel::setReps(const int row, const uint set_number) const
{
	return (row >= 0 && row < m_modeldata.count()) ? appUtils()->getCompositeValue(set_number, _setsReps(row), set_separator) : QString{};
}

void DBMesoSplitModel::setSetReps(const uint row, const uint set_number, const QString &new_setsreps)
{
	appUtils()->setCompositeValue(set_number, new_setsreps, m_modeldata[row][MESOSPLIT_COL_REPSNUMBER], set_separator);
}

QString DBMesoSplitModel::setReps1(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 0);
}

void DBMesoSplitModel::setSetReps1(const uint row, const uint set_number, const QString &new_setsreps)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 0, new_setsreps);
}

QString DBMesoSplitModel::setReps2(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 1);
}

void DBMesoSplitModel::setSetReps2(const uint row, const uint set_number, const QString &new_setsreps)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_REPSNUMBER, 1, new_setsreps);
}

QString DBMesoSplitModel::setWeight(const int row, const uint set_number) const
{
	return appUtils()->getCompositeValue(set_number, _setsWeights(row), set_separator);
}

void DBMesoSplitModel::setSetWeight(const uint row, const uint set_number, const QString &new_setsweight)
{
	appUtils()->setCompositeValue(set_number, new_setsweight, m_modeldata[row][MESOSPLIT_COL_WEIGHT], set_separator);
}

QString DBMesoSplitModel::setWeight1(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 0);
}

void DBMesoSplitModel::setSetWeight1(const uint row, const uint set_number, const QString &new_setsweight)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 0, new_setsweight);
}

QString DBMesoSplitModel::setWeight2(const int row, const uint set_number) const
{
	return getFromCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 1);
}

void DBMesoSplitModel::setSetWeight2(const uint row, const uint set_number, const QString &new_setsweight)
{
	replaceCompositeValue(row, set_number, MESOSPLIT_COL_WEIGHT, 1, new_setsweight);
}

void DBMesoSplitModel::setWorkingSet(const uint row, const uint new_workingset, const bool emitSignal)
{
	m_modeldata[row][MESOSPLIT_COL_WORKINGSET] = QString::number(new_workingset);
	if (emitSignal)
		emit workingSetChanged(row);
}

static void muscularGroupSimplified(QString &muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList &words(muscularGroup.split(' '));

	if (words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator &itr_end(words.end());
		muscularGroup.clear();

		do
		{
			if((*itr).length() < 3)
				continue;
			if (!muscularGroup.isEmpty())
				muscularGroup.append(' ');
			muscularGroup.append((*itr).toLower());
			if (muscularGroup.endsWith('s', Qt::CaseInsensitive))
				muscularGroup.chop(1);
			muscularGroup.remove('.');
			muscularGroup.remove('(');
			muscularGroup.remove(')');
		} while (++itr != itr_end);
	}
}

QString DBMesoSplitModel::findSwappableModel() const
{
	QString muscularGroup1{std::move(appMesoModel()->muscularGroup(mesoIdx(), _splitLetter()))};
	if (!muscularGroup1.isEmpty())
	{
		muscularGroupSimplified(muscularGroup1);
		QString muscularGroup2;
		const QString &mesoSplit{appMesoModel()->split(mesoIdx())};
		QString::const_iterator itr{mesoSplit.constBegin()};
		const QString::const_iterator &itr_end{mesoSplit.constEnd()};

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
	return QString{};
}

int DBMesoSplitModel::exportToFile(const QString &filename, const bool, const bool, const bool) const
{
	QFile *outFile{new QFile{filename}};
	const bool bOK(outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text));
	if (bOK)
	{
		const QString &strHeader{"## "_L1 + exportName() + " - 0x000"_L1 + QString::number(tableID()) + "\n\n"_L1};
		outFile->write(strHeader.toUtf8().constData());
		outFile->write(exportExtraInfo().toUtf8().constData());
		outFile->write("\n\n", 2);

		QString value;
		QList<QStringList>::const_iterator itr{m_modeldata.constBegin()};
		const QList<QStringList>::const_iterator &itr_end{m_modeldata.constEnd()};

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
						const int sep_idx(value.indexOf(comp_exercise_separator));
						if (sep_idx >= 0)
						{
							value.chop(1);
							value.replace(comp_exercise_separator, comp_exercise_fancy_separator);
						}
						value.replace(set_separator, fancy_record_separator2);
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

bool DBMesoSplitModel::checkIfFileContentMatchesThisSplit(QFile *inFile)
{
	char buf[256];
	int lineLength{0};
	QString inData;
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (lineLength > 10)
		{
			inData = buf;
			int idx(inData.indexOf(tr("Split: ")));
			if (idx != -1)
			{
				idx += 7;
				const QChar &splitletter{inData.sliced(idx+2, 1).at(0)};
				if (splitletter == _splitLetter())
				{
					idx = inData.indexOf('-', idx+1);
					if (idx != -1)
					{
						setMuscularGroup(inData.sliced(idx+2, inData.length() - idx - 3));
						return true;
					}
				}
				break;
			}
		}
	}
	return false;
}

//Only for a complete meso split
int DBMesoSplitModel::importFromFile(const QString &filename)
{
	QFile *inFile{new QFile{filename}};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	char buf[512];
	qint64 lineLength{0};
	uint col{MESOSPLIT_COL_EXERCISENAME};
	QString value;
	bool split_match{false};
	QStringList modeldata{COMPLETE_MESOSPLIT_TOTAL_COLS};
	modeldata[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (!split_match)
					{
						split_match = checkIfFileContentMatchesThisSplit(inFile);
						continue;
					}
					else
						break;
				}
				else
				{
					if (!split_match)
						continue;
					value = buf;
					value.remove(0, value.indexOf(':') + 2);
					value.replace(fancy_record_separator2, set_separator);
					if (value.contains(comp_exercise_fancy_separator))
						value.append(comp_exercise_separator);
					value.replace(comp_exercise_fancy_separator, QChar(comp_exercise_separator));
					if (!isFieldFormatSpecial(col))
						modeldata[col] = std::move(value.simplified());
					else
						modeldata[col] = std::move(formatFieldToImport(col, value));
					col++;
					if (col == MESOSPLIT_COL_WORKINGSET)
					{
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

bool DBMesoSplitModel::updateFromModel(TPListModel *model)
{
	clear();
	QList<QStringList>::iterator lst_itr(model->m_modeldata.begin());
	const QList<QStringList>::const_iterator &lst_itrend(model->m_modeldata.constEnd());
	do {
		appendList(std::move((*lst_itr)));
	} while (++lst_itr != lst_itrend);
	setSplitLetter(static_cast<DBMesoSplitModel *const>(model)->splitLetter());
	setMuscularGroup(static_cast<DBMesoSplitModel *const>(model)->muscularGroup());
	setMesoIdx(static_cast<DBMesoSplitModel *const>(model)->mesoIdx());
	appMesoModel()->setMuscularGroup(m_mesoIdx, _splitLetter(), muscularGroup(), false);
	emit modelChanged();
	if (model->importMode())
		delete model;
	return true;
}

QString DBMesoSplitModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		QString ret{fieldValue};
		ret.replace("0"_L1, tr("Regular"));
		ret.replace("1"_L1, tr("Pyramid"));
		ret.replace("2"_L1, tr("Drop Set"));
		ret.replace("3"_L1, tr("Cluster Set"));
		ret.replace("4"_L1, tr("Giant Set"));
		ret.replace("5"_L1, tr("Myo Reps"));
		ret.replace("6"_L1, tr("Inverted Pyramid"));
		return ret;
	}
	return QString{};
}

QString DBMesoSplitModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	QString ret;
	if (field == MESOSPLIT_COL_SETTYPE)
	{
		if (!fieldValue.isEmpty())
		{
			ret = fieldValue.simplified();
			ret.replace(tr("Regular"), "0"_L1);
			ret.replace(tr("Pyramid"), "1"_L1);
			ret.replace(tr("Drop Set"), "2"_L1);
			ret.replace(tr("Cluster Set"), "3"_L1);
			ret.replace(tr("Giant Set"), "4"_L1);
			ret.replace(tr("Myo Reps"), "5"_L1);
			ret.replace(tr("Inverted Pyramid"), "6"_L1);

		}
		else
			ret = "0"_L1;
	}
	return ret;
}

const QString DBMesoSplitModel::exportExtraInfo() const
{
	return mb_Complete ? std::move(tr("Split: ")) + m_splitLetter + " - "_L1 + std::move(m_muscularGroup) : QString{};
}

QString DBMesoSplitModel::getFromCompositeValue(const uint row, const uint set_number, const uint field, const uint pos) const
{
	const QString &value(appUtils()->getCompositeValue(pos, m_modeldata.at(row).at(field), comp_exercise_separator));
	return appUtils()->getCompositeValue(set_number, value, set_separator);
}

void DBMesoSplitModel::replaceCompositeValue(const uint row, const uint set_number, const uint field, const uint pos, const QString &value)
{
	QString fieldValue{std::move(appUtils()->getCompositeValue(pos, m_modeldata.at(row).at(field), comp_exercise_separator))};
	appUtils()->setCompositeValue(set_number, value, fieldValue, set_separator);
	appUtils()->setCompositeValue(pos, fieldValue, m_modeldata[row][field], comp_exercise_separator);
	setModified(row, field);
}

void DBMesoSplitModel::fillColumnNames()
{
	if (mb_Complete)
	{
		mColumnNames[MESOSPLIT_COL_EXERCISENAME] = std::move(tr("Exercise: "));
		mColumnNames[MESOSPLIT_COL_SETSNUMBER] = std::move(tr("Sets number: "));
		mColumnNames[MESOSPLIT_COL_NOTES] = std::move(tr("Set instructions: "));
		mColumnNames[MESOSPLIT_COL_SETTYPE] = std::move(tr("Sets type: "));
		mColumnNames[MESOSPLIT_COL_SUBSETSNUMBER] = std::move(tr("Number of subsets: "));
		mColumnNames[MESOSPLIT_COL_REPSNUMBER] = std::move(tr("Reps: "));
		mColumnNames[MESOSPLIT_COL_WEIGHT] = std::move(tr("Weight: "));
	}
	else
	{
		mColumnNames[MESOSPLIT_A] = std::move(tr("Split A: "));
		mColumnNames[MESOSPLIT_B] = std::move(tr("Split B: "));
		mColumnNames[MESOSPLIT_C] = std::move(tr("Split C: "));
		mColumnNames[MESOSPLIT_D] = std::move(tr("Split D: "));
		mColumnNames[MESOSPLIT_E] = std::move(tr("Split E: "));
		mColumnNames[MESOSPLIT_F] = std::move(tr("Split F: "));
	}
}
