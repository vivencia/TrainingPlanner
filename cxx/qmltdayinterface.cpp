#include "qmltdayinterface.h"

#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "dbmesocalendarmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "qmlexerciseentry.h"
#include "qmlexerciseinterface.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tptimer.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlTDayInterface::~QmlTDayInterface()
{
	emit removePageFromMainMenu(m_tDayPage);
	m_exerciseManager->deleteLater();
	m_tDayModel->deleteLater();
	delete m_tDayPage;
	delete m_tDayComponent;
	if (m_workoutTimer)
		delete m_workoutTimer;
	if (m_restTimer)
		delete m_restTimer;
}

//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
void QmlTDayInterface::setSplitLetter(const QString& new_value, const bool bFromQml, const bool bDontConfirm)
{
	if (bFromQml)
	{
		if (m_splitLetter != new_value)
		{
			if (m_exerciseManager->exercisesCount() > 0)
			{
				QMetaObject::invokeMethod(m_tDayPage, "showSplitLetterChangedDialog");
				return;
			}
		}
	}
	if (new_value != "X"_L1)
	{
		if (m_splitLetter != new_value)
		{
			m_splitLetter = new_value;
			if (bDontConfirm || !appSettings()->alwaysAskConfirmation())
				adjustCalendar(m_splitLetter, true);
			else
				QMetaObject::invokeMethod(m_tDayPage, "showAdjustCalendarDialog");
			m_tDayModel->setSplitLetter(m_splitLetter);
		}
	}
	emit splitLetterChanged();
}

void QmlTDayInterface::setTimeIn(const QString& new_value, const bool bFromQml)
{
	if (m_timeIn != new_value)
	{
		m_timeIn = new_value;
		emit timeInChanged();
		if (bFromQml)
		{
			m_tDayModel->setTimeIn(m_timeIn.startsWith("--"_L1) ? QString() : m_timeIn);
			calculateWorkoutTime();
		}
	}
}

void QmlTDayInterface::setTimeOut(const QString& new_value, const bool bFromQml)
{
	if (m_timeOut != new_value)
	{
		m_timeOut = new_value;
		emit timeOutChanged();
		if (bFromQml)
		{
			m_tDayModel->setTimeOut(m_timeOut.startsWith("--"_L1) ? QString() : m_timeOut);
			calculateWorkoutTime();
		}
	}
}

void QmlTDayInterface::setHeaderText(const QString&)
{
	const bool bRestDay(splitLetter() == "R"_L1);
	QString strWhatToTrain;
	if (!bRestDay)
	{
		appExercisesModel()->makeFilterString(appMesoModel()->muscularGroup(m_mesoIdx, _splitLetter()));
		strWhatToTrain = std::move(tr("Workout number: <b>") + m_tDayModel->trainingDay() + "</b><br><b>"_L1 +
			std::move(appMesoModel()->muscularGroup(m_mesoIdx, _splitLetter())) + "</b>"_L1);
	}
	else
		strWhatToTrain = std::move(tr("Rest day"));
	m_headerText = std::move("<b>"_L1 + appUtils()->formatDate(m_tDayModel->date()) + "</b><br>"_L1 + strWhatToTrain);
	emit headerTextChanged();
}

void QmlTDayInterface::setLastWorkOutLocation(const QString& new_value)
{
	if (m_lastWorkOutLocation != new_value)
	{
		m_lastWorkOutLocation = new_value;
		emit lastWorkOutLocationChanged();
		m_tDayModel->setLocation(m_lastWorkOutLocation);
	}
}

void QmlTDayInterface::setDayNotes(const QString& new_value, const bool bFromQml)
{
	m_dayNotes = new_value;
	emit dayNotesChanged();
	if (bFromQml)
		m_tDayModel->setDayNotes(m_dayNotes);
}

void QmlTDayInterface::setEditMode(const bool new_value)
{
	if (m_bEditMode != new_value)
	{
		m_bEditMode = new_value;
		emit editModeChanged();
		setDayIsEditable(m_bEditMode);
		setDayIsFinished(m_bEditMode);
	}
}

void QmlTDayInterface::setDayIsFinished(const bool new_value, const bool bFromQml)
{
	if (m_bDayIsFinished != new_value)
	{
		m_bDayIsFinished = new_value;
		emit dayIsFinishedChanged();
		if (bFromQml)
		{
			const QDate& date{m_tDayModel->date()};
			appMesoModel()->mesoCalendarModel(m_mesoIdx)->setDayIsFinished(date, new_value);
			appDBInterface()->setDayIsFinished(m_mesoIdx, date, new_value);
			if (new_value)
			{
				calculateWorkoutTime();
				rollUpExercises();
			}
			appDBInterface()->saveTrainingDay(m_tDayModel);
		}
	}
}

