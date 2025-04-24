#include "dbmesocyclesmodel.h"

#include "dbinterface.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "dbusermodel.h"
#include "homepagemesomodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tpglobals.h"
#include "tpsettings.h"
#include "translationclass.h"

#include <QSettings>
#include <utility>

#define NEW_MESO_REQUIRED_FIELDS 4

DBMesocyclesModel *DBMesocyclesModel::app_meso_model(nullptr);

DBMesocyclesModel::DBMesocyclesModel(QObject *parent, const bool bMainAppModel)
	: TPListModel{parent}, m_mostRecentOwnMesoIdx{-1}, m_lowestTempMesoId{-1}, m_bCanHaveTodaysWorkout{false}
{
	app_meso_model = this;
	setObjectName(DBMesocyclesObjectName);
	m_tableId = MESOCYCLES_TABLE_ID;
	m_fieldCount = MESOCYCLES_TOTAL_COLS;
	setCurrentMesoIdx(appSettings()->lastViewedMesoIdx(), false);
	m_exportName = std::move(tr("Training Program"));

	mColumnNames.reserve(MESOCYCLES_TOTAL_COLS);
	for(uint i{0}; i < MESOCYCLES_TOTAL_COLS; ++i)
		mColumnNames.append(std::move(QString{}));
	fillColumnNames();

	m_splitModel = new DBMesoSplitModel{this, false, 10000};
	m_calendarModel = new DBMesoCalendarModel{this};
	m_ownMesos = new homePageMesoModel{this};
	m_clientMesos = new homePageMesoModel{this};

	connect(m_calendarModel, &DBMesoCalendarModel::calendarChanged, this, [this] (const uint meso_idx, const int calendar_day, const uint field) {
		switch (field)
		{
			case MESOCALENDAR_TOTAL_COLS:
			case MESOCALENDAR_COL_TRAINING_COMPLETED:
			case MESOCALENDAR_COL_SPLITLETTER:
				appDBInterface()->saveMesoCalendar(meso_idx);
			break;
			case MESOCALENDAR_RENEW_DATABASE:
				appDBInterface()->remakeMesoCalendar(meso_idx);
			break;
		}
	});
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		fillColumnNames();
	});
}

void DBMesocyclesModel::fillColumnNames()
{
	mColumnNames[MESOCYCLES_COL_NAME] = std::move(tr("Program's name: "));
	mColumnNames[MESOCYCLES_COL_STARTDATE] = std::move(tr("Start date: "));
	mColumnNames[MESOCYCLES_COL_ENDDATE] = std::move(tr("End date: "));
	mColumnNames[MESOCYCLES_COL_NOTE] = std::move(tr("Program's considerations: "));
	mColumnNames[MESOCYCLES_COL_WEEKS] = std::move(tr("Number of weeks: "));
	mColumnNames[MESOCYCLES_COL_COACH] = std::move(tr("Coach/Trainer: "));
	mColumnNames[MESOCYCLES_COL_CLIENT] = std::move(tr("Client: "));
	mColumnNames[MESOCYCLES_COL_SPLIT] = std::move(tr("Weekly Training Division: "));
	mColumnNames[MESOCYCLES_COL_TYPE] = std::move(tr("Type: "));
	mColumnNames[MESOCYCLES_COL_REALMESO] = std::move(tr("Mesocycle-style program: "));
}

DBMesocyclesModel::~DBMesocyclesModel()
{
	delete m_ownMesos;
	delete m_clientMesos;
	delete m_splitModel;
	delete m_calendarModel;
	for (auto mesoManagerList : std::as_const(m_mesoManagerList))
		delete mesoManagerList;
}

