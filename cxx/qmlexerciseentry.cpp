#include "qmlexerciseentry.h"
#include "qmltdayinterface.h"
#include "qmlsetentry.h"
#include "dbtrainingdaymodel.h"
#include "tpglobals.h"
#include "tputils.h"
#include "tptimer.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

static const QString setTypePages[3] { std::move("qrc:/qml/ExercisesAndSets/SetTypeRegular.qml"_L1),
					std::move("qrc:/qml/ExercisesAndSets/SetTypeDrop.qml"_L1), std::move("qrc:/qml/ExercisesAndSets/SetTypeGiant.qml"_L1) };

QmlExerciseEntry::~QmlExerciseEntry()
{
	if (m_setComponents[0])
		delete m_setComponents[0];
	if (m_setComponents[1])
		delete m_setComponents[1];
	if (m_setComponents[2])
		delete m_setComponents[2];
	for (uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->deleteLater();
}

void QmlExerciseEntry::setExerciseEntry(QQuickItem* item)
{
	m_exerciseEntry = item;
	m_setsLayout = m_exerciseEntry->findChild<QQuickItem*>("exerciseSetsLayout"_L1);
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
			case SET_TYPE_DROP: strSets = "1"_L1; break;
			case SET_TYPE_CLUSTER: strSets = "2"_L1; break;
			case SET_TYPE_MYOREPS: strSets = "3"_L1; break;
			default: strSets = "4"_L1; break;
		}
		setSetsNumber(strSets);
	}
	const uint dataFromSetNumber(m_setObjects.isEmpty() ? 0 : USE_LAST_SET_DATA);
	setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_type, dataFromSetNumber));
	setRepsForExercise1(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, m_type, dataFromSetNumber, 0));
	setWeightForExercise1(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, m_type, dataFromSetNumber, 0));
	if (m_type == SET_TYPE_GIANT)
	{
		setRepsForExercise2(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, m_type, dataFromSetNumber, 1));
		setWeightForExercise2(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, m_type, dataFromSetNumber, 1));
	}
}

