#include "dbmesocyclesmodel.h"

#include "dbcalendarmodel.h"
#include "dbmesocyclestable.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendartable.h"
#include "dbusermodel.h"
#include "dbworkoutsorsplitstable.h"
#include "homepagemesomodel.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "return_codes.h"
#include "thread_manager.h"
#include "tpfilepath.h"
#include "tpsettings.h"
#include "translationclass.h"
#include "online_services/tponlineservices.h"

#include <QQuickItem>

#include <chrono>
#include <ranges>
#include <thread>

constexpr QLatin1StringView mesosViewIdxSetting{"mesosViewIdx"};

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: QObject{parent}
{
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBMesocyclesModel::labelChanged);
	uint view_index{0};
	if (appUserModel()->isClient(0)) {
		m_mesosHomePageModel[MT_MESO_FROM_COACH] = new HomePageMesoModel{this, MT_MESO_FROM_COACH, view_index++};
		m_mesosHomePageModel[MT_MESO_FOR_SELF] = new HomePageMesoModel{this, MT_MESO_FOR_SELF, view_index++};
	}
	if (appUserModel()->isCoach(0))
		m_mesosHomePageModel[MT_MESO_FOR_CLIENT] = new HomePageMesoModel{this, MT_MESO_FOR_CLIENT, view_index};
	for (uint i{0}; i < MT_TYPE_COUNT; ++i) {
		if (m_mesosHomePageModel[i])
			connect(m_mesosHomePageModel[i], &HomePageMesoModel::currentIndexChanged, this, [this,i] () {
				setWorkingCalendar(m_mesosHomePageModel[i]->currentMesoIdx());
			});
	}
	getAllMesocycles();
}

QMLMesoInterface *DBMesocyclesModel::mesoManager(const uint meso_idx)
{
	QMLMesoInterface *mesomanager{m_mesoManagerList.value(meso_idx)};
	if (!mesomanager) {
		mesomanager = new QMLMesoInterface{this, meso_idx};
		m_mesoManagerList[meso_idx] = mesomanager;
	}
	return mesomanager;
}

void DBMesocyclesModel::removeMesoManager(const uint meso_idx)
{
	QMLMesoInterface* mesomanager{m_mesoManagerList.value(meso_idx)};
	if (mesomanager) {
		delete mesomanager;
		m_mesoManagerList.remove(meso_idx);
		for (const auto mesomanager : m_mesoManagerList | std::views::drop(meso_idx))
			mesomanager->setMesoIdx(mesomanager->mesoIdx() - 1);
	}
}

void DBMesocyclesModel::getMesocyclePage(const uint meso_idx, const bool new_meso)
{
	m_mesosHomePageModel[mesoType(meso_idx)]->setCurrentIndexViaMesoIdx(meso_idx);
	if (meso_idx < m_mesoData.count())
		mesoManager(meso_idx)->getMesocyclePage(new_meso);
}

void DBMesocyclesModel::startNewMesocycle(const MesoType type)
{
	const uint meso_idx{newMesoData(std::move(QStringList{std::move(appUtils()->newDBTemporaryId()), QString{}, QString{},
		QString{}, QString{}, QString{}, std::move("RRRRRRR"_L1), QString{}, QString{}, QString{}, QString{},
		QString{}, QString{}, appUserModel()->userId(0), (type == MT_MESO_FOR_SELF ? appUserModel()->userId(0) : QString{}),
		QString{}, std::move("1"_L1)}))};
	addSubMesoModel(meso_idx);
	static_cast<void>(mesoManager(meso_idx));
	getMesocyclePage(meso_idx, true);
}

void DBMesocyclesModel::removeMesocycle(const uint meso_idx)
{
	if (meso_idx >= m_mesoData.count())
		return;

	m_dbModelInterface->setRemovalInfo(meso_idx, QList<uint>{1, MESO_FIELD_ID});
	appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords);
	removeSplitsForMeso(meso_idx);
	removeCalendarForMeso(meso_idx, false);
	removeMesoManager(meso_idx);
	m_metadata.remove(meso_idx);
	removeMesoFiles(meso_idx);
	static_cast<void>(QFile::remove(instructionsFile(meso_idx)));
	m_mesosHomePageModel[mesoType(meso_idx)]->removeMesoIdx(meso_idx);
	m_mesoData.remove(meso_idx);
}

void DBMesocyclesModel::getExercisesPlannerPage(const uint meso_idx)
{
	mesoManager(meso_idx)->getExercisesPlannerPage();
}

void DBMesocyclesModel::getMesoCalendarPage(const uint meso_idx)
{
	getCalendarForMeso(meso_idx);
	mesoManager(meso_idx)->getCalendarPage();
}

