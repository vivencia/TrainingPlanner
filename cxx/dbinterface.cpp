#include "dbinterface.h"
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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#define SPLITS_LOADED_ID 4321

static DBUserModel appUserModel;
static DBMesocyclesModel appMesocyclesModel;
static DBExercisesModel appExercisesModel;

#ifdef Q_OS_ANDROID

#include "urihandler.h"
#include "tpandroidnotification.h"

#include <QJniObject>
#include <QtGlobal>
#include <qnativeinterface.h>
#if QT_VERSION == QT_VERSION_CHECK(6, 7, 2)
#include <QtCore/6.7.2/QtCore/private/qandroidextras_p.h>
#else
#include <QtCore/6.6.3/QtCore/private/qandroidextras_p.h>
#endif

void DBInterface::checkPendingIntents() const
{
	QJniObject activity = QNativeInterface::QAndroidApplication::context();
	if(activity.isValid())
	{	
		activity.callMethod<void>("checkPendingIntents","()V");
		return;
	}
	MSG_OUT("checkPendingIntents: Activity not valid")
}

/*
 * As default we're going the Java - way with one simple JNI call (recommended)
 * if altImpl is true we're going the pure JNI way
 * HINT: we don't use altImpl anymore
 *
 * If a requestId was set we want to get the Activity Result back (recommended)
 * We need the Request Id and Result Id to control our workflow
*/
bool DBInterface::sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const
{
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	QJniObject jsMimeType = QJniObject::fromString(mimeType);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendFile",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>(), jsMimeType.object<jstring>(), requestId);
	if(!ok)
	{
		MSG_OUT("Unable to resolve activity from Java")
		return false;
	}
	return true;
}

void DBInterface::androidOpenURL(const QString& address) const
{
	QString url;
	if (!address.startsWith(u"http"_qs))
		url = u"https://" + address;
	else
		url = address;

	QJniObject jsPath = QJniObject::fromString(url);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"openURL",
													"(Ljava/lang/String;)Z",
													jsPath.object<jstring>());
	if(!ok)
		MSG_OUT("Unable to open the address: " << address)
}

bool DBInterface::androidSendMail(const QString& address, const QString& subject, const QString& attachment) const
{
	const QString attachment_file(attachment.isEmpty() ? QString() : u"file://" + attachment);
	QJniObject jsAddress = QJniObject::fromString(address);
	QJniObject jsSubject = QJniObject::fromString(subject);
	QJniObject jsAttach = QJniObject::fromString(attachment_file);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendEmail",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
													jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>());
	return ok;
}

bool DBInterface::viewFile(const QString& filePath, const QString& title) const
{
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"viewFile",
													"(Ljava/lang/String;Ljava/lang/String;)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>());
	if(!ok)
	{
		MSG_OUT("Unable to resolve view activity from Java")
		return false;
	}
	return true;
}

void DBInterface::appStartUpNotifications()
{
	m_AndroidNotification = new TPAndroidNotification(this);
	if (mesocyclesModel->count() > 0)
	{
		DBMesoCalendarTable* calTable(new DBMesoCalendarTable(m_DBFilePath));
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			if (dayInfoList.at(0).toUInt() == m_currentMesoManager->mesoId())
			{
				QString message;
				const QString splitLetter(dayInfoList.at(2));
				if (splitLetter != u"R"_qs) //day is training day
				{
					if (dayInfoList.at(3) == u"1"_qs) //day is completed
						message = tr("Your training routine seems to go well. Workout for the day is concluded");
					else
						message = tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter;
				}
				else
					message = tr("Enjoy your day of rest from workouts!");
				m_AndroidNotification->sendNotification(u"Training Planner"_qs, message, WORKOUT_NOTIFICATION);
			}
		}
		delete calTable;
	}
}
#else

#include <QProcess>
extern "C"
{
	#include <unistd.h>
}
#endif

