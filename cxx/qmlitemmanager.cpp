#include "qmlitemmanager.h"
#include "tpappcontrol.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "tpimage.h"
#include "tpimageprovider.h"
#include "tputils.h"
#include "osinterface.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QSettings>
#include <QFile>

QmlItemManager* QmlItemManager::app_root_items_manager(nullptr);

QQmlApplicationEngine* QmlItemManager::app_qml_engine(nullptr);
QQuickWindow* QmlItemManager::app_MainWindow(nullptr);
QQuickItem* QmlItemManager::app_StackView(nullptr);

QQmlComponent* QmlItemManager::exercisesComponent(nullptr);
QQuickItem* QmlItemManager::exercisesPage(nullptr);
QVariantMap QmlItemManager::exercisesProperties;

QQmlComponent* QmlItemManager::settingsComponent(nullptr);
QQuickItem* QmlItemManager::settingsPage(nullptr);
QVariantMap QmlItemManager::settingsProperties;
QQmlComponent* QmlItemManager::clientsOrCoachesComponent(nullptr);
QQuickItem* QmlItemManager::clientsOrCoachesPage(nullptr);
QVariantMap QmlItemManager::clientsOrCoachesProperties;
QQuickItem* QmlItemManager::userPage(nullptr);

inline QQuickItem* appStackView() { return QmlItemManager::app_StackView; }

static const QStringList setTypePages(QStringList() << u"qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_qs <<
					u"qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_qs << u"qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_qs);

QmlItemManager::~QmlItemManager()
{
	if (this == app_root_items_manager)
	{
		if (exercisesComponent)
		{
			delete exercisesPage;
			delete exercisesComponent;
			exercisesComponent = nullptr;
		}
		if (settingsComponent)
		{
			delete settingsPage;
			delete settingsComponent;
			settingsComponent = nullptr;
		}
		if (clientsOrCoachesComponent)
		{
			delete clientsOrCoachesPage;
			delete clientsOrCoachesComponent;
			clientsOrCoachesComponent = nullptr;
		}
	}

	if (m_mesoComponent)
	{
		removeMainMenuShortCut(m_mesoPage);
		delete m_mesoPage;
		delete m_mesoComponent;
	}
	if (m_calComponent)
	{
		removeMainMenuShortCut(m_calPage);
		delete m_calPage;
		delete m_calComponent;
	}

	if (m_splitComponent)
	{
		delete m_plannerPage;
		delete m_plannerComponent;

		QMapIterator<QChar,QQuickItem*> t(m_splitPages);
		t.toFront();
		while (t.hasNext()) {
			t.next();
			removeMainMenuShortCut(t.value());
			delete t.value();
		}
		delete m_splitComponent;

		QMapIterator<QChar,DBMesoSplitModel*> z(m_splitModels);
		z.toFront();
		while (z.hasNext()) {
			z.next();
			delete z.value();
		}
	}

	if (m_tDayComponent)
	{
		QMapIterator<QDate,QQuickItem*> i(m_tDayPages);
		i.toFront();
		while (i.hasNext()) {
			i.next();
			removeMainMenuShortCut(i.value());
			delete i.value();
		}
		delete m_tDayComponent;

		QMapIterator<QDate,tDayExercises*> y(m_tDayExercisesList);
		y.toFront();
		while (y.hasNext()) {
			y.next();
			delete y.value();
		}
		if (m_setComponents[0])
			delete m_setComponents[0];
		if (m_setComponents[1])
			delete m_setComponents[1];
		if (m_setComponents[2])
			delete m_setComponents[2];
		delete m_tDayExercisesComponent;

		QMapIterator<QDate,DBTrainingDayModel*> x(m_tDayModels);
		x.toFront();
		while (x.hasNext()) {
			x.next();
			delete x.value();
		}
	}
}

void QmlItemManager::configureQmlEngine(QQmlApplicationEngine* qml_engine)
{
	if (appQmlEngine())
		return;
	app_qml_engine = qml_engine;

	/*QString style(appSettings()->value("themeStyle").toString());
	if (style.isEmpty())
		style = u"Material"_qs;
	QQuickStyle::setStyle(style);*/

	qmlRegisterType<DBUserModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBUserModel");
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBTrainingDayModel");
	qmlRegisterType<QmlItemManager>("com.vivenciasoftware.qmlcomponents", 1, 0, "QmlItemManager");
	qmlRegisterType<TPTimer>("com.vivenciasoftware.qmlcomponents", 1, 0, "TPTimer");
	qmlRegisterType<TPImage>("com.vivenciasoftware.qmlcomponents", 1, 0, "TPImage");

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties(12);
	properties[0] = QQmlContext::PropertyPair{ u"appControl"_qs, QVariant::fromValue(appControl()) };
	properties[1] = QQmlContext::PropertyPair{ u"osInterface"_qs, QVariant::fromValue(appOsInterface()) };
	properties[2] = QQmlContext::PropertyPair{ u"appDB"_qs, QVariant::fromValue(appDBInterface()) };
	properties[3] = QQmlContext::PropertyPair{ u"appUtils"_qs, QVariant::fromValue(appUtils()) };
	properties[4] = QQmlContext::PropertyPair{ u"appTr"_qs, QVariant::fromValue(appTr()) };
	properties[5] = QQmlContext::PropertyPair{ u"userModel"_qs, QVariant::fromValue(appUserModel()) };
	properties[6] = QQmlContext::PropertyPair{ u"mesocyclesModel"_qs, QVariant::fromValue(appMesoModel()) };
	properties[7] = QQmlContext::PropertyPair{ u"exercisesModel"_qs, QVariant::fromValue(appExercisesModel()) };
	properties[8] = QQmlContext::PropertyPair{ u"lightIconFolder"_qs, u"white/"_qs };
	properties[9] = QQmlContext::PropertyPair{ u"darkIconFolder"_qs, u"black/"_qs };
	properties[10] = QQmlContext::PropertyPair{ u"listEntryColor1"_qs, QVariant(QColor(220, 227, 240)) };
	properties[11] = QQmlContext::PropertyPair{ u"listEntryColor2"_qs, QVariant(QColor(195, 202, 213)) };
	appQmlEngine()->rootContext()->setContextProperties(properties);

	const QUrl& url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (QObject* obj, const QUrl& objUrl) {
		if (!obj && url == objUrl)
		{
			MSG_OUT("*******************Mainwindow not loaded*******************")
			QCoreApplication::exit(-1);
		}
	});
	appQmlEngine()->addImportPath(u":/"_qs);
	appQmlEngine()->addImageProvider(u"tpimageprovider"_qs, new TPImageProvider{});
	appQmlEngine()->load(url);

	app_MainWindow = qobject_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
	connect(appMainWindow(), SIGNAL(mainWindowStarted()), rootItemsManager(), SLOT(mainWindowStarted()), static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	connect(appMainWindow(), SIGNAL(openFileChosen(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
	connect(appMainWindow(), SIGNAL(openFileRejected(const QString&)), this, SLOT(importSlot_FileChosen(const QString&)), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));

	app_StackView = appMainWindow()->findChild<QQuickItem*>(u"appStackView"_qs);
	const QQuickItem* const contentItem(app_StackView->parentItem());
	properties.clear();
	properties.append(QQmlContext::PropertyPair{ u"mainwindow"_qs, QVariant::fromValue(appMainWindow()) });
	properties.append(QQmlContext::PropertyPair{ u"windowHeight"_qs, contentItem->height() }); //mainwindow.height - header.height
	properties.append(QQmlContext::PropertyPair{ u"windowWidth"_qs, contentItem->width() });
	appQmlEngine()->rootContext()->setContextProperties(properties);

	QMetaObject::invokeMethod(appMainWindow(), "init");

	const QList<QObject*>& mainWindowChildren{appMainWindow()->findChildren<QObject*>()};
	for (uint i(0); i < mainWindowChildren.count(); ++i)
	{
		if (mainWindowChildren.at(i)->objectName() == u"mainMenu"_qs)
		{
			QObject* mainMenu = mainWindowChildren.at(i);
			mainMenu->setProperty("itemManager", QVariant::fromValue(rootItemsManager()));
			break;
		}
	}
}

//-----------------------------------------------------------USER-----------------------------------------------------------
void QmlItemManager::getSettingsPage(const uint startPageIndex)
{
	if (settingsComponent)
	{
		settingsPage->setProperty("startPageIndex", startPageIndex);
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, settingsPage));
	}
	else
	{
		settingsProperties.insert(QStringLiteral("startPageIndex"), startPageIndex);
		settingsComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ConfigurationPage.qml"_qs), QQmlComponent::Asynchronous);
		if (settingsComponent->status() != QQmlComponent::Ready)
		{
			connect(settingsComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
					return createSettingsPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createSettingsPage();
	}
}

void QmlItemManager::getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	setClientsOrCoachesPagesProperties(bManageClients, bManageCoaches);

	if (clientsOrCoachesComponent)
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, clientsOrCoachesPage));
	else
	{
		clientsOrCoachesProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
		clientsOrCoachesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ClientsOrCoachesPage.qml"_qs), QQmlComponent::Asynchronous);
		if (clientsOrCoachesComponent->status() != QQmlComponent::Ready)
		{
			connect(clientsOrCoachesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
					return createClientsOrCoachesPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		}
		else
			createClientsOrCoachesPage();
	}
}

