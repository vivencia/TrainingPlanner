#include "qmlworkoutinterface.h"

#include "dbcalendarmodel.h"
#include "dbexerciseslistmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "qmlsetentry.h"
#include "qmlexerciseentry.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tptimer.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

#include <ranges>

QmlWorkoutInterface::QmlWorkoutInterface(QObject *parent, const uint meso_idx, const QDate &date)
	: QObject{parent}, m_workoutPage{nullptr}, m_mesoIdx{meso_idx}, m_workoutTimer{nullptr}, m_restTimer{nullptr}
{
	m_calendarModel = appMesoModel()->mesoCalendarManager()->calendar(m_mesoIdx);
	m_calendarDay = appMesoModel()->mesoCalendarManager()->calendarDay(m_mesoIdx, date);
	m_workoutModel = appMesoModel()->mesoCalendarManager()->workout(m_mesoIdx, m_calendarDay);
	if (!appMesoModel()->mesoCalendarManager()->hasDBData(m_mesoIdx))
	{
		const int id{appDBInterface()->getMesoCalendar(m_mesoIdx)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn,id] (const uint thread_id) {
			if (id == thread_id)
			{
				disconnect(*conn);
				if (m_workoutModel->exerciseCount() == 0)
				{
					connect(appDBInterface(), &DBInterface::databaseReady, this, &QmlWorkoutInterface::verifyWorkoutOptions);
					appDBInterface()->getWorkout(m_workoutModel);
				}
				emit timeInChanged();
				emit timeOutChanged();
				emit locationChanged();
				emit lastWorkOutLocationChanged();
				emit notesChanged();
				emit headerTextChanged();
			}
		});
	}
	m_editMode = false;
	m_workoutIsEditable = false;
	setMainDateIsToday(date == QDate::currentDate());
}

QmlWorkoutInterface::~QmlWorkoutInterface()
{
	emit removePageFromMainMenu(m_workoutPage);
	qDeleteAll(m_exercisesList);
	delete m_exercisesComponent;
	m_workoutModel->deleteLater();
	delete m_workoutPage;
	delete m_workoutComponent;
	if (m_workoutTimer)
		delete m_workoutTimer;
	if (m_restTimer)
		delete m_restTimer;
}

void QmlWorkoutInterface::setWorkingExercise(QmlExerciseEntry *new_workingexercise)
{
	m_workingExercise = new_workingexercise;
	m_workoutModel->setWorkingExercise(new_workingexercise->exerciseNumber());
	emit workingExerciseChanged();
}

void QmlWorkoutInterface::setWorkingSet(QmlSetEntry *new_workingset)
{
	m_workingSet = new_workingset;
	m_workoutModel->setWorkingSet(new_workingset->exerciseNumber(), new_workingset->exerciseIdx(), new_workingset->setNumber());
	emit workingExerciseChanged();
	emit workingSetChanged();
}

//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
QChar QmlWorkoutInterface::splitLetter() const
{
	return m_calendarModel->splitLetter(m_calendarDay).at(0);
}

void QmlWorkoutInterface::setSplitLetter(const QChar &new_splitletter, const bool clear_exercises)
{
	if (m_workoutModel->splitLetter() != new_splitletter)
	{
		if (new_splitletter == "R"_L1)
			setWorkoutIsEditable(false);
		m_calendarModel->setSplitLetter(m_calendarDay, new_splitletter);
		m_workoutModel->setSplitLetter(new_splitletter);
		emit splitLetterChanged();
		if (clear_exercises)
			clearExercises(true);
	}
}

QString QmlWorkoutInterface::timeIn() const
{
	const QTime &time_in{m_calendarModel->timeIn(m_calendarDay)};
	if (time_in.isValid())
		return appUtils()->formatTime(time_in);
	else
		return "--:--"_L1;
}

void QmlWorkoutInterface::setTimeIn(const QString &new_timein)
{
	m_calendarModel->setTimeIn(m_calendarDay, appUtils()->getTimeFromTimeString(new_timein));
	calculateWorkoutTime();
	emit timeInChanged();
}

QString QmlWorkoutInterface::timeOut() const
{
	const QTime &time_out{m_calendarModel->timeOut(m_calendarDay)};
	if (time_out.isValid())
		return appUtils()->formatTime(time_out);
	else
		return "--:--"_L1;
}

