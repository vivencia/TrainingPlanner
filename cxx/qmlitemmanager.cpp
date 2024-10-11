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

#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QSettings>
#include <QFile>

QmlItemManager* QmlItemManager::app_qml_manager(nullptr);

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
void QmlItemManager::createExerciseObject()
{
	if (m_tDayExercisesComponent == nullptr)
		m_tDayExercisesComponent = new QQmlComponent(m_appQmlEngine, QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);

	if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
			{ return createExerciseObject_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createExerciseObject_part2();
}

void QmlItemManager::removeExerciseObject(const uint exercise_idx)
{
	if (exercise_idx < m_currentExercises->exerciseObjects.count())
	{
		m_CurrenttDayModel->removeExercise(exercise_idx);
		removeExerciseEntry(exercise_idx);
		for(uint i(exercise_idx); i < m_currentExercises->exerciseObjects.count(); ++i)
		{
			//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
			QMetaObject::invokeMethod(exerciseEntryItem(i), "moveExercise", Q_ARG(bool, true), Q_ARG(bool, false));
			for(uint x(0); x < exerciseSetsCount(i); ++x)
				exerciseSetItem(i, x)->setProperty("exerciseIdx", i);
		}
	}
}

void QmlItemManager::clearExercises()
{
	clearExerciseEntries();
	m_CurrenttDayModel->clearExercises();
	m_CurrenttDayModel->setDayIsFinished(false);
}

void QmlItemManager::moveExercise(const uint exercise_idx, const uint new_idx)
{
	uint nsets(exerciseSetsCount(exercise_idx));
	for(uint i(0); i < nsets; ++i)
	{
		exerciseSetItem(exercise_idx, i)->setProperty("exerciseIdx", new_idx);
		exerciseSetItem(exercise_idx, i)->setProperty("ownerExercise", QVariant::fromValue(exerciseEntryItem(new_idx)));
	}
	nsets = exerciseSetsCount(new_idx);
	for(uint i(0); i < nsets; ++i)
	{
		exerciseSetItem(new_idx, i)->setProperty("exerciseIdx", exercise_idx);
		exerciseSetItem(new_idx, i)->setProperty("ownerExercise", QVariant::fromValue(exerciseEntryItem(exercise_idx)));
	}
	m_CurrenttDayModel->moveExercise(exercise_idx, new_idx);

	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(nullptr);

	QQuickItem* parentLayout(m_currenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	m_currentExercises->exerciseObjects.swapItemsAt(exercise_idx, new_idx);
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(parentLayout);
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "moveExercise", Q_ARG(bool, new_idx > exercise_idx), Q_ARG(bool, false));
}

void QmlItemManager::manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type)
{
	if (!bTrackRestTime)
		bAutoRestTime = false;
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);
	setExerciseRestTime(exercise_idx, bAutoRestTime ?
											u"00:00"_qs :
											m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, new_set_type, 0));
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "updateScreenControls");
	enableDisableSetsRestTime(exercise_idx, bTrackRestTime, bAutoRestTime);
}

uint QmlItemManager::exerciseSetsCount(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.count();
}

uint QmlItemManager::exerciseDefaultSetType(const uint exercise_idx)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->newSetType;
}

inline void QmlItemManager::setExerciseDefaultSetType(const uint exercise_idx, const uint set_type)
{
	tDayExercises::exerciseObject* exerciseObj{m_currentExercises->exerciseObjects.at(exercise_idx)};
	exerciseObj->newSetType = set_type;
	const uint nSets(exerciseSetsCount(exercise_idx));

	if (nSets == 0)
	{
		switch(set_type)
		{
			case SET_TYPE_DROP: setExerciseSets(exercise_idx, u"1"_qs); break;
			case SET_TYPE_CLUSTER: setExerciseSets(exercise_idx, u"2"_qs); break;
			case SET_TYPE_MYOREPS: setExerciseSets(exercise_idx, u"3"_qs); break;
			default: break;
		}
	}
	else
	{
		setExerciseSets(exercise_idx, u"1"_qs);
		setExerciseRestTime(exercise_idx, m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type, nSets));
		setExerciseReps(exercise_idx, m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type, nSets));
		setExerciseWeight(exercise_idx, m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type, nSets));
	}

	if (m_CurrenttDayModel->trackRestTime(exercise_idx) && !m_CurrenttDayModel->autoRestTime(exercise_idx))
		setExerciseRestTime(exercise_idx, m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type, nSets));

	if (nSets == 0)
	{
		const bool bCompositeExercise{set_type == SET_TYPE_GIANT};
		if (bCompositeExercise) //trigger DBTrainingDayModel::compositeExerciseChanged. This will update the screen controls
			m_CurrenttDayModel->setExerciseName(m_CurrenttDayModel->exerciseName1(exercise_idx), exercise_idx);
		else
			m_CurrenttDayModel->setExerciseName1(m_CurrenttDayModel->exerciseName(exercise_idx), exercise_idx);
	}
}

