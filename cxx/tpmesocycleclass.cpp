#include "tpmesocycleclass.h"
#include "dbmesocyclesmodel.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

const QStringList setTypePages(QStringList() << u"qrc:/qml/SetTypeRegular.qml"_qs <<
					u"qrc:/qml/SetTypeDrop.qml"_qs << u"qrc:/qml/SetTypeGiant.qml"_qs);

TPMesocycleClass::TPMesocycleClass(const int meso_id, const uint meso_idx, QQmlApplicationEngine* QMlEngine, QObject *parent)
	: QObject{parent}, m_MesoId(meso_id), m_MesoIdx(meso_idx), m_QMlEngine(QMlEngine), m_mesoComponent(nullptr), m_MesoPage(nullptr),
		m_splitComponent(nullptr), m_mesosCalendarModel(nullptr), m_calComponent(nullptr), m_calPage(nullptr), m_tDayComponent(nullptr),
		m_tDayExercisesComponent(nullptr), m_setComponents{nullptr}, m_mainMenuShortcutComponent(nullptr)
{}

TPMesocycleClass::~TPMesocycleClass()
{
	if (!m_tDayExercisesList.isEmpty())
	{
		QMap<QDate,tDayExercises*>::iterator itr(m_tDayExercisesList.begin());
		QMap<QDate,tDayExercises*>::iterator itr_end(m_tDayExercisesList.end());
		do
		{
			delete *itr;
		} while (++itr != itr_end);
	}
	if (m_splitComponent) delete m_splitComponent;
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

void TPMesocycleClass::requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id)
{
	QMetaObject::invokeMethod(id == 0 ? m_qmlSplitObjectParent : m_CurrenttDayPage, "requestSimpleExercisesList",
					Q_ARG(QVariant, QVariant::fromValue(requester)), Q_ARG(QVariant, visible), Q_ARG(QVariant, multipleSelection));
}

void TPMesocycleClass::requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset)
{
	QMetaObject::invokeMethod(m_CurrenttDayPage, "requestFloatingButton", Q_ARG(int, exercise_idx.toInt()),
								Q_ARG(int, set_type.toInt()), Q_ARG(QString, nset.toString()));
}

//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
void TPMesocycleClass::createMesocyclePage(const QDate& minimumMesoStartDate, const QDate& maximumMesoEndDate, const QDate& calendarStartDate)
{
	m_mesoProperties.insert(QStringLiteral("mesoId"), m_MesoId);
	m_mesoProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
	m_mesoProperties.insert(QStringLiteral("mesoStartDate"), m_MesocyclesModel->getDate(m_MesoIdx, 2));
	m_mesoProperties.insert(QStringLiteral("mesoEndDate"), m_MesocyclesModel->getDate(m_MesoIdx, 3));
	m_mesoProperties.insert(QStringLiteral("minimumMesoStartDate"), !minimumMesoStartDate.isNull() ? minimumMesoStartDate : m_MesocyclesModel->getPreviousMesoEndDate(m_MesoId));
	m_mesoProperties.insert(QStringLiteral("maximumMesoEndDate"), !maximumMesoEndDate.isNull() ? maximumMesoEndDate : m_MesocyclesModel->getNextMesoStartDate(m_MesoId));
	m_mesoProperties.insert(QStringLiteral("calendarStartDate"), !calendarStartDate.isNull() ? calendarStartDate: m_MesocyclesModel->getDate(m_MesoIdx, 2));

	const bool bRealMeso(m_MesocyclesModel->getInt(m_MesoIdx, 8) == 1);
	if (m_mesoComponent == nullptr)
		m_mesoComponent = new QQmlComponent(m_QMlEngine, bRealMeso? QUrl(u"qrc:/qml/MesoCycle.qml"_qs) : QUrl(u"qrc:/qml/OpenEndedPlan.qml"_qs),
					QQmlComponent::Asynchronous);

	if (m_mesoComponent->status() != QQmlComponent::Ready)
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return TPMesocycleClass::createMesocyclePage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesocyclePage_part2();
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

	//m_appStackView is set for the first and only time here
	m_appStackView = m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(u"appStackView"_qs);
	m_MesoPage->setParentItem(m_appStackView);
	//emit pageReady(m_MesoPage, mesoPageCreateId);
	addMainMenuShortCut(m_MesocyclesModel->getFast(m_MesoIdx, 1), m_MesoPage);
}
//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
void TPMesocycleClass::createMesoSplitPage()
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous);

	m_qmlSplitObjectParent = m_MesoPage->findChild<QQuickItem*>(QStringLiteral("exercisesPlanner"));
	m_qmlSplitObjectContainer = m_qmlSplitObjectParent->findChild<QQuickItem*>(QStringLiteral("splitSwipeView"));
	m_splitProperties.insert(QStringLiteral("mesoId"), m_MesoId);
	m_splitProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
	m_splitProperties.insert(QStringLiteral("parentItem"), QVariant::fromValue(m_qmlSplitObjectParent));
	if (m_splitComponent->status() != QQmlComponent::Ready)
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createMesoSplitPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesoSplitPage_part2();
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
				connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*, const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
				QMetaObject::invokeMethod(m_qmlSplitObjectContainer, "insertItem", Q_ARG(int, static_cast<int>(i.key().cell()) - static_cast<int>('A')),
					Q_ARG(QQuickItem*, item));
				m_splitPages.insert(i.key(), item);
			}
		}
	}
}

