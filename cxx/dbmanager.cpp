#include "dbmanager.h"
#include "tputils.h"
#include "tpmesocycleclass.h"
#include "tptimer.h"
#include "translationclass.h"

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
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#define SPLITS_LOADED_ID 4321
static TPMesocycleClass* tempTPObj(nullptr);

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

void DbManager::checkPendingIntents() const
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
bool DbManager::sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const
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

void DbManager::androidOpenURL(const QString& address) const
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

bool DbManager::androidSendMail(const QString& address, const QString& subject, const QString& attachment) const
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

bool DbManager::viewFile(const QString& filePath, const QString& title) const
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

void DbManager::appStartUpNotifications()
{
	m_AndroidNotification = new TPAndroidNotification(this);
	if (mesocyclesModel->count() > 0)
	{
		DBMesoCalendarTable* calTable(new DBMesoCalendarTable(m_DBFilePath, appSettings()));
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
extern "C"
{
	#include <unistd.h>
}
#endif

DbManager::DbManager()
	: QObject (nullptr), mb_splitsLoaded(false), mb_importMode(false), m_exercisesPage(nullptr), m_settingsPage(nullptr),
		m_clientsOrCoachesPage(nullptr), m_userPage(nullptr)
{
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

void DbManager::init()
{
	m_DBFilePath = appSettings()->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath, appSettings()));
		db_exercises->createTable();
		delete db_exercises;
		appSettings()->setValue("exercisesListVersion", "0");
	}
	f_info.setFile(m_DBFilePath + DBMesocyclesFileName);
	if (!f_info.isReadable())
	{
		DBMesocyclesTable* db_mesos(new DBMesocyclesTable(m_DBFilePath, appSettings()));
		db_mesos->createTable();
		delete db_mesos;
	}
	f_info.setFile(m_DBFilePath + DBMesoSplitFileName);
	if (!f_info.isReadable())
	{
		DBMesoSplitTable* db_split(new DBMesoSplitTable(m_DBFilePath, appSettings()));
		db_split->createTable();
		delete db_split;
	}
	f_info.setFile(m_DBFilePath + DBMesoCalendarFileName);
	if (!f_info.isReadable())
	{
		DBMesoCalendarTable* db_cal(new DBMesoCalendarTable(m_DBFilePath, appSettings()));
		db_cal->createTable();
		delete db_cal;
	}
	f_info.setFile(m_DBFilePath + DBTrainingDayFileName);
	if (!f_info.isReadable())
	{
		DBTrainingDayTable* db_tday(new DBTrainingDayTable(m_DBFilePath, appSettings()));
		db_tday->createTable();
		delete db_tday;
	}
	f_info.setFile(m_DBFilePath + DBUserFileName);
	if (!f_info.isReadable())
	{
		DBUserTable* db_user(new DBUserTable(m_DBFilePath, appSettings()));
		db_user->createTable();
		delete db_user;
	}

	getExercisesListVersion();
	exercisesListModel = new DBExercisesModel(this);
	if (m_exercisesListVersion != appSettings()->value("exercisesListVersion").toString())
		updateExercisesList();
}

void DbManager::cleanUp()
{
	#ifdef Q_OS_ANDROID
	delete m_AndroidNotification;
	#endif
	cleanUpThreads();
	if (tempTPObj)
		delete tempTPObj;
	if (m_exercisesPage)
	{
		delete m_exercisesPage;
		delete m_exercisesComponent;
	}
	if (m_settingsPage)
	{
		delete m_settingsPage;
		delete m_settingsComponent;
	}
	if (m_clientsOrCoachesPage)
	{
		delete m_clientsOrCoachesPage;
		delete m_clientsOrCoachesComponent;
	}
	for(uint i(0); i < m_mesoManager.count(); ++i)
		delete m_mesoManager.at(i);
}

void DbManager::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}

