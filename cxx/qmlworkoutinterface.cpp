#include "qmlworkoutinterface.h"

#include "dbcalendarmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "thread_manager.h"
#include "tptimer.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

enum workoutStatusFlags {
	WS_IN_PROGRESS = 0x001,
	WS_EDIT_MODE = 0X002,
	WS_TODAY = 0X004,
	WS_EDITABLE = 0x008,
	WS_FINISHED = 0x010,
};

QmlWorkoutInterface::QmlWorkoutInterface(QObject *parent,DBMesocyclesModel *meso_model, const uint meso_idx, const QDate &date)
	: QObject{parent}, m_mesoModel{meso_model}, m_workoutPage{nullptr}, m_workoutModel{nullptr}, m_calendarModel{nullptr},
		m_mesoIdx{meso_idx}, m_workoutTimer{nullptr}, m_restTimer{nullptr}, m_date{date}, m_hour{0}, m_min{0}, m_sec{0},
		m_workoutStatus{0}, m_importFromSplitPlan{false}
{
	m_calendarModel = m_mesoModel->calendar(m_mesoIdx);
	m_calendarDay = m_calendarModel->calendarDay(m_date);
	m_workoutModel = m_mesoModel->workoutForDay(m_mesoIdx, m_calendarDay);
	connect(m_workoutModel, &DBWorkoutModel::exerciseCountChanged, [this] () {
		if (m_workoutModel->exerciseCount() == 0)
			verifyWorkoutOptions();
	});
	verifyWorkoutOptions();
}

void QmlWorkoutInterface::cleanUp()
{
	m_workoutModel->deleteLater();
	delete m_workoutPage;
	delete m_workoutComponent;
	if (m_workoutTimer)
		delete m_workoutTimer;
	if (m_restTimer)
		delete m_restTimer;
}

//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
void QmlWorkoutInterface::changeSplitLetter(const QString &new_splitletter)
{
	if (m_workoutModel->splitLetter() != new_splitletter)
	{
		setWorkoutIsEditable(new_splitletter != "R"_L1);
		m_workoutModel->setSplitLetter(new_splitletter.at(0));
		m_calendarModel->setSplitLetter(new_splitletter);
		setHeaderText();
		verifyWorkoutOptions();
	}
}

QString QmlWorkoutInterface::timeIn() const
{
	if (m_calendarModel)
	{
		const QTime &time_in{m_calendarModel->timeIn()};
		if (time_in.isValid())
			return appUtils()->formatTime(time_in);
	}
	return "--:--"_L1;
}

void QmlWorkoutInterface::setTimeIn(const QString &new_timein)
{
	m_calendarModel->setTimeIn(appUtils()->timeFromString(new_timein));
	calculateWorkoutTime();
	emit timeInChanged();
}

QString QmlWorkoutInterface::timeOut() const
{
	if (m_calendarModel)
	{
		const QTime &time_out{m_calendarModel->timeOut()};
		if (time_out.isValid())
			return appUtils()->formatTime(time_out);
	}
	return "--:--"_L1;
}

void QmlWorkoutInterface::setTimeOut(const QString &new_timeout)
{
	m_calendarModel->setTimeOut(appUtils()->timeFromString(new_timeout));
	calculateWorkoutTime();
	emit timeOutChanged();
}

void QmlWorkoutInterface::setLocation(const QString &new_location)
{
	m_calendarModel->setLocation(new_location);
}

QString QmlWorkoutInterface::location()
{
	if (m_calendarModel)
	{
		if (!todaysWorkout())
			return m_calendarModel->location(m_calendarDay);
		else
		{
			int i{static_cast<int>(m_calendarDay)};
			do {
				if (m_calendarModel->isWorkoutDay(i) && (!m_calendarModel->location(i).isEmpty()))
				{
					if (i != m_calendarDay)
						setLocation(m_calendarModel->location(i));
					return m_calendarModel->location(i);
				}
			} while (--i >= 0);
		}
	}
	return QString{};
}

QString QmlWorkoutInterface::notes() const
{
	return m_calendarModel->notes();
}

void QmlWorkoutInterface::setNotes(const QString &new_notes)
{
	m_calendarModel->setNotes(new_notes);
}

