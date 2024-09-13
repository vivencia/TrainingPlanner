#include "qmlitemmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocalendarmodel.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QSettings>

QQmlApplicationEngine* QmlItemManager::app_qml_engine(nullptr);

const QStringList setTypePages(QStringList() << u"qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_qs <<
					u"qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_qs << u"qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_qs);

QmlItemManager::QmlItemManager(const int meso_id, const uint meso_idx, QObject* parent)
	: QObject{parent}, m_mesoId(meso_id), m_mesoIdx(meso_idx),
		m_exercisesComponent(nullptr), m_mesoComponent(nullptr), m_mesoPage(nullptr), m_plannerComponent(nullptr), m_plannerPage(nullptr),
		m_splitComponent(nullptr), m_calComponent(nullptr), m_calPage(nullptr), m_tDayComponent(nullptr), m_tDayExercisesComponent(nullptr),
		m_setComponents{nullptr}, m_settingsComponent(nullptr), m_clientsOrCoachesComponent(nullptr)
{}

QmlItemManager::~QmlItemManager()
{
	if (m_exercisesComponent)
	{
		delete m_exercisesPage;
		delete m_exercisesComponent;
	}
	if (m_settingsComponent)
	{
		delete m_settingsPage;
		delete m_settingsComponent;
	}
	if (m_clientsOrCoachesComponent)
	{
		delete m_clientsOrCoachesPage;
		delete m_clientsOrCoachesComponent;
	}

	if (m_mesoComponent)
	{
		delete m_mesoPage;
		delete m_mesoComponent;
	}
	if (m_calComponent)
	{
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

void QmlItemManager::setQmlEngine(QQmlApplicationEngine* QMlEngine)
{
	if (app_qml_engine)
		return;

	app_qml_engine = QMlEngine;

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
		//updateDB(new DBMesoCalendarTable(m_DBFilePath));
		//updateDB(new DBMesocyclesTable(m_DBFilePath));
		//DBUserTable user(m_DBFilePath);
		//user.removeDBFile();
		appSettings()->setValue("appVersion", TP_APP_VERSION);
	}

	//getAllUsers();
	//getAllMesocycles();

	m_mainWindow = static_cast<QQuickWindow*>(appQmlEngine()->rootObjects().at(0));
	m_appStackView = m_mainWindow->findChild<QQuickItem*>(u"appStackView"_qs);
	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appUtils"), QVariant::fromValue(appUtils()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appTr"), QVariant::fromValue(appTr()) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("userModel"), QVariant::fromValue(userModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("m_ExercisesModel"), QVariant::fromValue(m_ExercisesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("lightIconFolder"), QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("darkIconFolder"), QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor1"), QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor2"), QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mainwindow"), QVariant::fromValue(m_mainWindow) });

	QQuickItem* appStackView(m_mainWindow->findChild<QQuickItem*>(u"appStackView"_qs));
	QQuickItem* contentItem(appStackView->parentItem());
	properties.append(QQmlContext::PropertyPair{ u"windowHeight"_qs, contentItem->height() }); //mainwindow.height - header.height
	properties.append(QQmlContext::PropertyPair{ u"windowWidth"_qs, contentItem->width() });

	appQmlEngine()->rootContext()->setContextProperties(properties);

	QMetaObject::invokeMethod(m_mainWindow, "init", Qt::AutoConnection);
	mAppDataFilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/"_qs;
#ifdef Q_OS_ANDROID
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &RunCommands::appResumed, this, &DBInterface::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(m_mainWindow, "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	appStartUpNotifications();
#endif
}

void QmlItemManager::setMesoId(const int new_mesoid)
{
	m_mesoId = new_mesoid;
	if (m_mesoPage)
		m_mesoPage->setProperty("mesoId", m_mesoId);
}

void QmlItemManager::changeMesoIdxFromPagesAndModels(const uint new_mesoidx)
{
	m_mesoIdx = new_mesoidx;
	if (m_mesoPage)
		m_mesoPage->setProperty("mesoIdx", new_mesoidx);
	QMapIterator<QDate,DBTrainingDayModel*> x(m_tDayModels);
	x.toFront();
	while (x.hasNext()) {
		x.next();
		x.value()->setMesoIdx(new_mesoidx);
	}
	QMapIterator<QChar,DBMesoSplitModel*> z(m_splitModels);
	z.toFront();
	while (z.hasNext()) {
		z.next();
		z.value()->setMesoIdx(new_mesoidx);
	}
}

void QmlItemManager::requestTimerDialog(QQuickItem* requester, const QVariant& args)
{
	const QVariantList strargs(args.toList());
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
}

void QmlItemManager::requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id)
{
	QMetaObject::invokeMethod(id == 0 ? m_plannerPage : m_CurrenttDayPage, "requestSimpleExercisesList",
					Q_ARG(QVariant, QVariant::fromValue(requester)), Q_ARG(QVariant, visible), Q_ARG(QVariant, multipleSelection));
}

void QmlItemManager::requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset)
{
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestFloatingButton", Q_ARG(int, exercise_idx.toInt()),
								Q_ARG(int, set_type.toInt()), Q_ARG(QString, nset.toString()));
}