void DbManager::setQmlEngine(QQmlApplicationEngine* QMlEngine)
{
	m_QMlEngine = QMlEngine;

	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBTrainingDayModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBUserModel");
	qmlRegisterType<TPTimer>("com.vivenciasoftware.qmlcomponents", 1, 0, "TPTimer");

	if (appSettings()->value("appVersion") != TP_APP_VERSION)
	{
		//All update code goes in here
		//updateDB(new DBMesoCalendarTable(m_DBFilePath, appSettings()));
		//updateDB(new DBMesocyclesTable(m_DBFilePath, appSettings()));
		//DBUserTable user(m_DBFilePath, appSettings());
		//user.removeDBFile();
		appSettings()->setValue("appVersion", TP_APP_VERSION);
	}

	getAllUsers();
	getAllMesocycles();

	m_mainWindow = static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0));
	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appUtils"), QVariant::fromValue(appUtils()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appTr"), QVariant::fromValue(appTr()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("userModel"), QVariant::fromValue(userModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("exercisesListModel"), QVariant::fromValue(exercisesListModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("lightIconFolder"), QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("darkIconFolder"), QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor1"), QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor2"), QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mainwindow"), QVariant::fromValue(m_mainWindow) });

	QQuickItem* appStackView(m_mainWindow->findChild<QQuickItem*>(u"appStackView"_qs));
	QQuickItem* contentItem(appStackView->parentItem());
	properties.append(QQmlContext::PropertyPair{ u"windowHeight"_qs, contentItem->height() }); //mainwindow.height - header.height
	properties.append(QQmlContext::PropertyPair{ u"windowWidth"_qs, contentItem->width() });

	m_QMlEngine->rootContext()->setContextProperties(properties);

	QMetaObject::invokeMethod(m_mainWindow, "init", Qt::AutoConnection);
	mAppDataFilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/"_qs;
#ifdef Q_OS_ANDROID
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &RunCommands::appResumed, this, &DbManager::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(m_mainWindow, "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	appStartUpNotifications();
#endif
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
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
					QQuickItem* tDayPage = m_mesoManager.at(tempModel->mesoIdx())->currenttDayPage();
					if (tempModel->count() > 0)
					{
						tDayPage->setProperty("previousTDays", QVariant::fromValue(tempModel->getRow_const(0)));
						tDayPage->setProperty("bHasPreviousTDays", true);
						if (tempModel->count() == 2)
							tDayPage->setProperty("lastWorkOutLocation",
								QVariant::fromValue(tempModel->getRow_const(1).at(TDAY_COL_LOCATION)));
					}
					else
					{
						tDayPage->setProperty("previousTDays", QVariant::fromValue(QStringList()));
						tDayPage->setProperty("previousTDays", QVariant::fromValue(QVariantList()));
						tDayPage->setProperty("bHasPreviousTDays", false);
					}
					tDayPage->setProperty("pageOptionsLoaded", true);
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

void DbManager::verifyBackupPageProperties(QQuickItem* page) const
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

void DbManager::copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const
{
	QString finalPath(targetPath);
	fixPath(finalPath);
	const bool bOK(copyDBFiles(appSettings()->value("dbFilePath").toString(), finalPath, backupFiles));
	page->setProperty("opResult", bOK ? 1 : 2);
	if (bOK)
		page->setProperty("backupCount", 0);
}

void DbManager::copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const
{
	QString origPath(sourcePath);
	fixPath(origPath);
	const bool bOK(copyDBFiles(origPath, appSettings()->value("dbFilePath").toString(), restoreFiles));
	page->setProperty("opResult", bOK ? 3 : 4);
	if (bOK)
		page->setProperty("restoreCount", 0);
}

#ifndef Q_OS_ANDROID
void DbManager::processArguments()
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

void DbManager::restartApp()
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

void DbManager::openRequestedFile(const QString &filename)
{
	const QString nameOnly(filename.right(filename.length() - filename.lastIndexOf('/') - 1));
	QMetaObject::invokeMethod(m_mainWindow, "tryToOpenFile", Q_ARG(QString, filename), Q_ARG(QString, nameOnly));
}

bool DbManager::exportToFile(const TPListModel* model, const QString& filename, QFile* &outFile) const
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
int DbManager::importFromFile(QString filename, QFile* inFile)
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
			model = new DBMesocyclesModel(this, userModel);
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
			case MESOCYCLES_TABLE_ID: model = new DBMesocyclesModel(this, userModel); break;
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

bool DbManager::importFromModel(TPListModel* model)
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
			TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
			if (splitModel->completeSplit())
			{
				DBMesoSplitModel* mesoSplitModel(mesoMngr->getSplitModel(splitModel->splitLetter().at(0)));
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
			TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
			DBTrainingDayModel* tDayModel(mesoMngr->gettDayModel(dayDate));
			if (tDayModel->updateFromModel(model))
			{
				if (mesocyclesModel->mesoCalendarModel(meso_idx)->count() == 0)
				{
					connect( this, &DbManager::databaseReady, this, [&,meso_idx,dayDate] () {
						connect( this, &DbManager::getPage, this, [&,meso_idx,dayDate] (QQuickItem* item, const uint) {
							return addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(dayDate), item);
						}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
						return getTrainingDay(meso_idx, dayDate);
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					getMesoCalendar(meso_idx, false);
				}
				else
				{
					connect( this, &DbManager::getPage, this, [&,dayDate] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(dayDate), item);
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					getTrainingDay(meso_idx, dayDate);
				}
			}
			else
				bOK = false;
		}
		break;
	}
	mb_importMode = false;
	return bOK;
}

void DbManager::saveFileDialogClosed(QString finalFileName, bool bResultOK)
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
	m_mainWindow->setProperty("importExportFilename", finalFileName);
	QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, resultCode));
}

int DbManager::parseFile(QString filename)
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
		QMetaObject::invokeMethod(m_mainWindow, "confirmImport", Q_ARG(QString, message.arg(tableMessage)));
		return 1;
	}
	else
		return -3;
}

void DbManager::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
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
		exportMesoSplit(meso_idx, u"X"_qs, bShare, outFile);

		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		m_mainWindow->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -10));
	}
}

void DbManager::openURL(const QString& address) const
{
	#ifdef Q_OS_ANDROID
	androidOpenURL(address);
	#else
	auto* __restrict proc(new QProcess ());
	proc->startDetached(u"xdg-open"_qs, QStringList() << address);
	delete proc;
	#endif
}

