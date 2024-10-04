#include "dbinterface.h"
#include "tpappcontrol.h"
#include "tputils.h"

#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "dbmesocylestable.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"
#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaytable.h"
#include "dbtrainingdaymodel.h"
#include "dbusertable.h"
#include "dbusermodel.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>

#define SPLITS_LOADED_ID 4321

DBInterface* DBInterface::app_db_interface(nullptr);

void DBInterface::init()
{
	m_DBFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/Files/Database/"_qs;
	QDir appDir(m_DBFilePath);
	if (!appDir.mkpath(m_DBFilePath))
		MSG_OUT("TP directory creation failed: " << m_DBFilePath);

	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath));
		db_exercises->createTable();
		delete db_exercises;
		appSettings()->setValue("exercisesListVersion", STR_ZERO);
	}
	f_info.setFile(m_DBFilePath + DBMesocyclesFileName);
	if (!f_info.isReadable())
	{
		DBMesocyclesTable* db_mesos(new DBMesocyclesTable(m_DBFilePath));
		db_mesos->createTable();
		delete db_mesos;
	}
	f_info.setFile(m_DBFilePath + DBMesoSplitFileName);
	if (!f_info.isReadable())
	{
		DBMesoSplitTable* db_split(new DBMesoSplitTable(m_DBFilePath));
		db_split->createTable();
		delete db_split;
	}
	f_info.setFile(m_DBFilePath + DBMesoCalendarFileName);
	if (!f_info.isReadable())
	{
		DBMesoCalendarTable* db_cal(new DBMesoCalendarTable(m_DBFilePath));
		db_cal->createTable();
		delete db_cal;
	}
	f_info.setFile(m_DBFilePath + DBTrainingDayFileName);
	if (!f_info.isReadable())
	{
		DBTrainingDayTable* db_tday(new DBTrainingDayTable(m_DBFilePath));
		db_tday->createTable();
		delete db_tday;
	}
	f_info.setFile(m_DBFilePath + DBUserFileName);
	if (!f_info.isReadable())
	{
		DBUserTable* db_user(new DBUserTable(m_DBFilePath));
		db_user->createTable();
		delete db_user;
	}

	getExercisesListVersion();
	getAllUsers();
	appMesoModel()->setUserModel(appUserModel());
	getAllMesocycles();

	if (appSettings()->value("appVersion") != TP_APP_VERSION)
	{
		//All update code goes in here
		//updateDB(new DBMesoCalendarTable(m_DBFilePath));
		//updateDB(new DBMesocyclesTable(m_DBFilePath));
		//DBUserTable user(m_DBFilePath);
		//user.removeDBFile();
		appSettings()->setValue("appVersion", TP_APP_VERSION);
	}
}

void DBInterface::threadFinished(TPDatabaseTable* dbObj)
{
	const QString& dbObjName(dbObj->objectName());
	dbObj->setResolved(true);
	if (dbObj->waitForThreadToFinish())
		dbObj->thread()->quit();
	MSG_OUT("Database  " << dbObjName << " - " << dbObj->uniqueID() << " calling databaseReady()")
	emit databaseReady(dbObj->uniqueID());
	if (m_WorkerLock[dbObj->tableID()].hasNext())
	{
		const TPDatabaseTable* const nextDbObj(m_WorkerLock[dbObj->tableID()].nextObj());
		MSG_OUT("Database  " << dbObjName << " - " << nextDbObj->uniqueID() <<" starting in sequence of previous thread")
		nextDbObj->thread()->start();
		if (nextDbObj->waitForThreadToFinish())
			nextDbObj->thread()->wait();
	}
}

void DBInterface::updateDB(TPDatabaseTable* worker)
{
	createThread(worker, [worker] () { worker->updateDatabase(); });
}