DBInterface::DBInterface()
	: QObject (nullptr), mb_splitsLoaded(false), mb_importMode(false)
{
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

void DBInterface::init(QQmlApplicationEngine* qml_engine)
{
	TPUtils::app_qml_engine = qml_engine;
	m_DBFilePath = appSettings()->value("dbFilePath").toString();
	if (m_DBFilePath.isEmpty())
	{
		m_DBFilePath = appUtils()->getAppDir(appQmlEngine()->offlineStoragePath());
		appSettings()->setValue("dbFilePath", m_DBFilePath);
		appUtils()->populateSettingsWithDefaultValue();
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
	exercisesModel = &appExercisesModel;
	if (m_exercisesListVersion != appSettings()->value("exercisesListVersion").toString())
		updateExercisesList();

	getAllUsers();
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

	QmlItemManager::configureQmlEngine(this);
	mAppDataFilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/"_qs;
#ifdef Q_OS_ANDROID
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &TPUtils::appResumed, this, &DBInterface::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(appMainWindow(), "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	appStartUpNotifications();
#endif
}

void DBInterface::cleanUp()
{
	#ifdef Q_OS_ANDROID
	delete m_AndroidNotification;
	#endif
	cleanUpThreads();
	for(uint i(0); i < m_itemManager.count(); ++i)
		delete m_itemManager.at(i);
}

void DBInterface::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
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
						mesoHasPlan(mesocyclesModel->getIntFast(tempModel->mesoIdx(), MESOCYCLES_COL_ID), tempModel->splitLetter()) ?
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

#ifndef Q_OS_ANDROID
void DBInterface::processArguments()
{
	const QStringList args(qApp->arguments());
	if (args.count() > 1)
	{
		QString filename;
		for (uint i(1); i < args.count(); ++i)
			filename += args.at(i) + ' ';
		filename.chop(1);
		QFileInfo file(filename);
		if (file.isFile())
			openRequestedFile(filename);
	}
}

void DBInterface::restartApp()
{
	char* args[2] = { nullptr, nullptr };
	const QString argv0(qApp->arguments().at(0));
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	exitApp();
}
#endif

void DBInterface::openRequestedFile(const QString &filename)
{
	const QString nameOnly(filename.right(filename.length() - filename.lastIndexOf('/') - 1));
	QMetaObject::invokeMethod(appMainWindow(), "tryToOpenFile", Q_ARG(QString, filename), Q_ARG(QString, nameOnly));
}

bool DBInterface::exportToFile(const TPListModel* const model, const QString& filename, QFile* &outFile) const
{
	QString fname(filename);
	if (filename.startsWith(u"file:"_qs))
		fname.remove(0, 7); //remove file://

	if (!outFile)
	{
		outFile = new QFile(fname);
		outFile->deleteLater();
	}
	if (outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text))
	{
		model->exportToText(outFile);
		outFile->close();
		return true;
	}
	return false;
}

/*Return values
 *	 0: success
 *	-1: Failed to open file
 *	-2: File format was not recognized
 *	-3: Nothing was imported, either because file was missing info or error in formatting
 *	-4: File has been previously imported
 */
int DBInterface::importFromFile(QString filename, QFile* inFile)
{
	if (!inFile)
	{
		if (filename.startsWith(u"file:"_qs))
			filename.remove(0, 7); //remove file://
		inFile = new QFile(filename, this);
		inFile->deleteLater();
		if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
			return -1;
	}

	TPListModel* model(nullptr);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 2)
		{
			inData = buf;
			break;
		}
	}

	const bool bFancy(!inData.startsWith(u"##0x"_qs));
	int sep_idx(0);

	if (bFancy)
	{
		if (inData.indexOf(DBMesoSplitObjectName) != -1)
			model = new DBMesoSplitModel(this);
		else if (inData.indexOf(DBMesocyclesObjectName) != -1)
			model = new DBMesocyclesModel(this);
		//else if (inData.indexOf(DBTrainingDayObjectName) != -1)
			//model = new DBTrainingDayModel(this);
		else if (inData.indexOf(DBExercisesObjectName) != -1)
			model = new DBExercisesModel(this);
		else
			return -2;

		while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
		{
			if (lineLength > 2)
			{
				inData = buf;
				break;
			}
		}
	}
	else
	{
		inData = inData.left(2);
		inData.chop(1);
		switch (inData.toUInt())
		{
			case EXERCISES_TABLE_ID: model = new DBExercisesModel(this); break;
			case MESOCYCLES_TABLE_ID: model = new DBMesocyclesModel(this); break;
			case MESOSPLIT_TABLE_ID: model = new DBMesoSplitModel(this); break;
			//case TRAININGDAY_TABLE_ID: model = new DBTrainingDayModel(this); break;
			default:
				return -2;
		}
	}

	model->deleteLater();
	if (!model->importExtraInfo(inData))
		return -4;

	if (model->importFromText(inFile, inData))
	{
		if (!importFromModel(model))
			return -4;
	}

	if (!inFile->atEnd())
		return importFromFile(filename, inFile);
	else
		return 0;
}

