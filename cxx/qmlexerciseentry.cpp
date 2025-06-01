#include "qmlexerciseentry.h"

#include "dbexercisesmodel.h"
#include "qmlitemmanager.h"
#include "qmlsetentry.h"
#include "qmlworkoutinterface.h"

#include "tpsettings.h"
#include "tptimer.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

#define CPP_SET_TYPE_REGULAR 0
#define CPP_SET_TYPE_DROP 1
#define CPP_SET_TYPE_GIANT 2

struct exerciseIdxEntry{
	QList<QmlSetEntry*> m_setObjects;
};

static const QString setTypePages[3] { std::move("qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_L1),
					std::move("qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_L1), std::move("qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_L1) };

QmlExerciseEntry::QmlExerciseEntry(QObject *parent, QmlWorkoutInterface *workoutPage,
													DBExercisesModel *workoutModel, const uint exercise_number)
	: QObject{parent}, m_workoutManager{workoutPage}, m_workoutModel{workoutModel}, m_exerciseNumber{exercise_number},
			m_setTimer{nullptr}, m_setComponents{nullptr}, m_setsToBeCreated{0}
{
	connect(this, &QmlExerciseEntry::setObjectCreated, this, &QmlExerciseEntry::setCreated);
	connect(m_workoutModel, &DBExercisesModel::exerciseModified, this,
					[this] (const uint exercise_number, const uint exercise_idx, const uint set_number, const uint field) {
		if (exercise_number == m_exerciseNumber)
		{
			switch (field)
			{
				case EXERCISES_COL_COMPLETED:
				{
					setCanEditRestTimeTracking(m_workoutModel->noSetsCompleted(m_exerciseNumber, exercise_idx));
					if (set_number < m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx) - 1)
					{
						const QmlSetEntry *qml_set_obj{m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number+1)};
						const QQuickItem *next_set_obj{qml_set_obj->setEntry()};
						QMetaObject::invokeMethod(m_workoutManager->workoutPage(), "placeSetIntoView",
														  Q_ARG(int, next_set_obj->y() + next_set_obj->height()));
					}
					else
						m_workoutManager->gotoNextExercise(m_exerciseNumber);
				}
				break;
			}
		}
	});
}

QmlExerciseEntry::~QmlExerciseEntry()
{
	if (m_setComponents[0])
		delete m_setComponents[0];
	if (m_setComponents[1])
		delete m_setComponents[1];
	if (m_setComponents[2])
		delete m_setComponents[2];
	for (const auto exercise_idx : std::as_const(m_exercisesIdxs))
		qDeleteAll(exercise_idx->m_setObjects);
	qDeleteAll(m_exercisesIdxs);
	delete m_exerciseEntry;
}

void QmlExerciseEntry::setExerciseEntry(QQuickItem *item)
{
	m_exerciseEntry = item;
	m_setsLayout = m_exerciseEntry->findChild<QQuickItem*>("exerciseSetsLayout"_L1);
};

void QmlExerciseEntry::setWorkingSet(QmlSetEntry *new_workingset)
{
	m_workoutModel->setWorkingExercise(m_exerciseNumber);
	m_workoutModel->setWorkingSubExercise(m_exerciseNumber, new_workingset->exerciseIdx());
	if (m_workingSet != new_workingset)
	{
		m_workingSet = new_workingset;
		m_workoutModel->setWorkingSet(m_exerciseNumber, new_workingset->exerciseIdx(), new_workingset->setNumber());
		emit workingSetChanged();
	}
}

void QmlExerciseEntry::setExerciseNumber(const uint new_value)
{
	m_exerciseNumber = new_value;
	m_exerciseEntry->setProperty("Layout.row", new_value);
	emit exerciseNumberChanged();
	for (const auto exercise_idx : std::as_const(m_exercisesIdxs))
	{
		for (const auto set : std::as_const(exercise_idx->m_setObjects))
			set->setExerciseNumber(m_exerciseNumber);
	}
}