HomePageMesoModel *DBMesocyclesModel::homePageViewModelViaIndex(const int view_index) const
{
	if (view_index >= 0) {
		for (uint i{0}; i < MT_TYPE_COUNT; ++i) {
			if (m_mesosHomePageModel[i] && m_mesosHomePageModel[i]->viewIndex() == view_index)
				return m_mesosHomePageModel[i];
		}
	}
	return nullptr;
}

int DBMesocyclesModel::currentMesosView() const
{
	int default_view{appUserModel()->mainUserConfigured()
									? (appUserModel()->onlineAccount(0)
										? (appUserModel()->mainUserIsCoach()
												? static_cast<int>(homePageViewModel(MT_MESO_FOR_CLIENT)->viewIndex())
												: static_cast<int>(homePageViewModel(MT_MESO_FROM_COACH)->viewIndex()))
										: static_cast<int>(homePageViewModel(MT_MESO_FOR_SELF)->viewIndex()))
									: -1};
	return appSettings()->getCustomValue(mesosViewIdxSetting, default_view).toInt();
}

void DBMesocyclesModel::setCurrentMesosView(const int view_index)
{
	HomePageMesoModel *view_model{homePageViewModelViaIndex(view_index)};
	if (view_model) {
		const auto new_working_meso{view_model->currentMesoIdx()};
		setWorkingCalendar(new_working_meso);
		appSettings()->setCustomValue(mesosViewIdxSetting, view_model->viewIndex());
	}
}

bool DBMesocyclesModel::isMesoOK(const int meso_idx) const
{
	if (meso_idx >= 0 && meso_idx < m_metadata.count()) {
		const uint ok_flags{static_cast<uint>(std::pow(2, static_cast<int>(MD_NAME_OK))) |
			static_cast<uint>(std::pow(2, static_cast<int>(MD_STARTDATE_OK))) |
			static_cast<uint>(std::pow(2, static_cast<int>(MD_ENDDATE_OK))) |
			static_cast<uint>(std::pow(2, static_cast<int>(MD_SPLIT_OK)))};
		return (m_metadata.at(meso_idx) & ok_flags) == ok_flags;
	}
	return false;
}

void DBMesocyclesModel::setModified(const uint meso_idx, const MesoFields field)
{
	emit mesoChanged(meso_idx, field);
	if (field != MESO_FIELD_METADATA) {
		if (isProgramSent(meso_idx)) { //any modification after the program has been sent to the client marks the program sendable again
			unsetMetaData(meso_idx, MD_PROGRAM_SENT, false);
			m_dbModelInterface->setModified(meso_idx, MESO_FIELD_METADATA);
		}
	}
	if (isMesoOK(meso_idx)) {
		switch (field) {
		case MESO_FIELD_STARTDATE:
		case MESO_FIELD_ENDDATE:
		case MESO_FIELD_SPLIT:
			removeCalendarForMeso(meso_idx, true);
		default:
			break;
		}
		if (field >= MESO_FIELD_SPLIT && field <= MESO_FIELD_SPLITF)
			checkIfCanExport(meso_idx);
	} else {
		MetaData md_field{mesoFieldToMetadataField(field)};
		if (md_field != MD_UNUSED)
			setMetaData(meso_idx, md_field, false);
		if (_id(meso_idx) < 0) {
			//When importing, splits will be created with mesoid = -1. When we incorporate the meso, we need to update those fields
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(m_db, &TPDatabaseTable::actionFinished, this, [this,conn,meso_idx]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2) {
				if (op == ThreadManager::InsertRecords) {
					disconnect(*conn);
					if (return_value1.toBool()) {
						const QMap<QChar,DBSplitModel*> &split_models{m_splitModels.value(meso_idx)};
						for (const auto &split_letter : m_usedSplits.at(meso_idx)) {
							DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
							if (split_model) //won't be nullptr when program was imported
								split_model->setMesoId(id(meso_idx));
						}
					}
				}
			});
			m_dbModelInterface->setModified(meso_idx, -1);
			appThreadManager()->runAction(m_db, ThreadManager::InsertRecords, m_dbModelInterface);
			return;
		}
	}
	m_dbModelInterface->setModified(meso_idx, field);
	appThreadManager()->runAction(m_db, m_dbModelInterface->isModified(meso_idx, MESO_FIELD_METADATA) ?
												ThreadManager::UpdateSeveralFields : ThreadManager::UpdateOneField);
}

int DBMesocyclesModel::idxFromFieldValue(const QString &field_value, const int field) const
{
	uint meso_idx{0};
	if (field >= 0) {
		for (const QStringList &meso_data : m_mesoData) {
			if (meso_data.at(field) == field_value)
				return meso_idx;
			++meso_idx;
		}
	} else {
		for (const QStringList &meso_data : m_mesoData) {
			const auto &meso_itr{std::find_if(meso_data.cbegin(), meso_data.cend(), [field_value] (const QString &meso_value) {
				return meso_value == field_value;
			})};
			if (meso_itr != meso_data.cend())
				return meso_idx;
			++meso_idx;
		}
	}
	return -1;
}

