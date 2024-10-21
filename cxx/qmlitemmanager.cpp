#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "dbusermodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tptimer.h"
#include "osinterface.h"
#include "translationclass.h"
#include "qmlexerciseentry.h"
#include "qmlsetentry.h"
#include "qmluserinterface.h"
#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmltdayinterface.h"
#include "qmlsetentry.h"

#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QSettings>
#include <QFile>

QQmlApplicationEngine* QmlItemManager::_appQmlEngine(nullptr);
QQuickWindow* QmlItemManager::_appMainWindow(nullptr);
QmlUserInterface* QmlItemManager::_appUsersManager(nullptr);
QmlExercisesDatabaseInterface* QmlItemManager::_appExercisesListManager(nullptr);

QmlItemManager::QmlItemManager(QQmlApplicationEngine* qml_engine)
		: QObject{nullptr}
{
	appDBInterface()->init();
	_appQmlEngine = qml_engine;
	configureQmlEngine();

#ifdef Q_OS_ANDROID
	appOsInterface()->appStartUpNotifications();
#endif
}

void QmlItemManager::configureQmlEngine()
{
	QQuickStyle::setStyle(appSettings()->themeStyle());

	qmlRegisterType<DBUserModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBUserModel");
	qmlRegisterType<DBExercisesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "DBTrainingDayModel");
	qmlRegisterType<TPTimer>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPTimer");
	qmlRegisterType<TPImage>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TPImage");
	qmlRegisterType<QmlUserInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "UserManager");
	qmlRegisterType<QmlExercisesDatabaseInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExercisesListManager");
	qmlRegisterType<QMLMesoInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "MesoManager");
	qmlRegisterType<QmlMesoSplitInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SplitManager");
	qmlRegisterType<QmlTDayInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TDayManager");
	qmlRegisterType<QmlExerciseEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExerciseEntryManager");
	qmlRegisterType<QmlSetEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SetEntryManager");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties(7);
	properties[0] = QQmlContext::PropertyPair{ u"appSettings"_qs, QVariant::fromValue(appSettings()) };
	properties[1] = QQmlContext::PropertyPair{ u"appUtils"_qs, QVariant::fromValue(appUtils()) };
	properties[2] = QQmlContext::PropertyPair{ u"appTr"_qs, QVariant::fromValue(appTr()) };
	properties[3] = QQmlContext::PropertyPair{ u"userModel"_qs, QVariant::fromValue(appUserModel()) };
	properties[4] = QQmlContext::PropertyPair{ u"mesocyclesModel"_qs, QVariant::fromValue(appMesoModel()) };
	properties[5] = QQmlContext::PropertyPair{ u"exercisesModel"_qs, QVariant::fromValue(appExercisesModel()) };
	properties[6] = QQmlContext::PropertyPair{ u"itemManager"_qs, QVariant::fromValue(this) };
	appQmlEngine()->rootContext()->setContextProperties(properties);

	const QUrl& url{u"qrc:/qml/main.qml"_qs};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (const QObject* const obj, const QUrl& objUrl) {
		if (!obj && url == objUrl)
		{
			LOG_MESSAGE("*******************Mainwindow not loaded*******************")
			QCoreApplication::exit(-1);
		}
	});
	appQmlEngine()->addImportPath(u":/"_qs);
	appQmlEngine()->addImageProvider(u"tpimageprovider"_qs, new TPImageProvider{});
	appQmlEngine()->load(url);

	_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
	connect(appMainWindow(), SIGNAL(openFileChosen(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
	connect(appMainWindow(), SIGNAL(openFileRejected(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
	appQmlEngine()->rootContext()->setContextProperty(u"mainwindow"_qs, QVariant::fromValue(appMainWindow()));

	if (!appSettings()->mainUserConfigured())
		QMetaObject::invokeMethod(appMainWindow(), "showFirstUseTimeDialog");
	else
	{
		appMainWindow()->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
		appOsInterface()->initialCheck();
	}

	connect(appMesoModel(), &DBMesocyclesModel::mostRecentOwnMesoChanged, this, [this] (const int meso_idx) {
		appMainWindow()->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
	});
	connect(appUserModel(), &DBUserModel::mainUserConfigurationFinishedSignal, this, [this] () {
		appSettings()->setMainUserConfigured(true);
		appOsInterface()->initialCheck();
	});
	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE) {
			appMesoModel()->updateColumnLabels();
			appMainWindow()->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
		}
	});

	connect(appMainWindow(), SIGNAL(saveFileChosen(const QString&)), this, SLOT(exportSlot(const QString&)));
	connect(appMainWindow(), SIGNAL(saveFileRejected(const QString&)), this, SLOT(exportSlot(const QString&)));
}

void QmlItemManager::openMainMenuShortCut(const int button_id)
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
}

void QmlItemManager::tryToImport(const QList<bool>& selectedFields)
{
	uint wanted_content(0);
	wanted_content |= (m_fileContents & IFC_MESO) && selectedFields.at(0) ? IFC_MESO : 0;
	wanted_content |= (m_fileContents & IFC_MESO) && selectedFields.at(1) ? IFC_USER : 0;
	wanted_content |= (m_fileContents & IFC_MESOSPLIT) && (m_fileContents & IFC_MESO) ? selectedFields.at(2) : (selectedFields.at(0) ? IFC_MESOSPLIT : 0);
	wanted_content |= (m_fileContents & IFC_TDAY) && selectedFields.at(0) ? IFC_TDAY : 0;
	wanted_content |= (m_fileContents & IFC_EXERCISES) && selectedFields.at(0) ? IFC_EXERCISES : 0;
	importFromFile(m_importFilename, wanted_content);
}

void QmlItemManager::displayActivityResultMessage(const int requestCode, const int resultCode) const
{
	int message_id(0);
	switch (resultCode)
	{
		case -1: message_id = APPWINDOW_MSG_SHARE_OK; break;
		case 0: message_id = APPWINDOW_MSG_SHARE_FAILED; break;
		default: message_id = APPWINDOW_MSG_UNKNOWN_ERROR; break;
	}
	displayMessageOnAppWindow(message_id);
	QFile::remove(m_exportFilename);
}

void QmlItemManager::displayImportDialogMessage(const uint fileContents, const QString& filename)
{
	m_fileContents = fileContents;
	m_importFilename = filename;

	QStringList importOptions;
	if (m_fileContents & IFC_MESO)
	{
		importOptions.append(tr("Complete Training Plan"));
		if (m_fileContents & IFC_USER)
			importOptions.append(tr("Coach information"));
		if (m_fileContents & IFC_MESOSPLIT)
			importOptions.append(tr("Exercises Program"));
	}
	else
	{
		if (m_fileContents & IFC_MESOSPLIT)
			importOptions.append(tr("Exercises Program"));
		else if (m_fileContents & IFC_TDAY)
			importOptions.append(tr("Exercises database update"));
		else if (m_fileContents & IFC_EXERCISES)
			importOptions.append(tr("Exercises database update"));
	}

	const QList<bool> selectedFields(importOptions.count(), true);
	QMetaObject::invokeMethod(appMainWindow(), "createImportConfirmDialog", Q_ARG(QmlItemManager*, this),
				Q_ARG(QVariant, importOptions), Q_ARG(QVariant, QVariant::fromValue(selectedFields)));
}

void QmlItemManager::openRequestedFile(const QString& filename, const int wanted_content)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return;
	}

	uint fileContents(0);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (inData.startsWith(u"##"_qs))
				{
					if (inData.indexOf(DBUserObjectName) != -1)
						fileContents |= IFC_USER;
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
						fileContents |= IFC_MESOSPLIT;
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						fileContents |= IFC_MESO;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						fileContents |= IFC_TDAY;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						fileContents |= IFC_EXERCISES;
				}
			}
		}
	}
	if (fileContents != 0)
	{
		QmlItemManager* itemMngr(nullptr);
		if (fileContents & IFC_MESO & wanted_content)
		{
			const uint tempmeso_idx{createNewMesocycle(false)};
			itemMngr = m_itemManager.at(tempmeso_idx);
		}
		else
		{
			if (fileContents & IFC_MESO & wanted_content || fileContents & IFC_TDAY & wanted_content)
				itemMngr = m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx());
			else if (fileContents & IFC_EXERCISES & wanted_content)
				itemMngr = QmlManager();
		}
		if (itemMngr)
			itemMngr->displayImportDialogMessage(fileContents, filename);
		else
			displayMessageOnAppWindow(APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE);
	}
}