void QmlWorkoutInterface::setHeaderText()
{
	const bool bRestDay{m_workoutModel->splitLetter() == 'R'};
	const QString &strWhatToTrain{!bRestDay ? tr("Workout number: <b>") % m_calendarModel->workoutNumber() % "</b>"_L1 : tr("Rest day")};
	m_headerText = std::move("<b>"_L1 % appUtils()->formatDate(m_date) % "</b><br>"_L1 % strWhatToTrain);
	m_headerText_2 = std::move(m_mesoModel->muscularGroup(m_mesoIdx, m_workoutModel->splitLetter()));
	emit headerTextChanged();
}

QString QmlWorkoutInterface::sessionLabel() const
{
	if (haveNewWorkoutOptions())
	{
		if (haveExercises())
			return tr("Use or edit this already saved workout session");
		else
			return tr("Start a new workout session");
	}
	return QString{};
}

bool QmlWorkoutInterface::haveExercises() const
{
	return m_workoutModel->exerciseCount();
}

bool QmlWorkoutInterface::workoutFinished() const
{
	return m_calendarModel->completed();
}

void QmlWorkoutInterface::setWorkoutFinished(const bool finished)
{
	m_calendarModel->setCompleted(finished);
	if (changeWorkoutStatus(WS_FINISHED, finished) && finished)
	{
		calculateWorkoutTime();
		rollUpExercises();
	}
}

bool QmlWorkoutInterface::workoutInProgress() const
{
	return checkWorkoutStatus(WS_IN_PROGRESS);
}

void QmlWorkoutInterface::setWorkoutInProgress(const bool in_progress)
{
	if (changeWorkoutStatus(WS_IN_PROGRESS, in_progress))
	{
		if (in_progress)
		{
			if (!m_workoutTimer)
			{
				m_workoutTimer = new TPTimer(this);
				connect(m_workoutTimer, &TPTimer::timeWarning, this, [this] (QString remaingTime, bool bminutes) {
					QMetaObject::invokeMethod(m_workoutPage, "displayTimeWarning", Q_ARG(QString, remaingTime), Q_ARG(bool, bminutes));
				});
				connect(m_workoutTimer, &TPTimer::hoursChanged, this, [this] () { setTimerHour(m_workoutTimer->hours()); });
				connect(m_workoutTimer, &TPTimer::minutesChanged, this, [this] () { setTimerMinute(m_workoutTimer->minutes()); });
				connect(m_workoutTimer, &TPTimer::secondsChanged, this, [this] () { setTimerSecond(m_workoutTimer->seconds()); });
			}
			m_workoutTimer->startTimer();
		}
		else if (m_workoutTimer)
		{
			if (workoutFinished())
				m_workoutTimer->stopTimer();
			else
				m_workoutTimer->pauseTimer();
		}
	}
}

bool QmlWorkoutInterface::editMode() const
{
	return checkWorkoutStatus(WS_EDIT_MODE);
}

void QmlWorkoutInterface::setEditMode(const bool edit_mode)
{
	changeWorkoutStatus(WS_EDIT_MODE, edit_mode);
}

bool QmlWorkoutInterface::workoutIsEditable() const
{
	return checkWorkoutStatus(WS_EDITABLE);
}

void QmlWorkoutInterface::setWorkoutIsEditable(const bool editable)
{
	changeWorkoutStatus(WS_EDITABLE, editable);
}

bool QmlWorkoutInterface::todaysWorkout() const
{
	return checkWorkoutStatus(WS_TODAY);
}

void QmlWorkoutInterface::setTodaysWorkout(const bool is_today)
{
	changeWorkoutStatus(WS_TODAY, is_today);
}

QStringList QmlWorkoutInterface::previousWorkoutsList_text() const
{
	QStringList texts;
	for (const auto &prev_workout : std::as_const(m_prevWorkouts))
		texts += prev_workout;
	return texts;
}

QList<uint> QmlWorkoutInterface::previousWorkoutsList_value() const
{
	QList<uint> values;
	QHash<uint,QString>::const_iterator prev_workout{m_prevWorkouts.constBegin()};
	const QHash<uint,QString>::const_iterator prev_workouts_end{m_prevWorkouts.constEnd()};
	while (prev_workout != prev_workouts_end)
	{
		values.append(prev_workout.key());
		++prev_workout;
	}
	return values;
}