void QmlExerciseEntry::setExerciseIdx(const uint new_value)
{
	m_exercise_idx = new_value;
	m_exerciseEntry->setProperty("Layout.row", new_value);
	emit exerciseIdxChanged();
	emit exerciseNumberChanged();
	for(uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setExerciseIdx(m_exercise_idx);
}

void QmlExerciseEntry::setExerciseName(const QString& new_value, const bool bFromQML)
{
	if (new_value != m_name)
	{
		m_name = new_value;
		m_name.replace(comp_exercise_separator, comp_exercise_fancy_separator);
		setCompositeExercise(m_name.contains('+'));
		if (bFromQML)
		{
			m_tDayModel->setExerciseName(m_exercise_idx, new_value);
			emit exerciseNameChanged();
		}
		else
		{
			if (compositeExercise())
			{
				for (uint i(0); i < m_setObjects.count(); ++i)
				{
					QmlSetEntry* const setObj(m_setObjects.at(i));
					if (setObj->type() == SET_TYPE_GIANT)
					{
						setObj->setExerciseName1(appUtils()->getCompositeValue(0, new_value, comp_exercise_separator), false);
						setObj->setExerciseName2(appUtils()->getCompositeValue(1, new_value, comp_exercise_separator), false);
					}
				}
			}
		}
	}
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

const bool QmlExerciseEntry::hasSets() const
{
	return m_tDayModel->setsNumber(m_exercise_idx);
}

void QmlExerciseEntry::setIsEditable(const bool new_value)
{
	m_bEditable = new_value;
	emit isEditableChanged();
	for (uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setIsEditable(m_bEditable);
}

void QmlExerciseEntry::setTrackRestTime(const bool new_value)
{
	m_bTrackRestTime = new_value;
	emit trackRestTimeChanged();
	m_tDayModel->setTrackRestTime(m_exercise_idx, m_bTrackRestTime);
	for (uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setTrackRestTime(m_bTrackRestTime);
	if (!m_bTrackRestTime)
		setAutoRestTime(false);
}

void QmlExerciseEntry::setAutoRestTime(const bool new_value)
{
	m_bAutoRestTime = new_value;
	emit autoRestTimeChanged();
	m_tDayModel->setAutoRestTime(m_exercise_idx, m_bAutoRestTime);
	for (uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setAutoRestTime(m_bAutoRestTime);
	if (m_bAutoRestTime)
		setRestTime("00:00"_L1);
	else
	{
		if (restTime() == "00:00"_L1)
			setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_type, m_setObjects.count() - 1));
	}
}

void QmlExerciseEntry::createAvailableSets()
{
	if (m_setObjects.isEmpty())
	{
		const uint nsets(m_tDayModel->setsNumber(m_exercise_idx));
		for (uint i(0); i < nsets; ++i)
		{
			m_expectedSetNumber = i;
			createSetObject(i, m_tDayModel->setType(m_exercise_idx, i));
		}
		emit hasSetsChanged();
		setNewSetType(m_tDayModel->setType(m_exercise_idx, m_setObjects.count() - 1));
	}
	m_tDayPage->gotoNextExercise(0);
}

void QmlExerciseEntry::removeExercise(const bool bAsk)
{
	m_tDayPage->removeExerciseObject(m_exercise_idx, bAsk);
}

void QmlExerciseEntry::exerciseCompleted()
{
	enableDisableExerciseCompletedButton();
	if (m_bIsCompleted)
		m_tDayPage->gotoNextExercise(m_exercise_idx);
}

void QmlExerciseEntry::appendNewSet()
{
	const uint nsets(nSets());
	if (nsets > 0)
	{
		if (!m_setObjects.isEmpty())
			m_setObjects.last()->setLastSet(false);

		for (uint i(0); i < nsets; ++i)
		{
			m_expectedSetNumber = i;
			if (i == 0)
				m_tDayModel->newFirstSet(m_exercise_idx, newSetType(), reps(), weight(), restTime());
			else
				m_tDayModel->newSet(m_exercise_idx, m_expectedSetNumber, newSetType(), reps(), weight(), restTime());
			createSetObject(i, newSetType());
		}
		emit hasSetsChanged();
		setNewSetType(m_tDayModel->setType(m_exercise_idx, m_setObjects.count() - 1));
	}
}

void QmlExerciseEntry::removeSetObject(const uint set_number, const bool bAsk)
{
	if (bAsk)
		m_tDayPage->askRemoveSet(m_exercise_idx, set_number);
	else
	{
		m_tDayModel->removeSet(set_number, m_exercise_idx);
		m_setObjects.at(set_number)->deleteLater();
		m_setObjects.removeAt(set_number);
		for(uint i(set_number); i < m_setObjects.count(); ++i)
			moveSet(i-1, i);

		if (m_setObjects.count() == 0)
			setCanEditRestTimeTracking(true);
		else
		{
			if (set_number == m_setObjects.count()) //last set was removed
			{
				m_setObjects.last()->setLastSet(true);
				setNewSetType(m_setObjects.last()->type()); //update visual controls of exercise entry
			}
			exerciseCompleted();
		}
		findCurrentSet();
	}
}

void QmlExerciseEntry::moveSet(const uint set_number, const uint new_set_number)
{
	for(uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setEntry()->setParentItem(nullptr);
	m_setObjects.swapItemsAt(set_number, new_set_number);
	for(uint i(0); i < m_setObjects.count(); ++i)
		m_setObjects.at(i)->setEntry()->setParentItem(m_setsLayout);
	m_setObjects.at(set_number)->setNumber(set_number);
	m_setObjects.at(new_set_number)->setNumber(new_set_number);
	if (set_number == m_setObjects.count() - 1)
	{
		m_setObjects.at(set_number)->setLastSet(true);
		m_setObjects.at(new_set_number)->setLastSet(false);
	}
	else if (new_set_number == m_setObjects.count() - 1)
	{
		m_setObjects.at(set_number)->setLastSet(false);
		m_setObjects.at(new_set_number)->setLastSet(true);
	}
}

void QmlExerciseEntry::changeSetType(const uint set_number, const uint new_type)
{
	if (new_type != USE_LAST_SET_DATA)
	{
		const uint current_type(m_setObjects.at(set_number)->type());
		if (current_type != new_type)
		{
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

			m_expectedSetNumber = USE_LAST_SET_DATA; //do not add the object to the parent layout
			connect(this, &QmlExerciseEntry::setObjectCreated, this, [this,set_number] {
				changeSetType(set_number, USE_LAST_SET_DATA);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));

			const QString& strRestTime{m_tDayModel->nextSetSuggestedTime(m_exercise_idx, new_type, set_number)};
			QString strReps{std::move(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, new_type, set_number, 0))};
			QString strWeight{std::move(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, new_type, set_number, 0))};
			if (m_type == SET_TYPE_GIANT)
			{
				strReps += comp_exercise_separator + std::move(m_tDayModel->nextSetSuggestedReps(m_exercise_idx, new_type, set_number, 1));
				strWeight += comp_exercise_separator + std::move(m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, new_type, set_number, 1));
			}
			m_tDayModel->setSetRestTime(m_exercise_idx, set_number, strRestTime);
			m_tDayModel->setSetReps(m_exercise_idx, set_number, strReps);
			m_tDayModel->setSetWeight(m_exercise_idx, set_number, strWeight);
			createSetObject(set_number, new_type);
		}
	}
	else
	{
		const uint nsets(m_setObjects.count());
		for(uint i(0); i < nsets; ++i)
			m_setObjects.at(i)->setEntry()->setParentItem(nullptr);
		for(uint i(0); i < nsets; ++i)
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
			if (set_number == 0 || !setObj->trackRestTime())
			{
				setObj->setCompleted(true);
				setObj->setMode(SET_MODE_SET_COMPLETED);
				m_tDayModel->setSetCompleted(m_exercise_idx, set_number, true);
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
				startRestTimer(set_number, m_tDayModel->setRestTime(m_exercise_idx, set_number), false);
		break;
		case SET_MODE_START_EXERCISE:
			setObj->setMode(SET_MODE_SET_COMPLETED);
			m_tDayModel->setSetCompleted(m_exercise_idx, set_number, true);
			stopRestTimer(set_number);
		break;
		case SET_MODE_SET_COMPLETED:
			if (set_number == 0 || !setObj->autoRestTime())
			{
				setObj->setCompleted(false);
				setObj->setMode(SET_MODE_UNDEFINED);
			}
			else
				setObj->setMode(SET_MODE_START_REST);
			m_tDayModel->setSetCompleted(m_exercise_idx, set_number, false);
	}
}

void QmlExerciseEntry::copyTypeValueIntoOtherSets(const uint set_number)
{
	const uint set_type(m_setObjects.at(set_number)->type());
	const uint nsets(m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_setObjects.at(i)->completed())
			changeSetType(i, set_type);
	}
}

void QmlExerciseEntry::copyTimeValueIntoOtherSets(const uint set_number)
{
	const uint set_type(m_setObjects.at(set_number)->type());
	const uint nsets(m_setObjects.count());

	QString strRestTime;
	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_setObjects.at(i)->completed())
		{
			strRestTime = m_tDayModel->nextSetSuggestedTime(m_exercise_idx, set_type, i-1);
			m_setObjects.at(i)->setRestTime(strRestTime);
		}
	}
}

