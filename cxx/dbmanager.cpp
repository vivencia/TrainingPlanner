#include "dbmanager.h"
#include "runcommands.h"
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
		// create a Java String for the Working Dir Path
		QJniObject jniWorkingDir = QJniObject::fromString(mAppDataFilesPath);
		if(!jniWorkingDir.isValid())
		{
			MSG_OUT("QJniObject jniWorkingDir not valid.WorkingDir not valid")
			return;
		}
		activity.callMethod<void>("checkPendingIntents","(Ljava/lang/String;)V", jniWorkingDir.object<jstring>());
		MSG_OUT("checkPendingIntents: " << mAppDataFilesPath)
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
		DBMesoCalendarTable* calTable(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			if (dayInfoList.at(0) == m_MesoIdStr)
			{
				QString message;
				const QString splitLetter(dayInfoList.at(2));
				if (splitLetter != u"N"_qs) //day is training day
				{
					if (dayInfoList.at(3) == u"1"_qs) //day is completed
						message = tr("Your training routine seems to go well. Workout for the day is concluded");
					else
						message = tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter;
				}
				else
					message = tr("Enjoy your day of rest from workouts!");
				m_AndroidNotification->sendNotification(tr("Training Planner"), message);
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

DbManager::DbManager(QSettings* appSettings)
	: QObject (nullptr), m_MesoId(-2), m_MesoIdx(-2), mb_splitsLoaded(false), mb_importMode(false), m_currentMesoManager(nullptr),
		m_appSettings(appSettings), m_exercisesPage(nullptr), m_settingsPage(nullptr), m_clientsOrCoachesPage(nullptr)
{}

DbManager::~DbManager()
{
	#ifdef Q_OS_ANDROID
	delete m_AndroidNotification;
	#endif
	cleanUp();
	if (tempTPObj)
		delete tempTPObj;
	delete mesoSplitModel;
	delete exercisesListModel;
	delete mesocyclesModel;
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
	for(uint i(0); i < m_MesoManager.count(); ++i)
		delete m_MesoManager.at(i);
}

void DbManager::init()
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath, m_appSettings));
		db_exercises->createTable();
		delete db_exercises;
		m_appSettings->setValue("exercisesListVersion", "0");
	}
	f_info.setFile(m_DBFilePath + DBMesocyclesFileName);
	if (!f_info.isReadable())
	{
		DBMesocyclesTable* db_mesos(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		db_mesos->createTable();
		delete db_mesos;
	}
	f_info.setFile(m_DBFilePath + DBMesoSplitFileName);
	if (!f_info.isReadable())
	{
		DBMesoSplitTable* db_split(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
		db_split->createTable();
		delete db_split;
	}
	f_info.setFile(m_DBFilePath + DBMesoCalendarFileName);
	if (!f_info.isReadable())
	{
		DBMesoCalendarTable* db_cal(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
		db_cal->createTable();
		delete db_cal;
	}
	f_info.setFile(m_DBFilePath + DBTrainingDayFileName);
	if (!f_info.isReadable())
	{
		DBTrainingDayTable* db_tday(new DBTrainingDayTable(m_DBFilePath, m_appSettings));
		db_tday->createTable();
		delete db_tday;
	}
	f_info.setFile(m_DBFilePath + DBUserFileName);
	if (!f_info.isReadable())
	{
		DBUserTable* db_user(new DBUserTable(m_DBFilePath, m_appSettings));
		db_user->createTable();
		delete db_user;
	}

	getExercisesListVersion();
	exercisesListModel = new DBExercisesModel(this);
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
		updateExercisesList();
}

void DbManager::exitApp()
{
	qApp->exit (0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit (0);
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

	if (m_appSettings->value("appVersion") != TP_APP_VERSION)
	{
		//All update code goes in here
		//updateDB(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
		//updateDB(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		//DBUserTable user(m_DBFilePath, m_appSettings);
		//user.removeDBFile();
		m_appSettings->setValue("appVersion", TP_APP_VERSION);
	}

	getAllUsers();
	getAllMesocycles();

	m_mainWindow = static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0));
	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("runCmd"), QVariant::fromValue(runCmd()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("itemManager"), QVariant::fromValue(m_currentMesoManager) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appTr"), QVariant::fromValue(appTr()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("userModel"), QVariant::fromValue(userModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoSplitModel"), QVariant::fromValue(mesoSplitModel) });
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
	connect(runCmd(), &RunCommands::appResumed, this, &DbManager::checkPendingIntents);
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
		switch (static_cast<DBMesocyclesTable*>(dbObj)->opCode())
		{
			default:
			break;
			case OP_DELETE_TABLE:
				dbObj->createTable();
			break;
			case OP_UPDATE_LIST:
				m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion);
			break;
			case OP_READ:
				if (dbObjName == DBMesoCalendarObjectName)
				{
					if (mesoCalendarModel->count() == 0)
					{
						mesoCalendarModel->createModel( m_MesoId, mesocyclesModel->getDateFast(m_MesoIdx, 2),
								mesocyclesModel->getDateFast(m_MesoIdx, 3), mesocyclesModel->getFast(m_MesoIdx, 6) );
						createMesoCalendar();
					}
				}
				else if (dbObjName == DBTrainingDayObjectName)
				{
					DBTrainingDayModel* tempModel(static_cast<DBTrainingDayModel*>(static_cast<DBTrainingDayTable*>(dbObj)->model()));
					if (tempModel->count() > 0)
					{
						m_currentMesoManager->currenttDayPage()->setProperty("previousTDays", QVariant::fromValue(tempModel->getRow_const(0)));
						m_currentMesoManager->currenttDayPage()->setProperty("bHasPreviousTDays", true);
						if (tempModel->count() == 2)
							m_currentMesoManager->currenttDayPage()->setProperty("lastWorkOutLocation",
								QVariant::fromValue(tempModel->getRow_const(1).at(TDAY_COL_LOCATION)));
					}
					else
					{
						m_currentMesoManager->currenttDayPage()->setProperty("previousTDays", QVariant::fromValue(QVariantList()));
						m_currentMesoManager->currenttDayPage()->setProperty("bHasPreviousTDays", false);
					}
					m_currentMesoManager->currenttDayPage()->setProperty("pageOptionsLoaded", true);
					delete tempModel;
				}
			break;
			case OP_ADD:
				if (dbObjName == DBMesocyclesObjectName)
				{
					m_MesoIdx = mesocyclesModel->count() - 1;
					m_MesoId = mesocyclesModel->getFast(m_MesoIdx, 0).toUInt();
					m_appSettings->setValue("lastViewedMesoId", m_MesoId);
					m_MesoIdStr = QString::number(m_MesoId);
					m_currentMesoManager->setMesoId(m_MesoId);
				}
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
	QFileInfo backupDirInfo(m_appSettings->value("backupFolder").toString());
	const bool bCanWriteToBackupFolder(backupDirInfo.isDir() && backupDirInfo.isWritable());
	uint restoreCount(0);

	if (bCanWriteToBackupFolder)
	{
		const QString appDir(m_appSettings->value("backupFolder").toString() + u"/tp/"_qs);
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
	const bool bOK(copyDBFiles(m_appSettings->value("dbFilePath").toString(), finalPath, backupFiles));
	page->setProperty("opResult", bOK ? 1 : 2);
	if (bOK)
		page->setProperty("backupCount", 0);
}

