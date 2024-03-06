#include "tpmesocycleclass.h"

#include "dbmesosplitmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaymodel.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

static const uint calPageCreateId(35);
static const uint tDayCreateId(70);
static const uint tDayExerciseCreateId(105);
static const uint tDaySetCreateId(140);

static const QStringList setTypePages(QStringList() << QStringLiteral("qrc:/qml/SetTypeRegular.qml") << QStringLiteral("qrc:/qml/SetTypePyramid.qml") <<
				QStringLiteral("qrc:/qml/SetTypeDrop.qml") << QStringLiteral("qrc:/qml/SetTypeCluster.qml") <<
				QStringLiteral("qrc:/qml/SetTypeGiant.qml") << QStringLiteral("qrc:/qml/SetTypeMyoReps.qml"));

TPMesocycleClass::TPMesocycleClass(const uint meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent)
	: QObject{parent}, m_MesoId(meso_id), m_mesoIdx(meso_idx), m_QMlEngine(QMlEngine), m_splitComponent(nullptr),
		m_mesosCalendarModel(nullptr), m_calComponent(nullptr), m_tDayComponent(nullptr)
{}

TPMesocycleClass::~TPMesocycleClass()
{
	if (m_splitComponent) delete m_splitComponent;
	if (m_mesosCalendarModel) delete m_mesosCalendarModel;
	if (m_calComponent) delete m_calComponent;
	if (m_tDayComponent) delete m_tDayComponent;
}

void TPMesocycleClass::requestTimerDialog(QQuickItem* requester, const QVariant& args)
{
	const QVariantList strargs(args.toList());
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
}

void TPMesocycleClass::requestExercisesList(QQuickItem* requester, const QVariant& visible)
{
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestSimpleExerciseList",
					Q_ARG(QVariant, QVariant::fromValue(requester)), Q_ARG(QVariant, visible));
}

//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
void TPMesocycleClass::createMesoSplitPage()
{
	if (m_splitComponent == nullptr)
	{
		m_qmlSplitObjectParent = m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("exercisesPlanner"));
		m_qmlSplitObjectContainer = m_qmlSplitObjectParent->findChild<QQuickItem*>(QStringLiteral("splitSwipeView"));
		m_splitProperties.insert(QStringLiteral("mesoId"), m_MesoId);
		m_splitProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
		m_splitProperties.insert(QStringLiteral("parentItem"), QVariant::fromValue(m_qmlSplitObjectParent));
		m_splitProperties.insert(QStringLiteral("splitModel"), QVariant());

		m_splitComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous);
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
				return TPMesocycleClass::createMesoSplitPage_part2(); } );
	}
}

void TPMesocycleClass::createMesoSplitPage_part2()
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

	QMapIterator<QChar,DBMesoSplitModel*> i(m_splitModels);
	i.toFront();
	while (i.hasNext()) {
		i.next();
		if (i.value()->isReady())
		{
			if (m_createdSplits.indexOf(i.key()) == -1)
			{
				m_createdSplits.append(i.key());
				m_splitProperties[QStringLiteral("splitLetter")] = QString(i.key());
				m_splitProperties[QStringLiteral("splitModel")] = QVariant::fromValue(m_splitModels.value(i.key()));
				QQuickItem* item (static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, m_QMlEngine->rootContext())));
				m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
				item->setParentItem(m_qmlSplitObjectParent);
				connect(item, SIGNAL(requestExercisesPaneAction(int,QVariant,QQuickItem*)), this, SLOT(receiveQMLSignal(int,QVariant,QQuickItem*)) );
				QMetaObject::invokeMethod(m_qmlSplitObjectContainer, "insertItem", Q_ARG(int, static_cast<int>(i.key().cell()) - static_cast<int>('A')),
					Q_ARG(QQuickItem*, item));
				m_splitPages.insert(i.key(), item);
			}
		}
	}
}
//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
uint TPMesocycleClass::createMesoCalendarPage()
{
	if (m_calComponent == nullptr)
	{
		m_calProperties.insert(QStringLiteral("mesoId"), m_MesoId);
		m_calProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
		m_calComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoContent.qml"_qs), QQmlComponent::Asynchronous);
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return TPMesocycleClass::createMesoCalendarPage_part2(); } );
	}
	return calPageCreateId;
}

