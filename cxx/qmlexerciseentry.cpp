#include "qmlexerciseentry.h"

#include "dbexercisesmodel.h"
#include "qmlexerciseinterface.h"
#include "qmlitemmanager.h"
#include "qmlsetentry.h"
#include "qmlworkoutinterface.h"

#include "tpglobals.h"
#include "tptimer.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

#define CPP_SET_TYPE_REGULAR 0
#define CPP_SET_TYPE_DROP 0
#define CPP_SET_TYPE_GIANT 0

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
		delete m_setObjects.at(i);
	delete m_exerciseEntry;
}

void QmlExerciseEntry::setExerciseEntry(QQuickItem *item)
{
	m_exerciseEntry = item;
	m_setsLayout = m_exerciseEntry->findChild<QQuickItem*>("exerciseSetsLayout"_L1);
};

void QmlExerciseEntry::setExerciseNumber(const uint new_value)
{
	m_exerciseNumber = new_value;
	m_exerciseEntry->setProperty("Layout.row", new_value);
	emit exerciseNumberChanged();
	for (const auto set : std::as_const(m_setObjects))
		set->setExerciseNumber(m_exerciseNumber);
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
	return QString::number(m_workoutModel->setsNumber(m_exerciseNumber));
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

QString QmlExerciseEntry::newSetRestTime1() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return appUtils()->formatTime(m_workoutModel->suggestedRestTime(appUtils()->getTimeFromTimeString(
			m_workoutModel->setRestTime(m_exerciseNumber, 0, last_set)), m_workoutModel->setType(m_exerciseNumber, 0, last_set)));
	else
		return appUtils()->formatTime(m_workoutModel->suggestedRestTime(QTime{0, 1, 0}, Regular));
}

QString QmlExerciseEntry::newSetRestTime2() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return appUtils()->formatTime(m_workoutModel->suggestedRestTime(appUtils()->getTimeFromTimeString(
			m_workoutModel->setRestTime(m_exerciseNumber, 1, last_set)), m_workoutModel->setType(m_exerciseNumber, 1, last_set)));
	else
		return appUtils()->formatTime(m_workoutModel->suggestedRestTime(QTime{0, 1, 0}, Regular));
}

QString QmlExerciseEntry::newSetReps1() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return m_workoutModel->suggestedReps(m_workoutModel->setReps(m_exerciseNumber, 0, last_set),
											 m_workoutModel->setType(m_exerciseNumber, 0, last_set));
	else
		return m_workoutModel->suggestedReps(QString{}, Regular);
}

QString QmlExerciseEntry::newSetReps2() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return m_workoutModel->suggestedReps(m_workoutModel->setReps(m_exerciseNumber, 1, last_set),
											 m_workoutModel->setType(m_exerciseNumber, 1, last_set));
	else
		return m_workoutModel->suggestedReps(QString{}, Regular);
}

QString QmlExerciseEntry::newSetWeight1() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return m_workoutModel->suggestedReps(m_workoutModel->setWeight(m_exerciseNumber, 0, last_set),
											 m_workoutModel->setType(m_exerciseNumber, 0, last_set));
	else
		return m_workoutModel->suggestedWeight(QString{}, Regular);
}

QString QmlExerciseEntry::newSetWeight2() const
{
	const int last_set{static_cast<int>(m_workoutModel->setsNumber(m_exerciseNumber) - 1)};
	if (last_set >= 0)
		return m_workoutModel->suggestedReps(m_workoutModel->setWeight(m_exerciseNumber, 1, last_set),
											 m_workoutModel->setType(m_exerciseNumber, 1, last_set));
	else
		return m_workoutModel->suggestedWeight(QString{}, Regular);
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
	return m_workoutModel->setsNumber(m_exerciseNumber) > 0;
}

