#include "dbmesocyclesmodel.h"

#include "thread_manager.h"
#include "dbmesocyclestable.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmanager.h"
#include "dbusermodel.h"
#include "dbworkoutsorsplitstable.h"
#include "homepagemesomodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tpsettings.h"
#include "translationclass.h"

#include <utility>

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: QObject{parent}, m_currentMesoIdx{-11111}, m_mostRecentOwnMesoIdx{-11111}, m_lowestTempMesoId{-1}, m_bCanHaveTodaysWorkout{false}
{
	setCurrentMesoIdx(appSettings()->lastViewedMesoIdx(), false);

	m_calendarManager = new DBMesoCalendarManager{this};
	m_ownMesos = new HomePageMesoModel{this};
	m_clientMesos = new HomePageMesoModel{this};

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBMesocyclesModel::labelChanged);
	connect(this, &DBMesocyclesModel::isNewMesoChanged, this, [this] (const uint meso_idx) { incorporateMeso(meso_idx); });
	connect(this, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field)
	{
		if (!isNewMeso(meso_idx))
		{
			m_dbModelInterface->setModified(meso_idx, field);
			appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField);
			switch (field)
			{
				case MESOCYCLES_COL_STARTDATE:
				case MESOCYCLES_COL_ENDDATE:
				case MESOCYCLES_COL_SPLIT:
					m_calendarManager->remakeMesoCalendar(meso_idx);
				break;
			}
			if (field >= MESOCYCLES_COL_SPLIT && field <= MESOCYCLES_COL_SPLITF)
			{
				if (!isOwnMeso(meso_idx))
					checkIfCanExport(meso_idx);
			}
		}
	});
	getAllMesocycles();
}

QMLMesoInterface *DBMesocyclesModel::mesoManager(const uint meso_idx)
{
	QMLMesoInterface *mesomanager{m_mesoManagerList.value(meso_idx)};
	if (!mesomanager)
	{
		mesomanager = new QMLMesoInterface{this, meso_idx};
		m_mesoManagerList[meso_idx] = mesomanager;
	}
	return mesomanager;
}

void DBMesocyclesModel::removeMesoManager(const uint meso_idx)
{
	QMLMesoInterface* mesomanager{m_mesoManagerList.value(meso_idx)};
	if (mesomanager)
	{
		delete mesomanager;
		m_mesoManagerList.remove(meso_idx);
	}
}

void DBMesocyclesModel::incorporateMeso(const uint meso_idx)
{
	m_dbModelInterface->setModified(meso_idx, 0);
	appThreadManager()->runAction(m_db, ThreadManager::InsertRecords);
}

void DBMesocyclesModel::getMesocyclePage(const uint meso_idx)
{
	setCurrentMesoIdx(meso_idx, true);
	if (meso_idx < m_mesoData.count())
		mesoManager(meso_idx)->getMesocyclePage();
}

uint DBMesocyclesModel::startNewMesocycle(const bool bCreatePage, const std::optional<bool> bOwnMeso)
{
	const bool own_meso{bOwnMeso.has_value() ? bOwnMeso.value() : true};
	const uint meso_idx{newMesocycle(std::move(QStringList{} << std::move(newMesoTemporaryId()) <<
		std::move(QString::number(appUtils()->generateUniqueId())) << QString{} << QString{} << QString{} << QString{} <<
		std::move("RRRRRRR"_L1) << QString{} << QString{} << QString{} << QString{} << QString{} << QString{} <<
		appUserModel()->userId(0) << (own_meso ? appUserModel()->userId(0) : appUserModel()->defaultClient()) <<
		QString{} << QString{} << std::move("1"_L1)))};

	int32_t newMesoRequiredFields{0};
	setBit(newMesoRequiredFields, MESOCYCLES_COL_NAME);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_STARTDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_ENDDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_SPLIT);
	m_isNewMeso[meso_idx] = newMesoRequiredFields;

	if (bCreatePage)
	{
		static_cast<void>(mesoManager(meso_idx));
		getMesocyclePage(meso_idx);
	}
	return meso_idx;
}