void DBInterface::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForDoneFunc([&] (TPDatabaseTable* obj) { return threadFinished(obj); });

	QThread* thread{new QThread()};
	connect(thread, &QThread::started, worker, execFunc);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread(thread);

	if (!m_threadCleaner.isActive())
	{
		MSG_OUT("Connecting timer")
		m_threadCleaner.setInterval(60000);
		connect(&m_threadCleaner, &QTimer::timeout, this, [&] { return cleanUpThreads(); });
		m_threadCleaner.start();
	}

	m_WorkerLock[worker->tableID()].appendObj(worker);
	if (m_WorkerLock[worker->tableID()].canStartThread())
	{
		MSG_OUT("Database  " << worker->objectName() << " -  " << worker->uniqueID() << " starting immediatelly")
		thread->start();
		if (worker->waitForThreadToFinish())
			thread->wait();
	}
	else
		MSG_OUT("Database  " << worker->objectName() << "  Waiting for it to be free: " << worker->uniqueID())
}

void DBInterface::cleanUpThreads()
{
	TPDatabaseTable* dbObj(nullptr);
	bool locks_empty(true);

	for (uint x(1); x <= APP_TABLES_NUMBER; ++x)
	{
		for(int i(m_WorkerLock[x].count() - 1); i >= 0 ; --i)
		{
			dbObj = m_WorkerLock[x].at(i);
			if (dbObj->resolved())
			{
				MSG_OUT("cleanUpThreads: " << dbObj->objectName() << "uniqueID: " << dbObj->uniqueID());
				dbObj->disconnect();
				dbObj->deleteLater();
				m_WorkerLock[x].removeAt(i);
			}
		}
		locks_empty &= m_WorkerLock[x].count() == 0;
	}
	if (locks_empty)
	{
		MSG_OUT("Disconnecting timer")
		m_threadCleaner.stop();
		m_threadCleaner.disconnect();
		disconnect(this, &DBInterface::databaseReady, this, nullptr);
	}
}

//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
void DBInterface::getAllUsers()
{
	DBUserTable worker{m_DBFilePath, appUserModel()};
	worker.getAllUsers();

	bool noUsers(appUserModel()->count() == 0);
	if (!noUsers)
		noUsers = appUserModel()->userName(0).isEmpty();
	if (noUsers)
		appUserModel()->addUser(false);
}

void DBInterface::saveUser(const uint row)
{
	DBUserTable* worker{new DBUserTable(m_DBFilePath, appUserModel())};
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); });
}

void DBInterface::removeUser(const uint row, const bool bCoach)
{
	DBUserTable* worker{new DBUserTable(m_DBFilePath)};
	worker->addExecArg(appUserModel()->userId(row));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable* worker{new DBUserTable(m_DBFilePath, appUserModel())};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DBInterface::getAllExercises()
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath, appExercisesModel())};
	createThread(worker, [worker] () { worker->getAllExercises(); });
}

void DBInterface::saveExercises()
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath, appExercisesModel())};
	createThread(worker, [worker] () { return worker->saveExercises(); });
}

void DBInterface::removeExercise(const uint row)
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath)};
	worker->addExecArg(appExercisesModel()->id(row));
	worker->addExecArg(row);
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath, appExercisesModel())};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DBInterface::updateExercisesList()
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath, appExercisesModel())};
	connect(worker, &DBExercisesTable::updatedFromExercisesList, this, [&] () {
		appSettings()->setValue("exercisesListVersion", m_exercisesListVersion);
	});
	createThread(worker, [worker] () { return worker->updateExercisesList(); });
}

void DBInterface::getExercisesListVersion()
{
	m_exercisesListVersion = STR_ZERO;
	QFile exercisesListFile(u":/extras/exerciseslist.lst"_qs);
	if (exercisesListFile.open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		char buf[20] = { 0 };
		qint64 lineLength;
		QString line;
		lineLength = exercisesListFile.readLine(buf, sizeof(buf));
		if (lineLength > 0)
		{
			line = buf;
			if (line.startsWith(u"#Vers"_qs))
				m_exercisesListVersion = line.split(';').at(1).trimmed();
		}
		exercisesListFile.close();
		if (m_exercisesListVersion != appSettings()->value("exercisesListVersion").toString())
			updateExercisesList();
	}
}
//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
void DBInterface::getAllMesocycles()
{
	DBMesocyclesTable worker{m_DBFilePath, appMesoModel()};
	worker.getAllMesocycles();

	if (appMesoModel()->count() > 0)
	{
		DBMesoSplitTable worker2{m_DBFilePath, appMesoModel()->mesoSplitModel()};
		worker2.getAllMesoSplits();
	}
}

