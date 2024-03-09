#include "tpmesocycleclass.h"
#include "dbmesocyclesmodel.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

static const uint mesoPageCreateId(175);
static const uint calPageCreateId(35);
static const uint tDayCreateId(70);
static const uint tDayExerciseCreateId(105);
static const uint tDaySetCreateId(140);

static const QStringList setTypePages(QStringList() << QStringLiteral("qrc:/qml/SetTypeRegular.qml") << QStringLiteral("qrc:/qml/SetTypePyramid.qml") <<
				QStringLiteral("qrc:/qml/SetTypeDrop.qml") << QStringLiteral("qrc:/qml/SetTypeCluster.qml") <<
				QStringLiteral("qrc:/qml/SetTypeGiant.qml") << QStringLiteral("qrc:/qml/SetTypeMyoReps.qml"));

TPMesocycleClass::TPMesocycleClass(const int meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent)
	: QObject{parent}, m_MesoId(meso_id), m_MesoIdx(meso_idx), m_QMlEngine(QMlEngine), m_splitComponent(nullptr),
		m_mesosCalendarModel(nullptr), m_calComponent(nullptr), m_tDayComponent(nullptr)
{}

TPMesocycleClass::~TPMesocycleClass()
{
	if (m_splitComponent) delete m_splitComponent;
	if (m_mesosCalendarModel) delete m_mesosCalendarModel;
	if (m_calComponent) delete m_calComponent;
	if (m_tDayComponent) delete m_tDayComponent;
	if (m_mesoComponent) delete m_mesoComponent;
}

void TPMesocycleClass::requestTimerDialog(QQuickItem* requester, const QVariant& args)
{
	const QVariantList strargs(args.toList());
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
}

void TPMesocycleClass::requestExercisesList(QQuickItem* requester, const QVariant& visible, int id)
{
	QMetaObject::invokeMethod(id == 0 ? m_qmlSplitObjectParent : m_CurrenttDayPage, "requestSimpleExerciseList",
					Q_ARG(QVariant, QVariant::fromValue(requester)), Q_ARG(QVariant, visible));
}

//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
void TPMesocycleClass::createMesocyclePage(const QDate& minimumMesoStartDate, const QDate& maximumMesoEndDate, const QDate& calendarStartDate)
{
	if (!m_mesoProperties.isEmpty())
		return;

	m_mesoProperties.insert(QStringLiteral("mesoId"), m_MesoId);
	m_mesoProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
	m_mesoProperties.insert(QStringLiteral("mesoStartDate"), m_MesocyclesModel->getDate(m_MesoIdx, 2));
	m_mesoProperties.insert(QStringLiteral("mesoEndDate"), m_MesocyclesModel->getDate(m_MesoIdx, 3));
	m_mesoProperties.insert(QStringLiteral("minimumMesoStartDate"), !minimumMesoStartDate.isNull() ? minimumMesoStartDate : m_MesocyclesModel->getPreviousMesoEndDate(m_MesoId));
	m_mesoProperties.insert(QStringLiteral("maximumMesoEndDate"), !maximumMesoEndDate.isNull() ? maximumMesoEndDate : m_MesocyclesModel->getNextMesoStartDate(m_MesoId));
	m_mesoProperties.insert(QStringLiteral("calendarStartDate"), !calendarStartDate.isNull() ? calendarStartDate: m_MesocyclesModel->getDate(m_MesoIdx, 2));

	const bool bRealMeso(m_MesocyclesModel->getInt(m_MesoIdx, 8) == 1);
	if (m_mesoComponent == nullptr)
	{
		m_mesoComponent = new QQmlComponent(m_QMlEngine, bRealMeso? QUrl(u"qrc:/qml/MesoCycle.qml"_qs) : QUrl(u"qrc:/qml/OpenEndedPlan.qml"_qs),
					QQmlComponent::Asynchronous);
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) { return TPMesocycleClass::createMesocyclePage_part2(); } );
	}
}

void TPMesocycleClass::createMesocyclePage_part2()
{
	m_MesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, m_QMlEngine->rootContext()));
	#ifdef DEBUG
	if (m_mesoComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_mesoComponent->errorString();
		for (uint i(0); i < m_mesoComponent->errors().count(); ++i)
			qDebug() << m_mesoComponent->errors().at(i).description();
		return;
	}
	#endif
	m_QMlEngine->setObjectOwnership(m_MesoPage, QQmlEngine::CppOwnership);
	QQuickItem* parent(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("appStackView")));
	m_MesoPage->setParentItem(parent);
	emit itemReady(m_MesoPage, mesoPageCreateId);
}
//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
void TPMesocycleClass::createMesoSplitPage()
{
	if (m_splitComponent == nullptr)
	{
		m_qmlSplitObjectParent = m_MesoPage->findChild<QQuickItem*>(QStringLiteral("exercisesPlanner"));
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
				connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,int)) );
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
		m_calProperties.insert(QStringLiteral("mesoCalendarModel"), QVariant::fromValue(m_mesosCalendarModel));
		m_calComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoContent.qml"_qs), QQmlComponent::Asynchronous);
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return createMesoCalendarPage_part2(); } );
	}
	else

	return calPageCreateId;
}

void TPMesocycleClass::createMesoCalendarPage_part2()
{
	if (m_mesosCalendarModel->isReady())
	{
		m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, m_QMlEngine->rootContext()));
		#ifdef DEBUG
		if (m_calComponent->status() == QQmlComponent::Error)
		{
			qDebug() << m_calComponent->errorString();
			for (uint i(0); i < m_calComponent->errors().count(); ++i)
				qDebug() << m_calComponent->errors().at(i).description();
			return;
		}
		#endif
		m_QMlEngine->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
		QQuickItem* parent(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("appStackView")));
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
			connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) { return createTrainingDayPage_part2(); } );
		}
		else
			createTrainingDayPage_part2();
	}
	return tDayCreateId;
}

