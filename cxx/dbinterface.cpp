#include "dbinterface.h"
#include "tpappcontrol.h"
#include "tputils.h"
#include "qmlitemmanager.h"

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

#include <QSettings>
#include <QQmlApplicationEngine>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QDir>

#define SPLITS_LOADED_ID 4321


void DBInterface::init()
{
	m_DBFilePath = appSettings()->value("dbFilePath").toString();
	if (m_DBFilePath.isEmpty())
	{
		m_DBFilePath = appUtils()->getAppDir(appQmlEngine()->offlineStoragePath());
		appSettings()->setValue("dbFilePath", m_DBFilePath);
	}

	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath));
		db_exercises->createTable();
		delete db_exercises;
		appSettings()->setValue("exercisesListVersion", "0");
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
	if (m_exercisesListVersion != appSettings()->value("exercisesListVersion").toString())
		updateExercisesList();

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

void DBInterface::gotResult(TPDatabaseTable* dbObj)
{
	const QString dbObjName(dbObj->objectName());
	if (dbObj->result())
	{
		switch (dbObj->opCode())
		{
			default:
			break;
			case OP_DELETE_TABLE:
				dbObj->createTable();
			break;
			case OP_UPDATE_LIST:
				appSettings()->setValue("exercisesListVersion", m_exercisesListVersion);
			break;
			case OP_READ:
				if (dbObjName == DBTrainingDayObjectName)
				{
					DBTrainingDayModel* tempModel(static_cast<DBTrainingDayModel*>(dbObj->model()));
					QmlItemManager* itemMngr = m_itemManager.at(tempModel->mesoIdx());
					tempModel->setFast(tempModel->count()-1, TDAY_COL_TRAININGDAYNUMBER,
						mesoHasPlan(appMesoModel()->getIntFast(tempModel->mesoIdx(), MESOCYCLES_COL_ID), tempModel->splitLetter()) ?
							STR_ONE : STR_ZERO);
					itemMngr->setTrainingDayPageEmptyDayOptions(tempModel);
					delete tempModel;
				}
			break;
			case OP_ADD:
				if (dbObjName == DBMesocyclesObjectName)
					;
			break;
		}
	}

	dbObj->setResolved(true);
	if (dbObj->waitForThreadToFinish())
		dbObj->thread()->quit();
	MSG_OUT("Database  " << dbObjName << " - " << dbObj->uniqueID() << " calling databaseReady()")
	emit databaseReady(dbObj->uniqueID());
	if (m_WorkerLock[dbObj->tableID()].hasNext())
	{
		TPDatabaseTable* nextDbObj(m_WorkerLock[dbObj->tableID()].nextObj());
		MSG_OUT("Database  " << dbObjName << " - " << nextDbObj->uniqueID() <<" starting in sequence of previous thread")
		nextDbObj->thread()->start();
		if (nextDbObj->waitForThreadToFinish())
			nextDbObj->thread()->wait();
	}
}