QMLMesoInterface *DBMesocyclesModel::mesoManager(const uint meso_idx)
{
	if (meso_idx >= m_mesoManagerList.count())
	{
		for (qsizetype i{m_mesoManagerList.count()}; i <= meso_idx ; ++i)
		{
			QMLMesoInterface *mesomanager{new QMLMesoInterface{this, static_cast<uint>(i)}};
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

uint DBMesocyclesModel::startNewMesocycle(const bool bCreatePage, const std::optional<bool> bOwnMeso)
{
	beginInsertRows(QModelIndex{}, count(), count());
	const uint meso_idx{newMesocycle(std::move(QStringList{} << QString::number(m_lowestTempMesoId--) << QString{} << QString{} << QString{} <<
		QString{} << QString{} << std::move("RRRRRRR"_L1) << appUserModel()->userId(0) <<
			(bOwnMeso.has_value() ? (bOwnMeso.value() ? appUserModel()->userId(0) : appUserModel()->defaultClient()) : QString{}) <<
						QString{} << QString{} << STR_ONE))};
	emit countChanged();
	endInsertRows();

	short newMesoRequiredFields{0};
	setBit(newMesoRequiredFields, MESOCYCLES_COL_NAME);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_STARTDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_ENDDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_SPLIT);
	m_isNewMeso[meso_idx] = newMesoRequiredFields;

	QMLMesoInterface *mesomanager{new QMLMesoInterface{this, meso_idx}};
	m_mesoManagerList.append(mesomanager);
	if (bCreatePage)
		getMesocyclePage(meso_idx);
	return meso_idx;
}

void DBMesocyclesModel::removeMesocycle(const uint meso_idx)
{
	if (_id(meso_idx) >= 0)
		appDBInterface()->removeMesocycle(meso_idx);
	m_calendarModel->removeCalendarForMeso(meso_idx);
	m_isNewMeso.remove(meso_idx);
	m_newMesoFieldCounter.remove(meso_idx);
	m_splitModel->removeRow(meso_idx);
	m_newMesoCalendarChanged.remove(meso_idx);
	m_canExport.remove(meso_idx);
	removeMesoFile(meso_idx);

	if (isOwnMeso(meso_idx))
		m_ownMesos->removeData(m_modeldata.at(meso_idx));
	else
		m_clientMesos->removeData(m_modeldata.at(meso_idx));

	removeRow(meso_idx);

	if (meso_idx < m_mesoManagerList.count())
	{
		delete m_mesoManagerList.at(meso_idx);
		m_mesoManagerList.removeAt(meso_idx);
	}

	const int most_recent_ownMesoIdx{m_mostRecentOwnMesoIdx};
	if (m_mostRecentOwnMesoIdx > meso_idx)
		--m_mostRecentOwnMesoIdx;
	else if (meso_idx == m_mostRecentOwnMesoIdx)
	{
		m_mostRecentOwnMesoIdx = -1;
		findNextOwnMeso();
	}
	if (m_mostRecentOwnMesoIdx != most_recent_ownMesoIdx)
	{
		emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
		changeCanHaveTodaysWorkout(m_mostRecentOwnMesoIdx);
	}
}

void DBMesocyclesModel::getExercisesPlannerPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getExercisesPlannerPage();
}

void DBMesocyclesModel::getMesoCalendarPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getCalendarPage();
}

void DBMesocyclesModel::openSpecificWorkout(const uint meso_idx, const QDate &date)
{
	mesoManager(meso_idx)->getTrainingDayPage(date);
}

void DBMesocyclesModel::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	mesoManager(meso_idx)->exportMeso(bShare, bCoachInfo);
}

const uint DBMesocyclesModel::newMesocycle(QStringList &&infolist)
{
	appendList_fast(std::move(infolist));
	const uint meso_idx{count()-1};

	m_splitModel->appendList_fast(std::move(QStringList{SIMPLE_MESOSPLIT_TOTAL_COLS}));

	m_newMesoCalendarChanged.append(false);
	m_canExport.append(false);
	//A temporary meso will not have enough info at this time to have it determined if it's a own meos or not
	if (_id(meso_idx) >= 0)
		setOwnMeso(meso_idx);
	else
		m_calendarModel->addNewCalendarForMeso(meso_idx);

	m_usedSplits.append(QStringList{});
	makeUsedSplits(meso_idx);
	m_isNewMeso.append(0);
	m_newMesoFieldCounter.append(NEW_MESO_REQUIRED_FIELDS);
	return meso_idx;
}