void TPMesocycleClass::pushSplitPage(const QChar& splitLetter) const
{
	QMetaObject::invokeMethod(m_qmlSplitObjectContainer, "insertItem",
					Q_ARG(int, static_cast<int>(splitLetter.toLatin1()) - static_cast<int>('A')),
					Q_ARG(QQuickItem*, m_splitPages.value(splitLetter)));
}

void TPMesocycleClass::swapPlans(const QString& splitLetter1, const QString& splitLetter2)
{
	m_splitPages.value(splitLetter1.at(0))->setProperty("splitLetter", splitLetter2);
	m_splitPages.value(splitLetter2.at(0))->setProperty("splitLetter", splitLetter1);
	DBMesoSplitModel* tempSplit(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter1.at(0)] = m_splitModels.value(splitLetter2.at(0));
	m_splitModels[splitLetter2.at(0)] = tempSplit;
}
//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
uint TPMesocycleClass::createMesoCalendarPage()
{
	if (m_calComponent == nullptr)
		m_calComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoContent.qml"_qs), QQmlComponent::Asynchronous);

	m_calProperties.insert(QStringLiteral("mesoId"), m_MesoId);
	m_calProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
	m_calProperties.insert(QStringLiteral("mesoCalendarModel"), QVariant::fromValue(m_mesosCalendarModel));

	if (m_calComponent->status() != QQmlComponent::Ready)
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
					{ return createMesoCalendarPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMesoCalendarPage_part2();
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
		m_calPage->setParentItem(m_appStackView);
		emit pageReady(m_calPage, calPageCreateId);
	}
}
//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
uint TPMesocycleClass::createTrainingDayPage(const QDate& date, DBMesoCalendarModel* mesoCal)
{
	if (!m_tDayPages.contains(date))
	{
		if (m_tDayComponent == nullptr)
			m_tDayComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/TrainingDayInfo.qml"_qs), QQmlComponent::Asynchronous);

		if (!m_tDayExercisesList.contains(date))
		{
			m_currentExercises = new tDayExercises;
			m_tDayExercisesList.insert(date, m_currentExercises);

			const QString tday(QString::number(mesoCal->getTrainingDay(date.month(), date.day()-1)));
			const QString splitLetter(mesoCal->getSplitLetter(date.month(), date.day()-1));
			//Because TrainingDayInfo.qml now uses the model directly, we need to have an working model before the page is created
			if (m_CurrenttDayModel->count() == 0)
			{
				m_CurrenttDayModel->appendRow();
				m_CurrenttDayModel->setMesoId(QString::number(m_MesoId));
				m_CurrenttDayModel->setDate(date);
				m_CurrenttDayModel->setSplitLetter(splitLetter);
				m_CurrenttDayModel->setTrainingDay(tday);
				m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
				m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
			}
			m_tDayProperties.insert(QStringLiteral("mainDate"), date);
			m_tDayProperties.insert(QStringLiteral("mesoId"), m_MesoId);
			m_tDayProperties.insert(QStringLiteral("mesoIdx"), m_MesoIdx);
			m_tDayProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
			m_tDayProperties.insert(QStringLiteral("tDay"), tday);
			m_tDayProperties.insert(QStringLiteral("splitLetter"), splitLetter);
			m_tDayProperties.insert(QStringLiteral("bDayIsFinished"), m_CurrenttDayModel->timeOut() != u"--:--"_qs);
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
	QQuickItem* page(static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_QMlEngine->rootContext())));
	m_QMlEngine->setObjectOwnership(page, QQmlEngine::CppOwnership);
	page->setParentItem(m_appStackView);
	m_CurrenttDayPage = page;
	m_tDayPages.insert(m_tDayModels.key(m_CurrenttDayModel), page);
	emit pageReady(page, tDayPageCreateId);
}