/*void DBInterface::verifyBackupPageProperties(QQuickItem* page) const
{
	QFileInfo backupDirInfo(appSettings()->value("backupFolder").toString());
	const bool bCanWriteToBackupFolder(backupDirInfo.isDir() && backupDirInfo.isWritable());
	uint restoreCount(0);

	if (bCanWriteToBackupFolder)
	{
		const QString appDir(appSettings()->value("backupFolder").toString() + u"/tp/"_qs);
		bool bCanReadFile(false);

		QFileInfo f_info(appDir + DBExercisesFileName);
		if ((bCanReadFile = f_info.isReadable()))
			restoreCount++;
		page->setProperty("bCanRestoreExercises", bCanReadFile);

		f_info.setFile(appDir + DBMesocyclesFileName);
		if ((bCanReadFile = f_info.isReadable()))
			restoreCount++;
		page->setProperty("bCanRestoreMeso", bCanReadFile);

		f_info.setFile(appDir + DBMesoSplitFileName);
		if ((bCanReadFile = f_info.isReadable()))
			restoreCount++;
		page->setProperty("bCanRestoreMesoSplit", bCanReadFile);

		f_info.setFile(appDir + DBMesoCalendarFileName);
		if ((bCanReadFile = f_info.isReadable()))
			restoreCount++;
		page->setProperty("bCanRestoreMesoCal", bCanReadFile);

		f_info.setFile(appDir + DBTrainingDayFileName);
		if ((bCanReadFile = f_info.isReadable()))
			restoreCount++;
		page->setProperty("bCanRestoreTraining", bCanReadFile);
	}
	page->setProperty("bCanWriteToBackupFolder", bCanWriteToBackupFolder);
	page->setProperty("restoreCount", restoreCount);
}

static bool copyDBFiles(const QString& sourcePath, const QString& targetPath, const QVariantList& selectedFiles)
{
	bool bOK(true);
	QFileInfo f_info(targetPath);
	if (!f_info.exists())
	{
		QDir dir;
		bOK = dir.mkdir(targetPath, QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner
					|QFileDevice::ReadGroup|QFileDevice::ExeGroup|QFileDevice::ReadOther|QFileDevice::ExeOwner);
	}
	if (bOK)
	{
		QFile inFile, outFile;
		QString dbFile;
		for (uint i(0); i < 5; ++i)
		{
			if (selectedFiles.at(i).toInt() == 1)
			{
				switch (i)
				{
					case 0: dbFile = DBMesocyclesFileName; break;
					case 1: dbFile = DBMesoSplitFileName; break;
					case 2: dbFile = DBMesoCalendarFileName; break;
					case 3: dbFile = DBTrainingDayFileName; break;
					case 4: dbFile = DBExercisesFileName; break;
				}
				inFile.setFileName(sourcePath + dbFile);
				outFile.setFileName(targetPath + dbFile);
				if (outFile.exists())
					outFile.remove();
				if ((bOK = inFile.copy(outFile.fileName())))
					QFile::setPermissions(targetPath + dbFile, QFileDevice::ReadUser | QFileDevice::WriteUser);
			}
		}
	}
	return bOK;
}

static void fixPath(QString& path)
{
	if (path.endsWith('/')) path.chop(1);
	if (!path.endsWith(u"/tp"_qs)) path.append(u"/tp"_qs);
	path.append('/');
}

void DBInterface::copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const
{
	QString finalPath(targetPath);
	fixPath(finalPath);
	const bool bOK(copyDBFiles(appSettings()->value("dbFilePath").toString(), finalPath, backupFiles));
	page->setProperty("opResult", bOK ? 1 : 2);
	if (bOK)
		page->setProperty("backupCount", 0);
}

void DBInterface::copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const
{
	QString origPath(sourcePath);
	fixPath(origPath);
	const bool bOK(copyDBFiles(origPath, appSettings()->value("dbFilePath").toString(), restoreFiles));
	page->setProperty("opResult", bOK ? 3 : 4);
	if (bOK)
		page->setProperty("restoreCount", 0);
}*/

void DBInterface::updateDB(TPDatabaseTable* worker)
{
	createThread(worker, [worker] () { worker->updateDatabase(); } );
}

void DBInterface::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForDoneFunc([&] (TPDatabaseTable* obj) { return gotResult(obj); });

	QThread* thread(new QThread());
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
	{
		appUserModel()->setIsEmpty(true);
		appUserModel()->addUser(false);
	}
}

void DBInterface::saveUser(const uint row)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, appUserModel()));
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); } );
}

void DBInterface::removeUser(const uint row, const bool bCoach)
{
	if (row >= 1)
	{
		DBUserTable* worker(new DBUserTable(m_DBFilePath, appUserModel()));
		worker->addExecArg(appUserModel()->userId(row));
		createThread(worker, [worker] () { return worker->removeEntry(); } );
		m_itemManager.at(0)->removeUser(row, bCoach);
	}
}

void DBInterface::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, appUserModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DBInterface::getAllExercises()
{
	if (appExercisesModel()->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appExercisesModel()));
		worker->setUniqueID(2222);
		createThread(worker, [worker] () { worker->getAllExercises(); } );
	}
	else
		emit databaseReady(2222);
}