void DbManager::startChatApp(const QString& phone, const QString& appname) const
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

void DbManager::sendMail(const QString& address, const QString& subject, const QString& attachment_file) const
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

void DbManager::viewExternalFile(const QString& filename) const
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

void DbManager::updateDB(TPDatabaseTable* worker)
{
	createThread(worker, [worker] () { worker->updateDatabase(); } );
}

void DbManager::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
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

void DbManager::cleanUpThreads()
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
		disconnect(this, &DbManager::databaseReady, this, nullptr);
	}
}

void DbManager::bridge(QQuickItem* item, const uint id) {
	MSG_OUT("bridge  id " << id)
	MSG_OUT("bridge item " << item->objectName())
	emit getPage(item, id);

	if (id == m_expectedPageId)
	{
		emit internalSignal(id);
		m_expectedPageId = 0;
	}
}

//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
void DbManager::getAllUsers()
{
	userModel = new DBUserModel(this);
	DBUserTable* worker(new DBUserTable(m_DBFilePath, appSettings(), userModel));
	worker->getAllUsers();
	delete worker;

	bool noUsers(userModel->count() == 0);
	if (!noUsers)
		noUsers = userModel->userName(0).isEmpty();
	if (noUsers)
	{
		userModel->setIsEmpty(true);
		userModel->addUser(false);
	}
}

void DbManager::saveUser(const uint row)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, appSettings(), userModel));
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); } );
}

void DbManager::removeUser(const uint row, const bool bCoach)
{
	if (row >= 1)
	{
		DBUserTable* worker(new DBUserTable(m_DBFilePath, appSettings(), userModel));
		worker->addExecArg(userModel->userId(row));
		createThread(worker, [worker] () { return worker->removeEntry(); } );
		const int curUserRow(userModel->removeUser(row, bCoach));
		int firstUserRow(-1), lastUserRow(-1);
		if (curUserRow > 0)
		{
			firstUserRow = userModel->findFirstUser(bCoach);
			lastUserRow = userModel->findLastUser(bCoach);
		}
		m_clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
		m_clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
		m_clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
	}
}

void DbManager::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, appSettings(), userModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DbManager::getAllExercises()
{
	if (exercisesListModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
		worker->setUniqueID(2222);
		createThread(worker, [worker] () { worker->getAllExercises(); } );
	}
	else
		emit databaseReady(2222);
}

void DbManager::saveExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->saveExercise(); } );
}

void DbManager::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->removeEntry(); } );
}

void DbManager::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DbManager::updateExercisesList(DBExercisesModel* model)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
	if (!model)
		createThread(worker, [worker] () { return worker->updateExercisesList(); } );
	else
	{
		worker->addExecArg(QVariant::fromValue(model));
		createThread(worker, [worker] () { return worker->updateFromModel(); } );
	}
}

void DbManager::openExercisesListPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (m_exercisesPage != nullptr)
	{
		m_exercisesPage->setProperty("bChooseButtonEnabled", bChooseButtonEnabled);
		exercisesListModel->clearSelectedEntries();
		if (connectPage)
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		return;
	}
	if (exercisesListModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, appSettings(), exercisesListModel));
		connect( this, &DbManager::databaseReady, this, [&,worker,connectPage] (const uint db_id) {
			if (db_id == worker->uniqueID()) return createExercisesListPage(connectPage); });
		createThread(worker, [worker] () { return worker->getAllExercises(); } );
	}

	m_exercisesProperties.insert(QStringLiteral("bChooseButtonEnabled"), bChooseButtonEnabled);
	m_exercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/Pages/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [&,connectPage](QQmlComponent::Status) {
		return createExercisesListPage(connectPage); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void DbManager::createExercisesListPage(QQuickItem* connectPage)
{
	#ifdef DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (uint i(0); i < m_exercisesComponent->errors().count(); ++i)
			qDebug() << m_exercisesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (exercisesListModel->isReady() && m_exercisesComponent->status() == QQmlComponent::Ready)
	{
		if (m_exercisesPage == nullptr)
		{
			m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
															m_exercisesProperties, m_QMlEngine->rootContext()));
			m_QMlEngine->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
			m_exercisesPage->setParentItem(m_mainWindow->contentItem());
			if (connectPage)
				connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
			QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		}
	}
}

void DbManager::getExercisesListVersion()
{
	m_exercisesListVersion = QStringLiteral("0");
	QFile exercisesListFile( QStringLiteral(":/extras/exerciseslist.lst") );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[20] = { 0 };
		qint64 lineLength;
		QString line;
		lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
		if (lineLength > 0)
		{
			line = buf;
			if (line.startsWith(QStringLiteral("#Vers")))
				m_exercisesListVersion = line.split(';').at(1).trimmed();
		}
		exercisesListFile.close();
	}
}