//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
uint TPMesocycleClass::createExerciseObject(const QString& exerciseName, const QString& nSets, const QString& nReps, const QString& nWeight)
{
	if (m_tDayExercisesComponent == nullptr)
		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);

	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exerciseCount());
	m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nSets"), nSets);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nReps"), nReps);
	m_tDayExerciseEntryProperties.insert(QStringLiteral("nWeight"), nWeight);
	if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createExerciseObject_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createExerciseObject_part2();
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

	const int idx(object_idx >= 0 ? object_idx : m_currentExercises->exerciseObjects.count());
	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_QMlEngine->rootContext())));
	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	item->setParentItem(parentLayout);
	item->setProperty("exerciseIdx", idx);
	item->setProperty("Layout.row", idx);
	item->setProperty("Layout.column", 0);
	item->setObjectName("exercise_" + QString::number(idx));
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
	connect( item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)) );
	m_currentExercises->appendExerciseEntry(item);
	emit itemReady(item, tDayExerciseCreateId);
}

void TPMesocycleClass::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous);
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

void TPMesocycleClass::removeExerciseObject(const uint exercise_idx)
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

void TPMesocycleClass::moveExercise(const uint exercise_idx, const uint new_idx)
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

	m_currentExercises->exerciseObjects.swapItemsAt(exercise_idx, new_idx);
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		m_currentExercises->exerciseEntry(x)->setParentItem(nullptr);
	QQuickItem* parentLayout(m_CurrenttDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		m_currentExercises->exerciseEntry(x)->setParentItem(parentLayout);

	m_CurrenttDayModel->moveExercise(exercise_idx, new_idx);
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(m_currentExercises->exerciseEntry(exercise_idx), "moveExercise", Q_ARG(bool, new_idx > exercise_idx), Q_ARG(bool, false));
}
//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
void TPMesocycleClass::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet,
										const QString& nReps, const QString& nWeight)
{
	const uint set_type_cpp(set_type == 2 ? 1 : set_type == 4 ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
		m_setComponents[set_type_cpp] = new QQmlComponent(m_QMlEngine, QUrl(setTypePages[set_type_cpp]), QQmlComponent::Asynchronous);

	m_setObjectProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_CurrenttDayModel));
	if (bNewSet)
	{
		if (set_number == 0)
			currenttDayModel()->newFirstSet(exercise_idx, set_type, nReps, nWeight);
		else
			currenttDayModel()->newSet(set_number, exercise_idx, set_type);
		m_expectedSetNumber = set_number;
	}

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [&,set_type,set_number,exercise_idx](QQmlComponent::Status status)
			{ if (status == QQmlComponent::Ready) return createSetObject_part2(set_type, set_number, exercise_idx); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(set_type, set_number, exercise_idx);
}