void QmlItemManager::showRemoveExerciseMessage(int exercise_idx)
{
	QMetaObject::invokeMethod(m_CurrenttDayPage, "showRemoveExerciseMessage", Q_ARG(int, exercise_idx));
}

void QmlItemManager::showRemoveSetMessage(int set_number, int exercise_idx)
{
	QMetaObject::invokeMethod(m_CurrenttDayPage, "showRemoveSetMessage", Q_ARG(int, set_number), Q_ARG(int, exercise_idx));
}

void QmlItemManager::exerciseCompleted(int exercise_idx)
{
	QMetaObject::invokeMethod(m_currentExercises->exerciseEntry_const(exercise_idx), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	if (exercise_idx < m_currentExercises->exercisesCount()-1)
	{
		if (!m_currentExercises->exerciseEntry_const(exercise_idx+1)->property("finishButtonEnabled").toBool())
		{
			QMetaObject::invokeMethod(m_currentExercises->exerciseEntry_const(exercise_idx+1), "paneExerciseShowHide", Q_ARG(bool, true), Q_ARG(bool, true));
			QMetaObject::invokeMethod(m_CurrenttDayPage, "placeSetIntoView", Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx+1)->property("y").toInt() + 50));
		}
	}
}

//-----------------------------------------------------------EXERCISES-----------------------------------------------------------
void QmlItemManager::openExercisesListPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (m_exercisesPage != nullptr)
	{
		m_exercisesPage->setProperty("bChooseButtonEnabled", bChooseButtonEnabled);
		m_ExercisesModel->clearSelectedEntries();
		if (connectPage)
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		return;
	}
	if (m_ExercisesModel->count() == 0)
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_ExercisesModel));
		connect( this, &DBInterface::databaseReady, this, [&,worker,connectPage] (const uint db_id) {
			if (db_id == worker->uniqueID()) return createExercisesListPage(connectPage); });
		createThread(worker, [worker] () { return worker->getAllExercises(); } );
	}

	m_exercisesProperties.insert(u"bChooseButtonEnabled"_qs, bChooseButtonEnabled);
	m_exercisesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [&,connectPage](QQmlComponent::Status) {
		return createExercisesListPage(connectPage); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void QmlItemManager::createExercisesListPage(QQuickItem* connectPage)
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
	if (m_ExercisesModel->isReady() && m_exercisesComponent->status() == QQmlComponent::Ready)
	{
		if (m_exercisesPage == nullptr)
		{
			m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
															m_exercisesProperties, appQmlEngine()->rootContext()));
			appQmlEngine()->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
			m_exercisesPage->setParentItem(m_mainWindow->contentItem());
			if (connectPage)
				connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
			QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		}
	}
}
//-----------------------------------------------------------EXERCISES-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
void QmlItemManager::createMesocyclePage(const QDate& minimumMesoStartDate, const QDate& maximumMesoEndDate, const QDate& calendarStartDate)
{
	m_mesoProperties.insert(QStringLiteral("mesoId"), m_mesoId);
	m_mesoProperties.insert(QStringLiteral("mesoIdx"), m_mesoIdx);
	m_mesoProperties.insert(QStringLiteral("minimumMesoStartDate"), !minimumMesoStartDate.isNull() ? minimumMesoStartDate : m_mesocyclesModel->getPreviousMesoEndDate(m_mesoId));
	m_mesoProperties.insert(QStringLiteral("maximumMesoEndDate"), !maximumMesoEndDate.isNull() ? maximumMesoEndDate : m_mesocyclesModel->getNextMesoStartDate(m_mesoId));
	m_mesoProperties.insert(QStringLiteral("calendarStartDate"), !calendarStartDate.isNull() ? calendarStartDate: m_mesocyclesModel->getDate(m_mesoIdx, 2));

	m_mesoComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoCycle.qml"_qs), QQmlComponent::Asynchronous);
	if (m_mesoComponent->status() != QQmlComponent::Ready)
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return QmlItemManager::createMesocyclePage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesocyclePage_part2();
}