void DbManager::exportExercisesList(const bool bShare)
{
	const QString suggestedName(tr("TrainingPlanner Exercises List.txt"));
	setExportFileName(suggestedName);
	if (!exercisesListModel->collectExportData())
	{
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -6));
		return;
	}
	QFile* outFile(nullptr);
	if (exportToFile(exercisesListModel, exportFileName(), outFile))
	{
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		m_mainWindow->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -10));
	}
}
//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
void DbManager::getAllMesocycles()
{
	mesocyclesModel = new DBMesocyclesModel(this, userModel);
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appSettings(), mesocyclesModel));
	worker->getAllMesocycles();
	delete worker;

	if (mesocyclesModel->count() > 0)
	{
		DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoSplitModel()));
		worker2->getAllMesoSplits();
		delete worker2;
	}

	connect(userModel, &DBUserModel::appUseModeChanged, this, [&] (const uint user_row) {
		if (user_row == 0) {
			mesocyclesModel->updateColumnLabels();
			QMetaObject::invokeMethod(m_mainWindow, "workoutButtonEnabled", Qt::AutoConnection);

			if (m_userPage)
				m_userPage->setProperty("useMode", userModel->appUseMode(0));
			for (uint i (0); i < m_mesoManager.count(); ++i)
			{
				if (m_mesoManager.at(i)->getMesoPage())
					m_mesoManager.at(i)->getMesoPage()->setProperty("useMode", userModel->appUseMode(0));
			}
		}
	});
}

void DbManager::createMesoManager(const uint meso_idx)
{
	TPMesocycleClass* mesoMngr = new TPMesocycleClass(mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID), meso_idx, m_QMlEngine, this);
	mesoMngr->setMesocycleModel(mesocyclesModel);
	m_mesoManager.append(mesoMngr);
	connect(mesoMngr, SIGNAL(pageReady(QQuickItem*,uint)), this, SLOT(bridge(QQuickItem*,uint)));
	connect(mesoMngr, SIGNAL(itemReady(QQuickItem*,uint)), this, SIGNAL(getItem(QQuickItem*,uint)));
}

void DbManager::getMesocyclePage(const uint meso_idx)
{
	if (meso_idx >= m_mesoManager.count())
		createMesoManager(meso_idx);
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	if (mesoMngr->getMesoPage() != nullptr)
	{
		addMainMenuShortCut(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME), mesoMngr->getMesoPage());
		return;
	}
	m_expectedPageId = mesoPageCreateId;
	connect(this, &DbManager::internalSignal, this, [&,mesoMngr] (const uint id) {
		if (id == mesoPageCreateId)
		{
			mesoMngr->getMesoPage()->setProperty("useMode", userModel->appUseMode(0));
			addMainMenuShortCut(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME), mesoMngr->getMesoPage());
		}
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	mesoMngr->createMesocyclePage();
}

uint DbManager::createNewMesocycle(const bool bCreatePage)
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

	const uint meso_idx = mesocyclesModel->newMesocycle(QStringList() << u"-1"_qs << tr("New Plan") << QString::number(startDate.toJulianDay()) <<
		QString::number(endDate.toJulianDay()) << QString() << QString::number(appUtils()->calculateNumberOfWeeks(startDate, endDate)) <<
		u"ABCDERR"_qs << QString() << userModel->userName(0) << QString() << QString() << u"1"_qs);

	if (bCreatePage)
	{
		createMesoManager(meso_idx);
		TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
		m_expectedPageId = mesoPageCreateId;
		connect(this, &DbManager::internalSignal, this, [&,meso_idx,mesoMngr] (const uint id) {
			if (id == mesoPageCreateId)
			{
				mesoMngr->getMesoPage()->setProperty("useMode", userModel->appUseMode(0));
				addMainMenuShortCut(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME), mesoMngr->getMesoPage());
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		mesoMngr->createMesocyclePage(minimumStartDate, appUtils()->createFutureDate(startDate,0,6,0));
	}
	return meso_idx;
}

void DbManager::saveMesocycle(const uint meso_idx)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appSettings(), mesocyclesModel));

	if (mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID) == -1)
	{
		if (mb_importMode)
			worker->setWaitForThreadToFinish(true);

		connect( this, &DbManager::databaseReady, this, [&,meso_idx,worker] (const uint db_id) {
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

void DbManager::removeMesocycle(const uint meso_idx)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	removeMainMenuShortCut(mesoMngr->getMesoPage());

	if (mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID))
	{
		removeMainMenuShortCut(mesoMngr->getCalendarPage());
		removeMainMenuShortCut(mesoMngr->getExercisesPlannerPage());
		removeMesoCalendar(meso_idx);
		removeMesoSplit(meso_idx);
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appSettings()));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
		createThread(worker, [worker] () { return worker->removeEntry(); });

		for (uint i(meso_idx+1); i < m_mesoManager.count(); ++i)
			m_mesoManager.at(i)->changeMesoIdxFromPagesAndModels(i-1);
	}

	TPMesocycleClass* tpObject(mesoMngr);
	m_mesoManager.removeAt(meso_idx);
	tpObject->disconnect();
	delete tpObject;

	mesocyclesModel->delMesocycle(meso_idx);
}

//Cannot order a mesocycle removal from within the qml page that will be deleted. Defer the removal to a little time after,
//just enough to have StackView remove the page
void DbManager::scheduleMesocycleRemoval(const uint meso_idx)
{
	QTimer::singleShot(200, this, [&,meso_idx] () { removeMesocycle(meso_idx); } );
}

