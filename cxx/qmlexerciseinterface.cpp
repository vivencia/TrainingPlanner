#include "qmlexerciseinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "tpappcontrol.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>


QmlExerciseInterface::QmlExerciseInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, const uint exercise_idx)
	: QObject{parent}, m_qmlEngine(qmlEngine), m_tDayExercisesComponent(nullptr), m_exerciseIdx(exercise_idx)
{}

QmlExerciseInterface::~QmlExerciseInterface()
{

}

void QmlExerciseInterface::createExerciseObject()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs}, QQmlComponent::Asynchronous};
		if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
			connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
				{ return createExerciseObject_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}
	createExerciseObject_part2();
}

void QmlExerciseInterface::manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type)
{
	if (!bTrackRestTime)
		bAutoRestTime = false;
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);
	setExerciseRestTime(exercise_idx, bAutoRestTime ?
											u"00:00"_qs :
											m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, new_set_type, 0));
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "updateScreenControls");
	enableDisableSetsRestTime(exercise_idx, bTrackRestTime, bAutoRestTime);
}

uint QmlExerciseInterface::exerciseSetsCount(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.count();
}

uint QmlExerciseInterface::exerciseDefaultSetType(const uint exercise_idx)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->newSetType;
}

inline void QmlExerciseInterface::setExerciseDefaultSetType(const uint exercise_idx, const uint set_type)
{
	tDayExercises::exerciseObject* exerciseObj{m_currentExercises->exerciseObjects.at(exercise_idx)};
	exerciseObj->newSetType = set_type;
	const uint nSets(exerciseSetsCount(exercise_idx));

	if (nSets == 0)
	{
		switch(set_type)
		{
			case SET_TYPE_DROP: setExerciseSets(exercise_idx, u"1"_qs); break;
			case SET_TYPE_CLUSTER: setExerciseSets(exercise_idx, u"2"_qs); break;
			case SET_TYPE_MYOREPS: setExerciseSets(exercise_idx, u"3"_qs); break;
			default: break;
		}
	}
	else
	{
		setExerciseSets(exercise_idx, u"1"_qs);
		setExerciseRestTime(exercise_idx, m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type, nSets));
		setExerciseReps(exercise_idx, m_CurrenttDayModel->nextSetSuggestedReps(exercise_idx, set_type, nSets));
		setExerciseWeight(exercise_idx, m_CurrenttDayModel->nextSetSuggestedWeight(exercise_idx, set_type, nSets));
	}

	if (m_CurrenttDayModel->trackRestTime(exercise_idx) && !m_CurrenttDayModel->autoRestTime(exercise_idx))
		setExerciseRestTime(exercise_idx, m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, set_type, nSets));

	if (nSets == 0)
	{
		const bool bCompositeExercise{set_type == SET_TYPE_GIANT};
		if (bCompositeExercise) //trigger DBTrainingDayModel::compositeExerciseChanged. This will update the screen controls
			m_CurrenttDayModel->setExerciseName(m_CurrenttDayModel->exerciseName1(exercise_idx), exercise_idx);
		else
			m_CurrenttDayModel->setExerciseName1(m_CurrenttDayModel->exerciseName(exercise_idx), exercise_idx);
	}
}

const QString QmlExerciseInterface::exerciseSets(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nSets;
}

void QmlExerciseInterface::setExerciseSets(const uint exercise_idx, const QString& new_nsets)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nSets = new_nsets;
}

const QString QmlExerciseInterface::exerciseRestTime(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->restTime;
}

inline void QmlExerciseInterface::setExerciseRestTime(const uint exercise_idx, const QString& new_resttime)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->restTime = new_resttime;
}

const QString QmlExerciseInterface::exerciseReps(const uint exercise_idx, const uint composite_idx) const
{
	return appUtils()->getCompositeValue(composite_idx, m_currentExercises->exerciseObjects.at(exercise_idx)->nReps, comp_exercise_separator);
}