void DBMesocyclesModel::changeCanHaveTodaysWorkout(const uint meso_idx)
{
	if (isOwnMeso(meso_idx))
	{
		if (m_bCanHaveTodaysWorkout != isDateWithinMeso(meso_idx, QDate::currentDate()))
		{
			m_bCanHaveTodaysWorkout = !m_bCanHaveTodaysWorkout;
			emit canHaveTodaysWorkoutChanged();
		}
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

int DBMesocyclesModel::idxFromId(const uint meso_id) const
{
	const QString &mesoIdStr{QString::number(meso_id)};
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(MESOCYCLES_COL_ID) == mesoIdStr)
			return i;
	}
	return -1;
}

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar &splitLetter) const
{
	return splitLetter != 'R' ? m_splitModel->splitX(meso_idx, appUtils()->splitLetterToMesoSplitIndex(splitLetter)) : QString{};
}

QString DBMesocyclesModel::splitLetter(const uint meso_idx, const uint day_of_week) const
{
	return day_of_week <= 6 ? split(meso_idx).at(day_of_week) : QString{};
}

void DBMesocyclesModel::setId(const uint meso_idx, const QString &new_id)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ID] = new_id;
	m_splitModel->m_modeldata[meso_idx][MESOSPLIT_COL_MESOID] = new_id;
}

void DBMesocyclesModel::setName(const uint meso_idx, const QString &new_name)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_NAME] = new_name;
	if (isBitSet(m_isNewMeso.at(meso_idx), MESOCYCLES_COL_NAME))
	{
		m_newMesoFieldCounter[meso_idx]--;
		emit newMesoFieldCounterChanged(meso_idx, MESOCYCLES_COL_NAME);
	}
	setModified(meso_idx, MESOCYCLES_COL_NAME);
	m_curMesos->emitDataChanged(meso_idx, mesoNameRole);
}

void DBMesocyclesModel::setStartDate(const uint meso_idx, const QDate &new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_STARTDATE] = std::move(QString::number(new_date.toJulianDay()));
	if (isBitSet(m_isNewMeso.at(meso_idx), MESOCYCLES_COL_STARTDATE))
	{
		m_newMesoFieldCounter[meso_idx]--;
		emit newMesoFieldCounterChanged(meso_idx, MESOCYCLES_COL_STARTDATE);
	}
	setModified(meso_idx, MESOCYCLES_COL_STARTDATE);
	m_curMesos->emitDataChanged(meso_idx, mesoStartDateRole);
	changeCanHaveTodaysWorkout(meso_idx);
	if (!isNewMeso(meso_idx) && m_newMesoFieldCounter.at(meso_idx) == NEW_MESO_REQUIRED_FIELDS)
		emit mesoCalendarFieldsChanged(meso_idx, MESOCYCLES_COL_STARTDATE);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setEndDate(const uint meso_idx, const QDate &new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ENDDATE] = std::move(QString::number(new_date.toJulianDay()));
	if (isBitSet(m_isNewMeso.at(meso_idx), MESOCYCLES_COL_ENDDATE))
	{
		m_newMesoFieldCounter[meso_idx]--;
		emit newMesoFieldCounterChanged(meso_idx, MESOCYCLES_COL_ENDDATE);
	}
	setModified(meso_idx, MESOCYCLES_COL_ENDDATE);
	m_curMesos->emitDataChanged(meso_idx, mesoEndDateRole);
	changeCanHaveTodaysWorkout(meso_idx);
	if (!isNewMeso(meso_idx) && m_newMesoFieldCounter.at(meso_idx) == NEW_MESO_REQUIRED_FIELDS)
		emit mesoCalendarFieldsChanged(meso_idx, MESOCYCLES_COL_ENDDATE);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setSplit(const uint meso_idx, const QString &new_split)
{
	if (new_split != split(meso_idx))
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_SPLIT] = new_split;
		if (isBitSet(m_isNewMeso.at(meso_idx), MESOCYCLES_COL_SPLIT))
		{
			m_newMesoFieldCounter[meso_idx]--;
			emit newMesoFieldCounterChanged(meso_idx, MESOCYCLES_COL_SPLIT);
		}
		setModified(meso_idx, MESOCYCLES_COL_SPLIT);
		m_curMesos->emitDataChanged(meso_idx, mesoSplitRole);
		if (!isNewMeso(meso_idx) && m_newMesoFieldCounter.at(meso_idx) == NEW_MESO_REQUIRED_FIELDS)
			emit mesoCalendarFieldsChanged(meso_idx, MESOCYCLES_COL_SPLIT);
		else
			m_newMesoCalendarChanged[meso_idx] = true;
		makeUsedSplits(meso_idx);
	}
}