void QmlWorkoutInterface::setWorkingSetMode()
{
	const uint exercise_number{m_workoutModel->workingExercise()};
	const uint exercise_idx{m_workoutModel->workingSubExercise()};
	const uint set_number{m_workoutModel->workingSet()};
	if (m_workoutModel->trackRestTime(exercise_number))
	{
		const bool auto_time{m_workoutModel->autoRestTime(exercise_number)};
		QTime rest_time{0, 0, 0};
		switch (m_workoutModel->setMode(exercise_number, exercise_idx, set_number))
		{
			case SM_START_REST:
				startRestTimer(exercise_number, m_workoutModel->setRestTime(exercise_number, m_workoutModel->workingSubExercise(), m_workoutModel->workingSet()));
			break;
			case SM_START_EXERCISE:
			{
				if (auto_time)
				{
					if (set_number == 0)
					{
						const std::optional<QTime> &time_in{m_calendarModel->timeIn()};
						if (time_in.has_value())
							rest_time = std::move(appUtils()->calculateTimeDifference(time_in.value(), QTime::currentTime()));
					}
					else
						rest_time = std::move(appUtils()->calculateTimeDifference(m_lastSetCompleted, QTime::currentTime()));
				}
				else
				{
					m_restTimer->stopTimer();
					rest_time = std::move(m_restTimer->elapsedTime());
				}
				const QString &new_resttime{appUtils()->formatTime(rest_time, appUtils()->TF_QML_DISPLAY_NO_HOUR)};
				m_workoutModel->setSetRestTime(exercise_number, exercise_idx, set_number, new_resttime);
				emit updateRestTime(exercise_number, new_resttime);
			}
			break;
		}
	}
	const uint next_mode{m_workoutModel->getSetNextMode(exercise_number, exercise_idx, set_number)};
	if (next_mode == SM_COMPLETED)
		m_lastSetCompleted = std::move(QTime::currentTime());
	m_workoutModel->setSetMode(exercise_number, exercise_idx, set_number, next_mode);
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
		createWorkoutPage();
	else
		appPagesListModel()->openPage(m_workoutPage);
}

void QmlWorkoutInterface::loadExercisesFromCalendarDay(const uint calendar_day)
{
	DBWorkoutModel *w_model{m_mesoModel->workoutForDay(m_mesoIdx, calendar_day)};
	auto load = [this,w_model] () -> void {
		*m_workoutModel = w_model;
		m_workoutModel->setAllSetsCompleted(false);
		delete w_model;
	};
	if (w_model->exerciseCount() == 0)
		connect(w_model, &DBSplitModel::exerciseCountChanged, this, [&load] () { load(); });
	else
		load();
}

void QmlWorkoutInterface::getExercisesFromSplitPlan()
{
	DBExercisesModel *split_model{m_mesoModel->splitModel(m_mesoIdx, m_workoutModel->splitLetter())};
	if (split_model->exerciseCount() > 0)
		*m_workoutModel = split_model;
}

void QmlWorkoutInterface::exportWorkoutToSplitPlan()
{
	DBExercisesModel *split_model{m_mesoModel->splitModel(m_mesoIdx, m_workoutModel->splitLetter())};
	*split_model = m_workoutModel;
}

void QmlWorkoutInterface::resetWorkout()
{
	setTimeIn("--:--"_L1);
	setTimeOut("--:--"_L1);
	setWorkoutFinished(false);
	m_workoutTimer->prepareTimer();
}

void QmlWorkoutInterface::exportWorkout(const bool bShare)
{
	const QString &suggestedName{m_mesoModel->name(m_mesoIdx) + tr(" - Workout ") + m_workoutModel->splitLetter() + ".txt"_L1};
	const QString &exportFileName{appItemManager()->setExportFileName(suggestedName)};
	appItemManager()->continueExport(m_workoutModel->exportToFile(exportFileName), bShare);
}

void QmlWorkoutInterface::importWorkout(const QString &filename)
{
	if (filename.isEmpty())
	{
		m_mesoModel->setImportIdx(m_mesoIdx);
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_WORKOUT));
	}
	else
		appItemManager()->openRequestedFile(filename, IFC_WORKOUT);
}

