#include "dbinterface.h"

#include "dbexerciseslisttable.h"
#include "dbexerciseslistmodel.h"
#include "dbmesocyclestable.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocalendartable.h"
#include "dbexercisesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "dbusertable.h"
#include "dbusermodel.h"
#include "tpsettings.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QThread>

DBInterface *DBInterface::app_db_interface(nullptr);

void DBInterface::init()
{
	QFileInfo f_info;
	for (uint i{EXERCISES_TABLE_ID}; i <= APP_TABLES_NUMBER; ++i)
	{
		f_info.setFile(TPDatabaseTable::dbFilePath(i));
		if (!f_info.isReadable())
		{
			TPDatabaseTable *db_table{TPDatabaseTable::createDBTable(i)};
			db_table->createTable();
		}
	}

	sanityCheck();
	getExercisesListVersion();
	getAllUsers();
	getAllMesocycles();

	if (appSettings()->appVersion() != TP_APP_VERSION)
	{
		//All the code to update the database goes in here
		//updateDB(new DBMesoCalendarTable{nullptr});
		//updateDB(new DBMesocyclesTable{nullptr});
		//DBUserTable user{nullptr};
		//user.removeDBFile();
		//appSettings()->saveAppVersion(TP_APP_VERSION);
	}
}

//So far, only DBWorkoutsOrSplitsTable has a sanity check to make
void DBInterface::sanityCheck()
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}};
	createThread(worker, [worker] () { worker->removeTemporaries(true); });
}

void DBInterface::threadFinished(TPDatabaseTable *dbObj)
{
	const QString &dbObjName{dbObj->objectName()};
	dbObj->setResolved(true);
	if (dbObj->waitForThreadToFinish())
		dbObj->thread()->quit();
	LOG_MESSAGE("Database  " << dbObjName << " - " << dbObj->uniqueId() << " calling databaseReady()")
	emit databaseReady(dbObj->uniqueId());
	if (m_WorkerLock[dbObj->tableId()].hasNext())
	{
		const TPDatabaseTable *const nextDbObj{m_WorkerLock[dbObj->tableId()].nextObj()};
		LOG_MESSAGE("Database  " << dbObjName << " - " << nextDbObj->uniqueId() <<" starting in sequence of previous thread")
		nextDbObj->thread()->start();
		if (nextDbObj->waitForThreadToFinish())
			nextDbObj->thread()->wait();
	}
}

void DBInterface::updateDB(TPDatabaseTable *worker)
{
	createThread(worker, [worker] () { worker->updateTable(); });
}

void DBInterface::createThread(TPDatabaseTable *worker, const std::function<void(void)> &execFunc )
{
	worker->setCallbackForDoneFunc([this] (TPDatabaseTable *obj) { return threadFinished(obj); });

	QThread *thread{new QThread{}};
	connect(thread, &QThread::started, worker, execFunc);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread(thread);

	if (!m_threadCleaner.isActive())
	{
		LOG_MESSAGE("Connecting timer")
		m_threadCleaner.setInterval(60000);
		connect(&m_threadCleaner, &QTimer::timeout, this, [this] { cleanUpThreads(); });
		m_threadCleaner.start();
	}

	m_WorkerLock[worker->tableId()].appendObj(worker);
	if (m_WorkerLock[worker->tableId()].canStartThread())
	{
		LOG_MESSAGE("Database  " << worker->objectName() << " -  " << worker->uniqueId() << " starting immediatelly")
		thread->start();
		if (worker->waitForThreadToFinish())
			thread->wait();
	}
	else
	{
		LOG_MESSAGE("Database  " << worker->objectName() << "  Waiting for it to be free: " << worker->uniqueId())
	}
}

void DBInterface::cleanUpThreads()
{
	TPDatabaseTable *dbObj{nullptr};
	bool locks_empty{true};

	for (uint x{EXERCISES_TABLE_ID}; x <= APP_TABLES_NUMBER; ++x)
	{
		for(int i{static_cast<int>(m_WorkerLock[x].count()) - 1}; i >= 0 ; --i)
		{
			dbObj = m_WorkerLock[x].at(i);
			if (dbObj->resolved())
			{
				LOG_MESSAGE("cleanUpThreads: " << dbObj->objectName() << "uniqueId: " << dbObj->uniqueId());
				dbObj->disconnect();
				dbObj->deleteLater();
				m_WorkerLock[x].removeAt(i);
			}
		}
		locks_empty &= m_WorkerLock[x].count() == 0;
	}
	if (locks_empty)
	{
		LOG_MESSAGE("Disconnecting timer")
		m_threadCleaner.stop();
		m_threadCleaner.disconnect();
		disconnect(this, &DBInterface::databaseReady, this, nullptr);
	}
}