void DbManager::copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const
{
	QString origPath(sourcePath);
	fixPath(origPath);
	const bool bOK(copyDBFiles(origPath, m_appSettings->value("dbFilePath").toString(), restoreFiles));
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
	QMetaObject::invokeMethod(m_mainWindow, "tryToOpenFile", Q_ARG(QString, filename));
}

bool DbManager::exportToFile(const TPListModel* model, const QString& filename, const bool bFancy, QFile* &outFile) const
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
		model->exportToText(outFile, bFancy);
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
		else if (inData.indexOf(DBTrainingDayObjectName) != -1)
			model = new DBTrainingDayModel(this);
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
			case TRAININGDAY_TABLE_ID: model = new DBTrainingDayModel(this); break;
			default:
				return -2;
		}
	}

	model->deleteLater();
	if (!model->importExtraInfo(inData))
		return -4;

	bool bDataImportSuccessfull(false);
	if (bFancy)
		bDataImportSuccessfull = model->importFromFancyText(inFile, inData);
	else
	{
		const QString data(inFile->readAll());
		if (!data.isEmpty())
		{
			sep_idx = data.indexOf(u"##0x"_qs);
			if (sep_idx == -1)
				bDataImportSuccessfull = model->importFromText(data);
			else
			{
				inFile->seek(sep_idx);
				bDataImportSuccessfull = model->importFromText(data.left(sep_idx));
			}
		}
	}

	if (bDataImportSuccessfull)
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
				createNewMesocycle(model->getFast(0, MESOCYCLES_COL_REALMESO) == u"1"_qs, model->getFast(0, 1), false);
				for (uint i(MESOCYCLES_COL_ID); i <= MESOCYCLES_COL_REALMESO; ++i)
					mesocyclesModel->setFast(m_MesoIdx, i, model->getFast(0, i));
				saveMesocycle(false, false, false);
				emit mesocyclesModel->currentRowChanged(); //notify main.qml::btnWorkout to evaluate its enabled state
			}
			else
				bOK = false;
		break;
		case MESOSPLIT_TABLE_ID:
		{
			if (static_cast<DBMesoSplitModel*>(model)->completeSplit())
			{
				DBMesoSplitModel* splitModel(m_currentMesoManager->getSplitModel(static_cast<DBMesoSplitModel*>(model)->splitLetter().at(0)));
				if (splitModel->updateFromModel(model))
				{
					saveMesoSplitComplete(splitModel);
					// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
					// by the time mb_splitsLoaded is ever checked upon
					mb_splitsLoaded = true;
				}
				else
					bOK = false;
			}
			else
			{
				for (uint i(0); i < 8; ++i)
					mesoSplitModel->setFast(m_MesoIdx, i, model->getFast(0, i));
				mesoSplitModel->setFast(m_MesoIdx, 1, m_MesoIdStr);
				mesoSplitModel->setCurrentRow(m_MesoIdx);
				saveMesoSplit();
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			DBTrainingDayModel* tDayModel(m_currentMesoManager->gettDayModel(dayDate));
			if (tDayModel->updateFromModel(model))
			{
				if (mesoCalendarModel->count() == 0)
				{
					connect( this, &DbManager::databaseReady, this, [&,dayDate] () {
						connect( this, &DbManager::getPage, this, [&,dayDate] (QQuickItem* item, const uint) {
							return addMainMenuShortCut(tr("Workout: ") + runCmd()->formatDate(dayDate), item);
								}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
						return getTrainingDay(dayDate); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					getMesoCalendar(false);
				}
				else
				{
					connect( this, &DbManager::getPage, this, [&,dayDate] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + runCmd()->formatDate(dayDate), item);
							}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					getTrainingDay(dayDate);
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

void DbManager::exportMeso(const bool bShare, const bool bFancy)
{
	if (!mb_splitsLoaded)
		loadCompleteMesoSplits(false);
	const QString suggestedName(mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_NAME) + tr(" - TP Complete Meso.txt"));
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	mesocyclesModel->setExportRow(m_MesoIdx);

	if (exportToFile(mesocyclesModel, exportFileName(), bFancy, outFile))
	{
		mesoSplitModel->setExportRow(m_MesoIdx);
		exportToFile(mesoSplitModel, QString(), bFancy, outFile);
		exportMesoSplit(u"X"_qs, bShare, bFancy, outFile);

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
	if (!runCmd()->canReadFile(runCmd()->getCorrectPath(filename)))
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
		connect(&m_threadCleaner, &QTimer::timeout, this, [&] { return cleanUp(); });
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

void DbManager::cleanUp()
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
				MSG_OUT("cleanUp: " << dbObj->objectName() << "uniqueID: " << dbObj->uniqueID());
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
	DBUserTable* worker(new DBUserTable(m_DBFilePath, m_appSettings, userModel));
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
	DBUserTable* worker(new DBUserTable(m_DBFilePath, m_appSettings, userModel));
	worker->addExecArg(row);
	createThread(worker, [worker] () { worker->saveUser(); } );
}

int DbManager::removeUser(const uint row, const bool bCoach)
{
	if (row >= 1)
	{
		DBUserTable* worker(new DBUserTable(m_DBFilePath, m_appSettings, userModel));
		worker->addExecArg(userModel->userId(row));
		createThread(worker, [worker] () { return worker->removeEntry(); } );
		return userModel->removeUser(row, bCoach);
	}
	return -1;
}

void DbManager::deleteUserTable(const bool bRemoveFile)
{
	DBUserTable* worker(new DBUserTable(m_DBFilePath, m_appSettings, userModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DbManager::getAllExercises()
{
	if (exercisesListModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
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
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->saveExercise(); } );
}

void DbManager::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->removeEntry(); } );
}

void DbManager::deleteExercisesTable(const bool bRemoveFile)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DbManager::updateExercisesList(DBExercisesModel* model)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
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
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
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

void DbManager::exportExercisesList(const bool bShare, const bool bFancy)
{
	const QString suggestedName(tr("TrainingPlanner Exercises List.txt"));
	setExportFileName(suggestedName);
	if (!exercisesListModel->collectExportData())
	{
		QMetaObject::invokeMethod(m_mainWindow, "displayResultMessage", Q_ARG(int, -6));
		return;
	}
	QFile* outFile(nullptr);
	if (exportToFile(exercisesListModel, exportFileName(), bFancy, outFile))
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
	mesoSplitModel = new DBMesoSplitModel(this, false);
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->getAllMesocycles();

	if (mesocyclesModel->count() > 0)
	{
		for(uint i(0); i < mesocyclesModel->count(); ++i)
			getMesoSplit(mesocyclesModel->getFast(i, MESOCYCLES_COL_ID));
		setWorkingMeso(m_appSettings->value("lastViewedMesoId", 0).toUInt());
	}
	else
	{
		m_currentMesoManager = new TPMesocycleClass(-10, -10, m_QMlEngine, this);
		tempTPObj = m_currentMesoManager;
	}
	delete worker;
}

void DbManager::setWorkingMeso(int meso_idx)
{
	if (meso_idx != m_MesoIdx)
	{
		if (meso_idx >= mesocyclesModel->count())
			meso_idx = mesocyclesModel->count() - 1;

		m_MesoIdx = meso_idx;
		if (meso_idx >= 0)
		{
			m_MesoId = mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID);
			m_totalSplits = mesocyclesModel->getTotalSplits(m_MesoIdx);
			m_MesoIdStr = QString::number(m_MesoId);
			mesocyclesModel->setCurrentRow(m_MesoIdx);
			mesoSplitModel->setCurrentRow(m_MesoIdx);
		}
		else
		{
			m_MesoId = -1;
			m_totalSplits = 0;
			return;
		}

		bool bFound(false);
		for(uint i(0); i < m_MesoManager.count(); ++i)
		{
			if (m_MesoManager.at(i)->mesoId() == m_MesoId)
			{
				m_currentMesoManager = m_MesoManager.at(i);
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			m_currentMesoManager = new TPMesocycleClass(m_MesoId, m_MesoIdx, m_QMlEngine, this);
			m_currentMesoManager->setMesocycleModel(mesocyclesModel);
			m_MesoManager.append(m_currentMesoManager);
			connect(m_currentMesoManager, SIGNAL(pageReady(QQuickItem*,uint)), this, SLOT(bridge(QQuickItem*,uint)));
			connect(m_currentMesoManager, SIGNAL(itemReady(QQuickItem*,uint)), this, SIGNAL(getItem(QQuickItem*,uint)));
		}
		if (m_MesoId >= 0)
			m_appSettings->setValue("lastViewedMesoId", m_MesoId);
		mesoCalendarModel = m_currentMesoManager->mesoCalendarModel();
	}
}

void DbManager::getMesocycle(const uint meso_idx)
{
	if (meso_idx != m_MesoIdx)
		setWorkingMeso(meso_idx);

	if (m_currentMesoManager->getMesoPage() != nullptr)
	{
		addMainMenuShortCut(mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getMesoPage());
		return;
	}
	m_expectedPageId = mesoPageCreateId;
	connect(this, &DbManager::internalSignal, this, [&] (const uint id ) { if (id == mesoPageCreateId)
				addMainMenuShortCut(mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getMesoPage());
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	m_currentMesoManager->createMesocyclePage();
}

void DbManager::createNewMesocycle(const bool bRealMeso, const QString& name, const bool bCreatePage)
{
	QDate startDate, endDate, minimumStartDate;
	if (mesocyclesModel->count() == 0)
	{
		minimumStartDate.setDate(2023, 0, 2); //first monday of that year
		startDate = QDate::currentDate();
		if (bRealMeso)
			endDate = runCmd()->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (mesocyclesModel->getInt(mesocyclesModel->count() - 1, 8) == 1)
			minimumStartDate = runCmd()->getMesoStartDate(mesocyclesModel->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		if (bRealMeso)
			endDate = runCmd()->createFutureDate(minimumStartDate, 0, 2, 0);
	}
	mesocyclesModel->appendList(QStringList() << u"-1"_qs << name << QString::number(startDate.toJulianDay()) <<
		(bRealMeso ? QString::number(endDate.toJulianDay()) : u"0"_qs) << QString() <<
		(bRealMeso ? QString::number(runCmd()->calculateNumberOfWeeks(startDate, endDate)) : u"0"_qs) <<
		u"ABCR"_qs << QString() << QString() << QString() << QString() << (bRealMeso ? u"1"_qs : u"0"_qs));

	mesoSplitModel->appendList(QStringList() << u"-1"_qs << u"-1"_qs << QString() << QString() <<
				QString() << QString() << QString() << QString() );

	setWorkingMeso(mesocyclesModel->count()-1);
	if (bCreatePage)
	{
		m_expectedPageId = mesoPageCreateId;
		connect(this, &DbManager::internalSignal, this, [&] (const uint id ) { if (id == mesoPageCreateId)
				addMainMenuShortCut(mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getMesoPage());
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		m_currentMesoManager->createMesocyclePage(minimumStartDate, bRealMeso ?
								runCmd()->createFutureDate(startDate,0,6,0) : QDate(2026,11,31), startDate);
	}
}

void DbManager::saveMesocycle(const bool bChangeCalendar, const bool bPreserveOldCalendar, const bool bPreserveUntillYesterday)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));

	if (mesocyclesModel->getIntFast(m_MesoIdx, MESOCYCLES_COL_ID) == -1)
	{
		if (mb_importMode)
			worker->setWaitForThreadToFinish(true);

		connect( this, &DbManager::databaseReady, this, [&,worker] (const uint db_id) {
			if (db_id == worker->uniqueID()) {
				mesoSplitModel->setFast(m_MesoIdx, 1, mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_ID));
				saveMesoSplit();
			}
		});
	}
	else
	{
		saveMesoSplit();
		if (bChangeCalendar)
			changeMesoCalendar(mesocyclesModel->getDateFast(m_MesoIdx, MESOCYCLES_COL_STARTDATE),
				mesocyclesModel->getDateFast(m_MesoIdx, MESOCYCLES_COL_ENDDATE),
				mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_SPLIT), bPreserveOldCalendar, bPreserveUntillYesterday);
	}
	worker->addExecArg(m_MesoIdx);
	createThread(worker, [worker] () { worker->saveMesocycle(); } );
	m_totalSplits = mesocyclesModel->getTotalSplits(m_MesoIdx);
}

void DbManager::removeMesocycle(const uint meso_idx)
{
	const int meso_id(mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID));
	mesocyclesModel->removeFromList(meso_idx);

	if (meso_idx == m_MesoIdx)
	{
		if (meso_id >= 0)
		{
			removeMainMenuShortCut(m_currentMesoManager->getCalendarPage());
			removeMainMenuShortCut(m_currentMesoManager->getExercisesPlannerPage());
			removeMainMenuShortCut(m_currentMesoManager->getMesoPage());
		}

		int idx(m_MesoManager.count() - 1);
		if (idx == meso_idx)
			--idx;
		setWorkingMeso(idx);
	}

	if (meso_id >= 0)
	{
		removeMesoCalendar(meso_id);
		removeMesoSplit(mesoSplitModel->getFast(meso_idx, 0));
		mesoSplitModel->removeFromList(meso_idx);
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		worker->addExecArg(QString::number(meso_id));
		createThread(worker, [worker] () { return worker->removeEntry(); } );

		for (uint i(meso_idx+1); i < m_MesoManager.count(); ++i)
			m_MesoManager.at(i)->changeMesoIdxFromPages(i-1);
	}

	TPMesocycleClass* tpObject(m_MesoManager.at(meso_idx));
	m_MesoManager.removeAt(meso_idx);
	tpObject->disconnect();
	delete tpObject;
}

//Cannot order a mesocycle removal from within the qml page that will be deleted. Defer the removal to a little time after,
//just enough to have StackView remove the page
void DbManager::scheduleMesocycleRemoval(const uint meso_idx)
{
	QTimer::singleShot(200, this, [&,meso_idx] () { removeMesocycle(meso_idx); } );
}

void DbManager::deleteMesocyclesTable(const bool bRemoveFile)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DbManager::getMesoSplit(const QString& mesoid)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(mesoid);
	createThread(worker, [worker] () { worker->getMesoSplit(); } );
}

void DbManager::saveMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoIdx);
	createThread(worker, [worker] () { worker->saveMesoSplit(); } );
	m_currentMesoManager->updateMuscularGroup(mesoSplitModel);
}

void DbManager::removeMesoSplit(const QString& id)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->removeEntry(); } );
}

