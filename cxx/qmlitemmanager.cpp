#include "qmlitemmanager.h"

#include "dbinterface.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbusermodel.h"

#include "qmlexerciseentry.h"
#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesosplitinterface.h"
#include "qmlsetentry.h"
#include "qmltdayinterface.h"
#include "qmluserinterface.h"

#include "osinterface.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tpsettings.h"
#include "statistics/tpstatistics.h"
#include "tptimer.h"
#include "translationclass.h"
#include "weather/weatherinfo.h"

#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>

QmlItemManager* QmlItemManager::_appItemManager(nullptr);
QQmlApplicationEngine* QmlItemManager::_appQmlEngine(nullptr);
QQuickWindow* QmlItemManager::_appMainWindow(nullptr);

QmlItemManager::QmlItemManager(QQmlApplicationEngine* qml_engine)
		: QObject{nullptr}, m_usersManager(nullptr), m_exercisesListManager(nullptr)
{
	_appItemManager = this;
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
	qmlRegisterType<QmlMesoCalendarInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "CalendarManager");
	qmlRegisterType<QmlMesoSplitInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SplitManager");
	qmlRegisterType<QmlTDayInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "TDayManager");
	qmlRegisterType<QmlExerciseEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExerciseEntryManager");
	qmlRegisterType<QmlSetEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "SetEntryManager");
	qmlRegisterType<WeatherInfo>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "WeatherInfo");
	qmlRegisterType<TPStatistics>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "Statistics");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties(8);
	properties[0] = QQmlContext::PropertyPair{ "appSettings"_L1, QVariant::fromValue(appSettings()) };
	properties[1] = QQmlContext::PropertyPair{ "appUtils"_L1, QVariant::fromValue(appUtils()) };
	properties[2] = QQmlContext::PropertyPair{ "appTr"_L1, QVariant::fromValue(appTr()) };
	properties[3] = QQmlContext::PropertyPair{ "userModel"_L1, QVariant::fromValue(appUserModel()) };
	properties[4] = QQmlContext::PropertyPair{ "mesocyclesModel"_L1, QVariant::fromValue(appMesoModel()) };
	properties[5] = QQmlContext::PropertyPair{ "exercisesModel"_L1, QVariant::fromValue(appExercisesModel()) };
	properties[6] = QQmlContext::PropertyPair{ "itemManager"_L1, QVariant::fromValue(this) };
	properties[7] = QQmlContext::PropertyPair{ "appStatistics"_L1, QVariant::fromValue(appStatistics()) };
	appQmlEngine()->rootContext()->setContextProperties(properties);

	const QUrl& url{"qrc:/qml/main.qml"_L1};
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (const QObject* const obj, const QUrl& objUrl) {
		if (!obj && url == objUrl)
		{
			LOG_MESSAGE("*******************Mainwindow not loaded*******************")
			QCoreApplication::exit(-1);
		}
	});
	appQmlEngine()->addImportPath(":/"_L1);
	appQmlEngine()->addImageProvider("tpimageprovider"_L1, new TPImageProvider{});
	appQmlEngine()->load(url);

	_appMainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
	connect(appMainWindow(), SIGNAL(openFileChosen(QString)), this, SLOT(importSlot_FileChosen(QString)));
	connect(appMainWindow(), SIGNAL(openFileRejected(QString)), this, SLOT(importSlot_FileChosen(QString)));
	appQmlEngine()->rootContext()->setContextProperty("mainwindow"_L1, QVariant::fromValue(appMainWindow()));

	if (!appSettings()->mainUserConfigured())
	{
		QMetaObject::invokeMethod(appMainWindow(), "showFirstUseTimeDialog");
		connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint) {
			appDBInterface()->saveUser(user_row);
		});
	}
	else
		appOsInterface()->initialCheck();

	connect(appUserModel(), &DBUserModel::mainUserConfigurationFinishedSignal, this, [this] () {
		disconnect(appUserModel(), nullptr, this, nullptr);
		appSettings()->setMainUserConfigured(true);
		appOsInterface()->initialCheck();
	});

	connect(appMainWindow(), SIGNAL(saveFileChosen(QString)), this, SLOT(exportSlot(QString)));
	connect(appMainWindow(), SIGNAL(saveFileRejected(QString)), this, SLOT(exportSlot(QString)));
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

