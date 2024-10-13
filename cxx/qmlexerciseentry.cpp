#include "qmlexerciseentry.h"
#include "qmlsetentry.h"
#include "dbtrainingdaymodel.h"
#include "tpglobals.h"
#include "tputils.h"

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
	}
	setSetsNumber(strSets);
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
	else
		enableDisableSetsRestTime();
}

void QmlExerciseEntry::setAutoRestTime(const bool new_value)
{
	m_bAutoRestTime = new_value;
	emit autoRestTimeChanged();
	m_tDayModel->setAutoRestTime(m_exercise_idx, false);
	enableDisableSetsRestTime();
}

void QmlExerciseEntry::appendNewSet()
{
	createSetObject(m_type, m_reps, m_weight, m_restTime);
	setNewSetType(m_tDayModel->setType(m_exercise_idx, m_setObjects.count() - 1));
	findCurrentSet();
}

void QmlExerciseEntry::createAvailableSets()
{
	if (m_setObjects.isEmpty())
	{
		m_expectedSetNumber = 0;
		const uint nsets(m_tDayModel->setsNumber(m_exercise_idx));
		uint i(0);
		for (uint i(0); i < nsets; ++i)
			createSetObject(m_tDayModel->setType(m_exercise_idx, i), m_tDayModel->setRestTime(m_exercise_idx, i),
				m_tDayModel->setReps(m_exercise_idx, i), m_tDayModel->setWeight(m_exercise_idx, i));
		findCurrentSet();
	}
	//else
		//Place into view: exercise entry + first set
	//	QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, exerciseEntryItem(exercise_idx)->property("y").toInt() + 50));
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

void QmlExerciseEntry::createSetObject(const uint type, const QString& resttime, const QString& nreps, const QString& weight)
{
	const uint set_type_cpp(m_type == SET_TYPE_DROP ? 1 : m_type == SET_TYPE_GIANT ? 2 : 0);
	if (m_setComponents[set_type_cpp] == nullptr)
		m_setComponents[set_type_cpp] = new QQmlComponent{m_qmlEngine, QUrl{setTypePages.at(set_type_cpp)}, QQmlComponent::Asynchronous};

	m_expectedSetNumber = m_setObjects.count();
	if (m_expectedSetNumber == 0)
		m_tDayModel->newFirstSet(m_exercise_idx, type, nreps, weight, resttime);
	else
		m_tDayModel->newSet(m_expectedSetNumber, m_exercise_idx, type, nreps, weight, resttime);

	if (m_setComponents[set_type_cpp]->status() != QQmlComponent::Ready)
		connect(m_setComponents[set_type_cpp], &QQmlComponent::statusChanged, this, [this,set_type_cpp](QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				createSetObject_part2(m_expectedSetNumber, set_type_cpp);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createSetObject_part2(m_expectedSetNumber, set_type_cpp);
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

	m_setObjectProperties.insert(u"itemManager"_qs, QVariant::fromValue(newSetEntry));
	m_setObjectProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_tDayModel));

	QQuickItem* item (static_cast<QQuickItem*>(m_setComponents[set_type_cpp]->
								createWithInitialProperties(m_setObjectProperties, m_qmlEngine->rootContext())));
	m_qmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	m_setObjects.at(set_number)->setSetEntry(item);

	connect(item, SIGNAL(requestTimerDialogSignal(QQuickItem*,QVariant)), this, SLOT(requestTimerDialog(QQuickItem*,QVariant)));
	connect(item, SIGNAL(exerciseCompleted(int)), this, SLOT(exerciseCompleted(int)));
	connect(item, SIGNAL(showRemoveSetMessage(int,int)), this, SLOT(showRemoveSetMessage(int,int)));

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
}

void QmlExerciseEntry::enableDisableSetsRestTime(const uint except_set_number)
{
	for(uint i(1); i < m_setObjects.count(); ++i)
	{
		if (i != except_set_number)
		{
			if (!m_tDayModel->setCompleted(i, m_exercise_idx))
			{
				if (m_bAutoRestTime)
					setRestTime(u"00:00"_qs);
				else if (m_bTrackRestTime)
					setRestTime(m_tDayModel->nextSetSuggestedTime(m_exercise_idx, m_tDayModel->setType(i, m_exercise_idx), i));
				m_tDayModel->setSetRestTime(m_exercise_idx, i, m_restTime);
				m_setObjects.at(i)->setMode(findSetMode(i));
				m_setObjects.at(i)->setTrackRestTime(m_bTrackRestTime);
				m_setObjects.at(i)->setAutoRestTime(m_bAutoRestTime);
			}
		}
	}
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
/*
 * void QmlSetsInterface::enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed)
{
	const tDayExercises::exerciseObject* exercise_obj(m_currentExercises->exerciseObjects.at(exercise_idx));
	const uint nsets(exercise_obj->m_setObjects.count());
	bool noSetsCompleted(true);
	for (uint i(0); i < nsets; ++i)
	{
		QQuickItem* setObject{exercise_obj->m_setObjects.at(i)};
		if (setObject->property("finishButtonVisible").toBool())
		{
			setObject->setProperty("finishButtonEnabled", completed);
			break;
		}
		if (setObject->property("setCompleted").toBool())
			noSetsCompleted = false;
	}
	exerciseEntryItem(exercise_idx)->setProperty("bCanEditRestTimeTracking", noSetsCompleted);
}


*/
