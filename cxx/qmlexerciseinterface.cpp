#include "qmlexerciseinterface.h"

#include "dbinterface.h"
#include "dbmesosplitmodel.h"
#include "dbworkoutmodel.h"
#include "qmlexerciseentry.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"

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
		m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_L1}, QQmlComponent::Asynchronous};
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
		bTrackRestTime = m_workoutModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_workoutModel->autoRestTime(exercise_idx-1);
		nRestTime = m_workoutModel->nextSetSuggestedTime(exercise_idx-1, SET_TYPE_REGULAR);
	}
	else
		nRestTime = m_workoutModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);

	QmlExerciseEntry* newExercise{new QmlExerciseEntry{this, m_workoutPage, m_workoutModel, exercise_idx}};
	if (exercise_idx > 0)
		m_exercisesList.last()->setLastExercise(false);
	m_exercisesList.append(newExercise);
	m_workoutModel->newExercise(exercise_idx);
	m_workoutPage->exerciseSelected(newExercise);

	newExercise->setIsEditable(true);
	newExercise->setRestTime(nRestTime);
	newExercise->setLastExercise(true);
	newExercise->setTrackRestTime(bTrackRestTime);
	newExercise->setAutoRestTime(bAutoRestTime);
	newExercise->setCanEditRestTimeTracking(true);
	newExercise->setAllSetsCompleted(false);
	newExercise->setLastExercise(true);

	createExerciseObject_part2(exercise_idx);
	QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, -2));
}

void QmlExerciseInterface::createExercisesObjects()
{
	if (m_exercisesComponent == nullptr)
	{
		m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
				createExercisesObjects();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}
	}

	const uint n_exercises(m_workoutModel->exerciseCount());
	for(uint i(0), set_type(0), last_set(0); i < n_exercises; ++i)
	{
		QmlExerciseEntry* newExercise{new QmlExerciseEntry{this, m_workoutPage, m_workoutModel, i}};
		last_set = m_workoutModel->setsNumber(i) - 1;
		if (last_set > 10) last_set = 0; //setsNumber was 0
		set_type = m_workoutModel->setType(i, last_set);
		newExercise->setExerciseName(m_workoutModel->exerciseName(i), false);
		newExercise->setIsEditable(false);
		newExercise->setNewSetType(set_type);
		newExercise->setTrackRestTime(m_workoutModel->trackRestTime(i-(i >= 1 ? 1 : 0)));
		newExercise->setAutoRestTime(m_workoutModel->autoRestTime(i-(i >= 1 ? 1 : 0)));
		newExercise->setAllSetsCompleted(m_workoutModel->allSetsCompleted(i));
		newExercise->setCanEditRestTimeTracking(!m_workoutModel->anySetCompleted(i));
		newExercise->setLastExercise(i == n_exercises - 1);
		m_exercisesList.append(newExercise);
		createExerciseObject_part2(i);
	}
	QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, -1));
}

void QmlExerciseInterface::removeExerciseObject(const uint exercise_idx)
{
	m_workoutModel->removeExercise(exercise_idx);
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
	m_workoutModel->clearExercises();
	m_workoutPage->setDayIsFinished(false);
	for(uint i(0); i < m_exercisesList.count(); ++i)
		delete m_exercisesList.at(i);
	m_exercisesList.clear();
}

void QmlExerciseInterface::setExercisesEditable(const bool editable)
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		m_exercisesList.at(i)->setIsEditable(editable);
}

void QmlExerciseInterface::moveExercise(const uint exercise_idx, const uint new_idx)
{
	m_exercisesList.swapItemsAt(exercise_idx, new_idx);
	m_workoutModel->moveExercise(exercise_idx, new_idx);

	for(uint i(0); i < m_exercisesList.count(); ++i)
	{
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(nullptr);
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(m_parentLayout);
	}
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
		QMetaObject::invokeMethod(m_exercisesList.at(exercise_idx)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false));
		for(uint i(exercise_idx+1); i < m_exercisesList.count(); ++i)
		{
			if (!m_exercisesList.at(i)->allSetsCompleted())
			{
				QMetaObject::invokeMethod(m_exercisesList.at(i)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, true));
				QQuickItem* exercise_entry{m_exercisesList.at(i-1)->exerciseEntry()};
				QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, exercise_entry->y() + exercise_entry->height()));
				return;
			}
		}
		QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, 0));
	}
}

void QmlExerciseInterface::hideSets() const
{
	for(uint i(0); i < m_exercisesList.count(); ++i)
		QMetaObject::invokeMethod(m_exercisesList.at(i)->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false));
	QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, 0));
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

	m_exercisesProperties.insert("exerciseManager"_L1, QVariant::fromValue(m_exercisesList.at(exercise_idx)));

	QQuickItem* item (static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
													m_exercisesProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);
	m_exercisesList.at(exercise_idx)->setExerciseEntry(item);
}