const QString QmlExerciseEntry::exerciseName(const int exercise_idx) const
{
	if (exercise_idx >= 0)
		return m_workoutModel->exerciseName(m_exerciseNumber, exercise_idx);
	else
	{
		const uint n_sub_exercises{m_workoutModel->subExercisesCount(m_exerciseNumber)};
		if (n_sub_exercises == 1)
			return m_workoutModel->exerciseName(m_exerciseNumber, 0);
		QString composite_name(std::move(tr("Giant exercise: ") + "<br>"_L1));
		for (uint i{0}; i < n_sub_exercises; ++i)
			composite_name += std::move(QString::number(i+1) + " - "_L1 + m_workoutModel->exerciseName(m_exerciseNumber, i) + "<br>\n");
		return composite_name;
	}
}

void QmlExerciseEntry::setExerciseName(const uint exercise_idx, const QString &new_exercisename)
{
	if (new_exercisename != m_workoutModel->exerciseName(m_exerciseNumber, 0))
	{
		m_workoutModel->setExerciseName(m_exerciseNumber, 0, new_exercisename);
		emit exerciseNameChanged();
	}
}

QString QmlExerciseEntry::setsNumber(const uint exercise_idx) const
{
	return QString::number(m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx));
}

void QmlExerciseEntry::addSubExercise()
{
	static_cast<void>(m_workoutModel->addSubExercise(m_exerciseNumber));
	m_exercisesIdxs.append(new exerciseIdxEntry);
}

const bool QmlExerciseEntry::trackRestTime() const
{
	return m_workoutModel->trackRestTime(m_exerciseNumber);
}

void QmlExerciseEntry::setTrackRestTime(const bool track_resttime)
{
	m_workoutModel->setTrackRestTime(m_exerciseNumber, track_resttime);
	emit trackRestTimeChanged();
	if (!track_resttime)
		setAutoRestTime(false);
}

const bool QmlExerciseEntry::autoRestTime() const
{
	return m_workoutModel->autoRestTime(m_exerciseNumber);
}

void QmlExerciseEntry::setAutoRestTime(const bool auto_resttime)
{
	m_workoutModel->setAutoRestTime(m_exerciseNumber, auto_resttime);
	emit autoRestTimeChanged();
}

const bool QmlExerciseEntry::hasSets() const
{
	for(uint exercise_idx{0}; exercise_idx < m_workoutModel->subExercisesCount(m_exerciseNumber); ++exercise_idx)
	{
		if (m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx) > 0)
			return true;
	}
	return false;
}

void QmlExerciseEntry::setIsEditable(const bool editable)
{
	m_bEditable = editable;
	emit isEditableChanged();

	for (const auto exercise_idx : std::as_const(m_exercisesIdxs))
	{
		for (const auto set : std::as_const(exercise_idx->m_setObjects))
		{
			set->setIsEditable(m_bEditable);
			if (!m_workoutManager->mainDateIsToday())
				set->setCurrent(m_bEditable);
		}
	}
}

const bool QmlExerciseEntry::allSetsCompleted() const
{
	return m_workoutModel->allSetsCompleted(m_exerciseNumber);
}

void QmlExerciseEntry::removeExercise(const bool bAsk)
{
	m_workoutManager->removeExerciseObject(m_exerciseNumber, bAsk);
}

void QmlExerciseEntry::exerciseCompleted()
{
	m_workoutManager->gotoNextExercise(m_exerciseNumber);
}

void QmlExerciseEntry::moveExerciseUp()
{
	if (m_exerciseNumber >= 1)
		m_workoutManager->moveExercise(m_exerciseNumber, m_exerciseNumber-1);
}

void QmlExerciseEntry::moveExerciseDown()
{
	if (!lastExercise())
		m_workoutManager->moveExercise(m_exerciseNumber, m_exerciseNumber+1);
}

