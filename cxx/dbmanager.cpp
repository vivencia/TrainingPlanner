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

#ifdef Q_OS_ANDROID

#include "urihandler.h"

#include <QJniObject>
#include <qnativeinterface.h>
#include <QtCore/6.6.3/QtCore/private/qandroidextras_p.h>

void DbManager::checkPendingIntents()
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
bool DbManager::sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId)
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

#else
extern "C"
{
	#include <unistd.h>
}
#endif

DbManager::DbManager(QSettings* appSettings, RunCommands* runcommands)
	: QObject (nullptr), m_MesoId(-2), m_MesoIdx(99999), mb_splitsLoaded(false), mb_importMode(false), m_appSettings(appSettings),
		m_runCommands(runcommands), m_exercisesPage(nullptr)
{}

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

void DbManager::restartApp()
{
	#ifdef Q_OS_ANDROID
	/*auto activity = QtAndroid::androidActivity();
	auto packageManager = activity.callObjectMethod("getPackageManager",
												"()Landroid/content/pm/PackageManager;");

	auto activityIntent = packageManager.callObjectMethod("getLaunchIntentForPackage",
													  "(Ljava/lang/String;)Landroid/content/Intent;",
													  activity.callObjectMethod("getPackageName",
													  "()Ljava/lang/String;").object());

	auto pendingIntent = QAndroidJniObject::callStaticObjectMethod("android/app/PendingIntent", "getActivity",
															   "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;",
															   activity.object(), jint(0), activityIntent.object(),
															   QAndroidJniObject::getStaticField<jint>("android/content/Intent",
																									   "FLAG_ACTIVITY_CLEAR_TOP"));

	auto alarmManager = activity.callObjectMethod("getSystemService",
											  "(Ljava/lang/String;)Ljava/lang/Object;",
											  QAndroidJniObject::getStaticObjectField("android/content/Context",
																					  "ALARM_SERVICE",
																					  "Ljava/lang/String;").object());

	alarmManager.callMethod<void>("set",
							  "(IJLandroid/app/PendingIntent;)V",
							  QAndroidJniObject::getStaticField<jint>("android/app/AlarmManager", "RTC"),
							  jlong(QDateTime::currentMSecsSinceEpoch() + 100), pendingIntent.object());

	qApp->quit();*/
	#else
	char* args[2] = { nullptr, nullptr };
	const QString argv0(qApp->arguments().at(0));
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	exitApp();
	#endif
}

void DbManager::setQmlEngine(QQmlApplicationEngine* QMlEngine)
{
	m_QMlEngine = QMlEngine;

	//QML type registration
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBTrainingDayModel");
	qmlRegisterType<TPTimer>("com.vivenciasoftware.qmlcomponents", 1, 0, "TPTimer");

	mesoSplitModel = new DBMesoSplitModel(this, false);
	mesocyclesModel = new DBMesocyclesModel(this);

	//Enable only when necessary to avoid problems
	/*if (m_appSettings->value("appVersion") != TP_APP_VERSION)
	{
		//All the update code goes in here
		//updateDB(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
		updateDB(new DBTrainingDayTable(m_DBFilePath, m_appSettings));
		m_appSettings->setValue("appVersion", TP_APP_VERSION);
	}*/

	getAllMesocycles();

	m_mainWindow = static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0));
	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("runCmd"), QVariant::fromValue(m_runCommands) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("itemManager"), QVariant::fromValue(m_currentMesoManager) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appTr"), QVariant::fromValue(appTr()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoSplitModel"), QVariant::fromValue(mesoSplitModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("exercisesListModel"), QVariant::fromValue(exercisesListModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoCalendarModel"), QVariant::fromValue(mesoCalendarModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("lightIconFolder"), QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("darkIconFolder"), QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor1"), QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor2"), QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mainwindow"), QVariant::fromValue(m_mainWindow) });

	QQuickItem* appStackView(m_mainWindow->findChild<QQuickItem*>(u"appStackView"_qs));
	properties.append(QQmlContext::PropertyPair{ u"appStackView"_qs, QVariant::fromValue(appStackView) });

	QQuickItem* contentItem(appStackView->parentItem());
	properties.append(QQmlContext::PropertyPair{ u"windowHeight"_qs, contentItem->height() }); //mainwindow.height: 640 - footer.height - header.height
	properties.append(QQmlContext::PropertyPair{ u"windowWidth"_qs, contentItem->width() });

	m_QMlEngine->rootContext()->setContextProperties(properties);

	QMetaObject::invokeMethod(m_mainWindow, "init", Qt::AutoConnection);

	mAppDataFilesPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0) + u"/"_qs;