void DbManager::deleteMesoSplitTable(const bool bRemoveFile)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DbManager::createExercisesPlannerPage()
{
	if (m_currentMesoManager->getExercisesPlannerPage())
	{
		addMainMenuShortCut(tr("Exercises Planner: ") + mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getExercisesPlannerPage());
		return;
	}
	m_expectedPageId = exercisesPlannerCreateId;
	connect(this, &DbManager::internalSignal, this, [&] (const uint id ) { if (id == exercisesPlannerCreateId)
				addMainMenuShortCut(tr("Exercises Planner: ") + mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getExercisesPlannerPage());
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	m_currentMesoManager->createPlannerPage();
}

void DbManager::loadCompleteMesoSplits(const bool bThreaded)
{
	const QString mesoSplit(mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_SPLIT));
	QString mesoLetters;
	DBMesoSplitModel* splitModel(nullptr);
	m_nSplits = m_totalSplits;
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
		splitModel = m_currentMesoManager->getSplitModel(static_cast<QChar>(*itr));

		if (bThreaded)
		{
			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, splitModel));
			worker->addExecArg(m_MesoIdStr);
			worker->addExecArg(static_cast<QChar>(*itr));
			if (!connected)
			{
				connect( this, &DbManager::databaseReady, this, [&] (const uint db_id) {
					MSG_OUT("loadCompleteMesoSplits received databaseReady() " << db_id)
					if (m_WorkerLock[MESOSPLIT_TABLE_ID].hasID(db_id))
					{
						if (--m_nSplits == 0)
						{
							mb_splitsLoaded = true;
							emit internalSignal(SPLITS_LOADED_ID);
						}
					}
				});
				connected = true;
			}
			createThread(worker, [worker] () { return worker->getCompleteMesoSplit(); } );
		}
		else
		{
			if (!worker2)
			{
				worker2 = new DBMesoSplitTable(m_DBFilePath, m_appSettings, splitModel);
				worker2->addExecArg(m_MesoIdStr);
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

void DbManager::getCompleteMesoSplit()
{
	m_nSplits = m_totalSplits;
	connect(this, &DbManager::getPage, this, [&] (QQuickItem* item, const uint id) { if (id <= 6) {
			QMetaObject::invokeMethod(m_currentMesoManager->getExercisesPlannerPage(), "insertSplitPage",
								Q_ARG(QQuickItem*, item), Q_ARG(int, static_cast<int>(id)));
			if (--m_nSplits == 0)
				disconnect(this, &DbManager::getPage, this, nullptr);
		}
	});

	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&] (const uint id) { if (id == SPLITS_LOADED_ID) {
				return m_currentMesoManager->createMesoSplitPage(); disconnect(this, &DbManager::internalSignal, this, nullptr); } } );
		loadCompleteMesoSplits();
	}
	else
		m_currentMesoManager->createMesoSplitPage();
}