void QmlWorkoutInterface::setTimeOut(const QString &new_timeout)
{
	m_calendarModel->setTimeOut(m_calendarDay, appUtils()->getTimeFromTimeString(new_timeout));
	calculateWorkoutTime();
	emit timeInChanged();
}

QString QmlWorkoutInterface::location() const
{
	return m_calendarModel->location(m_calendarDay);
}

void QmlWorkoutInterface::setLocation(const QString &new_location)
{
	m_calendarModel->setLocation(m_calendarDay, new_location);
	emit locationChanged();
}

QString QmlWorkoutInterface::lastWorkOutLocation() const
{
	QString last_location{};
	for (int i{static_cast<int>(m_calendarDay)}; i >= 0; --i)
	{
		if (m_calendarModel->isWorkoutDay(i))
		{
			last_location = std::move(m_calendarModel->location(i));
			if (!last_location.isEmpty())
				break;
		}
	}
	return last_location;
}

QString QmlWorkoutInterface::notes() const
{
	return m_calendarModel->notes(m_calendarDay);
}

void QmlWorkoutInterface::setNotes(const QString &new_notes)
{
	m_calendarModel->setNotes(m_calendarDay, new_notes);
	emit notesChanged();
}

bool QmlWorkoutInterface::dayIsFinished() const
{
	return m_calendarModel->completed(m_calendarDay);
}

void QmlWorkoutInterface::setDayIsFinished(const bool finished)
{
	m_calendarModel->setCompleted(m_calendarDay, finished);
	emit dayIsFinishedChanged();
	if (finished)
	{
		calculateWorkoutTime();
		rollUpExercises();
		appDBInterface()->saveMesoCalendar(m_mesoIdx);
	}
}

void QmlWorkoutInterface::setHeaderText(const QString &new_header)
{
	const bool bRestDay{splitLetter() == "R"_L1};
	const QString &strWhatToTrain{!bRestDay ?
						std::move(tr("Workout number: <b>") + m_calendarModel->workoutNumber(m_calendarDay) + "</b>"_L1) :
						std::move(tr("Rest day"))};
	m_headerText = std::move("<b>"_L1 + appUtils()->formatDate(m_calendarModel->date(m_calendarDay)) + "</b><br>"_L1 + strWhatToTrain);
	emit headerTextChanged();
}

QString QmlWorkoutInterface::muscularGroup() const
{
	return appMesoModel()->muscularGroup(m_mesoIdx, splitLetter());
}

void QmlWorkoutInterface::setEditMode(const bool edit_mode)
{
	if (m_editMode != edit_mode)
	{
		m_editMode = edit_mode;
		emit editModeChanged();
		setWorkoutIsEditable(m_editMode);
		setDayIsFinished(!m_editMode);
	}
}

void QmlWorkoutInterface::setWorkoutIsEditable(const bool editable)
{
	if (m_workoutIsEditable != editable)
	{
		m_workoutIsEditable = editable;
		emit workoutIsEditableChanged();
		for (const auto exercise_entry : std::as_const(m_exercisesList))
			exercise_entry->setIsEditable(editable);
	}
}

void QmlWorkoutInterface::setMainDateIsToday(const bool is_today)
{
	if (m_bMainDateIsToday != is_today)
	{
		m_bMainDateIsToday = is_today;
		emit mainDateIsTodayChanged();
		if (m_bMainDateIsToday && !m_workoutTimer)
		{
			m_workoutTimer = new TPTimer(this);
			connect(m_workoutTimer, &TPTimer::timeWarning, this, [this] (QString remaingTime, bool bminutes) {
				QMetaObject::invokeMethod(m_workoutPage, "displayTimeWarning", Q_ARG(QString, remaingTime), Q_ARG(bool, bminutes));
			});
			connect(m_workoutTimer, &TPTimer::hoursChanged, this, [this] () { setTimerHour(m_workoutTimer->hours()); });
			connect(m_workoutTimer, &TPTimer::minutesChanged, this, [this] () { setTimerMinute(m_workoutTimer->minutes()); });
			connect(m_workoutTimer, &TPTimer::secondsChanged, this, [this] () { setTimerSecond(m_workoutTimer->seconds()); });
		}
	}
}

bool QmlWorkoutInterface::hasExercises() const
{
	return m_workoutModel->exerciseCount() > 0;
}
//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