void DBMesocyclesModel::removeMesocycle(const uint meso_idx)
{
	if (meso_idx >= m_mesoData.count())
		return;

	m_dbModelInterface->setRemovalInfo(meso_idx, QList<uint>{1, MESOCYCLES_COL_ID});
	appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);
	DBSplitModel *split_model{splitModel(meso_idx, 'A')};
	m_splitsDB->setDBModelInterface(split_model->dbModelInterface());
	split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{1, EXERCISES_COL_MESOID});
	appThreadManager()->runAction(m_splitsDB, ThreadManager::DeleteRecords);

	qDeleteAll(m_splitModels.at(meso_idx));
	m_splitModels.remove(meso_idx);
	m_calendarManager->removeCalendarForMeso(meso_idx, true);
	m_isNewMeso.remove(meso_idx);
	m_canExport.remove(meso_idx);
	removeMesoFile(meso_idx);

	if (isOwnMeso(meso_idx))
		m_ownMesos->removeData(meso_idx);
	else
		m_clientMesos->removeData(meso_idx);

	removeMesoManager(meso_idx);
	m_mesoData.remove(meso_idx);

	const int most_recent_ownMesoIdx{m_mostRecentOwnMesoIdx};
	if (m_mostRecentOwnMesoIdx > static_cast<int>(meso_idx))
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
	if (meso_idx < m_mesoData.count())
		mesoManager(meso_idx)->getExercisesPlannerPage();
}

void DBMesocyclesModel::getMesoCalendarPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getCalendarPage();
}

void DBMesocyclesModel::openSpecificWorkout(const uint meso_idx, const QDate &date)
{
	mesoManager(meso_idx)->getWorkoutPage(date);
}

const uint DBMesocyclesModel::newMesocycle(QStringList &&infolist)
{
	const uint meso_idx{count()};
	m_mesoData.append(std::move(infolist));
	m_splitModels.append(std::move(QMap<QChar, DBSplitModel*>{{'A', nullptr}}));
	m_canExport.append(false);
	setOwnMeso(meso_idx);
	m_calendarManager->getCalendarForMeso(meso_idx);
	m_usedSplits.append(QString{});
	makeUsedSplits(meso_idx);
	m_isNewMeso.append(0);
	return meso_idx;
}

bool DBMesocyclesModel::isNewMesoFieldSet(const uint meso_idx, const uint field) const
{
	return isBitSet(m_isNewMeso.at(meso_idx), field);
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

int DBMesocyclesModel::idxFromId(const QString &meso_id) const
{
	uint meso_idx{0};
	const auto &meso = std::find_if(m_mesoData.cbegin(), m_mesoData.cend(), [meso_id,&meso_idx] (const auto &meso_info) {
		++meso_idx;
		return meso_info.at(MESOCYCLES_COL_ID) == meso_id;
	});
	return meso != m_mesoData.cend() ? meso_idx : -1;
}

void DBMesocyclesModel::setName(const uint meso_idx, const QString &new_name)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_NAME] = new_name;
	setModified(meso_idx, MESOCYCLES_COL_NAME);
	m_curMesos->emitDataChanged(meso_idx, mesoNameRole);
}

void DBMesocyclesModel::setStartDate(const uint meso_idx, const QDate &new_date)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_STARTDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESOCYCLES_COL_STARTDATE);
	m_curMesos->emitDataChanged(meso_idx, mesoStartDateRole);
	changeCanHaveTodaysWorkout(meso_idx);
}

void DBMesocyclesModel::setEndDate(const uint meso_idx, const QDate &new_date)
{
	m_mesoData[meso_idx][MESOCYCLES_COL_ENDDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESOCYCLES_COL_ENDDATE);
	m_curMesos->emitDataChanged(meso_idx, mesoEndDateRole);
	changeCanHaveTodaysWorkout(meso_idx);
}

