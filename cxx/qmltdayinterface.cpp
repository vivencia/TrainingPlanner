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
			QMap<QDate,DBTrainingDayModel*>::const_iterator itr(m_tDayModels.constBegin());
			const QMap<QDate,DBTrainingDayModel*>::const_iterator& itr_end(m_tDayModels.constEnd());
			while (itr != itr_end)
				(*itr)->setMesoIdx(new_meso_idx);
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

		if (m_currenttDayPage)
			m_currenttDayPage->setProperty("bNeedActivation", true);
		m_currenttDayPage = nullptr;
		m_CurrenttDayModel->appendRow();
		m_CurrenttDayModel->setMesoId(appMesoModel()->id(m_mesoIdx));
		m_CurrenttDayModel->setDate(date);
		m_CurrenttDayModel->setSplitLetter(splitLetter);
		m_CurrenttDayModel->setTrainingDay(tday);
		m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
		m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
		m_tDayProperties.insert(u"mainDate"_qs, date);
		m_tDayProperties.insert(u"tDayManager"_qs, QVariant::fromValue(this));
		m_tDayProperties.insert(u"tDayModel"_qs, QVariant::fromValue(m_CurrenttDayModel));
		m_tDayProperties.insert(u"bNeedActivation"_qs, false);
		m_tDayProperties.insert(u"mainDateIsToday"_qs, date == QDate::currentDate());
		createTrainingDayPage(date);
	}
	else
		emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(date), m_currenttDayPage);
}

void QmlTDayInterface::loadExercisesFromDate(const QString& strDate)
{
	//setModified is called with param true because the loaded exercises do not -yet- belong to the day indicated by strDate
	connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint) {
		const bool btoday(m_CurrenttDayModel->date() == QDate::currentDate());
		m_CurrenttDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appDBInterface()->loadExercisesFromDate(strDate, m_CurrenttDayModel);
}

void QmlTDayInterface::loadExercisesFromMesoPlan()
{
	DBMesoSplitModel* splitModel{getSplitModel(m_CurrenttDayModel->splitLetter().at(0))};
	if (splitModel->count() == 0)
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint) {
			loadExercisesFromMesoPlan();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appDBInterface()->loadExercisesFromMesoPlan(m_CurrenttDayModel, splitModel);
	}
	else
	{
		const bool btoday(m_CurrenttDayModel->date() == QDate::currentDate());
		m_CurrenttDayModel->setDayIsFinished(!btoday);
		createExercisesObjects();
	}
}

void QmlTDayInterface::convertTDayToPlan()
{
	appDBInterface()->convertTDayToPlan(m_CurrenttDayModel, getSplitModel(m_CurrenttDayModel->splitLetter().at(0)));
}

void QmlTDayInterface::resetWorkout()
{
	m_CurrenttDayModel->setTimeIn(u"--:--"_qs);
	m_CurrenttDayModel->setTimeOut(u"--:--"_qs);
	m_CurrenttDayModel->setDayIsFinished(false);
	m_currenttDayPage->setProperty("timeIn", m_CurrenttDayModel->timeIn());
	m_currenttDayPage->setProperty("timeOut", m_CurrenttDayModel->timeOut());
	m_currenttDayPage->setProperty("editMode", false);
	QMetaObject::invokeMethod(m_currenttDayPage, "resetTimer", Qt::AutoConnection);
}

void QmlTDayInterface::setDayIsFinished(const bool bFinished)
{
	m_CurrenttDayModel->setDayIsFinished(bFinished);
	const QDate& date(m_CurrenttDayModel->date());
	appMesoModel()->mesoCalendarModel(m_mesoIdx)->setDayIsFinished(date, bFinished);
	appDBInterface()->setDayIsFinished(m_mesoIdx, date, bFinished);
	if (bFinished)
		rollUpExercises();
}

void QmlTDayInterface::adjustCalendar(const QString& newSplitLetter, const bool bOnlyThisDay)
{
	uint tDay{m_currenttDayPage->property("tDay").toUInt()};
	if (newSplitLetter != u"R"_qs)
	{
		if (m_CurrenttDayModel->splitLetter() == u"R"_qs)
			tDay = m_CurrenttDayModel->getWorkoutNumberForTrainingDay();
	}
	else
	{
		tDay = 0;
		m_CurrenttDayModel->setDayIsFinished(false);
	}
	m_CurrenttDayModel->setTrainingDay(QString::number(tDay), false);
	m_CurrenttDayModel->setSplitLetter(newSplitLetter, true);
	if (bOnlyThisDay)
		appDBInterface()->updateMesoCalendarEntry(m_CurrenttDayModel);
	else
		appDBInterface()->updateMesoCalendarModel(m_CurrenttDayModel);
	if (newSplitLetter != u"R"_qs)
		appDBInterface()->verifyTDayOptions(m_CurrenttDayModel);
	makeTDayPageHeaderLabel(m_currenttDayPage, m_CurrenttDayModel);
}