void QmlExerciseEntry::setIsEditable(const bool editable)
{
	m_bEditable = editable;
	emit isEditableChanged();
	for (const auto set : std::as_const(m_setObjects))
	{
		set->setIsEditable(m_bEditable);
		if (!m_workoutPage->mainDateIsToday())
			set->setCurrent(m_bEditable);
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
	if (m_setObjects.isEmpty() && hasSets())
	{
		const uint nsets(m_workoutModel->setsNumber(m_exerciseNumber));
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(this, &QmlExerciseEntry::setObjectCreated, this, [this,conn,nsets] (const uint set_number) {
			setCreated(set_number, nsets, conn);
		});
		for (uint i{0}; i < nsets; ++i)
		{
			m_expectedSetNumber = i;
			createSetObject(i);
		}
	}
}

void QmlExerciseEntry::appendNewSet()
{
	const uint set_number{m_workoutModel->addSet(m_exerciseNumber, 0)};
	if (!m_setObjects.isEmpty())
		m_setObjects.last()->setLastSet(false);
	createSetObject(set_number);
}

void QmlExerciseEntry::removeSetObject(const uint set_number, const bool bAsk)
{
	if (bAsk)
		m_workoutPage->askRemoveSet(m_exerciseNumber, set_number);
	else
	{
		if (compositeExercise())
			m_workoutModel->delSet(m_exerciseNumber, 1, set_number, false);
		m_workoutModel->delSet(m_exerciseNumber, 0, set_number);

		m_setObjects.at(set_number)->deleteLater();
		m_setObjects.removeAt(set_number);
		for(uint i{set_number}; i < m_setObjects.count(); ++i)
			moveSet(0, i-1, i);

		if (m_setObjects.count() == 0)
			setCanEditRestTimeTracking(true);
		else
		{
			if (set_number == m_setObjects.count()) //last set was removed
				m_setObjects.last()->setLastSet(true);
			exerciseCompleted();
		}
		findCurrentSet();
	}
}

void QmlExerciseEntry::moveSet(const uint exercise_idx, const uint set_number, const uint new_set_number)
{
	m_workoutModel->moveSet(m_exerciseNumber, exercise_idx, set_number, new_set_number);
	m_setObjects.swapItemsAt(set_number, new_set_number);

	for (const auto set : m_setObjects)
		set->setEntry()->setParentItem(nullptr);

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

	for (const auto set : m_setObjects)
		set->setEntry()->setParentItem(m_setsLayout);
}

void QmlExerciseEntry::changeSetType(const uint exercise_idx, const uint set_number, const uint new_type)
{
	const uint old_type{m_workoutModel->setType(m_exerciseNumber, exercise_idx, set_number)};
	m_workoutModel->changeSetType(m_exerciseNumber, exercise_idx, set_number, new_type);
	if (old_type != new_type)
	{
		if (old_type != Drop)
		{
			if (new_type != Drop)
			{
				m_setObjects.at(set_number)->setType(new_type); //all types supported by SetTypeRegular.qml only require a simple property change
				return;
			}
		}
		removeSetObject(set_number);
		m_expectedSetNumber = -1; //do not add the object to the parent layout
		connect(this, &QmlExerciseEntry::setObjectCreated, this, [this,set_number] {
			const uint nsets(m_setObjects.count());
			for (const auto set : m_setObjects)
				set->setEntry()->setParentItem(nullptr);
			uint i{0};
			for (const auto set : m_setObjects)
			{
				set->setEntry()->setParentItem(m_setsLayout);
				if (i == set_number)
				{
					set->setEntry()->setProperty("Layout.row", set_number);
					set->setEntry()->setProperty("Layout.column", 0);
				}
				++i;
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		createSetObject(set_number);
	}
}

void QmlExerciseEntry::changeSetMode(const uint exercise_idx, const uint set_number)
{
	QmlSetEntry *setObj(m_setObjects.at(set_number));
	switch(setObj->mode())
	{
		case SET_MODE_UNDEFINED:
			if (set_number == 0 || !setObj->trackRestTime())
			{
				changeSetCompleteStatus(set_number, true);
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
			changeSetCompleteStatus(set_number, true);
		break;
		case SET_MODE_SET_COMPLETED:
			if (set_number == 0 || !setObj->autoRestTime())
			{
				const int curSet(findCurrentSet());
				if (curSet >= 0)
					m_setObjects.at(curSet)->setCurrent(false);
				setObj->setCurrent(true);
				changeSetCompleteStatus(set_number, false);
				setObj->setMode(SET_MODE_UNDEFINED);
			}
			else
				setObj->setMode(SET_MODE_START_REST);
	}
}

void QmlExerciseEntry::copyTypeValueIntoOtherSets(const uint set_number)
{
	const uint set_type(m_setObjects.at(set_number)->type());
	const uint nsets(m_setObjects.count());

	for (uint i(set_number+1); i < nsets; ++i)
	{
		if (!m_setObjects.at(i)->completed())
			changeSetType(i, set_type, false);
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
			strRestTime = m_workoutModel->nextSetSuggestedTime(m_exerciseNumber, set_type, i-1);
			m_setObjects.at(i)->setRestTime(strRestTime, false, false);
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
			strReps = m_workoutModel->nextSetSuggestedReps(m_exerciseNumber, set_type, i-1, sub_set);
			switch (sub_set)
			{
				case 0: m_setObjects.at(i)->setReps1(strReps, false); break;
				case 1: m_setObjects.at(i)->setReps2(strReps, false); break;
				case 2: m_setObjects.at(i)->setReps3(strReps, false); break;
				case 3: m_setObjects.at(i)->setReps4(strReps, false); break;
			}
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
			strWeight = m_workoutModel->nextSetSuggestedWeight(m_exerciseNumber, set_type, i-1, sub_set);
			switch (sub_set)
			{
				case 0: m_setObjects.at(i)->setWeight1(strWeight, false); break;
				case 1: m_setObjects.at(i)->setWeight2(strWeight, false); break;
				case 2: m_setObjects.at(i)->setWeight3(strWeight, false); break;
				case 3: m_setObjects.at(i)->setWeight4(strWeight, false); break;
			}
		}
	}
}

void QmlExerciseEntry::simpleExercisesList(const bool show, const bool multi_sel, uint comp_exercise)
{
	m_workoutPage->simpleExercisesList(m_exerciseNumber, show, multi_sel, comp_exercise);
}

void QmlExerciseEntry::insertSetEntry(const uint set_number, QmlSetEntry *new_setobject)
{
	if (set_number >= m_setObjects.count())
	{
		for(uint i(m_setObjects.count()); i <= set_number; ++i)
			m_setObjects.insert(i, nullptr);
	}
	m_setObjects[set_number] = new_setobject;
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

void QmlExerciseEntry::createSetObject(const uint set_number)
{
	const uint set_type_cpp{cppSetType(m_workoutModel->setType(m_exerciseNumber, 0, set_number))};
	if (m_setComponents[set_type_cpp] == nullptr)
	{
		m_setComponents[set_type_cpp] = new QQmlComponent{appQmlEngine(), QUrl{setTypePages[set_type_cpp]}, QQmlComponent::Asynchronous};
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

	QmlSetEntry *newSetEntry{new QmlSetEntry{this, this, m_workoutModel, m_exerciseNumber}};
	const uint set_type(m_workoutModel->setType(m_exerciseNumber, set_number));

	newSetEntry->_setExerciseName(m_workoutModel->exerciseName(m_exerciseNumber));
	newSetEntry->_setType(set_type);
	newSetEntry->_setNumber(set_number);
	newSetEntry->_setRestTime(m_workoutModel->setRestTime(m_exerciseNumber, set_number));
	newSetEntry->_setReps(m_workoutModel->setReps(m_exerciseNumber, set_number));
	newSetEntry->_setWeight(m_workoutModel->setWeight(m_exerciseNumber, set_number));
	newSetEntry->_setSubSets(m_workoutModel->setSubSets(m_exerciseNumber, set_number));
	newSetEntry->_setNotes(m_workoutModel->setsNotes(m_exerciseNumber));
	newSetEntry->_setCompleted(m_workoutModel->setCompleted(m_exerciseNumber, set_number));
	newSetEntry->_setLastSet(m_workoutModel->setsNumber(m_exerciseNumber) == set_number + 1);
	newSetEntry->_setTrackRestTime(m_workoutModel->trackRestTime(m_exerciseNumber));
	newSetEntry->_setAutoRestTime(m_workoutModel->autoRestTime(m_exerciseNumber));
	newSetEntry->_setHasSubSets(set_type == SET_TYPE_CLUSTER || set_type == SET_TYPE_DROP);
	newSetEntry->_setMode(findSetMode(set_number));
	newSetEntry->setIsEditable(isEditable());
	if (!m_workoutPage->mainDateIsToday())
		newSetEntry->setCurrent(true);
	else
		findCurrentSet();
	insertSetEntry(set_number, newSetEntry);

	m_setObjectProperties.insert("setManager"_L1, QVariant::fromValue(newSetEntry));

	QQuickItem *item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
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

void QmlExerciseEntry::setCreated(const uint set_number, const uint nsets, auto conn)
{
	static int sets_created(0);
	if (++sets_created == nsets)
	{
		disconnect(*conn);
		sets_created = 0;
		setNewSetType(m_workoutModel->setType(m_exerciseNumber, m_setObjects.count() - 1));
		emit hasSetsChanged();
		m_exerciseEntry->setProperty("showSets", true);
		const uint view_set(nsets > 1 ? 0 : m_setObjects.count() - 1);
		QQuickItem *setObj(m_setObjects.at(view_set)->setEntry());
		QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, exerciseEntry()->y() + exerciseEntry()->height() + setObj->y() + setObj->height()));
	}
}

inline void QmlExerciseEntry::changeSetCompleteStatus(const uint set_number, const bool bCompleted)
{
	m_setObjects.at(set_number)->setCompleted(bCompleted);
	m_workoutModel->setSetCompleted(m_exerciseNumber, set_number, bCompleted);
	setAllSetsCompleted(m_workoutModel->allSetsCompleted(m_exerciseNumber));
	setCanEditRestTimeTracking(!noSetsCompleted());
}

inline uint QmlExerciseEntry::findSetMode(const uint set_number) const
{
	if (!m_workoutModel->setCompleted(m_exerciseNumber, set_number))
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

inline int QmlExerciseEntry::findCurrentSet()
{
	if (m_workoutPage->mainDateIsToday())
	{
		for(uint i(0); i < m_setObjects.count(); ++i)
		{
			QmlSetEntry *set(m_setObjects.at(i));
			if (!m_workoutModel->setCompleted(m_exerciseNumber, i))
			{
				set->setCurrent(true);
				const QQuickItem *const setObj(set->setEntry());
				QMetaObject::invokeMethod(m_workoutPage->workoutPage(), "placeSetIntoView", Q_ARG(int, setObj->y() + setObj->height()));
				return i;
			}
			else
				set->setCurrent(false);
		}
	}
	return -1;
}

void QmlExerciseEntry::startRestTimer(const uint set_number, const QString &startTime, const bool bStopWatch)
{
	TPTimer *set_timer(m_workoutPage->restTimer());
	if (!set_timer->isActive())
	{
		set_timer->setStopWatch(bStopWatch);
		set_timer->prepareTimer(startTime);
		QmlSetEntry *const setObj(m_setObjects.at(set_number));
		setObj->setRestTime(startTime);
		connect(set_timer, &TPTimer::secondsChanged, this, [this,set_timer,setObj] () {
			setObj->setRestTime(set_timer->strMinutes() + ':' + set_timer->strSeconds());
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
		QmlSetEntry *const setObj(m_setObjects.at(set_number));
		if (setObj->autoRestTime())
			setObj->setRestTime(setObj->restTime(), false); //update the model with the current value displayed
		else
			setObj->setRestTime(appUtils()->formatTime(set_timer->elapsedTime(), TPUtils::TF_QML_DISPLAY_NO_SEC), false);
	}
}

inline bool QmlExerciseEntry::noSetsCompleted() const
{
	for (uint i(0); i < m_setObjects.count(); ++i)
		if (m_setObjects.at(i)->completed())
			return false;
	return true;
}
