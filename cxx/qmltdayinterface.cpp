#include "qmltdayinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaymodel.h"
#include "dbmesosplitmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "osinterface.h"
#include "tpappcontrol.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlTDayInterface::QmlTDayInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
	: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_tDayComponent(nullptr), m_mesoIdx(meso_idx)
{
	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
		{
			m_mesoIdx = new_meso_idx;
			m_tDayModel->setMesoIdx(new_meso_idx);
		}
	});
}

QmlTDayInterface::~QmlTDayInterface()
{
	/*QMapIterator<QDate,tDayExercises*> y(m_tDayExercisesList);
	y.toFront();
	while (y.hasNext()) {
		y.next();
		delete y.value();
	}
	if (m_setComponents[0])
		delete m_setComponents[0];
	if (m_setComponents[1])
		delete m_setComponents[1];
	if (m_setComponents[2])
		delete m_setComponents[2];
	delete m_tDayExercisesComponent;*/

	QMapIterator<QDate,DBTrainingDayModel*> x(m_tDayModels);
	x.toFront();
	while (x.hasNext()) {
		x.next();
		delete x.value();
	}
	//clearExerciseEntries(true);

	QMapIterator<QDate,QQuickItem*> i(m_tDayPages);
	i.toFront();
	while (i.hasNext()) {
		i.next();
		emit removePageFromMainMenu(i.value());
		delete i.value();
	}
	delete m_tDayComponent;
}

void QmlTDayInterface::getTrainingDayPage(const QDate& date)
{
	const QQuickItem* const tDayPage{m_tDayPages.value(date)};
	if (!tDayPage)
	{
		if (appMesoModel()->mesoCalendarModel(m_mesoIdx)->count() == 0)
		{
			connect(appDBInterface(), &DBInterface::databaseReady, this, [this,date] (const uint db_id) {
				getTrainingDayPage(date);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appDBInterface()->getMesoCalendar(m_mesoIdx);
			return;
		}
		static_cast<void>(gettDayModel(date));
		const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
		const QString& tday(QString::number(mesoCal->getTrainingDay(date.month(), date.day()-1)));
		const QString& splitLetter(mesoCal->getSplitLetter(date.month(), date.day()-1));

		if (m_tDayPage)
			m_tDayPage->setProperty("bNeedActivation", true);
		m_tDayPage = nullptr;
		m_tDayModel->appendRow();
		m_tDayModel->setMesoId(appMesoModel()->id(m_mesoIdx));
		m_tDayModel->setDate(date);
		m_tDayModel->setSplitLetter(splitLetter);
		m_tDayModel->setTrainingDay(tday);
		m_tDayModel->setTimeIn(u"--:--"_qs);
		m_tDayModel->setTimeOut(u"--:--"_qs);
		m_tDayProperties.insert(u"mainDate"_qs, date);
		m_tDayProperties.insert(u"tDayManager"_qs, QVariant::fromValue(this));
		m_tDayProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_tDayModel));
		m_tDayProperties.insert(u"bNeedActivation"_qs, false);
		m_tDayProperties.insert(u"mainDateIsToday"_qs, date == QDate::currentDate());
		createTrainingDayPage(date);
	}
	else
		emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(date), m_tDayPage);
}

void QmlTDayInterface::loadExercisesFromDate(const QString& strDate)
{
	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint) {
		const bool btoday(m_tDayModel->date() == QDate::currentDate());
		m_tDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appDBInterface()->loadExercisesFromDate(strDate, m_tDayModel);
}

void QmlTDayInterface::loadExercisesFromMesoPlan()
{
	DBMesoSplitModel* splitModel{getSplitModel(m_tDayModel->splitLetter().at(0))};
	if (splitModel->count() == 0)
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint) {
			loadExercisesFromMesoPlan();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appDBInterface()->loadExercisesFromMesoPlan(m_tDayModel, splitModel);
	}
	else
	{
		const bool btoday(m_tDayModel->date() == QDate::currentDate());
		m_tDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}
}

void QmlTDayInterface::convertTDayToPlan()
{
	appDBInterface()->convertTDayToPlan(m_tDayModel, getSplitModel(m_tDayModel->splitLetter().at(0)));
}

void QmlTDayInterface::resetWorkout()
{
	m_tDayModel->setTimeIn(u"--:--"_qs);
	m_tDayModel->setTimeOut(u"--:--"_qs);
	m_tDayModel->setDayIsFinished(false);
	m_tDayPage->setProperty("timeIn", m_tDayModel->timeIn());
	m_tDayPage->setProperty("timeOut", m_tDayModel->timeOut());
	m_tDayPage->setProperty("editMode", false);
	QMetaObject::invokeMethod(m_tDayPage, "resetTimer", Qt::AutoConnection);
}