void DBInterface::getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (appExercisesModel()->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appExercisesModel()));
		//connect( this, &DBInterface::databaseReady, this, [&,worker,connectPage] (const uint db_id) {
		//	if (db_id == worker->uniqueID()) return createExercisesListPage(connectPage); });
		createThread(worker, [worker] () { return worker->getAllExercises(); } );
	}
	QmlItemManager* itemMngr = m_itemManager.at(0);
	itemMngr->getExercisesPage(bChooseButtonEnabled, connectPage);
}

void DBInterface::saveExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appExercisesModel()));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->saveExercise(); } );
}

void DBInterface::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appExercisesModel()));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->removeEntry(); } );
}

void DBInterface::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appExercisesModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DBInterface::updateExercisesList()
{
	DBExercisesTable* worker{new DBExercisesTable(m_DBFilePath, appExercisesModel())};
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
	}
}

void DBInterface::exportExercisesList(const bool bShare)
{
	const QString suggestedName(tr("TrainingPlanner Exercises List.txt"));
	setExportFileName(suggestedName);
	if (!appExercisesModel()->collectExportData())
	{
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -6));
		return;
	}
	QFile* outFile(nullptr);
	if (exportToFile(appExercisesModel(), exportFileName(), outFile))
	{
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
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

void DBInterface::getMesocyclePage(const uint meso_idx)
{
	appMesoModel()->setCurrentMesoIdx(meso_idx);
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	itemMngr->getMesoPage();
}

uint DBInterface::createNewMesocycle(const bool bCreatePage)
{
	QDate startDate, endDate, minimumStartDate;
	if (appMesoModel()->count() == 0)
	{
		minimumStartDate.setDate(2023, 0, 2); //first monday of that year
		startDate = QDate::currentDate();
		endDate = appUtils()->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (appMesoModel()->getInt(appMesoModel()->count() - 1, 8) == 1)
			minimumStartDate = appUtils()->getMesoStartDate(appMesoModel()->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		endDate = appUtils()->createFutureDate(minimumStartDate, 0, 2, 0);
	}

	const uint meso_idx = appMesoModel()->newMesocycle(QStringList() << STR_MINUS_ONE << tr("New Plan") << QString::number(startDate.toJulianDay()) <<
		QString::number(endDate.toJulianDay()) << QString() << QString::number(appUtils()->calculateNumberOfWeeks(startDate, endDate)) <<
		u"ABCDERR"_qs << QString() << appUserModel()->userName(0) << QString() << QString() << STR_ONE);

	if (bCreatePage)
	{
		QmlItemManager* itemMngr = new QmlItemManager(meso_idx, this);
		m_itemManager.append(itemMngr);
		itemMngr->createMesocyclePage(minimumStartDate, appUtils()->createFutureDate(startDate,0,6,0));
	}
	return meso_idx;
}

void DBInterface::saveMesocycle(const uint meso_idx)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appMesoModel()));

	if (appMesoModel()->getIntFast(meso_idx, MESOCYCLES_COL_ID) == -1)
	{
		if (mb_importMode)
			worker->setWaitForThreadToFinish(true);

		connect( this, &DBInterface::databaseReady, this, [&,meso_idx,worker] (const uint db_id) {
			if (db_id == worker->uniqueID()) {
				appMesoModel()->mesoSplitModel()->setFast(meso_idx, 1, appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
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
	if (appMesoModel()->getIntFast(meso_idx, MESOCYCLES_COL_ID))
	{
		removeMesoCalendar(meso_idx);
		removeMesoSplit(meso_idx);
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath));
		worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
		createThread(worker, [worker] () { return worker->removeEntry(); });
	}

	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	QmlItemManager* tpObject(itemMngr);
	m_itemManager.remove(meso_idx);
	tpObject->disconnect();
	delete tpObject;

	appMesoModel()->delMesocycle(meso_idx);
}

//Cannot order a mesocycle removal from within the qml page that will be deleted. Defer the removal to a little time after,
//just enough to have StackView remove the page
void DBInterface::scheduleMesocycleRemoval(const uint meso_idx)
{
	QTimer::singleShot(200, this, [&,meso_idx] () { removeMesocycle(meso_idx); } );
}

void DBInterface::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appMesoModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DBInterface::saveMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appMesoModel()->mesoSplitModel()));
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesoSplit(); } );
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	itemMngr->updateMuscularGroup(appMesoModel()->mesoSplitModel());
}