bool DBInterface::importFromModel(TPListModel* model)
{
	mb_importMode = true;
	bool bOK(true);
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			updateExercisesList(static_cast<DBExercisesModel*>(model));
		break;
		case MESOCYCLES_TABLE_ID:
			if (mesocyclesModel->isDifferent(static_cast<DBMesocyclesModel*>(model)))
			{
				const uint meso_idx = createNewMesocycle(false);
				for (uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_TOTAL_COLS; ++i)
					mesocyclesModel->setFast(meso_idx, i, model->getFast(0, i));
				saveMesocycle(meso_idx);
				emit mesocyclesModel->currentRowChanged(); //notify main.qml::btnWorkout to evaluate its enabled state
			}
			else
				bOK = false;
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* splitModel = static_cast<DBMesoSplitModel*>(model);
			const uint meso_idx = splitModel->mesoIdx();
			QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
			if (splitModel->completeSplit())
			{
				DBMesoSplitModel* mesoSplitModel(itemMngr->getSplitModel(splitModel->splitLetter().at(0)));
				if (mesoSplitModel->updateFromModel(splitModel))
				{
					saveMesoSplitComplete(mesoSplitModel);
					// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
					// by the time mb_splitsLoaded is ever checked upon
					mb_splitsLoaded = true;
				}
				else
					bOK = false;
			}
			else
			{
				for (uint i(0); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
					mesocyclesModel->mesoSplitModel()->setFast(meso_idx, i, splitModel->getFast(0, i));
				mesocyclesModel->mesoSplitModel()->setFast(meso_idx, 1, mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
				saveMesoSplit(meso_idx);
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			const uint meso_idx = static_cast<DBTrainingDayModel*>(model)->mesoIdx();
			DBTrainingDayModel* tDayModel(m_itemManager.at(meso_idx)->gettDayModel(dayDate));
			if (tDayModel->updateFromModel(model))
				getTrainingDay(meso_idx, dayDate);
			else
				bOK = false;
		}
		break;
	}
	mb_importMode = false;
	return bOK;
}

void DBInterface::saveFileDialogClosed(QString finalFileName, bool bResultOK)
{
	int resultCode(-12);
	if (finalFileName.startsWith(u"file:"_qs))
		finalFileName.remove(0, 7); //remove file://
	if (bResultOK)
	{
		bResultOK = QFile::copy(exportFileName(), finalFileName);
		resultCode = bResultOK ? 3 : -10;
	}
	QFile::remove(exportFileName());
	appMainWindow()->setProperty("importExportFilename", finalFileName);
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, resultCode));
}

