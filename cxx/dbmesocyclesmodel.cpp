#include "dbmesocyclesmodel.h"

#include "dbinterface.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tpglobals.h"
#include "tpsettings.h"

#include <QSettings>
#include <utility>

DBMesocyclesModel* DBMesocyclesModel::app_meso_model(nullptr);

DBMesocyclesModel::DBMesocyclesModel(QObject* parent)
	: TPListModel{parent},
	 m_mostRecentOwnMesoIdx(-1), m_bCanHaveTodaysWorkout(false)
{
	app_meso_model = this;

	setObjectName(DBMesocyclesObjectName);
	m_tableId = MESOCYCLES_TABLE_ID;
	m_fieldCount = MESOCYCLES_TOTAL_COLS;
	m_exportName = std::move(tr("Training Plan"));

	m_roleNames[mesoNameRole] = std::move("mesoName");
	m_roleNames[mesoStartDateRole] = std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole] = std::move("mesoEndDate");
	m_roleNames[mesoSplitRole] = std::move("mesoSplit");
	m_roleNames[mesoCoachRole] = std::move("mesoCoach");
	m_roleNames[mesoClientRole] = std::move("mesoClient");

	mColumnNames.reserve(MESOCYCLES_TOTAL_COLS);
	for(uint i(0); i < MESOCYCLES_TOTAL_COLS; ++i)
		mColumnNames.append(QString());
	fillColumnNames();

	m_splitModel = new DBMesoSplitModel{this, false, 10000};

	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE)
			updateColumnLabels();
	});
}

void DBMesocyclesModel::fillColumnNames()
{
	mColumnNames[MESOCYCLES_COL_NAME] = std::move(tr("Plan's name: "));
	mColumnNames[MESOCYCLES_COL_STARTDATE] = std::move(tr("Start date: "));
	mColumnNames[MESOCYCLES_COL_ENDDATE] = std::move(tr("End date: "));
	mColumnNames[MESOCYCLES_COL_NOTE] = std::move(tr("Plan's considerations: "));
	mColumnNames[MESOCYCLES_COL_WEEKS] = std::move(tr("Number of weeks: "));
	mColumnNames[MESOCYCLES_COL_SPLIT] = std::move(tr("Weekly Training Division: "));
	mColumnNames[MESOCYCLES_COL_TYPE] = std::move(tr("Type: "));
	mColumnNames[MESOCYCLES_COL_REALMESO] = std::move(tr("Mesocycle-style plan: "));
	updateColumnLabels();
}

DBMesocyclesModel::~DBMesocyclesModel()
{
	delete m_splitModel;
	for (uint i(0); i < m_calendarModelList.count(); ++i)
		delete m_calendarModelList.at(i);
	for (uint i(0); i < m_mesoManagerList.count(); ++i)
		delete m_mesoManagerList.at(i);
}

QMLMesoInterface* DBMesocyclesModel::mesoManager(const uint meso_idx)
{
	if (meso_idx >= m_mesoManagerList.count())
	{
		for (uint i(m_mesoManagerList.count()); i <= meso_idx ; ++i)
		{
			QMLMesoInterface* mesomanager{new QMLMesoInterface{this, appQmlEngine(), appMainWindow(), i}};
			m_mesoManagerList.append(mesomanager);
		}
	}
	return m_mesoManagerList.at(meso_idx);
}

void DBMesocyclesModel::getMesocyclePage(const uint meso_idx)
{
	setCurrentMesoIdx(meso_idx, true);
	mesoManager(meso_idx)->getMesocyclePage();
}

uint DBMesocyclesModel::createNewMesocycle(const bool bCreatePage)
{
	beginInsertRows(QModelIndex(), count(), count());
	const uint meso_idx = newMesocycle(std::move(QStringList() << STR_MINUS_ONE << std::move(tr("New Plan")) << QString() << QString() <<
		QString() << QString() << std::move("ABCDERR"_L1) << appUserModel()->currentCoachName(0) << appUserModel()->userName(0) <<
		QString() << QString() << STR_ONE));
	emit countChanged();
	endInsertRows();

	uchar newMesoRequiredFields(0);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_NAME);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_STARTDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_ENDDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_SPLIT);
	m_isNewMeso[meso_idx] = newMesoRequiredFields;

	QMLMesoInterface* mesomanager{new QMLMesoInterface{this, appQmlEngine(), appMainWindow(), meso_idx}};
	m_mesoManagerList.append(mesomanager);
	if (bCreatePage)
		mesomanager->getMesocyclePage();
	return meso_idx;
}