//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
void DBInterface::getAllUsers()
{
	DBUserTable worker{appUserModel()};
	worker.getAllUsers();
}

void DBInterface::saveUser(const uint row)
{
	DBUserTable *worker{new DBUserTable{appUserModel()}};
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); });
}

void DBInterface::removeUser(const uint row)
{
	DBUserTable *worker{new DBUserTable{nullptr}};
	worker->addExecArg(appUserModel()->userId(row));
	createThread(worker, [worker] () { return worker->removeUser(); });
}

void DBInterface::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable *worker{new DBUserTable{appUserModel()}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
int DBInterface::getAllExercises()
{
	DBExercisesTable *worker{new DBExercisesTable{appExercisesList()}};
	createThread(worker, [worker] () { worker->getAllExercises(); });
	return worker->uniqueId();
}

void DBInterface::saveExercises()
{
	DBExercisesTable *worker{new DBExercisesTable{appExercisesList()}};
	createThread(worker, [worker] () { return worker->saveExercises(); });
}

void DBInterface::removeExercise(const uint row)
{
	DBExercisesTable *worker{new DBExercisesTable{nullptr}};
	worker->addExecArg(appExercisesList()->id(row));
	worker->addExecArg(row);
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable *worker{new DBExercisesTable{appExercisesList()}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DBInterface::updateExercisesList()
{
	DBExercisesTable *worker{new DBExercisesTable{appExercisesList()}};
	connect(worker, &DBExercisesTable::updatedFromExercisesList, this, [this] () {
		appSettings()->setExercisesListVersion(m_exercisesListVersion);
	});
	createThread(worker, [worker] () { return worker->updateExercisesList(); });
}

void DBInterface::getExercisesListVersion()
{
	m_exercisesListVersion = STR_ZERO;
	QFile exercisesListFile{":/extras/exerciseslist.lst"_L1};
	if (exercisesListFile.open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		char buf[20]{0};
		qint64 lineLength;
		lineLength = exercisesListFile.readLine(buf, sizeof(buf));
		if (lineLength > 0)
		{
			QString line{std::move(buf)};
			if (line.startsWith("#Vers"_L1))
				m_exercisesListVersion = std::move(line.split(';').at(1).trimmed());
		}
		exercisesListFile.close();
		if (m_exercisesListVersion != appSettings()->exercisesListVersion())
			updateExercisesList();
	}
}
//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
void DBInterface::getAllMesocycles()
{
	DBMesocyclesTable worker{appMesoModel()};
	worker.getAllMesocycles();
	if (appUserModel()->mainUserConfigured())
		appMesoModel()->scanTemporaryMesocycles();
}

void DBInterface::saveMesocycle(const uint meso_idx)
{
	DBMesocyclesTable *worker{new DBMesocyclesTable{appMesoModel()}};

	if (appMesoModel()->_id(meso_idx) < 0)
		worker->waitForThreadToFinish();
	else
		saveMesoCalendar(meso_idx);
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesocycle(); });
}

void DBInterface::removeMesocycle(const uint meso_idx)
{
	removeMesoCalendar(meso_idx);
	removeAllMesoSplits(meso_idx);
	DBMesocyclesTable *worker{new DBMesocyclesTable{nullptr}};
	worker->addExecArg(appMesoModel()->id(meso_idx));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable *worker{new DBMesocyclesTable{appMesoModel()}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
int DBInterface::getMesoSplit(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	createThread(worker, [worker] () { worker->getExercises(); });
	return worker->uniqueId();
}

void DBInterface::saveMesoSplit(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	if (model->id().toInt() < 0)
		worker->setWaitForThreadToFinish(true);
	createThread(worker, [worker] () { worker->saveExercises(); });
}

void DBInterface::removeMesoSplit(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	worker->addExecArg(false);
	createThread(worker, [worker] () { return worker->removeExercises(); });
}

void DBInterface::removeAllMesoSplits(const uint meso_idx)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}};
	worker->addExecArg(true);
	createThread(worker, [worker] () { return worker->removeExercises(); });
}

void DBInterface::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{nullptr}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}

bool DBInterface::mesoHasAllSplitPlans(const uint meso_idx) const
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}};
	const bool ret{worker->mesoHasAllSplitPlans(appMesoModel()->id(meso_idx), appMesoModel()->split(meso_idx))};
	delete worker;
	return ret;
}