void TPMesocycleClass::createTrainingDayPage_part2()
{
	if (m_tDayModels.last()->isReady())
	{
		QQuickItem* item (static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_QMlEngine->rootContext())));
		#ifdef DEBUG
		if (m_tDayComponent->status() == QQmlComponent::Error)
		{
			qDebug() << m_tDayComponent->errorString();
			for (uint i(0); i < m_tDayComponent->errors().count(); ++i)
				qDebug() << m_tDayComponent->errors().at(i).description();
			return;
		}
		#endif
		m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
		QQuickItem* parent(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("appStackView")));
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
	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exercisesNumber());
	if (m_tDayExercisesComponent == nullptr)
	{
		QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) { return createExerciseObject_part2(); } );
	}
	else
		createExerciseObject_part2();
	return tDayExerciseCreateId;
}

void TPMesocycleClass::createExerciseObject_part2(const int object_idx)
{
	m_tDayExerciseEntryProperties.insert(QStringLiteral("exerciseIdx"), object_idx >= 0 ? object_idx : m_tDayExercises.count());
	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_QMlEngine->rootContext())));
	#ifdef DEBUG
	if (m_tDayExercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayExercisesComponent->errorString();
		for (uint i(0); i < m_tDayExercisesComponent->errors().count(); ++i)
			qDebug() << m_tDayExercisesComponent->errors().at(i).description();
		return;
	}
	#endif

	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	item->setParentItem(parentLayout);
	connect( item, SIGNAL(requestTimerDialogSignal(QQuickItem*,const QVariant&)), this,
						SLOT(requestTimerDialog(QQuickItem*,const QVariant&)) );
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&,const uint)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const uint)) );
	m_tDayExercises.append(item);
	emit itemReady(item, tDayExerciseCreateId);
}

void TPMesocycleClass::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) { return createExercisesObjects(); } );
	}
	else {
		for(uint i(0); i < m_CurrenttDayModel->exercisesNumber(); ++i)
		{
			createExerciseObject_part2(i);
			for (uint x(0); x < m_CurrenttDayModel->setsNumber(i); ++x)
				createSetObject(m_CurrenttDayModel->setType(x, i), x, i);
		}
	}
}

void TPMesocycleClass::removeExercise(const uint exercise_idx)
{
	if (exercise_idx < m_tDayExercises.count())
	{
		for(uint i(exercise_idx+1); i < m_tDayExercises.count(); ++i)
		{
			static_cast<QQuickItem*>(m_tDayExercises[i])->setProperty("exerciseIdx", i-1);
			for(uint x(0); x < m_setObjects.value(i).count(); ++x)
				m_setObjects[i][x]->setProperty("exerciseIdx", i-1);
		}
		m_CurrenttDayModel->removeExercise(exercise_idx);
		delete m_tDayExercises[exercise_idx];
		m_tDayExercises.remove(exercise_idx);
	}
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
void TPMesocycleClass::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx)
{
	m_setObjectProperties.insert(QStringLiteral("exerciseIdx"), exercise_idx);
	m_setObjectProperties.insert(QStringLiteral("setNumber"), set_number);

	if (!m_setObjects.contains(exercise_idx))
	{
		uint i(0);
		QList<QQuickItem*> lst;
		for(i = 0; i < m_CurrenttDayModel->setsNumber(exercise_idx); ++i)
			lst.append(nullptr);
		m_setObjects[exercise_idx] = lst;
		m_setCounter.append(i);
	}

	if (m_setComponents[set_type] == nullptr)
	{
		m_setObjectProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
		QQuickItem* parentLayout(static_cast<QQuickItem*>(m_tDayExercises.at(exercise_idx))->
						findChild<QQuickItem*>(QStringLiteral("exerciseSetsLayout")));

		m_setComponents[set_type] = new QQmlComponent(m_QMlEngine, QUrl(setTypePages[set_type]), QQmlComponent::Asynchronous, parentLayout);
		connect(m_setComponents[set_type], &QQmlComponent::statusChanged, this, [&,set_type,set_number,exercise_idx](QQmlComponent::Status)
					{ return createSetObject_part2(set_type, set_number, exercise_idx); });
	}
	else
		createSetObject_part2(set_type, set_number, exercise_idx);
}

void TPMesocycleClass::createSetObject_part2(const uint set_type, const uint set_number, const uint exercise_idx)
{
	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type]->
								createWithInitialProperties(m_setObjectProperties, m_QMlEngine->rootContext())));
	#ifdef DEBUG
	if (m_setComponents[set_type]->status() == QQmlComponent::Error)
	{
		qDebug() << m_setComponents[set_type]->errorString();
		for (uint i(0); i < m_setComponents[set_type]->errors().count(); ++i)
			qDebug() << m_setComponents[set_type]->errors().at(i).description();
		return;
	}
	#endif

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

void TPMesocycleClass::removeSet(const uint set_number, const uint exercise_idx)
{
	if (exercise_idx < m_tDayExercises.count())
	{
		for(uint x(set_number+1); x < m_setObjects.value(exercise_idx).count(); ++x)
			m_setObjects[exercise_idx][x]->setProperty("set_number", x-1);
		m_CurrenttDayModel->removeSet(set_number, exercise_idx);
		delete m_setObjects[exercise_idx][set_number];
		m_setObjects.remove(set_number);
	}
}
//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
