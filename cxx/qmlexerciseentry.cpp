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

static const QString &setTypePages[3] { "qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_L1,
					"qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_L1, "qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_L1 };

QmlExerciseEntry::QmlExerciseEntry(QObject *parent, QmlWorkoutInterface *workoutPage,
													DBExercisesModel *workoutModel, const uint exercise_number)
	: QObject{parent}, m_workoutPage{workoutPage}, m_workoutModel{workoutModel}, m_exerciseNumber{exercise_number},
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
						QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView",
														  Q_ARG(int, next_set_obj->y() + next_set_obj->height()));
					}
					else
						m_workoutPage->gotoNextExercise(m_exerciseNumber);
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
		m_workoutModel->setWorkingSet(m_exerciseNumber, new_workingset->exerciseIdx(), new_workingset->number());
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

const QString QmlExerciseEntry::exerciseName1() const
{
	return m_workoutModel->exerciseName(m_exerciseNumber, 0);
}

const QString QmlExerciseEntry::exerciseName2() const
{
	return m_workoutModel->exerciseName(m_exerciseNumber, 1);
}

void QmlExerciseEntry::setExerciseName1(const QString &new_name)
{
	if (new_name != m_workoutModel->exerciseName(m_exerciseNumber, 0))
		m_workoutModel->setExerciseName(m_exerciseNumber, 0, new_name);
}

void QmlExerciseEntry::setExerciseName2(const QString &new_name)
{
	if (new_name != m_workoutModel->exerciseName(m_exerciseNumber, 1))
		m_workoutModel->setExerciseName(m_exerciseNumber, 1, new_name);
}

QString QmlExerciseEntry::setsNumber() const
{
	return QString::number(m_workoutModel->setsNumber(m_exerciseNumber, m_workingSet));
}

void QmlExerciseEntry::addSubExercise(const uint exercise_idx)
{

}

QString QmlExerciseEntry::newSetType1() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	return last_set >= 0 ? m_workoutModel->formatSetTypeToExport(m_workoutModel->setType(m_exerciseNumber, 0, last_set)) :
						m_workoutModel->formatSetTypeToExport(Regular);
}

QString QmlExerciseEntry::newSetType2() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	return last_set >= 0 ? m_workoutModel->formatSetTypeToExport(m_workoutModel->setType(m_exerciseNumber, 1, last_set)) :
						m_workoutModel->formatSetTypeToExport(Regular);
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
			if (!m_workoutPage->mainDateIsToday())
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
	m_workoutPage->removeExerciseObject(m_exerciseNumber, bAsk);
}

void QmlExerciseEntry::exerciseCompleted()
{
	m_workoutPage->gotoNextExercise(m_exerciseNumber);
}

void QmlExerciseEntry::moveExerciseUp()
{
	if (m_exerciseNumber >= 1)
		m_workoutPage->moveExercise(m_exerciseNumber, m_exerciseNumber-1);
}

void QmlExerciseEntry::moveExerciseDown()
{
	if (!lastExercise())
		m_workoutPage->moveExercise(m_exerciseNumber, m_exerciseNumber+1);
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
		QMetaObject::invokeMethod(m_workoutPage, "showRemoveSetMessage");
	else
	{
		const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
		const uint set_number{m_workoutModel->workingSet(m_exerciseNumber, exercise_idx)};
		m_workoutModel->delSet(m_exerciseNumber, exercise_idx, set_number);
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->deleteLater();
		m_exercisesIdxs.at(exercise_idx)->m_setObjects.remove(set_number);
		for(uint i{set_number}; i < m_exercisesIdxs.at(exercise_idx)->m_setObjects.count(); ++i)
			moveSet(i-1, i);

		if (m_exercisesIdxs.at(exercise_idx)->m_setObjects.count() == 0)
			setCanEditRestTimeTracking(true);
		else
		{
			if (set_number == m_exercisesIdxs.at(exercise_idx)->m_setObjects.count()) //last set was removed
				m_exercisesIdxs.at(exercise_idx)->m_setObjects.last()->setLastSet(true);
			exerciseCompleted();
		}
		findCurrentSet();
	}
}