void QmlWorkoutInterface::setMesoIdx(const uint new_meso_idx)
{
	m_mesoIdx = new_meso_idx;
	m_workoutModel->setMesoIdx(new_meso_idx);
}

void QmlWorkoutInterface::getWorkoutPage()
{
	if (!m_workoutPage)
		createTrainingDayPage();
	else
	{
		setNeedActivation(true);
		emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_calendarModel->date(m_calendarDay)), m_workoutPage);
	}
}

void QmlWorkoutInterface::loadExercisesFromDate(const QString &strDate)
{
	const int id{appDBInterface()->getWorkout(m_workoutModel)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn,id] (const uint thread_id) {
		if (id == thread_id)
		{
			disconnect(*conn);
			if (m_workoutModel->exerciseCount() > 0)
				loadExercises();
		}
	});
}

void QmlWorkoutInterface::getExercisesFromSplitPlan()
{
	DBExercisesModel *split_model{appMesoModel()->splitModel(m_mesoIdx, splitLetter(), false)};
	if (split_model->exerciseCount() > 0)
	{
		m_workoutModel = split_model;
		loadExercises();
	}
	else
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		const int id{appDBInterface()->getMesoSplit(split_model)};
		*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn,id] (const uint thread_id) {
			if (id == thread_id)
			{
				disconnect(*conn);
				getExercisesFromSplitPlan();
			}
		});
	}
}

void QmlWorkoutInterface::exportWorkoutToSplitPlan()
{
	DBExercisesModel *split_model{appMesoModel()->splitModel(m_mesoIdx, splitLetter(), false)};
	*split_model = m_workoutModel;
	appDBInterface()->saveMesoSplit(split_model);
}

void QmlWorkoutInterface::resetWorkout()
{
	setTimeIn("--:--"_L1);
	setTimeOut("--:--"_L1);
	setDayIsFinished(false);
	m_workoutTimer->prepareTimer();
}

void QmlWorkoutInterface::exportTrainingDay(const bool bShare)
{
	const QString &suggestedName{appMesoModel()->name(m_mesoIdx) + tr(" - Workout ") + splitLetter() + ".txt"_L1};
	const QString &exportFileName{appItemManager()->setExportFileName(suggestedName)};
	appItemManager()->continueExport(m_workoutModel->exportToFile(exportFileName), bShare);
}

void QmlWorkoutInterface::importTrainingDay(const QString &filename)
{
	if (filename.isEmpty())
	{
		appMesoModel()->setImportIdx(m_mesoIdx);
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	}
	else
		appItemManager()->openRequestedFile(filename, IFC_WORKOUT);
}

void QmlWorkoutInterface::prepareWorkOutTimer(const QString &strStartTime, const QString &strEndTime)
{
	if (!strEndTime.isEmpty())
	{
		m_workoutTimer->setStopWatch(false);
		m_workoutTimer->prepareTimer(strEndTime); //time constrained(limited) workout by length.
		if (!strStartTime.isEmpty()) //time constrained(limited) workout by time of day.
			m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(strStartTime, strEndTime));
	}
	else
	{
		m_workoutTimer->setStopWatch(true);
		if (strStartTime.isEmpty())
			m_workoutTimer->prepareTimer("00:00:00"_L1); //a regular workout timer. Open end time, start when begin workout is clicked
		else //some error made the app crash. We have saved the start time on workoutModel, but it is a little later now, so adjust
			m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(strStartTime, appUtils()->getCurrentTimeString()));
	}
}

void QmlWorkoutInterface::startWorkout()
{
	if (timeIn().contains('-'))
		setTimeIn(appUtils()->getCurrentTimeString());
	setWorkoutIsEditable(true);
	m_workoutTimer->startTimer();
	setTimerActive(true);
}

void QmlWorkoutInterface::stopWorkout()
{
	m_workoutTimer->stopTimer();
	setTimerActive(false);
	if (!m_workoutTimer->stopWatch())
	{
		const QTime &elapsedTime{m_workoutTimer->elapsedTime()};
		setTimerHour(elapsedTime.hour());
		setTimerMinute(elapsedTime.minute());
		setTimerSecond(elapsedTime.second());
	}
	setTimeOut(appUtils()->getCurrentTimeString());
	setWorkoutIsEditable(false);
	setDayIsFinished(true);
}