std::optional<bool> DBMesocyclesModel::isOwnMeso(const int meso_idx) const
{
	Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isOwnMeso", "out of range meso_idx");
	const QString &_client{m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT)};
	std::optional<bool> ret{std::nullopt};
	if (!_client.isEmpty())
		ret = m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == appUserModel()->userId(0);
	return ret;
}

void DBMesocyclesModel::setOwnMeso(const uint meso_idx)
{
	std::optional<bool> b_ownMeso{isOwnMeso(meso_idx)};
	if (b_ownMeso.has_value())
	{
		if (b_ownMeso.value())
		{
			m_mostRecentOwnMesoIdx = meso_idx;
			emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
			changeCanHaveTodaysWorkout(meso_idx);
			m_ownMesos->appendData(m_modeldata.at(meso_idx), meso_idx);
		}
		else
			m_clientMesos->appendData(m_modeldata.at(meso_idx), meso_idx);
	}
}

void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue, const bool bEmitSignal)
{
	const uint splitField{appUtils()->splitLetterToMesoSplitIndex(splitLetter)};
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
		if (meso_idx >= 0 && meso_idx < m_isNewMeso.count())
		{
			m_currentMesoIdx = meso_idx;
			setCurrentlyViewedMeso(meso_idx, bEmitSignal);
		}
		else
		{
			if (m_currentMesoIdx < 0 || m_currentMesoIdx > m_isNewMeso.count())
				m_currentMesoIdx = m_isNewMeso.count()-1;
		}
		if (bEmitSignal)
		{
			appSettings()->setLastViewedMesoIdx(meso_idx);
			emit currentMesoIdxChanged();
			changeCanHaveTodaysWorkout(meso_idx);
		}
	}
}

void DBMesocyclesModel::makeUsedSplits(const uint meso_idx)
{
	QStringList *usedSplit{&(m_usedSplits[meso_idx])};
	usedSplit->clear();
	const QString &strSplit{split(meso_idx)};
	for (uint i{0}; i < strSplit.length(); ++i)
	{
		const char chr{strSplit.at(i).toLatin1()};
		if (chr != 'R' && !usedSplit->contains(chr))
			usedSplit->append(QString{chr});
	}
	emit usedSplitsChanged(meso_idx);
}

bool DBMesocyclesModel::mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const
{
	for (const QStringList &modeldata : m_modeldata)
	{
		if (modeldata.at(MESOCYCLES_COL_NAME) == mesoName)
		{
			if (modeldata.at(MESOCYCLES_COL_COACH) == coach)
			{
				if (modeldata.at(MESOCYCLES_COL_CLIENT) == client)
					return true;
			}
		}
	}
	return false;
}

void DBMesocyclesModel::findNextOwnMeso()
{
	for (qsizetype i{count() -1}; i >= 0; --i)
	{
		if (isOwnMeso(i))
		{
			m_mostRecentOwnMesoIdx = i;
			break;
		}
	}
}