void QmlTDayInterface::setDayIsFinished(const bool bFinished)
{
	m_tDayModel->setDayIsFinished(bFinished);
	const QDate& date(m_tDayModel->date());
	appMesoModel()->mesoCalendarModel(m_mesoIdx)->setDayIsFinished(date, bFinished);
	appDBInterface()->setDayIsFinished(m_mesoIdx, date, bFinished);
	if (bFinished)
		rollUpExercises();
}

void QmlTDayInterface::adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay)
{
	uint tDay{m_tDayPage->property("tDay").toUInt()};
	if (newSplitLetter != u"R"_qs)
	{
		if (m_tDayModel->splitLetter() == u"R"_qs)
			tDay = m_tDayModel->getWorkoutNumberForTrainingDay();
	}
	else
	{
		tDay = 0;
		m_tDayModel->setDayIsFinished(false);
	}
	m_tDayModel->setTrainingDay(QString::number(tDay), false);
	m_tDayModel->setSplitLetter(newSplitLetter, true);
	if (bOnlyThisDay)
		appDBInterface()->updateMesoCalendarEntry(m_tDayModel);
	else
		appDBInterface()->updateMesoCalendarModel(m_tDayModel);
	if (newSplitLetter != u"R"_qs)
		appDBInterface()->verifyTDayOptions(m_tDayModel);
	makeTDayPageHeaderLabel(m_tDayPage, m_tDayModel);
}

void QmlTDayInterface::setCurrenttDay(const QDate& date)
{
	m_tDayModel = m_tDayModels.value(date);
	m_tDayPage = m_tDayPages.value(date);
	//m_currentExercises = m_tDayExercisesList.value(date);
	appExercisesModel()->makeFilterString(appMesoModel()->muscularGroup(m_mesoIdx, m_tDayModel->splitLetter()));
}

void QmlTDayInterface::exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel)
{
	int exportFileMessageId(0);
	const QString& exportFileName{appOsInterface()->appDataFilesPath() + tr(" - Workout ") + tDayModel->splitLetter() + u".txt"_qs};
	exportFileMessageId = tDayModel->exportToFile(exportFileName);
	if (exportFileMessageId >= 0)
	{
		if (bShare)
		{
			appOsInterface()->shareFile(exportFileName);
			exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
		}
		else
			QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, exportFileName));
	}
	emit displayMessageOnAppWindow(exportFileMessageId, exportFileName);
}

void QmlTDayInterface::importTrainingDay(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(m_mainWindow, "chooseFileToImport");
	else
		appControl()->openRequestedFile(filename, IFC_TDAY);
}



void QmlTDayInterface::removeExerciseObject(const uint exercise_idx)
{
	if (exercise_idx < m_exerciseObjects.count())
	{
		m_tDayModel->removeExercise(exercise_idx);
		removeExerciseEntry(exercise_idx);
		for(uint i(exercise_idx); i < m_currentExercises->exerciseObjects.count(); ++i)
		{
			//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
			QMetaObject::invokeMethod(exerciseEntryItem(i), "moveExercise", Q_ARG(bool, true), Q_ARG(bool, false));
			for(uint x(0); x < exerciseSetsCount(i); ++x)
				exerciseSetItem(i, x)->setProperty("exerciseIdx", i);
		}
	}
}

void QmlTDayInterface::clearExercises()
{
	clearExerciseEntries();
	m_tDayModel->clearExercises();
	m_tDayModel->setDayIsFinished(false);
}