void QmlTDayInterface::setDayIsEditable(const bool new_value)
{
	m_bDayIsEditable = new_value;
	emit dayIsEditableChanged();
	if (m_exerciseManager)
		m_exerciseManager->setExercisesEditable(m_bDayIsEditable);
}

void QmlTDayInterface::setMainDateIsToday(const bool new_value)
{
	if (m_bMainDateIsToday != new_value)
	{
		m_bMainDateIsToday = new_value;
		emit mainDateIsTodayChanged();
		if (m_bMainDateIsToday && !m_workoutTimer)
		{
			m_workoutTimer = new TPTimer(this);
			connect(m_workoutTimer, &TPTimer::timeWarning, this, [this] (QString remaingTime, bool bminutes) {
				QMetaObject::invokeMethod(m_tDayPage, "displayTimeWarning", Q_ARG(QString, remaingTime), Q_ARG(bool, bminutes));
			});
			connect(m_workoutTimer, &TPTimer::hoursChanged, this, [this] () { setTimerHour(m_workoutTimer->hours()); });
			connect(m_workoutTimer, &TPTimer::minutesChanged, this, [this] () { setTimerMinute(m_workoutTimer->minutes()); });
			connect(m_workoutTimer, &TPTimer::secondsChanged, this, [this] () { setTimerSecond(m_workoutTimer->seconds()); });
		}
	}
}
//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

void QmlTDayInterface::setMesoIdx(const uint new_meso_idx)
{
	m_mesoIdx = new_meso_idx;
	m_tDayModel->setMesoIdx(new_meso_idx);
}

void QmlTDayInterface::getTrainingDayPage()
{
	if (!m_tDayPage)
	{
		if (!appMesoModel()->mesoCalendarModel(m_mesoIdx)->isReady())
		{
			connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint db_id) {
				getTrainingDayPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appDBInterface()->getMesoCalendar(m_mesoIdx);
			return;
		}

		const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
		const QString& tday{QString::number(mesoCal->getTrainingDay(m_Date.month(), m_Date.day()-1))};
		const QString& strSplitLetter{mesoCal->getSplitLetter(m_Date.month(), m_Date.day()-1)};

		m_tDayModel = new DBTrainingDayModel{this, m_mesoIdx};
		m_tDayModel->appendRow();
		m_tDayModel->setMesoId(appMesoModel()->id(m_mesoIdx));
		m_tDayModel->setDate(m_Date);

		m_splitLetter = std::move(strSplitLetter);
		m_tDayModel->setSplitLetter(m_splitLetter, false);
		m_tDayModel->setTrainingDay(tday, false);
		m_tDayProperties.insert("tDayManager"_L1, QVariant::fromValue(this));
		createTrainingDayPage();
	}
	else
	{
		setNeedActivation(true);
		emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_Date), m_tDayPage);
	}
}

void QmlTDayInterface::loadExercisesFromDate(const QString& strDate)
{
	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint) {
		loadExercises();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appDBInterface()->loadExercisesFromDate(strDate, m_tDayModel);
}

void QmlTDayInterface::loadExercisesFromMesoPlan(DBMesoSplitModel* const splitModel)
{
	if (!splitModel)
		emit requestMesoSplitModel(splitLetter().at(0));
	else
	{
		if (splitModel->count() > 0)
		{
			m_tDayModel->convertMesoSplitModelToTDayModel(splitModel);
			setHasExercises(true);
			m_exerciseManager->createExercisesObjects();
		}
	}
}

void QmlTDayInterface::convertTDayToPlan()
{
	emit convertTDayToSplitPlan(m_tDayModel);
}

void QmlTDayInterface::resetWorkout()
{
	setTimeIn("--:--"_L1);
	setTimeOut("--:--"_L1);
	setDayIsFinished(false);
	m_workoutTimer->prepareTimer();
}

void QmlTDayInterface::changeSplit(const QString& newSplitLetter, const bool bClearExercises)
{
	if (bClearExercises)
		clearExercises();
	setSplitLetter(newSplitLetter, false);
}