void QmlItemManager::createMesocyclePage_part2()
{
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	#ifdef DEBUG
	if (m_mesoComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_mesoComponent->errorString();
		for (uint i(0); i < m_mesoComponent->errors().count(); ++i)
			qDebug() << m_mesoComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(m_appStackView);
	m_mesoPage->setProperty("useMode", userModel->appUseMode(0));
	addMainMenuShortCut(m_mesocyclesModel->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_mesoPage);
	emit pageReady(m_mesoPage, mesoPageCreateId);
}

void QmlItemManager::getMesoPage()
{
	if (m_mesoPage)
		addMainMenuShortCut(m_mesocyclesModel->getFast(m_mesoIdx, MESOCYCLES_COL_NAME), m_mesoPage);
	else
		createMesocyclePage();
}
//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
void QmlItemManager::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ExercisesPlanner.qml"_qs), QQmlComponent::Asynchronous);

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
	m_plannerPage->setParentItem(m_appStackView);
	emit pageReady(m_plannerPage, exercisesPlannerCreateId);
}

void QmlItemManager::createMesoSplitPage()
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous);

	if (m_splitComponent->status() != QQmlComponent::Ready)
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createMesoSplitPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesoSplitPage_part2();
}

void QmlItemManager::createMesoSplitPage_part2()
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

	int prevMesoId(-2);
	DBMesoSplitModel* splitModel(nullptr);

	QMapIterator<QChar,DBMesoSplitModel*> i(m_splitModels);
	i.toFront();
	while (i.hasNext()) {
		i.next();
		if (m_createdSplits.indexOf(i.key()) == -1)
		{
			splitModel = m_splitModels.value(i.key());
			m_createdSplits.append(i.key());
			m_splitProperties[QStringLiteral("splitModel")] = QVariant::fromValue(splitModel);
			m_splitProperties[QStringLiteral("parentItem")] = QVariant::fromValue(m_plannerPage);
			QQuickItem* item (static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext())));
			appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
			item->setParentItem(m_plannerPage);
			if (splitModel->count() == 0)
			{
				prevMesoId = m_mesocyclesModel->getPreviousMesoId(m_mesoId);
				item->setProperty("prevMesoId", prevMesoId);
			}
			connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&,const QVariant&,int)), this,
					SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
			emit pageReady(item, static_cast<int>(i.key().cell()) - static_cast<int>('A'));
			m_splitPages.insert(i.key(), item);
		}
	}
}

void QmlItemManager::swapPlans(const QString& splitLetter1, const QString& splitLetter2)
{
	m_splitPages.value(splitLetter1.at(0))->setProperty("splitLetter", splitLetter2);
	m_splitPages.value(splitLetter2.at(0))->setProperty("splitLetter", splitLetter1);
	DBMesoSplitModel* tempSplit(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter1.at(0)] = m_splitModels.value(splitLetter2.at(0));
	m_splitModels[splitLetter2.at(0)] = tempSplit;
}