void QmlTDayInterface::moveExercise(const uint exercise_idx, const uint new_idx)
{
	uint nsets(exerciseSetsCount(exercise_idx));
	for(uint i(0); i < nsets; ++i)
	{
		exerciseSetItem(exercise_idx, i)->setProperty("exerciseIdx", new_idx);
		exerciseSetItem(exercise_idx, i)->setProperty("ownerExercise", QVariant::fromValue(exerciseEntryItem(new_idx)));
	}
	nsets = exerciseSetsCount(new_idx);
	for(uint i(0); i < nsets; ++i)
	{
		exerciseSetItem(new_idx, i)->setProperty("exerciseIdx", exercise_idx);
		exerciseSetItem(new_idx, i)->setProperty("ownerExercise", QVariant::fromValue(exerciseEntryItem(exercise_idx)));
	}
	m_tDayModel->moveExercise(exercise_idx, new_idx);

	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(nullptr);

	QQuickItem* parentLayout(m_tDayPage->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
	m_currentExercises->exerciseObjects.swapItemsAt(exercise_idx, new_idx);
	for(uint x(0); x < m_currentExercises->exerciseObjects.count(); ++x)
		exerciseEntryItem(x)->setParentItem(parentLayout);
	//Changing the properties via c++ is not working for some unknown reason. Let QML update its properties then
	QMetaObject::invokeMethod(exerciseEntryItem(exercise_idx), "moveExercise", Q_ARG(bool, new_idx > exercise_idx), Q_ARG(bool, false));
}

void QmlTDayInterface::createTrainingDayPage(const QDate& date)
{
	if (!m_tDayPages.contains(date))
	{
		if (m_tDayComponent == nullptr)
			m_tDayComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/TrainingDayInfo.qml"_qs}, QQmlComponent::Asynchronous};

		/*if (!m_tDayExercisesList.contains(date))
		{
			m_currentExercises = new tDayExercises;
			m_tDayExercisesList.insert(date, m_currentExercises);
		}*/

		if (m_tDayComponent->status() != QQmlComponent::Ready)
			connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status)
				{ return createTrainingDayPage_part2(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		else
			createTrainingDayPage_part2();
	}
}

void QmlTDayInterface::createTrainingDayPage_part2()
{
	#ifdef DEBUG
	if (m_tDayComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayComponent->errorString();
		for (uint i(0); i < m_tDayComponent->errors().count(); ++i)
			qDebug() << m_tDayComponent->errors().at(i).description();
		return;
	}
	#endif
	m_tDayPage = static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_qmlEngine->rootContext()));
	m_qmlEngine->setObjectOwnership(m_tDayPage, QQmlEngine::CppOwnership);
	m_tDayPage->setParentItem(m_mainWindow->findChild<QQuickItem*>("appStackView"));
	m_tDayPages.insert(m_tDayModels.key(m_tDayModel), m_tDayPage);

	emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_tDayModel->date()), m_tDayPage);

	makeTDayPageHeaderLabel(m_tDayPage, m_tDayModel);

	connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this] (const uint table_id, const QVariant data) {
		if (table_id == TRAININGDAY_TABLE_ID)
		{
			const DBTrainingDayModel* const tDayModel{data.value<DBTrainingDayModel*>()};
			//The connected signal is only meant for the working page. All *possible* other pages are not affected by it, so we must filter them out
			if (tDayModel->dateStr() == m_tDayModel->dateStr())
			{
				if (m_tDayModel->splitLetter() != u"R"_qs)
					setTrainingDayPageEmptyDayOrChangedDayOptions(data.value<DBTrainingDayModel*>());
			}
		}
	});

	if (m_tDayModel->splitLetter() != u"R"_qs)
		appDBInterface()->getTrainingDay(m_tDayModel);

	connect(appMesoModel()->mesoCalendarModel(m_mesoIdx), &DBMesoCalendarModel::calendarChanged, this, [this]
																				(const QDate& startDate, const QDate& endDate) {
		updateOpenTDayPagesWithNewCalendarInfo(startDate, endDate);
	});

	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const uint initiator_id, const int splitIndex, const QChar& splitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			QMap<QDate,QQuickItem*>::const_iterator itr{m_tDayPages.constBegin()};
			const QMap<QDate,QQuickItem*>::const_iterator& itr_end{m_tDayPages.constEnd()};
			while (itr != itr_end)
			{
				const DBTrainingDayModel* const tDayModel{m_tDayModels.value(itr.key())};
				if (tDayModel->splitLetter() == splitLetter)
					makeTDayPageHeaderLabel((*itr), tDayModel);
				++itr;
			}
		}
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx && field == MESOCYCLES_COL_SPLIT)
		{
			QMap<QDate,QQuickItem*>::const_iterator itr{m_tDayPages.constBegin()};
			const QMap<QDate,QQuickItem*>::const_iterator& itr_end{m_tDayPages.constEnd()};
			while (itr != itr_end)
			{
				QMetaObject::invokeMethod(*itr, "changeComboModel", Qt::AutoConnection);
				++itr;
			}
		}
	});

	connect(m_tDayModel, &DBTrainingDayModel::exerciseCompleted, this, [this] (const uint exercise_idx, const bool completed) {
		enableDisableExerciseCompletedButton(exercise_idx, completed);
	});

	connect(m_tDayModel, &DBTrainingDayModel::tDayChanged, this, [this] () {
		appDBInterface()->saveTrainingDay(m_tDayModel);
	});
}