void QmlItemManager::getSettingsPage(const uint startPageIndex)
{
	if (!m_usersManager)
		m_usersManager = new QmlUserInterface{this, appQmlEngine(), appMainWindow()};
	m_usersManager->getSettingsPage(startPageIndex);
}

void QmlItemManager::getExercisesPage(QQuickItem* connectPage)
{
	if (!m_exercisesListManager)
		m_exercisesListManager = new QmlExercisesDatabaseInterface{this, appQmlEngine(), appMainWindow()};
	m_exercisesListManager->getExercisesPage(connectPage);
}

const QString& QmlItemManager::setExportFileName(const QString& filename)
{
	m_exportFilename = appOsInterface()->appDataFilesPath() + filename;
	return m_exportFilename;
}

void QmlItemManager::continueExport(int exportMessageId, const bool bShare)
{
	const QString& filename{appUtils()->getFileName(m_exportFilename)};
	if (exportMessageId == APPWINDOW_MSG_EXPORT_OK)
	{
		if (bShare)
		{
			appOsInterface()->shareFile(m_exportFilename);
			exportMessageId = APPWINDOW_MSG_SHARE_OK;
		}
		else
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, filename));
	}
	displayMessageOnAppWindow(exportMessageId, filename);
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
		importOptions.append(std::move(tr("Complete Training Plan")));
		if (m_fileContents & IFC_USER)
			importOptions.append(std::move(tr("Coach information")));
		if (m_fileContents & IFC_MESOSPLIT)
			importOptions.append(std::move(tr("Exercises Program")));
	}
	else
	{
		if (m_fileContents & IFC_MESOSPLIT)
			importOptions.append(std::move(tr("Exercises Program")));
		else if (m_fileContents & IFC_TDAY)
			importOptions.append(std::move(tr("Exercises database update")));
		else if (m_fileContents & IFC_EXERCISES)
			importOptions.append(std::move(tr("Exercises database update")));
	}

	const QList<bool> selectedFields(importOptions.count(), true);
	QMetaObject::invokeMethod(appMainWindow(), "createImportConfirmDialog", Q_ARG(QmlItemManager*, this),
				Q_ARG(QVariant, importOptions), Q_ARG(QVariant, QVariant::fromValue(selectedFields)));
}