void QmlWorkoutInterface::addExercise(const bool show_exercises_list_page)
{
	const uint exercise_number{m_workoutModel->addExercise()};
	m_nExercisesToCreate = -1;
	createExerciseObject(exercise_number);
	if (show_exercises_list_page)
		appItemManager()->getExercisesPage(this);
}

void QmlWorkoutInterface::clearExercises(const bool bShowIntentDialog)
{
	m_workoutModel->clearExercises();
	setDayIsFinished(false);
	qDeleteAll(m_exercisesList);
	m_exercisesList.clear();
	emit hasExercisesChanged();
	if (bShowIntentDialog)
		verifyWorkoutOptions();
}

void QmlWorkoutInterface::removeExercise(const uint exercise_number)
{
	m_workoutModel->delExercise(exercise_number);
	m_exercisesList.at(exercise_number)->deleteLater();
	m_exercisesList.removeAt(exercise_number);

	if (m_exercisesList.count() == 0)
		emit hasExercisesChanged();
	else
	{
		for(uint i{exercise_number}; i < m_exercisesList.count(); ++i)
			moveExercise(i-1, i);
		setWorkingExercise(m_exercisesList.at(exercise_number));
	}
}

void QmlWorkoutInterface::removeExerciseObject(const uint exercise_idx, const bool bAsk)
{
	if (bAsk)
		askRemoveExercise(exercise_idx);
	else
		removeExercise(exercise_idx);
}

void QmlWorkoutInterface::moveExercise(const uint exercise_number, const uint new_exercisenumber)
{
	for (const auto exercise : std::as_const(m_exercisesList))
		exercise->exerciseEntry()->setParentItem(nullptr);

	m_exercisesList.swapItemsAt(exercise_number, new_exercisenumber);
	m_workoutModel->moveExercise(exercise_number, new_exercisenumber);
	m_exercisesList.at(exercise_number)->setExerciseNumber(exercise_number);
	m_exercisesList.at(new_exercisenumber)->setExerciseNumber(new_exercisenumber);
	if (exercise_number == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_number)->setLastExercise(true);
		m_exercisesList.at(new_exercisenumber)->setLastExercise(false);
	}
	else if (new_exercisenumber == m_exercisesList.count() - 1)
	{
		m_exercisesList.at(exercise_number)->setLastExercise(false);
		m_exercisesList.at(new_exercisenumber)->setLastExercise(true);
	}

	for (const auto exercise : std::as_const(m_exercisesList))
	{
		if (workingExercise() == exercise)
			setWorkingExercise(exercise); //fix the workingExercise value in m_workoutModel after the move
		exercise->exerciseEntry()->setParentItem(m_exercisesLayout);
	}
}

void QmlWorkoutInterface::simpleExercisesList(const bool show, const bool multi_sel)
{
	if (show)
	{
		if (appExercisesList()->count() == 0)
			appDBInterface()->getAllExercises();
		connect(m_workoutPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(newExerciseFromExercisesList()));
		connect(m_workoutPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_workoutPage, "showSimpleExercisesList", Q_ARG(bool, multi_sel));
	}
	else
	{
		disconnect(m_workoutPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(newExerciseFromExercisesList()));
		disconnect(m_workoutPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_workoutPage, "hideSimpleExercisesList");
	}
}

void QmlWorkoutInterface::displayMessage(const QString &title, const QString &message, const bool error, const uint msecs) const
{
	QMetaObject::invokeMethod(m_workoutPage, "showMessageDialog", Q_ARG(QString, title), Q_ARG(QString, message), Q_ARG(bool, error), Q_ARG(int, static_cast<int>(msecs)));
}

void QmlWorkoutInterface::askRemoveExercise(const uint exercise_idx)
{
	if (appSettings()->alwaysAskConfirmation())
		QMetaObject::invokeMethod(m_workoutPage, "showRemoveExerciseMessage", Q_ARG(int, static_cast<int>(exercise_idx)),
			Q_ARG(QString, m_workoutModel->exerciseName(exercise_idx)));
	else
		removeExercise(exercise_idx);
}

void QmlWorkoutInterface::gotoNextExercise(const uint exercise_number)
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
				QMetaObject::invokeMethod(m_workoutPage, "placeSetIntoView",
							Q_ARG(int, anterior_exercise_entry->exerciseEntry()->y() + exercise_entry->exerciseEntry()->height()));
				return;
			}
			anterior_exercise_entry = exercise_entry;
		}
		QMetaObject::invokeMethod(m_workoutPage, "placeSetIntoView", Q_ARG(int, 0));
	}
}