void QmlTDayInterface::updateOpenTDayPagesWithNewCalendarInfo(const QDate& startDate, const QDate& endDate)
{
	QMap<QDate,QQuickItem*>::const_iterator itr{m_tDayPages.constBegin()};
	const QMap<QDate,QQuickItem*>::const_iterator& itr_end{m_tDayPages.constEnd()};
	const DBMesoCalendarModel* const mesoCal(appMesoModel()->mesoCalendarModel(m_mesoIdx));
	bool tDayChanged(false);
	while (itr != itr_end)
	{
		const QDate& date(itr.key());
		if (date > startDate) //the startDate page is the page that initiated the update. No need to alter it
		{
			if (date <= endDate)
			{
				DBTrainingDayModel* tDayModel{m_tDayModels.value(itr.key())};
				const QString& tDay{QString::number(mesoCal->getTrainingDay(date.month(), date.day()))};
				if (tDay != tDayModel->trainingDay())
				{
					tDayModel->setTrainingDay(tDay);
					tDayChanged = true;
				}
				const QString& splitLetter{mesoCal->getSplitLetter(date.month(), date.day())};
				if (splitLetter != tDayModel->splitLetter())
				{
					tDayModel->setSplitLetter(splitLetter);
					tDayChanged = true;
					if (splitLetter == u"R"_qs)
						clearExercises();
					else
						appDBInterface()->verifyTDayOptions(tDayModel);
				}
				if (tDayChanged)
					makeTDayPageHeaderLabel((*itr), tDayModel);
				tDayChanged = false;
			}
		}
		++itr;
	}
}

void QmlTDayInterface::makeTDayPageHeaderLabel(QQuickItem* tDayPage, const DBTrainingDayModel* const tDayModel)
{
	const bool bRestDay(tDayModel->splitLetter() == u"R"_qs);
	QString strWhatToTrain;
	if (!bRestDay)
	{
		appExercisesModel()->makeFilterString(appMesoModel()->muscularGroup(m_mesoIdx, tDayModel->splitLetter()));
		strWhatToTrain = tr("Workout number: <b>") + tDayModel->trainingDay() + u"</b><br><b>"_qs +
			appMesoModel()->muscularGroup(m_mesoIdx, tDayModel->splitLetter() + u"</b>"_qs);
	}
	else
		strWhatToTrain = tr("Rest day");
	const QString& headerText(u"<b>"_qs + appUtils()->formatDate(tDayModel->date()) + u"</b><br>"_qs + strWhatToTrain);
	tDayPage->setProperty("headerText", headerText);

}

void QmlTDayInterface::setTrainingDayPageEmptyDayOrChangedDayOptions(const DBTrainingDayModel* const tDayModel)
{
	m_tDayPage->setProperty("timeIn", m_tDayModel->timeIn());
	m_tDayPage->setProperty("timeOut", m_tDayModel->timeOut());
	if (m_tDayModel->timeOut() != u"--:--"_qs)
		m_tDayModel->setDayIsFinished(true);
	if (m_tDayModel->dayIsFinished())
	{
		const QTime& workoutLenght(appUtils()->calculateTimeDifference(m_tDayModel->timeIn(), m_tDayModel->timeOut()));
			QMetaObject::invokeMethod(m_tDayPage, "updateTimer", Q_ARG(int, workoutLenght.hour()),
				Q_ARG(int, workoutLenght.minute()), Q_ARG(int, workoutLenght.second()));
	}

	if (tDayModel->isReady())
	{
		m_tDayPage->setProperty("lastWorkOutLocation", tDayModel->location());
		//TDAY_COL_TRAININGDAYNUMBER is just a placeholder for the value we need
		m_tDayPage->setProperty("bHasMesoPlan", tDayModel->trainingDay() == STR_ONE);
		if (tDayModel->count() == 2)
		{
			m_tDayPage->setProperty("previousTDays", QVariant::fromValue(tDayModel->getRow_const(1)));
			m_tDayPage->setProperty("bHasPreviousTDays", true);
		}
	}
	else
	{
		m_tDayPage->setProperty("previousTDays", QVariant::fromValue(QStringList()));
		m_tDayPage->setProperty("bHasMesoPlan", false);
		m_tDayPage->setProperty("bHasPreviousTDays", false);
	}
	QMetaObject::invokeMethod(m_tDayPage, "showIntentionDialog");
}

void QmlTDayInterface::rollUpExercises() const
{
	for (uint i(0); i < exercisesCount(); ++i)
		QMetaObject::invokeMethod(exerciseEntryItem(i), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	QMetaObject::invokeMethod(m_tDayPage, "placeSetIntoView", Q_ARG(int, -100));
}