int DBInterface::parseFile(QString filename)
{
	if (filename.startsWith(u"file:"_qs))
		filename.remove(0, 7); //remove file://
	QFile* inFile(new QFile(filename, this));
	inFile->deleteLater();
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
		return -1;

	qint64 lineLength(0);
	char buf[128];
	QString inData;
	QString tableMessage;
	bool createMessage[4] = { false };

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (!inData.startsWith(u"##0x"_qs)) //Fancy
				{
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
					{
						if (createMessage[1] && createMessage[0])
							continue;
						createMessage[0] = true;
					}
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						createMessage[1] = true;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						createMessage[2] = true;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						createMessage[3] = true;
					else
						return -2;
				}
				else
				{
					inData = inData.left(2);
					inData.chop(1);
					switch (inData.toUInt())
					{
						case EXERCISES_TABLE_ID: createMessage[3] = true; break;
						case MESOCYCLES_TABLE_ID: createMessage[1] = true; break;
						case MESOSPLIT_TABLE_ID:
							if (createMessage[1] && createMessage[0])
								continue;
							createMessage[0] = true;
						break;
						case TRAININGDAY_TABLE_ID: createMessage[3] = true; break;
						default: return -2;
					}
				}
				if (createMessage[0])
				{
					if (tableMessage.isEmpty())
						tableMessage = tr("a new Training Split Exercise Plan");
					else
					{
						if (createMessage[1])
							tableMessage += tr("new Training Split Exercise Plans");
						else
							tableMessage = tr("new Training Split Exercise Plans");
					}
				}
				else if (createMessage[1])
					tableMessage = tr("an entire Mesocycle Plan, including ");
				else if (createMessage[2])
					tableMessage = tr("One Training Day");
				else if (createMessage[3])
				{
					if (!createMessage[1])
						tableMessage = tr("An updated exercises database list");
					else
						tableMessage.append(tr("and an updated exercises database list"));
				}
			}
		}
	}

	if (createMessage[0] || createMessage[1] || createMessage[2] || createMessage[3])
	{
		if (!createMessage[1] && mesocyclesModel->count() == 0)
			return -5;
		const QString message(tr("This will import data to create: %1"));
		QMetaObject::invokeMethod(appMainWindow(), "confirmImport", Q_ARG(QString, message.arg(tableMessage)));
		return 1;
	}
	else
		return -3;
}