void DBMesocyclesModel::removeMesocycle(const uint meso_idx)
{
	if (_id(meso_idx) >= 0)
		appDBInterface()->removeMesocycle(meso_idx);

	delete m_calendarModelList.at(meso_idx);
	m_calendarModelList.remove(meso_idx);
	m_isNewMeso.remove(meso_idx);

	m_splitModel->removeRow(meso_idx);
	removeRow(meso_idx);

	if (meso_idx < m_mesoManagerList.count())
	{
		delete m_mesoManagerList.at(meso_idx);
		m_mesoManagerList.removeAt(meso_idx);
	}

	for (uint i(meso_idx); i < count(); ++i)
	{
		m_splitModel->setMesoIdx(i);
		m_calendarModelList.at(i)->setMesoIdx(i);
		emit mesoIdxChanged(i+1, i);
	}

	if (m_mostRecentOwnMesoIdx > meso_idx)
		--m_mostRecentOwnMesoIdx;
	else if (meso_idx == m_mostRecentOwnMesoIdx)
	{
		m_mostRecentOwnMesoIdx = -1;
		findNextOwnMeso();
	}

	emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
	changeCanHaveTodaysWorkout();
}

void DBMesocyclesModel::getExercisesPlannerPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getExercisesPlannerPage();
}

void DBMesocyclesModel::getMesoCalendarPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getCalendarPage();
}

void DBMesocyclesModel::todaysWorkout()
{
	mesoManager(mostRecentOwnMesoIdx())->getTrainingDayPage(QDate::currentDate());
}

void DBMesocyclesModel::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	mesoManager(meso_idx)->exportMeso(bShare, bCoachInfo);
}

const uint DBMesocyclesModel::newMesocycle(QStringList&& infolist)
{
	appendList_fast(std::move(infolist));
	m_splitModel->appendList_fast(std::move(QStringList(SIMPLE_MESOSPLIT_TOTAL_COLS)));

	const uint meso_idx(count()-1);
	m_splitModel->setMesoId(meso_idx, id(meso_idx));
	m_calendarModelList.append(new DBMesoCalendarModel{this, meso_idx});
	m_newMesoCalendarChanged.append(false);
	if (isOwnMeso(meso_idx))
	{
		m_mostRecentOwnMesoIdx = meso_idx;
		emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
		changeCanHaveTodaysWorkout();
	}
	m_usedSplits.append(QStringList());
	makeUsedSplits(meso_idx);
	m_isNewMeso.append(uchar(0));
	setCurrentMesoIdx(meso_idx, false);
	return meso_idx;
}

void DBMesocyclesModel::finishedLoadingFromDatabase()
{
	setReady(true);
	m_currentMesoIdx = appSettings()->lastViewedMesoIdx();
	if (m_currentMesoIdx == -1)
		setCurrentMesoIdx(count()-1, false);
}

void DBMesocyclesModel::changeCanHaveTodaysWorkout()
{
	if (m_bCanHaveTodaysWorkout != isDateWithinMeso(m_mostRecentOwnMesoIdx, QDate::currentDate()))
	{
		m_bCanHaveTodaysWorkout = !m_bCanHaveTodaysWorkout;
		emit canHaveTodaysWorkoutChanged();
	}
}

void DBMesocyclesModel::setModified(const uint meso_idx, const uint field)
{
	if (isNewMeso(meso_idx))
	{
		unSetBit(m_isNewMeso[meso_idx], field);
		if (!isNewMeso(meso_idx))
			emit isNewMesoChanged(meso_idx);
	}
	emit mesoChanged(meso_idx, field);
}

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar& splitLetter) const
{
	return m_splitModel->splitX(meso_idx, appUtils()->splitLetterToMesoSplitIndex(splitLetter));
}

