#include "qmlexerciseinterface.h"

#include "dbexercisesmodel.h"
#include "qmlexerciseentry.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

#include <ranges>

using namespace Qt::Literals::StringLiterals;

QmlExerciseInterface::~QmlExerciseInterface()
{
	qDeleteAll(m_exercisesList);
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

	const uint exercise_number{m_workoutModel->exerciseCount()};
	m_workoutModel->newExerciseFromExercisesList();

	QmlExerciseEntry *newExercise{new QmlExerciseEntry{this, m_workoutPage, m_workoutModel, exercise_number}};
	if (exercise_number > 0)
		m_exercisesList.last()->setLastExercise(false);

	newExercise->setIsEditable(true);
	newExercise->setLastExercise(true);
	newExercise->setCanEditRestTimeTracking(true);
	newExercise->setLastExercise(true);
	m_exercisesList.append(newExercise);

	createExerciseObject_part2(exercise_number);
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

	const uint n_exercises{m_workoutModel->exerciseCount()};
	for(uint i{0}, set_type(0), last_set(0); i < n_exercises; ++i)
	{
		QmlExerciseEntry *newExercise{new QmlExerciseEntry{this, m_workoutPage, m_workoutModel, i}};
		newExercise->setIsEditable(true);
		newExercise->setLastExercise(true);
		newExercise->setCanEditRestTimeTracking(!m_workoutModel->anySetCompleted(i));
		newExercise->setLastExercise(i == n_exercises - 1);
		m_exercisesList.append(newExercise);
		createExerciseObject_part2(i);
	}
	QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, -1));
}

void QmlExerciseInterface::removeExerciseObject(const uint exercise_idx)
{
	m_workoutModel->delExercise(exercise_idx);
	m_exercisesList.at(exercise_idx)->deleteLater();
	m_exercisesList.removeAt(exercise_idx);

	for(uint i(exercise_idx); i < m_exercisesList.count(); ++i)
		moveExercise(i-1, i);
}

void QmlExerciseInterface::clearExercises()
{
	m_workoutModel->clearExercises();
	m_workoutPage->setDayIsFinished(false);
	qDeleteAll(m_exercisesList);
	m_exercisesList.clear();
}

void QmlExerciseInterface::setExercisesEditable(const bool editable)
{
	for (const auto exercise_entry : std::as_const(m_exercisesList))
		exercise_entry->setIsEditable(editable);
}

void QmlExerciseInterface::moveExercise(const uint exercise_idx, const uint new_exercisenumber)
{
	m_exercisesList.swapItemsAt(exercise_idx, new_exercisenumber);
	m_workoutModel->moveExercise(exercise_idx, new_exercisenumber);

	for(uint i{0}; i < m_exercisesList.count(); ++i)
	{
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(nullptr);
		m_exercisesList.at(i)->exerciseEntry()->setParentItem(m_parentLayout);
	}
	m_exercisesList.at(exercise_idx)->setExerciseNumber(exercise_idx);
	m_exercisesList.at(new_exercisenumber)->setExerciseNumber(new_exercisenumber);
	if (exercise_idx == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_idx)->setLastExercise(true);
		m_exercisesList.at(new_exercisenumber)->setLastExercise(false);
	}
	else if (new_exercisenumber == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_idx)->setLastExercise(false);
		m_exercisesList.at(new_exercisenumber)->setLastExercise(true);
	}
}

void QmlExerciseInterface::gotoNextExercise(const uint exercise_number) const
{
	if (exercise_number < m_exercisesList.count())
	{
		QmlExerciseEntry *anterior_exercise_entry{m_exercisesList.at(exercise_number)};
		QMetaObject::invokeMethod(anterior_exercise_entry->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false));
		for (const auto exercise_entry : m_exercisesList | std::views::drop(exercise_number+1) )
		{
			if (!exercise_entry->allSetsCompleted())
			{
				QMetaObject::invokeMethod(exercise_entry->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, true));
				QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView",
							Q_ARG(int, anterior_exercise_entry->exerciseEntry()->y() + exercise_entry->exerciseEntry()->height()));
				return;
			}
			anterior_exercise_entry = exercise_entry;
		}
		QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, 0));
	}
}

void QmlExerciseInterface::hideSets() const
{
	for (const auto exercise_entry : std::as_const(m_exercisesList))
		QMetaObject::invokeMethod(exercise_entry->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false));
	QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, 0));
}

void QmlExerciseInterface::createExerciseObject_part2(const uint exercise_number)
{
	#ifndef QT_NO_DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (auto error : m_exercisesComponent->errors())
			qDebug() << error;
		return;
	}
	#endif

	m_exercisesProperties.insert("exerciseManager"_L1, QVariant::fromValue(m_exercisesList.at(exercise_number)));

	QQuickItem *item{static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
													m_exercisesProperties, appQmlEngine()->rootContext()))};
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_parentLayout);
	item->setProperty("Layout.row", exercise_number);
	item->setProperty("Layout.column", 0);
	m_exercisesList.at(exercise_number)->setExerciseEntry(item);
}