const QString QmlItemManager::exerciseSets(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nSets;
}

void QmlItemManager::setExerciseSets(const uint exercise_idx, const QString& new_nsets)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nSets = new_nsets;
}

const QString QmlItemManager::exerciseRestTime(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->restTime;
}

inline void QmlItemManager::setExerciseRestTime(const uint exercise_idx, const QString& new_resttime)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->restTime = new_resttime;
}

const QString QmlItemManager::exerciseReps(const uint exercise_idx, const uint composite_idx) const
{
	return appUtils()->getCompositeValue(composite_idx, m_currentExercises->exerciseObjects.at(exercise_idx)->nReps, comp_exercise_separator);
}

void QmlItemManager::setExerciseReps(const uint exercise_idx, const uint composite_idx, const QString& new_nreps)
{
	appUtils()->setCompositeValue(composite_idx, new_nreps, m_currentExercises->exerciseObjects.at(exercise_idx)->nReps, comp_exercise_separator);
}

const QString QmlItemManager::exerciseWeight(const uint exercise_idx, const uint composite_idx) const
{
	return appUtils()->getCompositeValue(composite_idx, m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight, comp_exercise_separator);
}

void QmlItemManager::setExerciseWeight(const uint exercise_idx, const uint composite_idx, const QString& new_nweight)
{
	appUtils()->setCompositeValue(composite_idx, new_nweight, m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight, comp_exercise_separator);
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
void QmlItemManager::getSetObjects(const uint exercise_idx)
{
	if (exerciseSetsCount(exercise_idx) == 0)
	{
		m_expectedSetNumber = 0;
		const uint nsets(currenttDayModel()->setsNumber(exercise_idx));
		for (uint i(0); i < nsets; ++i)
			createSetObject(exercise_idx, i, false, currenttDayModel()->setType(i, exercise_idx));
		setExerciseDefaultSetType(exercise_idx, m_CurrenttDayModel->setType(m_CurrenttDayModel->setsNumber(exercise_idx), exercise_idx));
	}
	else
		//Place into view: exercise entry + first set
		QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, exerciseEntryItem(exercise_idx)->property("y").toInt() + 50));
}

void QmlItemManager::addNewSet(const uint exercise_idx)
{
	createSetObject(exercise_idx, exerciseSetsCount(exercise_idx), true, exerciseDefaultSetType(exercise_idx), exerciseReps(exercise_idx),
				exerciseWeights(exercise_idx), exerciseRestTime(exercise_idx));
	//Place into view: most recent set added
	QMetaObject::invokeMethod( m_currenttDayPage, "placeSetIntoView", Q_ARG(int, exerciseEntryItem(exercise_idx)->property("height").toInt()));
	setExerciseDefaultSetType(exercise_idx, m_CurrenttDayModel->setType(m_CurrenttDayModel->setsNumber(exercise_idx), exercise_idx));
}