bool DBInterface::mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter) const
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}};
	const bool ret{worker->mesoHasSplitPlan(meso_id, split_letter)};
	delete worker;
	return ret;
}
//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
int DBInterface::getMesoCalendar(const uint meso_idx)
{
	appMesoModel()->mesoCalendarManager()->addCalendarForMeso(meso_idx);
	DBMesoCalendarTable *worker{new DBMesoCalendarTable{appMesoModel()->mesoCalendarManager()}};
	worker->addExecArg(meso_idx);
	worker->addExecArg(appMesoModel()->id(meso_idx));
	createThread(worker, [worker] () { worker->getMesoCalendar(); });
	return worker->uniqueId();
}

void DBInterface::saveMesoCalendar(const uint meso_idx)
{
	DBMesoCalendarTable *worker{new DBMesoCalendarTable{appMesoModel()->mesoCalendarManager()}};
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesoCalendar(); });
}

void DBInterface::removeMesoCalendar(const uint meso_idx)
{
	DBMesoCalendarTable *worker{new DBMesoCalendarTable{nullptr}};
	worker->addExecArg(appMesoModel()->id(meso_idx));
	createThread(worker, [worker] () { return worker->removeEntry(true); });
}

void DBInterface::deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile)
{
	DBMesoCalendarTable *worker{new DBMesoCalendarTable{nullptr}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------
int DBInterface::getWorkout(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	createThread(worker, [worker] () { return worker->getExercises(); });
	return worker->uniqueId();
}

int DBInterface::getPreviousWorkouts(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	createThread(worker, [worker] () { return worker->getPreviousWorkouts(); });
	return worker->uniqueId();
}

void DBInterface::saveWorkout(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	createThread(worker, [worker] () { return worker->saveExercises(); });
}

void DBInterface::removeWorkout(DBExercisesModel *model)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{model}};
	worker->addExecArg(false);
	createThread(worker, [worker] () { return worker->removeExercises(); });
}

void DBInterface::removeAllWorkouts(const uint meso_idx)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{WORKOUT_TABLE_ID}};
	worker->addExecArg(true);
	createThread(worker, [worker] () { return worker->removeExercises(); });
}

void DBInterface::deleteWorkoutsTable(const bool bRemoveFile)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{nullptr}};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------

//-----------------------------------------------------------STATISTICS-----------------------------------------------------------
/*void DBInterface::getExercisesForSplitWithinMeso(const uint meso_idx, const QChar &splitLetter)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{nullptr}};
	auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker] (const uint db_id) {
		if (db_id == worker->uniqueId())
		{
			disconnect(*conn);
			emit databaseReadyWithData(MESOSPLIT_TABLE_ID, QVariant::fromValue(worker->retrievedStats()));
		}
	});
	worker->addExecArg(splitLetter);
	worker->addExecArg(appMesoModel()->id(meso_idx));
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { return worker->getExercisesForSplitWithinMeso(); });
}

void DBInterface::completedDaysForSplitWithinTimePeriod(const QChar &splitLetter, const QDate &startDate, const QDate &endDate)
{
	DBMesoCalendarTable *worker{new DBMesoCalendarTable{nullptr}};
	auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker] (const uint db_id) {
		if (db_id == worker->uniqueId())
		{
			disconnect(*conn);
			emit databaseReadyWithData(MESOCALENDAR_TABLE_ID, QVariant::fromValue(worker->retrievedDates()));
		}
	});
	worker->addExecArg(splitLetter);
	worker->addExecArg(startDate);
	worker->addExecArg(endDate);
	createThread(worker, [worker] () { return worker->completedDaysForSplitWithinTimePeriod(); });
}

void DBInterface::workoutsInfoForTimePeriod(const QStringList &exercises, const QList<QDate> &workoutDates)
{
	DBWorkoutsOrSplitsTable *worker{new DBWorkoutsOrSplitsTable{nullptr}};
	auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker] (const uint db_id) {
		if (db_id == worker->uniqueId())
		{
			disconnect(*conn);
			emit databaseReadyWithData(WORKOUT_TABLE_ID, QVariant::fromValue(worker->workoutsInfo()));
		}
	});
	worker->addExecArg(exercises);
	worker->addExecArg(QVariant::fromValue(workoutDates));
	createThread(worker, [worker] () { return worker->workoutsInfoForTimePeriod(); });
}*/
//-----------------------------------------------------------STATISTICS-----------------------------------------------------------