void QmlItemManager::removeUser(const uint user_row, const bool bCoach)
{
	appDBInterface()->removeUser(user_row, bCoach);
	const int curUserRow(appUserModel()->removeUser(user_row, bCoach));
	int firstUserRow(-1), lastUserRow(-1);
	if (curUserRow > 0)
	{
		firstUserRow = appUserModel()->findFirstUser(bCoach);
		lastUserRow = appUserModel()->findLastUser(bCoach);
	}
	clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
	clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
	clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
}
//-----------------------------------------------------------USER-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES-----------------------------------------------------------
const uint QmlItemManager::removeExercise(const uint row)
{
	appDBInterface()->removeExercise(appExercisesModel()->actualIndex(row));
	appExercisesModel()->removeExercise(row);
	return row > 0 ? row - 1 : 0;
}

void QmlItemManager::exportExercises(const bool bShare)
{
	int exportFileMessageId(0);
	if (appExercisesModel()->collectExportData())
	{
		const QString& suggestedName(tr("TrainingPlanner Exercises List.txt"));
		m_exportFilename = appOsInterface()->appDataFilesPath() + suggestedName;
		exportFileMessageId = appExercisesModel()->exportToFile(m_exportFilename);
		if (exportFileMessageId >= 0)
		{
			if (bShare)
			{
				appOsInterface()->shareFile(m_exportFilename);
				exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
			}
			else
				QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
		}
	}
	else
		exportFileMessageId = APPWINDOW_MSG_NOTHING_TO_EXPORT;
	displayMessageOnAppWindow(exportFileMessageId);
}

void QmlItemManager::importExercises(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_EXERCISES);
}

void QmlItemManager::getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (!exercisesComponent)
	{
		if (appExercisesModel()->count() == 0)
			appDBInterface()->getAllExercises();
		createExercisesPage(bChooseButtonEnabled, connectPage);
	}
	else
	{
		exercisesPage->setProperty("bChooseButtonEnabled", bChooseButtonEnabled);
		appExercisesModel()->clearSelectedEntries();
		if (connectPage)
		{
			disconnect(exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
			connect(exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
		}
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, exercisesPage));
	}
}
//-----------------------------------------------------------EXERCISES-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
void QmlItemManager::changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday)
{
	appDBInterface()->changeMesoCalendar(m_mesoIdx, preserve_old_cal, preserve_untilyesterday);
}

void QmlItemManager::getMesocyclePage()
{
	appMesoModel()->setCurrentMesoIdx(m_mesoIdx);
	if (!m_mesoComponent)
		createMesocyclePage();
	else
		addMainMenuShortCut(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_mesoPage);
}

void QmlItemManager::exportMeso(const bool bShare, const bool bCoachInfo)
{
	int exportFileMessageId(0);
	const QString& suggestedName(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME) + tr(" - TP Complete Meso.txt"));
	m_exportFilename = appOsInterface()->appDataFilesPath() + suggestedName;
	if (bCoachInfo)
	{
		appUserModel()->setExportRow(appUserModel()->getRowByCoachName(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_COACH)));
		appUserModel()->exportToFile(m_exportFilename);
	}
	appMesoModel()->setExportRow(m_mesoIdx);
	exportFileMessageId = appMesoModel()->exportToFile(m_exportFilename);
	if (!appDBInterface()->splitsLoaded())
		appDBInterface()->loadCompleteMesoSplits(m_mesoIdx, m_splitModels, false);
	exportMesoSplit(bShare, u"X"_qs, m_exportFilename, true);
	if (bShare)
	{
		appOsInterface()->shareFile(m_exportFilename);
		exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
	}
	else
		QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));

	displayMessageOnAppWindow(exportFileMessageId);
}

void QmlItemManager::importMeso(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_MESO);
}
//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
void QmlItemManager::getMesoCalendarPage()
{
	if (!m_calComponent)
	{
		appDBInterface()->getMesoCalendar(m_mesoIdx);
		createMesoCalendarPage();
	}
	else
		addMainMenuShortCut(tr("Calendar: ") + appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_calPage);
}
//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
void QmlItemManager::getExercisesPlannerPage()
{
	if (!m_plannerComponent)
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [&] (const uint) {
			createPlannerPage();
		});
		appDBInterface()->loadCompleteMesoSplits(m_mesoIdx, allSplitModels());
	}
	else
		addMainMenuShortCut(tr("Exercises Planner: ") + appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_plannerPage);
}

void QmlItemManager::getMesoSplitPage(const uint page_index)
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous);

	if (m_splitComponent->status() == QQmlComponent::Ready)
		createMesoSplitPage(page_index);
	else
	{
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [&,page_index] (QQmlComponent::Status) {
			return createMesoSplitPage(page_index);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void QmlItemManager::swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2)
{
	m_splitPages.value(splitLetter1.at(0))->setProperty("splitLetter", splitLetter2);
	m_splitPages.value(splitLetter2.at(0))->setProperty("splitLetter", splitLetter1);
	DBMesoSplitModel* tempSplit(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter1.at(0)] = m_splitModels.value(splitLetter2.at(0));
	appDBInterface()->saveMesoSplitComplete(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter2.at(0)] = tempSplit;
	appDBInterface()->saveMesoSplitComplete(m_splitModels.value(splitLetter1.at(0)));
}

void QmlItemManager::exportMesoSplit(const bool bShare, const QString& splitLetter, const QString& filePath, const bool bJustExport)
{
	int exportFileMessageId(0);
	QString mesoSplit, suggestedName;
	if (filePath.isEmpty())
	{
		if (splitLetter == u"X"_qs)
		{
			mesoSplit = appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_SPLIT);
			suggestedName = appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME) + tr(" - Exercises Plan.txt");
		}
		else
		{
			mesoSplit = splitLetter;
			suggestedName = appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME) + tr(" - Exercises Plan - Split ") + splitLetter + u".txt"_qs;
		}
		m_exportFilename = appOsInterface()->appDataFilesPath() + suggestedName;
	}
	else
	{
		m_exportFilename = filePath;
		mesoSplit = appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_SPLIT);
	}

	QString mesoLetters;
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator& itr_end(mesoSplit.constEnd());
	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;
		mesoLetters.append(static_cast<QChar>(*itr));
		m_splitModels.value(*itr)->exportToFile(m_exportFilename);
	} while (++itr != itr_end);
	if (bJustExport)
		return;
	if (bShare)
	{
		appOsInterface()->shareFile(m_exportFilename);
		exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
	}
	else
		QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));

	displayMessageOnAppWindow(exportFileMessageId);
}

void QmlItemManager::importMesoSplit(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_MESOSPLIT);
}