void TPMesocycleClass::createSetObject_part2(const uint set_type, const uint set_number, const uint exercise_idx)
{
	const uint set_type_cpp(set_type == 2 ? 1 : set_type == 4 ? 2 : 0);
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
	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, m_QMlEngine->rootContext())));

	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	if (set_number > 0)
	{
		connect( item, SIGNAL(requestTimerDialogSignal(QQuickItem*,const QVariant&)), this,
					SLOT(requestTimerDialog(QQuickItem*,const QVariant&)) );
	}
	if (set_type == 4)
		item->setProperty("ownerExercise", QVariant::fromValue(m_currentExercises->exerciseEntry(exercise_idx)));

	if (set_number >= m_currentExercises->setCount(exercise_idx))
		m_currentExercises->appendSet(exercise_idx, item);
	else
		m_currentExercises->insertSet(set_number, exercise_idx, item);

	//Sets may be crated at any random order, specially when there are set objects of different kind within an exercise. m_expectedSetNumber keeps
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
	//After any set added, by default, set the number of sets to be added afterwards is one at a time, and set the suggested reps and weight for the next set
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nSets", "1");
	QString suggestedValue(m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type));
	int idx(suggestedValue.indexOf(subrecord_separator));
	if (idx != -1)
		suggestedValue = suggestedValue.left(idx);
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nReps", suggestedValue);
	suggestedValue = m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type);
	idx = suggestedValue.indexOf(subrecord_separator);
	if (idx != -1)
		suggestedValue = suggestedValue.left(idx);
	m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nWeight", suggestedValue);
}

void TPMesocycleClass::createSetObjects(const uint exercise_idx)
{
	if (!setsLoaded(exercise_idx))
	{
		m_expectedSetNumber = 0;
		const uint nsets(currenttDayModel()->setsNumber(exercise_idx));
		for (uint i(0); i < nsets; ++i)
			createSetObject(currenttDayModel()->setType(i, exercise_idx), i, exercise_idx, false);
	}
}

