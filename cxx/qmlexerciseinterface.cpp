#include "qmlexerciseinterface.h"
#include "qmltdayinterface.h"
#include "qmlexerciseentry.h"
#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
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
}

void QmlExerciseInterface::createExerciseObject()
{
	if (!m_exercisesComponent)
	{
		m_exercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_s}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createExerciseObject();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	const uint exercise_idx(m_exercisesList.count());
	bool bTrackRestTime(false), bAutoRestTime(false);
	QString nRestTime;
	if (exercise_idx >= 1)
	{
		bTrackRestTime = m_tDayModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_tDayModel->autoRestTime(exercise_idx-1);
		nRestTime = m_tDayModel->nextSetSuggestedTime(exercise_idx, SET_TYPE_REGULAR);
	}
	else
		nRestTime = m_tDayModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);

	QmlExerciseEntry* newExercise{new QmlExerciseEntry(this, m_tDayPage, m_qmlEngine, m_tDayModel, exercise_idx)};
	getInfoFromExercisesList(newExercise);

	m_tDayModel->newExercise(exercise_idx, newExercise->exerciseName());
	m_tDayModel->setTrackRestTime(exercise_idx, bTrackRestTime);
	m_tDayModel->setAutoRestTime(exercise_idx, bAutoRestTime);
	newExercise->setRestTime(nRestTime);
	newExercise->setLastExercise(true);
	newExercise->setTrackRestTime(bTrackRestTime);
	newExercise->setAutoRestTime(bAutoRestTime);
	newExercise->setCanEditRestTimeTracking(true);
	newExercise->setIsCompleted(false);
	newExercise->setLastExercise(true);
	m_exercisesList.last()->setLastExercise(false);
	m_exercisesList.append(newExercise);
	createExerciseObject_part2(exercise_idx);
}

void QmlExerciseInterface::createExercisesObjects()
{
	if (m_exercisesComponent == nullptr)
	{
		m_exercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_s}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createExercisesObjects();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}
	}

	const uint n_exercises(m_tDayModel->exerciseCount());
	for(uint i(0), set_type(0), last_set(0); i < n_exercises; ++i)
	{
		QmlExerciseEntry* newExercise{new QmlExerciseEntry{this, m_tDayPage, m_qmlEngine, m_tDayModel, i}};
		last_set = m_tDayModel->setsNumber(i);
		if (last_set > 10) last_set = 0; //setsNumber was 0
		set_type = m_tDayModel->setType(last_set, i);
		newExercise->setExerciseName(m_tDayModel->exerciseName(i), false);
		newExercise->setNewSetType(set_type);
		newExercise->setTrackRestTime(m_tDayModel->trackRestTime(i-(i >= 1 ? 1 : 0)));
		newExercise->setAutoRestTime(m_tDayModel->autoRestTime(i-(i >= 1 ? 1 : 0)));
		newExercise->setIsCompleted(m_tDayModel->allSetsCompleted(i));
		newExercise->setCanEditRestTimeTracking(!m_tDayModel->anySetCompleted(i));
		newExercise->setLastExercise(i == n_exercises - 1);
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

void QmlExerciseInterface::removeExerciseSet(const uint exercise_idx, const uint set_number)
{
	m_exercisesList.at(exercise_idx)->removeSetObject(set_number, false);
}

void QmlExerciseInterface::clearExercises()
{
	m_tDayModel->clearExercises();
	m_tDayPage->setDayIsFinished(false);
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->deleteLater();
	m_exercisesList.clear();
}

void QmlExerciseInterface::setExercisesEditable(const bool editable)
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->setIsEditable(editable);
}

void QmlExerciseInterface::moveExercise(const uint exercise_idx, const uint new_idx)
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(nullptr);
	m_exercisesList.swapItemsAt(exercise_idx, new_idx);
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(m_parentLayout);
	m_exercisesList.at(exercise_idx)->setExerciseIdx(exercise_idx);
	m_exercisesList.at(new_idx)->setExerciseIdx(new_idx);
	if (exercise_idx == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_idx)->setLastExercise(true);
		m_exercisesList.at(new_idx)->setLastExercise(false);
	}
	else if (new_idx == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_idx)->setLastExercise(false);
		m_exercisesList.at(new_idx)->setLastExercise(true);
	}
}