void QmlTDayInterface::setCurrenttDay(const QDate& date)
{
	m_CurrenttDayModel = m_tDayModels.value(date);
	m_currenttDayPage = m_tDayPages.value(date);
	//m_currentExercises = m_tDayExercisesList.value(date);
	appExercisesModel()->makeFilterString(appMesoModel()->muscularGroup(m_mesoIdx, m_CurrenttDayModel->splitLetter()));
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

DBTrainingDayModel* QmlTDayInterface::gettDayModel(const QDate& date)
{
	if (!m_tDayModels.contains(date))
	{
		DBTrainingDayModel* tDayModel{new DBTrainingDayModel(this, m_mesoIdx)};
		m_tDayModels.insert(date, tDayModel);
		m_CurrenttDayModel = tDayModel;
	}
	else
		m_CurrenttDayModel = m_tDayModels.value(date);
	return m_CurrenttDayModel;
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
	m_currenttDayPage = static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_qmlEngine->rootContext()));
	m_qmlEngine->setObjectOwnership(m_currenttDayPage, QQmlEngine::CppOwnership);
	m_currenttDayPage->setParentItem(m_mainWindow->findChild<QQuickItem*>("appStackView"));
	m_tDayPages.insert(m_tDayModels.key(m_CurrenttDayModel), m_currenttDayPage);

	emit addPageToMainMenu(tr("Workout: ") + appUtils()->formatDate(m_CurrenttDayModel->date()), m_currenttDayPage);

	makeTDayPageHeaderLabel(m_currenttDayPage, m_CurrenttDayModel);

	connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this] (const uint table_id, const QVariant data) {
		if (table_id == TRAININGDAY_TABLE_ID)
		{
			const DBTrainingDayModel* const tDayModel{data.value<DBTrainingDayModel*>()};
			//The connected signal is only meant for the working page. All *possible* other pages are not affected by it, so we must filter them out
			if (tDayModel->dateStr() == m_CurrenttDayModel->dateStr())
			{
				if (m_CurrenttDayModel->splitLetter() != u"R"_qs)
					setTrainingDayPageEmptyDayOrChangedDayOptions(data.value<DBTrainingDayModel*>());
			}
		}
	});

	if (m_CurrenttDayModel->splitLetter() != u"R"_qs)
		appDBInterface()->getTrainingDay(m_CurrenttDayModel);

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

	connect(m_CurrenttDayModel, &DBTrainingDayModel::exerciseCompleted, this, [this] (const uint exercise_idx, const bool completed) {
		enableDisableExerciseCompletedButton(exercise_idx, completed);
	});

	connect(m_CurrenttDayModel, &DBTrainingDayModel::tDayChanged, this, [this] () {
		appDBInterface()->saveTrainingDay(m_CurrenttDayModel);
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
	m_currenttDayPage->setProperty("timeIn", m_CurrenttDayModel->timeIn());
	m_currenttDayPage->setProperty("timeOut", m_CurrenttDayModel->timeOut());
	if (m_CurrenttDayModel->timeOut() != u"--:--"_qs)
		m_CurrenttDayModel->setDayIsFinished(true);
	if (m_CurrenttDayModel->dayIsFinished())
	{
		const QTime& workoutLenght(appUtils()->calculateTimeDifference(m_CurrenttDayModel->timeIn(), m_CurrenttDayModel->timeOut()));
			QMetaObject::invokeMethod(m_currenttDayPage, "updateTimer", Q_ARG(int, workoutLenght.hour()),
				Q_ARG(int, workoutLenght.minute()), Q_ARG(int, workoutLenght.second()));
	}

	if (tDayModel->isReady())
	{
		m_currenttDayPage->setProperty("lastWorkOutLocation", tDayModel->location());
		//TDAY_COL_TRAININGDAYNUMBER is just a placeholder for the value we need
		m_currenttDayPage->setProperty("bHasMesoPlan", tDayModel->trainingDay() == STR_ONE);
		if (tDayModel->count() == 2)
		{
			m_currenttDayPage->setProperty("previousTDays", QVariant::fromValue(tDayModel->getRow_const(1)));
			m_currenttDayPage->setProperty("bHasPreviousTDays", true);
		}
	}
	else
	{
		m_currenttDayPage->setProperty("previousTDays", QVariant::fromValue(QStringList()));
		m_currenttDayPage->setProperty("bHasMesoPlan", false);
		m_currenttDayPage->setProperty("bHasPreviousTDays", false);
	}
	QMetaObject::invokeMethod(m_currenttDayPage, "showIntentionDialog");
}

void QmlTDayInterface::rollUpExercises() const
{
	for (uint i(0); i < exercisesCount(); ++i)
		QMetaObject::invokeMethod(exerciseEntryItem(i), "paneExerciseShowHide", Q_ARG(bool, false), Q_ARG(bool, true));
	QMetaObject::invokeMethod(m_currenttDayPage, "placeSetIntoView", Q_ARG(int, -100));
}