void DbManager::saveMesoSplitComplete(DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, model));
	worker->addExecArg(m_MesoIdStr);
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); } );
}

bool DbManager::mesoHasPlan(const uint meso_id, const QString& splitLetter) const
{
	if (splitLetter != u"R"_qs)
	{
		DBMesoSplitTable* meso_split(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
		const bool ret(meso_split->mesoHasPlan(QString::number(meso_id), splitLetter));
		meso_split->deleteLater();
		return ret;
	}
	return false;
}

void DbManager::loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, model));
	worker->addExecArg(QString::number(prev_meso_id));
	worker->addExecArg(model->splitLetter().at(0));
	createThread(worker, [worker] () { worker->getCompleteMesoSplit(); } );
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

QString DbManager::checkIfSplitSwappable(const QString& splitLetter) const
{
	if (mesoHasPlan(m_MesoId, splitLetter))
	{
		QString muscularGroup1(mesoSplitModel->get(m_MesoIdx, static_cast<int>(splitLetter.at(0).toLatin1()) - static_cast<int>('A') + 2));
		if (!muscularGroup1.isEmpty())
		{
			QString muscularGroup2;
			const QString mesoSplit(mesocyclesModel->get(m_MesoIdx, 6));
			muscularGroupSimplified(muscularGroup1);

			QString::const_iterator itr(mesoSplit.constBegin());
			const QString::const_iterator itr_end(mesoSplit.constEnd());

			do {
				if (static_cast<QChar>(*itr) == QChar('R'))
					continue;
				else if (static_cast<QChar>(*itr) == splitLetter.at(0))
					continue;

				muscularGroup2 = mesoSplitModel->get(m_MesoIdx, static_cast<int>((*itr).toLatin1()) - static_cast<int>('A') + 2);
				if (!muscularGroup2.isEmpty())
				{
					muscularGroupSimplified(muscularGroup2);
					if (runCmd()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
						return QString(*itr);
				}
			} while (++itr != itr_end);
		}
	}
	return QString();
}

void DbManager::swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2)
{
	m_currentMesoManager->swapPlans(splitLetter1, splitLetter2);
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_currentMesoManager->getSplitModel(splitLetter1.at(0))));
	worker->addExecArg(m_MesoIdStr);
	createThread(worker, [worker] () { worker->saveMesoSplitComplete(); } );
	DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_currentMesoManager->getSplitModel(splitLetter2.at(0))));
	worker2->addExecArg(m_MesoIdStr);
	createThread(worker2, [worker2] () { worker2->saveMesoSplitComplete(); } );
}