//Updates MesoSplitPlanner(and its corresponding models) with the changes originating in MesoCycle.qml
void QmlItemManager::updateMuscularGroup(DBMesoSplitModel* splitModel)
{
	for(uint i(0); i < 6; ++i)
	{
		if (m_splitModels.value(QChar('A'+i)) != nullptr)
			m_splitModels[QChar('A'+i)]->setMuscularGroup(splitModel->getFast(splitModel->currentRow(), i+2));
	}
}
//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
uint QmlItemManager::createMesoCalendarPage()
{
	m_calComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/MesoCalendar.qml"_qs), QQmlComponent::Asynchronous);
	m_calProperties.insert(QStringLiteral("mesoIdx"), m_mesoIdx);
	m_calProperties.insert(QStringLiteral("mesoCalendarModel"), QVariant::fromValue(m_mesocyclesModel->mesoCalendarModel(m_mesoIdx)));

	if (m_calComponent->status() != QQmlComponent::Ready)
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return createMesoCalendarPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesoCalendarPage_part2();
	return calPageCreateId;
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
	m_calPage->setParentItem(m_appStackView);
	emit pageReady(m_calPage, calPageCreateId);
}
//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
uint QmlItemManager::createTrainingDayPage(const QDate& date)
{
	if (!m_tDayPages.contains(date))
	{
		if (m_tDayComponent == nullptr)
			m_tDayComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/TrainingDayInfo.qml"_qs), QQmlComponent::Asynchronous);

		if (!m_tDayExercisesList.contains(date))
		{
			m_currentExercises = new tDayExercises;
			m_tDayExercisesList.insert(date, m_currentExercises);

			const DBMesoCalendarModel* mesoCal(m_mesocyclesModel->mesoCalendarModel(m_mesoIdx));
			const QString tday(QString::number(mesoCal->getTrainingDay(date.month(), date.day()-1)));
			const QString splitLetter(mesoCal->getSplitLetter(date.month(), date.day()-1));

			//Because TrainingDayInfo.qml uses the model directly, we need to have an working model before the page is created
			if (m_CurrenttDayModel->count() == 0)
			{
				m_CurrenttDayModel->appendRow();
				m_CurrenttDayModel->setMesoId(QString::number(m_mesoId));
				m_CurrenttDayModel->setDate(0, TDAY_COL_DATE, date);
				m_CurrenttDayModel->setSplitLetter(splitLetter);
				m_CurrenttDayModel->setTrainingDay(tday);
				m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
				m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
				m_CurrenttDayModel->setModified(false);
			}
			else
			{
				if (m_CurrenttDayModel->timeOut() != u"--:--"_qs)
					m_CurrenttDayModel->setDayIsFinished(true);
			}

			m_tDayProperties.insert(QStringLiteral("mainDate"), date);
			m_tDayProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
			m_tDayProperties.insert(QStringLiteral("tDay"), tday);
			m_tDayProperties.insert(QStringLiteral("splitLetter"), splitLetter);
			m_tDayProperties.insert(QStringLiteral("timeIn"), m_CurrenttDayModel->timeIn());
			m_tDayProperties.insert(QStringLiteral("timeOut"), m_CurrenttDayModel->timeOut());
		}

		if (m_tDayComponent->status() != QQmlComponent::Ready)
			connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
				{ return createTrainingDayPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		else
			createTrainingDayPage_part2();
	}
	return tDayPageCreateId;
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
	QQuickItem* page(static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(page, QQmlEngine::CppOwnership);
	page->setParentItem(m_appStackView);
	m_CurrenttDayPage = page;
	m_tDayPages.insert(m_tDayModels.key(m_CurrenttDayModel), page);
	emit pageReady(page, tDayPageCreateId);

	connect(m_CurrenttDayModel, &DBTrainingDayModel::exerciseCompleted, this, [&] (const uint exercise_idx, const bool completed) {
							enableDisableExerciseCompletedButton(exercise_idx, completed);
	});

	if (m_CurrenttDayModel->dayIsFinished())
	{
		const QTime workoutLenght(appUtils()->calculateTimeDifference(m_CurrenttDayModel->timeIn(), m_CurrenttDayModel->timeOut()));
		QMetaObject::invokeMethod(m_CurrenttDayPage, "updateTimer", Q_ARG(int, workoutLenght.hour()),
				Q_ARG(int, workoutLenght.minute()), Q_ARG(int, workoutLenght.second()));
	}
}

void QmlItemManager::resetWorkout()
{
	m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
	m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
	m_CurrenttDayModel->setDayIsFinished(false);
	m_CurrenttDayPage->setProperty("timeIn", m_CurrenttDayModel->timeIn());
	m_CurrenttDayPage->setProperty("timeOut", m_CurrenttDayModel->timeOut());
	m_CurrenttDayPage->setProperty("editMode", false);
	QMetaObject::invokeMethod(m_CurrenttDayPage, "resetTimer", Qt::AutoConnection);
}

void QmlItemManager::setCurrenttDay(const QDate& date)
{
	m_CurrenttDayModel = m_tDayModels.value(date);
	m_CurrenttDayPage = m_tDayPages.value(date);
	m_currentExercises = m_tDayExercisesList.value(date);
}

void QmlItemManager::updateOpenTDayPagesWithNewCalendarInfo(const uint meso_idx, const QDate& startDate, const QDate& endDate)
{
	QMapIterator<QDate,QQuickItem*> i(m_tDayPages);
	i.toFront();
	const DBMesoCalendarModel* const mesoCal(m_mesocyclesModel->mesoCalendarModel(meso_idx));
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
					Q_ARG(QString, m_mesocyclesModel->getMuscularGroup(meso_idx, mesoCal->getSplitLetter(startDate.month(), startDate.day()))));
			}
		}
	}
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
uint QmlItemManager::createExerciseObject(DBExercisesModel* exercisesModel)
{
	if (m_tDayExercisesComponent == nullptr)
		m_tDayExercisesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);

	QString exerciseName, nSets, nReps, nWeight, nRestTime;
	if (exercisesModel->selectedEntriesCount() == 1)
	{
		exerciseName = exercisesModel->selectedEntriesValue_fast(0, 1) + u" - "_qs + exercisesModel->selectedEntriesValue_fast(0, 2);
		nSets = exercisesModel->selectedEntriesValue_fast(0, 4);
		nReps = exercisesModel->selectedEntriesValue_fast(0, 5);
		nWeight = exercisesModel->selectedEntriesValue_fast(0, 6);
	}
	else
	{
		appUtils()->setCompositeValue(0, exercisesModel->selectedEntriesValue_fast(0, 1) + u" - "_qs + exercisesModel->selectedEntriesValue_fast(0, 2), exerciseName);
		appUtils()->setCompositeValue(1, exercisesModel->selectedEntriesValue_fast(1, 1) + u" - "_qs + exercisesModel->selectedEntriesValue_fast(1, 2), exerciseName);
		appUtils()->setCompositeValue(0, exercisesModel->selectedEntriesValue_fast(0, 4), nSets);
		appUtils()->setCompositeValue(1, exercisesModel->selectedEntriesValue_fast(1, 4), nSets);
		appUtils()->setCompositeValue(0, exercisesModel->selectedEntriesValue_fast(0, 5), nReps);
		appUtils()->setCompositeValue(1, exercisesModel->selectedEntriesValue_fast(1, 5), nReps);
		appUtils()->setCompositeValue(0, exercisesModel->selectedEntriesValue_fast(0, 6), nWeight);
		appUtils()->setCompositeValue(1, exercisesModel->selectedEntriesValue_fast(1, 6), nWeight);
	}

	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exerciseCount());

	bool bTrackRestTime(false), bAutoRestTime(false);
	const int exercise_idx(m_currentExercises->exercisesCount());

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

	m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nSets"), nSets);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nReps"), nReps);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nWeight"), nWeight);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nRestTime"), nRestTime);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("bTrackRestTime"), bTrackRestTime);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("bAutoRestTime"), bAutoRestTime);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("bCanEditRestTimeTracking"), true);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("bCompositeExercise"), m_CurrenttDayModel->compositeExercise(m_CurrenttDayModel->exerciseCount()-1));

	if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createExerciseObject_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createExerciseObject_part2();
	return tDayExerciseCreateId;
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

	const int idx(object_idx >= 0 ? object_idx : m_currentExercises->exerciseObjects.count());
	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	item->setParentItem(parentLayout);
	item->setProperty("exerciseIdx", idx);
	item->setProperty("Layout.row", idx);
	item->setProperty("Layout.column", 0);
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
	connect( item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)) );
	connect( item, SIGNAL(showRemoveExerciseMessage(int)), this, SLOT(showRemoveExerciseMessage(int)) );

	m_currentExercises->appendExerciseEntry(item);
	emit itemReady(item, tDayExerciseCreateId);
	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
}