void QmlWorkoutInterface::prepareWorkOutTimer(const QString &strStartTime, const QString &strEndTime)
{
	if (!m_workoutTimer)
		return;
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
	setWorkoutInProgress(true);
}

void QmlWorkoutInterface::stopWorkout()
{
	m_workoutTimer->stopTimer();
	if (!m_workoutTimer->stopWatch())
	{
		const QTime &elapsedTime{m_workoutTimer->elapsedTime()};
		setTimerHour(elapsedTime.hour());
		setTimerMinute(elapsedTime.minute());
		setTimerSecond(elapsedTime.second());
	}
	setTimeOut(appUtils()->getCurrentTimeString());
	setWorkoutFinished(true);
}

bool QmlWorkoutInterface::canChangeSetMode(const uint exercise_number, const uint exercise_idx, const uint set_number) const
{
	const bool set_has_data{!m_workoutModel->setReps(exercise_number, exercise_idx, set_number).isEmpty() &&
		!m_workoutModel->setWeight(exercise_number, exercise_idx, set_number).isEmpty()};
	return set_has_data && (set_number == 0 ? true : m_workoutModel->setCompleted(exercise_number, exercise_idx, set_number - 1));
}

void QmlWorkoutInterface::gotoNextExercise()
{
	if (m_workoutModel->workingExercise() < (m_workoutModel->exerciseCount() - 1) )
	{
		rollUpExercise(m_workoutModel->workingExercise());
		m_workoutModel->setWorkingExercise(m_workoutModel->workingExercise() + 1);
		QMetaObject::invokeMethod(m_workoutPage, "setViewingExercise", Q_ARG(int, m_workoutModel->workingExercise()));
	}
}

void QmlWorkoutInterface::rollUpExercise(const uint exercise_number) const
{
	QMetaObject::invokeMethod(m_workoutPage, "rollUpExercise", Q_ARG(int, exercise_number));
}

void QmlWorkoutInterface::rollUpExercises() const
{
	QMetaObject::invokeMethod(m_workoutPage, "rollUpExercises");
}

void QmlWorkoutInterface::silenceTimeWarning()
{
	m_workoutTimer->stopAlarmSound();
}

inline bool QmlWorkoutInterface::checkWorkoutStatus(uint8_t flag) const
{
	return m_workoutStatus & flag;
}

bool QmlWorkoutInterface::changeWorkoutStatus(uint8_t flag, const bool set, const bool emit_signal)
{
	bool ok{false};
	switch (flag)
	{
		case WS_IN_PROGRESS:
			if ((ok = checkWorkoutStatus(WS_TODAY)))
				changeWorkoutStatus(WS_EDITABLE, set, false);
		break;
		case WS_EDIT_MODE:
			if ((ok = checkWorkoutStatus(WS_FINISHED)) || (ok = !checkWorkoutStatus(WS_TODAY)))
			{
				changeWorkoutStatus(WS_EDITABLE, set, false);
				changeWorkoutStatus(WS_FINISHED, !set, false);
			}
		break;
		case WS_TODAY:
			if ((ok = !checkWorkoutStatus(WS_FINISHED)))
			{
				changeWorkoutStatus(WS_EDITABLE, set, false);
				changeWorkoutStatus(WS_IN_PROGRESS, !set, false);
				changeWorkoutStatus(WS_FINISHED, !set, false);
			}
		break;
		case WS_EDITABLE:
			ok = !checkWorkoutStatus(WS_IN_PROGRESS);
		break;
		case WS_FINISHED:
			if ((ok = checkWorkoutStatus(WS_TODAY)))
			{
				changeWorkoutStatus(WS_IN_PROGRESS, !set, false);
				changeWorkoutStatus(WS_EDITABLE, set, false);
			}
		break;
	}
	if (ok)
	{
		set ? m_workoutStatus |= flag : m_workoutStatus &= ~flag;
		if (emit_signal)
			emit workoutStatusChanged();
	}
	return ok;
}