void QmlExerciseInterface::setExerciseReps(const uint exercise_idx, const uint composite_idx, const QString& new_nreps)
{
	appUtils()->setCompositeValue(composite_idx, new_nreps, m_currentExercises->exerciseObjects.at(exercise_idx)->nReps, comp_exercise_separator);
}

const QString QmlExerciseInterface::exerciseWeight(const uint exercise_idx, const uint composite_idx) const
{
	return appUtils()->getCompositeValue(composite_idx, m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight, comp_exercise_separator);
}

void QmlExerciseInterface::setExerciseWeight(const uint exercise_idx, const uint composite_idx, const QString& new_nweight)
{
	appUtils()->setCompositeValue(composite_idx, new_nweight, m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight, comp_exercise_separator);
}

void QmlExerciseInterface::createExercisesObjects()
{
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExercisesComponent = new QQmlComponent{m_qmlEngine, QUrl(u"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous};
		if (m_tDayExercisesComponent->status() != QQmlComponent::Ready)
			connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
					{ return createExercisesObjects(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		else
			createExercisesObjects();
		QMetaObject::invokeMethod(m_currenttDayPage, "createNavButtons", Qt::AutoConnection);
	}
	else
	{
		m_currenttDayPage->setProperty("bHasMesoPlan", false);
		m_currenttDayPage->setProperty("bHasPreviousTDays", false);
		for(uint i(0); i < m_CurrenttDayModel->exerciseCount(); ++i)
		{
			createExerciseObject_part2(i);
			exerciseEntryItem(i)->setProperty("setNbr", m_CurrenttDayModel->setsNumber(i));
		}
	}
}

void QmlExerciseInterface::createExerciseObject_part2(const int object_idx)
{
	#ifdef DEBUG
	if (m_tDayExercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayExercisesComponent->errorString();
		for (uint i(0); i < m_tDayExercisesComponent->errors().count(); ++i)
			qDebug() << m_tDayExercisesComponent->errors().at(i).description();
		return;
	}
	#endif

	const int exercise_idx(object_idx >= 0 ? object_idx : m_currentExercises->exerciseObjects.count());
	appendExerciseEntry();

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

	bool bTrackRestTime(false), bAutoRestTime(false);
	if (exercise_idx > 1)
	{
		bTrackRestTime = m_CurrenttDayModel->trackRestTime(exercise_idx-1);
		bAutoRestTime = m_CurrenttDayModel->autoRestTime(exercise_idx-1);
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(exercise_idx, SET_TYPE_REGULAR, 0);
	}
	else
		nRestTime = m_CurrenttDayModel->nextSetSuggestedTime(0, SET_TYPE_REGULAR, 0);

	setExerciseSets(exercise_idx, nSets);
	setExerciseReps(exercise_idx, nReps);
	setExerciseWeight(exercise_idx, nWeight);
	setExerciseRestTime(exercise_idx, nRestTime);
	m_CurrenttDayModel->newExercise(exerciseName, m_CurrenttDayModel->exerciseCount());
	m_CurrenttDayModel->setTrackRestTime(bTrackRestTime, exercise_idx);
	m_CurrenttDayModel->setAutoRestTime(bAutoRestTime, exercise_idx);
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "updateScreenControls");

	m_tDayExerciseEntryProperties.insert(u"itemManager"_qs, QVariant::fromValue(this));
	m_tDayExerciseEntryProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
	m_tDayExerciseEntryProperties.insert(u"exerciseIdx"_qs, exercise_idx);
	m_tDayExerciseEntryProperties.insert(u"nRestTime"_qs, nRestTime);
	m_tDayExerciseEntryProperties.insert(u"bTrackRestTime"_qs, bTrackRestTime);
	m_tDayExerciseEntryProperties.insert(u"bAutoRestTime"_qs, bAutoRestTime);
	m_tDayExerciseEntryProperties.insert(u"bCanEditRestTimeTracking"_qs, true);
	m_tDayExerciseEntryProperties.insert(u"bCompositeExercise"_qs, m_CurrenttDayModel->compositeExercise(m_CurrenttDayModel->exerciseCount()-1));

	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_qmlEngine->rootContext())));
	m_qmlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	setExerciseItem(exercise_idx, item);
	QQuickItem* parentLayout(m_currenttDayPage->findChild<QQuickItem*>(u"tDayExercisesLayout"_qs));
	item->setParentItem(parentLayout);
	item->setProperty("Layout.row", exercise_idx);
	item->setProperty("Layout.column", 0);
	connect( item, SIGNAL(requestSimpleExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)), this,
						SLOT(requestExercisesList(QQuickItem*,const QVariant&,const QVariant&,int)) );
	connect( item, SIGNAL(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)), this,
						SLOT(requestFloatingButton(const QVariant&,const QVariant&,const QVariant&)) );
	connect( item, SIGNAL(showRemoveExerciseMessage(int)), this, SLOT(showRemoveExerciseMessage(int)) );

	QMetaObject::invokeMethod(item, "liberateSignals", Q_ARG(bool, true));
}