void QmlItemManager::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);
		if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
			connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return createExercisesObjects(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		else
			createExercisesObjects();
		QMetaObject::invokeMethod(m_CurrenttDayPage, "createNavButtons", Qt::AutoConnection);
	}
	else
	{
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
		m_CurrenttDayPage->setProperty("bHasMesoPlan", false);
		m_CurrenttDayPage->setProperty("bHasPreviousTDays", false);
		for(uint i(0); i < m_CurrenttDayModel->exerciseCount(); ++i)
		{
			createExerciseObject_part2(i);
			m_currentExercises->exerciseEntry(i)->setProperty("setNbr", m_CurrenttDayModel->setsNumber(i));
		}
	}
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

	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
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
	QMetaObject::invokeMethod(m_CurrenttDayPage, "placeSetIntoView", Q_ARG(int, -100));
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

	m_setObjectProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
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

	m_setObjectProperties.insert(QStringLiteral("exerciseIdx"), exercise_idx);
	m_setObjectProperties.insert(QStringLiteral("setNumber"), set_number);
	m_setObjectProperties.insert(QStringLiteral("setType"), set_type);
	m_setObjectProperties.insert(QStringLiteral("setCompleted"), m_CurrenttDayModel->setCompleted(set_number, exercise_idx));
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
			QMetaObject::invokeMethod(m_CurrenttDayPage, "placeSetIntoView",
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
				emit itemReady(m_currentExercises->setObject(exercise_idx, i), tDaySetCreateId);
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
		QMetaObject::invokeMethod(m_CurrenttDayPage, "placeSetIntoView",
			Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx)->property("y").toInt() + 50));
	}
}