void QmlWorkoutInterface::rollUpExercises() const
{
	for (const auto exercise_entry : std::as_const(m_exercisesList))
		QMetaObject::invokeMethod(exercise_entry->exerciseEntry(), "paneExerciseShowHide", Q_ARG(bool, false));
	QMetaObject::invokeMethod(m_workoutPage, "placeSetIntoView", Q_ARG(int, 0));
}

TPTimer *QmlWorkoutInterface::restTimer()
{
	if (!m_restTimer)
		m_restTimer = new TPTimer(this);
	return m_restTimer;
}

void QmlWorkoutInterface::newExerciseFromExercisesList()
{
	addExercise(false);
	changeExerciseFromExercisesList(m_workoutModel->exerciseCount() - 1);
}

void QmlWorkoutInterface::changeExerciseFromExercisesList(int exercise_number)
{
	const uint n_subexercises{appExercisesList()->selectedEntriesCount()};
	if (n_subexercises == 0)
		return;

	if (exercise_number)
		exercise_number = m_workoutModel->workingExercise();

	for (uint exercise_idx{0}; exercise_idx < n_subexercises; ++exercise_idx)
	{
		if (exercise_idx >= m_workoutModel->subExercisesCount(exercise_number))
			m_exercisesList.at(exercise_idx)->addSubExercise();
		m_workoutModel->setExerciseName(exercise_number, exercise_idx, std::move(
			appExercisesList()->selectedEntriesValue(exercise_idx, EXERCISES_LIST_COL_MAINNAME) + " - "_L1 +
			appExercisesList()->selectedEntriesValue(exercise_idx, EXERCISES_LIST_COL_SUBNAME)));
	}
}

void QmlWorkoutInterface::silenceTimeWarning()
{
	m_workoutTimer->stopAlarmSound();
}

void QmlWorkoutInterface::hideSimpleExercisesList()
{
	simpleExercisesList(false);
}

void QmlWorkoutInterface::verifyWorkoutOptions()
{
	setCanImportFromSplitPlan(appDBInterface()->mesoHasSplitPlan(appMesoModel()->id(m_mesoIdx), splitLetter()));
	auto conn = std::make_shared<QMetaObject::Connection>();
	const int id{appDBInterface()->getPreviousWorkouts(m_workoutModel)};
	*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn,id] (const uint db_id) {
		if (id == db_id)
		{
			disconnect(*conn);
			setCanImportFromPreviousWorkout(m_workoutModel->previousWorkouts().count() > 0);
			m_prevWorkouts.clear();
			for (const auto prev_calday : m_workoutModel->previousWorkouts())
			{
				m_prevWorkouts.append(std::move(appUtils()->formatDate(
								appMesoModel()->mesoCalendarManager()->dateFromCalendarDay(m_mesoIdx, prev_calday).value())));
			}
			QMetaObject::invokeMethod(m_workoutPage, "showIntentionDialog");
		}
	});
}

void QmlWorkoutInterface::createTrainingDayPage()
{
	m_workoutComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/TrainingDayPage.qml"_L1}, QQmlComponent::Asynchronous};
	if (m_workoutComponent->status() != QQmlComponent::Ready)
		connect(m_workoutComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
			{ return createTrainingDayPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createTrainingDayPage_part2();
}

void QmlWorkoutInterface::createTrainingDayPage_part2()
{
	#ifndef QT_NO_DEBUG
	if (m_workoutComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_workoutComponent->errorString();
		for (auto &error : m_workoutComponent->errors())
			qDebug() << error;
		return;
	}
	#endif

	m_workoutProperties.insert("workoutManager"_L1, QVariant::fromValue(this));
	m_workoutPage = static_cast<QQuickItem*>(m_workoutComponent->createWithInitialProperties(m_workoutProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_workoutPage, QQmlEngine::CppOwnership);
	m_workoutPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_exercisesLayout = m_workoutPage->findChild<QQuickItem*>("exercisesLayout"_L1);

	connect(this, &QmlWorkoutInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QmlWorkoutInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_calendarModel->date(m_calendarDay)), m_workoutPage);
	setHeaderText();

	if (m_workoutModel->splitLetter() != "R"_L1)
		QMetaObject::invokeMethod(m_workoutPage, "createNavButtons");

	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar &chrSplitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			if (splitLetter() == chrSplitLetter)
				emit this->muscularGroupChanged();
		}
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx  &&field == MESOCYCLES_COL_SPLIT)
			QMetaObject::invokeMethod(m_workoutPage, "changeComboModel", Q_ARG(QString, appMesoModel()->split(m_mesoIdx)));
	});

	connect(m_workoutModel, &DBExercisesModel::exerciseModified, this, [this] (const uint exercise_number, const uint exercise_idx, const uint set_number, const uint field) {
		if (field != EXERCISE_IGNORE_NOTIFY_IDX)
			appDBInterface()->saveWorkout(m_workoutModel);
	});

	if (mainDateIsToday())
		connect(m_workoutPage, SIGNAL(silenceTimeWarning()), this, SLOT(silenceTimeWarning()));
}