DBMesoSplitModel* QmlItemManager::getSplitModel(const QChar& splitLetter)
{
	if (!m_splitModels.contains(splitLetter))
	{
		DBMesoSplitModel* splitModel{new DBMesoSplitModel(this, true, m_mesoIdx)};
		connect(this, &QmlItemManager::mesoIdxChanged, splitModel, [&,splitModel] { splitModel->setMesoIdx(m_mesoIdx); });
		m_splitModels.insert(splitLetter, splitModel);
	}
	return m_splitModels.value(splitLetter);
}
//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
void QmlItemManager::getTrainingDayPage(const QDate& date)
{
	const QQuickItem* const tDayPage{m_tDayPages.value(date)};
	if (!tDayPage)
	{
		if (!appMesoModel()->mesoCalendarModel(m_mesoIdx)->isReady())
		{
			connect(appDBInterface(), &DBInterface::databaseReady, this, [&,date] (const uint db_id) {
				getTrainingDayPage(date);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appDBInterface()->getMesoCalendar(m_mesoIdx);
			return;
		}
		const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
		const QString& tday(QString::number(mesoCal->getTrainingDay(date.month(), date.day()-1)));
		const QString& splitLetter(mesoCal->getSplitLetter(date.month(), date.day()-1));

		m_currenttDayPage = nullptr;
		m_CurrenttDayModel->appendRow();
		m_CurrenttDayModel->setMesoId(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_ID));
		m_CurrenttDayModel->setDate(date);
		m_CurrenttDayModel->setSplitLetter(splitLetter);
		m_CurrenttDayModel->setTrainingDay(tday);
		m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
		m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
		m_CurrenttDayModel->setModified(false);
		m_tDayProperties.insert(u"mainDate"_qs, date);
		m_tDayProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
		m_tDayProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
		m_tDayProperties.insert(u"tDay"_qs, tday);
		m_tDayProperties.insert(u"splitLetter"_qs, splitLetter);

		connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [&] (const QVariant& data) {
				if (m_CurrenttDayModel->timeOut() != u"--:--"_qs)
					m_CurrenttDayModel->setDayIsFinished(true);
				setTrainingDayPageEmptyDayOptions(data.value<DBTrainingDayModel*>());
				if (m_currenttDayPage)
				{
					m_currenttDayPage->setProperty("timeIn", m_CurrenttDayModel->timeIn());
					m_currenttDayPage->setProperty("timeOut", m_CurrenttDayModel->timeOut());
				}
				else
				{
					m_tDayProperties.insert(u"timeIn"_qs, m_CurrenttDayModel->timeIn());
					m_tDayProperties.insert(u"timeOut"_qs, m_CurrenttDayModel->timeOut());
				}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appDBInterface()->getTrainingDay(m_CurrenttDayModel);
		createTrainingDayPage(date);
	}
	else
		addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(date), m_currenttDayPage);
}

void QmlItemManager::loadExercisesFromDate(const QString& strDate)
{
	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect(appDBInterface(), &DBInterface::databaseReady, this, [&] (const uint) {
		const bool btoday(m_CurrenttDayModel->date() == QDate::currentDate());
		m_CurrenttDayModel->setModified(btoday);
		m_CurrenttDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appDBInterface()->loadExercisesFromDate(strDate, m_CurrenttDayModel);
}

void QmlItemManager::loadExercisesFromMesoPlan()
{
	connect(appDBInterface(), &DBInterface::databaseReady, this, [&] (const uint) {
		const bool btoday(m_CurrenttDayModel->date() == QDate::currentDate());
		m_CurrenttDayModel->setModified(btoday);
		m_CurrenttDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appDBInterface()->loadExercisesFromMesoPlan(m_CurrenttDayModel, allSplitModels());
}

void QmlItemManager::convertTDayToPlan()
{
	appDBInterface()->convertTDayToPlan(m_CurrenttDayModel, allSplitModels());
}

void QmlItemManager::resetWorkout()
{
	m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
	m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
	m_CurrenttDayModel->setDayIsFinished(false);
	m_currenttDayPage->setProperty("timeIn", m_CurrenttDayModel->timeIn());
	m_currenttDayPage->setProperty("timeOut", m_CurrenttDayModel->timeOut());
	m_currenttDayPage->setProperty("editMode", false);
	QMetaObject::invokeMethod(m_currenttDayPage, "resetTimer", Qt::AutoConnection);
}

void QmlItemManager::setCurrenttDay(const QDate& date)
{
	m_CurrenttDayModel = m_tDayModels.value(date);
	m_currenttDayPage = m_tDayPages.value(date);
	m_currentExercises = m_tDayExercisesList.value(date);
}

void QmlItemManager::exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel)
{
	int exportFileMessageId(0);
	const QString& suggestedName(tr(" - Workout ") + tDayModel->splitLetter() + u".txt"_qs);
	m_exportFilename = appOsInterface()->appDataFilesPath() + suggestedName;
	exportFileMessageId = tDayModel->exportToFile(m_exportFilename);
	if (exportFileMessageId >= 0)
	{
		if (bShare)
		{
			appOsInterface()->shareFile(m_exportFilename);
			exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
		}
		else
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	displayMessageOnAppWindow(exportFileMessageId);
}

void QmlItemManager::importTrainingDay(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_TDAY);
}

DBTrainingDayModel* QmlItemManager::gettDayModel(const QDate& date)
{
	if (!m_tDayModels.contains(date))
	{
		m_CurrenttDayModel = new DBTrainingDayModel(this, m_mesoIdx);
		connect(this, &QmlItemManager::mesoIdxChanged, m_CurrenttDayModel, [&] { m_CurrenttDayModel->setMesoIdx(m_mesoIdx); });
		m_tDayModels.insert(date, m_CurrenttDayModel);
	}
	else
		m_CurrenttDayModel = m_tDayModels.value(date);
	return m_CurrenttDayModel;
}
//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
void QmlItemManager::createExerciseObject()
{
	if (m_tDayExercisesComponent == nullptr)
		m_tDayExercisesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);

	QString exerciseName, nSets, nReps, nWeight;
	if (appExercisesModel()->selectedEntriesCount() == 1)
	{
		exerciseName = appExercisesModel()->selectedEntriesValue_fast(0, 1) + u" - "_qs + appExercisesModel()->selectedEntriesValue_fast(0, 2);
		nSets = appExercisesModel()->selectedEntriesValue_fast(0, 4);
		nReps = appExercisesModel()->selectedEntriesValue_fast(0, 5);
		nWeight = appExercisesModel()->selectedEntriesValue_fast(0, 6);
	}
	else
	{
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, 1) + u" - "_qs + appExercisesModel()->selectedEntriesValue_fast(0, 2), exerciseName);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, 1) + u" - "_qs + appExercisesModel()->selectedEntriesValue_fast(1, 2), exerciseName);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, 4), nSets);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, 4), nSets);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, 5), nReps);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, 5), nReps);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, 6), nWeight);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, 6), nWeight);
	}
	m_tDayExerciseEntryProperties.insert(u"nSets"_qs, nSets);
	m_tDayExerciseEntryProperties.insert(u"nReps"_qs, nReps);
	m_tDayExerciseEntryProperties.insert(u"nWeight"_qs, nWeight);

	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exerciseCount());

	if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createExerciseObject_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createExerciseObject_part2();
}

void QmlItemManager::removeExerciseObject(const uint exercise_idx)
{
	if (exercise_idx < m_currentExercises->exerciseObjects.count())
	{
		m_CurrenttDayModel->removeExercise(exercise_idx);
		m_currentExercises->removeExerciseEntry(exercise_idx);
		for(uint i(exercise_idx); i < m_currentExercises->exerciseObjects.count(); ++i)
		{
			//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
			QMetaObject::invokeMethod(m_currentExercises->exerciseEntry(i), "moveExercise", Q_ARG(bool, true), Q_ARG(bool, false));
			for(uint x(0); x < m_currentExercises->setCount(i); ++x)
				m_currentExercises->setObject(i, x)->setProperty("exerciseIdx", i);
		}
	}
}

void QmlItemManager::clearExercises()
{
	appDBInterface()->verifyTDayOptions(currenttDayModel());
	m_currentExercises->clear();
	m_CurrenttDayModel->clearExercises();
	m_CurrenttDayModel->setDayIsFinished(false);
}

void QmlItemManager::moveExercise(const uint exercise_idx, const uint new_idx)
{
	uint nsets(m_currentExercises->setCount(exercise_idx));
	for(uint i(0); i < nsets; ++i)
	{
		m_currentExercises->setObject(exercise_idx, i)->setProperty("exerciseIdx", new_idx);
		m_currentExercises->setObject(exercise_idx, i)->setProperty("ownerExercise", QVariant::fromValue(m_currentExercises->exerciseEntry(new_idx)));
	}
	nsets = m_currentExercises->setCount(new_idx);
	for(uint i(0); i < nsets; ++i)
	{
		m_currentExercises->setObject(new_idx, i)->setProperty("exerciseIdx", exercise_idx);
		m_currentExercises->setObject(new_idx, i)->setProperty("ownerExercise", QVariant::fromValue(m_currentExercises->exerciseEntry(exercise_idx)));
	}
	m_CurrenttDayModel->moveExercise(exercise_idx, new_idx);

	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		m_currentExercises->exerciseEntry(x)->setParentItem(nullptr);

	QQuickItem* parentLayout(m_currenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	m_currentExercises->exerciseObjects.swapItemsAt(exercise_idx, new_idx);
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		m_currentExercises->exerciseEntry(x)->setParentItem(parentLayout);
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(m_currentExercises->exerciseEntry(exercise_idx), "moveExercise", Q_ARG(bool, new_idx > exercise_idx), Q_ARG(bool, false));
}

void QmlItemManager::rollUpExercises() const
{
	for (uint i(0); i < m_currentExercises->exercisesCount(); ++i)
		QMetaObject::invokeMethod(m_currentExercises->exerciseEntry_const(i), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, -100));
}

void QmlItemManager::manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type)
{
	if (!bTrackRestTime)
		bAutoRestTime = false;
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nRestTime", bAutoRestTime ?
											u"00:00"_qs :
											m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, new_set_type, 0));

	enableDisableSetsRestTime(exercise_idx, bTrackRestTime, bAutoRestTime);
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
void QmlItemManager::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet,
										const QString& nReps, const QString& nWeight, const QString& nRestTime)
{
	const uint set_type_cpp(set_type == SET_TYPE_DROP ? 1 : set_type == SET_TYPE_GIANT ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
		m_setComponents[set_type_cpp] = new QQmlComponent(appQmlEngine(), QUrl(setTypePages[set_type_cpp]), QQmlComponent::Asynchronous);

	if (bNewSet)
	{
		if (set_number == 0)
			currenttDayModel()->newFirstSet(exercise_idx, set_type, nReps, nWeight, nRestTime);
		else
			currenttDayModel()->newSet(set_number, exercise_idx, set_type, nReps, nWeight, nRestTime);
		m_expectedSetNumber = set_number;
	}

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [&,set_type,set_number,exercise_idx,bNewSet](QQmlComponent::Status status)
			{ if (status == QQmlComponent::Ready) return createSetObject_part2(set_type, set_number, exercise_idx, bNewSet); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(set_type, set_number, exercise_idx, bNewSet);
}

void QmlItemManager::createSetObjects(const uint exercise_idx)
{
	if (!setsLoaded(exercise_idx))
	{
		m_expectedSetNumber = 0;
		const uint nsets(currenttDayModel()->setsNumber(exercise_idx));
		for (uint i(0); i < nsets; ++i)
			createSetObject(currenttDayModel()->setType(i, exercise_idx), i, exercise_idx, false);
	}
	else
	{
		//Place into view: exercise entry + first set
		QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView",
			Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx)->property("y").toInt() + 50));
	}
}