void DBMesocyclesModel::setName(const uint meso_idx, const QString &new_name)
{
	m_mesoData[meso_idx][MESO_FIELD_NAME] = new_name;
	setModified(meso_idx, MESO_FIELD_NAME);
}

void DBMesocyclesModel::setStartDate(const uint meso_idx, const QDate &new_date)
{
	m_mesoData[meso_idx][MESO_FIELD_STARTDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESO_FIELD_STARTDATE);
}

void DBMesocyclesModel::setEndDate(const uint meso_idx, const QDate &new_date)
{
	m_mesoData[meso_idx][MESO_FIELD_ENDDATE] = std::move(QString::number(new_date.toJulianDay()));
	setModified(meso_idx, MESO_FIELD_ENDDATE);
}

void DBMesocyclesModel::setSplit(const uint meso_idx, const QString &new_split)
{
	if (new_split != split(meso_idx)) {
		m_mesoData[meso_idx][MESO_FIELD_SPLIT] = new_split;
		setModified(meso_idx, MESO_FIELD_SPLIT);
		makeUsedSplits(meso_idx);
	}
}

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar &splitLetter) const
{
	return !splitLetter.isNull() && splitLetter != 'R' ?
		m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITA + static_cast<int>(splitLetter.cell()) - static_cast<int>('A')) :
		splitR();
}
void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue)
{
	const int split_col{MESO_FIELD_SPLITA + static_cast<int>(splitLetter.cell()) - static_cast<int>('A')};
	m_mesoData[meso_idx][split_col] = newSplitValue;
	setModified(meso_idx, static_cast<MesoFields>(split_col));
}

void DBMesocyclesModel::setCoach(const uint meso_idx, const QString &new_coach)
{
	m_mesoData[meso_idx][MESO_FIELD_COACH] = new_coach;
	setModified(meso_idx, MESO_FIELD_COACH);
}

void DBMesocyclesModel::setClient(const uint meso_idx, const QString &new_client)
{
	m_mesoData[meso_idx][MESO_FIELD_CLIENT] = new_client;
	setModified(meso_idx, MESO_FIELD_CLIENT);
}

void DBMesocyclesModel::setInstructionsFile(const uint meso_idx, const QString &new_file)
{
	if (m_mesoData.at(meso_idx).at(MESO_FIELD_INSTRUCTIONS_FILE) != new_file) {
		static_cast<void>(QFile::remove(m_mesoData.at(meso_idx).at(MESO_FIELD_INSTRUCTIONS_FILE)));
		m_mesoData[meso_idx][MESO_FIELD_INSTRUCTIONS_FILE] = new_file;
		setModified(meso_idx, MESO_FIELD_INSTRUCTIONS_FILE);
	}
}

QString DBMesocyclesModel::description(const uint meso_idx, const int field) const
{
	if (field < 0)
		return m_mesoData.at(meso_idx).at(MESO_FIELD_DESCRIPTION);
	else
		return appUtils()->getCompositeValue(field, m_mesoData.at(meso_idx).at(MESO_FIELD_DESCRIPTION), record_separator);
}

void DBMesocyclesModel::setDescription(const uint meso_idx, const QString &field_info, const int field)
{
	if (field < 0)
		m_mesoData[meso_idx][MESO_FIELD_DESCRIPTION] = field_info;
	else
		appUtils()->setCompositeValue(field, field_info, m_mesoData[meso_idx][MESO_FIELD_DESCRIPTION], record_separator);
	setModified(meso_idx, MESO_FIELD_DESCRIPTION);
}

DBMesocyclesModel::MesoType DBMesocyclesModel::mesoType(const uint meso_idx) const
{
	if (client(meso_idx) != appUserModel()->userId(0))
		return MT_MESO_FOR_CLIENT;
	else
		return coach(meso_idx) != appUserModel()->userId(0) ? MT_MESO_FROM_COACH : MT_MESO_FOR_SELF;
}