void QmlWorkoutInterface::createWorkoutPage()
{
	m_workoutComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/WorkoutPage.qml"_L1}, QQmlComponent::Asynchronous};
	switch (m_workoutComponent->status())
	{
		case QQmlComponent::Ready:
			createWorkoutPage_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_workoutComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createWorkoutPage_part2();
			}, Qt::SingleShotConnection);
		break;
		#ifndef QT_NO_DEBUG
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_workoutComponent->errorString();
		break;
		#endif
	}
}

void QmlWorkoutInterface::createWorkoutPage_part2()
{
	m_workoutProperties["workoutManager"_L1] = QVariant::fromValue(this);
	m_workoutProperties["workoutModel"_L1] = QVariant::fromValue(m_workoutModel);
	m_workoutPage = static_cast<QQuickItem*>(m_workoutComponent->createWithInitialProperties(m_workoutProperties, appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_workoutPage)
	{
		qDebug() << m_workoutComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(m_workoutPage, QQmlEngine::CppOwnership);
	m_workoutPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));

	appPagesListModel()->openPage(m_workoutPage, std::move(tr("Workout: ") + appUtils()->formatDate(m_date)), [this] () {
		cleanUp();
	});
	setHeaderText();

	connect(m_workoutModel, &DBExercisesModel::muscularGroupChanged, this, [this] () {
		setHeaderText();
	});

	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx  &&field == MESO_FIELD_SPLIT)
			QMetaObject::invokeMethod(m_workoutPage, "changeComboModel", Q_ARG(QString, m_mesoModel->split(m_mesoIdx)));
	});

	connect(appItemManager(), &QmlItemManager::selectedExerciseFromSimpleExercisesList, [this] (QQuickItem *parentPage) {
		if (parentPage == m_workoutPage)
			m_workoutModel->newExerciseFromExercisesList();
	});

	connect(m_workoutModel, &DBExercisesModel::exerciseModified, this, [this]
					(const uint exercise_number, const uint exercise_idx, const uint set_number, const uint field)
	{
		if (field == EXERCISES_FIELD_COMPLETED)
		{
			const bool all_exercises_completed{m_workoutModel->allSetsCompleted()};
			if (all_exercises_completed != workoutFinished())
			{
				if (!all_exercises_completed || !m_workoutTimer->isActive())
					setWorkoutFinished(all_exercises_completed);
				appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
					appUtils()->string_strings({ tr("Workout"), workoutCompletedMessage(all_exercises_completed)}, record_separator),
							"app_logo"_L1, m_workoutTimer->isActive() ? 0 : 5000);
			}
		}
	});

	if (todaysWorkout())
		connect(m_workoutPage, SIGNAL(silenceTimeWarning()), this, SLOT(silenceTimeWarning()));

	auto createExercisesItem = [this] () -> void {
		m_exercisesProperties["pageManager"] = QVariant::fromValue(this);
		m_exercisesProperties["exercisesModel"] = QVariant::fromValue(m_workoutModel);
		m_exercisesProperties["height"_L1] = appSettings()->pageHeight() - m_workoutPage->property("bottomBarHeight").toInt();
		m_exercisesProperties["width"_L1] = appSettings()->pageWidth() - 10;
		m_exercisesProperties["x"_L1] = 0;
		m_exercisesProperties["y"_L1] = 0;
		m_exercisesItem = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
																		m_exercisesProperties, appQmlEngine()->rootContext()));
		#ifndef QT_NO_DEBUG
		if (!m_exercisesItem)
		{
			qDebug() << m_exercisesComponent->errorString();
			return;
		}
		#endif
		appQmlEngine()->setObjectOwnership(m_exercisesItem, QQmlEngine::CppOwnership);
		m_exercisesItem->setParentItem(m_workoutPage->findChild<QQuickItem*>("exercisesFrame"_L1));
		m_workoutPage->setProperty("lstWorkoutExercises", QVariant::fromValue(m_exercisesItem));
	};

	m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/ExercisesAndSets/WorkoutOrSplitExercisesList.qml"_L1}, QQmlComponent::Asynchronous};
	switch (m_exercisesComponent->status())
	{
		case QQmlComponent::Ready:
			createExercisesItem();
		break;
		case QQmlComponent::Loading:
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,createExercisesItem] (QQmlComponent::Status status) {
				createExercisesItem();
			}, Qt::SingleShotConnection);
		break;
		#ifndef QT_NO_DEBUG
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_exercisesComponent->errorString();
		break;
		#endif
	}
}

