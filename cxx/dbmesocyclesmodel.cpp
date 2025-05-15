#include "dbmesocyclesmodel.h"

#include "dbinterface.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmanager.h"
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

using DBSplitModel = DBExercisesModel;

static const QString &mesoFileIdentifier{"0x02"_L1};
DBMesocyclesModel *DBMesocyclesModel::app_meso_model(nullptr);

DBMesocyclesModel::DBMesocyclesModel(QObject *parent, const bool bMainAppModel)
	: QObject{parent}, m_mostRecentOwnMesoIdx{-1}, m_lowestTempMesoId{-1}, m_bCanHaveTodaysWorkout{false}
{
	app_meso_model = this;
	setCurrentMesoIdx(appSettings()->lastViewedMesoIdx(), false);

	m_calendarModel = new DBMesoCalendarManager{this};
	m_ownMesos = new homePageMesoModel{this};
	m_clientMesos = new homePageMesoModel{this};

	connect(m_calendarModel, &DBMesoCalendarManager::calendarChanged, this, [this] (const uint meso_idx, const int calendar_day, const uint field) {
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
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBMesocyclesModel::labelChanged);
}

DBMesocyclesModel::~DBMesocyclesModel()
{
	delete m_ownMesos;
	delete m_clientMesos;
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

DBExercisesModel *DBMesocyclesModel::splitModel(const uint meso_idx, const QChar &split_letter)
{
	if (!m_splitModels.at(meso_idx).value(split_letter))
		m_splitModels[meso_idx][split_letter] = new DBSplitModel{mesoCalendarModel(), meso_idx, split_letter};
	return m_splitModels.at(meso_idx).value(split_letter);
}

void DBMesocyclesModel::getMesocyclePage(const uint meso_idx)
{
	setCurrentMesoIdx(meso_idx, true);
	mesoManager(meso_idx)->getMesocyclePage();
}

uint DBMesocyclesModel::startNewMesocycle(const bool bCreatePage, const std::optional<bool> bOwnMeso)
{
	const uint meso_idx{newMesocycle(std::move(QStringList{} << std::move(newMesoTemporaryId()) << QString{} << QString{} << QString{} <<
		QString{} << QString{} << std::move("RRRRRRR"_L1) << appUserModel()->userId(0) <<
			(bOwnMeso.has_value() ? (bOwnMeso.value() ? appUserModel()->userId(0) : appUserModel()->defaultClient()) : QString{}) <<
						QString{} << QString{} << STR_ONE))};

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
	{
		appDBInterface()->removeMesocycle(meso_idx);
		appDBInterface()->removeAllMesoSplits(meso_idx);
	}
	m_splitModels.remove(meso_idx);
	m_calendarModel->removeCalendarForMeso(meso_idx);
	m_isNewMeso.remove(meso_idx);
	m_newMesoFieldCounter.remove(meso_idx);
	m_newMesoCalendarChanged.remove(meso_idx);
	m_canExport.remove(meso_idx);
	removeMesoFile(meso_idx);

	if (isOwnMeso(meso_idx))
		m_ownMesos->removeData(m_mesoData.at(meso_idx));
	else
		m_clientMesos->removeData(m_mesoData.at(meso_idx));

	m_mesoData.remove(meso_idx);

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
	m_mesoData.append(std::move(infolist));
	m_splitModels.append(std::move(QMap<QChar, DBSplitModel*>{{}}));
	const uint meso_idx{count()-1};

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
	for (uint i{0}; i < m_mesoData.count(); ++i)
	{
		if (m_mesoData.at(i).at(MESOCYCLES_COL_ID) == mesoIdStr)
			return i;
	}
	return -1;
}

void DBMesocyclesModel::setName(const uint meso_idx, const QString &new_name)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_NAME] = new_name;
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
	m_mesoData[meso_idx][MESOCYCLES_COL_STARTDATE] = std::move(QString::number(new_date.toJulianDay()));
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
	m_mesoData[meso_idx][MESOCYCLES_COL_ENDDATE] = std::move(QString::number(new_date.toJulianDay()));
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
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLIT] = new_split;
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

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar &splitLetter) const
{
	return splitLetter != 'R' ?
		m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITA - static_cast<int>(splitLetter.cell()) - static_cast<int>('A')) :
		QString{};
}
void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue)
{
	const int split_col{MESOCYCLES_COL_SPLITA - static_cast<int>(splitLetter.cell()) - static_cast<int>('A')};
	m_mesoData[meso_idx][split_col] = newSplitValue;
	setModified(meso_idx, split_col);
}