void DBMesocyclesModel::removeSplitsForMeso(const uint meso_idx)
{
	DBSplitModel *split_model{splitModel(meso_idx, 'A')};
	if (split_model) {
		split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{1, DBExercisesModel::EXERCISES_FIELD_MESOID});
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_splitsDB, &DBWorkoutsOrSplitsTable::dbOperationsFinished, this, [this,conn,meso_idx]
															(const ThreadManager::StandardOps op, const bool success) {
			if (op == ThreadManager::DeleteRecords && success) {
				disconnect(*conn);
				qDeleteAll(m_splitModels.value(meso_idx));
				m_splitModels.remove(meso_idx);
				for (const QMap<QChar,DBSplitModel*> &split_model_list : std::as_const(m_splitModels) | std::views::drop(meso_idx)) {
					for (DBSplitModel *split_model : split_model_list)
						split_model->setMesoIdx(split_model->mesoIdx() - 1);
				}
			}
		});
		appThreadManager()->runAction(m_splitsDB, ThreadManager::DeleteRecords);
	}
}

void DBMesocyclesModel::makeUsedSplits(const uint meso_idx)
{
	QString *usedSplit{&(m_usedSplits[meso_idx])};
	usedSplit->clear();
	const QString &strSplit{split(meso_idx)};
	for (const auto &split_letter : strSplit) {
		if (split_letter != 'R' && !usedSplit->contains(split_letter))
			usedSplit->append(split_letter);
	}
	emit usedSplitsChanged(meso_idx);
}