void QmlExerciseEntry::createAvailableSets()
{
	if (m_exercisesIdxs.isEmpty() && hasSets())
	{
		for (uint i{0}; i < m_workoutModel->subExercisesCount(m_exerciseNumber); ++i)
		{
			const uint nsets(m_workoutModel->setsNumber(m_exerciseNumber, i));
			m_setsToBeCreated += nsets;
			m_exercisesIdxs.append(new exerciseIdxEntry);
			emit subExercisesCountChanged();
			for (uint x{0}; x < nsets; ++i)
			{
				m_expectedSetNumber = x;
				createSetObject(x, i);
			}
		}
	}
}

void QmlExerciseEntry::appendNewSet()
{
	const uint exercise_idx{m_workoutModel->workingSubExercise()};
	const uint set_number{m_workoutModel->addSet(m_exerciseNumber, exercise_idx)};
	if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.isEmpty())
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.last()->setLastSet(false);
	m_workoutModel->setWorkingSet(set_number);
	createSetObject(exercise_idx, set_number);
}

void QmlExerciseEntry::removeSetObject(const bool show_delete_dialog)
{
	if (show_delete_dialog && appSettings()->alwaysAskConfirmation())
		QMetaObject::invokeMethod(m_workoutManager, "showRemoveSetMessage");
	else
	{
		const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
		const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
		m_workoutModel->delSet(m_exerciseNumber, exercise_idx, set_number);
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->deleteLater();
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.remove(set_number);
		for(uint i{set_number}; i < m_exercisesIdxs.at(exercise_idx)->m_setObjects.count(); ++i)
			moveSet(i-1, i);
		m_workoutManager->setWorkingSet(m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number));
		if (m_exercisesIdxs.at(exercise_idx)->m_setObjects.count() == 0)
			setCanEditRestTimeTracking(true);
		else
		{
			if (set_number == m_exercisesIdxs.at(exercise_idx)->m_setObjects.count()) //last set was removed
				m_exercisesIdxs.at(exercise_idx)->m_setObjects.last()->setLastSet(true);
			exerciseCompleted();
		}
	}
}

void QmlExerciseEntry::moveSet(const uint set_number, const uint new_set_number)
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	m_workoutModel->moveSet(m_exerciseNumber, exercise_idx, set_number, new_set_number);
	m_exercisesIdxs.at(exercise_idx)->m_setObjects.swapItemsAt(set_number, new_set_number);

	for (const auto set : std::as_const(m_exercisesIdxs.at(exercise_idx)->m_setObjects))
	{
		set->setEntry()->setParentItem(nullptr);
		set->setEntry()->setProperty("Layout.row", QString{});
	}

	m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setSetNumber(set_number);
	m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(new_set_number)->setSetNumber(new_set_number);
	if (set_number == m_exercisesIdxs.at(exercise_idx)->m_setObjects.count() - 1)
	{
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setLastSet(true);
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(new_set_number)->setLastSet(false);
	}
	else if (new_set_number == m_exercisesIdxs.at(exercise_idx)->m_setObjects.count() - 1)
	{
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setLastSet(false);
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(new_set_number)->setLastSet(true);
	}

	for (const auto set : std::as_const(m_exercisesIdxs.at(exercise_idx)->m_setObjects))
	{
		set->setEntry()->setParentItem(m_setsLayout);
		set->setEntry()->setProperty("Layout.row", set->setNumber());
	}
}

void QmlExerciseEntry::changeSetType(const uint set_number, const uint new_type)
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint old_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
	m_workoutModel->changeSetType(m_exerciseNumber, exercise_idx, set_number, new_type);
	if (old_type != new_type)
	{
		if (old_type != Drop)
		{
			if (new_type != Drop)
				return;
		}
		removeSetObject(set_number);
		createSetObject(exercise_idx, set_number);
	}
}