bool DBMesocyclesModel::isDateWithinMeso(const int meso_idx, const QDate &date) const
{
	if (meso_idx >= 0 && count() > 0)
	{
		if (date >= startDate(meso_idx))
			return date <= endDate(meso_idx);
	}
	return false;
}

int DBMesocyclesModel::getPreviousMesoId(const QString &userid, const int current_mesoid) const
{
	int meso_idx{static_cast<int>(count()-1)};
	for (; meso_idx >= 0; --meso_idx)
	{
		if (client(meso_idx) == userid)
			if (_id(meso_idx) < current_mesoid)
				break;
	}
	return meso_idx >= 0 ? _id(meso_idx) : -1;
}

QDate DBMesocyclesModel::getMesoMinimumStartDate(const QString &userid, const uint exclude_idx) const
{
	int meso_idx{static_cast<int>(count()-1)};
	for (; meso_idx >= 0; --meso_idx)
	{
		if (meso_idx != exclude_idx)
		{
			if (client(meso_idx) == userid)
				if (id(meso_idx) != STR_MINUS_ONE && isRealMeso(meso_idx))
					break;
		}
	}
	return meso_idx >= 0 ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, -6, 0);
}

QDate DBMesocyclesModel::getMesoMaximumEndDate(const QString &userid, const uint exclude_idx) const
{
	uint meso_idx{exclude_idx+1};
	for (; meso_idx < count(); ++meso_idx)
	{
		if (client(meso_idx) == userid)
			if (id(meso_idx) != STR_MINUS_ONE && isRealMeso(meso_idx))
				break;
	}
	return meso_idx < count() ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, 6, 0);
}

void DBMesocyclesModel::checkIfCanExport(const uint meso_idx, const bool bEmitSignal)
{
	if (!isNewMeso(meso_idx))
	{
		if (appDBInterface()->mesoHasAllPlans(meso_idx))
		{
			if (!m_canExport.at(meso_idx))
			{
				m_canExport[meso_idx] = true;
				if (bEmitSignal)
					emit canExportChanged(meso_idx, true);
			}
		}
		else
		{
			if (m_canExport.at(meso_idx))
			{
				m_canExport[meso_idx] = false;
				if (bEmitSignal)
					emit canExportChanged(meso_idx, false);
			}
		}
	}
}

int DBMesocyclesModel::exportToFile(const QString &filename, const bool, const bool, const bool) const
{
	const uint exportrow{m_exportRows.at(0)};
	int res{this->TPListModel::exportToFile(filename, true, false)};
	if (res >= 0)
	{
		m_splitModel->setExportRow(exportrow);
		res = m_splitModel->TPListModel::exportToFile(filename, false, true);
	}
	return res;
}