void DBMesocyclesModel::loadSplits(const uint meso_idx)
{
	for (const auto &split_letter : m_usedSplits.at(meso_idx)) {
		loadSplit(meso_idx, split_letter);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void DBMesocyclesModel::loadSplit(const uint meso_idx, const QChar &splitletter)
{
	DBSplitModel *split_model{splitModel(meso_idx, splitletter)};
	if (!split_model) {
		split_model = new DBSplitModel{this, m_splitsDB, meso_idx, splitletter, true};
		connect(split_model, &DBSplitModel::exerciseCountChanged, this, [this,split_model,meso_idx] () {
			emit splitLoaded(meso_idx, split_model->splitLetter());
		}, Qt::SingleShotConnection);
		m_splitModels[meso_idx].insert(splitletter, split_model);
	} else {
		emit splitLoaded(meso_idx, split_model->splitLetter());
	}
}

void DBMesocyclesModel::removeSplit(const uint meso_idx, const QChar &split_letter)
{
	DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
	split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{2} << DBExercisesModel::EXERCISES_FIELD_MESOID << DBExercisesModel::EXERCISES_FIELD_SPLITLETTER);
	appThreadManager()->runAction(m_splitsDB, ThreadManager::DeleteRecords);
	m_splitModels[meso_idx].remove(split_letter);
}

int DBMesocyclesModel::mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const
{
	if (!mesoName.isEmpty()) {
		int meso_idx{0};
		for (const QStringList &modeldata : m_mesoData) {
			if (modeldata.at(MESO_FIELD_NAME) == mesoName) {
				if (modeldata.at(MESO_FIELD_COACH) == coach)
					return meso_idx;
			}
			++meso_idx;
		}
	}
	return -1;
}

bool DBMesocyclesModel::isDateWithinMeso(const int meso_idx, const QDate &date) const
{
	if (meso_idx >= 0 && count() > 0) {
		if (date >= startDate(meso_idx))
			return date <= endDate(meso_idx);
	}
	return false;
}

int DBMesocyclesModel::getPreviousMesoId(const QString &userid, const int current_mesoid) const
{
	int meso_idx{static_cast<int>(count()-1)};
	for (; meso_idx >= 0; --meso_idx) {
		if (client(meso_idx) == userid)
			if (_id(meso_idx) >= 0 && _id(meso_idx) < current_mesoid)
				break;
	}
	return meso_idx >= 0 ? _id(meso_idx) : -1;
}

QDate DBMesocyclesModel::getMesoMinimumStartDate(const QString &userid, const uint exclude_idx) const
{
	int meso_idx{static_cast<int>(count()-1)};
	for (; meso_idx >= 0; --meso_idx) {
		if (meso_idx != exclude_idx) {
			if (client(meso_idx) == userid)
				if (!isMesoTemporary(meso_idx) && isRealMeso(meso_idx))
					break;
		}
	}
	return meso_idx >= 0 ? endDate(meso_idx) : appUtils()->createDate(0, -3, 0);
}

void DBMesocyclesModel::removeCalendarForMeso(const uint meso_idx, const bool remake_calendar)
{
	if (_id(meso_idx) >= 0) {
		if (remake_calendar) {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(m_calendarDB, &TPDatabaseTable::dbOperationsFinished, this, [=,this]
															(const ThreadManager::StandardOps op, const bool success) {
				if (op == ThreadManager::CustomOperation && success) {
					delete m_calendars.value(meso_idx);
					m_calendars.remove(meso_idx);
					getCalendarForMeso(meso_idx);
				}
			});
		}
		else {
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(m_workoutsDB, &TPDatabaseTable::dbOperationsFinished, this, [this,meso_idx,conn]
															(const ThreadManager::StandardOps op, const bool success) {
				if (op == ThreadManager::CustomOperation && success) {
					delete m_calendars.value(meso_idx);
					m_calendars.remove(meso_idx);
					qDeleteAll(m_workouts.value(meso_idx));
					m_workouts.remove(meso_idx);
					uint i{meso_idx};
					for (const auto calendar : std::as_const(m_calendars) | std::views::drop(meso_idx)) {
						calendar->setMesoIdx(i);
						const QMap<uint,DBExercisesModel*> &meso_workouts{m_workouts.value(i)};
						for (DBExercisesModel *workout: std::as_const(meso_workouts))
							workout->setMesoIdx(i);
						i++;
					}
				}
			});
		}
		auto x = [this,meso_idx] (DBModelInterface *) -> std::pair<QVariant,QVariant> {
				return m_calendarDB->removeMesoCalendar(id(meso_idx));
		};
		m_calendarDB->setCustomQueryFunction(x);
		appThreadManager()->runAction(m_calendarDB, ThreadManager::CustomOperation);

		if (!remake_calendar) {
			auto y = [this,meso_idx] (DBModelInterface*) -> std::pair<QVariant,QVariant> {
					return m_workoutsDB->removeAllMesoWorkouts(id(meso_idx));
			};
			m_workoutsDB->setCustomQueryFunction(y);
			appThreadManager()->runAction(m_workoutsDB, ThreadManager::CustomOperation);
		}
	}
}

void DBMesocyclesModel::getCalendarForMeso(const uint meso_idx)
{
	DBCalendarModel *model{m_calendars.value(meso_idx)};
	if (!model) {
		model = new DBCalendarModel{this, m_calendarDB, meso_idx};
		connect(model, &DBCalendarModel::calendarLoaded, this, [this, meso_idx] (const bool success) {
			if (!success)
				getCalendarForMeso(meso_idx);
			else {
				setWorkingCalendar(meso_idx);
				emit calendarReady(meso_idx);
			}
		}, Qt::SingleShotConnection);
		m_calendars.insert(meso_idx, model);
		return;
	}
	if (model->nMonths() == -1) {
		setWorkingCalendar(meso_idx);
		const uint n_months{populateCalendarDays(meso_idx)};
		model->setNMonths(n_months);
		appThreadManager()->runAction(m_calendarDB, ThreadManager::InsertRecords);
	}
	emit calendarReady(meso_idx);
}

uint DBMesocyclesModel::populateCalendarDays(const uint meso_idx)
{
	const QString &split{this->DBMesocyclesModel::split(meso_idx)};
	QString::const_iterator splitletter{split.constBegin()};
	QDate day_date{startDate(meso_idx)};
	const qsizetype n_days{day_date.daysTo(endDate(meso_idx))};
	uint workout_number{1};
	for (uint i{0}; i < n_days; ++i) {
		QStringList day_info{DBMesoCalendarTable::CALDB_TOTAL_FIELDS};
		day_info[DBMesoCalendarTable::CALDB_MESOID] = id(meso_idx);
		day_info[DBMesoCalendarTable::CALDB_DATE] = std::move(appUtils()->formatDate(day_date, TPUtils::DF_DATABASE));
		day_info[DBMesoCalendarTable::CALDB_DATA] = std::move(appUtils()->string_strings({id(meso_idx), QString{},
				day_info.at(DBMesoCalendarTable::CALDB_DATE), *splitletter != 'R' ? QString::number(workout_number++)
					: QString{}, *splitletter, QString{}, QString{}, QString{}, QString{}, "0"_L1}, record_separator));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
		m_workingCalendar->dbModelInterface()->modelData().append(std::move(day_info));
		m_workingCalendar->dbModelInterface()->setModified(i, -1);
		day_date = std::move(day_date.addDays(1));
	}
	return appUtils()->calculateNumberOfMonths(startDate(meso_idx), day_date);
}

void DBMesocyclesModel::setWorkingCalendar(const uint meso_idx)
{
	m_workingCalendar = m_calendars.value(meso_idx);
	if (m_workingCalendar) {
		DBExercisesModel *workout{workingWorkout()};
		if (!workout) {
			const QDate &cur_date{QDate::currentDate()};
			if (isDateWithinMeso(meso_idx, cur_date)) {
				m_workingCalendar->setCurrentDate(cur_date);
				workout = workoutForDay(workout, meso_idx, calendar(meso_idx)->calendarDay(cur_date));
			}
		}
		if (workout)
			setWorkingWorkout(meso_idx, workout);
	}
}

DBExercisesModel *DBMesocyclesModel::workingWorkout() const
{
	return m_workingWorkouts.value(m_workingCalendar->mesoIdx());
}

void DBMesocyclesModel::openSpecificWorkout(const uint meso_idx, const QDate &date)
{
	connect(this, &DBMesocyclesModel::calendarReady, [this,date] (const uint meso_idx) {
		mesoManager(meso_idx)->getWorkoutPage(date);
	});
	getCalendarForMeso(meso_idx);
}

void DBMesocyclesModel::setWorkingWorkout(const uint meso_idx, DBExercisesModel* model)
{
	m_workingWorkouts.insertOrAssign(meso_idx, model);
}

DBExercisesModel *DBMesocyclesModel::workoutForDay(DBExercisesModel *w_model, const uint meso_idx, const int calendar_day)
{
	if (!w_model) {
		w_model = m_workouts.value(meso_idx).value(calendar_day);
		if (!w_model)
			w_model = new DBExercisesModel{this, m_workoutsDB, meso_idx, calendar_day};
	}
	QMap<uint,DBExercisesModel*> workouts_for_meso;
	workouts_for_meso.insert(calendar_day, w_model);
	m_workouts.insert(meso_idx, workouts_for_meso);
	return w_model;
}

void DBMesocyclesModel::newWorkoutFromFile(const TPFilePath &filename, const bool formatted, const uint meso_idx,
																		const int calendar_day, const QChar &splitletter)
{
	DBCalendarModel *cal{calendar(meso_idx)};
	DBExercisesModel *workout{new DBExercisesModel(this, m_workoutsDB, meso_idx, calendar_day)};
	if (formatted) {
		const auto ret{workout->importFromFormattedFile(filename, false)};
		if (ret == TP_RET_CODE_IMPORT_OK) {
			connect(workout, &DBExercisesModel::workoutIncorporated, this, [=,this] (const bool success) {
				if (success)
					static_cast<void>(workoutForDay(workout, meso_idx, -1));
				emit workoutImported(success ? TP_RET_CODE_IMPORT_OK : TP_RET_CODE_IMPORT_FAILED, success
					? tr("Extra workout is set to happen on ") % appUtils()->formatDate(cal->date(workout->calendarDay()))
					: filename.fileName());
			});
			QMLMesoInterface *mesomanager{m_mesoManagerList.value(meso_idx)};
			workout->incorporateIntoCalendar(cal, mesomanager ? mesomanager->qmlPage() : appItemManager()->appHomePage());
		} else {
			emit workoutImported(TP_RET_CODE_IMPORT_FAILED, filename.fileName());
		}
	}
	else {
		if (workout->importFromFile(filename) == TP_RET_CODE_IMPORT_OK)
			static_cast<void>(workoutForDay(workout, meso_idx, -1));
		else
			qCritical() << "Failed to import workout from file"_L1 << filename.toString();
	}
}

void DBMesocyclesModel::checkIfCanExport(const uint meso_idx, const bool emit_signal)
{
	if (isMesoOK(meso_idx)) {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_splitsDB, &TPDatabaseTable::actionFinished, this, [this,conn,meso_idx,emit_signal]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2) {
			if (op == ThreadManager::CustomOperation) {
				disconnect(*conn);
				const bool can_export{return_value2.toBool()};
				if (canExport(meso_idx) != can_export) {
					if (can_export)
						setMetaData(meso_idx, MD_CAN_EXPORT);
					else
						unsetMetaData(meso_idx, MD_CAN_EXPORT);
					if (emit_signal)
						emit canExportChanged(meso_idx, can_export);
				}
			}
		});
		auto x = [this,meso_idx] (DBModelInterface*) -> std::pair<QVariant,QVariant> {
				return m_splitsDB->mesoHasAllSplitPlans(id(meso_idx), usedSplits(meso_idx));
		};
		m_splitsDB->setCustomQueryFunction(x);
		appThreadManager()->runAction(m_splitsDB, ThreadManager::CustomOperation);
	}
}