void DbManager::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, appSettings(), mesocyclesModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DbManager::saveMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoSplitModel()));
	worker->addExecArg(meso_idx);
	createThread(worker, [worker] () { worker->saveMesoSplit(); } );
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	mesoMngr->updateMuscularGroup(mesocyclesModel->mesoSplitModel());
}

void DbManager::removeMesoSplit(const uint meso_idx)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings()));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { return worker->removeEntry(); });
}

void DbManager::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoSplitModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); });
}

void DbManager::getExercisesPlannerPage(const uint meso_idx)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	if (mesoMngr->getExercisesPlannerPage())
	{
		addMainMenuShortCut(tr("Exercises Planner: ") + mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME),
									mesoMngr->getExercisesPlannerPage());
		return;
	}
	m_expectedPageId = exercisesPlannerCreateId;
	connect(this, &DbManager::internalSignal, this, [&] (const uint id ) {
		if (id == exercisesPlannerCreateId)
		{
			getCompleteMesoSplit(meso_idx);
			addMainMenuShortCut(tr("Exercises Planner: ") + mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME),
								mesoMngr->getExercisesPlannerPage());
		}
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	mesoMngr->createPlannerPage();
}

void DbManager::loadCompleteMesoSplits(const uint meso_idx, const bool bThreaded)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
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
		splitModel = mesoMngr->getSplitModel(static_cast<QChar>(*itr));

		if (bThreaded)
		{
			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), splitModel));
			worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
			worker->addExecArg(static_cast<QChar>(*itr));
			if (!connected)
			{
				connect( this, &DbManager::databaseReady, this, [&,nSplits] (const uint db_id) mutable {
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
				worker2 = new DBMesoSplitTable(m_DBFilePath, appSettings(), splitModel);
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

void DbManager::getCompleteMesoSplit(const uint meso_idx)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	uint nSplits(mesocyclesModel->totalSplits(meso_idx));
	connect(this, &DbManager::getPage, this, [&,nSplits] (QQuickItem* item, const uint id) mutable { if (id <= 6) {
			QMetaObject::invokeMethod(mesoMngr->getExercisesPlannerPage(), "insertSplitPage",
								Q_ARG(QQuickItem*, item), Q_ARG(int, static_cast<int>(id)));
			if (--nSplits == 0)
				disconnect(this, &DbManager::getPage, this, nullptr);
		}
	});

	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&] (const uint id) { if (id == SPLITS_LOADED_ID) {
				return mesoMngr->createMesoSplitPage(); disconnect(this, &DbManager::internalSignal, this, nullptr); } } );
		loadCompleteMesoSplits(meso_idx);
	}
	else
		mesoMngr->createMesoSplitPage();
}

void DbManager::saveMesoSplitComplete(DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), model));
	worker->addExecArg(model->mesoIdx());
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); });
}

bool DbManager::mesoHasPlan(const uint meso_id, const QString& splitLetter) const
{
	if (splitLetter != u"R"_qs)
	{
		DBMesoSplitTable* meso_split(new DBMesoSplitTable(m_DBFilePath, appSettings()));
		const bool ret(meso_split->mesoHasPlan(QString::number(meso_id), splitLetter));
		meso_split->deleteLater();
		return ret;
	}
	return false;
}

void DbManager::loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), model));
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

QString DbManager::checkIfSplitSwappable(const DBMesoSplitModel* splitModel) const
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

void DbManager::swapMesoPlans(const uint meso_idx, const QString& splitLetter1, const QString& splitLetter2)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	mesoMngr->swapPlans(splitLetter1, splitLetter2);
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesoMngr->getSplitModel(splitLetter1.at(0))));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); } );
	DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesoMngr->getSplitModel(splitLetter2.at(0))));
	worker2->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	createThread(worker2, [worker2] () { worker2->saveMesoSplitComplete(); });
}

void DbManager::exportMesoSplit(const uint meso_idx, const QString& splitLetter, const bool bShare, QFile* outFileInUse)
{
	QString mesoSplit;
	QString suggestedName;
	QFile* outFile(nullptr);

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
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;
		mesoLetters.append(static_cast<QChar>(*itr));
		bExportToFileOk &= exportToFile(mesoMngr->getSplitModel(static_cast<QChar>(*itr)), exportFileName(), outFile);
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
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		m_mainWindow->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -10));
	}
}
//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
void DbManager::getMesoCalendar(const uint meso_idx, const bool bCreatePage)
{
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(meso_idx)));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
		if (bCreatePage)
		{
			connect( this, &DbManager::databaseReady, this, [&,worker,meso_idx,bCreatePage] (const uint db_id) {
				if (db_id == worker->uniqueID()) return getMesoCalendar(meso_idx, bCreatePage); } );
		}
		createThread(worker, [worker] () { worker->getMesoCalendar(); });
		return;
	}
	if (bCreatePage)
	{
		TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
		if (mesoMngr->getCalendarPage() != nullptr)
		{
			addMainMenuShortCut(tr("Calendar: ") + mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME), mesoMngr->getCalendarPage());
			return;
		}
		m_expectedPageId = calPageCreateId;
		connect(this, &DbManager::internalSignal, this, [&] (const uint id ) { if (id == calPageCreateId)
			addMainMenuShortCut(tr("Calendar: ") + mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_NAME), mesoMngr->getCalendarPage());
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		mesoMngr->createMesoCalendarPage();
	}
}