void DbManager::exportMesoSplit(const QString& splitLetter, const bool bShare, const bool bFancy, QFile* outFileInUse)
{
	QString mesoSplit;
	QString suggestedName;
	QFile* outFile(nullptr);

	if (!outFileInUse)
	{
		if (splitLetter == u"X"_qs)
		{
			mesoSplit = mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_SPLIT);
			suggestedName = mesocyclesModel->getFast(m_MesoIdx, 1) + tr(" - Exercises Plan.txt");
		}
		else
		{
			mesoSplit = splitLetter;
			suggestedName = mesocyclesModel->getFast(m_MesoIdx, 1) + tr(" - Exercises Plan - Split ") + splitLetter + u".txt"_qs;
		}
		setExportFileName(suggestedName);
	}
	else
	{
		outFile = outFileInUse;
		mesoSplit = mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_SPLIT);
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
		bExportToFileOk &= exportToFile(m_currentMesoManager->getSplitModel(static_cast<QChar>(*itr)), exportFileName(), bFancy, outFile);
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
void DbManager::getMesoCalendar(const bool bCreatePage)
{
	if (!mesoCalendarModel->isReady())
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
		worker->addExecArg(m_MesoId);
		if (bCreatePage)
		{
			connect( this, &DbManager::databaseReady, this, [&,worker,bCreatePage] (const uint db_id) {
				if (db_id == worker->uniqueID()) return getMesoCalendar(bCreatePage); } );
		}
		createThread(worker, [worker] () { worker->getMesoCalendar(); });
		return;
	}
	if (bCreatePage)
	{
		if (m_currentMesoManager->getCalendarPage() != nullptr)
		{
			addMainMenuShortCut(tr("Calendar: ") + mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getCalendarPage());
			return;
		}
		m_expectedPageId = calPageCreateId;
		connect(this, &DbManager::internalSignal, this, [&] (const uint id ) { if (id == calPageCreateId)
				addMainMenuShortCut(tr("Calendar: ") + mesocyclesModel->getFast(m_MesoIdx, 1), m_currentMesoManager->getCalendarPage());
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		m_currentMesoManager->createMesoCalendarPage();
	}
}

void DbManager::createMesoCalendar()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	createThread(worker, [worker] () { worker->createMesoCalendar(); } );
	if (m_currentMesoManager->getMesoPage())
		m_currentMesoManager->getMesoPage()->setProperty("bCalendarCreated", true);
}