void QmlExerciseEntry::copyRepsValueIntoOtherSets(const uint set_number, const uint sub_set)
{
	const uint set_type(m_setObjects.at(set_number)->type());
	const uint nsets(m_setObjects.count());

	QString strReps;
	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_setObjects.at(i)->completed())
		{
			strReps = m_tDayModel->nextSetSuggestedReps(m_exercise_idx, set_type, i-1, sub_set);
			m_tDayModel->setSetReps(m_exercise_idx, i, sub_set, strReps);
		}
	}
}

void QmlExerciseEntry::copyWeightValueIntoOtherSets(const uint set_number, const uint sub_set)
{
	const uint set_type(m_setObjects.at(set_number)->type());
	const uint nsets(m_setObjects.count());

	QString strWeight;
	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_setObjects.at(i)->completed())
		{
			strWeight = m_tDayModel->nextSetSuggestedWeight(m_exercise_idx, set_type, i-1, sub_set);
			m_tDayModel->setSetWeight(m_exercise_idx, i, sub_set, strWeight);
		}
	}
}

void QmlExerciseEntry::simpleExercisesList(const bool show, const bool multi_sel)
{
	if (show)
		m_tDayPage->showSimpleExercisesList(m_exercise_idx, multi_sel);
	else
		m_tDayPage->hideSimpleExercisesList();
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

void QmlExerciseEntry::createSetObject(const uint set_number, const uint type)
{
	const uint set_type_cpp(type == SET_TYPE_DROP ? 1 : type == SET_TYPE_GIANT ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
	{
		m_setComponents[set_type_cpp] = new QQmlComponent{m_qmlEngine, QUrl{setTypePages[set_type_cpp]}, QQmlComponent::Asynchronous};
		m_setObjectProperties.insert("exerciseManager"_L1, QVariant::fromValue(this));
	}

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
	#ifndef QT_NO_DEBUG
	if (m_setComponents[set_type_cpp]->status() == QQmlComponent::Error)
	{
		for (uint i(0); i < m_setComponents[set_type_cpp]->errors().count(); ++i)
			qDebug() << m_setComponents[set_type_cpp]->errors().at(i).description();
		return;
	}
	#endif

	QmlSetEntry* newSetEntry{new QmlSetEntry(this, this, m_tDayModel, m_exercise_idx)};
	const uint set_type(m_tDayModel->setType(m_exercise_idx, set_number));

	newSetEntry->_setExerciseName(m_tDayModel->exerciseName(m_exercise_idx));
	newSetEntry->_setType(set_type);
	newSetEntry->_setNumber(set_number);
	newSetEntry->_setRestTime(m_tDayModel->setRestTime(m_exercise_idx, set_number));
	newSetEntry->_setReps(m_tDayModel->setReps(m_exercise_idx, set_number));
	newSetEntry->_setWeight(m_tDayModel->setWeight(m_exercise_idx, set_number));
	newSetEntry->_setSubSets(m_tDayModel->setSubSets(m_exercise_idx, set_number));
	newSetEntry->_setNotes(m_tDayModel->setsNotes(m_exercise_idx));
	newSetEntry->_setCompleted(m_tDayModel->setCompleted(m_exercise_idx, set_number));
	newSetEntry->_setLastSet(m_tDayModel->setsNumber(m_exercise_idx) == set_number + 1);
	newSetEntry->setFinishButtonEnabled(m_tDayModel->allSetsCompleted(m_exercise_idx));
	newSetEntry->_setTrackRestTime(m_tDayModel->trackRestTime(m_exercise_idx));
	newSetEntry->_setAutoRestTime(m_tDayModel->autoRestTime(m_exercise_idx));
	newSetEntry->_setHasSubSets(set_type == SET_TYPE_CLUSTER || set_type == SET_TYPE_DROP);
	newSetEntry->_setMode(findSetMode(set_number));
	if (!m_tDayPage->mainDateIsToday())
	{
		newSetEntry->setCurrent(false);
		newSetEntry->setIsEditable(isEditable());
	}
	else
		findCurrentSet();
	insertSetEntry(set_number, newSetEntry);

	m_setObjectProperties.insert("setManager"_L1, QVariant::fromValue(newSetEntry));

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
	emit setObjectCreated(set_number);

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this, newSetEntry] () {
		emit newSetEntry->modeLabelChanged();
	});
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
	m_bIsCompleted = bAllSetsCompleted;
}