//Convenience function for ExerciseEntry::createSetObject. Create last_set - first_set sets on the fly as per user command
void QmlItemManager::createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
							const QString& nReps, const QString& nWeight, const QString& nRestTime)
{
	if (!nReps.isEmpty())
	{
		connect(this, &QmlItemManager::setObjectReady, this, [&,set_type,first_set, last_set,exercise_idx] {
			createSetObjects(exercise_idx, first_set, last_set, set_type);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		createSetObject(set_type, first_set, exercise_idx, true, nReps, nWeight, nRestTime);
		m_expectedSetNumber = first_set;
		return;
	}
	if ((last_set - first_set) > 1)
	{
		for (uint i(first_set+1); i < last_set; ++i)
		{
			currenttDayModel()->newSet(i, exercise_idx, set_type);
			createSetObject_part2(set_type, i, exercise_idx, true);
		}
		//Place into view: exercise entry + first set
		QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView",
			Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx)->property("y").toInt() + 50));
	}
	else
	{
		//Place into view: most recent set added
		QMetaObject::invokeMethod( m_currenttDayPage, "placeSetIntoView",
			Q_ARG( int, first_set > 0 ? (m_currentExercises->setObject(exercise_idx, first_set-1)->property("y").toInt() + 50) :
										(m_currentExercises->exerciseEntry(exercise_idx)->property("height").toInt()) ) );
	}
}

void QmlItemManager::removeSetObject(const uint set_number, const uint exercise_idx)
{
	if (exercise_idx < m_currentExercises->exercisesCount())
	{
		for(uint x(set_number+1); x < m_currentExercises->setCount(exercise_idx); ++x)
			m_currentExercises->setObject(exercise_idx, x)->setProperty("setNumber", x-1);
		m_CurrenttDayModel->removeSet(set_number, exercise_idx);
		m_currentExercises->removeSet(exercise_idx, set_number);
		const uint nsets(m_currentExercises->setCount(exercise_idx));
		m_currentExercises->exerciseEntry(exercise_idx)->setProperty("setNbr", nsets);
		if (nsets == 0)
		{
			m_currentExercises->exerciseEntry(exercise_idx)->setProperty("bCanEditRestTimeTracking", true);
			return;
		}
		else if (set_number == nsets) //last set was removed, update suggested values for a possible set addition
		{
			m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nReps", m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
			m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nWeight", m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
			if (set_number > 1)
			{
				m_currentExercises->setObject(exercise_idx, set_number-1)->setProperty("finishButtonVisible", true);
				enableDisableExerciseCompletedButton(exercise_idx, currenttDayModel()->setCompleted(set_number-1, exercise_idx));
			}
		}
		findCurrentSet(exercise_idx, set_number-1);
	}
}

void QmlItemManager::changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text, const bool bChangeModel)
{
	if (bChangeModel)
	{
		if (label_idx == 1)
			m_CurrenttDayModel->setExerciseName1(new_text, exercise_idx);
		else
			m_CurrenttDayModel->setExerciseName2(new_text, exercise_idx);
	}

	QQuickItem* setObj(nullptr);
	QQuickItem* txtExercise(nullptr);
	for (uint i(0); i < m_currentExercises->setCount(exercise_idx); ++i)
	{
		if (m_CurrenttDayModel->setType(i, exercise_idx) == SET_TYPE_GIANT)
		{
			setObj = m_currentExercises->setObject_const(exercise_idx, i);
			QMetaObject::invokeMethod(setObj, "liberateSignals", Q_ARG(bool, false));
			txtExercise = setObj->findChild<QQuickItem*>(label_idx == 1 ? u"txtExercise1"_qs : u"txtExercise2"_qs);
			QMetaObject::invokeMethod(setObj, "changeExerciseText", Q_ARG(QVariant, QVariant::fromValue(txtExercise)),
				Q_ARG(QString, label_idx == 1 ? m_CurrenttDayModel->exerciseName1(exercise_idx) : m_CurrenttDayModel->exerciseName2(exercise_idx)));
			QMetaObject::invokeMethod(setObj, "liberateSignals", Q_ARG(bool, true));
		}
	}
}

void QmlItemManager::changeSetType(const uint set_number, const uint exercise_idx, const uint new_type)
{
	if (new_type != 100)
	{
		const uint current_type(m_CurrenttDayModel->setType(set_number, exercise_idx));
		m_CurrenttDayModel->changeSetType(set_number, exercise_idx, current_type, new_type);
		if (current_type != SET_TYPE_DROP && current_type != SET_TYPE_GIANT)
		{
			if (new_type != SET_TYPE_DROP && new_type != SET_TYPE_GIANT)
			{
				m_currentExercises->setObject(exercise_idx, set_number)->setProperty("setType", new_type);
				return;
			}
		}

		m_setObjectProperties.insert(u"copyTypeButtonValue"_qs, m_currentExercises->setObject(exercise_idx, set_number)->property("copyTypeButtonValue").toString());
		m_setObjectProperties.insert(u"copyTimeButtonValue"_qs, m_currentExercises->setObject(exercise_idx, set_number)->property("copyTimeButtonValue").toString());
		m_setObjectProperties.insert(u"copyRepsButtonValue"_qs, m_currentExercises->setObject(exercise_idx, set_number)->property("copyRepsButtonValue").toString());
		m_setObjectProperties.insert(u"copyWeightButtonValue"_qs, m_currentExercises->setObject(exercise_idx, set_number)->property("copyWeightButtonValue").toString());
		m_currentExercises->removeSet(exercise_idx, set_number);

		m_expectedSetNumber = 100; //do not trigger the itemReady signal nor add the object to the parent layout
		connect(this, &QmlItemManager::setObjectReady, this, [&,set_number,exercise_idx] {
			return changeSetType(set_number, exercise_idx, 100);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		createSetObject(new_type, set_number, exercise_idx, false);
		return;
	}

	tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	QList<QQuickItem*>& set_objs(exercise_obj->m_setObjects);
	QQuickItem* parentLayout(exercise_obj->m_exerciseEntry->findChild<QQuickItem*>(u"exerciseSetsLayout"_qs));

	for(uint x(0); x < set_objs.count(); ++x)
		set_objs[x]->setParentItem(nullptr);
	for(uint x(0); x < set_objs.count(); ++x)
		set_objs[x]->setParentItem(parentLayout);
}

void QmlItemManager::changeSetMode(const uint exercise_idx, const uint set_number)
{
	QQuickItem* set_object(m_currentExercises->setObject(exercise_idx, set_number));
	uint set_mode(set_object->property("setMode").toUInt());
	switch(set_mode)
	{
		case 0:
		{
			const bool b_set_completed(!set_object->property("setCompleted").toBool());
			set_object->setProperty("setCompleted", b_set_completed);
			m_CurrenttDayModel->setSetCompleted(set_number, exercise_idx, b_set_completed);
			set_object->setProperty("bCurrentSet", !b_set_completed);
			if (!b_set_completed) //set was completed before, now it is not
				findSetMode(exercise_idx, set_number);

			QQuickItem* next_set_object = nextSetObject(exercise_idx, set_number);
			if (next_set_object)
			{
				if (!currenttDayModel()->setCompleted(set_number+1, exercise_idx))
					next_set_object->setProperty("bCurrentSet", b_set_completed);
			}
			return;
		}
		break;
		case 1:
			set_mode = 2;
			startRestTimer(exercise_idx, set_number);
		break;
		case 2:
			set_mode = 0;
			stopRestTimer(exercise_idx, set_number);
		break;
	}
	set_object->setProperty("setMode", set_mode);
}

void QmlItemManager::copyTypeValueIntoOtherSets(const uint exercise_idx, const uint set_number)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	const uint set_type(m_CurrenttDayModel->setType(set_number, exercise_idx));
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
		{
			changeSetType(i, exercise_idx, set_type);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeSetType", Q_ARG(int, static_cast<int>(set_type)));
		}
	}
}

void QmlItemManager::copyTimeValueIntoOtherSets(const uint exercise_idx, const uint set_number)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_CurrenttDayModel->setType(i, exercise_idx);
			updatedValue = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type, i-1);
			m_CurrenttDayModel->setSetRestTime(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeTime", Q_ARG(QString, updatedValue));
		}
	}
}

void QmlItemManager::copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_CurrenttDayModel->setType(i, exercise_idx);
			updatedValue = m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type, i-1, sub_set);
			if (set_type == SET_TYPE_DROP || set_type == SET_TYPE_GIANT)
				m_CurrenttDayModel->setSetReps(i, exercise_idx, sub_set, updatedValue);
			else
				m_CurrenttDayModel->setSetReps(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeReps", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
		}
	}
}

void QmlItemManager::copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_CurrenttDayModel->setType(i, exercise_idx);
			updatedValue = m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type, i-1, sub_set);
			if (set_type == SET_TYPE_DROP || set_type == SET_TYPE_GIANT)
				m_CurrenttDayModel->setSetWeight(i, exercise_idx, sub_set, updatedValue);
			else
				m_CurrenttDayModel->setSetWeight(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeWeight", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
		}
	}
}