#ifndef Q_OS_ANDROID
	processArguments();
#else
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(m_runCommands, &RunCommands::appResumed, this, &DbManager::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(m_mainWindow, "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	checkPendingIntents();
#endif
}

DbManager::~DbManager()
{
	delete mesoSplitModel;
	delete exercisesListModel;
	delete mesocyclesModel;
	delete m_exercisesPage;
	for(uint i(0); i < m_MesoManager.count(); ++i)
		delete m_MesoManager.at(i);
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
{
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
				if (dbObj->objectName() == DBMesoCalendarObjectName)
				{
					if (mesoCalendarModel->count() == 0)
					{
						mesoCalendarModel->createModel( m_MesoId, mesocyclesModel->getDateFast(m_MesoIdx, 2),
								mesocyclesModel->getDateFast(m_MesoIdx, 3), mesocyclesModel->getFast(m_MesoIdx, 6) );
						createMesoCalendar();
					}
				}
				else if (dbObj->objectName() == DBTrainingDayObjectName)
				{
					DBTrainingDayModel* tempModel(static_cast<DBTrainingDayModel*>(static_cast<DBTrainingDayTable*>(dbObj)->model()));
					if (tempModel->count() > 0)
					{
						m_currentMesoManager->currenttDayPage()->setProperty("previousTDays", QVariant::fromValue(tempModel->getRow_const(0)));
						m_currentMesoManager->currenttDayPage()->setProperty("bHasPreviousTDays", true);
					}
					else
					{
						m_currentMesoManager->currenttDayPage()->setProperty("previousTDays", QVariant::fromValue(QVariantList()));
						m_currentMesoManager->currenttDayPage()->setProperty("bHasPreviousTDays", false);
					}
					delete tempModel;
				}
			break;
			case OP_ADD:
				if (dbObj->objectName() == DBMesocyclesObjectName)
				{
					m_MesoIdx = mesocyclesModel->count() - 1;
					m_MesoId = mesocyclesModel->getFast(m_MesoIdx, 0).toUInt();
					m_appSettings->setValue("lastViewedMesoId", m_MesoId);
					m_MesoIdStr = QString::number(m_MesoId);
					m_currentMesoManager->setMesoId(m_MesoId);
					//if (m_currentMesoManager->getMesoPage())
					//	m_currentMesoManager->getMesoPage()->setProperty("mesoId", m_MesoId);
				}
			break;
		}
	}

	m_WorkerLock[dbObj->objectName()]--;
	if (m_WorkerLock[dbObj->objectName()] <= 0)
		cleanUp(dbObj);
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
			model = new DBMesocyclesModel(this);
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
			case EXERCISES_TABLE_ID: model = new DBExercisesModel; break;
			case MESOCYCLES_TABLE_ID: model = new DBMesocyclesModel; break;
			case MESOSPLIT_TABLE_ID: model = new DBMesoSplitModel; break;
			case TRAININGDAY_TABLE_ID: model = new DBTrainingDayModel; break;
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
		importFromModel(model);

	if (!inFile->atEnd())
		return importFromFile(filename, inFile);
	else
		return 0;
}