void QmlItemManager::removeSetObject(const uint set_number, const uint exercise_idx)
{
	if (exercise_idx < exercisesCount())
	{
		for(uint x(set_number+1); x < exerciseSetsCount(exercise_idx); ++x)
			exerciseSetItem(exercise_idx, x)->setProperty("setNumber", x-1);
		m_CurrenttDayModel->removeSet(set_number, exercise_idx);
		removeExerciseSet(exercise_idx, set_number);
		const uint nsets(exerciseSetsCount(exercise_idx));
		exerciseEntryItem(exercise_idx)->setProperty("setNbr", nsets);
		if (nsets == 0)
		{
			exerciseEntryItem(exercise_idx)->setProperty("bCanEditRestTimeTracking", true);
			return;
		}
		else if (set_number == nsets) //last set was removed, update suggested values for a possible set addition
		{
			exerciseEntryItem(exercise_idx)->setProperty("nReps", m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
			exerciseEntryItem(exercise_idx)->setProperty("nWeight", m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
			if (set_number > 1)
			{
				exerciseSetItem(exercise_idx, set_number-1)->setProperty("finishButtonVisible", true);
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
	for (uint i(0); i < exerciseSetsCount(exercise_idx); ++i)
	{
		if (m_CurrenttDayModel->setType(i, exercise_idx) == SET_TYPE_GIANT)
		{
			setObj = exerciseSetItem(exercise_idx, i);
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
				exerciseSetItem(exercise_idx, set_number)->setProperty("setType", new_type);
				return;
			}
		}

		m_setObjectProperties.insert(u"copyTypeButtonValue"_qs, exerciseSetItem(exercise_idx, set_number)->property("copyTypeButtonValue").toString());
		m_setObjectProperties.insert(u"copyTimeButtonValue"_qs, exerciseSetItem(exercise_idx, set_number)->property("copyTimeButtonValue").toString());
		m_setObjectProperties.insert(u"copyRepsButtonValue"_qs, exerciseSetItem(exercise_idx, set_number)->property("copyRepsButtonValue").toString());
		m_setObjectProperties.insert(u"copyWeightButtonValue"_qs, exerciseSetItem(exercise_idx, set_number)->property("copyWeightButtonValue").toString());
		removeExerciseSet(exercise_idx, set_number);

		m_expectedSetNumber = 100; //do not trigger the itemReady signal nor add the object to the parent layout
		connect(this, &QmlItemManager::setObjectReady, this, [this,set_number,exercise_idx] {
			changeSetType(set_number, exercise_idx, 100);
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
	QQuickItem* set_object(exerciseSetItem(exercise_idx, set_number));
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
//-----------------------------------------------------------SLOTS-----------------------------------------------------------


//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------
void QmlItemManager::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent{m_appQmlEngine, QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous};
		if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
			connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
					{ return createExercisesObjects(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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
			exerciseEntryItem(i)->setProperty("setNbr", m_CurrenttDayModel->setsNumber(i));
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

	const int exercise_idx(object_idx >= 0 ? object_idx : m_currentExercises->exerciseObjects.count());
	appendExerciseEntry();

	QString exerciseName, nSets, nReps, nWeight, nRestTime;
	if (appExercisesModel()->selectedEntriesCount() == 1)
	{
		exerciseName = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_qs + appExercisesModel()->selectedEntriesValue_fast(0, 2);
		nSets = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_SETSNUMBER);
		nReps = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER);
		nWeight = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT);
	}
	else
	{
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_qs +
						appExercisesModel()->selectedEntriesValue_fast(0, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_MAINNAME) + u" - "_qs +
						appExercisesModel()->selectedEntriesValue_fast(1, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_SETSNUMBER), nSets, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_SETSNUMBER), nSets, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER), nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_REPSNUMBER), nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT), nWeight, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_WEIGHT), nWeight, comp_exercise_separator);
	}

	bool bTrackRestTime(false), bAutoRestTime(false);
	if (exercise_idx > 1)
	{
		bTrackRestTime = m_CurrenttDayModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_CurrenttDayModel->autoRestTime(exercise_idx-1);
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, SET_TYPE_REGULAR, 0);
	}
	else
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);

	setExerciseSets(exercise_idx, nSets);
	setExerciseReps(exercise_idx, nReps);
	setExerciseWeight(exercise_idx, nWeight);
	setExerciseRestTime(exercise_idx, nRestTime);
	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exerciseCount());
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "updateScreenControls");

	m_tDayExerciseEntryProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_tDayExerciseEntryProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
	m_tDayExerciseEntryProperties.insert(u"exerciseIdx"_qs, exercise_idx);
	m_tDayExerciseEntryProperties.insert(u"nRestTime"_qs, nRestTime);
	m_tDayExerciseEntryProperties.insert(u"bTrackRestTime"_qs, bTrackRestTime);
	m_tDayExerciseEntryProperties.insert(u"bAutoRestTime"_qs, bAutoRestTime);
	m_tDayExerciseEntryProperties.insert(u"bCanEditRestTimeTracking"_qs, true);
	m_tDayExerciseEntryProperties.insert(u"bCompositeExercise"_qs, m_CurrenttDayModel->compositeExercise(m_CurrenttDayModel->exerciseCount()-1));

	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_appQmlEngine->rootContext())));
	m_appQmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	setExerciseItem(exercise_idx, item);
	QQuickItem* parentLayout(m_currenttDayPage->findChild<QQuickItem*>(u"tDayExercisesLayout"_qs));
	item->setParentItem(parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
	connect( item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)) );
	connect( item, SIGNAL(showRemoveExerciseMessage(int)), this, SLOT(showRemoveExerciseMessage(int)) );

	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
}