void DbManager::changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore)
{
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,meso_idx,bPreserveOldInfo,bPreserveOldInfoUntilDayBefore] ()
		{
			return changeMesoCalendar(meso_idx, bPreserveOldInfo, bPreserveOldInfoUntilDayBefore);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(meso_idx)));
	const QDate endDate(bPreserveOldInfo && bPreserveOldInfoUntilDayBefore ?
					QDate::currentDate() :
					mesocyclesModel->getDate(meso_idx, MESOCYCLES_COL_ENDDATE));
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilDayBefore);
	worker->addExecArg(endDate);
	if (!bPreserveOldInfo)
	{
		connect( this, &DbManager::databaseReady, this, [&,worker,meso_idx,endDate] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_mesoManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(
						mesocyclesModel->getDateFast(meso_idx, MESOCYCLES_COL_STARTDATE), endDate);
		});
	}
	createThread(worker, [worker] () { worker->changeMesoCalendar(); });
}

void DbManager::updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,tDayModel] () {
				return updateMesoCalendarModel(tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(meso_idx)));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID)); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->splitLetter());
	connect( this, &DbManager::databaseReady, this, [&,tDayModel] (const uint db_id) {
			if (db_id == worker->uniqueID())
				return m_mesoManager.at(meso_idx)->updateOpenTDayPagesWithNewCalendarInfo(tDayModel);
	});
	createThread(worker, [worker] () { worker->updateMesoCalendar(); });
}

void DbManager::updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(tDayModel->mesoIdx())));
	worker->addExecArg(tDayModel->getDateFast(0, TDAY_COL_DATE));
	worker->addExecArg(tDayModel->trainingDay());
	worker->addExecArg(tDayModel->splitLetter());
	worker->addExecArg(tDayModel->dayIsFinished() ? STR_ONE : STR_ZERO);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DbManager::setDayIsFinished(DBTrainingDayModel* const tDayModel, const bool bFinished)
{
	const uint meso_idx(tDayModel->mesoIdx());
	if (!mesocyclesModel->mesoCalendarModel(meso_idx)->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,tDayModel,bFinished] () {
			return setDayIsFinished(tDayModel, bFinished);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}
	TPMesocycleClass* mesoMngr(m_mesoManager.at(meso_idx));
	tDayModel->setDayIsFinished(bFinished);
	const QDate date(tDayModel->getDateFast(0, TDAY_COL_DATE));
	mesocyclesModel->mesoCalendarModel(meso_idx)->setDayIsFinished(date, bFinished);
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(meso_idx)));
	worker->addExecArg(date);
	worker->addExecArg(bFinished);
	createThread(worker, [worker] () { worker->updateDayIsFinished(); });
}

void DbManager::removeMesoCalendar(const uint meso_id)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings()));
	worker->addExecArg(QString::number(meso_id));
	createThread(worker, [worker] () { return worker->removeMesoCalendar(); });
}

void DbManager::deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, appSettings(), mesocyclesModel->mesoCalendarModel(meso_idx)));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DbManager::getTrainingDay(const uint meso_idx, const QDate& date)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	if (mesoMngr->gettDayPage(date) != nullptr)
	{
		addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(date), mesoMngr->gettDayPage(date));
		return;
	}

	if (mesocyclesModel->mesoCalendarModel(meso_idx)->count() == 0)
	{
		connect( this, &DbManager::databaseReady, this, [&,meso_idx,date] () {
			getTrainingDay(meso_idx, date);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(meso_idx, false);
		return;
	}

	m_expectedPageId = tDayPageCreateId;
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), mesoMngr->gettDayModel(date)));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) mesoMngr->createTrainingDayPage(date); } );
	connect( this, &DbManager::internalSignal, this, [&,date] (const uint id ) {
				if (id == tDayPageCreateId)
				{
					addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(date), mesoMngr->gettDayPage(date));
					mesoMngr->currenttDayPage()->setProperty("dayIsNotCurrent", date != QDate::currentDate());
					getTrainingDayExercises(meso_idx, date);
				}
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	createThread(worker, [worker] () { return worker->getTrainingDay(); });
}

void DbManager::getTrainingDayExercises(const uint meso_idx, const QDate& date)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), mesoMngr->currenttDayModel()));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,meso_idx,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) return verifyTDayOptions(meso_idx, date); });
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DbManager::verifyTDayOptions(const DBTrainingDayModel* const tDayModel)
{
	const uint meso_idx(tDayModel->mesoIdx());
	TPMesocycleClass* mesoMngr(m_mesoManager.at(meso_idx));
	if (mesoMngr->currenttDayModel()->exerciseCount() > 0)
	{
		mesoMngr->createExercisesObjects();
		return;
	}

	/*const QString splitletter(splitLetter.isEmpty() ?
			mesocyclesModel->mesoCalendarModel(meso_idx)->getSplitLetter(date.month(), date.day()-1) :
			splitLetter);*/
	mesoMngr->currenttDayPage()->setProperty("bHasMesoPlan", mesoHasPlan(mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID), tDayModel->splitLetter()));
	if (tDayModel->splitLetter() >= u"A"_qs && tDayModel->splitLetter() <= u"F"_qs)
	{
		DBTrainingDayModel* tempModel(new DBTrainingDayModel(this, meso_idx));
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), tempModel));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
		worker->addExecArg(tDayModel->splitLetter());
		worker->addExecArg(tDayModel->getFast(0, TDAY_COL_DATE));
		createThread(worker, [worker] () { return worker->getPreviousTrainingDaysInfo(); } );
	}
}