void DBInterface::saveMesocycle(const uint meso_idx)
{
	DBMesocyclesTable* worker{new DBMesocyclesTable(m_DBFilePath, appMesoModel())};

	if (appMesoModel()->_id(meso_idx) == -1)
	{
		if (appMesoModel()->importMode())
			worker->setWaitForThreadToFinish(true);

		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,meso_idx,worker] (const uint db_id) {
			if (db_id == worker->uniqueID())
			{
				disconnect(*conn);
				if (appMesoModel()->newMesoCalendarChanged(meso_idx))
				{
					appMesoModel()->setNewMesoCalendarChanged(meso_idx, false);
					changeMesoCalendar(meso_idx, false, false);
				}
				saveMesoSplit(meso_idx);
			}
		});
	}
	else
		saveMesoSplit(meso_idx);

	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesocycle(); });
}

void DBInterface::removeMesocycle(const uint meso_idx)
{
	if (appMesoModel()->_id(meso_idx))
	{
		removeMesoCalendar(meso_idx);
		removeMesoSplit(meso_idx);
		DBMesocyclesTable* worker{new DBMesocyclesTable(m_DBFilePath)};
		worker->addExecArg(appMesoModel()->id(meso_idx));
		createThread(worker, [worker] () { return worker->removeEntry(); });
	}
}

void DBInterface::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable* worker{new DBMesocyclesTable(m_DBFilePath, appMesoModel())};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DBInterface::saveMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, appMesoModel()->mesoSplitModel())};
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesoSplit(); });
}

void DBInterface::removeMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath)};
	worker->addExecArg(appMesoModel()->id(meso_idx));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, appMesoModel()->mesoSplitModel())};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}

void DBInterface::loadCompleteMesoSplits(const uint meso_idx, QMap<QChar,DBMesoSplitModel*>& splitModels, const bool bThreaded)
{
	const QString& mesoSplit(appMesoModel()->split(meso_idx));
	QString mesoLetters;
	DBMesoSplitModel* splitModel(nullptr);
	uint nSplits(appMesoModel()->totalSplits(meso_idx));
	DBMesoSplitTable* worker2(nullptr);
	bool connected(false);

	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator& itr_end(mesoSplit.constEnd());

	do {
		if (*itr == QChar('R'))
			continue;
		if (mesoLetters.contains(*itr))
			continue;

		mesoLetters.append(*itr);
		splitModel = splitModels.value(*itr);

		if (bThreaded)
		{
			DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, splitModel)};
			worker->addExecArg(appMesoModel()->id(meso_idx));
			worker->addExecArg(*itr);
			if (!connected)
			{
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,nSplits] (const uint db_id) mutable {
					MSG_OUT("loadCompleteMesoSplits received databaseReady() " << db_id)
					if (m_WorkerLock[MESOSPLIT_TABLE_ID].hasID(db_id))
					{
						if (--nSplits == 0)
						{
							disconnect(*conn);
							mb_splitsLoaded = true;
							emit internalSignal(SPLITS_LOADED_ID);
							emit databaseReadyWithData(QVariant());
						}
					}
				});
				connected = true;
			}
			createThread(worker, [worker] () { return worker->getCompleteMesoSplit(); });
		}
		else
		{
			if (!worker2)
			{
				worker2 = new DBMesoSplitTable(m_DBFilePath, splitModel);
				worker2->addExecArg(appMesoModel()->id(meso_idx));
				worker2->addExecArg(static_cast<QChar>(*itr));
			}
			else
			{
				worker2->changeExecArg(static_cast<QChar>(*itr), 1);
				worker2->setAnotherModel(splitModel);
			}
			worker2->getCompleteMesoSplit(false);
		}
	} while (++itr != itr_end);
	if (worker2)
		delete worker2;
}