inline uint QmlItemManager::exercisesCount() const
{
	return m_currentExercises->exerciseObjects.count();
}

inline QQuickItem* QmlItemManager::exerciseEntryItem(const uint exercise_idx)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry;
}

inline QQuickItem* QmlItemManager::exerciseEntryItem(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry;
}

inline QQuickItem* QmlItemManager::exerciseSetItem(const uint exercise_idx, const uint set_number)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number);
}

inline QQuickItem* QmlItemManager::exerciseSetItem(const uint exercise_idx, const uint set_number) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number);
}

inline void QmlItemManager::appendExerciseEntry()
{
	tDayExercises::exerciseObject* exerciseObj{new tDayExercises::exerciseObject};
	m_currentExercises->exerciseObjects.append(exerciseObj);
}

void QmlItemManager::removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow)
{
	tDayExercises::exerciseObject* exerciseObj(m_currentExercises->exerciseObjects.at(exercise_idx));
	for (uint x(0); x < exerciseObj->m_setObjects.count(); ++x)
		bDeleteNow ? delete exerciseObj->m_setObjects.at(x) : exerciseObj->m_setObjects.at(x)->deleteLater();
	exerciseObj->m_setObjects.clear();
	if (exerciseObj->m_setTimer)
		bDeleteNow ? delete exerciseObj->m_setTimer : exerciseObj->m_setTimer->deleteLater();
	bDeleteNow ? delete exerciseObj->m_exerciseEntry : exerciseObj->m_exerciseEntry->deleteLater();
	m_currentExercises->exerciseObjects.removeAt(exercise_idx);
	delete exerciseObj;
}

inline void QmlItemManager::setExerciseItem(const uint exercise_idx, QQuickItem* new_exerciseItem)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry = new_exerciseItem;
}

inline const QString& QmlItemManager::exerciseReps(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nReps;
}

inline void QmlItemManager::setExerciseReps(const uint exercise_idx, const QString& nreps)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nReps = nreps;
}

inline const QString& QmlItemManager::exerciseWeights(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight;
}

inline void QmlItemManager::setExerciseWeight(const uint exercise_idx, const QString& nweight)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight = nweight;
}

inline void QmlItemManager::insertExerciseSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.insert(set_number, new_setObject);
}

inline void QmlItemManager::appendExerciseSet(const uint exercise_idx, QQuickItem* new_setObject)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.append(new_setObject);
}

inline void QmlItemManager::removeExerciseSet(const uint exercise_idx, const uint set_number)
{
	exerciseSetItem(exercise_idx, set_number)->deleteLater();
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.remove(set_number);
}

inline void QmlItemManager::clearExerciseEntries(const bool bDeleteNow)
{
	for (int i(m_currentExercises->exerciseObjects.count() - 1); i >= 0 ; --i)
		removeExerciseEntry(i, bDeleteNow);
}