//Convenience function for ExerciseEntry::createSetObject. Create last_set - first_set sets on the fly as per user command
void TPMesocycleClass::createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
							const QString& nReps, const QString& nWeight)
{
	if (!nReps.isEmpty())
	{
		connect(this, &TPMesocycleClass::itemReady, this, [&,set_type,first_set, last_set,exercise_idx](QQuickItem* newSet, uint id)
			{
				if (id == tDaySetCreateId)
				{
					emit itemReady(newSet, id);
					return createSetObjects(exercise_idx, first_set, last_set, set_type);
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		createSetObject(set_type, first_set, exercise_idx, true, nReps, nWeight);
		m_expectedSetNumber = first_set;
		return;
	}
	for (uint i(first_set+1); i < last_set; ++i)
	{
		currenttDayModel()->newSet(i, exercise_idx, set_type);
		createSetObject_part2(set_type, i, exercise_idx);
	}
}

void TPMesocycleClass::removeSetObject(const uint set_number, const uint exercise_idx)
{
	if (exercise_idx < m_currentExercises->exercisesCount())
	{
		for(uint x(set_number+1); x < m_currentExercises->setCount(exercise_idx); ++x)
			m_currentExercises->setObject(exercise_idx, x)->setProperty("setNumber", x-1);
		m_CurrenttDayModel->removeSet(set_number, exercise_idx);
		m_currentExercises->removeSet(exercise_idx, set_number);
		const uint nsets(m_currentExercises->setCount(exercise_idx));
		m_currentExercises->exerciseEntry(exercise_idx)->setProperty("setNbr", nsets);
		if (nsets > 0 && set_number == nsets) //last set was removed, update suggested values for a possible set addition
		{
			m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nReps", m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
			m_currentExercises->exerciseEntry(exercise_idx)->setProperty("nWeight", m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, m_CurrenttDayModel->setType(set_number-1, exercise_idx)));
		}
	}
}

void TPMesocycleClass::changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text)
{
	if (label_idx == 1)
		m_CurrenttDayModel->setExerciseName1(new_text, exercise_idx);
	else
		m_CurrenttDayModel->setExerciseName2(new_text, exercise_idx);

	QQuickItem* setObj(nullptr);
	QQuickItem* txtExercise(nullptr);
	for (uint i(0); i < m_currentExercises->setCount(exercise_idx); ++i)
	{
		setObj = m_currentExercises->setObject_const(exercise_idx, i);
		txtExercise = setObj->findChild<QQuickItem*>(label_idx == 1 ? u"txtExercise1"_qs : u"txtExercise2"_qs);
		if (txtExercise)
			QMetaObject::invokeMethod(setObj, "changeExerciseText", Q_ARG(QVariant, QVariant::fromValue(txtExercise)), Q_ARG(QString, new_text));
	}
}

void TPMesocycleClass::changeSetType(const uint set_number, const uint exercise_idx, const uint new_type)
{
	if (new_type != 100)
	{
		const uint current_type(m_CurrenttDayModel->setType(set_number, exercise_idx));
		m_CurrenttDayModel->changeSetType(set_number, exercise_idx, new_type);
		if (current_type != 2 && current_type != 4)
		{
			if (new_type != 2 && new_type != 4)
			{
				m_currentExercises->setObject(exercise_idx, set_number)->setProperty("setType", new_type);
				return;
			}
		}

		m_currentExercises->removeSet(exercise_idx, set_number);

		m_expectedSetNumber = 100; //do not trigger the itemReady signal nor add the object to the parent layout
		connect(this, &TPMesocycleClass::itemReady, this, [&, set_number, exercise_idx](QQuickItem*, uint)
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

QQuickItem* TPMesocycleClass::nextSetObject(const uint exercise_idx, const uint set_number) const
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

void TPMesocycleClass::copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i) {
		set_type = m_CurrenttDayModel->setType(i, exercise_idx);
		updatedValue = m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type, i-1, sub_set);
		if (set_type == 2 || set_type == 4)
			m_CurrenttDayModel->setSetReps(i, exercise_idx, sub_set, updatedValue);
		else
			m_CurrenttDayModel->setSetReps(i, exercise_idx, updatedValue);

		QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeReps", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
	}
}

void TPMesocycleClass::copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i) {
		set_type = m_CurrenttDayModel->setType(i, exercise_idx);
		updatedValue = m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type, i-1, sub_set);
		if (set_type == 2 || set_type == 4)
			m_CurrenttDayModel->setSetWeight(i, exercise_idx, sub_set, updatedValue);
		else
			m_CurrenttDayModel->setSetWeight(i, exercise_idx, updatedValue);

		QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeWeight", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
	}
}
//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

void TPMesocycleClass::tDayExercises::appendExerciseEntry(QQuickItem* new_exerciseItem)
{
	exerciseObject* exerciseObj(new exerciseObject);
	exerciseObj->m_exerciseEntry = new_exerciseItem;
	exerciseObjects.append(exerciseObj);
}

void TPMesocycleClass::tDayExercises::removeExerciseEntry(const uint exercise_idx)
{
	exerciseObject* exerciseObj(exerciseObjects.at(exercise_idx));
	for (uint x(0); x < exerciseObj->m_setObjects.count(); ++x)
		exerciseObj->m_setObjects.at(x)->deleteLater();
	exerciseObj->m_setObjects.clear();
	exerciseObj->m_exerciseEntry->deleteLater();
	exerciseObjects.removeAt(exercise_idx);
	delete exerciseObj;
}