QQuickItem* QmlItemManager::nextSetObject(const uint exercise_idx, const uint set_number) const
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	if (set_number < exercise_obj->m_setObjects.count()-1)
		return exercise_obj->m_setObjects.at(set_number+1);
	else
	{
		if (exercise_idx < m_currentExercises->exerciseObjects.count() - 1)
		{
			exercise_obj = m_currentExercises->exerciseObjects.at(exercise_idx+1);
			if (set_number < exercise_obj->m_setObjects.count())
				return exercise_obj->m_setObjects.at(set_number);
		}
	}
	return nullptr;
}
//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
void QmlItemManager::openMainMenuShortCut(const int button_id)
{
	QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
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

void QmlItemManager::displayMessageOnAppWindow(const int message_id) const
{
	QString title, message;
	const QString& fileName(appUtils()->getFileName(m_exportFilename));
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
	displayMessageOnAppWindow(title, message);
}

void QmlItemManager::displayMessageOnAppWindow(const QString& title, const QString& message) const
{
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, title), Q_ARG(QString, message));
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
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

//-----------------------------------------------------------SLOTS-----------------------------------------------------------
void QmlItemManager::mainWindowStarted() const
{
	appOsInterface()->initialCheck();
}

void QmlItemManager::requestTimerDialog(QQuickItem* requester, const QVariant& args)
{
	const QVariantList& strargs(args.toList());
	QMetaObject::invokeMethod(m_currenttDayPage, "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
}

void QmlItemManager::requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id)
{
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
	QMetaObject::invokeMethod(m_currentExercises->exerciseEntry_const(exercise_idx), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	if (exercise_idx < m_currentExercises->exercisesCount()-1)
	{
		if (!m_currentExercises->exerciseEntry_const(exercise_idx+1)->property("finishButtonEnabled").toBool())
		{
			QMetaObject::invokeMethod(m_currentExercises->exerciseEntry_const(exercise_idx+1), "paneExerciseShowHide", Q_ARG(bool, true), Q_ARG(bool, true));
			QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx+1)->property("y").toInt() + 50));
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
//-----------------------------------------------------------SLOTS-----------------------------------------------------------

//-----------------------------------------------------------USER PRIVATE-----------------------------------------------------------
void QmlItemManager::createSettingsPage()
{
	#ifdef DEBUG
	if (settingsComponent->status() == QQmlComponent::Error)
	{
		qDebug() << settingsComponent->errorString();
		for (uint i(0); i < settingsComponent->errors().count(); ++i)
			qDebug() << settingsComponent->errors().at(i).description();
		return;
	}
	#endif
	if (settingsComponent->status() == QQmlComponent::Ready)
	{
		settingsPage = static_cast<QQuickItem*>(settingsComponent->createWithInitialProperties(settingsProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(settingsPage, QQmlEngine::CppOwnership);
		settingsPage->setParentItem(appMainWindow()->contentItem());
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, settingsPage));
		userPage = settingsPage->findChild<QQuickItem*>(u"userPage"_qs);
		userPage->setProperty("useMode", appUserModel()->appUseMode(0));
		userPage->setProperty("itemManager", QVariant::fromValue(rootItemsManager()));
		connect(appUserModel(), &DBUserModel::appUseModeChanged, this, [&] (const uint user_row) {
			if (user_row == 0) {
				appMesoModel()->updateColumnLabels();
				QMetaObject::invokeMethod(appMainWindow(), "workoutButtonEnabled", Qt::AutoConnection);
				userPage->setProperty("useMode", appUserModel()->appUseMode(0));
			}
		});
		connect(appUserModel(), &DBUserModel::userModified, this, [&] (const uint user_row) {
			appDBInterface()->saveUser(user_row);
		});
	}
}

void QmlItemManager::createClientsOrCoachesPage()
{
	#ifdef DEBUG
	if (clientsOrCoachesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << clientsOrCoachesComponent->errorString();
		for (uint i(0); i < clientsOrCoachesComponent->errors().count(); ++i)
			qDebug() << clientsOrCoachesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (clientsOrCoachesComponent->status() == QQmlComponent::Ready)
	{
		clientsOrCoachesPage = static_cast<QQuickItem*>(clientsOrCoachesComponent->createWithInitialProperties(
				clientsOrCoachesProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(clientsOrCoachesPage, QQmlEngine::CppOwnership);
		clientsOrCoachesPage->setParentItem(appMainWindow()->contentItem());
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, clientsOrCoachesPage));
	}
}
//-----------------------------------------------------------USER PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISES TABLE PRIVATE-----------------------------------------------------------
void QmlItemManager::createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	exercisesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ExercisesPage.qml"_qs), QQmlComponent::Asynchronous);
	exercisesProperties.insert(u"bChooseButtonEnabled"_qs, bChooseButtonEnabled);
	exercisesProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));

	if (exercisesComponent->status() != QQmlComponent::Ready)
	{
		connect(exercisesComponent, &QQmlComponent::statusChanged, this, [&,connectPage] (QQmlComponent::Status) {
			return createExercisesPage_part2(connectPage);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createExercisesPage_part2(connectPage);
}

void QmlItemManager::createExercisesPage_part2(QQuickItem* connectPage)
{
	#ifdef DEBUG
	if (exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << exercisesComponent->errorString();
		for (uint i(0); i < exercisesComponent->errors().count(); ++i)
			qDebug() << exercisesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (exercisesComponent->status() == QQmlComponent::Ready)
	{
		exercisesPage = static_cast<QQuickItem*>(exercisesComponent->createWithInitialProperties(exercisesProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(exercisesPage, QQmlEngine::CppOwnership);
		exercisesPage->setParentItem(appMainWindow()->contentItem());
		appExercisesModel()->clearSelectedEntries();
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, exercisesPage));
		if (connectPage)
			connect(exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
		connect(appExercisesModel(), &DBExercisesModel::exerciseChanged, this, [&] (const uint index) {
			appDBInterface()->saveExercises();
		});
	}
}
//-----------------------------------------------------------EXERCISES TABLE PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES PRIVATE-----------------------------------------------------------
void QmlItemManager::createMesocyclePage(const QDate& minimumMesoStartDate, const QDate& maximumMesoEndDate, const QDate& calendarStartDate)
{
	const int meso_id(appMesoModel()->_id(m_mesoIdx));
	m_mesoProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_mesoProperties.insert(u"minimumMesoStartDate"_qs, !minimumMesoStartDate.isNull() ?
								minimumMesoStartDate : appMesoModel()->getPreviousMesoEndDate(meso_id));
	m_mesoProperties.insert(u"maximumMesoEndDate"_qs, !maximumMesoEndDate.isNull() ?
								maximumMesoEndDate : appMesoModel()->getNextMesoStartDate(meso_id));
	m_mesoProperties.insert(u"calendarStartDate"_qs, !calendarStartDate.isNull() ?
								calendarStartDate: appMesoModel()->startDate(m_mesoIdx));
	m_mesoMuscularGroupId = QTime::currentTime().msecsSinceStartOfDay();
	m_mesoProperties.insert(u"muscularGroupId"_qs, m_mesoMuscularGroupId);

	m_mesoComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoCycle.qml"_qs), QQmlComponent::Asynchronous);
	if (m_mesoComponent->status() != QQmlComponent::Ready)
	{
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
					createMesocyclePage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createMesocyclePage_part2();

	connect(appMainWindow(), SIGNAL(saveFileChosen(const QString&)), this, SLOT(exportSlot(const QString&)));
	connect(appMainWindow(), SIGNAL(saveFileRejected(const QString&)), this, SLOT(exportSlot(const QString&)));
}

void QmlItemManager::createMesocyclePage_part2()
{
	#ifdef DEBUG
	if (m_mesoComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_mesoComponent->errorString();
		for (uint i(0); i < m_mesoComponent->errors().count(); ++i)
			qDebug() << m_mesoComponent->errors().at(i).description();
		return;
	}
	#endif
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(app_StackView);
	m_mesoPage->setProperty("useMode", appUserModel()->appUseMode(0));
	m_mesoPage->setProperty("bOwnMeso", appMesoModel()->isOwnMeso(m_mesoIdx));
	m_mesoPage->setProperty("bRealMeso", appMesoModel()->isRealMeso(m_mesoIdx));

	connect(appUserModel(), &DBUserModel::appUseModeChanged, this, [&] (const uint user_row) {
		if (user_row == 0)
				m_mesoPage->setProperty("useMode", appUserModel()->appUseMode(0));
	});
	connect(appUserModel(), &DBUserModel::userAdded, this, [&] (const uint user_row) {
		QMetaObject::invokeMethod(m_mesoPage, "updateCoachesModel", Q_ARG(int, static_cast<int>(user_row)));
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, this, [&] (const uint meso_idx) {
		if (meso_idx == m_mesoIdx)
			QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
	});
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [&] (const uint meso_idx, const uint initiator_id, const int splitIndex, const QChar& splitLetter) {
		if (meso_idx == m_mesoIdx && initiator_id == m_mesoMuscularGroupId )
			QMetaObject::invokeMethod(m_mesoPage, "updateMuscularGroup", Q_ARG(int, splitIndex), Q_ARG(QString, QString(splitLetter)));
	});
	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [&] (const uint meso_idx, const uint meso_field) {
		if (meso_idx == m_mesoIdx)
		{
			appDBInterface()->saveMesocycle(meso_idx);
			QMetaObject::invokeMethod(m_mesoPage, "updateFieldValues", Q_ARG(int, static_cast<int>(meso_field)), Q_ARG(int, static_cast<int>(meso_idx)));
			m_mesoPage->setProperty("bRealMeso", appMesoModel()->isRealMeso(meso_idx));
		}
	});
}
//-----------------------------------------------------------MESOCYCLES PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR PRIVATE-----------------------------------------------------------
void QmlItemManager::createMesoCalendarPage()
{
	m_calComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoCalendar.qml"_qs), QQmlComponent::Asynchronous);
	m_calProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_calProperties.insert(u"mesoCalendarModel"_qs, QVariant::fromValue(appMesoModel()->mesoCalendarModel(m_mesoIdx)));

	if (m_calComponent->status() != QQmlComponent::Ready)
	{
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
			return createMesoCalendarPage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	}
	else
		createMesoCalendarPage_part2();
}

void QmlItemManager::createMesoCalendarPage_part2()
{
	m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, appQmlEngine()->rootContext()));
	#ifdef DEBUG
	if (m_calComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_calComponent->errorString();
		for (uint i(0); i < m_calComponent->errors().count(); ++i)
			qDebug() << m_calComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
	m_calPage->setParentItem(app_StackView);
	addMainMenuShortCut(tr("Calendar: ") + appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_calPage);
}
//-----------------------------------------------------------MESOCALENDAR PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT PRIVATE-----------------------------------------------------------
void QmlItemManager::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent{appQmlEngine(), QUrl(u"qrc:/qml/Pages/ExercisesPlanner.qml"_qs), QQmlComponent::Asynchronous};
	m_plannerProperties[u"itemManager"_qs] = QVariant::fromValue(this);
	if (m_plannerComponent->status() != QQmlComponent::Ready)
		connect(m_plannerComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return QmlItemManager::createPlannerPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createPlannerPage_part2();
}

void QmlItemManager::createPlannerPage_part2()
{
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(m_plannerProperties, appQmlEngine()->rootContext()));
	#ifdef DEBUG
	if (m_plannerComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_plannerComponent->errorString();
		for (uint i(0); i < m_plannerComponent->errors().count(); ++i)
			qDebug() << m_plannerComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(app_StackView);
	QMetaObject::invokeMethod(m_plannerPage, "createNavButtons");
	addMainMenuShortCut(tr("Exercises Planner: ") + appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_plannerPage);
}

void QmlItemManager::createMesoSplitPage(const uint page_index)
{
	#ifdef DEBUG
	if (m_splitComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_splitComponent->errorString();
		for (uint i(0); i < m_splitComponent->errors().count(); ++i)
			qDebug() << m_splitComponent->errors().at(i).description();
		return;
	}
	#endif

	DBMesoSplitModel* splitModel(nullptr);
	int i(-1);
	QMap<QChar,DBMesoSplitModel*>::const_iterator itr(m_splitModels.constBegin());
	const QMap<QChar,DBMesoSplitModel*>::const_iterator& itr_end(m_splitModels.constEnd());
	while (++itr != itr_end)
	{
		++i;
		if (i == page_index)
		{
			if (m_splitPages.contains((*itr)->splitLetter().at(0)))
				return;
			splitModel = *itr;
			break;
		}
		++itr;
	}
	if (!splitModel)
		return;

	m_splitProperties[u"splitModel"_qs] = QVariant::fromValue(splitModel);
	m_splitProperties[u"parentItem"_qs] = QVariant::fromValue(m_plannerPage);
	m_splitProperties[u"itemManager"_qs] = QVariant::fromValue(this);
	m_splitMuscularGroupId = QTime::currentTime().msecsSinceStartOfDay();
	m_mesoProperties[u"muscularGroupId"_qs] = m_splitMuscularGroupId;

	QQuickItem* item (static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_plannerPage);

	connect(item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)));
	splitModel->setCurrentRow(0);

	if (splitModel->count() == 0)
		splitModel->addExercise(tr("Choose exercise..."), SET_TYPE_REGULAR, u"4"_qs, u"12"_qs, u"20"_qs);
	setSplitPageProperties(item, splitModel);

	m_splitPages.insert(splitModel->splitLetter().at(0), item);
	QMetaObject::invokeMethod(m_plannerPage, "insertSplitPage", Q_ARG(QQuickItem*, item),
								Q_ARG(int, appUtils()->splitLetterToIndex(splitModel->splitLetter())));

	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [&] (const uint meso_idx, const uint initiator_id, const int splitIndex, const QChar& splitLetter) {
			if (meso_idx == m_mesoIdx && initiator_id == m_splitMuscularGroupId )
			{
				if (splitIndex < m_splitModels.count())
					updateMuscularGroup(m_splitModels.value(splitLetter));
			}
	});
	addMainMenuShortCut(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_mesoPage);
}