void DBMesocyclesModel::setCoach(const uint meso_idx, const QString &new_coach)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_COACH] = new_coach;
	setModified(meso_idx, MESOCYCLES_COL_COACH);
	m_curMesos->emitDataChanged(meso_idx, mesoCoachRole);
}

void DBMesocyclesModel::setClient(const uint meso_idx, const QString &new_client)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_CLIENT] = new_client;
	setModified(meso_idx, MESOCYCLES_COL_CLIENT);
	m_curMesos->emitDataChanged(meso_idx, mesoClientRole);
}

std::optional<bool> DBMesocyclesModel::isOwnMeso(const int meso_idx) const
{
	Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_mesoData.count(), "DBMesocyclesModel::isOwnMeso", "out of range meso_idx");
	const QString &_client{m_mesoData.at(meso_idx).at(MESOCYCLES_COL_CLIENT)};
	std::optional<bool> ret{std::nullopt};
	if (!_client.isEmpty())
		ret = m_mesoData.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == appUserModel()->userId(0);
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
			m_ownMesos->appendData(m_mesoData.at(meso_idx), meso_idx);
		}
		else
			m_clientMesos->appendData(m_mesoData.at(meso_idx), meso_idx);
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
	for (const QStringList &modeldata : m_mesoData)
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
		if (appDBInterface()->mesoHasAllSplitPlans(meso_idx))
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