void DbManager::clearExercises(DBTrainingDayModel* const tDayModel)
{
	TPMesocycleClass* mesoMngr(m_mesoManager.at(tDayModel->mesoIdx()));
	mesoMngr->clearExercises();
	verifyTDayOptions(tDayModel);
}

void DbManager::loadExercisesFromDate(const uint meso_idx, const QString& strDate)
{
	TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
	DBTrainingDayModel* tDayModel = mesoMngr->currenttDayModel();
	const QDate date(appUtils()->getDateFromStrDate(strDate));
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), tDayModel));
	worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
	worker->addExecArg(QString::number(date.toJulianDay()));

	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect( this, &DbManager::databaseReady, this, [&,mesoMngr,tDayModel,date,worker] (const uint db_id) {
		if (db_id == worker->uniqueID())
		{
			if (tDayModel->date() != QDate::currentDate())
				tDayModel->setDayIsFinished(true);
			else
				tDayModel->setModified(true);
			return mesoMngr->createExercisesObjects();
		}
	});
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(true); });
}

void DbManager::loadExercisesFromMesoPlan(const uint meso_idx, const QString& splitLetter)
{
	const QChar splitletter(splitLetter.at(0));
	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&,meso_idx,splitLetter] (const uint id) {
			if (id == SPLITS_LOADED_ID)
				loadExercisesFromMesoPlan(meso_idx, splitLetter);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(meso_idx);
	}
	else
	{
		TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
		DBTrainingDayModel* tDayModel = mesoMngr->currenttDayModel();
		tDayModel->convertMesoSplitModelToTDayModel(mesoMngr->getSplitModel(splitletter));
		mesoMngr->createExercisesObjects();
		if (tDayModel->date() != QDate::currentDate())
			tDayModel->setDayIsFinished(true);
	}
}

void DbManager::convertTDayToPlan(const uint meso_idx, DBTrainingDayModel* tDayModel)
{
	const QChar splitletter(tDayModel->splitLetter().at(0));
	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&,meso_idx,tDayModel] (const uint id) {
			if (id == SPLITS_LOADED_ID)
				return convertTDayToPlan(meso_idx, tDayModel);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		loadCompleteMesoSplits(meso_idx);
	}
	else
	{
		TPMesocycleClass* mesoMngr = m_mesoManager.at(meso_idx);
		DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, appSettings(), mesoMngr->getSplitModel(splitletter)));
		worker->addExecArg(mesocyclesModel->getFast(meso_idx, MESOCYCLES_COL_ID));
		worker->addExecArg(tDayModel->splitLetter());
		createThread(worker, [worker,tDayModel] () { return worker->convertTDayExercisesToMesoPlan(tDayModel); });
	}
}

void DbManager::saveTrainingDay(const uint meso_idx)
{
	DBTrainingDayModel* tDayModel = m_mesoManager.at(meso_idx)->currenttDayModel();
	if (tDayModel->modified())
	{
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), tDayModel));
		createThread(worker, [worker] () { return worker->saveTrainingDay(); });
	}
}

void DbManager::removeTrainingDay(const uint meso_idx)
{
	DBTrainingDayModel* tDayModel = m_mesoManager.at(meso_idx)->currenttDayModel();
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), tDayModel));
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DbManager::deleteTrainingDayTable(const uint meso_idx, const bool bRemoveFile)
{
	DBTrainingDayModel* tDayModel = m_mesoManager.at(meso_idx)->currenttDayModel();
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, appSettings(), tDayModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DbManager::exportTrainingDay(const uint meso_idx, const QDate& date, const QString& splitLetter, const bool bShare)
{
	const QString suggestedName(tr(" - Workout ") + splitLetter + u".txt"_qs);
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	if (exportToFile(m_mesoManager.at(meso_idx)->gettDayModel(date), exportFileName(), outFile))
	{
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		m_mainWindow->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -10));
	}
}