//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------
void QmlItemManager::createSetObject(const uint exercise_idx, const uint set_number, const bool bNewSet, const uint set_type,
										const QString& nReps, const QString& nWeight, const QString& nRestTime)
{
	const uint set_type_cpp(set_type == SET_TYPE_DROP ? 1 : set_type == SET_TYPE_GIANT ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
		m_setComponents[set_type_cpp] = new QQmlComponent(m_appQmlEngine, QUrl(setTypePages[set_type_cpp]), QQmlComponent::Asynchronous);

	if (bNewSet)
	{
		if (set_number == 0)
			currenttDayModel()->newFirstSet(exercise_idx, set_type, nReps, nWeight, nRestTime);
		else
			currenttDayModel()->newSet(set_number, exercise_idx, set_type, nReps, nWeight, nRestTime);
		m_expectedSetNumber = set_number;
	}

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [this,set_type,set_number,exercise_idx,bNewSet](QQmlComponent::Status status)
			{ if (status == QQmlComponent::Ready) return createSetObject_part2(set_type, set_number, exercise_idx, bNewSet); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(set_type, set_number, exercise_idx, bNewSet);
}

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
								createWithInitialProperties(m_setObjectProperties, m_appQmlEngine->rootContext())));
	m_appQmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);

	//Default values for these properties. They are only modified, on the c++ side, in changeSetType().
	m_setObjectProperties.insert(u"copyTypeButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyTimeButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyRepsButtonValue"_qs, QString());
	m_setObjectProperties.insert(u"copyWeightButtonValue"_qs, QString());

	if (set_number >= exerciseSetsCount(exercise_idx))
		appendExerciseSet(exercise_idx, item);
	else
		insertExerciseSet(set_number, exercise_idx, item);

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
		}
		else
		{
			for (uint i(0); i < exerciseSetsCount(exercise_idx)-1; ++i)
				exerciseSetItem(exercise_idx, i)->setProperty("finishButtonVisible", false);
			item->setProperty("finishButtonVisible", true);
		}
	}

	//Sets may be created at any random order, specially when there are set objects of different kinds within an exercise. m_expectedSetNumber keeps
	//track of the order in which the sets are added. When set_number is greater than m_expectedSetNumber, the set objects are not inserted into
	//the parent layout(with setParentItem). When the expected set_number is finally created, put all sets already in the list (m_setObjects)
	//orderly into the layout
	if (set_number <= m_expectedSetNumber)
	{
		QQuickItem* parent(exerciseEntryItem(exercise_idx)->findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));
		for (uint i(set_number); i < exerciseSetsCount(exercise_idx); ++i, ++m_expectedSetNumber)
		{
			if (exerciseSetItem(exercise_idx, i)->property("setNumber").toUInt() <= i)
			{
				exerciseSetItem(exercise_idx, i)->setParentItem(parent);
				emit setObjectReady();
			}
		}
	}

	if (set_type == SET_TYPE_GIANT)
	{
		item->setProperty("ownerExercise", QVariant::fromValue(exerciseEntryItem(exercise_idx)));
		QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
	}
	else if (set_type == SET_TYPE_DROP)
		QMetaObject::invokeMethod(item, "init");
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
	exerciseEntryItem(exercise_idx)->setProperty("bCanEditRestTimeTracking", noSetsCompleted);
}

void QmlItemManager::enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime,
								const uint bAutoRestTime, const uint except_set_number)
{
	const uint nsets(exerciseSetsCount(exercise_idx));
	QString strRestTime;
	for(uint i(1); i < nsets; ++i)
	{
		if (i != except_set_number)
		{
			if (!m_CurrenttDayModel->setCompleted(i, exercise_idx))
			{
				findSetMode(exercise_idx, i);
				exerciseSetItem(exercise_idx, i)->setProperty("bTrackRestTime", bTrackRestTime);
				exerciseSetItem(exercise_idx, i)->setProperty("bAutoRestTime", bAutoRestTime);
				if (bAutoRestTime)
					strRestTime = u"00:00"_qs;
				else if (bTrackRestTime)
					strRestTime = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, m_CurrenttDayModel->setType(i, exercise_idx), i);
				m_CurrenttDayModel->setSetRestTime(i, exercise_idx, strRestTime);
				QMetaObject::invokeMethod(exerciseSetItem(exercise_idx, i), "updateRestTime", Q_ARG(QString, strRestTime));
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
	exerciseSetItem(exercise_idx, set_number)->setProperty("setMode", set_mode);
}

void QmlItemManager::findCurrentSet(const uint exercise_idx, const uint set_number)
{
	QQuickItem* set_object{exerciseSetItem(exercise_idx, set_number)};
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
	QQuickItem* set_object{exerciseSetItem(exercise_idx, set_number)};
	connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer,set_object] () {
		QMetaObject::invokeMethod(set_object, "updateRestTime", Q_ARG(QString, set_timer->strMinutes() + ':' + set_timer->strSeconds()));
	});
	connect(set_timer, &TPTimer::minutesChanged, this, [this,set_timer,set_object] () {
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