inline uint QmlExerciseEntry::findSetMode(const uint set_number) const
{
	if (!m_tDayModel->setCompleted(m_exercise_idx, set_number))
	{
		int ret(SET_MODE_UNDEFINED);
		if (set_number > 0)
		{
			if (m_tDayModel->trackRestTime(m_exercise_idx) || m_tDayModel->autoRestTime(m_exercise_idx))
				ret =  SET_MODE_START_REST;
		}
		return ret;
	}
	return SET_MODE_SET_COMPLETED;
}

inline void QmlExerciseEntry::findCurrentSet()
{
	if (m_tDayPage->mainDateIsToday())
	{
		for(uint i(0); i < m_setObjects.count(); ++i)
		{
			if (!m_tDayModel->setCompleted(m_exercise_idx, i))
			{
				m_setObjects.at(i)->setCurrent(true);
				m_setObjects.at(i)->setIsEditable(true);
				const QQuickItem* const setObj(m_setObjects.at(i)->setEntry());
				QMetaObject::invokeMethod(m_tDayPage->tDayPage(), "placeSetIntoView", Q_ARG(int, setObj->y() + setObj->height()));
				break;
			}
			else
			{
				m_setObjects.at(i)->setCurrent(false);
				m_setObjects.at(i)->setIsEditable(true);
			}
		}
	}
}

void QmlExerciseEntry::startRestTimer(const uint set_number, const QString& startTime, const bool bStopWatch)
{
	TPTimer* set_timer(m_tDayPage->restTimer());
	if (!set_timer->isActive())
	{
		set_timer->setStopWatch(bStopWatch);
		set_timer->prepareTimer(startTime);
		QmlSetEntry* const setObj(m_setObjects.at(set_number));
		setObj->setRestTime(startTime);
		connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer,setObj] () {
			setObj->setRestTime(set_timer->strMinutes() + ':' + set_timer->strSeconds());
		});
		set_timer->startTimer();
	}
	else
		m_tDayPage->displayMessage(tr("Cannot start timer!"), tr("Another set is using it"), false, 3000);
}

void QmlExerciseEntry::stopRestTimer(const uint set_number)
{
	TPTimer* set_timer(m_tDayPage->restTimer());
	if (set_timer->isActive())
	{
		set_timer->stopTimer();
		disconnect(set_timer, nullptr, nullptr, nullptr);
		QmlSetEntry* const setObj(m_setObjects.at(set_number));
		if (setObj->autoRestTime())
			setObj->setRestTime(setObj->restTime(), false); //update the model with the current value displayed
		else
			setObj->setRestTime(appUtils()->formatTime(set_timer->elapsedTime(), false, true), false);
	}
}