//Convenience function for ExerciseEntry::createSetObject. Create last_set - first_set sets on the fly as per user command
void QmlItemManager::createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
							const QString& nReps, const QString& nWeight, const QString& nRestTime)
{
	if (!nReps.isEmpty())
	{
		connect(this, &QmlItemManager::itemReady, this, [&,set_type,first_set, last_set,exercise_idx](QQuickItem* newSet, uint id)
			{
				if (id == tDaySetCreateId)
				{
					emit itemReady(newSet, id);
					return createSetObjects(exercise_idx, first_set, last_set, set_type);
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
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
		QMetaObject::invokeMethod(m_CurrenttDayPage, "placeSetIntoView",
			Q_ARG(int, m_currentExercises->exerciseEntry(exercise_idx)->property("y").toInt() + 50));
	}
	else
	{
		//Place into view: most recent set added
		QMetaObject::invokeMethod( m_CurrenttDayPage, "placeSetIntoView",
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
		connect(this, &QmlItemManager::itemReady, this, [&,set_number,exercise_idx](QQuickItem*, uint)
			{ return changeSetType(set_number, exercise_idx, 100); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		createSetObject(new_type, set_number, exercise_idx, false);
		return;
	}

	tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	QList<QQuickItem*> set_objs(exercise_obj->m_setObjects);
	QQuickItem* parentLayout(exercise_obj->m_exerciseEntry->findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));

	for(uint x(0); x < set_objs.count(); ++x)
		set_objs[x]->setParentItem(nullptr);
	for(uint x(0); x < set_objs.count(); ++x)
		set_objs[x]->setParentItem(parentLayout);
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

void QmlItemManager::enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	const uint nsets(exercise_obj->m_setObjects.count());
	bool noSetsCompleted(true);
	for (uint i(0); i < nsets; ++i)
	{
		if (exercise_obj->m_setObjects.at(i)->property("finishButtonVisible").toBool())
		{
			exercise_obj->m_setObjects.at(i)->setProperty("finishButtonEnabled", completed);
			break;
		}
		if (exercise_obj->m_setObjects.at(i)->property("setCompleted").toBool())
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
	QQuickItem* set_object(m_currentExercises->setObject(exercise_idx, set_number));
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

void QmlItemManager::startRestTimer(const uint exercise_idx, const uint set_number)
{
	TPTimer* set_timer(m_currentExercises->setTimer(exercise_idx));
	set_timer->setInterval(1000);
	set_timer->setStopWatch(true);
	set_timer->prepareTimer(u"-"_qs);
	enableDisableSetsRestTime(exercise_idx, false, false, set_number); //Prevent the user from starting the timer for another set before finishing this one
	QQuickItem* set_object(m_currentExercises->setObject(exercise_idx, set_number));
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
	TPTimer* set_timer(m_currentExercises->setTimer(exercise_idx));
	if (set_timer->isActive())
	{
		set_timer->stopTimer();
		disconnect(set_timer, nullptr, nullptr, nullptr);
		enableDisableSetsRestTime(exercise_idx, true, true, set_number);
		m_CurrenttDayModel->setSetRestTime(set_number, exercise_idx, set_timer->strMinutes() + ':' + set_timer->strSeconds());
	}
}
//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

void QmlItemManager::tDayExercises::appendExerciseEntry(QQuickItem* new_exerciseItem)
{
	exerciseObject* exerciseObj(new exerciseObject);
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
//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

//-----------------------------------------------------------USER-----------------------------------------------------------
void QmlItemManager::openSettingsPage(const uint startPageIndex)
{
	if (m_settingsPage != nullptr)
	{
		m_settingsPage->setProperty("startPageIndex", startPageIndex);
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
		return;
	}

	m_settingsProperties.insert(QStringLiteral("startPageIndex"), startPageIndex);
	m_settingsComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ConfigurationPage.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_settingsComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
		return createSettingsPage(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void QmlItemManager::createSettingsPage()
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
		m_settingsPage = static_cast<QQuickItem*>(m_settingsComponent->createWithInitialProperties(m_settingsProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(m_settingsPage, QQmlEngine::CppOwnership);
		m_settingsPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_settingsPage));
		m_userPage = m_settingsPage->findChild<QQuickItem*>(u"userPage"_qs);
		m_userPage->setProperty("useMode", userModel->appUseMode(0));
	}
}

void QmlItemManager::openClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches)
{
	setClientsOrCoachesPagesProperties(bManageClients, bManageCoaches);

	if (m_clientsOrCoachesPage != nullptr)
	{
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
		return;
	}

	m_clientsOrCoachesComponent = new QQmlComponent(appQmlEngine(), QUrl(u"qrc:/qml/Pages/ClientsOrCoachesPage.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_clientsOrCoachesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
		return createClientsOrCoachesPage(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void QmlItemManager::createClientsOrCoachesPage()
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
				m_clientsOrCoachesProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(m_clientsOrCoachesPage, QQmlEngine::CppOwnership);
		m_clientsOrCoachesPage->setParentItem(m_mainWindow->contentItem());
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_clientsOrCoachesPage));
	}
}

void QmlItemManager::setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches)
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
//-----------------------------------------------------------USER-----------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
void QmlItemManager::addMainMenuShortCut(const QString& label, QQuickItem* page)
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

void QmlItemManager::removeMainMenuShortCut(QQuickItem* page)
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

void QmlItemManager::openMainMenuShortCut(const int button_id)
{
	QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_mainMenuShortcutPages.at(button_id)));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