void DBMesocyclesModel::exportToFile(const uint meso_idx, const TPFilePath &filename)
{
	const QList<uint> export_row{meso_idx};
	const auto ret{appUtils()->writeDataToFile(filename.toString(), appUtils()->mesoFileIdentifier, m_mesoData, export_row)};
	if (ret == TP_RET_CODE_EXPORT_OK)
		exportToFile_splitData(meso_idx, filename, false);
	else
		emit mesoExported(meso_idx, filename, ret);
}

void DBMesocyclesModel::exportToFormattedFile(const uint meso_idx, const TPFilePath &filename)
{
	const QList<uint> export_row{meso_idx};
	const QList<std::function<QString(void)>> &field_description{
										nullptr, //do not include the id field
										std::move([this] () { return mesoNameLabel(); }),
										std::move([this] () { return startDateLabel(); }),
										std::move([this] () { return endDateLabel(); }),
										std::move([this] () { return notesLabel(); }),
										std::move([this] () { return nWeeksLabel(); }),
										std::move([this] () { return splitLabel(); }),
										std::move([this] () { return splitLabelA(); }),
										std::move([this] () { return splitLabelB(); }),
										std::move([this] () { return splitLabelC(); }),
										std::move([this] () { return splitLabelD(); }),
										std::move([this] () { return splitLabelE(); }),
										std::move([this] () { return splitLabelF(); }),
										std::move([this] () { return coachLabel(); }),
										std::move([this] () { return clientLabel(); }),
										std::move([this] () { return fileLabel(); }),
										std::move([this] () { return metadataLabel(); }),
										std::move([this] () { return typeLabel(); })
	};

	const auto ret{appUtils()->writeDataToFormattedFile(filename.toString(),
					appUtils()->mesoFileIdentifier,
					m_mesoData,
					field_description,
					[this] (const uint field, const QString &value) { return formatFieldToExport(field, value); },
					export_row,
					QString{tr("Exercises Program")})
	};
	if (ret == TP_RET_CODE_EXPORT_OK)
		exportToFile_splitData(meso_idx, filename, true);
	else
		emit mesoExported(meso_idx, filename, TP_RET_CODE_EXPORT_FAILED);
	return;
}