void QmlItemManager::importFromFile(const QString& filename, const int wanted_content)
{
	int importFileMessageId(0);
	if (wanted_content & IFC_MESO)
	{
		if (wanted_content & IFC_USER)
		{
			DBUserModel* usermodel{new DBUserModel};
			usermodel->deleteLater();
			importFileMessageId = usermodel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(usermodel);
		}

		DBMesocyclesModel* mesomodel{new DBMesocyclesModel};
		mesomodel->deleteLater();
		importFileMessageId = mesomodel->importFromFile(filename);
		if (importFileMessageId >= 0)
			incorporateImportedData(mesomodel);

		if (wanted_content & IFC_MESOSPLIT)
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel{nullptr}};
			splitModel->deleteLater();
			importFileMessageId = splitModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(splitModel);
		}
	}
	else
	{
		if (wanted_content & IFC_MESOSPLIT)
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel};
			splitModel->deleteLater();
			importFileMessageId = splitModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(splitModel);
		}
		else if (wanted_content & IFC_TDAY)
		{
			DBTrainingDayModel* tDayModel{new DBTrainingDayModel};
			tDayModel->deleteLater();
			importFileMessageId = tDayModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(tDayModel);
		}
		else if (wanted_content & IFC_EXERCISES)
		{
			DBExercisesModel* exercisesModel{new DBExercisesModel};
			exercisesModel->deleteLater();
			importFileMessageId = exercisesModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(exercisesModel);
		}
	}
	displayMessageOnAppWindow(importFileMessageId);
}