void DbManager::importFromModel(TPListModel* model)
{
	mb_importMode = true;
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			updateExercisesList(static_cast<DBExercisesModel*>(model));
		break;
		case MESOCYCLES_TABLE_ID:
			createNewMesocycle(model->getFast(0, MESOCYCLES_COL_REALMESO) == u"1"_qs, model->getFast(0, 1), false);
			for (uint i(MESOCYCLES_COL_ID); i <= MESOCYCLES_COL_REALMESO; ++i)
				mesocyclesModel->setFast(m_MesoIdx, i, model->getFast(0, i));
			saveMesocycle(true, false, false, false);
		break;
		case MESOSPLIT_TABLE_ID:
		{
			if (static_cast<DBMesoSplitModel*>(model)->completeSplit())
			{
				DBMesoSplitModel* splitModel(m_currentMesoManager->getSplitModel(static_cast<DBMesoSplitModel*>(model)->splitLetter().at(0)));
				splitModel->updateFromModel(model);
				updateMesoSplitComplete(splitModel);
				// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
				// by the time mb_splitsLoaded is ever checked upon
				mb_splitsLoaded = true;
			}
			else
			{
				for (uint i(0); i < 8; ++i)
					mesoSplitModel->setFast(m_MesoIdx, i, model->getFast(0, i));
				mesoSplitModel->setFast(m_MesoIdx, 1, m_MesoIdStr);
				newMesoSplit();
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			DBTrainingDayModel* tDayModel(m_currentMesoManager->gettDayModel(dayDate));
			tDayModel->updateFromModel(model);
			if (mesoCalendarModel->count() == 0)
			{
				connect( this, &DbManager::databaseReady, this, [&,dayDate] {
					connect( this, &DbManager::getPage, this, [&] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(dayDate), item);
							}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					return getTrainingDay(dayDate); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				getMesoCalendar(false);
			}
			else
			{
				connect( this, &DbManager::getPage, this, [&] (QQuickItem* item, const uint) {
						return addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(dayDate), item);
							}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				getTrainingDay(dayDate);
			}
		}
		break;
	}
	mb_importMode = false;
}

void DbManager::saveFileDialogClosed(QString finalFileName, bool bResultOK)
{
	int resultCode(-5);
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

void DbManager::updateDB(TPDatabaseTable* worker)
{
	createThread(worker, [worker] () { worker->updateDatabase(); } );
}

void DbManager::startThread(QThread* thread, TPDatabaseTable* dbObj)
{
	if (!thread->isFinished())
	{
		MSG_OUT("starting thread for " << dbObj->objectName())
		m_WorkerLock[dbObj->objectName()]++;
		thread->start();
		if (dbObj->waitForThreadToFinish())
			thread->wait();
	}
}

void DbManager::cleanUp(TPDatabaseTable* dbObj)
{
	MSG_OUT("cleanUp: " << dbObj->objectName());
	dbObj->disconnect();
	dbObj->deleteLater();
	dbObj->thread()->quit();
	MSG_OUT("calling databaseReady()");
	emit databaseReady();
}

void DbManager::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForDoneFunc( [&] (TPDatabaseTable* obj) { return gotResult(obj); } );

	QThread *thread = new QThread ();
	connect ( thread, &QThread::started, worker, execFunc );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_WorkerLock[worker->objectName()] <= 0)
		startThread(thread, worker);
	else
	{
		MSG_OUT("Database  " << worker->objectName() << "  Waiting for it to be free: " << m_WorkerLock[worker->objectName()])
		connect( this, &DbManager::databaseReady, this, [&,thread, worker] () {
			return DbManager::startThread(thread, worker); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
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

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DbManager::getAllExercises()
{
	if (exercisesListModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
		createThread(worker, [worker] () { worker->getAllExercises(); } );
	}
	else
		emit databaseReady();
}

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(QStringLiteral("0"), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { worker->newExercise(); } );
}

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->updateExercise(); } );
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
		connect( this, &DbManager::databaseReady, this, [&,connectPage] { return createExercisesListPage(connectPage); },
				static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		createThread(worker, [worker] () { return worker->getAllExercises(); } );
	}

	m_exercisesProperties.insert(QStringLiteral("bChooseButtonEnabled"), bChooseButtonEnabled);
	m_exercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
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
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->getAllMesocycles();

	for(uint i(0); i < mesocyclesModel->count(); ++i)
		getMesoSplit(mesocyclesModel->getFast(i, MESOCYCLES_COL_ID));
	setWorkingMeso(m_appSettings->value("lastViewedMesoId", 0).toUInt());
	delete worker;
}