void TPMesocycleClass::tDayExercises::removeSet(const uint exercise_idx, const uint set_number)
{
	setObject(exercise_idx, set_number)->deleteLater();
	exerciseObjects[exercise_idx]->m_setObjects.remove(set_number);
}


//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
void TPMesocycleClass::addMainMenuShortCut(const QString& label, QQuickItem* page)
{
	const int button_id(m_mainMenuShortcutPages.indexOf(page));
	if (button_id != -1)
		openMainMenuShortCut(button_id);
	else
	{
		if (m_mainMenuShortcutEntries.count() < 5)
		{
			connect( this, &TPMesocycleClass::itemReady, this, [&,label, page](QQuickItem* entry, uint id)
			{
				if (id == menuShortCutCreatedId)
				{
					m_mainMenuShortcutEntries.append(entry);
					m_mainMenuShortcutPages.append(page);
					QMetaObject::invokeMethod(m_appStackView, "push", Q_ARG(QQuickItem*, page));
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
			createMainMenuShortCut(label);
			return;
		}
		else
		{
			for (uint i(0); i < m_mainMenuShortcutPages.count()-1; ++i)
			{
				m_mainMenuShortcutPages.move(i+1, i);
				m_mainMenuShortcutEntries.move(i+1, i);
			}
			m_mainMenuShortcutEntries.at(4)->setProperty("label", label);
			m_mainMenuShortcutPages.replace(4, page);
			QMetaObject::invokeMethod(m_appStackView, "push", Q_ARG(QVariant, QVariant::fromValue(page)));
		}
	}
}

void TPMesocycleClass::createMainMenuShortCut(const QString& label)
{
	if (m_mainMenuShortcutComponent == nullptr)
	{
		m_mainMenuShortcutComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/TransparentButton.qml"_qs), QQmlComponent::Asynchronous);
		QObject* appMainMenu(m_QMlEngine->rootContext()->contextProperty(u"appMainMenu"_qs).value<QObject*>());
		m_mainMenuShortcutLayout = appMainMenu->findChild<QQuickItem*>(u"windowListLayout"_qs);
	}

	m_mainMenuShortcutProperties.insert( u"clickId"_qs, m_mainMenuShortcutEntries.count());
	m_mainMenuShortcutProperties.insert( u"text"_qs, label);

	if (m_mainMenuShortcutComponent->status() != QQmlComponent::Ready)
		connect(m_mainMenuShortcutComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status)
			{ return createMainMenuShortCut_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	else
		createMainMenuShortCut_part2();
}

void TPMesocycleClass::createMainMenuShortCut_part2()
{
	if (m_mainMenuShortcutComponent->isReady())
	{
		QQuickItem* entry(static_cast<QQuickItem*>(m_mainMenuShortcutComponent->createWithInitialProperties(
					m_mainMenuShortcutProperties, m_QMlEngine->rootContext())));
		#ifdef DEBUG
		if (m_mainMenuShortcutComponent->status() == QQmlComponent::Error)
		{
			qDebug() << m_mainMenuShortcutComponent->errorString();
			for (uint i(0); i < m_mainMenuShortcutComponent->errors().count(); ++i)
				qDebug() << m_mainMenuShortcutComponent->errors().at(i).description();
			return;
		}
		#endif
		entry->setProperty("Layout.fillWidth", true);
		m_QMlEngine->setObjectOwnership(entry, QQmlEngine::CppOwnership);
		entry->setParentItem(m_mainMenuShortcutLayout);
		connect( entry, SIGNAL(buttonClicked(int)), this, SLOT(openMainMenuShortCut(int)) );
		emit itemReady(entry, menuShortCutCreatedId);
	}
}

void TPMesocycleClass::openMainMenuShortCut(int button_id)
{
	QQuickWindow* mainWindow(static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0)));
	QMetaObject::invokeMethod(mainWindow, "stackViewPushExistingPage", Q_ARG(QVariant, QVariant::fromValue(m_mainMenuShortcutPages.at(button_id))));
}
//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
