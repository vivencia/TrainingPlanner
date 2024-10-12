#include "qmlitemmanager.h"
#include "tpappcontrol.h"
#include "tpsettings.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "osinterface.h"
#include "translationclass.h"

#include "qmluserinterface.h"
#include "qmlexercisesdatabaseinterface.h"
#include "qmlmesointerface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlmesosplitinterface.h"
#include "qmltdayinterface.h"
#include "qmlexerciseinterface.h"
#include "qmlexerciseentry.h"

#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QSettings>
#include <QFile>

static const QStringList& setTypePages(QStringList() << u"qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_qs <<
					u"qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_qs << u"qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_qs);

void QmlItemManager::configureQmlEngine(QQmlApplicationEngine* qml_engine)
{
	m_appQmlEngine = qml_engine;

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
	qmlRegisterType<QmlExerciseInterface>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExerciseManager");
	qmlRegisterType<QmlExerciseEntry>("org.vivenciasoftware.TrainingPlanner.qmlcomponents", 1, 0, "ExerciseEntryManager");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties(8);
	properties[0] = QQmlContext::PropertyPair{ u"appSettings"_qs, QVariant::fromValue(appSettings()) };
	properties[1] = QQmlContext::PropertyPair{ u"appControl"_qs, QVariant::fromValue(appControl()) };
	properties[2] = QQmlContext::PropertyPair{ u"osInterface"_qs, QVariant::fromValue(appOsInterface()) };
	properties[3] = QQmlContext::PropertyPair{ u"appUtils"_qs, QVariant::fromValue(appUtils()) };
	properties[4] = QQmlContext::PropertyPair{ u"appTr"_qs, QVariant::fromValue(appTr()) };
	properties[5] = QQmlContext::PropertyPair{ u"userModel"_qs, QVariant::fromValue(appUserModel()) };
	properties[6] = QQmlContext::PropertyPair{ u"mesocyclesModel"_qs, QVariant::fromValue(appMesoModel()) };
	properties[7] = QQmlContext::PropertyPair{ u"exercisesModel"_qs, QVariant::fromValue(appExercisesModel()) };
	m_appQmlEngine->rootContext()->setContextProperties(properties);

	const QUrl& url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(m_appQmlEngine, &QQmlApplicationEngine::objectCreated, m_appQmlEngine, [url] (QObject* obj, const QUrl& objUrl) {
		if (!obj && url == objUrl)
		{
			LOG_MESSAGE("*******************Mainwindow not loaded*******************")
			QCoreApplication::exit(-1);
		}
	});
	m_appQmlEngine->addImportPath(u":/"_qs);
	m_appQmlEngine->addImageProvider(u"tpimageprovider"_qs, new TPImageProvider{});
	m_appQmlEngine->load(url);

	m_appMainWindow = qobject_cast<QQuickWindow*>(m_appQmlEngine->rootObjects().at(0));
	connect(m_appMainWindow, SIGNAL(openFileChosen(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
	connect(m_appMainWindow, SIGNAL(openFileRejected(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
	m_appQmlEngine->rootContext()->setContextProperty(u"mainwindow"_qs, QVariant::fromValue(m_appMainWindow));

	if (!appSettings()->mainUserConfigured())
		QMetaObject::invokeMethod(m_appMainWindow, "showFirstUseTimeDialog");
	else
	{
		m_appMainWindow->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
		appOsInterface()->initialCheck();
	}

	connect(appMesoModel(), &DBMesocyclesModel::mostRecentOwnMesoChanged, this, [this] (const int meso_idx) {
		m_appMainWindow->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
	});
	connect(appUserModel(), &DBUserModel::mainUserConfigurationFinishedSignal, this, [this] () {
		appSettings()->setMainUserConfigured(true);
		appOsInterface()->initialCheck();
	});
	connect(appUserModel(), &DBUserModel::userModified, this, [this] (const uint user_row, const uint field) {
		if (user_row == 0 && field == USER_COL_APP_USE_MODE) {
			appMesoModel()->updateColumnLabels();
			m_appMainWindow->setProperty("bCanHaveTodaysWorkout", appMesoModel()->isDateWithinMeso(appMesoModel()->mostRecentOwnMesoIdx(), QDate::currentDate()));
		}
	});

	connect(m_appMainWindow, SIGNAL(saveFileChosen(const QString&)), this, SLOT(exportSlot(const QString&)));
	connect(m_appMainWindow, SIGNAL(saveFileRejected(const QString&)), this, SLOT(exportSlot(const QString&)));

	const QList<QObject*>& mainWindowChildren{m_appMainWindow->findChildren<QObject*>()};
	for (uint i(0); i < mainWindowChildren.count(); ++i)
	{
		if (mainWindowChildren.at(i)->objectName() == u"mainMenu"_qs)
		{
			QObject* mainMenu = mainWindowChildren.at(i);
			mainMenu->setProperty("itemManager", QVariant::fromValue(QmlManager()));
			break;
		}
	}
}

QmlUserInterface* QmlItemManager::usersManager()
{
	if (!m_appUsersManager)
		m_appUsersManager = new QmlUserInterface{this, m_appQmlEngine, m_appMainWindow};
	return m_appUsersManager;
}

QmlExercisesDatabaseInterface* QmlItemManager::exercisesListManager()
{
	if (!m_appExercisesListManager)
	{
		m_appExercisesListManager = new QmlExercisesDatabaseInterface{this, m_appQmlEngine, m_appMainWindow};
		connect(m_appExercisesListManager, &QmlExercisesDatabaseInterface::displayMessageOnAppWindow, this, &QmlItemManager::displayMessageOnAppWindow);
	}
	return m_appExercisesListManager;
}

QMLMesoInterface* QmlItemManager::mesocyclesManager(const uint meso_idx)
{
	if (meso_idx >= m_appMesosManager.count())
	{
		QMLMesoInterface* mesoInterface(nullptr);
		for(uint i(m_appMesosManager.count()); i <= meso_idx; ++i)
		{
			if (i == meso_idx)
			{
				mesoInterface = new QMLMesoInterface{this, m_appQmlEngine, m_appMainWindow, meso_idx};
				connect(mesoInterface, &QMLMesoInterface::displayMessageOnAppWindow, this, &QmlItemManager::displayMessageOnAppWindow);
				connect(mesoInterface, &QMLMesoInterface::addPageToMainMenu, this, &QmlItemManager::addMainMenuShortCut);
				connect(mesoInterface, &QMLMesoInterface::removePageFromMainMenu, this, &QmlItemManager::removeMainMenuShortCut);
			}
			m_appMesosManager.insert(i, mesoInterface);
		}
	}
	return m_appMesosManager.at(meso_idx);
}

QmlMesoCalendarInterface* QmlItemManager::calendarManager(const uint meso_idx)
{
	if (meso_idx >= m_appCalendarManager.count())
	{
		QmlMesoCalendarInterface* calendarInterface(nullptr);
		for(uint i(m_appCalendarManager.count()); i <= meso_idx; ++i)
		{
			if (i == meso_idx)
			{
				calendarInterface = new QmlMesoCalendarInterface{this, m_appQmlEngine, m_appMainWindow, meso_idx};
				connect(calendarInterface, &QmlMesoCalendarInterface::addPageToMainMenu, this, &QmlItemManager::addMainMenuShortCut);
				connect(calendarInterface, &QmlMesoCalendarInterface::removePageFromMainMenu, this, &QmlItemManager::removeMainMenuShortCut);
			}
			m_appCalendarManager.insert(i, calendarInterface);
		}
	}
	return m_appCalendarManager.at(meso_idx);
}

QmlMesoSplitInterface* QmlItemManager::splitManager(const uint meso_idx)
{
	if (meso_idx >= m_appSplitManager.count())
	{
		QmlMesoSplitInterface* splitInterface(nullptr);
		for(uint i(m_appSplitManager.count()); i <= meso_idx; ++i)
		{
			if (i == meso_idx)
			{
				splitInterface = new QmlMesoSplitInterface{this, m_appQmlEngine, m_appMainWindow, meso_idx};
				connect(splitInterface, &QmlMesoSplitInterface::displayMessageOnAppWindow, this, &QmlItemManager::displayMessageOnAppWindow);
				connect(splitInterface, &QmlMesoSplitInterface::addPageToMainMenu, this, &QmlItemManager::addMainMenuShortCut);
				connect(splitInterface, &QmlMesoSplitInterface::removePageFromMainMenu, this, &QmlItemManager::removeMainMenuShortCut);
			}
			m_appSplitManager.insert(i, splitInterface);
		}
	}
	return m_appSplitManager.at(meso_idx);
}

QmlTDayInterface* QmlItemManager::tDayManager(const uint meso_idx)
{
	if (meso_idx >= m_appTDayManager.count())
	{
		QmlTDayInterface* tDayInterface(nullptr);
		for(uint i(m_appTDayManager.count()); i <= meso_idx; ++i)
		{
			if (i == meso_idx)
			{
				tDayInterface = new QmlTDayInterface{this, m_appQmlEngine, m_appMainWindow, meso_idx};
				connect(tDayInterface, &QmlTDayInterface::displayMessageOnAppWindow, this, &QmlItemManager::displayMessageOnAppWindow);
				connect(tDayInterface, &QmlTDayInterface::addPageToMainMenu, this, &QmlItemManager::addMainMenuShortCut);
				connect(tDayInterface, &QmlTDayInterface::removePageFromMainMenu, this, &QmlItemManager::removeMainMenuShortCut);
			}
			m_appTDayManager.insert(i, tDayInterface);
		}
	}
	return m_appTDayManager.at(meso_idx);
}

//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
void QmlItemManager::openMainMenuShortCut(const int button_id)
{
	QMetaObject::invokeMethod(m_appMainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
}

void QmlItemManager::tryToImport(const QList<bool>& selectedFields)
{
	uint wanted_content(0);
	wanted_content |= (m_fileContents & IFC_MESO) && selectedFields.at(0) ? IFC_MESO : 0;
	wanted_content |= (m_fileContents & IFC_MESO) && selectedFields.at(1) ? IFC_USER : 0;
	wanted_content |= (m_fileContents & IFC_MESOSPLIT) && (m_fileContents & IFC_MESO ? selectedFields.at(2) : selectedFields.at(0)) ? IFC_MESOSPLIT : 0;
	wanted_content |= (m_fileContents & IFC_TDAY) && selectedFields.at(0) ? IFC_TDAY : 0;
	wanted_content |= (m_fileContents & IFC_EXERCISES) && selectedFields.at(0) ? IFC_EXERCISES : 0;
	appControl()->importFromFile(m_importFilename, wanted_content);
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
	QMetaObject::invokeMethod(m_appMainWindow, "createImportConfirmDialog", Q_ARG(QmlItemManager*, this),
				Q_ARG(QVariant, importOptions), Q_ARG(QVariant, QVariant::fromValue(selectedFields)));
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
	QMetaObject::invokeMethod(m_appMainWindow, "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message));
}

void QmlItemManager::requestTimerDialog(QQuickItem* requester, const QVariant& args)
{
	const QVariantList& strargs(args.toList());
	QMetaObject::invokeMethod(m_currenttDayPage, "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
}

void QmlItemManager::requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id)
{
	if (appExercisesModel()->count() == 0)
		appDBInterface()->getAllExercises();
	QMetaObject::invokeMethod(id == 0 ? m_plannerPage : m_currenttDayPage, "requestSimpleExercisesList",
					Q_ARG(QVariant, QVariant::fromValue(requester)), Q_ARG(QVariant, visible), Q_ARG(QVariant, multipleSelection));
}

void QmlItemManager::requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset)
{
	QMetaObject::invokeMethod(m_currenttDayPage, "requestFloatingButton", Q_ARG(int, exercise_idx.toInt()),
								Q_ARG(int, set_type.toInt()), Q_ARG(QString, nset.toString()));
}

void QmlItemManager::showRemoveExerciseMessage(int exercise_idx)
{
	QMetaObject::invokeMethod(m_currenttDayPage, "showRemoveExerciseMessage", Q_ARG(int, exercise_idx));
}

void QmlItemManager::showRemoveSetMessage(int set_number, int exercise_idx)
{
	QMetaObject::invokeMethod(m_currenttDayPage, "showRemoveSetMessage", Q_ARG(int, set_number), Q_ARG(int, exercise_idx));
}

void QmlItemManager::exerciseCompleted(int exercise_idx)
{
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	if (exercise_idx < exercisesCount()-1)
	{
		if (!exerciseEntryItem(exercise_idx+1)->property("finishButtonEnabled").toBool())
		{
			QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx+1), "paneExerciseShowHide", Q_ARG(bool, true), Q_ARG(bool, true));
			QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, exerciseEntryItem(exercise_idx+1)->property("y").toInt() + 50));
		}
	}
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
		appControl()->openRequestedFile(filePath);
	else
		displayMessageOnAppWindow(APPWINDOW_MSG_IMPORT_FAILED);
}

//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
void QmlItemManager::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	if (m_mainMenuShortcutPages.contains(page))
		QMetaObject::invokeMethod(m_appMainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
	else
	{
		if (m_mainMenuShortcutPages.count() < 5)
		{
			QMetaObject::invokeMethod(m_appMainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
			QMetaObject::invokeMethod(m_appMainWindow, "createShortCut", Q_ARG(QString, label),
													Q_ARG(QQuickItem*, page), Q_ARG(int, m_mainMenuShortcutPages.count()));
			m_mainMenuShortcutPages.append(page);
		}
		else
		{
			QMetaObject::invokeMethod(m_appMainWindow, "pushOntoStack", Q_ARG(QQuickItem*, page));
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
		QMetaObject::invokeMethod(m_appMainWindow, "popFromStack", Q_ARG(QQuickItem*, page));
		m_mainMenuShortcutPages.remove(idx);
		delete m_mainMenuShortcutEntries.at(idx);
		m_mainMenuShortcutEntries.remove(idx);
		for (uint i(idx); i < m_mainMenuShortcutEntries.count(); ++i)
			m_mainMenuShortcutEntries.at(i)->setProperty("clickid", i);
	}
}
//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