void DBInterface::removeMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath));
	worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appMesoModel()->mesoSplitModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}

void DBInterface::getExercisesPlannerPage(QmlItemManager* const itemMngr)
{
	getCompleteMesoSplit(itemMngr->mesoIdx());
	itemMngr->getExercisesPlannerPage();
}

void DBInterface::loadCompleteMesoSplits(const uint meso_idx, const bool bThreaded)
{
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	const QString mesoSplit(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT));
	QString mesoLetters;
	DBMesoSplitModel* splitModel(nullptr);
	uint nSplits(appMesoModel()->totalSplits(meso_idx));
	DBMesoSplitTable* worker2(nullptr);
	bool connected(false);

	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());

	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;

		mesoLetters.append(static_cast<QChar>(*itr));
		splitModel = itemMngr->getSplitModel(static_cast<QChar>(*itr));

		if (bThreaded)
		{
			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, splitModel));
			worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
			worker->addExecArg(static_cast<QChar>(*itr));
			if (!connected)
			{
				connect( this, &DBInterface::databaseReady, this, [&,nSplits] (const uint db_id) mutable {
					MSG_OUT("loadCompleteMesoSplits received databaseReady() " << db_id)
					if (m_WorkerLock[MESOSPLIT_TABLE_ID].hasID(db_id))
					{
						if (--nSplits == 0)
						{
							mb_splitsLoaded = true;
							emit internalSignal(SPLITS_LOADED_ID);
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
				worker2->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
				worker2->addExecArg(static_cast<QChar>(*itr));
			}
			else
			{
				worker2->changeExecArg(static_cast<QChar>(*itr), 1);
				worker2->setModel(splitModel);
			}
			worker2->getCompleteMesoSplit(false);
		}
	} while (++itr != itr_end);
	if (worker2)
		delete worker2;
}

void DBInterface::getCompleteMesoSplit(const uint meso_idx)
{
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	if (!mb_splitsLoaded)
	{
		//connect( this, &DBInterface::internalSignal, this, [&] (const uint id) { if (id == SPLITS_LOADED_ID) {
		//		return itemMngr->createMesoSplitPage(); disconnect(this, &DBInterface::internalSignal, this, nullptr); } } );
		loadCompleteMesoSplits(meso_idx);
	}
	itemMngr->createMesoSplitPage();
}

void DBInterface::saveMesoSplitComplete(DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, model));
	worker->addExecArg(model->mesoIdx());
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); });
}

bool DBInterface::mesoHasPlan(const uint meso_id, const QString& splitLetter) const
{
	if (splitLetter != u"R"_qs)
	{
		DBMesoSplitTable* meso_split(new DBMesoSplitTable(m_DBFilePath));
		const bool ret(meso_split->mesoHasPlan(QString::number(meso_id), splitLetter));
		meso_split->deleteLater();
		return ret;
	}
	return false;
}

void DBInterface::loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, model));
	worker->addExecArg(QString::number(prev_meso_id));
	worker->addExecArg(model->splitLetter().at(0));
	createThread(worker, [worker] () { worker->getCompleteMesoSplit(); });
}

static void muscularGroupSimplified(QString& muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList words(muscularGroup.split(' '));

	if ( words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator itr_end(words.end());
		muscularGroup.clear();

		do
		{
			if(static_cast<QString>(*itr).length() < 3)
				continue;
			if (!muscularGroup.isEmpty())
				muscularGroup.append(' ');
			muscularGroup.append(static_cast<QString>(*itr).toLower());
			if (muscularGroup.endsWith('s', Qt::CaseInsensitive) )
				muscularGroup.chop(1);
			muscularGroup.remove('.');
			muscularGroup.remove('(');
			muscularGroup.remove(')');
		} while (++itr != itr_end);
	}
}