void QmlItemManager::incorporateImportedData(const TPListModel* const model)
{
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			appExercisesModel()->updateFromModel(model);
			//appDBInterface()->saveExercise();
		break;
		case USER_TABLE_ID:
			static_cast<void>(appUserModel()->updateFromModel(model));
			appDBInterface()->saveUser(appUserModel()->count()-1);
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(model))
			{
				const uint meso_idx = createNewMesocycle(false);
				appMesoModel()->updateFromModel(meso_idx, model);
				appDBInterface()->saveMesocycle(meso_idx);
			}
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* newSplitModel{static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model))};
			DBMesoSplitModel* splitModel{m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx())->getSplitModel(newSplitModel->splitLetter().at(0))};
			splitModel->updateFromModel(newSplitModel);
			appDBInterface()->saveMesoSplitComplete(splitModel);
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			DBTrainingDayModel* newTDayModel{static_cast<DBTrainingDayModel*>(const_cast<TPListModel*>(model))};
			DBTrainingDayModel* tDayModel{m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx())->gettDayModel(QDate::currentDate())};
			if (tDayModel->exerciseCount() == 0)
			{
				tDayModel->updateFromModel(newTDayModel);
				appDBInterface()->saveTrainingDay(tDayModel);
			}
			else
				; //Offer option to import into another day
		}
		break;
	}
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::mainWindowStarted() const
{
	appOsInterface()->initialCheck();
}

