#include "qmlexerciseentry.h"
#include "qmltdayinterface.h"
#include "qmlsetentry.h"
#include "dbtrainingdaymodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "tptimer.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

static const QStringList& setTypePages{QStringList() << u"qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_qs <<
					u"qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_qs << u"qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_qs};

QmlExerciseEntry::~QmlExerciseEntry()
{
	if (m_setComponents[0])
		delete m_setComponents[0];
	if (m_setComponents[1])
		delete m_setComponents[1];
	if (m_setComponents[2])
		delete m_setComponents[2];
}

void QmlExerciseEntry::setExerciseEntry(QQuickItem* item)
{
	m_exerciseEntry = item;
	m_setsLayout = m_exerciseEntry->findChild<QQuickItem*>(u"exerciseSetsLayout"_qs);
};

void QmlExerciseEntry::setNewSetType(const uint new_value)
{
	m_type = new_value;
	emit newSetTypeChanged();

	const uint nsets(nSets());
	QString strSets(STR_ONE);

	if (nsets == 0)
	{
		switch(m_type)
		{
			case SET_TYPE_DROP: strSets = STR_ONE; break;
			case SET_TYPE_CLUSTER: strSets = u"2"_qs; break;
			case SET_TYPE_MYOREPS: strSets = u"3"_qs; break;
			default: strSets = u"4"_qs; break;
		}
		setSetsNumber(strSets);
	}
	setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_type, nsets));
	setRepsForExercise1(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, m_type, nsets, 0));
	setWeightForExercise1(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, m_type, nsets, 0));
	if (m_type == SET_TYPE_GIANT)
	{
		setRepsForExercise2(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, m_type, nsets, 1));
		setWeightForExercise2(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, m_type, nsets, 1));
	}
}

void QmlExerciseEntry::setExerciseIdx(const uint new_value)
{
	m_exercise_idx = new_value;
	m_exerciseEntry->setProperty("Layout.row", new_value);
	emit exerciseIdxChanged();
	emit exerciseNumberChanged();
}

const QString QmlExerciseEntry::exerciseName() const
{
	return m_tDayModel->exerciseName(m_exercise_idx).replace(comp_exercise_separator, comp_exercise_fancy_separator);
}

void QmlExerciseEntry::setExerciseName(const QString& new_value)
{
	m_tDayModel->setExerciseName(m_exercise_idx, new_value);
	emit exerciseNameChanged();
	setCompositeExercise(new_value.contains('+') || new_value.contains(comp_exercise_separator));
}

QString QmlExerciseEntry::repsForExercise1()
{
	return appUtils()->getCompositeValue(0, m_reps, comp_exercise_separator);
}

void QmlExerciseEntry::setRepsForExercise1(const QString& new_value)
{
	appUtils()->setCompositeValue(0, new_value, m_reps, comp_exercise_separator);
	emit repsForExercise1Changed();
}

QString QmlExerciseEntry::weightForExercise1()
{
	return appUtils()->getCompositeValue(0, m_weight, comp_exercise_separator);
}

void QmlExerciseEntry::setWeightForExercise1(const QString& new_value)
{
	appUtils()->setCompositeValue(0, new_value, m_weight, comp_exercise_separator);
	emit weightForExercise1Changed();
}

QString QmlExerciseEntry::repsForExercise2()
{
	return appUtils()->getCompositeValue(1, m_reps, comp_exercise_separator);
}

void QmlExerciseEntry::setRepsForExercise2(const QString& new_value)
{
	appUtils()->setCompositeValue(1, new_value, m_reps, comp_exercise_separator);
	emit repsForExercise2Changed();
}

QString QmlExerciseEntry::weightForExercise2()
{
	return appUtils()->getCompositeValue(1, m_weight, comp_exercise_separator);
}

void QmlExerciseEntry::setWeightForExercise2(const QString& new_value)
{
	appUtils()->setCompositeValue(1, new_value, m_weight, comp_exercise_separator);
	emit weightForExercise2Changed();
}

void QmlExerciseEntry::setTrackRestTime(const bool new_value)
{
	m_bTrackRestTime = new_value;
	emit trackRestTimeChanged();
	m_tDayModel->setTrackRestTime(m_bTrackRestTime, m_exercise_idx);
	if (!m_bTrackRestTime)
		setAutoRestTime(false);
}

void QmlExerciseEntry::setAutoRestTime(const bool new_value)
{
	m_bAutoRestTime = new_value;
	emit autoRestTimeChanged();
	m_tDayModel->setAutoRestTime(m_exercise_idx, m_bAutoRestTime);
	if (m_bAutoRestTime)
		setRestTime(u"00:00"_qs);
	else
	{
		if (restTime() == u"00::00"_qs)
			setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_type, m_setObjects.count() - 1));
	}
}