int DBMesocyclesModel::importFromFile(const QString &filename)
{
	QFile *inFile{new QFile{filename}};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return  APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList modeldata{MESOCYCLES_TOTAL_COLS};
	modeldata[0] = STR_MINUS_ONE;
	QStringList splitmodeldata{SIMPLE_MESOSPLIT_TOTAL_COLS};
	splitmodeldata[MESOSPLIT_COL_ID] = STR_MINUS_ONE;
	splitmodeldata[MESOSPLIT_COL_MESOID] = STR_MINUS_ONE;

	uint col{MESOCYCLES_COL_NAME};
	QString value;
	char buf[512];
	qint64 lineLength{0};
	const QString tableIdStr("0x000"_L1 + QString::number(MESOCYCLES_TABLE_ID));
	bool bFoundModelInfo{false};

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
					if (col < MESOCYCLES_TOTAL_COLS)
					{
						value = buf;
						value = value.remove(0, value.indexOf(':') + 2).simplified();
						if (!isFieldFormatSpecial(col))
							modeldata[col] = std::move(value);
						else
							modeldata[col] = std::move(formatFieldToImport(col, value, buf));
						if (col == MESOCYCLES_COL_CLIENT)
							col ++; //skip MESOCYCLES_COL_FILE
					}
					else
					{
						value = buf;
						const uint splitidx{appUtils()->splitLetterToMesoSplitIndex(value.at(value.indexOf(':')-1))};
						if (splitidx >= 2 && splitidx <= 7)
							splitmodeldata[splitidx] = std::move(value.remove(0, value.indexOf(':') + 2).simplified());
						else
							break;
						if (col >= MESOCYCLES_TOTAL_COLS + 5)
							break;
					}
					col++;
				}
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	if (bFoundModelInfo)
	{
		m_modeldata.append(std::move(modeldata));
		m_splitModel->m_modeldata.append(std::move(splitmodeldata));
	}
	return col >= MESOCYCLES_COL_SPLIT ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBMesocyclesModel::updateFromModel(const uint meso_idx, TPListModel *model)
{
	setImportMode(true);
	for (uint i{MESOCYCLES_COL_NAME}; i < MESOCYCLES_TOTAL_COLS; ++i)
		m_modeldata[meso_idx][i] = std::move(model->m_modeldata[0][i]);
	DBMesoSplitModel *splitModel{static_cast<DBMesocyclesModel*>(model)->mesoSplitModel()};
	for (uint i{MESOSPLIT_A}; i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
		m_splitModel->m_modeldata[meso_idx][i] = std::move(splitModel->m_modeldata[0][i]);
	setImportIdx(meso_idx);
	m_isNewMeso[meso_idx] = 0;
	changeCanHaveTodaysWorkout(meso_idx);
	return true;
}

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString &fieldValue) const
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
	return QString{}; //never reached
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString &fieldValue, const QString &fieldName) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return QString::number(appUtils()->getDateFromDateString(fieldValue).toJulianDay());
		case MESOCYCLES_COL_COACH:
			return fieldName.contains(tr("Coach")) ? fieldValue : QString{};
		case MESOCYCLES_COL_CLIENT:
			return fieldName.contains(tr("Client")) ? fieldValue : QString{};
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == tr("Yes") ? STR_ONE : STR_ZERO;
	}
	return QString{}; //never reached
}

QString DBMesocyclesModel::mesoFileName(const uint meso_idx) const
{
	return appUserModel()->localDir(client(meso_idx)) + mesosDir +
			(coach(meso_idx) != appUserModel()->userId(0) ? coach(meso_idx) + '/': QString{}) + name(meso_idx) + ".txt"_L1;
}

void DBMesocyclesModel::removeMesoFile(const uint meso_idx)
{
	const QString &mesofilename{mesoFileName(meso_idx)};
	appUserModel()->removeFileFromServer(appUtils()->getFileName(mesofilename), mesosDir, coach(meso_idx));
	static_cast<void>(QFile::remove(mesofilename));
}

void DBMesocyclesModel::sendMesoToUser(const uint meso_idx)
{
	QFile *mesoFile{appUtils()->openFile(mesoFileName(meso_idx), QIODeviceBase::ReadWrite|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (mesoFile)
	{
		setExportRow(meso_idx);
		if (exportContentsOnlyToFile(mesoFile))
		{
			mesoSplitModel()->exportContentsOnlyToFile(mesoFile); //export meso split(muscular groups)

			char splitletter{'A'};
			DBMesoSplitModel *splitModel{nullptr};
			if (meso_idx < m_mesoManagerList.count())
				splitModel = m_mesoManagerList.at(meso_idx)->plannerSplitModel(splitletter);

			if (splitModel != nullptr)
			{
				do {
					splitModel->exportContentsOnlyToFile(mesoFile, true);
				} while ((splitModel = m_mesoManagerList.at(meso_idx)->plannerSplitModel(++splitletter)) != nullptr);
				mesoFile->close();
				appUserModel()->sendFileToServer(mesoFile->fileName(), !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
													mesosDir + coach(meso_idx), client(meso_idx));
				delete mesoFile;
			}
			else
			{
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this,conn,mesoFile,meso_idx]
															(const uint table_idx, const QVariant &data) {
					if (table_idx == MESOSPLIT_TABLE_ID)
					{
						disconnect(*conn);
						const QMap<QChar,DBMesoSplitModel*> &allSplits(data.value<QMap<QChar,DBMesoSplitModel*>>());
						for (const auto &splitModel : allSplits)
							splitModel->exportContentsOnlyToFile(mesoFile, true);
						mesoFile->close();
						appUserModel()->sendFileToServer(mesoFile->fileName(), !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
															mesosDir + coach(meso_idx), client(meso_idx));
						delete mesoFile;
					}
				});
				appDBInterface()->loadAllSplits(meso_idx);
			}
		}
	}
}