void QmlItemManager::initializeSplitModels()
{
	const QString& mesoSplit(appMesoModel()->getFast(m_mesoIdx, MESOCYCLES_COL_SPLIT));
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator& itr_end(mesoSplit.constEnd());

	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		static_cast<void>(getSplitModel(*itr));
	} while (++itr != itr_end);
}

void QmlItemManager::setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel)
{
	int prevMesoId(-1);
	prevMesoId = appMesoModel()->getPreviousMesoId(appMesoModel()->_id(m_mesoIdx));
	if (prevMesoId >= 0)
	{
		if (appDBInterface()->mesoHasPlan(prevMesoId, splitModel->splitLetter()))
		{
			splitPage->setProperty("prevMesoName", appMesoModel()->getFast(prevMesoId, MESOCYCLES_COL_NAME));
			prevMesoId = -1; //Nothing from previous meso to import
		}
	}
	const QString& swappableLetter(splitModel->findSwappableModel());
	splitPage->setProperty("prevMesoId", prevMesoId);
	splitPage->setProperty("swappableLetter", swappableLetter);
	splitPage->setProperty("bCanSwapPlan", !swappableLetter.isEmpty());
}

//Updates MesoSplitPlanner(and its corresponding models) with the changes originating in MesoCycle.qml
void QmlItemManager::updateMuscularGroup(DBMesoSplitModel* splitModel)
{
	const QString& musculargroup{appMesoModel()->muscularGroup(m_mesoIdx, splitModel->splitLetter())};
	splitModel->setMuscularGroup(musculargroup);
	QQuickItem* splitPage(getSplitPage(splitModel->splitLetter().at(0)));
	if (splitPage)
	{
		setSplitPageProperties(splitPage, splitModel);
		QMetaObject::invokeMethod(splitPage, "updateTxtGroups", Q_ARG(QString, musculargroup));
	}
}

void QmlItemManager::changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel, const uint initiator_id)
{
	splitModel->setMuscularGroup(new_musculargroup);
	appMesoModel()->setMuscularGroup(m_mesoIdx, splitModel->splitLetter(), new_musculargroup, initiator_id);
	setSplitPageProperties(m_splitPages.value(splitModel->splitLetter().at(0)), splitModel);
}
//-----------------------------------------------------------MESOSPLIT PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------
void QmlItemManager::createTrainingDayPage(const QDate& date)
{
	if (!m_tDayPages.contains(date))
	{
		if (m_tDayComponent == nullptr)
			m_tDayComponent = new QQmlComponent{appQmlEngine(), QUrl(u"qrc:/qml/Pages/TrainingDayInfo.qml"_qs), QQmlComponent::Asynchronous};

		if (!m_tDayExercisesList.contains(date))
		{
			m_currentExercises = new tDayExercises;
			m_tDayExercisesList.insert(date, m_currentExercises);
		}

		if (m_tDayComponent->status() != QQmlComponent::Ready)
			connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
				{ return createTrainingDayPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		else
			createTrainingDayPage_part2();
	}
}

void QmlItemManager::createTrainingDayPage_part2()
{
	#ifdef DEBUG
	if (m_tDayComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayComponent->errorString();
		for (uint i(0); i < m_tDayComponent->errors().count(); ++i)
			qDebug() << m_tDayComponent->errors().at(i).description();
		return;
	}
	#endif
	QQuickItem* page{static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, appQmlEngine()->rootContext()))};
	appQmlEngine()->setObjectOwnership(page, QQmlEngine::CppOwnership);
	page->setParentItem(app_StackView);
	m_currenttDayPage = page;
	m_tDayPages.insert(m_tDayModels.key(m_CurrenttDayModel), page);

	connect(appMesoModel()->mesoCalendarModel(m_mesoIdx), &DBMesoCalendarModel::calendarChanged, this, [&] (const QDate& startDate, const QDate& endDate) {
							updateOpenTDayPagesWithNewCalendarInfo(startDate, endDate);
	});
	connect(m_CurrenttDayModel, &DBTrainingDayModel::exerciseCompleted, this, [&] (const uint exercise_idx, const bool completed) {
							enableDisableExerciseCompletedButton(exercise_idx, completed);
	});

	const QDate& date(m_CurrenttDayModel->date());
	m_currenttDayPage->setProperty("dayIsNotCurrent", date != QDate::currentDate());
	if (m_CurrenttDayModel->dayIsFinished())
	{
		const QTime& workoutLenght(appUtils()->calculateTimeDifference(m_CurrenttDayModel->timeIn(), m_CurrenttDayModel->timeOut()));
		QMetaObject::invokeMethod(m_currenttDayPage, "updateTimer", Q_ARG(int, workoutLenght.hour()),
				Q_ARG(int, workoutLenght.minute()), Q_ARG(int, workoutLenght.second()));
	}
	addMainMenuShortCut(tr("Workout: ") + appUtils()->formatDate(date), m_currenttDayPage);
}