void QmlExerciseEntry::createAvailableSets()
{
	if (m_setObjects.isEmpty())
	{
		m_expectedSetNumber = 0;
		const uint nsets(m_tDayModel->setsNumber(m_exercise_idx));
		uint i(0);
		for (uint i(0); i < nsets; ++i)
		{
			m_expectedSetNumber = i;
			createSetObject(i, m_tDayModel->setType(m_exercise_idx, i), m_tDayModel->setRestTime(m_exercise_idx, i),
				m_tDayModel->setReps(m_exercise_idx, i), m_tDayModel->setWeight(m_exercise_idx, i));
		}
		findCurrentSet();
	}
	//else
		//Place into view: exercise entry + first set
	//	QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, exerciseEntryItem(exercise_idx)->property("y").toInt() + 50));
}

void QmlExerciseEntry::appendNewSet()
{
	m_expectedSetNumber = m_setObjects.count();
	createSetObject(m_expectedSetNumber, m_type, m_reps, m_weight, m_restTime);
	connect(this, &QmlExerciseEntry::setObjectCreated, this, [this] {
			setNewSetType(m_tDayModel->setType(m_exercise_idx, m_setObjects.count() - 1));
			findCurrentSet();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

void QmlExerciseEntry::removeSetObject(const uint set_number)
{
	m_tDayModel->removeSet(set_number, m_exercise_idx);
	m_setObjects.at(set_number)->deleteLater();
	m_setObjects.removeAt(set_number);
	for(uint i(set_number); i < m_setObjects.count(); ++i)
	{
		m_setObjects.at(i)->setNumber(i);
		m_setObjects.at(i)->setEntry()->setProperty("Layout.row", i);
	}

	if (m_setObjects.count() == 0)
		setCanEditRestTimeTracking(true);
	else
	{
		if (set_number == m_setObjects.count()) //last set was removed
		{
			m_setObjects.last()->setLastSet(true);
			setNewSetType(m_setObjects.last()->type()); //update visual controls of exercise entry
		}
		enableDisableExerciseCompletedButton();
	}
	findCurrentSet();
}

void QmlExerciseEntry::changeSetType(const uint set_number, const uint new_type)
{
	if (new_type != 100)
	{
		const uint current_type(m_setObjects.at(set_number)->type());
		m_tDayModel->changeSetType(m_exercise_idx, set_number, current_type, new_type);
		if (current_type != SET_TYPE_DROP && current_type != SET_TYPE_GIANT)
		{
			if (new_type != SET_TYPE_DROP && new_type != SET_TYPE_GIANT)
			{
				m_setObjects.at(set_number)->setType(new_type); //all types supported by SetTypeRegular.qml only require a simple property change
				return;
			}
		}
		removeSetObject(set_number);

		m_expectedSetNumber = 100; //do not add the object to the parent layout
		connect(this, &QmlExerciseEntry::setObjectCreated, this, [this,set_number] {
			changeSetType(set_number, 100);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));

		const QString& restTime{m_tDayModel->nextSetSuggestedTime(m_exercise_idx, new_type, set_number)};
		QString nreps{m_tDayModel->nextSetSuggestedReps(m_exercise_idx, new_type, set_number, 0)};
		QString weight{m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, new_type, set_number, 0)};
		if (m_type == SET_TYPE_GIANT)
		{
			nreps += comp_exercise_separator + m_tDayModel->nextSetSuggestedReps(m_exercise_idx, new_type, set_number, 1);
			weight += comp_exercise_separator + m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, new_type, set_number, 1);
		}
		createSetObject(set_number, new_type, restTime, nreps, weight);
	}
	else
	{
		const uint nSets(m_setObjects.count());
		for(uint i(0); i < nSets; ++i)
			m_setObjects.at(i)->setEntry()->setParentItem(nullptr);
		for(uint i(0); i < nSets; ++i)
		{
			m_setObjects.at(i)->setEntry()->setParentItem(m_setsLayout);
			if (i == set_number)
			{
				m_setObjects.at(i)->setEntry()->setProperty("Layout.row", set_number);
				m_setObjects.at(i)->setEntry()->setProperty("Layout.column", 0);
			}
		}
	}
}

void QmlExerciseEntry::changeSetMode(const uint set_number)
{
	QmlSetEntry* setObj(m_setObjects.at(set_number));
	switch(setObj->mode())
	{
		case SET_MODE_UNDEFINED:
			if (!setObj->autoRestTime())
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
			startRestTimer(set_number);
		break;
		case SET_MODE_START_EXERCISE:
			setObj->setMode(SET_MODE_SET_COMPLETED);
			stopRestTimer(set_number);
		break;
		case SET_MODE_SET_COMPLETED:
			if (!setObj->autoRestTime())
			{
				setObj->setCompleted(false);
				setObj->setMode(SET_MODE_UNDEFINED);
			}
			else
				setObj->setMode(SET_MODE_START_REST);
	}
}

void QmlExerciseEntry::copyTypeValueIntoOtherSets(const uint set_number)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	const uint set_type(m_tDayModel->setType(set_number, exercise_idx));
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_tDayModel->setCompleted(i, exercise_idx))
		{
			changeSetType(i, exercise_idx, set_type);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeSetType", Q_ARG(int, static_cast<int>(set_type)));
		}
	}
}