void TPMesocycleClass::createMesoCalendarPage_part2()
{
	#ifdef DEBUG
	if (m_calComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_calComponent->errorString();
		for (uint i(0); i < m_calComponent->errors().count(); ++i)
			qDebug() << m_calComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_mesosCalendarModel->isReady())
	{
		m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, m_QMlEngine->rootContext()));
		m_QMlEngine->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
		QQuickWindow* parent(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("mainWindow")));
		m_calPage->setParentItem(parent);
		emit itemReady(m_calPage, calPageCreateId);
	}
}
//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
uint TPMesocycleClass::createTrainingDayPage(const QDate& date)
{
	if (!m_tDayPages.contains(date))
	{
		m_tDayProperties.insert(QStringLiteral("mainDate"), date);
		m_tDayProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(gettDayModel(date)));

		if (m_tDayComponent == nullptr)
		{
			m_tDayProperties.insert(QStringLiteral("mesoId"), m_MesoId);
			m_tDayProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);

			m_tDayComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/TrainingDayInfo.qml"_qs), QQmlComponent::Asynchronous);
			connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return DbManager::createTrainingDayPage_part2(); } );
		}
	}
	return tDayCreateId;
}

void TPMesocycleClass::createTrainingDayPage_part2()
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

	if (m_tDayModels.last()->isReady())
	{
		QQuickItem* item (static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_QMlEngine->rootContext())));
		m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
		QQuickWindow* parent(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("mainWindow")));
		item->setParentItem(parent);
		m_CurrenttDayPage = item;
		m_CurrenttDayModel = m_tDayModels.last();
		m_tDayPages.insert(m_tDayModels.key(m_CurrenttDayModel), item);
		emit itemReady(item, tDayCreateId);
	}
}

//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
uint TPMesocycleClass::createExerciseObject(const QString& exerciseName)
{
	if (m_tDayExercisesComponent == nullptr)
	{
		QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) { return createExerciseObject_part2(); } );
	}
	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exercisesNumber());
	return tDayExerciseCreateId;
}

void TPMesocycleClass::createExerciseObject_part2(const int object_idx)
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

	m_tDayExerciseEntryProperties.insert(QStringLiteral("thisObjectIdx"), object_idx >= 0 ? object_idx : m_tDayExercises.count());
	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_QMlEngine->rootContext())));
	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	item->setParentItem(parentLayout);
	connect( item, SIGNAL(requestTimerDialogSignal(QQuickItem*,const QVariant&)), this,
						SLOT(requestTimerDialog(QQuickItem*,const QVariant&)) );
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&)) );
	m_tDayExercises.append(item);
	emit itemReady(item, tDayExerciseCreateId);
}

void TPMesocycleClass::createExercisesObjects(DBTrainingDayModel* model)
{
	if (m_tDayExercisesComponent == nullptr)
	{
		QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&,model](QQmlComponent::Status)
					{ return createExercisesObjects(model); } );
	}
	else {
		for(uint i(0); i < model->exercisesNumber(); ++i)
		{
			createExerciseObject_part2(i);
			for (uint x(0); x < model->setsNumber(i); ++x)
				createSetObject(model->setType(x, i), x, i, model);
		}
	}
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
void TPMesocycleClass::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, DBTrainingDayModel* model)
{
	if (m_setComponents[set_type] == nullptr)
	{
		QQuickItem* parentLayout(static_cast<QQuickItem*>(m_tDayExercises.at(exercise_idx))->
						findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));

		m_setObjectProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(model));
		m_setObjectProperties.insert(QStringLiteral("exerciseIdx"), exercise_idx);
		m_setObjectProperties.insert(QStringLiteral("setNumber"), set_number);

		uint i(0);
		QList<QQuickItem*> lst;
		for(i = 0; i < model->setsNumber(exercise_idx); ++i)
			lst.append(nullptr);
		m_setObjects[exercise_idx] = lst;
		m_setCounter.append(i);

		m_setComponents[set_type] = new QQmlComponent(m_QMlEngine, QUrl(setTypePages[set_type]), QQmlComponent::Asynchronous, parentLayout);
		connect(m_setComponents[set_type], &QQmlComponent::statusChanged, this, [&,set_type,set_number,exercise_idx](QQmlComponent::Status)
					{ return TPMesocycleClass::createSetObject_part2(set_type,set_number,exercise_idx); });
	}
}

void TPMesocycleClass::createSetObject_part2(const uint set_type, const uint set_number, const uint exercise_idx)
{
	#ifdef DEBUG
	if (m_setComponents[set_type]->status() == QQmlComponent::Error)
	{
		qDebug() << m_setComponents[set_type]->errorString();
		for (uint i(0); i < m_setComponents[set_type]->errors().count(); ++i)
			qDebug() << m_setComponents[set_type]->errors().at(i).description();
		return;
	}
	#endif
	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type]->
								createWithInitialProperties(m_setObjectProperties, m_QMlEngine->rootContext())));
	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parent(static_cast<QQuickItem*>(m_tDayExercises.at(exercise_idx))->
								findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));
	item->setParentItem(parent);
	m_setObjects[exercise_idx][set_number] = item;
	if (--m_setCounter[exercise_idx] == 0)
	{
		const uint n(m_setObjects.value(exercise_idx).count());
		for (uint i(0); i < n; ++i) //notify object creation orderly
			emit itemReady(m_setObjects.value(exercise_idx).at(i), tDaySetCreateId);
	}
}
//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
