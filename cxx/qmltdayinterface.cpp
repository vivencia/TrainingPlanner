#include "qmltdayinterface.h"

#include "dbexerciseslistmodel.h"
#include "dbinterface.h"
#include "DBMesoCalendarManager.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbworkoutmodel.h"
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
			if (m_splitLetter == "R"_L1)
				setDayIsEditable(false);
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
	const QString& strWhatToTrain{!bRestDay ? std::move(tr("Workout number: <b>") + m_tDayModel->trainingDay() + "</b>"_L1) : std::move(tr("Rest day"))};
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

void QmlTDayInterface::setEditMode(const bool new_value, const bool bFromQml)
{
	if (m_bEditMode != new_value)
	{
		m_bEditMode = new_value;
		if (bFromQml)
		{
			emit editModeChanged();
			setDayIsEditable(m_bEditMode);
			setDayIsFinished(m_bEditMode ? true : !m_tDayModel->timeOut().isEmpty());
		}
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
			appMesoModel()->setWorkoutIsFinished(m_mesoIdx, date, new_value);
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
		if (!appMesoModel()->mesoCalendarManager(m_mesoIdx)->isReady())
		{
			connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint db_id) {
				getTrainingDayPage();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appDBInterface()->getMesoCalendar(m_mesoIdx);
			return;
		}

		const DBMesoCalendarManager* const mesoCal(appMesoModel()->mesoCalendarManager(m_mesoIdx));
		const QString& tday{QString::number(mesoCal->getTrainingDay(m_Date.month(), m_Date.day()-1))};
		const QString& strSplitLetter{mesoCal->getSplitLetter(m_Date.month(), m_Date.day()-1)};

		m_tDayModel = new DBWorkoutModel{this, m_mesoIdx};
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
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this,conn] (const uint table_idx, QVariant data) {
			if (table_idx == MESOSPLIT_TABLE_ID)
			{
				disconnect(*conn);
				loadExercisesFromMesoPlan(data.value<DBMesoSplitModel*>());
			}
		});
		appDBInterface()->loadCompleteMesoSplit(m_mesoIdx, _splitLetter());
	}
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
	DBMesoSplitModel* splitModel{new DBMesoSplitModel{this, true, m_mesoIdx}};
	splitModel->setImportMode(true); //Have the database thread delete the model after procuring its contents
	splitModel->convertFromTDayModel(m_tDayModel);
	appDBInterface()->saveMesoSplitComplete(splitModel);
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
		clearExercises(false);
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
		appDBInterface()->updatemesoCalendarManager(m_mesoIdx, m_Date, m_splitLetter);
	if (newSplitLetter != "R"_L1)
	{
		appDBInterface()->verifyTDayOptions(m_tDayModel);
		QMetaObject::invokeMethod(m_tDayPage, "createNavButtons");
	}
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
		appItemManager()->openRequestedFile(filename, IFC_WORKOUT);
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