void QmlItemManager::displayMessageOnAppWindow(const int message_id, const QString& fileName) const
{
	QString title, message;
	switch (message_id)
	{
		case APPWINDOW_MSG_EXPORT_OK:
			title = tr("Succesfully exported");
			message = fileName;
		break;
		case APPWINDOW_MSG_SHARE_OK:
			title = tr("Succesfully shared");
			message = fileName;
		break;
		case APPWINDOW_MSG_IMPORT_OK:
			title = tr("Successfully imported");
			message = fileName;
		break;
		case APPWINDOW_MSG_OPEN_FAILED:
			title = tr("Failed to open file");
			message = fileName;
		break;
		case APPWINDOW_MSG_UNKNOWN_FILE_FORMAT:
			title = tr("Error");
			message = tr("File type not recognized");
		break;
		case APPWINDOW_MSG_CORRUPT_FILE:
			title = tr("Error");
			message = tr("File is formatted wrongly or is corrupted");
		break;
		case APPWINDOW_MSG_NOTHING_TODO:
			title = tr("Nothing to be done");
			message = tr("File had already been imported");
		break;
		case APPWINDOW_MSG_NO_MESO:
			title = tr("No program to import into");
			message = tr("Either create a new training plan or import from a complete program file");
		break;
		case APPWINDOW_MSG_NOTHING_TO_EXPORT:
			title = tr("Nothing to export");
			message = tr("Only exercises that do not come by default with the app can be exported");
		break;
		case APPWINDOW_MSG_SHARE_FAILED:
			title = tr("Sharing failed");
			message = fileName;
		break;
		case APPWINDOW_MSG_EXPORT_FAILED:
			title = tr("Export failed");
			message = tr("Operation canceled");
		break;
		case APPWINDOW_MSG_IMPORT_FAILED:
			title = tr("Import failed");
			message = tr("Operation canceled");
		break;
		case APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED:
			title = tr("Could not open file for exporting");
			message = fileName;
		break;
		case APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE:
			title = tr("Cannot import");
			message = tr("Contents of the file are incompatible with the requested operation");
		break;
		case APPWINDOW_MSG_UNKNOWN_ERROR:
			title = tr("Error");
			message = tr("Something went wrong");
		break;
	}
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::exportSlot(const QString& filePath)
{
	if (!filePath.isEmpty())
		QFile::copy(m_exportFilename, filePath);
	displayMessageOnAppWindow(filePath.isEmpty() ? APPWINDOW_MSG_EXPORT_FAILED : APPWINDOW_MSG_EXPORT_OK);
	QFile::remove(m_exportFilename);
}

void QmlItemManager::importSlot_FileChosen(const QString& filePath)
{
	if (!filePath.isEmpty())
		openRequestedFile(filePath);
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_FAILED);
}

//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
void QmlItemManager::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	if (m_mainMenuShortcutPages.contains(page))
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
	else
	{
		if (m_mainMenuShortcutPages.count() < 5)
		{
			QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
			QMetaObject::invokeMethod(appMainWindow(), "createShortCut", Q_ARG(QString, label),
													Q_ARG(QQuickItem*, page), Q_ARG(int, m_mainMenuShortcutPages.count()));
			m_mainMenuShortcutPages.append(page);
		}
		else
		{
			QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, page));
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

void QmlItemManager::removeMainMenuShortCut(QQuickItem* page)
{
	const int idx(m_mainMenuShortcutPages.indexOf(page));
	if (idx != -1)
	{
		QMetaObject::invokeMethod(appMainWindow(), "popFromStack", Q_ARG(QQuickItem*, page));
		m_mainMenuShortcutPages.remove(idx);
		delete m_mainMenuShortcutEntries.at(idx);
		m_mainMenuShortcutEntries.remove(idx);
		for (uint i(idx); i < m_mainMenuShortcutEntries.count(); ++i)
			m_mainMenuShortcutEntries.at(i)->setProperty("clickid", i);
	}
}
//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