void QmlExerciseEntry::changeSetMode()
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
	QmlSetEntry *setObj(m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number));
	switch (setObj->mode())
	{
		case SET_MODE_UNDEFINED:
			if (set_number == 0 || !setObj->trackRestTime())
			{
				setObj->setCompleted(true);
				setObj->setMode(SET_MODE_SET_COMPLETED);
				gotoNextSet(exercise_idx, set_number);
			}
			else
				setObj->setMode(SET_MODE_START_REST);
			break;
		case SET_MODE_START_REST:
			setObj->setMode(SET_MODE_START_EXERCISE);
			startRestTimer(exercise_idx, set_number, setObj->autoRestTime());
		break;
		case SET_MODE_START_EXERCISE:
			stopRestTimer(exercise_idx, set_number);
			setObj->setCompleted(true);
		break;
		case SET_MODE_SET_COMPLETED:
			if (set_number == 0 || !setObj->autoRestTime())
			{
				setObj->setCurrent(true);
				setObj->setCompleted(false);
				setObj->setMode(SET_MODE_UNDEFINED);
			}
			else
				setObj->setMode(SET_MODE_START_REST);
	}
}

void QmlExerciseEntry::updateSetTypeForNextSets()
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
	const uint set_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
	const uint nsets{m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx)};

	for (uint i{set_number+1}; i < nsets; ++i)
	{
		if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->completed())
			changeSetType(i, set_type);
	}
}

void QmlExerciseEntry::updateRestTimeForNextSets()
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
	const uint nsets{m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx)};

	for (uint i{set_number+1}; i < nsets; ++i)
	{
		if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->completed())
			m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->setRestTime(appUtils()->formatTime(m_workoutModel->suggestedRestTime(
						m_workoutModel->restTime(m_exerciseNumber, exercise_idx, i-1),
						m_workoutModel->setType(m_exerciseNumber, exercise_idx, i))));
	}
}

void QmlExerciseEntry::updateRepsForNextSets(const uint sub_set)
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
	const uint nsets{m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx)};

	for (uint i{set_number+1}; i < nsets; ++i)
	{
		if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->completed())
		{
			m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->setReps(m_workoutModel->suggestedReps(
						m_workoutModel->setReps(m_exerciseNumber, exercise_idx, i-1, sub_set),
						m_workoutModel->setType(m_exerciseNumber, exercise_idx, i)));
		}
	}
}

void QmlExerciseEntry::updateWeightForNextSets(const uint sub_set)
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
	const uint nsets{m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx)};

	for (uint i{set_number+1}; i < nsets; ++i)
	{
		if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->completed())
		{
			m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->setWeight(m_workoutModel->suggestedWeight(
						m_workoutModel->setWeight(m_exerciseNumber, exercise_idx, i-1, sub_set),
						m_workoutModel->setType(m_exerciseNumber, exercise_idx, i)));
		}
	}
}

void QmlExerciseEntry::simpleExercisesList(const bool show, const bool multi_sel)
{
	m_workoutManager->simpleExercisesList(show, multi_sel);
}

void QmlExerciseEntry::insertSetObject(const uint exercise_idx, const uint set_number, QmlSetEntry *new_setobject)
{
	if (set_number >= m_exercisesIdxs.at(exercise_idx)->m_setObjects.count())
	{
		for(uint i{static_cast<uint>(m_exercisesIdxs.at(exercise_idx)->m_setObjects.count())}; i <= set_number; ++i)
			m_exercisesIdxs.at(exercise_idx)->m_setObjects.insert(i, nullptr);
	}
	m_exercisesIdxs.at(exercise_idx)->m_setObjects[set_number] = new_setobject;
}

static inline uint cppSetType(const uint set_type)
{
	switch (set_type)
	{
		case Drop:
			return CPP_SET_TYPE_DROP;
		default:
			return CPP_SET_TYPE_REGULAR;
	}
}

void QmlExerciseEntry::createSetObject(const uint exercise_idx, const uint set_number)
{
	const uint set_type_cpp{cppSetType(m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number))};
	if (m_setComponents[set_type_cpp] == nullptr)
	{
		m_setComponents[set_type_cpp] = new QQmlComponent{appQmlEngine(), QUrl{setTypePages[set_type_cpp]}, QQmlComponent::Asynchronous};
		m_setObjectProperties.insert("exerciseManager"_L1, QVariant::fromValue(this));
	}

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [this,exercise_idx, set_number](QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				createSetObject_part2(set_number, exercise_idx);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(exercise_idx, set_number);
}