QString DBMesocyclesModel::splitLetter(const uint meso_idx, const uint day_of_week) const
{
	return day_of_week <= 6 ? split(meso_idx).at(day_of_week) : QString();
}

void DBMesocyclesModel::setId(const uint meso_idx, const QString& new_id)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ID] = new_id;
	m_splitModel->m_modeldata[meso_idx][MESOSPLIT_COL_MESOID] = new_id;
}

void DBMesocyclesModel::setStartDate(const uint meso_idx, const QDate& new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_STARTDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESOCYCLES_COL_STARTDATE);
	emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoStartDateRole);
	changeCanHaveTodaysWorkout();
	if (!isNewMeso(meso_idx))
		emit mesoCalendarFieldsChanged(meso_idx);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setEndDate(const uint meso_idx, const QDate& new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ENDDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESOCYCLES_COL_ENDDATE);
	emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoEndDateRole);
	changeCanHaveTodaysWorkout();
	if (!isNewMeso(meso_idx))
		emit mesoCalendarFieldsChanged(meso_idx);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setSplit(const uint meso_idx, const QString& new_split)
{
	if (new_split != split(meso_idx))
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_SPLIT] = new_split;
		setModified(meso_idx, MESOCYCLES_COL_SPLIT);
		emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoSplitRole);
		if (!isNewMeso(meso_idx))
			emit mesoCalendarFieldsChanged(meso_idx);
		else
			m_newMesoCalendarChanged[meso_idx] = true;
		makeUsedSplits(meso_idx);
	}
}

bool DBMesocyclesModel::isOwnMeso(const int meso_idx) const
{
	Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isOwnMeso", "out of range meso_idx");
	return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == appUserModel()->userName(0);
}

void DBMesocyclesModel::setOwnMeso(const uint meso_idx, const bool bOwnMeso)
{
	if (isOwnMeso(meso_idx) != bOwnMeso)
	{
		setClient(meso_idx, bOwnMeso ? appUserModel()->userName(0) : appUserModel()->userName(0));
		const int cur_ownmeso(m_mostRecentOwnMesoIdx);
		findNextOwnMeso();
		if (cur_ownmeso != m_mostRecentOwnMesoIdx)
		{
			emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
			changeCanHaveTodaysWorkout();
		}
	}
}

void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar& splitLetter, const QString& newSplitValue, const bool bEmitSignal)
{
	const uint splitField(appUtils()->splitLetterToMesoSplitIndex(splitLetter));
	if (splitField < SIMPLE_MESOSPLIT_TOTAL_COLS)
	{
		m_splitModel->m_modeldata[meso_idx][splitField] = newSplitValue;
		setModified(meso_idx, MESOCYCLES_COL_MUSCULARGROUP);
		if (bEmitSignal)
			emit muscularGroupChanged(meso_idx, splitField, splitLetter);
	}
}

void DBMesocyclesModel::setCurrentMesoIdx(const int meso_idx, const bool bEmitSignal)
{
	if (meso_idx != m_currentMesoIdx)
	{
		m_currentMesoIdx = meso_idx;
		if (bEmitSignal)
		{
			appSettings()->setLastViewedMesoIdx(meso_idx);
			emit currentMesoIdxChanged();
		}
	}
}

void DBMesocyclesModel::makeUsedSplits(const uint meso_idx)
{
	QStringList* usedSplit = &(m_usedSplits[meso_idx]);
	usedSplit->clear();
	const QString& strSplit{split(meso_idx)};
	for (uint i(0); i < strSplit.length(); ++i)
	{
		const char chr(strSplit.at(i).toLatin1());
		if (chr != 'R' && !usedSplit->contains(chr))
			usedSplit->append(QString(chr));
	}
	emit usedSplitsChanged(meso_idx);
}

void DBMesocyclesModel::findNextOwnMeso()
{
	for (int i(count() -1); i >= 0; --i)
	{
		if (isOwnMeso(i))
		{
			m_mostRecentOwnMesoIdx = i;
			break;
		}
	}
}