void DbManager::setWorkingMeso(int meso_idx)
{
	if (meso_idx != m_MesoIdx)
	{
		if (meso_idx >= mesocyclesModel->count())
			meso_idx = mesocyclesModel->count() - 1;
		m_MesoId = mesocyclesModel->getIntFast(meso_idx, MESOCYCLES_COL_ID);
		m_MesoIdx = meso_idx;
		m_MesoIdStr = QString::number(m_MesoId);
		m_totalSplits = mesocyclesModel->getTotalSplits(m_MesoIdx);
		mesocyclesModel->setCurrentRow(m_MesoIdx);
		mesoSplitModel->setCurrentRow(m_MesoIdx);

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
			m_currentMesoManager = new TPMesocycleClass(m_MesoId, m_MesoIdx, m_QMlEngine, m_runCommands, this);
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
			endDate = m_runCommands->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (mesocyclesModel->getInt(mesocyclesModel->count() - 1, 8) == 1)
			minimumStartDate = m_runCommands->getMesoStartDate(mesocyclesModel->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		if (bRealMeso)
			endDate = m_runCommands->createFutureDate(minimumStartDate, 0, 2, 0);
	}
	mesocyclesModel->appendList(QStringList() << u"-1"_qs << name << QString::number(startDate.toJulianDay()) <<
		(bRealMeso ? QString::number(endDate.toJulianDay()) : u"0"_qs) << QString() <<
		(bRealMeso ? QString::number(m_runCommands->calculateNumberOfWeeks(startDate, endDate)) : u"0"_qs) <<
		u"ABCR"_qs << QString() << (bRealMeso ? u"1"_qs : u"0"_qs));

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
								m_runCommands->createFutureDate(startDate,0,6,0) : QDate(2026,11,31), startDate);
	}
}

void DbManager::saveMesocycle(const bool bNewMeso, const bool bChangeCalendar, const bool bPreserveOldCalendar, const bool bPreserveUntillYesterday)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->addExecArg(m_MesoIdx);

	if (bNewMeso)
	{
		if (mb_importMode)
			worker->setWaitForThreadToFinish(true);
		else
		{
			connect( this, &DbManager::databaseReady, this, [&] {
				mesoSplitModel->setFast(m_MesoIdx, 1, mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_ID));
				newMesoSplit();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		}
		createThread(worker, [worker] () { worker->newMesocycle(); } );
	}
	else
	{
		createThread(worker, [worker] () { worker->updateMesocycle(); } );
		updateMesoSplit();
		if (bChangeCalendar)
			changeMesoCalendar(mesocyclesModel->getDateFast(m_MesoIdx, MESOCYCLES_COL_STARTDATE),
				mesocyclesModel->getDateFast(m_MesoIdx, MESOCYCLES_COL_ENDDATE),
				mesocyclesModel->getFast(m_MesoIdx, MESOCYCLES_COL_SPLIT), bPreserveOldCalendar, bPreserveUntillYesterday);
	}
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
		if (m_currentMesoManager)
		{
			m_MesoManager.removeOne(m_currentMesoManager);
			delete m_currentMesoManager;
		}

		if (m_MesoManager.count() > 0)
		{
			const uint idx(m_MesoManager.count() - 1);
			setWorkingMeso(m_MesoManager.at(idx)->mesoIdx());
		}
		else
			setWorkingMeso(0);
	}
	else
	{
		for (uint i(0); i < m_MesoManager.count(); ++i)
		{
			if (m_MesoManager.at(i)->mesoIdx() == meso_idx)
			{
				TPMesocycleClass* meso(m_MesoManager.at(i));
				m_MesoManager.removeAt(i);
				delete meso;
			}
		}
	}

	if (meso_id >= 0)
	{
		removeMesoCalendar(meso_id);
		removeMesoSplit(meso_id);
		mesoSplitModel->removeFromList(meso_idx);
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		worker->addExecArg(QString::number(meso_id));
		createThread(worker, [worker] () { return worker->removeEntry(); } );

		for (uint i(meso_idx+1); i < m_MesoManager.count(); ++i)
			m_MesoManager.at(i)->changeMesoIdxFromPages(i-1);
	}
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