void QmlWorkoutInterface::calculateWorkoutTime()
{
	if (!todaysWorkout() || editMode() || workoutFinished())
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
		if (todaysWorkout())
		{
			if (!timeOut().isEmpty()  &&!timeOut().contains('-'))
			{
				m_workoutTimer->setStopWatch(false);
				m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(appUtils()->getCurrentTimeString(), timeOut()));
			}
		}
	}
}

void QmlWorkoutInterface::startRestTimer(const uint exercise_number, const QString &rest_time)
{
	if (!m_restTimer)
	{
		m_restTimer = new TPTimer{this};
		m_restTimer->setInterval(1000);
	}
	m_restTimer->prepareTimer("00:"_L1 % rest_time, rest_time.isEmpty() || rest_time == "00:00"_L1);
	m_restTimer->callOnTimeout([this, exercise_number] () {
		const QString &rest_time{m_restTimer->strMinutes() % ':' % m_restTimer->strSeconds()};
		emit updateRestTime(exercise_number, rest_time);
	});
	m_restTimer->startTimer();
}

void QmlWorkoutInterface::verifyWorkoutOptions()
{
	setTodaysWorkout(m_date == QDate::currentDate());

	if (m_workoutModel->splitLetter() != 'R')
	{
		DBSplitModel *split_model{m_mesoModel->splitModel(m_mesoIdx, m_workoutModel->splitLetter())};
		auto conn{std::make_shared<QMetaObject::Connection>()};
		if (split_model)
		{
			*conn = connect(split_model->database(), &TPDatabaseTable::dbOperationsFinished, this, [this,conn,split_model]
																	(const ThreadManager::StandardOps op, const bool success)
			{
				if (op == ThreadManager::CustomOperation)
				{
					disconnect(*conn);
					setCanImportFromSplitPlan(success);
				}
			});
			auto x = [this,split_model] () -> std::pair<QVariant,QVariant> { return split_model->database()->mesoHasSplitPlan(); };
			split_model->database()->setCustQueryFunction(x);
			appThreadManager()->runAction(split_model->database(), ThreadManager::CustomOperation);
		}
		else {
			*conn = connect(m_mesoModel, &DBMesocyclesModel::splitLoaded, [this,conn] (const uint meso_idx, const QChar &splitletter) {
				if (meso_idx == m_mesoIdx && splitletter == m_workoutModel->splitLetter())
				{
					disconnect(*conn);
					verifyWorkoutOptions();
				}
			});
			m_mesoModel->loadSplit(m_mesoIdx, m_workoutModel->splitLetter());
		}

		if (!m_importFromPrevWorkout.has_value())
		{
			auto conn2{std::make_shared<QMetaObject::Connection>()};
			*conn2 = connect(m_workoutModel->database(), &TPDatabaseTable::actionFinished, this, [this,conn2]
						(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2)
			{
				if (op == ThreadManager::CustomOperation)
				{
					disconnect(*conn2);
					m_prevWorkouts.clear();
					for (const auto &prev_calday : return_value2.toList())
					{
						const uint prev_calendar_day{prev_calday.toUInt()};
						m_prevWorkouts.insert(prev_calendar_day, appUtils()->formatDate(m_calendarModel->date(prev_calendar_day)));
					}
					setCanImportFromPreviousWorkout(return_value1.toBool());
				}
			});
			auto y = [this,split_model] () -> std::pair<QVariant,QVariant> {
				return m_workoutModel->database()->getPreviousWorkoutsIds();
			};
			m_workoutModel->database()->setCustQueryFunction(y);
			appThreadManager()->runAction(m_workoutModel->database(), ThreadManager::CustomOperation);
		}
	}
}

QString QmlWorkoutInterface::workoutCompletedMessage(const bool completed) const
{
	QString message{completed ? std::move(tr("All exercises finished")) : std::move(tr("Workout not yet finished"))};
	if (!completed && m_workoutTimer->isActive())
		message += std::move("<br>"_L1 + tr("You should press Finish to save your workout"));
	return message;
}