QVariant DBMesocyclesModel::data(const QModelIndex& index, int role) const
{
	const int row(index.row());
	if(row >= 0 && row < m_modeldata.count())
	{
		switch(role)
		{
			case mesoNameRole:
				if (appUserModel()->userName(0) == client(row))
					return QVariant("<b>**  "_L1 + name(row) + "  **</b>"_L1);
				else
					return QVariant("<b>"_L1 + name(row) + "</b>"_L1);
			case mesoStartDateRole:
				return QVariant(mColumnNames.at(MESOCYCLES_COL_STARTDATE) + "<b>"_L1 +
					appUtils()->formatDate(startDate(row)) + "</b>"_L1);
			case mesoEndDateRole:
				return QVariant(mColumnNames.at(MESOCYCLES_COL_ENDDATE) + "<b>"_L1 +
					appUtils()->formatDate(endDate(row)) + "</b>"_L1);
			case mesoSplitRole:
				return QVariant(mColumnNames.at(MESOCYCLES_COL_SPLIT) + "<b>"_L1 + split(row) + "</b>"_L1);
			case mesoCoachRole:
				if (!coach(row).isEmpty())
					return QVariant(mColumnNames.at(MESOCYCLES_COL_COACH) + "<b>"_L1 + coach(row) + "</b>"_L1);
			case mesoClientRole:
				if (!client(row).isEmpty())
					return QVariant(mColumnNames.at(MESOCYCLES_COL_CLIENT) + "<b>"_L1 + client(row) + "</b>"_L1);
		}
	}
	return QString();
}

bool DBMesocyclesModel::isDateWithinMeso(const int meso_idx, const QDate& date) const
{
	if (meso_idx >= 0 && count() > 0)
	{
		if (date >= startDate(meso_idx))
			return date <= endDate(meso_idx);
	}
	return false;
}

int DBMesocyclesModel::getPreviousMesoId(const QString& clientName, const int current_mesoid) const
{
	int meso_idx(count()-1);
	for (; meso_idx >= 0; --meso_idx)
	{
		if (client(meso_idx) == clientName)
			if (_id(meso_idx) < current_mesoid)
				break;
	}
	return meso_idx >= 0 ? _id(meso_idx) : -1;
}

QDate DBMesocyclesModel::getMesoMinimumStartDate(const QString& clientName, const uint exclude_idx) const
{
	int meso_idx(count()-1);
	for (; meso_idx >= 0; --meso_idx)
	{
		if (meso_idx != exclude_idx)
		{
			if (client(meso_idx) == clientName)
				if (id(meso_idx) != STR_MINUS_ONE && isRealMeso(meso_idx))
					break;
		}
	}
	return meso_idx >= 0 ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, -6, 0);
}

QDate DBMesocyclesModel::getMesoMaximumEndDate(const QString& clientName, const uint exclude_idx) const
{
	int meso_idx(exclude_idx+1);
	for (; meso_idx < count(); ++meso_idx)
	{
		if (client(meso_idx) == clientName)
			if (id(meso_idx) != STR_MINUS_ONE && isRealMeso(meso_idx))
				break;
	}
	return meso_idx < count() ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, 6, 0);
}

//Called when importing from a text file
bool DBMesocyclesModel::isDifferent(const TPListModel* const model)
{
	if (model->count() > 0)
	{
		if (count() == 0)
			return true;
	}
	else
		return false; //model is not usefull

	bool bEqual(true);
	for (uint n(0); n < count(); ++n)
	{
		for (uint i(1); i < model->m_modeldata.at(0).count(); ++i)
		{
			if (m_modeldata.at(n).at(i) != model->m_modeldata.at(0).at(i))
			{
				bEqual = false;
				break;
			}
		}
		if (bEqual)
			return false;
		bEqual = true;
	}
	return true;
}