void DbManager::changeMesoCalendar(const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday)
{
	if (!mesoCalendarModel->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,newStartDate,newEndDate,newSplit,bPreserveOldInfo,bPreserveOldInfoUntilToday] ()
		{
			return changeMesoCalendar(newStartDate,newEndDate,newSplit,bPreserveOldInfo,bPreserveOldInfoUntilToday);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(m_MesoId);
	worker->addExecArg(newStartDate);
	worker->addExecArg(newEndDate);
	worker->addExecArg(newSplit);
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilToday);
	if (!bPreserveOldInfo)
	{
		connect( this, &DbManager::databaseReady, this, [&,worker,newStartDate,newEndDate,newSplit] (const uint db_id) {
				if (db_id == worker->uniqueID())
					return m_currentMesoManager->updateOpenTDayPagesWithNewCalendarInfo(newStartDate, newEndDate, newSplit); });
	}
	createThread(worker, [worker] () { worker->changeMesoCalendar(); } );
}

void DbManager::updateMesoCalendarModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter)
{
	if (!mesoCalendarModel->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,mesoSplit,startDate,splitLetter] ()
		{
			return updateMesoCalendarModel(mesoSplit,startDate,splitLetter);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(m_MesoId); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(mesoSplit);
	worker->addExecArg(startDate);
	worker->addExecArg(splitLetter);
	connect( this, &DbManager::databaseReady, this, [&,worker,startDate,mesoSplit] (const uint db_id) {
				if (db_id == worker->uniqueID())
				{
					return m_currentMesoManager->updateOpenTDayPagesWithNewCalendarInfo(
					startDate, mesocyclesModel->getDateFast(m_MesoIdx, MESOCYCLES_COL_ENDDATE), mesoSplit);
				} });
	createThread(worker, [worker] () { worker->updateMesoCalendar(); } );
}

void DbManager::updateMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit, const bool bDayIsFinished)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(calDate);
	worker->setData(QString::number(calNDay), calSplit, bDayIsFinished ? u"1"_qs : u"0"_qs);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DbManager::setDayIsFinished(const QDate& date, const bool bFinished)
{
	if (!mesoCalendarModel->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,date,bFinished] ()
		{
			return setDayIsFinished(date, bFinished);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}
	m_currentMesoManager->gettDayModel(date)->setDayIsFinished(bFinished);
	mesoCalendarModel->setDayIsFinished(date, bFinished);
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(date);
	worker->addExecArg(bFinished);
	createThread(worker, [worker] () { worker->updateDayIsFinished(); } );
}

