#include "qmlexerciseinterface.h"
#include "qmlexerciseentry.h"
#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "tpappcontrol.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlExerciseInterface::~QmlExerciseInterface()
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		delete m_exercisesList.at(i);
	delete m_exercisesComponent;
/*if (m_setComponents[0])
		delete m_setComponents[0];
	if (m_setComponents[1])
		delete m_setComponents[1];
	if (m_setComponents[2])
		delete m_setComponents[2];*/
}

void QmlExerciseInterface::createExerciseObject()
{
	if (!m_exercisesComponent)
	{
		m_exercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createExerciseObject();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}
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
	const uint exercise_idx(m_exercisesList.count());
	bool bTrackRestTime(false), bAutoRestTime(false);
	if (exercise_idx >= 1)
	{
		bTrackRestTime = m_tDayModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_tDayModel->autoRestTime(exercise_idx-1);
		nRestTime = m_tDayModel->nextSetSuggestedTime(exercise_idx, SET_TYPE_REGULAR);
	}
	else
		nRestTime = m_tDayModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);

	m_tDayModel->newExercise(exercise_idx, exerciseName);
	m_tDayModel->setTrackRestTime(exercise_idx, bTrackRestTime);
	m_tDayModel->setAutoRestTime(exercise_idx, bAutoRestTime);

	QmlExerciseEntry* newExercise{new QmlExerciseEntry(this, exercise_idx, m_tDayModel)};
	newExercise->setCompositeExercise(m_tDayModel->compositeExercise(exercise_idx));
	newExercise->setSetsNumber(nSets);
	newExercise->setRestTime(nRestTime);
	newExercise->setRepsNumber(nReps);
	newExercise->setWeight(nWeight);
	m_exercisesList.append(newExercise);
	createExerciseObject_part2(exercise_idx);
}

void QmlExerciseInterface::createExercisesObjects()
{
	if (m_exercisesComponent == nullptr)
	{
		m_exercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createExercisesObjects();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}
	}

	bool bTrackRestTime(false), bAutoRestTime(false);
	for(uint i(0); i < m_tDayModel->exerciseCount(); ++i)
	{
		bTrackRestTime = m_tDayModel->trackRestTime(i-(i >= 1 ? 1 : 0));
		bAutoRestTime = m_tDayModel->autoRestTime(i-(i >= 1 ? 1 : 0));
		QmlExerciseEntry* newExercise{new QmlExerciseEntry(this, i, m_tDayModel)};
		newExercise->setCompositeExercise(m_tDayModel->compositeExercise(i));
		newExercise->setSetsNumber(STR_ONE);
		newExercise->setRestTime(m_tDayModel->nextSetSuggestedTime(i, m_tDayModel->setType(0, i), 0));
		newExercise->setRepsNumber(m_tDayModel->nextSetSuggestedReps(i, m_tDayModel->setType(0, i), 0));
		newExercise->setWeight(m_tDayModel->nextSetSuggestedWeight(i, m_tDayModel->setType(0, i), 0));
		m_exercisesList.append(newExercise);
		createExerciseObject_part2(i);
	}
}

void QmlExerciseInterface::removeExerciseObject(const uint exercise_idx)
{
	m_tDayModel->removeExercise(exercise_idx);
	m_exercisesList.at(exercise_idx)->deleteLater();
	m_exercisesList.removeAt(exercise_idx);

	for(uint i(exercise_idx); i < m_exercisesList.count(); ++i)
		moveExercise(i-1, i);
}

void QmlExerciseInterface::clearExercises()
{
	m_tDayModel->clearExercises();
	m_tDayModel->setDayIsFinished(false);
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->deleteLater();
	m_exercisesList.clear();
}

void QmlExerciseInterface::moveExercise(const uint exercise_idx, const uint new_idx)
{
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(m_exerciseObjects.at(i)->exerciseEntry(), "moveExercise", Q_ARG(bool, true), Q_ARG(bool, false));

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
	m_tDayModel->moveExercise(exercise_idx, new_idx);

	for(uint x(0); x <m_exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(nullptr);

	QQuickItem* parentLayout(m_tDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	m_currentExercises->exerciseObjects.swapItemsAt(exercise_idx, new_idx);
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(parentLayout);
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "moveExercise", Q_ARG(bool, new_idx > exercise_idx), Q_ARG(bool, false));
}

void QmlExerciseInterface::createExerciseObject_part2(const uint exercise_idx)
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

	m_exercisesProperties.insert(u"exerciseManager"_qs, QVariant::fromValue(m_exercisesList.at(exercise_idx)));
	m_exercisesProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_tDayModel));

	QQuickItem* item (static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
													m_exercisesProperties, m_qmlEngine->rootContext())));
	m_qmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);
	connect(item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)));
	connect(item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)));
	connect(item, SIGNAL(showRemoveExerciseMessage(int)), this, SLOT(showRemoveExerciseMessage(int)));

	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
	m_exercisesList.at(exercise_idx)->setExerciseEntry(item);
}