void QmlItemManager::updateOpenTDayPagesWithNewCalendarInfo(const QDate& startDate, const QDate& endDate)
{
	QMapIterator<QDate,QQuickItem*> i(m_tDayPages);
	i.toFront();
	const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
	while (i.hasNext())
	{
		i.next();
		if (i.key() > startDate) //the startDate page is the page that initiated the update. No need to alter it
		{
			if (i.key() <= endDate)
			{
				QMetaObject::invokeMethod(i.value(), "warnCalendarChanged",
					Q_ARG(QString, mesoCal->getSplitLetter(i.key().month(), i.key().day())),
					Q_ARG(QString, QString::number(mesoCal->getTrainingDay(i.key().month(), i.key().day()))),
					Q_ARG(QString, appMesoModel()->muscularGroup(m_mesoIdx, mesoCal->getSplitLetter(startDate.month(), startDate.day()))));
			}
		}
	}
}

void QmlItemManager::setTrainingDayPageEmptyDayOptions(const DBTrainingDayModel* const model)
{
	if (model->count() > 0)
	{
		if (m_currenttDayPage)
		{
			m_currenttDayPage->setProperty("previousTDays", QVariant::fromValue(model->getRow_const(0)));
			m_currenttDayPage->setProperty("bHasPreviousTDays", true);
			if (model->count() == 2)
			{
				if (m_currenttDayPage)
				{
					m_currenttDayPage->setProperty("lastWorkOutLocation", model->getRow_const(1).at(TDAY_COL_LOCATION));
					//TDAY_COL_TRAININGDAYNUMBER is just a placeholder for the value we need
					m_currenttDayPage->setProperty("bHasMesoPlan", model->getRow_const(1).at(TDAY_COL_TRAININGDAYNUMBER) == STR_ONE);
				}
				else
				{
					m_tDayProperties.insert(u"lastWorkOutLocation"_qs, QVariant::fromValue(model->getRow_const(1).at(TDAY_COL_LOCATION)));
					m_tDayProperties.insert(u"bHasMesoPlan"_qs, model->getRow_const(1).at(TDAY_COL_TRAININGDAYNUMBER) == STR_ONE);
				}
			}
		}
		else
		{
			m_tDayProperties.insert(u"previousTDays"_qs, QVariant::fromValue(model->getRow_const(0)));
			m_tDayProperties.insert(u"bHasPreviousTDays"_qs, true);
		}
	}
	else
	{
		if (m_currenttDayPage)
		{
			m_currenttDayPage->setProperty("previousTDays", QVariant::fromValue(QStringList()));
			m_currenttDayPage->setProperty("bHasMesoPlan", false);
			m_currenttDayPage->setProperty("bHasPreviousTDays", false);
		}
		else
		{
			m_tDayProperties.insert(u"previousTDays"_qs, QVariant::fromValue(QStringList()));
			m_tDayProperties.insert(u"bHasPreviousTDays"_qs, false);
			m_tDayProperties.insert(u"bHasPreviousTDays"_qs, false);
		}
	}
	if (m_currenttDayPage)
		m_currenttDayPage->setProperty("pageOptionsLoaded", true);
	else
		m_tDayProperties.insert(u"pageOptionsLoaded"_qs, true);
}

void QmlItemManager::tDayExercises::appendExerciseEntry(QQuickItem* new_exerciseItem)
{
	exerciseObject* exerciseObj{new exerciseObject};
	exerciseObj->m_exerciseEntry = new_exerciseItem;
	exerciseObjects.append(exerciseObj);
}

void QmlItemManager::tDayExercises::removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow)
{
	exerciseObject* exerciseObj(exerciseObjects.at(exercise_idx));
	for (uint x(0); x < exerciseObj->m_setObjects.count(); ++x)
		bDeleteNow ? delete exerciseObj->m_setObjects.at(x) : exerciseObj->m_setObjects.at(x)->deleteLater();
	exerciseObj->m_setObjects.clear();
	if (exerciseObj->m_setTimer)
		bDeleteNow ? delete exerciseObj->m_setTimer : exerciseObj->m_setTimer->deleteLater();
	bDeleteNow ? delete exerciseObj->m_exerciseEntry : exerciseObj->m_exerciseEntry->deleteLater();
	exerciseObjects.removeAt(exercise_idx);
	delete exerciseObj;
}

void QmlItemManager::tDayExercises::removeSet(const uint exercise_idx, const uint set_number)
{
	setObject(exercise_idx, set_number)->deleteLater();
	exerciseObjects.at(exercise_idx)->m_setObjects.remove(set_number);
}
//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------

//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------
void QmlItemManager::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent{appQmlEngine(), QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous};
		if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
			connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return createExercisesObjects(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		else
			createExercisesObjects();
		QMetaObject::invokeMethod(m_currenttDayPage, "createNavButtons", Qt::AutoConnection);
	}
	else
	{
		m_currenttDayPage->setProperty("bHasMesoPlan", false);
		m_currenttDayPage->setProperty("bHasPreviousTDays", false);
		for(uint i(0); i < m_CurrenttDayModel->exerciseCount(); ++i)
		{
			createExerciseObject_part2(i);
			m_currentExercises->exerciseEntry(i)->setProperty("setNbr", m_CurrenttDayModel->setsNumber(i));
		}
	}
}