void QmlExerciseEntry::moveSet(const uint set_number, const uint new_set_number)
{
	const uint exercise_idx{m_workoutModel->workingSubExercise(m_exerciseNumber)};
	m_workoutModel->moveSet(m_exerciseNumber, exercise_idx, set_number, new_set_number);
	m_exercisesIdxs.at(exercise_idx)->m_setObjects.swapItemsAt(set_number, new_set_number);

	for (const auto set : std::as_const(m_exercisesIdxs.at(exercise_idx)->m_setObjects))
		set->setEntry()->setParentItem(nullptr);

	m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setNumber(set_number);
	m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(new_set_number)->setNumber(new_set_number);
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
		set->setEntry()->setParentItem(m_setsLayout);
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
			{
				m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(set_number)->setType(new_type); //all types supported by SetTypeRegular.qml only require a simple property change
				return;
			}
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
	switch(setObj->mode())
	{
		case SET_MODE_UNDEFINED:
			if (set_number == 0 || !setObj->trackRestTime())
			{
				setObj->setCompleted(true);
				setObj->setMode(SET_MODE_SET_COMPLETED);
				findCurrentSet();
			}
			else
				setObj->setMode(SET_MODE_START_REST);
			break;
		case SET_MODE_START_REST:
			setObj->setMode(SET_MODE_START_EXERCISE);
			if (setObj->autoRestTime())
				startRestTimer(set_number, "00:00"_L1, true);
			else
				startRestTimer(set_number, m_workoutModel->setRestTime(m_exerciseNumber, exercise_idx, set_number), false);
		break;
		case SET_MODE_START_EXERCISE:
			stopRestTimer(set_number);
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
	const uint set_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
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
	const uint set_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
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
	const uint set_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
	const uint nsets{m_workoutModel->setsNumber(m_exerciseNumber, exercise_idx)};

	for (uint i{set_number+1}; i < nsets; ++i)
	{
		if (!m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->completed())
		{
			m_exercisesIdxs.at(exercise_idx)->m_setObjects.at(i)->setWeight(m_workoutModel->suggestedReps(
						m_workoutModel->setWeight(m_exerciseNumber, exercise_idx, i-1, sub_set),
						m_workoutModel->setType(m_exerciseNumber, exercise_idx, i)));
		}
	}
}

void QmlExerciseEntry::simpleExercisesList(const bool show, const bool multi_sel)
{
	m_workoutPage->simpleExercisesList(show, multi_sel);
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
		for (auto error : m_setComponents[set_type_cpp]->errors())
			qDebug() << error.description();
		return;
	}
	#endif

	QmlSetEntry *newSetEntry{new QmlSetEntry{this, this, m_workoutModel, exercise_idx, set_number}};
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
	qml_set_object->setProperty("Layout.row", set->exerciseIdx());
	qml_set_object->setProperty("Layout.column", 0);

	if (m_setsToBeCreated == 0)
	{
		//TODO
		setWorkingSet(m_exercisesIdxs.first()->m_setObjects.first());
		m_exerciseEntry->setProperty("showSets", true);
		QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView",
					Q_ARG(int, exerciseEntry()->y() + exerciseEntry()->height() + qml_set_object->y() + qml_set_object->height()));
	}
}

inline uint QmlExerciseEntry::findSetMode(const uint exercise_idx, const uint set_number) const
{
	if (!m_workoutModel->setCompleted(m_exerciseNumber, exercise_idx, set_number))
	{
		int ret(SET_MODE_UNDEFINED);
		if (set_number > 0)
		{
			if (m_workoutModel->trackRestTime(m_exerciseNumber) || m_workoutModel->autoRestTime(m_exerciseNumber))
				ret =  SET_MODE_START_REST;
		}
		return ret;
	}
	return SET_MODE_SET_COMPLETED;
}

void QmlExerciseEntry::startRestTimer(const bool bStopWatch)
{
	TPTimer *set_timer(m_workoutPage->restTimer());
	if (!set_timer->isActive())
	{
		set_timer->setStopWatch(bStopWatch);
		set_timer->prepareTimer(m_workoutModel->setRestTime());
		m_workingSet->setRestTime(startTime);
		connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer] () {
			m_workingSet->setRestTime(set_timer->strMinutes() + ':' + set_timer->strSeconds(), false);
		});
		set_timer->startTimer();
	}
	else
		m_workoutPage->displayMessage(tr("Cannot start timer!"), tr("Another set is using it"), false, 3000);
}

void QmlExerciseEntry::stopRestTimer(const uint set_number)
{
	TPTimer *set_timer(m_workoutPage->restTimer());
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