void DBMesocyclesModel::updateColumnLabels()
{
	QString strCoach;
	QString strClient;
	switch (appUserModel()->appUseMode(0))
	{
		case APP_USE_MODE_SINGLE_USER: break;
		case APP_USE_MODE_SINGLE_COACH:
			strClient = std::move(tr("Client: "));
		break;
		case APP_USE_MODE_SINGLE_USER_WITH_COACH:
			strCoach = std::move(tr("Coach/Trainer: "));
		break;
		case APP_USE_MODE_COACH_USER_WITH_COACH:
			strClient = std::move(tr("Client: "));
			strCoach = std::move(tr("Coach/Trainer: "));
		break;
	}
	mColumnNames[MESOCYCLES_COL_COACH] = std::move(strCoach);
	mColumnNames[MESOCYCLES_COL_CLIENT] = std::move(strClient);
}

int DBMesocyclesModel::exportToFile(const QString& filename, const bool, const bool) const
{
	int res(this->TPListModel::exportToFile(filename, true, false));
	if (res >= 0)
	{
		m_splitModel->setExportRow(m_exportRows.at(0));
		res = m_splitModel->TPListModel::exportToFile(filename, false, true);
		const_cast<DBMesocyclesModel*>(this)->m_exportRows.clear();
	}
	return res;
}

int DBMesocyclesModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return  APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList modeldata(MESOCYCLES_TOTAL_COLS);
	modeldata[0] = STR_MINUS_ONE;
	QStringList splitmodeldata(SIMPLE_MESOSPLIT_TOTAL_COLS);
	splitmodeldata[MESOSPLIT_COL_ID] = STR_MINUS_ONE;
	splitmodeldata[MESOSPLIT_COL_MESOID] = STR_MINUS_ONE;

	uint col(2);
	QString value;
	char buf[512];
	qint64 lineLength(0);
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (col < MESOCYCLES_TOTAL_COLS)
					{
						if (col != MESOCYCLES_COL_FILE)
						{
							value = buf;
							value.remove(0, value.indexOf(':') + 2);
							if (isFieldFormatSpecial(col))
								modeldata[col] = std::move(formatFieldToImport(col, value, buf));
							else
								modeldata[col] = std::move(value);
						}
					}
					else
					{
						if (col == SIMPLE_MESOSPLIT_TOTAL_COLS)
							break;
						value = buf;
						const int splitidx(appUtils()->splitLetterToMesoSplitIndex(value.at(value.indexOf(':')-1)));
						if (splitidx >= 2 && splitidx <= 7)
							splitmodeldata[splitidx] = std::move(value.remove(0, value.indexOf(':') + 2));
					}
					++col;
				}
			}
		}
		else
			break;
	}
	m_modeldata.append(std::move(modeldata));
	m_splitModel->m_modeldata.append(std::move(splitmodeldata));
	inFile->close();
	delete inFile;
	return col >= MESOCYCLES_COL_SPLIT ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBMesocyclesModel::updateFromModel(const uint meso_idx, TPListModel* model)
{
	setImportMode(true);
	for (uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_TOTAL_COLS; ++i)
		m_modeldata[meso_idx][i] = std::move(model->m_modeldata.at(0).at(i));
	const DBMesoSplitModel* const splitModel(static_cast<const DBMesoSplitModel* const>(const_cast<TPListModel*>(model)));
	for (uint i(MESOSPLIT_COL_ID); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
		m_splitModel->m_modeldata[meso_idx][i] = splitModel->m_modeldata.at(0).at(i);
	return true;
}

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case MESOCYCLES_COL_COACH:
		case MESOCYCLES_COL_CLIENT:
			return fieldValue;
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == STR_ONE ? tr("Yes") : tr("No");
	}
	return QString(); //never reached
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString& fieldValue, const QString& fieldName) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return QString::number(appUtils()->getDateFromStrDate(fieldValue).toJulianDay());
		case MESOCYCLES_COL_COACH:
			return fieldName.contains(tr("Coach")) ? fieldValue : QString();
		case MESOCYCLES_COL_CLIENT:
			return fieldName.contains(tr("Client")) ? fieldValue : QString();
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == tr("Yes") ? STR_ONE : STR_ZERO;
	}
	return QString(); //never reached
}
