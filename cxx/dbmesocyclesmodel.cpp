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
#include "thread_manager.h"
#include "tpsettings.h"
#include "translationclass.h"

#include <QQuickItem>

#include <ranges>

constexpr QLatin1StringView mesosViewIdxSetting{"mesosViewIdx"};

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: QObject{parent}, m_currentWorkingMeso{-1}
{
	if (appUserModel()->mainUserIsClient())
		m_ownMesos = new HomePageMesoModel{this, true};
	if (appUserModel()->mainUserIsCoach())
		m_clientMesos = new HomePageMesoModel{this, false};

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBMesocyclesModel::labelChanged);
	connect(this, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field)
	{
		if (isMesoOK(meso_idx))
		{
			m_dbModelInterface->setModified(meso_idx, field);
			appThreadManager()->runAction(m_db, ThreadManager::UpdateOneField);
			switch (field)
			{
				case MESO_FIELD_STARTDATE:
				case MESO_FIELD_ENDDATE:
				case MESO_FIELD_SPLIT:
					removeCalendarForMeso(meso_idx, true);
				break;
			}
			if (field >= MESO_FIELD_SPLIT && field <= MESO_FIELD_SPLITF)
				checkIfCanExport(meso_idx);
		}
		else
			static_cast<void>(exportToFile(meso_idx, mesoFileName(meso_idx), false)); //temporary new mesocycles
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
		for (const auto mesomanager : m_mesoManagerList | std::views::drop(meso_idx))
			mesomanager->setMesoIdx(mesomanager->mesoIdx() - 1);
	}
}

void DBMesocyclesModel::incorporateMeso(const uint meso_idx)
{
	m_dbModelInterface->setModified(meso_idx, 0);
	appThreadManager()->runAction(m_db, ThreadManager::InsertRecords);
	getCalendarForMeso(meso_idx);
	removeMesoFile(meso_idx);
}

void DBMesocyclesModel::getMesocyclePage(const uint meso_idx, const bool new_meso)
{
	isOwnMeso(meso_idx) ? m_ownMesos->setCurrentIndexViaMesoIdx(meso_idx) : m_clientMesos->setCurrentIndexViaMesoIdx(meso_idx);
	if (meso_idx < m_mesoData.count())
		mesoManager(meso_idx)->getMesocyclePage(new_meso);
}