void DbManager::newMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoIdx);
	createThread(worker, [worker] () { worker->newMesoSplit(); } );
}

void DbManager::updateMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoIdx);
	createThread(worker, [worker] () { worker->updateMesoSplit(); } );
	m_currentMesoManager->updateMuscularGroup(mesoSplitModel);
}

void DbManager::removeMesoSplit(const uint meso_id)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(QString::number(meso_id));
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
			connect( this, &DbManager::databaseReady, this, [&] {
				if (--m_nSplits == 0) {
					mb_splitsLoaded = true;
					emit internalSignal(SPLITS_LOADED_ID);
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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

void DbManager::updateMesoSplitComplete(DBMesoSplitModel* model)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, model));
	worker->addExecArg(m_MesoIdStr);
	createThread(worker, [worker] () { worker->updateMesoSplitComplete(); } );
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
					if (m_runCommands->stringsAreSimiliar(muscularGroup1, muscularGroup2))
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
	worker->addExecArg(splitLetter1);
	createThread(worker, [worker] () { worker->updateMesoSplitComplete(); } );
	DBMesoSplitTable* worker2(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_currentMesoManager->getSplitModel(splitLetter2.at(0))));
	worker2->addExecArg(m_MesoIdStr);
	worker2->addExecArg(splitLetter2);
	createThread(worker2, [worker2] () { worker2->updateMesoSplitComplete(); } );
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
			connect( this, &DbManager::databaseReady, this, [&,bCreatePage] {
				return getMesoCalendar(bCreatePage); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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
		m_currentMesoManager->setCurrenttDay(date);
		addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(date), m_currentMesoManager->gettDayPage(date));
		return;
	}

	m_expectedPageId = tDayPageCreateId;
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->gettDayModel(date)));
	/*worker->addExecArg("13");
	worker->removeEntry();
	worker->clearExecArgs();*/
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date] { return m_currentMesoManager->createTrainingDayPage(date, mesoCalendarModel); },
			static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	connect(this, &DbManager::internalSignal, this, [&,date] (const uint id ) { if (id == tDayPageCreateId)
		{
			addMainMenuShortCut(tr("Workout: ") + m_runCommands->formatDate(date), m_currentMesoManager->gettDayPage(date));
			return getTrainingDayExercises(date);
		} }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	createThread(worker, [worker] () { return worker->getTrainingDay(); } );
}

void DbManager::getTrainingDayExercises(const QDate& date)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	worker->addExecArg(m_MesoIdStr);
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date] { return verifyTDayOptions(date); },
		static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
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
		createThread(worker, [worker] () { return worker->getPreviousTrainingDays(); } );
	}
}

void DbManager::clearExercises()
{
	m_currentMesoManager->clearExercises();
	verifyTDayOptions(m_currentMesoManager->currenttDayModel()->date(), m_currentMesoManager->currenttDayModel()->splitLetter());
}

void DbManager::loadExercisesFromDate(const QString& strDate)
{
	const QDate date(m_runCommands->getDateFromStrDate(strDate));
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_currentMesoManager->currenttDayModel()));
	worker->addExecArg(m_MesoIdStr);
	worker->addExecArg(QString::number(date.toJulianDay()));

	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect( this, &DbManager::databaseReady, this, [&,date] { m_currentMesoManager->currenttDayModel()->setModified(true);
			 return m_currentMesoManager->createExercisesObjects(); },
				static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
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
		if (m_currentMesoManager->currenttDayModel()->id() == -1)
			createThread(worker, [worker] () { return worker->newTrainingDay(); } );
		else
			createThread(worker, [worker] () { return worker->updateTrainingDay(); } );
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
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
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
	QMetaObject::invokeMethod(m_mainWindow, "stackViewPushExistingPage", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