int DBMesocyclesModel::exportToFile(const uint meso_idx, const QString &filename) const
{
	QFile *out_file{appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (!out_file)
		return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;

	int ret{APPWINDOW_MSG_EXPORT_FAILED};
	const QList<uint> &export_row{QList<uint>{} << meso_idx};
	if (appUtils()->writeDataToFile(out_file, mesoFileIdentifier, m_mesoData, export_row, false))
	{
		if (m_splitModels.at(meso_idx).count() > 0)
			ret = exportToFile_splitData(meso_idx, out_file, false);
		else
		{
			out_file->close();
			return continueExport(meso_idx, filename, false);
		}
	}
	out_file->close();
	return ret;
}

int DBMesocyclesModel::exportToFormattedFile(const uint meso_idx, const QString &filename) const
{
	QFile *out_file{appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (!out_file)
		return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;

	const QList<uint> &export_row{QList<uint>{} << meso_idx};
	QList<std::function<QString(void)>> field_description{QList<std::function<QString(void)>>{} << nullptr <<
										[this] () { return mesoNameLabel(); } <<
										[this] () { return startDateLabel(); } <<
										[this] () { return endDateLabel(); } <<
										[this] () { return notesLabel(); } <<
										[this] () { return nWeeksLabel(); } <<
										[this] () { return splitLabel(); } <<
										[this] () { return splitLabelA(); } <<
										[this] () { return splitLabelB(); } <<
										[this] () { return splitLabelC(); } <<
										[this] () { return splitLabelD(); } <<
										[this] () { return splitLabelE(); } <<
										[this] () { return splitLabelF(); } <<
										[this] () { return coachLabel(); } <<
										[this] () { return clientLabel(); } <<
										nullptr <<
										[this] () { return typeLabel(); } <<
										[this] () { return realMesoLabel(); }
	};

	int ret{APPWINDOW_MSG_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					mesoFileIdentifier,
					m_mesoData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_row,
					QString{tr("Exercises Program") + "\n\n"_L1})
	)
	{
		if (m_splitModels.at(meso_idx).count() > 0)
			ret = exportToFile_splitData(meso_idx, out_file, true);
		{
			out_file->close();
			return continueExport(meso_idx, filename, true);
		}
	}
	out_file->close();
	return ret;
}

int DBMesocyclesModel::importFromFile(const uint meso_idx, const QString &filename)
{
	QFile *in_file{appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (!in_file)
		return  APPWINDOW_MSG_OPEN_FAILED;

	int ret{appUtils()->readDataFromFile(in_file, m_mesoData, fieldCount(), mesoFileIdentifier, meso_idx)};
	if (ret > 0)
	{
		setId(meso_idx, newMesoTemporaryId());
		for (auto split_model : m_splitModels.at(meso_idx))
		{
			if ((ret = split_model->importFromFile(filename, in_file)) != APPWINDOW_MSG_READ_FROM_FILE_OK)
				break;
		} while (true);
	}
	in_file->close();
	return ret;
}

int DBMesocyclesModel::importFromFormattedFile(const uint meso_idx, const QString &filename)
{
	QFile *in_file{appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (!in_file)
		return  APPWINDOW_MSG_OPEN_FAILED;

	int ret{appUtils()->readDataFromFormattedFile(in_file,
												m_mesoData,
												fieldCount(),
												mesoFileIdentifier,
												[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	if (ret > 0)
	{
		setId(meso_idx, newMesoTemporaryId());
		for (auto split_model : m_splitModels.at(meso_idx))
		{
			if ((ret = split_model->importFromFormattedFile(filename, in_file)) != APPWINDOW_MSG_READ_FROM_FILE_OK)
				break;
		} while (true);
	}
	in_file->close();
	return ret;
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
	return fieldValue;
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return QString::number(appUtils()->getDateFromDateString(fieldValue).toJulianDay());
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == tr("Yes") ? STR_ONE : STR_ZERO;
	}
	return fieldValue;
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
	const QString &filename{mesoFileName(meso_idx)};
	const int ret{exportToFile(meso_idx, filename)};
	if (ret >= APPWINDOW_MSG_DEFERRED_ACTION)
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBMesocyclesModel::deferredActionFinished, this, [this,conn,ret,meso_idx,filename] (const uint action_id, const int action_result) {
			if (ret == action_id)
			{
				disconnect(*conn);
				if (action_result)
					appUserModel()->sendFileToServer(filename, !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
													mesosDir + coach(meso_idx), client(meso_idx));
			}
		});
	}
	else
		appUserModel()->sendFileToServer(filename, !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
										mesosDir + coach(meso_idx), client(meso_idx));
}

int DBMesocyclesModel::newMesoFromFile(const QString &filename)
{
	const uint meso_idx{startNewMesocycle(false, false)};
	int import_result{importFromFile(meso_idx, filename)};
	if (import_result == APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE)
	{
		import_result = importFromFormattedFile(meso_idx, filename);
		if (import_result < 0)
			return import_result;
	}
	setNewMesoFieldCounter(meso_idx, 20);
	m_isNewMeso[meso_idx] = 0;
	makeUsedSplits(meso_idx);
	setOwnMeso(meso_idx);
	return APPWINDOW_MSG_IMPORT_OK;
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

int DBMesocyclesModel::continueExport(const uint meso_idx, const QString &filename, const bool formatted) const
{
	const int id{deferredActionId()};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &DBMesocyclesModel::internalSignal, this, [this,conn,id,meso_idx,filename,formatted]
												(const uint _meso_idx, const uint _id, const bool _result) {
		if (meso_idx == _meso_idx && id == _id)
		{
			disconnect(*conn);
			if (_result)
			{
				QFile *out_file{appUtils()->openFile(filename, QIODeviceBase::WriteOnly|QIODeviceBase::Append|QIODeviceBase::Text)};
				int ret{APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED};
				if (out_file)
					ret = exportToFile_splitData(meso_idx, out_file, formatted);
				out_file->close();
				emit const_cast<DBMesocyclesModel*>(this)->deferredActionFinished(id, ret);
			}
		}
	});
	const_cast<DBMesocyclesModel*>(this)->loadSplitModels(meso_idx, id);
	return id;
}

int DBMesocyclesModel::exportToFile_splitData(const uint meso_idx, QFile *mesoFile, const bool formatted) const
{
	QList<QStringList> split_data{m_splitModels.at(meso_idx).count()};
	int ret{0};
	if (!formatted)
	{
		for (const auto splitModel : m_splitModels.at(meso_idx))
			ret = splitModel->exportToFile(QString{}, mesoFile);
	}
	else
	{
		for (const auto splitModel : m_splitModels.at(meso_idx))
			ret = splitModel->exportToFormattedFile(QString{}, mesoFile);
	}
	return ret;
}