void DBInterface::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	if (!mb_splitsLoaded)
		loadCompleteMesoSplits(false);
	const QString suggestedName(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME) + tr(" - TP Complete Meso.txt"));
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	mesocyclesModel->setExportRow(meso_idx);

	if (bCoachInfo)
	{
		userModel->setExportRow(userModel->getRowByCoachName(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_COACH)));
		exportToFile(userModel, exportFileName(), outFile);
	}

	if (exportToFile(mesocyclesModel, exportFileName(), outFile))
	{
		mesocyclesModel->mesoSplitModel()->setExportRow(meso_idx);
		exportToFile(mesocyclesModel->mesoSplitModel(), QString(), outFile);
		exportMesoSplit(itemManager(meso_idx), u"X"_qs, bShare, outFile);

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

void DBInterface::openURL(const QString& address) const
{
	#ifdef Q_OS_ANDROID
	androidOpenURL(address);
	#else
	auto* __restrict proc(new QProcess());
	proc->startDetached(u"xdg-open"_qs, QStringList() << address);
	delete proc;
	#endif
}

void DBInterface::startChatApp(const QString& phone, const QString& appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	QString::const_iterator itr(phone.constBegin());
	const QString::const_iterator itr_end(phone.constEnd());
	do {
		if ((*itr).isDigit())
			phoneNumbers += *itr;
	} while (++itr != itr_end);

	QString address;
	if (appname.contains(u"Whats"_qs))
		address = u"https://wa.me/"_qs + phoneNumbers;
	else
		address = u"https://t.me/+"_qs + phoneNumbers;

	openURL(address);
}

void DBInterface::sendMail(const QString& address, const QString& subject, const QString& attachment_file) const
{
	#ifdef Q_OS_ANDROID
	if (!androidSendMail(address, subject, attachment_file))
	{
		if (userModel->email(0).contains(u"gmail.com"_qs))
		{
			const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(userModel->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList args (QStringList() <<
		u"--utf8"_qs << u"--subject"_qs << QChar('\'') + subject + QChar('\'') << u"--attach"_qs << attachment_file <<
			QChar('\'') + address + QChar('\''));
	auto* __restrict proc(new QProcess ());
	proc->start(u"xdg-email"_qs, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exitCode, QProcess::ExitStatus)
	{
		if (exitCode != 0)
		{
			if (userModel->email(0).contains(u"gmail.com"_qs))
			{
				const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(userModel->email(0), address, subject));
				openURL(gmailURL);
			}
		}
		proc->deleteLater();
	});
	#endif
}

void DBInterface::viewExternalFile(const QString& filename) const
{
	if (!appUtils()->canReadFile(appUtils()->getCorrectPath(filename)))
		return;
	#ifdef Q_OS_ANDROID
	const QString localFile(mAppDataFilesPath + u"tempfile"_qs + filename.right(4));
	static_cast<void>(QFile::remove(localFile));
	if (QFile::copy(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "coud not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}

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
	userModel = &appUserModel;
	DBUserTable worker{m_DBFilePath, userModel};
	worker.getAllUsers();

	bool noUsers(userModel->count() == 0);
	if (!noUsers)
		noUsers = userModel->userName(0).isEmpty();
	if (noUsers)
	{
		userModel->setIsEmpty(true);
		userModel->addUser(false);
	}
}

void DBInterface::saveUser(const uint row)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, userModel));
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); } );
}

void DBInterface::removeUser(const uint row, const bool bCoach)
{
	if (row >= 1)
	{
		DBUserTable* worker(new DBUserTable(m_DBFilePath, userModel));
		worker->addExecArg(userModel->userId(row));
		createThread(worker, [worker] () { return worker->removeEntry(); } );
		m_itemManager.at(0)->removeUser(row, bCoach);
	}
}

void DBInterface::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, userModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DBInterface::getAllExercises()
{
	if (exercisesModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
		worker->setUniqueID(2222);
		createThread(worker, [worker] () { worker->getAllExercises(); } );
	}
	else
		emit databaseReady(2222);
}

void DBInterface::getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (exercisesModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
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
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->saveExercise(); } );
}

void DBInterface::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->removeEntry(); } );
}

void DBInterface::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DBInterface::updateExercisesList(DBExercisesModel* model)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, exercisesModel));
	if (!model)
		createThread(worker, [worker] () { return worker->updateExercisesList(); } );
	else
	{
		worker->addExecArg(QVariant::fromValue(model));
		createThread(worker, [worker] () { return worker->updateFromModel(); } );
	}
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
	if (!exercisesModel->collectExportData())
	{
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -6));
		return;
	}
	QFile* outFile(nullptr);
	if (exportToFile(exercisesModel, exportFileName(), outFile))
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
	mesocyclesModel = &appMesocyclesModel;
	mesocyclesModel->setUserModel(appUserModel);
	DBMesocyclesTable worker{m_DBFilePath, mesocyclesModel};
	worker.getAllMesocycles();

	const uint n_mesos(mesocyclesModel->count());
	if (n_mesos > 0)
	{
		DBMesoSplitTable worker2{m_DBFilePath, mesocyclesModel->mesoSplitModel()};
		worker2.getAllMesoSplits();

		m_itemManager.reserve(n_mesos);
		for (uint i(0); i < n_mesos; ++i)
			m_itemManager.append(new QmlItemManager(i, this));
	}
}

void DBInterface::getMesocyclePage(const uint meso_idx)
{
	mesocyclesModel->setCurrentMesoIdx(meso_idx);
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	itemMngr->getMesoPage();
}