void QmlTDayInterface::adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay)
{
	uint tDay(0);
	if (newSplitLetter != "R"_L1)
	{
		if (m_tDayModel->splitLetter() == "R"_L1)
			tDay = m_tDayModel->getWorkoutNumberForTrainingDay();
	}
	else
		setDayIsFinished(false);

	m_tDayModel->setTrainingDay(QString::number(tDay), false);
	m_tDayModel->setSplitLetter(newSplitLetter, true);
	if (bOnlyThisDay)
		appDBInterface()->updateMesoCalendarEntry(m_mesoIdx, m_Date, m_tDayModel->trainingDay(), m_splitLetter);
	else
		appDBInterface()->updateMesoCalendarModel(m_mesoIdx, m_Date, m_splitLetter);
	if (newSplitLetter != "R"_L1)
		appDBInterface()->verifyTDayOptions(m_tDayModel);
}

void QmlTDayInterface::exportTrainingDay(const bool bShare)
{
	const QString& exportFileName{appItemManager()->setExportFileName(tr(" - Workout ") + splitLetter() + ".txt"_L1)};
	appItemManager()->continueExport(m_tDayModel->exportToFile(exportFileName), bShare);
}

void QmlTDayInterface::importTrainingDay(const QString& filename)
{
	if (filename.isEmpty())
	{
		appMesoModel()->setImportIdx(m_mesoIdx);
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	}
	else
		appItemManager()->openRequestedFile(filename, IFC_TDAY);
}

void QmlTDayInterface::prepareWorkOutTimer(const QString& strStartTime, const QString& strEndTime)
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
		else //some error made the app crash. We have saved the start time on tDayModel, but it is a little later now, so adjust
			m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(strStartTime, appUtils()->getCurrentTimeString()));
	}
}

void QmlTDayInterface::startWorkout()
{
	if (timeIn().contains('-'))
		setTimeIn(appUtils()->getCurrentTimeString());
	setDayIsEditable(true);
	m_workoutTimer->startTimer();
	setTimerActive(true);
}

void QmlTDayInterface::stopWorkout()
{
	m_workoutTimer->stopTimer();
	setTimerActive(false);
	if (!m_workoutTimer->stopWatch())
	{
		const QTime& elapsedTime{m_workoutTimer->elapsedTime()};
		setTimerHour(elapsedTime.hour());
		setTimerMinute(elapsedTime.minute());
		setTimerSecond(elapsedTime.second());
	}
	setTimeOut(appUtils()->getCurrentTimeString());
	setDayIsEditable(false);
	setDayIsFinished(true);
}

void QmlTDayInterface::clearExercises()
{
	m_exerciseManager->clearExercises();
	setHasExercises(false);
}

void QmlTDayInterface::removeExercise(const uint exercise_idx)
{
	m_exerciseManager->removeExerciseObject(exercise_idx);
	setHasExercises(m_exerciseManager->exercisesCount() > 0);
}

void QmlTDayInterface::removeSetFromExercise(const uint exercise_idx, const uint set_number)
{
	m_exerciseManager->removeExerciseSet(exercise_idx, set_number);
}

void QmlTDayInterface::removeExerciseObject(const uint exercise_idx, const bool bAsk)
{
	if (bAsk)
		askRemoveExercise(exercise_idx);
	else
		removeExercise(exercise_idx);
}

void QmlTDayInterface::moveExercise(const uint exercise_idx, const uint new_idx)
{
	m_exerciseManager->moveExercise(exercise_idx, new_idx);
}