void DBMesocyclesModel::setSplit(const uint meso_idx, const QString &new_split)
{
	if (new_split != split(meso_idx))
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLIT] = new_split;
		if (isSplitOK(meso_idx))
			setModified(meso_idx, MESOCYCLES_COL_SPLIT);
		m_curMesos->emitDataChanged(meso_idx, mesoSplitRole);
		makeUsedSplits(meso_idx);
	}
}

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar &splitLetter) const
{
	return splitLetter != 'R' ?
		m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITA + static_cast<int>(splitLetter.cell()) - static_cast<int>('A')) :
		QString{};
}
void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue)
{
	const int split_col{MESOCYCLES_COL_SPLITA + static_cast<int>(splitLetter.cell()) - static_cast<int>('A')};
	m_mesoData[meso_idx][split_col] = newSplitValue;
	setModified(meso_idx, split_col);
}

bool DBMesocyclesModel::isSplitOK(const QString &strsplit, const uint meso_idx) const
{
	//static const QRegularExpression rgex{"(?=.*[ABCDEF])(?=.*[R])"_L1};
	//return rgex.match(split).hasMatch()
	bool ok{false};
	for (const auto &splitletter : strsplit)
	{
		if (splitletter >= 'A' && splitletter <= 'F')
		{
			ok = true;
			if (muscularGroup(meso_idx, splitletter).isEmpty())
			{
				ok = false;
				break;
			}
		}
	}
	return ok;
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

bool DBMesocyclesModel::isOwnMeso(const uint meso_idx) const
{
	if (meso_idx < m_mesoData.count())
	{
		const QString &_client{m_mesoData.at(meso_idx).at(MESOCYCLES_COL_CLIENT)};
		if (!_client.isEmpty())
			return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == appUserModel()->userId(0);
	}
	return false;
}

void DBMesocyclesModel::setOwnMeso(const uint meso_idx)
{
	const bool own_meso{isOwnMeso(meso_idx)};
	if (own_meso)
	{
		m_mostRecentOwnMesoIdx = meso_idx;
		emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
		changeCanHaveTodaysWorkout(meso_idx);
		m_ownMesos->appendData(meso_idx);
		const int conflicting_meso_idx{m_clientMesos->indexOf(meso_idx)};
		if (conflicting_meso_idx >= 0)
			m_clientMesos->removeRow(conflicting_meso_idx);
	}
	else
	{
		m_clientMesos->appendData(meso_idx);
		const int conflicting_meso_idx{m_ownMesos->indexOf(meso_idx)};
		if (conflicting_meso_idx >= 0)
			m_ownMesos->removeRow(conflicting_meso_idx);
	}
}

void DBMesocyclesModel::setCurrentMesoIdx(const int meso_idx, const bool emit_signal)
{
	if (meso_idx != m_currentMesoIdx)
	{
		if (meso_idx >= 0 && meso_idx < m_isNewMeso.count())
		{
			m_currentMesoIdx = meso_idx;
			setCurrentlyViewedMeso(meso_idx, emit_signal);
		}
		else
		{
			if (m_currentMesoIdx < 0 || m_currentMesoIdx > m_isNewMeso.count())
				m_currentMesoIdx = m_isNewMeso.count() - 1;
		}
		if (emit_signal)
		{
			appSettings()->setLastViewedMesoIdx(m_currentMesoIdx);
			emit currentMesoIdxChanged();
			changeCanHaveTodaysWorkout(m_currentMesoIdx);
		}
	}
}

void DBMesocyclesModel::makeUsedSplits(const uint meso_idx)
{
	QString *usedSplit{&(m_usedSplits[meso_idx])};
	usedSplit->clear();
	const QString &strSplit{split(meso_idx)};
	for (const auto &split_letter : strSplit)
	{
		if (split_letter != 'R' && !usedSplit->contains(split_letter))
			usedSplit->append(split_letter);
	}
	emit usedSplitsChanged(meso_idx);
}

void DBMesocyclesModel::loadSplits(const uint meso_idx)
{
	for (const auto &split_letter : m_usedSplits.at(meso_idx))
	{
		DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
		if (!split_model)
		{
			split_model = new DBSplitModel{this, m_splitsDB, meso_idx, split_letter};
			m_splitModels[meso_idx].insert(split_letter, split_model);
		}
	}
}

void DBMesocyclesModel::removeSplit(const uint meso_idx, const QChar &split_letter)
{
	DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
	split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{2} << EXERCISES_COL_MESOID << EXERCISES_COL_SPLITLETTER);
	appThreadManager()->runAction(m_splitsDB, ThreadManager::DeleteRecords);
	m_splitModels[meso_idx].remove(split_letter);
}