QString DBInterface::checkIfSplitSwappable(const DBMesoSplitModel* const splitModel) const
{
	const uint meso_idx(splitModel->mesoIdx());
	const QString splitLetter(splitModel->splitLetter());
	if (mesoHasPlan(appMesoModel()->getIntFast(meso_idx, MESOCYCLES_COL_ID), splitLetter))
	{
		QString muscularGroup1(appMesoModel()->getMuscularGroup(meso_idx, splitLetter));
		if (!muscularGroup1.isEmpty())
		{
			QString muscularGroup2;
			const QString mesoSplit(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT));
			muscularGroupSimplified(muscularGroup1);

			QString::const_iterator itr(mesoSplit.constBegin());
			const QString::const_iterator itr_end(mesoSplit.constEnd());

			do {
				if (static_cast<QChar>(*itr) == QChar('R'))
					continue;
				else if (static_cast<QChar>(*itr) == splitLetter.at(0))
					continue;

				muscularGroup2 = appMesoModel()->getMuscularGroup(meso_idx, *itr);
				if (!muscularGroup2.isEmpty())
				{
					muscularGroupSimplified(muscularGroup2);
					if (appUtils()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
						return QString(*itr);
				}
			} while (++itr != itr_end);
		}
	}
	return QString();
}

void DBInterface::swapMesoPlans(QmlItemManager* const itemMngr, const QString& splitLetter1, const QString& splitLetter2)
{
	itemMngr->swapPlans(splitLetter1, splitLetter2);
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, itemMngr->getSplitModel(splitLetter1.at(0))));
	worker->addExecArg(appMesoModel()->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); } );
	DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, itemMngr->getSplitModel(splitLetter2.at(0))));
	worker2->addExecArg(appMesoModel()->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
	createThread(worker2, [worker2] () { worker2->saveMesoSplitComplete(); });
}

void DBInterface::exportMesoSplit(const QmlItemManager* const itemMngr, const QString& splitLetter, const bool bShare, QFile* outFileInUse)
{
	QString mesoSplit;
	QString suggestedName;
	QFile* outFile(nullptr);
	const uint meso_idx(itemMngr->mesoIdx());

	if (!outFileInUse)
	{
		if (splitLetter == u"X"_qs)
		{
			mesoSplit = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
			suggestedName = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_NAME) + tr(" - Exercises Plan.txt");
		}
		else
		{
			mesoSplit = splitLetter;
			suggestedName = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_NAME) +
								tr(" - Exercises Plan - Split ") + splitLetter + u".txt"_qs;
		}
		setExportFileName(suggestedName);
	}
	else
	{
		outFile = outFileInUse;
		mesoSplit = appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
	}

	QString mesoLetters;
	bool bExportToFileOk(true);

	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());
	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;
		mesoLetters.append(static_cast<QChar>(*itr));
		bExportToFileOk &= exportToFile(const_cast<QmlItemManager*>(itemMngr)->getSplitModel(static_cast<QChar>(*itr)),
											exportFileName(), outFile);
	} while (++itr != itr_end);

	if (outFileInUse)
		return;

	if (bExportToFileOk)
	{
		#ifdef Q_OS_ANDROID
		/*setExportFileName("app_logo.png");
		if (!QFile::exists(exportFileName()))
		{
			QFile::copy(":/images/app_logo.png", exportFileName());
			QFile::setPermissions(exportFileName(), QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
		}
		sendFile(exportFileName(), tr("Send file"), u"image/png"_qs, 10);*/
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
	}
}
//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
void DBInterface::getMesoCalendar(const uint meso_idx, const bool bCreatePage)
{
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx)));
		worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
		if (bCreatePage)
		{
			connect( this, &DBInterface::databaseReady, this, [&,worker,meso_idx,bCreatePage] (const uint db_id) {
				if (db_id == worker->uniqueID()) return getMesoCalendar(meso_idx, bCreatePage); } );
		}
		createThread(worker, [worker] () { worker->getMesoCalendar(); });
		return;
	}
	if (bCreatePage)
		m_itemManager.at(meso_idx)->getCalendarPage();
}