void QmlTDayInterface::simpleExercisesList(const uint exercise_idx, const bool show, const bool multi_sel, const uint comp_exercise)
{
	m_SimpleExercisesListRequesterExerciseIdx = exercise_idx;
	m_SimpleExercisesListRequesterExerciseComp = comp_exercise;
	if (show)
	{
		if (appExercisesModel()->count() == 0)
			appDBInterface()->getAllExercises();
		connect(m_tDayPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
		connect(m_tDayPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_tDayPage, "showSimpleExercisesList", Q_ARG(bool, multi_sel));
	}
	else
	{
		disconnect(m_tDayPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
		disconnect(m_tDayPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_tDayPage, "hideSimpleExercisesList");
	}
}

void QmlTDayInterface::displayMessage(const QString& title, const QString& message, const bool error, const uint msecs) const
{
	QMetaObject::invokeMethod(m_tDayPage, "showMessageDialog", Q_ARG(QString, title), Q_ARG(QString, message), Q_ARG(bool, error), Q_ARG(int, static_cast<int>(msecs)));
}

void QmlTDayInterface::askRemoveExercise(const uint exercise_idx)
{
	if (appSettings()->alwaysAskConfirmation())
		QMetaObject::invokeMethod(m_tDayPage, "showRemoveExerciseMessage", Q_ARG(int, static_cast<int>(exercise_idx)),
			Q_ARG(QString, m_tDayModel->exerciseName(exercise_idx)));
	else
		removeExercise(exercise_idx);
}

void QmlTDayInterface::askRemoveSet(const uint exercise_idx, const uint set_number)
{
	if (appSettings()->alwaysAskConfirmation())
		QMetaObject::invokeMethod(m_tDayPage, "showRemoveSetMessage", Q_ARG(int, static_cast<int>(exercise_idx)),
			Q_ARG(int, static_cast<int>(exercise_idx)));
	else
		removeSetFromExercise(exercise_idx, set_number);
}

void QmlTDayInterface::gotoNextExercise(const uint exercise_idx)
{
	m_exerciseManager->gotoNextExercise(exercise_idx);
}

void QmlTDayInterface::rollUpExercises() const
{
	m_exerciseManager->hideSets();
}

TPTimer* QmlTDayInterface::restTimer()
{
	if (!m_restTimer)
		m_restTimer = new TPTimer(this);
	return m_restTimer;
}

void QmlTDayInterface::createExerciseObject()
{
	m_exerciseManager->createExerciseObject();
	setHasExercises(true);
}

void QmlTDayInterface::silenceTimeWarning()
{
	m_workoutTimer->stopAlarmSound();
}

void QmlTDayInterface::exerciseSelected(QmlExerciseEntry* exerciseEntry)
{
	const bool b_is_composite(appExercisesModel()->selectedEntriesCount() > 1);
	const QString& nSets{appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_SETSNUMBER)};
	const QString& nReps{appUtils()->makeCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER),
							2, comp_exercise_separator)};
	const QString& nWeight{appUtils()->makeCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT),
							2, comp_exercise_separator)};
	QString exerciseName{appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + " - "_L1 +
					appExercisesModel()->selectedEntriesValue_fast(0, 2)};

	if (!exerciseEntry)
		exerciseEntry = m_exerciseManager->exerciseEntry(m_SimpleExercisesListRequesterExerciseIdx);
	switch (m_SimpleExercisesListRequesterExerciseComp)
	{
		case 0:
			exerciseEntry->setExerciseName(exerciseName);
			exerciseEntry->setNewSetType(b_is_composite ? SET_TYPE_GIANT : SET_TYPE_REGULAR);
			exerciseEntry->setSetsNumber(nSets);
			exerciseEntry->setReps(nReps);
			exerciseEntry->setWeight(nWeight);
		break;
		case 1:
			appUtils()->setCompositeValue(1, m_tDayModel->exerciseName2(m_SimpleExercisesListRequesterExerciseIdx), exerciseName, comp_exercise_separator);
			exerciseEntry->setExerciseName(exerciseName);
		break;
		case 2:
			appUtils()->setCompositeValue(0, m_tDayModel->exerciseName1(m_SimpleExercisesListRequesterExerciseIdx), exerciseName, comp_exercise_separator);
			exerciseEntry->setExerciseName(exerciseName);
	}
}

void QmlTDayInterface::hideSimpleExercisesList()
{
	simpleExercisesList(-1, false, false, 0);
}