bool DBMesocyclesModel::mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const
{
	for (const QStringList &modeldata : m_mesoData)
	{
		if (modeldata.at(MESOCYCLES_COL_NAME) == mesoName)
		{
			if (coach != "mesocycles"_L1)
			{
				if (modeldata.at(MESOCYCLES_COL_COACH) == coach)
				{
					if (modeldata.at(MESOCYCLES_COL_CLIENT) == client)
						return true;
				}
			}
			else
				return true;
		}
	}
	return false;
}

void DBMesocyclesModel::findNextOwnMeso()
{
	for (int i{static_cast<int>(count()) - 1}; i >= 0; --i)
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
				if (!isMesoTemporary(meso_idx) && isRealMeso(meso_idx))
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
			if (id(meso_idx) != "-1"_L1 && isRealMeso(meso_idx))
				break;
	}
	return meso_idx < count() ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, 6, 0);
}

void DBMesocyclesModel::checkIfCanExport(const uint meso_idx, const bool emit_signal)
{
	if (!isNewMeso(meso_idx))
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_splitsDB, &TPDatabaseTable::actionFinished, this, [this,conn,meso_idx,emit_signal]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2)
		{
			if (op == ThreadManager::CustomOperation)
			{
				disconnect(*conn);
				const bool can_export{return_value2.toBool()};
				if (m_canExport.at(meso_idx) != can_export)
				{
					m_canExport[meso_idx] = can_export;
					if (emit_signal)
						emit canExportChanged(meso_idx, can_export);
				}
			}
		});
		auto x = [this,meso_idx] () -> std::pair<QVariant,QVariant> {
							return m_splitsDB->mesoHasAllSplitPlans(id(meso_idx), usedSplits(meso_idx)); };
		m_splitsDB->setCustQueryFunction(x);
		appThreadManager()->runAction(m_splitsDB, ThreadManager::CustomOperation);
	}
}

int DBMesocyclesModel::exportToFile(const uint meso_idx, const QString &filename) const
{
	QFile *out_file{appUtils()->openFile(filename, false, true, false, true)};
	if (!out_file)
		return TP_RET_CODE_OPEN_CREATE_FAILED;

	int ret{TP_RET_CODE_EXPORT_FAILED};
	QList<uint> export_row;
	export_row.append(meso_idx);
	if (appUtils()->writeDataToFile(out_file, appUtils()->mesoFileIdentifier, m_mesoData, export_row))
		ret = exportToFile_splitData(meso_idx, out_file, false);
	out_file->close();
	return ret;
}

int DBMesocyclesModel::exportToFormattedFile(const uint meso_idx, const QString &filename) const
{
	QFile *out_file{appUtils()->openFile(filename, false, true, false, true)};
	if (!out_file)
		return TP_RET_CODE_OPEN_CREATE_FAILED;

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

	int ret{TP_RET_CODE_EXPORT_FAILED};
	if (appUtils()->writeDataToFormattedFile(out_file,
					appUtils()->mesoFileIdentifier,
					m_mesoData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_row,
					QString{tr("Exercises Program") + "\n\n"_L1})
	)
		ret = exportToFile_splitData(meso_idx, out_file, true);

	out_file->close();
	return ret;
}

int DBMesocyclesModel::importFromFile(const uint meso_idx, const QString &filename)
{
	QFile *in_file{appUtils()->openFile(filename)};
	if (!in_file)
		return  TP_RET_CODE_OPEN_READ_FAILED;

	int ret{appUtils()->readDataFromFile(in_file, m_mesoData, fieldCount(), appUtils()->mesoFileIdentifier, meso_idx)};
	if (ret > 0)
	{
		setId(meso_idx, newMesoTemporaryId());
		for (const auto split_model : std::as_const(m_splitModels.at(meso_idx)))
		{
			if ((ret = split_model->importFromFile(filename, in_file)) != TP_RET_CODE_IMPORT_OK)
				break;
		}
	}
	in_file->close();
	return ret;
}