int DBMesocyclesModel::importFromFile(const uint meso_idx, const TPFilePath &filename, const bool formatted)
{
	const auto ret{formatted
					? appUtils()->readDataFromFormattedFile(
					   filename.toString(),
					   m_mesoData,
					   fieldCount(),
					   appUtils()->mesoFileIdentifier,
					   [this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
					: appUtils()->readDataFromFile(
						filename.toString(),
						m_mesoData,
						fieldCount(),
						appUtils()->mesoFileIdentifier, meso_idx)};

	if (ret == TP_RET_CODE_IMPORT_OK) {
		setId(meso_idx, appUtils()->newDBTemporaryId());
		makeUsedSplits(meso_idx);
		auto n_splits{m_usedSplits.at(meso_idx).length()};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(this, &DBMesocyclesModel::splitLoaded, this, [=,this]
											(const uint _meso_idx, const QChar &splitletter) mutable -> void {
			if (meso_idx == _meso_idx) {
				DBSplitModel *split_model{splitModel(meso_idx, splitletter)};
				int ret{TP_RET_CODE_IMPORT_OK};
				if (split_model->exerciseCount() == 0) { //only import into an empty model
					if (formatted)
						ret = split_model->importFromFormattedFile(filename, true);
					else
						ret = split_model->importFromFile(filename);
#ifndef QT_NO_DEBUG
					qInfo() << "Split "_L1 << splitletter << (ret == TP_RET_CODE_SUCCESS ? QString{
						" successfully imported"_L1} : QString{" failed to import with error "_L1 % QString::number(ret)});
#endif
				}
				if (--n_splits == 0) {
					emit splitsImported(meso_idx, ret);
					disconnect(*conn);
				}
			}
		});
		loadSplits(meso_idx);
	}
	return ret;
}

TPFilePathPtr DBMesocyclesModel::suggestedName(const int meso_idx, const bool external_filename) const
{
	QString userid;
	const MesoType mt{mesoType(meso_idx)};
	switch (mt) {
		case MT_MESO_FOR_CLIENT:	userid = std::move(client(meso_idx)); break;
		case MT_MESO_FROM_COACH:	userid = std::move(coach(meso_idx)); break;
		case MT_MESO_FOR_SELF:		userid = appUserModel()->userId(0); break;
		default: Q_UNREACHABLE();
	}
	return TPFilePath::newTPFilePath(name(meso_idx) % (!external_filename ? TPUtils::TP_FILE_EXTENSION : QString{}),
																	appUserModel()->userId(0), userid, {mesos_subdir});
}

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field) {
	case MESO_FIELD_STARTDATE:
	case MESO_FIELD_ENDDATE:
		return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
	case MESO_FIELD_DESCRIPTION:
		return QString{fieldValue}.replace(record_separator, fancy_record_separator1);
	}
	return fieldValue;
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field) {
	case MESO_FIELD_STARTDATE:
	case MESO_FIELD_ENDDATE:
		return QString::number(appUtils()->dateFromString(fieldValue).toJulianDay());
	case MESO_FIELD_DESCRIPTION:
		return QString{fieldValue}.replace(fancy_record_separator1, record_separator);
	}
	return fieldValue;
}

void DBMesocyclesModel::removeMesoFiles(const uint meso_idx)
{
	auto meso_filename{suggestedName(meso_idx)};
	appOnlineServices()->removeFileFromServer(*meso_filename);
	static_cast<void>(QFile::remove(meso_filename->toString()));
	static_cast<void>(QFile::remove(instructionsFile(meso_idx)));
}