void QmlExerciseEntry::copyTimeValueIntoOtherSets(const uint set_number)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_tDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_tDayModel->setType(i, exercise_idx);
			updatedValue = m_tDayModel->nextSetSuggestedTime(exercise_idx, set_type, i-1);
			m_tDayModel->setSetRestTime(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeTime", Q_ARG(QString, updatedValue));
		}
	}
}

void QmlExerciseEntry::copyRepsValueIntoOtherSets(const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_tDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_tDayModel->setType(i, exercise_idx);
			updatedValue = m_tDayModel->nextSetSuggestedReps(exercise_idx, set_type, i-1, sub_set);
			if (set_type == SET_TYPE_DROP || set_type == SET_TYPE_GIANT)
				m_tDayModel->setSetReps(i, exercise_idx, sub_set, updatedValue);
			else
				m_tDayModel->setSetReps(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeReps", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
		}
	}
}

void QmlExerciseEntry::copyWeightValueIntoOtherSets(const uint set_number, const uint sub_set)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	uint set_type(0);
	QString updatedValue;
	const uint nsets(exercise_obj->m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_tDayModel->setCompleted(i, exercise_idx))
		{
			set_type = m_tDayModel->setType(i, exercise_idx);
			updatedValue = m_tDayModel->nextSetSuggestedWeight(exercise_idx, set_type, i-1, sub_set);
			if (set_type == SET_TYPE_DROP || set_type == SET_TYPE_GIANT)
				m_tDayModel->setSetWeight(i, exercise_idx, sub_set, updatedValue);
			else
				m_tDayModel->setSetWeight(i, exercise_idx, updatedValue);
			QMetaObject::invokeMethod(exercise_obj->m_setObjects.at(i), "changeWeight", Q_ARG(QString, updatedValue), Q_ARG(int, sub_set));
		}
	}
}

QQuickItem* QmlExerciseEntry::nextSetObject(const uint set_number) const
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

void QmlExerciseEntry::insertSetEntry(const uint set_number, QmlSetEntry* new_setobject)
{
	if (set_number >= m_setObjects.count())
	{
		for(uint i(m_setObjects.count()); i <= set_number; ++i)
			m_setObjects.insert(i, nullptr);
	}
	m_setObjects[set_number] = new_setobject;
}

void QmlExerciseEntry::createSetObject(const uint set_number, const uint type, const QString& resttime, const QString& nreps, const QString& weight)
{
	const uint set_type_cpp(m_type == SET_TYPE_DROP ? 1 : m_type == SET_TYPE_GIANT ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
	{
		m_setComponents[set_type_cpp] = new QQmlComponent{m_qmlEngine, QUrl{setTypePages.at(set_type_cpp)}, QQmlComponent::Asynchronous};
		m_setObjectProperties.insert(u"exerciseManager"_qs, QVariant::fromValue(this));
		m_setObjectProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_tDayModel));
	}

	if (set_number == 0)
		m_tDayModel->newFirstSet(m_exercise_idx, type, nreps, weight, resttime);
	else
		m_tDayModel->newSet(set_number, m_exercise_idx, type, nreps, weight, resttime);

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [this,set_number,set_type_cpp](QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				createSetObject_part2(set_number, set_type_cpp);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(set_number, set_type_cpp);
}