int DBMesocyclesModel::importFromFormattedFile(const uint meso_idx, const QString &filename)
{
	QFile *in_file{appUtils()->openFile(filename)};
	if (!in_file)
		return  TP_RET_CODE_OPEN_READ_FAILED;

	int ret{appUtils()->readDataFromFormattedFile(in_file,
												m_mesoData,
												fieldCount(),
												appUtils()->mesoFileIdentifier,
												[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	if (ret > 0)
	{
		setId(meso_idx, newMesoTemporaryId());
		for (auto split_model : m_splitModels.at(meso_idx))
		{
			if ((ret = split_model->importFromFormattedFile(filename, in_file)) != TP_RET_CODE_IMPORT_OK)
				break;
		}
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
			return fieldValue == '1' ? tr("Yes") : tr("No");
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
			return fieldValue == tr("Yes") ? "1"_L1 : "0"_L1;
	}
	return fieldValue;
}

QString DBMesocyclesModel::mesoFileName(const uint meso_idx) const
{
	return appUserModel()->userDir(client(meso_idx)) + mesosSubDir +
			(coach(meso_idx) != appUserModel()->userId(0) ? coach(meso_idx) % '/': QString{}) % name(meso_idx) % ".txt"_L1;
}

void DBMesocyclesModel::removeMesoFile(const uint meso_idx)
{
	const QString &mesofilename{mesoFileName(meso_idx)};
	appUserModel()->removeFileFromServer(appUtils()->getFileName(mesofilename), mesosSubDir, coach(meso_idx));
	static_cast<void>(QFile::remove(mesofilename));
}

void DBMesocyclesModel::sendMesoToUser(const uint meso_idx, const bool just_save_local_file)
{
	if (isNewMeso(meso_idx))
		return;
	const QString &filename{mesoFileName(meso_idx)};
	const int ret{exportToFile(meso_idx, filename)};
	if (just_save_local_file)
		return;

	if (ret >= TP_RET_CODE_DEFERRED_ACTION)
	{
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &DBMesocyclesModel::deferredActionFinished, this, [this,conn,ret,meso_idx,filename]
											(const uint action_id, const int action_result)
		{
			if (ret == action_id)
			{
				disconnect(*conn);
				if (action_result == TP_RET_CODE_EXPORT_OK)
					appUserModel()->sendFileToServer(filename, nullptr, !isOwnMeso(meso_idx) ?
						tr("Exercises Program sent to client") : QString{}, mesosSubDir + coach(meso_idx), client(meso_idx));
			}
		});
	}
	else
		appUserModel()->sendFileToServer(filename, nullptr, !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
										mesosSubDir + coach(meso_idx), client(meso_idx));
}

int DBMesocyclesModel::newMesoFromFile(const QString &filename, const bool from_coach, const std::optional<bool> &file_formatted)
{
	const uint meso_idx{startNewMesocycle(false, false)};
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value())
	{
		if (file_formatted.value())
			import_result = importFromFormattedFile(meso_idx, filename);
		else
			import_result = importFromFile(meso_idx, filename);
	}
	else
	{
		import_result = importFromFile(meso_idx, filename);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(meso_idx, filename);
	}
	if (import_result < 0)
	{
		removeMesocycle(meso_idx);
		return import_result;
	}

	int32_t newMesoRequiredFields{0};
	if (from_coach)
		setBit(newMesoRequiredFields, MESOCYCLES_COL_IMPORTED_AND_UNACCEPTED);
	else
	{
		if (name(meso_idx).length() < 5)
			setBit(newMesoRequiredFields, MESOCYCLES_COL_NAME);
		if (strStartDate(meso_idx).isEmpty())
			setBit(newMesoRequiredFields, MESOCYCLES_COL_STARTDATE);
		if (isRealMeso(meso_idx) && strEndDate(meso_idx).isEmpty())
			setBit(newMesoRequiredFields, MESOCYCLES_COL_ENDDATE);
		if (!isSplitOK(meso_idx))
			setBit(newMesoRequiredFields, MESOCYCLES_COL_SPLIT);
	}
	m_isNewMeso[meso_idx] = newMesoRequiredFields;
	makeUsedSplits(meso_idx);
	setOwnMeso(meso_idx);
	return TP_RET_CODE_IMPORT_OK;
}

int DBMesocyclesModel::importSplitFromFile(const QString &filename, const uint meso_idx, uint split,
													const std::optional<bool> &file_formatted)
{
	QChar split_letter;
	switch (split)
	{
		case IFC_MESOSPLIT_A: split_letter = 'A'; break;
		case IFC_MESOSPLIT_B: split_letter = 'B'; break;
		case IFC_MESOSPLIT_C: split_letter = 'C'; break;
		case IFC_MESOSPLIT_D: split_letter = 'D'; break;
		case IFC_MESOSPLIT_E: split_letter = 'E'; break;
		case IFC_MESOSPLIT_F: split_letter = 'F'; break;
		default: return TP_RET_CODE_CUSTOM_ERROR;
	}

	DBSplitModel *new_split{splitModel(meso_idx, split_letter)};
	new_split->clearExercises();
	return new_split->newExercisesFromFile(filename, file_formatted);
}

void DBMesocyclesModel::viewOnlineMeso(const QString &coach, const QString &mesoFileName)
{
	const int request_id{appUserModel()->downloadFileFromServer(mesoFileName, appUserModel()->userDir() + mesosSubDir + coach + '/',
								QString{}, mesosSubDir + coach, appUserModel()->userId(0))};

	auto getPage = [this] (const QString &meso_file_name) {
		const int meso_idx{newMesoFromFile(meso_file_name, true)};
		if (meso_idx >= 0)
			getMesocyclePage(meso_idx);
	};

	if (request_id == TP_RET_CODE_DOWNLOAD_FAILED)
		return;
	else if (request_id == TP_RET_CODE_NO_CHANGES_SUCCESS)
	{
		getPage(mesoFileName);
		return;
	}

	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(appUserModel(), &DBUserModel::fileDownloaded, this, [this,conn,request_id,getPage]
												(const bool success, const uint requestid, const QString &localFileName)
	{
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (success)
				getPage(localFileName);
		}
	});
}