void QmlTDayInterface::clearExercises(const bool bShowIntentDialog)
{
	m_exerciseManager->clearExercises();
	setHasExercises(false);
	if (bShowIntentDialog)
		appDBInterface()->verifyTDayOptions(m_tDayModel);
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
		if (appExercisesList()->count() == 0)
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
	const bool b_is_composite(appExercisesList()->selectedEntriesCount() > 1);
	const QString& nSets{appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_SETSNUMBER)};
	const QString& nReps{b_is_composite ? appUtils()->makeCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_REPSNUMBER),
							2, comp_exercise_separator) : appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_REPSNUMBER)};
	const QString& nWeight{b_is_composite ? appUtils()->makeCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_WEIGHT),
							2, comp_exercise_separator) : appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_WEIGHT)};

	QString exerciseName;
	if (b_is_composite)
	{
		exerciseName = std::move(appUtils()->string_strings({
					appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_MAINNAME) +
						(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_SUBNAME).isEmpty() ? QString() :
							" - "_L1 + appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_SUBNAME)),
					appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_MAINNAME) +
						(appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_SUBNAME).isEmpty() ? QString() :
							" - "_L1 + appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_SUBNAME))}, comp_exercise_separator));
	}
	else
	{
		exerciseName = appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_MAINNAME);
		if (!appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_SUBNAME).isEmpty())
			exerciseName += " - "_L1 + appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_SUBNAME);
	}

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
			exerciseName = std::move(appUtils()->string_strings({ exerciseName,
					appUtils()->getCompositeValue(1, m_tDayModel->exerciseName(m_SimpleExercisesListRequesterExerciseIdx), comp_exercise_separator)
				}, comp_exercise_separator));
			exerciseEntry->setExerciseName(exerciseName);
		break;
		case 2:
			exerciseName = std::move(appUtils()->string_strings({
					appUtils()->getCompositeValue(0, m_tDayModel->exerciseName(m_SimpleExercisesListRequesterExerciseIdx), comp_exercise_separator),
				exerciseName }, comp_exercise_separator));
			exerciseEntry->setExerciseName(exerciseName);
		break;
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

	connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this] (const uint table_id, QVariant data) {
		if (table_id == WORKOUT_TABLE_ID)
		{
			const bool bFinished(!m_tDayModel->timeOut().isEmpty());

			setEditMode(false, false);
			setDayIsFinished(bFinished, false);
			setDayIsEditable(false);
			setHasMesoPlan(false);
			setHasPreviousTDays(false);
			setMainDateIsToday(m_Date == QDate::currentDate());
			setNeedActivation(false);
			setTimerActive(false);
			setHasExercises(false);
			setTimeOut(bFinished ? m_tDayModel->timeOut() : "--:--"_L1, false);
			setDayNotes(m_tDayModel->dayNotes(), false);
			setTimeIn(m_tDayModel->timeIn().isEmpty() ? "--:--"_L1 : m_tDayModel->timeIn(), false);
			if (bFinished)
				calculateWorkoutTime();

			if (data.isValid())
			{
				const DBWorkoutModel* const tDayModel{data.value<DBWorkoutModel*>()};
				//The connected signal is only meant for the working page. All *possible* other pages are not to be affected by it, so we must filter them out
				if (tDayModel->dateStr() == m_tDayModel->dateStr())
				{
					if (m_tDayModel->splitLetter() != "R"_L1)
						setTrainingDayPageEmptyDayOrChangedDayOptions(tDayModel);
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
	{
		appDBInterface()->getTrainingDay(m_tDayModel);
		QMetaObject::invokeMethod(m_tDayPage, "createNavButtons");
	}
	else
	{
		setEditMode(false, false);
		setDayIsFinished(true, false);
		setDayIsEditable(true);
		setHasMesoPlan(false);
		setHasPreviousTDays(false);
		setMainDateIsToday(m_Date == QDate::currentDate());
		setNeedActivation(false);
		setTimerActive(false);
		setHasExercises(false);
		setTimeIn("--:--"_L1, false);
		setTimeOut("--:--"_L1, false);
	}

	connect(appMesoModel()->mesoCalendarManager(m_mesoIdx), &DBMesoCalendarManager::calendarChanged, this, [this]
																				(const QDate& startDate, const QDate& endDate) {
		if (m_tDayPage)
			updateTDayPageWithNewCalendarInfo(startDate, endDate);
	});

	m_muscularGroup = std::move(appMesoModel()->muscularGroup(m_mesoIdx, _splitLetter()));
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar& chrSplitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			if (splitLetter().at(0) == chrSplitLetter)
				emit this->muscularGroupChanged();
		}
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx && field == MESOCYCLES_COL_SPLIT)
			QMetaObject::invokeMethod(m_tDayPage, "changeComboModel", Q_ARG(QString, appMesoModel()->split(m_mesoIdx)));
	});

	connect(m_tDayModel, &DBWorkoutModel::tDayChanged, this, [this] () {
		appDBInterface()->saveTrainingDay(m_tDayModel);
	});

	if (mainDateIsToday())
		connect(m_tDayPage, SIGNAL(silenceTimeWarning()), this, SLOT(silenceTimeWarning()));
}

void QmlTDayInterface::updateTDayPageWithNewCalendarInfo(const QDate& startDate, const QDate& endDate)
{
	if (m_Date > startDate) //the startDate page is the page that initiated the update. No need to alter it
	{
		if (m_Date <= endDate)
		{
			bool tDayChanged{false};
			const DBMesoCalendarManager* const mesoCal{appMesoModel()->mesoCalendarManager(m_mesoIdx)};
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
					clearExercises(false);
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
				m_workoutTimer->setStopWatch(false);
				m_workoutTimer->prepareTimer(appUtils()->calculateTimeDifference_str(appUtils()->getCurrentTimeString(), timeOut()));
			}
		}
	}
}

void QmlTDayInterface::setTrainingDayPageEmptyDayOrChangedDayOptions(const DBWorkoutModel* const tDayModel)
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