uint DbManager::getWorkoutNumberForTrainingDay(const uint meso_idx, const QDate& date) const
{
	return mesocyclesModel->mesoCalendarModel(meso_idx)->getLastTrainingDayBeforeDate(date) + 1;
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
void DbManager::openSettingsPage(const uint startPageIndex)
{
	if (m_settingsPage != nullptr)
	{
		m_settingsPage->setProperty("startPageIndex", startPageIndex);
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
		return;
	}

	m_settingsProperties.insert(QStringLiteral("startPageIndex"), startPageIndex);
	m_settingsComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/Pages/ConfigurationPage.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_settingsComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
		return createSettingsPage(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void DbManager::createSettingsPage()
{
	#ifdef DEBUG
	if (m_settingsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_settingsComponent->errorString();
		for (uint i(0); i < m_settingsComponent->errors().count(); ++i)
			qDebug() << m_settingsComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_settingsComponent->status() == QQmlComponent::Ready)
	{
		m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->createWithInitialProperties(m_settingsProperties, m_QMlEngine->rootContext()));
		m_QMlEngine->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
		m_settingsPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
		m_userPage = m_settingsPage->findChild<QQuickItem*>(u"userPage"_qs);
		m_userPage->setProperty("useMode", userModel->appUseMode(0));
	}
}

void DbManager::openClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	setClientsOrCoachesPagesProperties(bManageClients, bManageCoaches);

	if (m_clientsOrCoachesPage != nullptr)
	{
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
		return;
	}

	m_clientsOrCoachesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/Pages/ClientsOrCoachesPage.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_clientsOrCoachesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
		return createClientsOrCoachesPage(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void DbManager::createClientsOrCoachesPage()
{
	#ifdef DEBUG
	if (m_clientsOrCoachesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_clientsOrCoachesComponent->errorString();
		for (uint i(0); i < m_clientsOrCoachesComponent->errors().count(); ++i)
			qDebug() << m_clientsOrCoachesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_clientsOrCoachesComponent->status() == QQmlComponent::Ready)
	{
		m_clientsOrCoachesPage = static_cast<QQuickItem*>(m_clientsOrCoachesComponent->createWithInitialProperties(
				m_clientsOrCoachesProperties, m_QMlEngine->rootContext()));
		m_QMlEngine->setObjectOwnership(m_clientsOrCoachesPage, QQmlEngine::CppOwnership);
		m_clientsOrCoachesPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
	}
}

void DbManager::setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches)
{
	int curUserRow(0), firstUserRow(-1), lastUserRow(-1);

	if (userModel->appUseMode(0) == APP_USE_MODE_SINGLE_USER)
		return;

	if (bManageClients)
	{
		if (userModel->appUseMode(0) == APP_USE_MODE_SINGLE_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = userModel->findFirstUser(false);
			lastUserRow = userModel->findLastUser(false);
			curUserRow = userModel->currentUser(0);
		}
	}

	if (bManageCoaches)
	{
		if (userModel->appUseMode(0) == APP_USE_MODE_SINGLE_USER_WITH_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = userModel->findFirstUser(true);
			lastUserRow = userModel->findLastUser(true);
			curUserRow = userModel->currentCoach(0);
		}
	}

	if (curUserRow == 0)
		curUserRow = lastUserRow;
	if (m_clientsOrCoachesPage)
	{
		m_clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
		m_clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
		m_clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
		m_clientsOrCoachesPage->setProperty("showUsers", bManageClients);
		m_clientsOrCoachesPage->setProperty("showCoaches", bManageCoaches);

	}
	else
	{
		m_clientsOrCoachesProperties.insert(u"curUserRow"_qs, curUserRow);
		m_clientsOrCoachesProperties.insert(u"firstUserRow"_qs, firstUserRow);
		m_clientsOrCoachesProperties.insert(u"lastUserRow"_qs, lastUserRow);
		m_clientsOrCoachesProperties.insert(u"showUsers"_qs, bManageClients);
		m_clientsOrCoachesProperties.insert(u"showCoaches"_qs, bManageCoaches);
	}
}

void DbManager::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	if (m_mainMenuShortcutPages.contains(page))
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
	else
	{
		if (m_mainMenuShortcutEntries.count() < 5)
		{
			QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
			QMetaObject::invokeMethod(m_mainWindow, "createShortCut", Q_ARG(QString, label),
													Q_ARG(QQuickItem*, page), Q_ARG(int, m_mainMenuShortcutPages.count()));
			m_mainMenuShortcutPages.append(page);
		}
		else
		{
			QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
			for (uint i(0); i < m_mainMenuShortcutPages.count()-1; ++i)
			{
				m_mainMenuShortcutPages.move(i+1, i);
				m_mainMenuShortcutEntries.at(i)->setProperty("text", m_mainMenuShortcutEntries.at(i+1)->property("text").toString());
			}
			m_mainMenuShortcutEntries.at(4)->setProperty("text", label);
			m_mainMenuShortcutPages.replace(4, page);
		}
	}
}

void DbManager::removeMainMenuShortCut(QQuickItem* page)
{
	const int idx(m_mainMenuShortcutPages.indexOf(page));
	if (idx != -1)
	{
		QMetaObject::invokeMethod(m_mainWindow, "popFromStack", Q_ARG(QQuickItem*, page));
		m_mainMenuShortcutPages.remove(idx);
		delete m_mainMenuShortcutEntries.at(idx);
		m_mainMenuShortcutEntries.remove(idx);
		for (uint i(idx); i < m_mainMenuShortcutEntries.count(); ++i)
			m_mainMenuShortcutEntries.at(i)->setProperty("clickid", i);
	}
}

void DbManager::openMainMenuShortCut(const int button_id)
{
	QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