void DBMesocyclesModel::startNewMesocycle(const bool own_meso)
{
	const uint meso_idx{newMesoData(std::move(QStringList{std::move(appUtils()->newDBTemporaryId()), QString{}, QString{}, QString{},
		QString{}, QString{}, std::move("RRRRRRR"_L1), QString{}, QString{}, QString{}, QString{}, QString{}, QString{},
		appUserModel()->userId(0), (own_meso ? appUserModel()->userId(0) : appUserModel()->mostRecentClientId()), QString{},
		QString{}, std::move("1"_L1)}))};

	addSubMesoModel(meso_idx, own_meso);
	for (uint8_t i{0}; i < MESO_N_REQUIRED_FIELDS; ++i)
		setBit(m_isMesoOK[meso_idx], meso_required_fields[i]);

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

	m_isMesoOK.remove(meso_idx);
	m_canExport.remove(meso_idx);
	removeMesoFile(meso_idx);
	if (isOwnMeso(meso_idx))
		m_ownMesos->removeMesoIdx(meso_idx);
	else
		m_clientMesos->removeMesoIdx(meso_idx);

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

void DBMesocyclesModel::openSpecificWorkout(const uint meso_idx, const QDate &date)
{
	connect(this, &DBMesocyclesModel::calendarReady, [this,date] (const uint meso_idx) {
		mesoManager(meso_idx)->getWorkoutPage(date);
	});
	getCalendarForMeso(meso_idx);
}

void DBMesocyclesModel::setCurrentMesosView(const bool own_mesos_view)
{
	int new_working_meso{own_mesos_view ? m_ownMesos->currentMesoIdx() : m_clientMesos->currentMesoIdx()};
	if (new_working_meso != m_currentWorkingMeso)
	{
		m_currentWorkingMeso = new_working_meso;
		setWorkingCalendar(m_currentWorkingMeso);
		int view_idx{-1};
		QMetaObject::invokeMethod(appItemManager()->appHomePage(), "mesosViewIndex", qReturnArg(view_idx));
		if (view_idx != -1)
			appSettings()->setCustomValue(mesosViewIdxSetting, view_idx);
	}
}

bool DBMesocyclesModel::isRequiredFieldWrong(const uint meso_idx, const uint field) const
{
	return isBitSet(m_isMesoOK.at(meso_idx), field);
}

void DBMesocyclesModel::setModified(const uint meso_idx, const uint field)
{
	if (!isMesoOK(meso_idx))
	{
		unSetBit(m_isMesoOK[meso_idx], field);
		if (isMesoOK(meso_idx))
		{
			incorporateMeso(meso_idx);
			return;
		}
	}
	emit mesoChanged(meso_idx, field);
}

int DBMesocyclesModel::idxFromId(const QString &meso_id) const
{
	uint meso_idx{0};
	const auto &meso = std::find_if(m_mesoData.cbegin(), m_mesoData.cend(), [meso_id,&meso_idx] (const auto &meso_info) {
		++meso_idx;
		return meso_info.at(MESO_FIELD_ID) == meso_id;
	});
	return meso != m_mesoData.cend() ? meso_idx : -1;
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
	if (new_split != split(meso_idx))
	{
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
	setModified(meso_idx, split_col);
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

bool DBMesocyclesModel::isOwnMeso(const uint meso_idx) const
{
	if (meso_idx < m_mesoData.count())
	{
		const QString &_client{m_mesoData.at(meso_idx).at(MESO_FIELD_CLIENT)};
		if (!_client.isEmpty())
			return m_mesoData.at(meso_idx).at(MESO_FIELD_CLIENT) == appUserModel()->userId(0);
	}
	return false;
}

void DBMesocyclesModel::addSubMesoModel(const uint meso_idx, const bool own_meso)
{
	if (own_meso)
		m_ownMesos->appendMesoIdx(meso_idx);
	else
		m_clientMesos->appendMesoIdx(meso_idx);
}

void DBMesocyclesModel::removeSplitsForMeso(const uint meso_idx)
{
	DBSplitModel *split_model{splitModel(meso_idx, 'A')};
	if (split_model)
	{
		m_splitsDB->setDBModelInterface(split_model->dbModelInterface());
		split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{1, EXERCISES_FIELD_MESOID});
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(m_splitsDB, &DBWorkoutsOrSplitsTable::dbOperationsFinished, [this,conn,meso_idx]
																	(const ThreadManager::StandardOps op, const bool success) {
			if (op == ThreadManager::DeleteRecords && success)
			{
				disconnect(*conn);
				qDeleteAll(m_splitModels.value(meso_idx));
				m_splitModels.remove(meso_idx);
				for (const QMap<QChar,DBSplitModel*> &split_model_list : std::as_const(m_splitModels) | std::views::drop(meso_idx))
				{
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
			split_model = new DBSplitModel{this, m_splitsDB, meso_idx, split_letter, true};
			m_splitModels[meso_idx].insert(split_letter, split_model);
			::usleep(1000);
		}
	}
}

void DBMesocyclesModel::removeSplit(const uint meso_idx, const QChar &split_letter)
{
	DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
	split_model->dbModelInterface()->setRemovalInfo(0, QList<uint>{2} << EXERCISES_FIELD_MESOID << EXERCISES_FIELD_SPLITLETTER);
	appThreadManager()->runAction(m_splitsDB, ThreadManager::DeleteRecords);
	m_splitModels[meso_idx].remove(split_letter);
}

int DBMesocyclesModel::mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const
{
	if (!mesoName.isEmpty())
	{
		int meso_idx{0};
		for (const QStringList &modeldata : m_mesoData)
		{
			if (modeldata.at(MESO_FIELD_NAME) == mesoName)
			{
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
			if (_id(meso_idx) >= 0 && _id(meso_idx) < current_mesoid)
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
	uint meso_idx{exclude_idx + 1};
	for (; meso_idx < count(); ++meso_idx)
	{
		if (client(meso_idx) == userid)
			if (_id(meso_idx) >= 0 && isRealMeso(meso_idx))
				break;
	}
	return meso_idx < count() ? endDate(meso_idx) : appUtils()->createDate(QDate::currentDate(), 0, 6, 0);
}

void DBMesocyclesModel::removeCalendarForMeso(const uint meso_idx, const bool remake_calendar)
{
	if (_id(meso_idx) >= 0)
	{
		if (remake_calendar)
		{
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(m_calendarDB, &TPDatabaseTable::dbOperationsFinished, [this,meso_idx,remake_calendar,conn]
																	(const ThreadManager::StandardOps op, const bool success) {
				if (op == ThreadManager::CustomOperation && success)
				{
					delete m_calendars.value(meso_idx);
					m_calendars.remove(meso_idx);
					getCalendarForMeso(meso_idx);
				}
			});
		}
		else
		{
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(m_workoutsDB, &TPDatabaseTable::dbOperationsFinished, [this,meso_idx,conn]
																	(const ThreadManager::StandardOps op, const bool success) {
				if (op == ThreadManager::CustomOperation && success)
				{
					delete m_calendars.value(meso_idx);
					m_calendars.remove(meso_idx);
					qDeleteAll(m_workouts.value(meso_idx));
					m_workouts.remove(meso_idx);
					uint i{meso_idx};
					for (const auto calendar : std::as_const(m_calendars) | std::views::drop(meso_idx))
					{
						calendar->setMesoIdx(i);
						for (const auto workout: m_workouts.value(i))
							workout->setMesoIdx(i);
						i++;
					}
				}
			});
		}
		auto x = [this,meso_idx] () -> std::pair<QVariant,QVariant> { return m_calendarDB->removeMesoCalendar(id(meso_idx)); };
		m_calendarDB->setCustQueryFunction(x);
		appThreadManager()->runAction(m_calendarDB, ThreadManager::CustomOperation);

		if (!remake_calendar)
		{
			auto y = [this,meso_idx] () -> std::pair<QVariant,QVariant> {
											return m_workoutsDB->removeAllMesoWorkouts(id(meso_idx)); };
			m_workoutsDB->setCustQueryFunction(y);
			appThreadManager()->runAction(m_workoutsDB, ThreadManager::CustomOperation);
		}
	}
}

void DBMesocyclesModel::getCalendarForMeso(const uint meso_idx)
{
	DBCalendarModel *model{m_calendars.value(meso_idx)};
	if (!model)
	{
		model = new DBCalendarModel{this, m_calendarDB, meso_idx};
		connect(model, &DBCalendarModel::calendarLoaded, this, [this, meso_idx] (const bool success) {
			if (!success)
				getCalendarForMeso(meso_idx);
			else
			{
				setWorkingCalendar(meso_idx);
				emit calendarReady(meso_idx);
			}
		}, Qt::SingleShotConnection);
		m_calendars.insert(meso_idx, model);
		return;
	}
	if (model->nMonths() == 0)
	{
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
	for (uint i{0}; i < n_days; ++i)
	{
		QStringList day_info{CALENDAR_DATABASE_TOTAL_FIELDS};
		day_info[CALENDAR_DATABASE_MESOID] = id(meso_idx);
		day_info[CALENDAR_DATABASE_DATE] = std::move(appUtils()->formatDate(day_date, TPUtils::DF_DATABASE));
		day_info[CALENDAR_DATABASE_DATA] = std::move(appUtils()->string_strings({id(meso_idx), QString{}, day_info.at(CALENDAR_DATABASE_DATE),
			*splitletter != 'R' ? QString::number(workout_number++) : QString{}, *splitletter, QString{}, QString{},
			QString{}, QString{}, "0"_L1}, record_separator));
		if (++splitletter == split.constEnd())
			splitletter = split.constBegin();
		m_workingCalendar->dbModelInterface()->modelData().append(std::move(day_info));
		m_workingCalendar->dbModelInterface()->setModified(i, 0);
		day_date = std::move(day_date.addDays(1));
	}
	return appUtils()->calculateNumberOfMonths(startDate(meso_idx), day_date);
}

void DBMesocyclesModel::setWorkingCalendar(const uint meso_idx)
{
	m_workingCalendar = m_calendars.value(meso_idx);
	if (m_workingCalendar)
	{
		m_calendarDB->setDBModelInterface(m_workingCalendar->dbModelInterface());
		DBExercisesModel *workout{workingWorkout()};
		if (!workout)
			if (isDateWithinMeso(meso_idx, QDate::currentDate()))
			{
				m_workingCalendar->setCurrentDate(QDate::currentDate());
				workout = workoutForDay(meso_idx, QDate::currentDate());
			}
		if (workout)
			setWorkingWorkout(meso_idx, workout);
	}
}

DBExercisesModel *DBMesocyclesModel::workingWorkout() const
{
	return m_workingWorkouts.value(m_workingCalendar->mesoIdx());
}

void DBMesocyclesModel::setWorkingWorkout(const uint meso_idx, DBExercisesModel* model)
{
	m_workingWorkouts.insertOrAssign(meso_idx, model);
	m_workoutsDB->setDBModelInterface(model->dbModelInterface());
}

DBExercisesModel *DBMesocyclesModel::workoutForDay(const uint meso_idx, const int calendar_day)
{
	DBExercisesModel *w_model{m_workouts.value(meso_idx).value(calendar_day)};
	if (!w_model)
	{
		w_model = new DBExercisesModel{this, m_workoutsDB, meso_idx, calendar_day};
		QMap<uint,DBExercisesModel*> workouts_for_meso;
		workouts_for_meso.insert(calendar_day, w_model);
		m_workouts.insert(meso_idx, workouts_for_meso);
	}
	return w_model;
}

void DBMesocyclesModel::checkIfCanExport(const uint meso_idx, const bool emit_signal)
{
	if (isMesoOK(meso_idx))
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

int DBMesocyclesModel::exportToFile(const uint meso_idx, const QString &filename, const bool include_splits) const
{
	QFile *out_file{appUtils()->openFile(filename, false, true, false, true)};
	if (!out_file)
		return TP_RET_CODE_OPEN_CREATE_FAILED;

	int ret{TP_RET_CODE_EXPORT_FAILED};
	QList<uint> export_row;
	export_row.append(meso_idx);
	if (appUtils()->writeDataToFile(out_file, appUtils()->mesoFileIdentifier, m_mesoData, export_row))
		ret = include_splits ? exportToFile_splitData(meso_idx, out_file, false) : TP_RET_CODE_EXPORT_OK;
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
	if (ret == TP_RET_CODE_IMPORT_OK)
	{
		setId(meso_idx, appUtils()->newDBTemporaryId());
		for (const auto &split_letter : m_usedSplits.at(meso_idx))
		{
			DBSplitModel *split_model{splitModel(meso_idx, split_letter)};
			if (!split_model)
			{
				split_model = new DBSplitModel{this, m_splitsDB, meso_idx, split_letter, false};
				m_splitModels[meso_idx].insert(split_letter, split_model);
			}
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

	int ret{appUtils()->readDataFromFormattedFile(
								in_file,
								m_mesoData,
								fieldCount(),
								appUtils()->mesoFileIdentifier,
								[this] (const uint field, const QString &value) { return formatFieldToImport(field, value); })
	};
	if (ret == TP_RET_CODE_IMPORT_OK)
	{
		setId(meso_idx, appUtils()->newDBTemporaryId());
		const QMap<QChar,DBSplitModel*> &split_models{m_splitModels.value(meso_idx)};
		for (DBSplitModel *split_model : split_models)
		{
			if (split_model)
			{
				if ((ret = split_model->importFromFormattedFile(filename, in_file)) != TP_RET_CODE_IMPORT_OK)
					break;
			}
		}
	}
	in_file->close();
	return ret;
}

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case MESO_FIELD_STARTDATE:
		case MESO_FIELD_ENDDATE:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case MESO_FIELD_COACH:
		case MESO_FIELD_CLIENT:
			return fieldValue;
		case MESO_FIELD_REALMESO:
			return fieldValue == '1' ? tr("Yes") : tr("No");
	}
	return fieldValue;
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case MESO_FIELD_STARTDATE:
		case MESO_FIELD_ENDDATE:
			return QString::number(appUtils()->dateFromString(fieldValue).toJulianDay());
		case MESO_FIELD_REALMESO:
			return fieldValue == tr("Yes") ? "1"_L1 : "0"_L1;
	}
	return fieldValue;
}

QString DBMesocyclesModel::mesoFileName(const uint meso_idx) const
{
	QString userid;
	if (client(meso_idx) != appUserModel()->userId(0))
		userid = client(meso_idx);
	else
	{
		if (coach(meso_idx) != appUserModel()->userId(0))
			userid = coach(meso_idx);
		else
			userid = appUserModel()->userId(0);
	}
	return appUserModel()->userDir(userid) % mesos_subdir % name(meso_idx) % ".txt"_L1;
}

void DBMesocyclesModel::removeMesoFile(const uint meso_idx)
{
	const QString &mesofilename{mesoFileName(meso_idx)};
	appUserModel()->removeFileFromServer(appUtils()->getFileName(mesofilename), mesos_subdir, coach(meso_idx));
	static_cast<void>(QFile::remove(mesofilename));
}

void DBMesocyclesModel::sendMesoToUser(const uint meso_idx)
{
	if (!isMesoOK(meso_idx))
		return;
	const QString &filename{mesoFileName(meso_idx)};
	const int ret{exportToFile(meso_idx, filename)};

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
						tr("Exercises Program sent to client") : QString{}, mesos_subdir + coach(meso_idx), client(meso_idx));
			}
		});
	}
	else if (ret == TP_RET_CODE_EXPORT_OK)
		appUserModel()->sendFileToServer(filename, nullptr, !isOwnMeso(meso_idx) ? tr("Exercises Program sent to client") : QString{},
										mesos_subdir + coach(meso_idx), client(meso_idx));
}

int DBMesocyclesModel::newMesoFromFile(const QString &filename, const bool own_meso, const std::optional<bool> &file_formatted)
{
	const uint meso_idx{newMesoData(std::move(QStringList{MESO_TOTAL_FIELDS}))};
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
	if (import_result != TP_RET_CODE_IMPORT_OK)
		removeMesocycle(meso_idx);
	else
	{
		uint8_t meso_required_fields{0};
		if (!isMesoNameOK(meso_idx))
			setBit(meso_required_fields, MESO_FIELD_NAME);
		if (!isStartDateOK(meso_idx))
			setBit(meso_required_fields, MESO_FIELD_STARTDATE);
		if (!isEndDateOK(meso_idx))
			setBit(meso_required_fields, MESO_FIELD_ENDDATE);
		if (!isSplitOK(meso_idx))
			setBit(meso_required_fields, MESO_FIELD_SPLIT);
		m_isMesoOK[meso_idx] = meso_required_fields;
		makeUsedSplits(meso_idx);
		addSubMesoModel(meso_idx, own_meso);
	}
	return import_result;
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

void DBMesocyclesModel::viewOnlineMeso(const QString &coach, const QString &filename)
{
	const int request_id{appUserModel()->downloadFileFromServer(filename, appUserModel()->userDir(coach) % mesos_subdir,
								QString{}, mesos_subdir + coach, appUserModel()->userId(0))};

	auto getPage = [this] (const QString &meso_file_name) ->void {
		const int meso_idx{newMesoFromFile(meso_file_name, false, true)};
		if (meso_idx >= 0)
			getMesocyclePage(meso_idx, false);
	};

	if (request_id == TP_RET_CODE_DOWNLOAD_FAILED)
		return;
	else if (request_id == TP_RET_CODE_NO_CHANGES_SUCCESS)
	{
		getPage(filename);
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
	//Get clients' or coaches' temporary mesos
	std::pair<QList<bool>,QFileInfoList> mesos;
	appUserModel()->scanUsersSubDirs(mesos, mesos_subdir, "*.txt"_L1);
	if (!mesos.first.isEmpty())
	{
		uint idx{0};
		for(const auto &mesofile : std::as_const(mesos.second))
		{
			const QString &user_id{appUtils()->getLastDirInPath(mesofile.filePath())};
			if (mesoPlanExists(appUtils()->getFileName(mesofile.fileName(), true),
						mesos.first.at(idx) ? user_id : appUserModel()->userId(0),
						mesos.first.at(idx) ? appUserModel()->userId(0) : user_id) == -1)
				static_cast<void>(newMesoFromFile(mesofile.filePath(), mesos.first.at(idx), false));
			else
				static_cast<void>(QFile::remove(mesofile.filePath()));
		}
	}

	//Get own temporary mesos
	mesos.second.clear();
	appUtils()->scanDir(appUserModel()->userDir(0) % mesos_subdir, mesos.second, "*.txt"_L1);
	for(const auto &mesofile : std::as_const(mesos.second))
	{
		if (mesoPlanExists(appUtils()->getFileName(mesofile.fileName(), true),
															appUserModel()->userId(0), appUserModel()->userId(0)) == -1)
			static_cast<void>(newMesoFromFile(mesofile.filePath(), true, false));
		else
			static_cast<void>(QFile::remove(mesofile.filePath()));
	}
}

const uint DBMesocyclesModel::newMesoData(QStringList &&infolist)
{
	const uint meso_idx{count()};
	m_mesoData.append(std::move(infolist));
	m_canExport.append(false);
	m_usedSplits.append(QString{});
	makeUsedSplits(meso_idx);
	m_isMesoOK.append(0);
	return meso_idx;
}

void DBMesocyclesModel::getAllMesocycles()
{
	m_dbModelInterface = new DBModelInterfaceMesocycle{this};
	m_db = new DBMesocyclesTable{m_dbModelInterface};
	appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(m_db, &DBMesocyclesTable::mesocyclesAcquired, this, [this,conn] (QStringList meso_info, const bool last_meso)
	{
		if (!last_meso)
		{
			const uint meso_idx{newMesoData(std::move(meso_info))};
			addSubMesoModel(meso_idx, isOwnMeso(meso_idx));
		}
		else
		{
			disconnect(*conn);
			scanTemporaryMesocycles();
			QMetaObject::invokeMethod(appItemManager()->appHomePage(), "setMesosViewIndex",
					Q_ARG(int, appSettings()->getCustomValue(mesosViewIdxSetting, 0).toInt()));
			if (m_ownMesos)
			{
				connect(m_ownMesos, &HomePageMesoModel::currentIndexChanged, [this] () {
					setWorkingCalendar(m_ownMesos->currentMesoIdx());
				});
			}
			if (m_clientMesos) {
				connect(m_clientMesos, &HomePageMesoModel::currentIndexChanged, [this] () {
					setWorkingCalendar(m_clientMesos->currentMesoIdx());
				});
			}
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

int DBMesocyclesModel::exportToFile_splitData(const uint meso_idx, QFile *mesoFile, const bool formatted) const
{
	int ret{TP_RET_CODE_EXPORT_FAILED};
	const QMap<QChar,DBSplitModel*> &split_models{m_splitModels.value(meso_idx)};
	for (const DBSplitModel *split_model : split_models)
	{
		if (!split_model)
			continue;
		if (!formatted)
			ret = split_model->exportToFile(QString{}, mesoFile);
		else
			ret = split_model->exportToFormattedFile(QString{}, mesoFile);
	}
	return ret;
}