void QmlExerciseEntry::createSetObject_part2(const uint set_number, const uint set_type_cpp)
{
	#ifdef DEBUG
	if (m_setComponents[set_type_cpp]->status() == QQmlComponent::Error)
	{
		for (uint i(0); i < m_setComponents[set_type_cpp]->errors().count(); ++i)
			qDebug() << m_setComponents[set_type_cpp]->errors().at(i).description();
		return;
	}
	#endif

	QmlSetEntry* newSetEntry{new QmlSetEntry(this, m_tDayModel)};
	newSetEntry->setType(m_tDayModel->setType(m_exercise_idx, set_number));
	newSetEntry->setNumber(set_number);
	newSetEntry->setMode(findSetMode(set_number));
	newSetEntry->setCompleted(m_tDayModel->setCompleted(m_exercise_idx, set_number));
	newSetEntry->setLastSet(set_number >= m_setObjects.count());
	newSetEntry->setFinishButtonEnabled(false);
	newSetEntry->setTrackRestTime(m_tDayModel->trackRestTime(m_exercise_idx));
	newSetEntry->setAutoRestTime(m_tDayModel->autoRestTime(m_exercise_idx));
	insertSetEntry(set_number, newSetEntry);

	m_setObjectProperties.insert(u"setManager"_qs, QVariant::fromValue(newSetEntry));

	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, m_qmlEngine->rootContext())));
	m_qmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	m_setObjects.at(set_number)->setSetEntry(item);

	//Sets may be created at any random order, specially when there are set objects of different kinds within an exercise. m_expectedSetNumber keeps
	//track of the order in which the sets are added. When set_number is greater than m_expectedSetNumber, the set objects are not inserted into
	//the parent layout(with setParentItem). When the expected set_number is finally created, put all sets already in the list (m_setObjects)
	//orderly into the layout
	if (set_number <= m_expectedSetNumber)
	{
		for (uint i(set_number); i < m_setObjects.count(); ++i, ++m_expectedSetNumber)
		{
			if (m_setObjects.at(i)->number() <= i)
			{
				m_setObjects.at(i)->setEntry()->setParentItem(m_setsLayout);
				m_setObjects.at(i)->setEntry()->setProperty("Layout.row", i);
				m_setObjects.at(i)->setEntry()->setProperty("Layout.column", 0);
			}
		}
	}

	if (newSetEntry->type() == SET_TYPE_DROP)
		QMetaObject::invokeMethod(item, "init");
	emit setObjectCreated(set_number);
}

inline uint QmlExerciseEntry::findSetMode(const uint set_number) const
{
	return set_number > 0 ? (m_tDayModel->autoRestTime(m_exercise_idx) ? 1 : 0) : 0;
}

inline void QmlExerciseEntry::findCurrentSet()
{
	uint ret(0);
	bool current(false);
	for(uint i(0); i < m_setObjects.count(); ++i)
	{
		current = !m_tDayModel->setCompleted(m_exercise_idx, i);
		if (!current)
			current = i > 0 ? m_tDayModel->setCompleted(m_exercise_idx, i) : true;
		m_setObjects.at(i)->setCurrent(current);
		if (current)
			break;
	}
}
void QmlExerciseEntry::enableDisableExerciseCompletedButton()
{
	bool bNoSetsCompleted(false);
	bool bAllSetsCompleted(true);
	for (uint i(0); i < m_setObjects.count() - 1; ++i)
	{
		m_setObjects.at(i)->setFinishButtonEnabled(false);
		bNoSetsCompleted |= m_setObjects.at(i)->completed();
		bAllSetsCompleted &= m_setObjects.at(i)->completed();
	}
	m_setObjects.last()->setFinishButtonEnabled(bAllSetsCompleted);
	setCanEditRestTimeTracking(!bNoSetsCompleted);
}

void QmlExerciseEntry::startRestTimer(const uint set_number)
{
	TPTimer* set_timer(m_tDayPage->getTimer());
	if (set_timer)
	{
		set_timer->setInterval(1000);
		set_timer->setStopWatch(true);
		set_timer->prepareTimer(u"-"_qs);
		QQuickItem* set_object(m_setObjects.at(set_number)->setEntry());
		connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer,set_object] () {
			QMetaObject::invokeMethod(set_object, "updateRestTime", Q_ARG(QString, set_timer->strMinutes() + ':' + set_timer->strSeconds()));
		});
		connect(set_timer, &TPTimer::minutesChanged, this, [this,set_timer,set_object] () {
			QMetaObject::invokeMethod(set_object, "updateRestTime", Q_ARG(QString, set_timer->strMinutes() + ':' + set_timer->strSeconds()));
		});
		set_timer->startTimer(u"-"_qs);
	}
	else
		m_tDayPage->displayMessage(tr("Cannot start timer!"), tr("Another set is using it"), false, 3000);
}

void QmlExerciseEntry::stopRestTimer(const uint set_number)
{
	TPTimer* set_timer(m_tDayPage->getTimer());
	if (set_timer->isActive())
	{
		set_timer->stopTimer();
		disconnect(set_timer, nullptr, nullptr, nullptr);
		m_tDayModel->setSetRestTime(m_exercise_idx, set_number, set_timer->strMinutes() + ':' + set_timer->strSeconds());
	}
}