void QmlTDayInterface::createTrainingDayPage()
{
	m_tDayComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/TrainingDayPage.qml"_L1}, QQmlComponent::Asynchronous};
	if (m_tDayComponent->status() != QQmlComponent::Ready)
		connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
			{ return createTrainingDayPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createTrainingDayPage_part2();
}

void QmlTDayInterface::createTrainingDayPage_part2()
{
	#ifndef QT_NO_DEBUG
	if (m_tDayComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayComponent->errorString();
		for (uint i(0); i < m_tDayComponent->errors().count(); ++i)
			qDebug() << m_tDayComponent->errors().at(i).description();
		return;
	}
	#endif
	m_tDayPage = static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_tDayPage, QQmlEngine::CppOwnership);
	m_tDayPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_exerciseManager = new QmlExerciseInterface{this, this, m_tDayModel, m_tDayPage->findChild<QQuickItem*>("tDayExercisesLayout"_L1)};

	connect(this, &QmlTDayInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QmlTDayInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_tDayModel->date()), m_tDayPage);
	setHeaderText();

	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_id, const QVariant data) {
		if (table_id == TRAININGDAY_TABLE_ID)
		{
			disconnect(*conn);

			setMainDateIsToday(m_Date == QDate::currentDate());
			setEditMode(false);
			setDayIsFinished(false, false);
			setHasMesoPlan(false);
			setHasPreviousTDays(false);
			setNeedActivation(false);
			setTimerActive(false);
			setHasExercises(false);

			const bool bFinished(!m_tDayModel->timeOut().isEmpty());
			setDayIsFinished(bFinished, false);
			setTimeOut(bFinished ? m_tDayModel->timeOut() : "--:--"_L1, false);
			setDayNotes(m_tDayModel->dayNotes(), false);
			setTimeIn(m_tDayModel->timeIn().isEmpty() ? "--:--"_L1 : m_tDayModel->timeIn(), false);
			if (bFinished)
				calculateWorkoutTime();

			if (data.isValid())
			{
				const DBTrainingDayModel* const tDayModel{data.value<DBTrainingDayModel*>()};
				//The connected signal is only meant for the working page. All *possible* other pages are not to be affected by it, so we must filter them out
				if (tDayModel->dateStr() == m_tDayModel->dateStr())
				{
					if (m_tDayModel->splitLetter() != "R"_L1)
						setTrainingDayPageEmptyDayOrChangedDayOptions(data.value<DBTrainingDayModel*>());
				}
			}
			else
			{
				if (m_tDayModel->isReady())
					loadExercises();
			}
		}
	});

	if (m_tDayModel->splitLetter() != "R"_L1)
		appDBInterface()->getTrainingDay(m_tDayModel);

	connect(appMesoModel()->mesoCalendarModel(m_mesoIdx), &DBMesoCalendarModel::calendarChanged, this, [this]
																				(const QDate& startDate, const QDate& endDate) {
		if (m_tDayPage)
			updateTDayPageWithNewCalendarInfo(startDate, endDate);
	});

	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar& chrSplitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			if (splitLetter().at(0) == chrSplitLetter)
				setHeaderText();
		}
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx && field == MESOCYCLES_COL_SPLIT)
			QMetaObject::invokeMethod(m_tDayPage, "changeComboModel", Q_ARG(QString, appMesoModel()->split(m_mesoIdx)));
	});

	connect(m_tDayModel, &DBTrainingDayModel::tDayChanged, this, [this] () {
		appDBInterface()->saveTrainingDay(m_tDayModel);
	});

	if (mainDateIsToday())
		connect(m_tDayPage, SIGNAL(silenceTimeWarning()), this, SLOT(silenceTimeWarning()));

	QMetaObject::invokeMethod(m_tDayPage, "createNavButtons");
}

void QmlTDayInterface::updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate)
{
	if (m_Date > startDate) //the startDate page is the page that initiated the update. No need to alter it
	{
		if (m_Date <= endDate)
		{
			bool tDayChanged(false);
			const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
			const QString& tDay{QString::number(mesoCal->getTrainingDay(m_Date.month(), m_Date.day()))};
			if (tDay != m_tDayModel->trainingDay())
			{
				m_tDayModel->setTrainingDay(tDay);
				tDayChanged = true;
			}
			const QString& strSplitLetter{mesoCal->getSplitLetter(m_Date.month(), m_Date.day())};
			if (strSplitLetter != splitLetter())
			{
				setSplitLetter(strSplitLetter, false, true);
				tDayChanged = true;
				if (strSplitLetter == "R"_L1)
					clearExercises();
				else
					appDBInterface()->verifyTDayOptions(m_tDayModel);
			}
			if (tDayChanged)
				setHeaderText();
		}
	}
}

void QmlTDayInterface::loadExercises()
{
	m_exerciseManager->createExercisesObjects();
	setHasExercises(true);
}

void QmlTDayInterface::calculateWorkoutTime()
{
	if (!mainDateIsToday() || editMode() || dayIsFinished())
	{
		if (!timeIn().isEmpty() && !timeIn().contains('-'))
		{
			if (!timeOut().isEmpty() && !timeOut().contains('-'))
			{
				const QTime& workoutLenght{appUtils()->calculateTimeDifference(timeIn(), timeOut())};
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
			if (!timeOut().isEmpty() && !timeOut().contains('-'))
			{
				//optTimeConstrainedSession.checked = true;
				m_workoutTimer->setStopWatch(false);
				m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(appUtils()->getCurrentTimeString(), timeOut()));
			}
		}
	}
}

void QmlTDayInterface::setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const tDayModel)
{
	setLastWorkOutLocation(tDayModel->location());
	setHasMesoPlan(tDayModel->trainingDay() == STR_ONE); //trainingDay() is just a placeholder for the value we need
	if (tDayModel->count() == 2)
	{
		setHasPreviousTDays(true);
		setPreviousTDays(tDayModel->getRow_const(1));
	}
	else
	{
		setHasPreviousTDays(false);
		setPreviousTDays(QStringList());
	}
	QMetaObject::invokeMethod(m_tDayPage, "showIntentionDialog");
}