void DBInterface::saveMesoSplitComplete(DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, model)};
	worker->addExecArg(appMesoModel()->id(model->mesoIdx()));
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); });
}

bool DBInterface::mesoHasPlan(const uint meso_id, const QString& splitLetter) const
{
	if (splitLetter != u"R"_qs)
	{
		DBMesoSplitTable* meso_split{new DBMesoSplitTable(m_DBFilePath)};
		const bool ret(meso_split->mesoHasPlan(QString::number(meso_id), splitLetter));
		meso_split->deleteLater();
		return ret;
	}
	return false;
}

void DBInterface::loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, model)};
	worker->addExecArg(QString::number(prev_meso_id));
	worker->addExecArg(model->splitLetter().at(0));
	createThread(worker, [worker] () { worker->getCompleteMesoSplit(); });
}
//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
void DBInterface::getMesoCalendar(const uint meso_idx)
{
	DBMesoCalendarTable* worker{new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx))};
	createThread(worker, [worker] () { worker->getMesoCalendar(); });
}

void DBInterface::changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore)
{
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [this,meso_idx,bPreserveOldInfo,bPreserveOldInfoUntilDayBefore] ()
		{
			return changeMesoCalendar(meso_idx, bPreserveOldInfo, bPreserveOldInfoUntilDayBefore);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx);
		return;
	}
	DBMesoCalendarTable* worker{new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx))};
	const QDate& endDate{bPreserveOldInfo && bPreserveOldInfoUntilDayBefore ?
					QDate::currentDate() :
					appMesoModel()->endDate(meso_idx)};
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilDayBefore);
	worker->addExecArg(endDate);
	createThread(worker, [worker] () { worker->changeMesoCalendar(); });
}

void DBInterface::updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx{static_cast<uint>(tDayModel->mesoIdx())};
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [this,tDayModel] () {
				return updateMesoCalendarModel(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx);
		return;
	}
	DBMesoCalendarTable* worker{new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx))};
	worker->addExecArg(appMesoModel()->id(meso_idx)); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(tDayModel->date());
	worker->addExecArg(tDayModel->splitLetter());
	createThread(worker, [worker] () { worker->updateMesoCalendar(); });
}

void DBInterface::updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel)
{
	DBMesoCalendarTable* worker{new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(tDayModel->mesoIdx()))};
	worker->addExecArg(tDayModel->date());
	worker->addExecArg(tDayModel->trainingDay());
	worker->addExecArg(tDayModel->splitLetter());
	worker->addExecArg(tDayModel->dayIsFinished() ? STR_ONE : STR_ZERO);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DBInterface::setDayIsFinished(const uint meso_idx, const QDate& date, const bool bFinished)
{
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [this,meso_idx,date,bFinished] () {
			return setDayIsFinished(meso_idx, date, bFinished);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx);
		return;
	}
	DBMesoCalendarTable* worker{new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx))};
	worker->addExecArg(date);
	worker->addExecArg(bFinished);
	createThread(worker, [worker] () { worker->updateDayIsFinished(); });
}

void DBInterface::removeMesoCalendar(const uint meso_id)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath));
	worker->addExecArg(QString::number(meso_id));
	createThread(worker, [worker] () { return worker->removeMesoCalendar(); });
}

void DBInterface::deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx)));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DBInterface::getTrainingDay(DBTrainingDayModel* tDayModel)
{
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	worker->addExecArg(tDayModel->dateStr());
	worker->addExecArg(appMesoModel()->id(tDayModel->mesoIdx()));
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker,tDayModel] (const uint db_id) {
		if (db_id == worker->uniqueID())
		{
			disconnect(*conn);
			if (tDayModel->exerciseCount() == 0)
				verifyTDayOptions(tDayModel);
		}
	});
	createThread(worker, [worker] () { return worker->getTrainingDay(); });
}