void QmlExerciseEntry::createSetObject_part2(const uint exercise_idx, const uint set_number)
{
	const uint set_type_cpp{cppSetType(m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number))};
	#ifndef QT_NO_DEBUG
	if (m_setComponents[set_type_cpp]->status() == QQmlComponent::Error)
	{
		for (auto &error : m_setComponents[set_type_cpp]->errors())
			qDebug() << error.description();
		return;
	}
	#endif

	QmlSetEntry *newSetEntry{new QmlSetEntry{this, this, m_workoutModel, m_exerciseNumber, exercise_idx, set_number}};
	insertSetObject(exercise_idx, set_number, newSetEntry);
	m_setObjectProperties["setManager"_L1] = QVariant::fromValue(newSetEntry);

	QQuickItem *item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setSetEntry(item);
	emit setObjectCreated(newSetEntry);
}

void QmlExerciseEntry::setCreated(QmlSetEntry *set)
{
	m_setsToBeCreated--;
	QQuickItem *qml_set_object{set->setEntry()};
	QVariant set_layout;
	QMetaObject::invokeMethod(qml_set_object, "getLayoutForSubExercise",
									Q_RETURN_ARG(QVariant, set_layout),
									Q_ARG(int, set->exerciseIdx()));
	qml_set_object->setParentItem(set_layout.value<QQuickItem*>());
	qml_set_object->setProperty("Layout.row", set->setNumber());
	qml_set_object->setProperty("Layout.column", 0);

	if (m_setsToBeCreated == 0)
	{
		//TODO
		setWorkingSet(m_exercisesIdxs.first()->m_setObjects.first());
		m_exerciseEntry->setProperty("showSets", true);
		QMetaObject::invokeMethod(m_workoutManager->workoutPage(), "placeSetIntoView",
					Q_ARG(int, exerciseEntry()->y() + exerciseEntry()->height() + qml_set_object->y() + qml_set_object->height()));
	}
}

void QmlExerciseEntry::gotoNextSet(const uint exercise_idx, const uint set_number)
{
	if (m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx) - 1 == set_number)
		m_workoutManager->gotoNextExercise(m_exerciseNumber);
	else
	{
		QmlSetEntry *set{m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number+1)};
		m_workoutManager->setWorkingSet(set);
		QMetaObject::invokeMethod(m_workoutManager->workoutPage(), "placeSetIntoView",
				Q_ARG(int, exerciseEntry()->y() + exerciseEntry()->height() + set->setEntry()->y() + set->setEntry()->height()));
	}
}

void QmlExerciseEntry::startRestTimer(const uint exercise_idx, const uint set_number, const bool stop_watch)
{
	TPTimer *set_timer(m_workoutManager->restTimer());
	if (!set_timer->isActive())
	{
		set_timer->setStopWatch(stop_watch);
		if (stop_watch)
			m_workingSet->setRestTime("00:00"_L1);
		set_timer->prepareTimer(m_workoutModel->setRestTime(m_exerciseNumber, exercise_idx, set_number));

		connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer] () {
			m_workingSet->setRestTime(set_timer->strMinutes() + ':' + set_timer->strSeconds(), false);
		});
		set_timer->startTimer();
	}
	else
		m_workoutManager->displayMessage(tr("Cannot start timer!"), tr("Another set is using it"), false, 3000);
}

void QmlExerciseEntry::stopRestTimer(const uint exercise_idx, const uint set_number)
{
	TPTimer *set_timer(m_workoutManager->restTimer());
	if (set_timer->isActive())
	{
		set_timer->stopTimer();
		disconnect(set_timer, nullptr, nullptr, nullptr);
		if (m_workingSet->autoRestTime())
			m_workingSet->setRestTime(m_workingSet->restTime(), true); //update the model with the current value displayed
		else
			m_workingSet->setRestTime(appUtils()->formatTime(set_timer->elapsedTime(), TPUtils::TF_QML_DISPLAY_NO_SEC), false);
	}
}