void DBMesocyclesModel::newMesoFromFile(const TPFilePath &filename, const std::optional<bool> &file_formatted)
{
	auto meso_idx{newMesoData(std::move(QStringList{MESO_TOTAL_FIELDS}))};
	m_mesoData.remove(meso_idx);
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value()) {
		import_result = importFromFile(meso_idx, filename, file_formatted.value());
	} else {
		import_result = importFromFile(meso_idx, filename, false);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFile(meso_idx, filename, true);
	}
	if (import_result != TP_RET_CODE_IMPORT_OK) {
		removeMesocycle(meso_idx);
		emit mesoImported(import_result);
	} else {
		connect(this, &DBMesocyclesModel::splitsImported, this, [this,meso_idx,filename] (const uint _meso_idx, const int ret_code) {
			setModified(meso_idx, MESO_TOTAL_FIELDS); //save program and update mesoid in all the imported splits
			emit mesoImported(ret_code, TP_RET_CODE_IMPORT_OK ? tr("Training program ") % name(meso_idx) % tr(" from ")
												% appUserModel()->userNameFromId(coach(meso_idx)) : filename.fileName());
		}, Qt::SingleShotConnection);
		const auto plan_idx{mesoPlanExists(name(meso_idx), coach(meso_idx), client(meso_idx))};
		const bool existing_meso{plan_idx != -1 && plan_idx != meso_idx};
		if (existing_meso) {
			m_mesoData.swapItemsAt(meso_idx, plan_idx);
			removeMesocycle(meso_idx);
			meso_idx = plan_idx;
			m_dbModelInterface->setModified(meso_idx, -1);
			appThreadManager()->runAction(m_db, ThreadManager::UpdateSeveralFields, m_dbModelInterface);
		}
		addSubMesoModel(meso_idx);
		checkIfCanExport(meso_idx);
		QMLMesoInterface *mesomanager{m_mesoManagerList.value(meso_idx)};
		if (mesomanager)
			mesomanager->updateInterface();
		else
			static_cast<void>(mesoManager(meso_idx));
	}
}

inline void DBMesocyclesModel::addSubMesoModel(const uint meso_idx)
{
	m_mesosHomePageModel[mesoType(meso_idx)]->appendMesoIdx(meso_idx);
}

const uint DBMesocyclesModel::newMesoData(QStringList &&infolist)
{
	const uint meso_idx{count()};
	m_mesoData.append(std::move(infolist));
	m_metadata.append(metadata(meso_idx).toUInt());
	m_usedSplits.append(QString{});
	makeUsedSplits(meso_idx);
	return meso_idx;
}

void DBMesocyclesModel::getAllMesocycles()
{
	m_dbModelInterface = new DBModelInterfaceMesocycle{this};
	m_db = new DBMesocyclesTable{};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(m_db, &DBMesocyclesTable::mesocycleAcquired, this, [this,conn] (QStringList meso_info, const bool last_meso) {
		if (!last_meso) {
			const uint meso_idx{newMesoData(std::move(meso_info))};
			addSubMesoModel(meso_idx);
			checkIfCanExport(meso_idx);
		} else {
			disconnect(*conn);
			#ifndef QT_NO_DEBUG
			emit mesoDataLoaded();
			#endif
		}
	});
	appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	m_splitsDB = new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID};
	appThreadManager()->runAction(m_splitsDB, ThreadManager::CreateTable);
	m_calendarDB = new DBMesoCalendarTable{};
	appThreadManager()->runAction(m_calendarDB, ThreadManager::CreateTable);
	m_workoutsDB = new DBWorkoutsOrSplitsTable{WORKOUT_TABLE_ID};
	appThreadManager()->runAction(m_workoutsDB, ThreadManager::CreateTable);
}

void DBMesocyclesModel::exportToFile_splitData(const uint meso_idx, const TPFilePath &filename, const bool formatted)
{
	auto n_conns{usedSplits(meso_idx).length()};
	auto conn{std::make_shared<QMetaObject::Connection>()};
	*conn = connect(this, &DBMesocyclesModel::splitLoaded, this, [this,conn,n_conns,formatted,filename]
																(const uint meso_idx, const QChar &splitletter) mutable {
		int ret;
		if (!formatted)
			ret = splitModel(meso_idx, splitletter)->exportToFile(filename);
		else
			ret = splitModel(meso_idx, splitletter)->exportToFormattedFile(filename);

		if (--n_conns == 0 || ret != TP_RET_CODE_EXPORT_OK) {
			disconnect(*conn);
			emit mesoExported(meso_idx, filename, ret);
			if (ret != TP_RET_CODE_EXPORT_OK)
				static_cast<void>(QFile::remove(filename.toString()));
		}
	});
	loadSplits(meso_idx);
}