int DBMesocyclesModel::newMesoFromFile(const QString &filename)
{
	QFile *mesoFile{appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (mesoFile)
	{
		const uint meso_idx{startNewMesocycle(false, false)};
		setImportMode(true);
		if (importFromContentsOnlyFile(mesoFile, meso_idx) == 0)
		{
			//Save the splits with a negative mesoId. This will only hold for the current session. Upon a new start up, the databases will be rid of all
			//negative Ids. If the meso is incorporated, the mesoIds will be replaced with the correct mesoId.
			m_modeldata[meso_idx][MESOCYCLES_COL_ID] = std::move(QString::number(m_lowestTempMesoId--));
			setNewMesoFieldCounter(meso_idx, 20);
			m_isNewMeso[meso_idx] = 0;
			makeUsedSplits(meso_idx);
			setOwnMeso(meso_idx);

			if (mesoSplitModel()->importFromContentsOnlyFile(mesoFile, meso_idx) == 0)
			{
				mesoSplitModel()->setMesoId(meso_idx, id(meso_idx));
				mesoSplitModel()->setImportMode(true);
				appDBInterface()->saveMesoSplit(meso_idx);
				mesoSplitModel()->setImportMode(false);
				for (const auto &splitletter: m_usedSplits.at(meso_idx))
				{
					DBMesoSplitModel* splitModel{new DBMesoSplitModel(this, true, meso_idx)};
					splitModel->setImportMode(true);
					splitModel->deleteLater();
					if (splitModel->importFromContentsOnlyFile(mesoFile) != -1)
					{
						splitModel->setSplitLetter(splitletter);
						appDBInterface()->saveMesoSplitComplete(splitModel);
					}
				}

				mesoFile->close();
				delete mesoFile;
				if (appDBInterface()->mesoHasAllPlans(meso_idx))
					return meso_idx;
			}
		}
	}
	return -1;
}

void DBMesocyclesModel::viewOnlineMeso(const QString &coach, const QString &mesoFileName)
{
	const int request_id{appUserModel()->downloadFileFromServer(mesoFileName, appUserModel()->localDir(appUserModel()->userId(0)) + mesosDir + coach + '/',
		QString{}, mesosDir + coach, appUserModel()->userId(0))};
	if (request_id != -1)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appUserModel(), &DBUserModel::fileDownloaded, this, [this,conn,request_id]
									(const bool success, const uint requestid, const QString &localFileName) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (success)
				{
					const int meso_idx{newMesoFromFile(localFileName)};
					if (meso_idx >= 0)
						getMesocyclePage(meso_idx);
				}
			}
		});
	}
}

void DBMesocyclesModel::scanTemporaryMesocycles()
{
	QFileInfoList mesos;
	appUtils()->scanDir(appUserModel()->localDir(appUserModel()->userId(0)) + mesosDir, mesos, "*.txt"_L1, true);
	if (!mesos.isEmpty())
	{
		for(const auto &mesofile : std::as_const(mesos))
		{
			if (!mesoPlanExists(appUtils()->getFileName(mesofile.fileName(), true), appUtils()->getLastDirInPath(mesofile.filePath()), appUserModel()->userId(0)))
				static_cast<void>(newMesoFromFile(mesofile.filePath()));
		}
	}
}