void DBMesocyclesModel::scanTemporaryMesocycles()
{
	QFileInfoList mesos;
	appUtils()->scanDir(appUserModel()->userDir() + mesosSubDir, mesos, "*.txt"_L1, true);
	if (!mesos.isEmpty())
	{
		for(const auto &mesofile : std::as_const(mesos))
		{
			const QString &coach{appUtils()->getLastDirInPath(mesofile.filePath())};
			if (!mesoPlanExists(appUtils()->getFileName(mesofile.fileName(), true), coach, appUserModel()->userId(0)))
			{
				if (newMesoFromFile(mesofile.filePath(), coach != "mesocycles"_L1) < 0)
					static_cast<void>(QFile::remove(mesofile.filePath()));
			}
		}
	}
}

void DBMesocyclesModel::getAllMesocycles()
{
	m_dbModelInterface = new DBModelInterfaceMesocycle{this};
	m_db = new DBMesocyclesTable{m_dbModelInterface};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
	connect(m_db, &DBMesocyclesTable::mesocyclesAcquired, this, [this] (QStringList meso_info) {
		newMesocycle(std::move(meso_info));
	});
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);

	m_splitsDB = new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID};
	appThreadManager()->runAction(m_splitsDB, ThreadManager::CreateTable);
}

int DBMesocyclesModel::exportToFile_splitData(const uint meso_idx, QFile *mesoFile, const bool formatted) const
{
	int ret{TP_RET_CODE_EXPORT_FAILED};
	const QMap<QChar,DBSplitModel*> &split_models{m_splitModels.at(meso_idx)};
	if (!split_models.isEmpty())
	{
		for (const DBSplitModel *split_model : split_models)
		{
			if (!split_model)
				continue;
			if (!formatted)
				ret = split_model->exportToFile(QString{}, mesoFile);
			else
				ret = split_model->exportToFormattedFile(QString{}, mesoFile);
		}
	}
	return ret;
}