void QmlItemManager::openRequestedFile(const QString& filename, const int wanted_content)
{
	QFile* inFile{new QFile{filename}};
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
				if (inData.startsWith("##"_L1))
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
		if ((fileContents & IFC_MESO & wanted_content) || (fileContents & IFC_MESOSPLIT & wanted_content) ||
				(fileContents & IFC_TDAY & wanted_content) || (fileContents & IFC_EXERCISES & wanted_content))
			displayImportDialogMessage(fileContents, filename);
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
			DBUserModel* usermodel{new DBUserModel{this}};
			usermodel->deleteLater();
			importFileMessageId = usermodel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(usermodel);
		}

		DBMesocyclesModel* mesomodel{new DBMesocyclesModel{this}};
		mesomodel->deleteLater();
		importFileMessageId = mesomodel->importFromFile(filename);
		if (importFileMessageId >= 0)
			incorporateImportedData(mesomodel);

		if (wanted_content & IFC_MESOSPLIT)
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel{this, true}};
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
			DBMesoSplitModel* splitModel{new DBMesoSplitModel{this, true}};
			splitModel->deleteLater();
			importFileMessageId = splitModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(splitModel);
		}
		else if (wanted_content & IFC_TDAY)
		{
			DBTrainingDayModel* tDayModel{new DBTrainingDayModel{this}};
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

void QmlItemManager::incorporateImportedData(TPListModel* model)
{
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			appExercisesModel()->updateFromModel(model);
			appDBInterface()->saveExercises();
		break;
		case USER_TABLE_ID:
			static_cast<void>(appUserModel()->updateFromModel(model));
			appDBInterface()->saveUser(appUserModel()->count()-1);
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(model))
			{
				const uint meso_idx = appMesoModel()->createNewMesocycle(false);
				appMesoModel()->updateFromModel(meso_idx, model);
				appDBInterface()->saveMesocycle(meso_idx);
			}
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* newSplitModel(static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model)));
			DBMesoSplitModel* splitModel(appMesoModel()->mesoManager(appMesoModel()->currentMesoIdx())->plannerSplitModel(newSplitModel->_splitLetter()));
			if (splitModel) //exercises planner page for the current meso has been loaded in the session
			{
				splitModel->updateFromModel(newSplitModel);
				appDBInterface()->saveMesoSplitComplete(splitModel);
			}
			else //exercises planner page for the current meso has NOT been loaded in the session
				appDBInterface()->saveMesoSplitComplete(newSplitModel);
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			DBTrainingDayModel* newTDayModel(static_cast<DBTrainingDayModel*>(const_cast<TPListModel*>(model)));
			DBTrainingDayModel* tDayModel(appMesoModel()->mesoManager(appMesoModel()->currentMesoIdx())->tDayModelForToday());
			if (tDayModel)
			{
				tDayModel->updateFromModel(newTDayModel);
				appDBInterface()->saveTrainingDay(tDayModel);
			}
			else
				appDBInterface()->saveTrainingDay(newTDayModel);
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
			title = std::move(tr("Succesfully exported"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_SHARE_OK:
			title = std::move(tr("Succesfully shared"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_IMPORT_OK:
			title = std::move(tr("Successfully imported"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_OPEN_FAILED:
			title = std::move(tr("Failed to open file"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_UNKNOWN_FILE_FORMAT:
			title = std::move(tr("Error"));
			message = std::move(tr("File type not recognized"));
		break;
		case APPWINDOW_MSG_CORRUPT_FILE:
			title = std::move(tr("Error"));
			message = std::move(tr("File is formatted wrongly or is corrupted"));
		break;
		case APPWINDOW_MSG_NOTHING_TODO:
			title = std::move(tr("Nothing to be done"));
			message = std::move(tr("File had already been imported"));
		break;
		case APPWINDOW_MSG_NO_MESO:
			title = std::move(tr("No program to import into"));
			message = std::move(tr("Either create a new training plan or import from a complete program file"));
		break;
		case APPWINDOW_MSG_NOTHING_TO_EXPORT:
			title = std::move(tr("Nothing to export"));
			message = std::move(tr("Only exercises that do not come by default with the app can be exported"));
		break;
		case APPWINDOW_MSG_SHARE_FAILED:
			title = std::move(tr("Sharing failed"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_EXPORT_FAILED:
			title = std::move(tr("Export failed"));
			message = std::move(tr("Operation canceled"));
		break;
		case APPWINDOW_MSG_IMPORT_FAILED:
			title = std::move(tr("Import failed"));
			message = std::move(tr("Operation canceled"));
		break;
		case APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED:
			title = std::move(tr("Could not open file for exporting"));
			message = std::move(fileName);
		break;
		case APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE:
			title = std::move(tr("Cannot import"));
			message = std::move(tr("Contents of the file are incompatible with the requested operation"));
		break;
		case APPWINDOW_MSG_UNKNOWN_ERROR:
			title = std::move(tr("Error"));
			message = std::move(tr("Something went wrong"));
		break;
	}
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::exportSlot(const QString& filePath)
{
	if (!filePath.isEmpty())
	{
		static_cast<void>(QFile::remove(filePath));
		static_cast<void>(QFile::copy(m_exportFilename, filePath));
	}
	displayMessageOnAppWindow(filePath.isEmpty() ? APPWINDOW_MSG_EXPORT_FAILED : APPWINDOW_MSG_EXPORT_OK);
	QFile::remove(m_exportFilename);
	m_exportFilename.clear();
}

void QmlItemManager::importSlot_FileChosen(const QString& filePath)
{
	if (!filePath.isEmpty())
		openRequestedFile(filePath);
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_FAILED);
}

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