void QmlWorkoutInterface::createExerciseObject(const uint exercise_number)
{
	if (!m_exercisesComponent)
	{
		m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/ExercisesAndSets/ExerciseEntry.qml"_L1}, QQmlComponent::Asynchronous};
		if (m_exercisesComponent->status() != QQmlComponent::Ready)
		{
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,exercise_number] (QQmlComponent::Status status) {
				if (status == QQmlComponent::Ready)
				{
					disconnect(m_exercisesComponent, &QQmlComponent::statusChanged, this, nullptr);
					createExerciseObject(exercise_number);
				}
			});
			return;
		}
	}

	QmlExerciseEntry *newExercise{new QmlExerciseEntry{this, this, m_workoutModel, exercise_number}};
	if (exercise_number > 0)
		m_exercisesList.last()->setLastExercise(false);

	newExercise->setIsEditable(true);
	newExercise->setLastExercise(true);
	newExercise->setCanEditRestTimeTracking(true);
	newExercise->setLastExercise(true);
	m_exercisesList.append(newExercise);
	createExerciseObject_part2(exercise_number);
}

void QmlWorkoutInterface::createExerciseObject_part2(const uint exercise_number)
{
	#ifndef QT_NO_DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (auto &error : m_exercisesComponent->errors())
			qDebug() << error;
		return;
	}
	#endif

	m_exercisesProperties.insert("exerciseManager"_L1, QVariant::fromValue(m_exercisesList.at(exercise_number)));

	QQuickItem *item{static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
													m_exercisesProperties, appQmlEngine()->rootContext()))};
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_exercisesLayout);
	item->setProperty("Layout.row", exercise_number);
	item->setProperty("Layout.column", 0);
	m_exercisesList.at(exercise_number)->setExerciseEntry(item);
	if (m_nExercisesToCreate == -1)
	{
		setWorkingExercise(m_exercisesList.at(exercise_number));
		QMetaObject::invokeMethod(m_workoutPage, "placeSetIntoView", Q_ARG(int, -2));
	}
	else
	{
		if (--m_nExercisesToCreate == 0)
		{
			setWorkingExercise(m_exercisesList.at(0));
			QMetaObject::invokeMethod(m_workoutPage, "placeSetIntoView", Q_ARG(int, -1));
		}
	}
	if (m_exercisesList.count() == 1)
		emit hasExercisesChanged();
}

void QmlWorkoutInterface::loadExercises()
{
	m_nExercisesToCreate = m_workoutModel->exerciseCount();
	for (uint i{0}; i < m_workoutModel->exerciseCount(); ++i)
		createExerciseObject(i);
}

void QmlWorkoutInterface::calculateWorkoutTime()
{
	if (!mainDateIsToday() || editMode() || dayIsFinished())
	{
		if (!timeIn().isEmpty()  &&!timeIn().contains('-'))
		{
			if (!timeOut().isEmpty()  &&!timeOut().contains('-'))
			{
				const QTime &workoutLenght{appUtils()->calculateTimeDifference(timeIn(), timeOut())};
				setTimerHour(workoutLenght.hour());
				setTimerMinute(workoutLenght.minute());
				setTimerSecond(workoutLenght.second());
				return;
			}
		}
		m_hour = m_min = m_sec = 0;
	}
	else
	{
		if (mainDateIsToday())
		{
			if (!timeOut().isEmpty()  &&!timeOut().contains('-'))
			{
				m_workoutTimer->setStopWatch(false);
				m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(appUtils()->getCurrentTimeString(), timeOut()));
			}
		}
	}
}