void DBInterface::getTrainingDayExercises(DBTrainingDayModel* tDayModel)
{
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, const_cast<DBTrainingDayModel*>(tDayModel))};
	worker->addExecArg(tDayModel->dateStr());
	worker->addExecArg(appMesoModel()->id(tDayModel->mesoIdx()));
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker,tDayModel] (const uint db_id) {
		if (db_id == worker->uniqueID())
		{
			disconnect(*conn);
			if (tDayModel->exerciseCount() == 0)
				verifyTDayOptions(tDayModel);
		}
	});
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DBInterface::verifyTDayOptions(DBTrainingDayModel* tDayModel)
{
	if (tDayModel->splitLetter() >= u"A"_qs && tDayModel->splitLetter() <= u"F"_qs)
	{
		DBTrainingDayModel* tempModel{new DBTrainingDayModel(this, tDayModel->mesoIdx())};
		DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tempModel)};
		worker->addExecArg(appMesoModel()->id(tDayModel->mesoIdx()));
		worker->addExecArg(tDayModel->splitLetter());
		worker->addExecArg(tDayModel->date());
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &DBInterface::databaseReady, this, [this,conn,worker,tDayModel] (const uint db_id) {
			if (db_id == worker->uniqueID())
			{
				disconnect(*conn);
				DBTrainingDayModel* tempModel{worker->model()};
				//setTrainingDay does not relate to training day in the temporary model. It's only a place to store a value we need this model to carry
				tempModel->setTrainingDay(mesoHasPlan(appMesoModel()->_id(tempModel->mesoIdx()), tempModel->splitLetter()) ?
						STR_ONE : STR_ZERO);
				emit databaseReadyWithData(QVariant::fromValue(tempModel));
				delete tempModel;
			}
		});
		createThread(worker, [worker] () { return worker->getPreviousTrainingDaysInfo(); });
	}
}

void DBInterface::loadExercisesFromDate(const QString& strDate, DBTrainingDayModel* tDayModel)
{
	const QDate& date{appUtils()->getDateFromStrDate(strDate)};
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	worker->addExecArg(appMesoModel()->id(tDayModel->mesoIdx()));
	worker->addExecArg(QString::number(date.toJulianDay()));
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(true); });
}

void DBInterface::loadExercisesFromMesoPlan(DBTrainingDayModel* tDayModel, QMap<QChar, DBMesoSplitModel *>& splitModels)
{
	if (!mb_splitsLoaded)
	{
		connect( this, &DBInterface::internalSignal, this, [&,tDayModel,splitModels] (const uint id) mutable {
			if (id == SPLITS_LOADED_ID)
				loadExercisesFromMesoPlan(tDayModel, splitModels);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(tDayModel->mesoIdx(), splitModels);
	}
	else
		tDayModel->convertMesoSplitModelToTDayModel(splitModels.value(tDayModel->splitLetter().at(0)));
}

void DBInterface::convertTDayToPlan(const DBTrainingDayModel* const tDayModel, QMap<QChar, DBMesoSplitModel *> &splitModels)
{
	if (!mb_splitsLoaded)
	{
		connect( this, &DBInterface::internalSignal, this, [&,tDayModel,splitModels] (const uint id) mutable {
			if (id == SPLITS_LOADED_ID)
				return convertTDayToPlan(tDayModel, splitModels);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(tDayModel->mesoIdx(), splitModels);
	}
	else
	{
		DBMesoSplitTable* worker{new DBMesoSplitTable(m_DBFilePath, splitModels.value(tDayModel->splitLetter().at(0)))};
		worker->addExecArg(appMesoModel()->id(tDayModel->mesoIdx()));
		worker->addExecArg(tDayModel->splitLetter());
		createThread(worker, [worker,tDayModel] () { return worker->convertTDayExercisesToMesoPlan(tDayModel); });
	}
}

void DBInterface::saveTrainingDay(DBTrainingDayModel* const tDayModel)
{
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	createThread(worker, [worker] () { return worker->saveTrainingDay(); });
}

void DBInterface::removeTrainingDay(const uint meso_idx)
{
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath)};
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DBInterface::deleteTrainingDayTable(const bool bRemoveFile)
{
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath)};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