void DBInterface::changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore)
{
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,meso_idx,bPreserveOldInfo,bPreserveOldInfoUntilDayBefore] ()
		{
			return changeMesoCalendar(meso_idx, bPreserveOldInfo, bPreserveOldInfoUntilDayBefore);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx)));
	const QDate endDate(bPreserveOldInfo && bPreserveOldInfoUntilDayBefore ?
					QDate::currentDate() :
					appMesoModel()->getDate(meso_idx, MESOCYCLES_COL_ENDDATE));
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilDayBefore);
	worker->addExecArg(endDate);
	if (!bPreserveOldInfo)
	{
		connect( this, &DBInterface::databaseReady, this, [&,worker,meso_idx,endDate] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_itemManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(meso_idx,
								appMesoModel()->getDateFast(meso_idx, MESOCYCLES_COL_STARTDATE), endDate);
		});
	}
	createThread(worker, [worker] () { worker->changeMesoCalendar(); });
}

void DBInterface::updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,tDayModel] () {
				return updateMesoCalendarModel(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx)));
	worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID)); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->splitLetter());
	connect( this, &DBInterface::databaseReady, this, [&,worker,tDayModel] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_itemManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(tDayModel->mesoIdx(),
					tDayModel->getDateFast(0, TDAY_COL_DATE), appMesoModel()->getDateFast(tDayModel->mesoIdx(), MESOCYCLES_COL_ENDDATE));
	});
	createThread(worker, [worker] () { worker->updateMesoCalendar(); });
}

void DBInterface::updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(tDayModel->mesoIdx())));
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->trainingDay());
	worker->addExecArg(tDayModel->splitLetter());
	worker->addExecArg(tDayModel->dayIsFinished() ? STR_ONE : STR_ZERO);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DBInterface::setDayIsFinished(DBTrainingDayModel* const tDayModel, const bool bFinished)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!appMesoModel()->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,tDayModel,bFinished] () {
			return setDayIsFinished(tDayModel, bFinished);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	tDayModel->setDayIsFinished(bFinished);
	const QDate date(tDayModel->getDateFast(0, TDAY_COL_DATE));
	appMesoModel()->mesoCalendarModel(meso_idx)->setDayIsFinished(date, bFinished);
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appMesoModel()->mesoCalendarModel(meso_idx)));
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
void DBInterface::getTrainingDay(const uint meso_idx, const QDate& date)
{
	if (appMesoModel()->mesoCalendarModel(meso_idx)->count() == 0)
	{
		connect( this, &DBInterface::databaseReady, this, [&,meso_idx,date] () {
			getTrainingDay(meso_idx, date);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	QmlItemManager* itemMngr(itemManager(meso_idx));
	if (!itemMngr->currenttDayModel()->isReady())
	{
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, itemMngr->gettDayModel(date)));
		worker->addExecArg(QString::number(date.toJulianDay()));
		createThread(worker, [worker] () { return worker->getTrainingDay(); });
	}
	if (itemMngr->currenttDayModel()->exerciseCount() == 0)
		getTrainingDayExercises(meso_idx, date);
	itemMngr->getTrainingDayPage(date);
}

void DBInterface::getTrainingDayExercises(const uint meso_idx, const QDate& date)
{
	const DBTrainingDayModel* const tDayModel(m_itemManager.at(meso_idx)->currenttDayModel());
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, const_cast<DBTrainingDayModel*>(tDayModel)));
	worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DBInterface::databaseReady, this, [&,meso_idx,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) return verifyTDayOptions(tDayModel); });
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DBInterface::verifyTDayOptions(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx(tDayModel->mesoIdx());
	QmlItemManager* itemMngr(m_itemManager.at(meso_idx));
	if (itemMngr->currenttDayModel()->exerciseCount() > 0)
		itemMngr->createExercisesObjects();
	else
	{
		/*const QString splitletter(splitLetter.isEmpty() ?
			appMesoModel()->mesoCalendarModel(meso_idx)->getSplitLetter(date.month(), date.day()-1) :
			splitLetter);*/
		if (tDayModel->splitLetter() >= u"A"_qs && tDayModel->splitLetter() <= u"F"_qs)
		{
			DBTrainingDayModel* tempModel{new DBTrainingDayModel(this, meso_idx)};
			DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tempModel)};
			worker->addExecArg(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
			worker->addExecArg(tDayModel->splitLetter());
			worker->addExecArg(tDayModel->getFast(0, TDAY_COL_DATE));
			createThread(worker, [worker] () { return worker->getPreviousTrainingDaysInfo(); } );
		}
	}
}