inline uint QmlExerciseInterface::exercisesCount() const
{
	return m_currentExercises->exerciseObjects.count();
}

inline QQuickItem* QmlExerciseInterface::exerciseEntryItem(const uint exercise_idx)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry;
}

inline QQuickItem* QmlExerciseInterface::exerciseEntryItem(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry;
}

inline QQuickItem* QmlExerciseInterface::exerciseSetItem(const uint exercise_idx, const uint set_number)
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number);
}

inline QQuickItem* QmlExerciseInterface::exerciseSetItem(const uint exercise_idx, const uint set_number) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number);
}

inline void QmlExerciseInterface::appendExerciseEntry()
{
	tDayExercises::exerciseObject* exerciseObj{new tDayExercises::exerciseObject};
	m_currentExercises->exerciseObjects.append(exerciseObj);
}

void QmlExerciseInterface::removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow)
{
	tDayExercises::exerciseObject* exerciseObj(m_currentExercises->exerciseObjects.at(exercise_idx));
	for (uint x(0); x < exerciseObj->m_setObjects.count(); ++x)
		bDeleteNow ? delete exerciseObj->m_setObjects.at(x) : exerciseObj->m_setObjects.at(x)->deleteLater();
	exerciseObj->m_setObjects.clear();
	if (exerciseObj->m_setTimer)
		bDeleteNow ? delete exerciseObj->m_setTimer : exerciseObj->m_setTimer->deleteLater();
	bDeleteNow ? delete exerciseObj->m_exerciseEntry : exerciseObj->m_exerciseEntry->deleteLater();
	m_currentExercises->exerciseObjects.removeAt(exercise_idx);
	delete exerciseObj;
}

inline void QmlExerciseInterface::setExerciseItem(const uint exercise_idx, QQuickItem* new_exerciseItem)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_exerciseEntry = new_exerciseItem;
}

inline const QString& QmlExerciseInterface::exerciseReps(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nReps;
}

inline void QmlExerciseInterface::setExerciseReps(const uint exercise_idx, const QString& nreps)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nReps = nreps;
}

inline const QString& QmlExerciseInterface::exerciseWeights(const uint exercise_idx) const
{
	return m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight;
}

inline void QmlExerciseInterface::setExerciseWeight(const uint exercise_idx, const QString& nweight)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->nWeight = nweight;
}

inline void QmlExerciseInterface::insertExerciseSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.insert(set_number, new_setObject);
}

inline void QmlExerciseInterface::appendExerciseSet(const uint exercise_idx, QQuickItem* new_setObject)
{
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.append(new_setObject);
}

inline void QmlExerciseInterface::removeExerciseSet(const uint exercise_idx, const uint set_number)
{
	exerciseSetItem(exercise_idx, set_number)->deleteLater();
	m_currentExercises->exerciseObjects.at(exercise_idx)->m_setObjects.remove(set_number);
}

inline void QmlExerciseInterface::clearExerciseEntries(const bool bDeleteNow)
{
	for (int i(m_currentExercises->exerciseObjects.count() - 1); i >= 0 ; --i)
		removeExerciseEntry(i, bDeleteNow);
}