uint DBInterface::createNewMesocycle(const bool bCreatePage)
{
	QDate startDate, endDate, minimumStartDate;
	if (mesocyclesModel->count() == 0)
	{
		minimumStartDate.setDate(2023, 0, 2); //first monday of that year
		startDate = QDate::currentDate();
		endDate = appUtils()->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (mesocyclesModel->getInt(mesocyclesModel->count() - 1, 8) == 1)
			minimumStartDate = appUtils()->getMesoStartDate(mesocyclesModel->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		endDate = appUtils()->createFutureDate(minimumStartDate, 0, 2, 0);
	}

	const uint meso_idx = mesocyclesModel->newMesocycle(QStringList() << STR_MINUS_ONE << tr("New Plan") << QString::number(startDate.toJulianDay()) <<
		QString::number(endDate.toJulianDay()) << QString() << QString::number(appUtils()->calculateNumberOfWeeks(startDate, endDate)) <<
		u"ABCDERR"_qs << QString() << userModel->userName(0) << QString() << QString() << STR_ONE);

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
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, mesocyclesModel));

	if (mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID) == -1)
	{
		if (mb_importMode)
			worker->setWaitForThreadToFinish(true);

		connect( this, &DBInterface::databaseReady, this, [&,meso_idx,worker] (const uint db_id) {
			if (db_id == worker->uniqueID()) {
				mesocyclesModel->mesoSplitModel()->setFast(meso_idx, 1, mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
	if (mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID))
	{
		removeMesoCalendar(meso_idx);
		removeMesoSplit(meso_idx);
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
		createThread(worker, [worker] () { return worker->removeEntry(); });
	}

	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	QmlItemManager* tpObject(itemMngr);
	m_itemManager.remove(meso_idx);
	tpObject->disconnect();
	delete tpObject;

	mesocyclesModel->delMesocycle(meso_idx);
}

//Cannot order a mesocycle removal from within the qml page that will be deleted. Defer the removal to a little time after,
//just enough to have StackView remove the page
void DBInterface::scheduleMesocycleRemoval(const uint meso_idx)
{
	QTimer::singleShot(200, this, [&,meso_idx] () { removeMesocycle(meso_idx); } );
}

void DBInterface::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, mesocyclesModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DBInterface::saveMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, mesocyclesModel->mesoSplitModel()));
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesoSplit(); } );
	QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
	itemMngr->updateMuscularGroup(mesocyclesModel->mesoSplitModel());
}

void DBInterface::removeMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DBInterface::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, mesocyclesModel->mesoSplitModel()));
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
	const QString mesoSplit(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_SPLIT));
	QString mesoLetters;
	DBMesoSplitModel* splitModel(nullptr);
	uint nSplits(mesocyclesModel->totalSplits(meso_idx));
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
			worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
				worker2->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
	if (mesoHasPlan(mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID), splitLetter))
	{
		QString muscularGroup1(mesocyclesModel->getMuscularGroup(meso_idx, splitLetter));
		if (!muscularGroup1.isEmpty())
		{
			QString muscularGroup2;
			const QString mesoSplit(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_SPLIT));
			muscularGroupSimplified(muscularGroup1);

			QString::const_iterator itr(mesoSplit.constBegin());
			const QString::const_iterator itr_end(mesoSplit.constEnd());

			do {
				if (static_cast<QChar>(*itr) == QChar('R'))
					continue;
				else if (static_cast<QChar>(*itr) == splitLetter.at(0))
					continue;

				muscularGroup2 = mesocyclesModel->getMuscularGroup(meso_idx, *itr);
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
	worker->addExecArg(mesocyclesModel->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); } );
	DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, itemMngr->getSplitModel(splitLetter2.at(0))));
	worker2->addExecArg(mesocyclesModel->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
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
			mesoSplit = mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
			suggestedName = mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME) + tr(" - Exercises Plan.txt");
		}
		else
		{
			mesoSplit = splitLetter;
			suggestedName = mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME) +
								tr(" - Exercises Plan - Split ") + splitLetter + u".txt"_qs;
		}
		setExportFileName(suggestedName);
	}
	else
	{
		outFile = outFileInUse;
		mesoSplit = mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_SPLIT);
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
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(meso_idx)));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,meso_idx,bPreserveOldInfo,bPreserveOldInfoUntilDayBefore] ()
		{
			return changeMesoCalendar(meso_idx, bPreserveOldInfo, bPreserveOldInfoUntilDayBefore);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(meso_idx)));
	const QDate endDate(bPreserveOldInfo && bPreserveOldInfoUntilDayBefore ?
					QDate::currentDate() :
					mesocyclesModel->getDate(meso_idx, MESOCYCLES_COL_ENDDATE));
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilDayBefore);
	worker->addExecArg(endDate);
	if (!bPreserveOldInfo)
	{
		connect( this, &DBInterface::databaseReady, this, [&,worker,meso_idx,endDate] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_itemManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(meso_idx,
								mesocyclesModel->getDateFast(meso_idx, MESOCYCLES_COL_STARTDATE), endDate);
		});
	}
	createThread(worker, [worker] () { worker->changeMesoCalendar(); });
}