void DbManager::removeMesoCalendar(const uint meso_id)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(QString::number(meso_id));
	createThread(worker, [worker] () { return worker->removeMesoCalendar(); } );
}

void DbManager::deleteMesoCalendarTable(const bool bRemoveFile)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DbManager::getTrainingDay(const QDate& date)
{
	if (m_currentMesoManager->gettDayPage(date) != nullptr)
	{
		//m_currentMesoManager->setCurrenttDay(date);
		addMainMenuShortCut(tr("Workout: ") + runCmd()->formatDate(date), m_currentMesoManager->gettDayPage(date));
		return;
	}

	if (mesoCalendarModel->count() == 0)
	{
		connect( this, &DbManager::databaseReady, this, [&,date] () { getTrainingDay(date); },
				static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}

	m_expectedPageId = tDayPageCreateId;
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->gettDayModel(date)));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) m_currentMesoManager->createTrainingDayPage(date, mesoCalendarModel); } );
	connect( this, &DbManager::internalSignal, this, [&,date] (const uint id ) {
				if (id == tDayPageCreateId)
				{
					addMainMenuShortCut(tr("Workout: ") + runCmd()->formatDate(date), m_currentMesoManager->gettDayPage(date));
					m_currentMesoManager->currenttDayPage()->setProperty("dayIsNotCurrent", date != QDate::currentDate());
					getTrainingDayExercises(date);
				} }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	createThread(worker, [worker] () { return worker->getTrainingDay(); } );
}

void DbManager::getTrainingDayExercises(const QDate& date)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	worker->addExecArg(m_MesoIdStr);
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) return verifyTDayOptions(date); });
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DbManager::verifyTDayOptions(const QDate& date, const QString& splitLetter)
{
	if (m_currentMesoManager->currenttDayModel()->exerciseCount() > 0)
	{
		m_currentMesoManager->createExercisesObjects();
		return;
	}

	const QString splitletter(splitLetter.isEmpty() ? mesoCalendarModel->getSplitLetter(date.month(), date.day()-1) : splitLetter);
	m_currentMesoManager->currenttDayPage()->setProperty("bHasMesoPlan", mesoHasPlan(m_MesoId, splitletter));

	if (splitletter >= u"A"_qs && splitletter <= u"F"_qs)
	{
		DBTrainingDayModel* tempModel(new DBTrainingDayModel(this));
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, tempModel));
		worker->addExecArg(m_MesoIdStr);
		worker->addExecArg(splitletter);
		worker->addExecArg(QString::number(date.toJulianDay()));
		createThread(worker, [worker] () { return worker->getPreviousTrainingDaysInfo(); } );
	}
}

void DbManager::clearExercises()
{
	m_currentMesoManager->clearExercises();
	verifyTDayOptions(m_currentMesoManager->currenttDayModel()->date(), m_currentMesoManager->currenttDayModel()->splitLetter());
}