void QmlExerciseInterface::gotoNextExercise(const uint exercise_idx) const
{
	if (exercise_idx < m_exercisesList.count())
	{
		QMetaObject::invokeMethod(m_exercisesList.at(exercise_idx)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
		for(uint i(exercise_idx+1); i < m_exercisesList.count(); ++i)
		{
			if (!m_exercisesList.at(exercise_idx)->exerciseEntry())
			{
				QMetaObject::invokeMethod(m_exercisesList.at(exercise_idx+1)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, true), Q_ARG(bool, true));
				QMetaObject::invokeMethod(m_tDayPage, "placeSetIntoView", Q_ARG(int, m_exercisesList.at(exercise_idx+1)->exerciseEntry()->y() + 50));
				return;
			}
		}
		QMetaObject::invokeMethod(m_tDayPage, "placeSetIntoView", Q_ARG(int, 0));
	}
}

void QmlExerciseInterface::hideSets() const
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		QMetaObject::invokeMethod(m_exercisesList.at(i)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	QMetaObject::invokeMethod(m_tDayPage, "placeSetIntoView", Q_ARG(int, 0));
}

void QmlExerciseInterface::showSimpleExercisesList(const uint exercise_idx, const bool bMultiSel)
{
	if (m_simpleExercisesListRequester < 0)
	{
		m_simpleExercisesListRequester = exercise_idx;
		connect(m_tDayPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
		connect(m_tDayPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_tDayPage, "showSimpleExercisesList", Q_ARG(bool, bMultiSel));
	}
}

void QmlExerciseInterface::hideSimpleExercisesList()
{
	disconnect(m_tDayPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
	disconnect(m_tDayPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
	QMetaObject::invokeMethod(m_tDayPage, "hideSimpleExercisesList");
	m_simpleExercisesListRequester = -1;
}

void QmlExerciseInterface::exerciseSelected()
{
	getInfoFromExercisesList(m_exercisesList.at(m_simpleExercisesListRequester));
}

void QmlExerciseInterface::createExerciseObject_part2(const uint exercise_idx)
{
	#ifndef QT_NO_DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (uint i(0); i < m_exercisesComponent->errors().count(); ++i)
			qDebug() << m_exercisesComponent->errors().at(i).description();
		return;
	}
	#endif

	m_exercisesProperties.insert(u"exerciseManager"_s, QVariant::fromValue(m_exercisesList.at(exercise_idx)));

	QQuickItem* item (static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
													m_exercisesProperties, m_qmlEngine->rootContext())));
	m_qmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);

	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
	m_exercisesList.at(exercise_idx)->setExerciseEntry(item);
}

void QmlExerciseInterface::getInfoFromExercisesList(QmlExerciseEntry* exerciseEntry)
{
	QString exerciseName, nSets, nReps, nWeight;
	if (appExercisesModel()->selectedEntriesCount() == 1)
	{
		exerciseName = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_s + appExercisesModel()->selectedEntriesValue_fast(0, 2);
		nSets = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_SETSNUMBER);
		nReps = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER);
		nWeight = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT);
	}
	else
	{
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + u" - "_s +
						appExercisesModel()->selectedEntriesValue_fast(0, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_MAINNAME) + u" - "_s +
						appExercisesModel()->selectedEntriesValue_fast(1, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_SETSNUMBER), nSets, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_SETSNUMBER), nSets, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER), nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_REPSNUMBER), nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(0, appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT), nWeight, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_WEIGHT), nWeight, comp_exercise_separator);
	}
	exerciseEntry->setExerciseName(exerciseName, false);
	exerciseEntry->setNewSetType(!exerciseEntry->compositeExercise() ? SET_TYPE_REGULAR : SET_TYPE_GIANT);
	exerciseEntry->setSetsNumber(nSets);
	exerciseEntry->setReps(nReps);
	exerciseEntry->setWeight(nWeight);
}