void DBInterface::updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,tDayModel] () {
				return updateMesoCalendarModel(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(meso_idx)));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID)); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->splitLetter());
	connect( this, &DBInterface::databaseReady, this, [&,worker,tDayModel] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_itemManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(tDayModel->mesoIdx(),
					tDayModel->getDateFast(0, TDAY_COL_DATE), mesocyclesModel->getDateFast(tDayModel->mesoIdx(), MESOCYCLES_COL_ENDDATE));
	});
	createThread(worker, [worker] () { worker->updateMesoCalendar(); });
}

void DBInterface::updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(tDayModel->mesoIdx())));
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->trainingDay());
	worker->addExecArg(tDayModel->splitLetter());
	worker->addExecArg(tDayModel->dayIsFinished() ? STR_ONE : STR_ZERO);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DBInterface::setDayIsFinished(DBTrainingDayModel* const tDayModel, const bool bFinished)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DBInterface::databaseReady, this, [&,tDayModel,bFinished] () {
			return setDayIsFinished(tDayModel, bFinished);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	tDayModel->setDayIsFinished(bFinished);
	const QDate date(tDayModel->getDateFast(0, TDAY_COL_DATE));
	mesocyclesModel->mesoCalendarModel(meso_idx)->setDayIsFinished(date, bFinished);
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(meso_idx)));
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
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, mesocyclesModel->mesoCalendarModel(meso_idx)));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DBInterface::getTrainingDay(const uint meso_idx, const QDate& date)
{
	if (mesocyclesModel->mesoCalendarModel(meso_idx)->count() == 0)
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
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
			mesocyclesModel->mesoCalendarModel(meso_idx)->getSplitLetter(date.month(), date.day()-1) :
			splitLetter);*/
		if (tDayModel->splitLetter() >= u"A"_qs && tDayModel->splitLetter() <= u"F"_qs)
		{
			DBTrainingDayModel* tempModel{new DBTrainingDayModel(this, meso_idx)};
			DBTrainingDayTable* worker{new DBTrainingDayTable(m_DBFilePath, tempModel)};
			worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
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
	worker->addExecArg(mesocyclesModel->getFast(itemMngr->mesoIdx(), MESOCYCLES_COL_ID));
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
		worker->addExecArg(mesocyclesModel->getFast(tDayModel->mesoIdx(), MESOCYCLES_COL_ID));
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
	return mesocyclesModel->mesoCalendarModel(tDayModel->mesoIdx())->getLastTrainingDayBeforeDate(tDayModel->getDateFast(0, TDAY_COL_DATE)) + 1;
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