void DbManager::loadExercisesFromDate(const QString& strDate)
{
	const QDate date(runCmd()->getDateFromStrDate(strDate));
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	worker->addExecArg(m_MesoIdStr);
	worker->addExecArg(QString::number(date.toJulianDay()));

	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect( this, &DbManager::databaseReady, this, [&,date,worker] (const uint db_id) {
				if (db_id == worker->uniqueID()) {
					if (m_currentMesoManager->currenttDayModel()->date() != QDate::currentDate())
						m_currentMesoManager->currenttDayModel()->setDayIsFinished(true);
					else
						m_currentMesoManager->currenttDayModel()->setModified(true);
					return m_currentMesoManager->createExercisesObjects();
				} });
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(true); });
}

void DbManager::loadExercisesFromMesoPlan(const QString& splitLetter)
{
	const QChar splitletter(splitLetter.at(0));
	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&,splitLetter] (const uint id) { if (id == SPLITS_LOADED_ID)
			loadExercisesFromMesoPlan(splitLetter); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		loadCompleteMesoSplits();
	}
	else
	{
		m_currentMesoManager->currenttDayModel()->convertMesoSplitModelToTDayModel(m_currentMesoManager->getSplitModel(splitletter));
		m_currentMesoManager->createExercisesObjects();
		if (m_currentMesoManager->currenttDayModel()->date() != QDate::currentDate())
			m_currentMesoManager->currenttDayModel()->setDayIsFinished(true);
	}
}

void DbManager::convertTDayToPlan(DBTrainingDayModel* tDayModel)
{
	const QChar splitletter(tDayModel->splitLetter().at(0));
	if (!mb_splitsLoaded)
	{
		connect( this, &DbManager::internalSignal, this, [&,tDayModel] (const uint id) { if (id == SPLITS_LOADED_ID)
				return convertTDayToPlan(tDayModel); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		loadCompleteMesoSplits();
	}
	else
	{
		DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_currentMesoManager->getSplitModel(splitletter)));
		worker->addExecArg(m_MesoIdStr);
		worker->addExecArg(tDayModel->splitLetter());
		createThread(worker, [worker, tDayModel] () { return worker->convertTDayExercisesToMesoPlan(tDayModel); } );
	}
}

void DbManager::saveTrainingDay()
{
	if (m_currentMesoManager->currenttDayModel()->modified())
	{
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
		createThread(worker, [worker] () { return worker->saveTrainingDay(); } );
	}
}

void DbManager::removeTrainingDay()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DbManager::deleteTrainingDayTable(const bool bRemoveFile)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	createThread(worker, [worker,bRemoveFile] () { return bRemoveFile ? worker->removeDBFile() : worker->clearTable(); } );
}

void DbManager::exportTrainingDay(const QDate& date, const QString& splitLetter, const bool bShare, const bool bFancy)
{
	const QString suggestedName(tr(" - Workout ") + splitLetter + u".txt"_qs);
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	if (exportToFile(m_currentMesoManager->gettDayModel(date), exportFileName(), bFancy, outFile))
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

uint DbManager::getWorkoutNumberForTrainingDay(const QDate& date) const
{
	return mesoCalendarModel->getLastTrainingDayBeforeDate(date) + 1;
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
	}
}

void DbManager::openClientsOrCoachesPage(const bool bManageCoaches)
{
	if (m_clientsOrCoachesPage != nullptr)
	{
		m_clientsOrCoachesPage->setProperty("showUsers", !bManageCoaches);
		m_clientsOrCoachesPage->setProperty("showCoaches", bManageCoaches);
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
		return;
	}

	bool showUsers(false);
	bool showCoaches(false);
	int curUserRow(0);
	switch (userModel->appUseMode(0))
	{
		case APP_USE_MODE_SINGLE_USER: return;
		case APP_USE_MODE_SINGLE_COACH:
			if (bManageCoaches)
				return;
			if ((curUserRow = userModel->findFirstUser(false)) < 0)
				curUserRow = userModel->addUser(false);
			showUsers = true;
		break;
		case APP_USE_MODE_SINGLE_USER_WITH_COACH:
			if (!bManageCoaches)
				return;
			if ((curUserRow = userModel->findFirstUser(true)) < 0)
				curUserRow = userModel->addUser(true);
			showCoaches = true;
		break;
		case APP_USE_MODE_COACH_USER_WITH_COACHES:
		{
			showUsers = (curUserRow = userModel->findFirstUser(false)) > 0;
			if (!showUsers)
				showCoaches = (curUserRow = userModel->findFirstUser(true)) > 0;
			if (!showUsers && !showCoaches)
				curUserRow = userModel->addUser(false);
			showUsers = showCoaches = true;
		}
	}

	m_clientsOrCoachesProperties.insert(QStringLiteral("showUsers"), showUsers);
	m_clientsOrCoachesProperties.insert(QStringLiteral("showCoaches"), showCoaches);
	m_clientsOrCoachesProperties.insert(QStringLiteral("curUserRow"), curUserRow);
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