void DBInterface::clearExercises(QmlItemManager* const itemMngr)
{
	itemMngr->clearExercises();
	verifyTDayOptions(itemMngr->currenttDayModel());
}

void DBInterface::loadExercisesFromDate(QmlItemManager* const itemMngr, const QString& strDate)
{
	DBTrainingDayModel* tDayModel(itemMngr->currenttDayModel());
	const QDate date(appUtils()->getDateFromStrDate(strDate));
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	worker->addExecArg(appMesoModel()->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
	worker->addExecArg(QString::number(date.toJulianDay()));

	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect( this, &DBInterface::databaseReady, this, [&,itemMngr,tDayModel,date,worker] (const uint db_id) {
		if (db_id == worker->uniqueID())
		{
			if (tDayModel->getDateFast(0, TDAY_COL_DATE) != QDate::currentDate())
				tDayModel->setDayIsFinished(true);
			else
				tDayModel->setModified(true);
			return itemMngr->createExercisesObjects();
		}
	});
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(true); });
}

void DBInterface::loadExercisesFromMesoPlan(QmlItemManager* const itemMngr, const QString& splitLetter)
{
	const QChar splitletter(splitLetter.at(0));
	if (!mb_splitsLoaded)
	{
		connect( this, &DBInterface::internalSignal, this, [&,itemMngr,splitLetter] (const uint id) {
			if (id == SPLITS_LOADED_ID)
				loadExercisesFromMesoPlan(itemMngr, splitLetter);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(itemMngr->mesoIdx());
	}
	else
	{
		DBTrainingDayModel* tDayModel(itemMngr->currenttDayModel());
		tDayModel->convertMesoSplitModelToTDayModel(itemMngr->getSplitModel(splitletter));
		itemMngr->createExercisesObjects();
		if (tDayModel->getDateFast(0, TDAY_COL_DATE) != QDate::currentDate())
			tDayModel->setDayIsFinished(true);
	}
}

void DBInterface::convertTDayToPlan(const DBTrainingDayModel* const tDayModel)
{
	if (!mb_splitsLoaded)
	{
		connect( this, &DBInterface::internalSignal, this, [&,tDayModel] (const uint id) {
			if (id == SPLITS_LOADED_ID)
				return convertTDayToPlan(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(tDayModel->mesoIdx());
	}
	else
	{
		QmlItemManager* itemMngr(m_itemManager.at(tDayModel->mesoIdx()));
		DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, itemMngr->getSplitModel(tDayModel->splitLetter().at(0))));
		worker->addExecArg(appMesoModel()->getFast(tDayModel->mesoIdx(), MESOCYCLES_COL_ID));
		worker->addExecArg(tDayModel->splitLetter());
		createThread(worker, [worker,tDayModel] () { return worker->convertTDayExercisesToMesoPlan(tDayModel); });
	}
}

void DBInterface::saveTrainingDay(DBTrainingDayModel* const tDayModel)
{
	if (tDayModel->modified())
	{
		DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
		createThread(worker, [worker] () { return worker->saveTrainingDay(); });
	}
}

void DBInterface::removeTrainingDay(const uint meso_idx)
{
	DBTrainingDayModel* tDayModel(m_itemManager.at(meso_idx)->currenttDayModel());
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DBInterface::deleteTrainingDayTable(const uint meso_idx, const bool bRemoveFile)
{
	DBTrainingDayModel* tDayModel(m_itemManager.at(meso_idx)->currenttDayModel());
	DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tDayModel)};
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DBInterface::exportTrainingDay(const DBTrainingDayModel* tDayModel, const bool bShare)
{
	const QString suggestedName(tr(" - Workout ") + tDayModel->splitLetter() + u".txt"_qs);
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	if (exportToFile(tDayModel, exportFileName(), outFile))
	{
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
	}
}

uint DBInterface::getWorkoutNumberForTrainingDay(const DBTrainingDayModel* const tDayModel) const
{
	return appMesoModel()->mesoCalendarModel(tDayModel->mesoIdx())->getLastTrainingDayBeforeDate(tDayModel->getDateFast(0, TDAY_COL_DATE)) + 1;
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