void QmlItemManager::createExerciseObject_part2(const int object_idx)
{
	#ifdef DEBUG
	if (m_tDayExercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayExercisesComponent->errorString();
		for (uint i(0); i < m_tDayExercisesComponent->errors().count(); ++i)
			qDebug() << m_tDayExercisesComponent->errors().at(i).description();
		return;
	}
	#endif

	QString nRestTime;
	bool bTrackRestTime(false), bAutoRestTime(false);
	const int exercise_idx(object_idx >= 0 ? object_idx : m_currentExercises->exerciseObjects.count());

	if (exercise_idx > 1)
	{
		bTrackRestTime = m_CurrenttDayModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_CurrenttDayModel->autoRestTime(exercise_idx-1);
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, SET_TYPE_REGULAR, 0);
	}
	else
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);

	m_tDayExerciseEntryProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_tDayExerciseEntryProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
	m_tDayExerciseEntryProperties.insert(u"exerciseIdx"_qs, exercise_idx);
	m_tDayExerciseEntryProperties.insert(u"nRestTime"_qs, nRestTime);
	m_tDayExerciseEntryProperties.insert(u"bTrackRestTime"_qs, bTrackRestTime);
	m_tDayExerciseEntryProperties.insert(u"bAutoRestTime"_qs, bAutoRestTime);
	m_tDayExerciseEntryProperties.insert(u"bCanEditRestTimeTracking"_qs, true);
	m_tDayExerciseEntryProperties.insert(u"bCompositeExercise"_qs, m_CurrenttDayModel->compositeExercise(m_CurrenttDayModel->exerciseCount()-1));

	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parentLayout(m_currenttDayPage->findChild<QQuickItem*>(u"tDayExercisesLayout"_qs));
	item->setParentItem(parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
	connect( item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)) );
	connect( item, SIGNAL(showRemoveExerciseMessage(int)), this, SLOT(showRemoveExerciseMessage(int)) );

	m_currentExercises->appendExerciseEntry(item);
	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
}
//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------
void QmlItemManager::createSetObject_part2(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet)
{
	const uint set_type_cpp(set_type == SET_TYPE_DROP ? 1 : set_type == SET_TYPE_GIANT ? 2 : 0);
	#ifdef DEBUG
	if (m_setComponents[set_type_cpp]->status() == QQmlComponent::Error)
	{
		for (uint i(0); i < m_setComponents[set_type_cpp]->errors().count(); ++i)
			qDebug() << m_setComponents[set_type_cpp]->errors().at(i).description();
		return;
	}
	#endif

	m_setObjectProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_setObjectProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
	m_setObjectProperties.insert(u"exerciseIdx"_qs, exercise_idx);
	m_setObjectProperties.insert(u"setNumber"_qs, set_number);
	m_setObjectProperties.insert(u"setType"_qs, set_type);
	m_setObjectProperties.insert(u"setCompleted"_qs, m_CurrenttDayModel->setCompleted(set_number, exercise_idx));
	m_setObjectProperties.insert(u"bTrackRestTime"_qs, m_CurrenttDayModel->trackRestTime(exercise_idx));
	m_setObjectProperties.insert(u"bAutoRestTime"_qs, m_CurrenttDayModel->autoRestTime(exercise_idx));
	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);

	//Default values for these properties. They are only modified, on the c++ side, in changeSetType().
	m_setObjectProperties.insert(u"copyTypeButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyTimeButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyRepsButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyWeightButtonValue"_qs, QString());

	if (set_number >= m_currentExercises->setCount(exercise_idx))
		m_currentExercises->appendSet(exercise_idx, item);
	else
		m_currentExercises->insertSet(set_number, exercise_idx, item);

	findSetMode(exercise_idx, set_number);
	findCurrentSet(exercise_idx, set_number);
	connect( item, SIGNAL(requestTimerDialogSignal(QQuickItem*,const QVariant&)), this, SLOT(requestTimerDialog(QQuickItem*,const QVariant&)) );
	connect( item, SIGNAL(exerciseCompleted(int)), this, SLOT(exerciseCompleted(int)) );
	connect( item, SIGNAL(showRemoveSetMessage(int,int)), this, SLOT(showRemoveSetMessage(int,int)) );
	if (set_number == currenttDayModel()->setsNumber(exercise_idx)-1)
	{
		if (!bNewSet)
		{
			item->setProperty("finishButtonVisible", true);
			enableDisableExerciseCompletedButton(exercise_idx, currenttDayModel()->setCompleted(set_number, exercise_idx));
			//Place into view: exercise entry + first set
			QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView",
						Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx)->property("height").toInt()));
		}
		else
		{
			for (uint i(0); i < m_currentExercises->setCount(exercise_idx)-1; ++i)
				m_currentExercises->setObject(exercise_idx, i)->setProperty("finishButtonVisible", false);
			item->setProperty("finishButtonVisible", true);
		}
	}

	//Sets may be created at any random order, specially when there are set objects of different kinds within an exercise. m_expectedSetNumber keeps
	//track of the order in which the sets are added. When set_number is greater than m_expectedSetNumber, the set objects are not inserted into
	//the parent layout(with setParentItem). When the expected set_number is finally created, put all sets already in the list (m_setObjects)
	//orderly into the layout
	if (set_number <= m_expectedSetNumber)
	{
		QQuickItem* parent(m_currentExercises->exerciseEntry(exercise_idx)->findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));
		for (uint i(set_number); i < m_currentExercises->setCount(exercise_idx); ++i, ++m_expectedSetNumber)
		{
			if (m_currentExercises->setObject(exercise_idx, i)->property("setNumber").toUInt() <= i)
			{
				m_currentExercises->setObject(exercise_idx, i)->setParentItem(parent);
				emit setObjectReady();
			}
		}
	}

	if (set_type == SET_TYPE_GIANT)
	{
		item->setProperty("ownerExercise", QVariant::fromValue(m_currentExercises->exerciseEntry(exercise_idx)));
		QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
	}
	else if (set_type == SET_TYPE_DROP)
		QMetaObject::invokeMethod(item, "init");

	//After any set added, by default, set the number of sets to be added afterwards to 1, and set the suggested rest time, reps and weight for the next set based on this last one
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nSets", "1");
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nRestTime", m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type));
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nReps", m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type));
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nWeight", m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type));
}

void QmlItemManager::enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	const uint nsets(exercise_obj->m_setObjects.count());
	bool noSetsCompleted(true);
	for (uint i(0); i < nsets; ++i)
	{
		QQuickItem* setObject{exercise_obj->m_setObjects.at(i)};
		if (setObject->property("finishButtonVisible").toBool())
		{
			setObject->setProperty("finishButtonEnabled", completed);
			break;
		}
		if (setObject->property("setCompleted").toBool())
			noSetsCompleted = false;
	}
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("bCanEditRestTimeTracking", noSetsCompleted);
}

void QmlItemManager::enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime,
								const uint bAutoRestTime, const uint except_set_number)
{
	const uint nsets(m_currentExercises->setCount(exercise_idx));
	QString strRestTime;
	for(uint i(1); i < nsets; ++i)
	{
		if (i != except_set_number)
		{
			if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
			{
				findSetMode(exercise_idx, i);
				m_currentExercises->setObject(exercise_idx, i)->setProperty("bTrackRestTime", bTrackRestTime);
				m_currentExercises->setObject(exercise_idx, i)->setProperty("bAutoRestTime", bAutoRestTime);
				if (bAutoRestTime)
					strRestTime = u"00:00"_qs;
				else if (bTrackRestTime)
					strRestTime = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, m_CurrenttDayModel->setType(i, exercise_idx), i);
				m_CurrenttDayModel->setSetRestTime(i, exercise_idx, strRestTime);
				QMetaObject::invokeMethod(m_currentExercises->setObject(exercise_idx, i), "updateRestTime", Q_ARG(QString, strRestTime));
			}
		}
	}
}

void QmlItemManager::findSetMode(const uint exercise_idx, const uint set_number)
{
	int set_mode(0);
	if (set_number > 0)
	{
		if (m_CurrenttDayModel->trackRestTime(exercise_idx))
		{
			if (m_CurrenttDayModel->autoRestTime(exercise_idx))
				set_mode = 1;
		}
	}
	m_currentExercises->setObject(exercise_idx, set_number)->setProperty("setMode", set_mode);
}

void QmlItemManager::findCurrentSet(const uint exercise_idx, const uint set_number)
{
	QQuickItem* set_object{m_currentExercises->setObject(exercise_idx, set_number)};
	if (set_object)
	{
		if (!currenttDayModel()->setCompleted(set_number, exercise_idx))
		{
			if (set_number > 0)
				set_object->setProperty("bCurrentSet", currenttDayModel()->setCompleted(set_number-1, exercise_idx));
			else
				set_object->setProperty("bCurrentSet", true);
		}
	}
}

void QmlItemManager::startRestTimer(const uint exercise_idx, const uint set_number)
{
	TPTimer* set_timer{m_currentExercises->setTimer(exercise_idx)};
	set_timer->setInterval(1000);
	set_timer->setStopWatch(true);
	set_timer->prepareTimer(u"-"_qs);
	enableDisableSetsRestTime(exercise_idx, false, false, set_number); //Prevent the user from starting the timer for another set before finishing this one
	QQuickItem* set_object{m_currentExercises->setObject(exercise_idx, set_number)};
	connect(set_timer, &TPTimer::secondsChanged, this, [&,set_timer] () {
		QMetaObject::invokeMethod(set_object, "updateRestTime", Q_ARG(QString, set_timer->strMinutes() + ':' + set_timer->strSeconds()));
	});
	connect(set_timer, &TPTimer::minutesChanged, this, [&,set_timer] () {
		QMetaObject::invokeMethod(set_object, "updateRestTime", Q_ARG(QString, set_timer->strMinutes() + ':' + set_timer->strSeconds()));
	});
	set_timer->startTimer(u"-"_qs);
}

void QmlItemManager::stopRestTimer(const uint exercise_idx, const uint set_number)
{
	TPTimer* set_timer{m_currentExercises->setTimer(exercise_idx)};
	if (set_timer->isActive())
	{
		set_timer->stopTimer();
		disconnect(set_timer, nullptr, nullptr, nullptr);
		enableDisableSetsRestTime(exercise_idx, true, true, set_number);
		m_CurrenttDayModel->setSetRestTime(set_number, exercise_idx, set_timer->strMinutes() + ':' + set_timer->strSeconds());
	}
}
//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
void QmlItemManager::setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches)
{
	int curUserRow(0), firstUserRow(-1), lastUserRow(-1);

	if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_USER)
		return;

	if (bManageClients)
	{
		if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = appUserModel()->findFirstUser(false);
			lastUserRow = appUserModel()->findLastUser(false);
			curUserRow = appUserModel()->currentUser(0);
		}
	}

	if (bManageCoaches)
	{
		if (appUserModel()->appUseMode(0) == APP_USE_MODE_SINGLE_USER_WITH_COACH || APP_USE_MODE_COACH_USER_WITH_COACH)
		{
			firstUserRow = appUserModel()->findFirstUser(true);
			lastUserRow = appUserModel()->findLastUser(true);
			curUserRow = appUserModel()->currentCoach(0);
		}
	}

	if (curUserRow == 0)
		curUserRow = lastUserRow;
	if (clientsOrCoachesPage)
	{
		clientsOrCoachesPage->setProperty("curUserRow", curUserRow);
		clientsOrCoachesPage->setProperty("firstUserRow", firstUserRow);
		clientsOrCoachesPage->setProperty("lastUserRow", lastUserRow);
		clientsOrCoachesPage->setProperty("showUsers", bManageClients);
		clientsOrCoachesPage->setProperty("showCoaches", bManageCoaches);

	}
	else
	{
		clientsOrCoachesProperties.insert(u"curUserRow"_qs, curUserRow);
		clientsOrCoachesProperties.insert(u"firstUserRow"_qs, firstUserRow);
		clientsOrCoachesProperties.insert(u"lastUserRow"_qs, lastUserRow);
		clientsOrCoachesProperties.insert(u"showUsers"_qs, bManageClients);
		clientsOrCoachesProperties.insert(u"showCoaches"_qs, bManageCoaches);
	}
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
//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
