#include "qmlmesointerface.h"

#include "dbinterface.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "qmlmesosplitinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlworkoutinterface.h"
#include "osinterface.h"
#include "tputils.h"
#include "translationclass.h"

#include <QRegularExpression>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QMLMesoInterface::cleanUp()
{
	if (m_mesoComponent)
	{
		delete m_mesoPage;
		delete m_mesoComponent;
	}
	if (m_exercisesPage)
		delete m_exercisesPage;
	if (m_calendarPage)
		delete m_calendarPage;

	for (const auto &it: std::as_const(m_workoutPages))
		delete it;
}

bool QMLMesoInterface::isMesoNameOK(const QString &meso_name) const
{
	if (meso_name.length() >= 5)
		return !appMesoModel()->mesoPlanExists(meso_name, appMesoModel()->coach(m_mesoIdx), appMesoModel()->client(m_mesoIdx));
	return false;
}

void QMLMesoInterface::setMesoNameOK(const bool nameok)
{
	if (m_mesoNameOK != nameok)
	{
		m_mesoNameOK = nameok;
		emit mesoNameOKChanged();
	}
}

void QMLMesoInterface::setStartDateOK(const bool dateok)
{
	if (m_startDateOK != dateok)
	{
		m_startDateOK = dateok;
		emit startDateOKChanged();
	}
}

void QMLMesoInterface::setEndDateOK(const bool dateok)
{
	if (m_endDateOK != dateok)
	{
		m_endDateOK = dateok;
		emit endDateOKChanged();
	}
}

bool QMLMesoInterface::realMeso() const
{
	return appMesoModel()->isRealMeso(m_mesoIdx);
}

void QMLMesoInterface::setRealMeso(const bool new_value)
{
	if (appMesoModel()->isRealMeso(m_mesoIdx) != new_value)
	{
		appMesoModel()->setIsRealMeso(m_mesoIdx, new_value);
		setEndDate(new_value ? appMesoModel()->endDate(m_mesoIdx) : maximumMesoEndDate());
		emit realMesoChanged();
	}
}

bool QMLMesoInterface::splitOK() const
{
	return isSplitOK(appMesoModel()->split(m_mesoIdx));
}

bool QMLMesoInterface::ownMeso() const
{
	std::optional<bool> own_meso{appMesoModel()->isOwnMeso(m_mesoIdx)};
	return own_meso.has_value() ? own_meso.value() : false;
}

bool QMLMesoInterface::isNewMeso() const
{
	return appMesoModel()->isNewMeso(m_mesoIdx);
}

bool QMLMesoInterface::isTempMeso() const
{
	return appMesoModel()->_id(m_mesoIdx) < 0;
}

QString QMLMesoInterface::name() const
{
	return appMesoModel()->name(m_mesoIdx);
}

void QMLMesoInterface::setName(const QString &new_name)
{
	if (appMesoModel()->name(m_mesoIdx) != new_name)
	{
		if (isMesoNameOK(new_name))
		{
			appMesoModel()->setName(m_mesoIdx, new_name);
			emit nameChanged();
			setMesoNameOK(true);
		}
		else {
			m_nameError = new_name.length() < 5 ?
							std::move(tr("Error: name too short") ): std::move(tr("Error: Name already in use."));
			setMesoNameOK(false);
		}
	}
}

QString QMLMesoInterface::coach() const
{
	return appUserModel()->userNameFromId(appMesoModel()->coach(m_mesoIdx));
}

void QMLMesoInterface::setCoach(const QString &new_value)
{
	if (appMesoModel()->coach(m_mesoIdx) != new_value)
	{
		appMesoModel()->setCoach(m_mesoIdx, new_value);
		emit coachChanged();
	}
}

QString QMLMesoInterface::client() const
{
	return appUserModel()->userNameFromId(appMesoModel()->client(m_mesoIdx));
}

void QMLMesoInterface::setClient(const QString &new_value)
{
	if (appMesoModel()->client(m_mesoIdx) != new_value)
	{
		appMesoModel()->setClient(m_mesoIdx, new_value);
		emit clientChanged();
		setMinimumMesoStartDate(appMesoModel()->getMesoMinimumStartDate(new_value, m_mesoIdx));
		setStartDate(m_minimumMesoStartDate);
	}
}

QString QMLMesoInterface::type() const
{
	return appMesoModel()->type(m_mesoIdx);
}

void QMLMesoInterface::setType(const QString &new_value)
{
	if (appMesoModel()->type(m_mesoIdx) != new_value)
	{
		appMesoModel()->setType(m_mesoIdx, new_value);
		emit typeChanged();
	}
}

QString QMLMesoInterface::displayFileName() const
{
	return appUtils()->getFileName(appMesoModel()->file(m_mesoIdx));
}

QString QMLMesoInterface::fileName() const
{
	return appMesoModel()->file(m_mesoIdx);
}

void QMLMesoInterface::setFileName(const QString &new_filename)
{
	if (appMesoModel()->file(m_mesoIdx) != new_filename)
	{
		const QString &good_filepath{appUtils()->getCorrectPath(new_filename)};
		if (!good_filepath.isEmpty())
		{
			if (!appUtils()->canReadFile(good_filepath))
				return;
		}
		appMesoModel()->setFile(m_mesoIdx, good_filepath);
		emit fileNameChanged();
		emit displayFileNameChanged();
	}
}

QDate QMLMesoInterface::startDate() const
{
	return appMesoModel()->startDate(m_mesoIdx);
}

void QMLMesoInterface::setStartDate(const QDate &new_startdate)
{
	if (m_startDate != new_startdate)
	{
		m_startDate = new_startdate;
		appMesoModel()->setStartDate(m_mesoIdx, m_startDate);
		m_strStartDate = appUtils()->formatDate(m_startDate);
		emit startDateChanged();
		appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
		emit weeksChanged();
		setStartDateOK(true);
	}
	else
		setStartDateOK(false);
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate &new_value)
{
	m_minimumMesoStartDate = new_value;
	emit minimumStartDateChanged();
}

void QMLMesoInterface::setEndDate(const QDate &new_enddate)
{
	if (m_endDate != new_enddate)
	{
		m_endDate = new_enddate;
		appMesoModel()->setEndDate(m_mesoIdx, m_endDate);
		m_strEndDate = appUtils()->formatDate(m_endDate);
		emit endDateChanged();
		appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
		emit weeksChanged();
		setEndDateOK(true);
	}
	else
		setEndDateOK(false);
}

void QMLMesoInterface::setMaximumMesoEndDate(const QDate &new_value)
{
	m_maximumMesoEndDate = new_value;
}

QString QMLMesoInterface::weeks() const
{
	return appMesoModel()->nWeeks(m_mesoIdx);
}

QString QMLMesoInterface::split() const
{
	return appMesoModel()->split(m_mesoIdx);
}

void QMLMesoInterface::setSplit(const QString &new_split)
{
	if (appMesoModel()->split(m_mesoIdx) != new_split)
	{
		if (isSplitOK(new_split))
		{
			appMesoModel()->setSplit(m_mesoIdx, new_split);
			emit splitChanged();
			emit splitOKChanged();
		}
	}
}

QString QMLMesoInterface::notes() const
{
	return appMesoModel()->notes(m_mesoIdx);
}

void QMLMesoInterface::setNotes(const QString &new_value)
{
	appMesoModel()->setNotes(m_mesoIdx, new_value);
	emit notesChanged();
}

QString QMLMesoInterface::muscularGroupA() const
{
	return appMesoModel()->splitA(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupA(const QString &new_value)
{
	if (appMesoModel()->splitA(m_mesoIdx) != new_value)
		appMesoModel()->setSplitA(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupB() const
{
	return appMesoModel()->splitB(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupB(const QString &new_value)
{
	if (appMesoModel()->splitB(m_mesoIdx) != new_value)
		appMesoModel()->setSplitB(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupC() const
{
	return appMesoModel()->splitC(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupC(const QString &new_value)
{
	if (appMesoModel()->splitC(m_mesoIdx) != new_value)
		appMesoModel()->setSplitC(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupD() const
{
	return appMesoModel()->splitD(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupD(const QString &new_value)
{
	if (appMesoModel()->splitD(m_mesoIdx) != new_value)
		appMesoModel()->setSplitD(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupE() const
{
	return appMesoModel()->splitE(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupE(const QString &new_value)
{
	if (appMesoModel()->splitE(m_mesoIdx) != new_value)
		appMesoModel()->setSplitE(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupF() const
{
	return appMesoModel()->splitF(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupF(const QString &new_value)
{
	if (appMesoModel()->splitF(m_mesoIdx) != new_value)
		appMesoModel()->setSplitF(m_mesoIdx, new_value);
}

QString QMLMesoInterface::muscularGroupR() const
{
	return appMesoModel()->splitR();
}

void QMLMesoInterface::changeMesoCalendar(const bool preserve_old_cal)
{
	appMesoModel()->mesoCalendarManager()->remakeMesoCalendar(m_mesoIdx, preserve_old_cal);
}

void QMLMesoInterface::doNotChangeMesoCalendar()
{
	mesoChanged(m_mesoIdx, 0);
}

void QMLMesoInterface::getCalendarPage()
{
	if (!m_calendarPage)
		m_calendarPage = new QmlMesoCalendarInterface{this, m_mesoIdx};
	m_calendarPage->getMesoCalendarPage();
}

void QMLMesoInterface::getExercisesPlannerPage()
{
	if (!m_exercisesPage)
		m_exercisesPage = new QmlMesoSplitInterface{this, m_mesoIdx};
	m_exercisesPage->getExercisesPlannerPage();
}

void QMLMesoInterface::getWorkoutPage(const QDate &date)
{
	QmlWorkoutInterface *workoutPage(m_workoutPages.value(date));
	if (!workoutPage)
	{
		workoutPage = new QmlWorkoutInterface{this, m_mesoIdx, date};
		m_workoutPages.insert(date, workoutPage);
	}
	workoutPage->getWorkoutPage();
}

void QMLMesoInterface::getMesocyclePage()
{
	if (!m_mesoComponent)
		createMesocyclePage();
	else
		appItemManager()->addMainMenuShortCut(appMesoModel()->name(m_mesoIdx), m_mesoPage);
}

void QMLMesoInterface::sendMesocycleFileToServer()
{
	appMesoModel()->sendMesoToUser(m_mesoIdx);
	m_canSendMesoToServer = false;
}

void QMLMesoInterface::incorporateMeso()
{
	appDBInterface()->saveMesocycle(m_mesoIdx);
}

void QMLMesoInterface::createMesocyclePage()
{
	if (!appMesoModel()->isNewMeso(m_mesoIdx))
	{
		setName(appMesoModel()->name(m_mesoIdx));
		setStartDate(appMesoModel()->startDate(m_mesoIdx));
		setEndDate(appMesoModel()->endDate(m_mesoIdx));
		setMinimumMesoStartDate(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
		setMaximumMesoEndDate(appMesoModel()->getMesoMaximumEndDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
	}
	else
	{
		QString meso_name{std::move(tr("New Program"))};
		uint i{1};
		while (!isMesoNameOK(meso_name))
			meso_name = std::move(tr("New Program") + " %1"_L1.arg(QString::number(i)));
		appMesoModel()->setName(m_mesoIdx, meso_name);
		setMesoNameOK(true);
		const QDate &minimumStartDate{appUtils()->getNextMonday(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), 99999))};
		const QDate &currentDate{QDate::currentDate()};
		setStartDate(currentDate);
		setEndDate(appUtils()->createDate(currentDate, 0, 2, 0));
		setMinimumMesoStartDate(minimumStartDate);
		setMaximumMesoEndDate(appUtils()->createDate(currentDate, 0, 6, 0));
	}
	setNewMesoFieldCounter(appMesoModel()->newMesoFieldCounter(m_mesoIdx));
	m_mesoProperties.insert("mesoManager"_L1, QVariant::fromValue(this));

	m_mesoComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesocyclePage.qml"_L1}, QQmlComponent::Asynchronous};
	if (m_mesoComponent->status() != QQmlComponent::Ready)
	{
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				createMesocyclePage_part2();
#ifndef QT_NO_DEBUG
			else if (status == QQmlComponent::Error)
			{
				qDebug() << m_mesoComponent->errorString();
				return;
			}
#endif
		}, Qt::SingleShotConnection);
	}
	else
		createMesocyclePage_part2();
}

void QMLMesoInterface::createMesocyclePage_part2()
{
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"_L1));

	connect(appOsInterface(), &OSInterface::appAboutToExit, this, [this] () {
		if (m_canSendMesoToServer)
			sendMesocycleFileToServer();
	});
	appItemManager()->addMainMenuShortCut(appMesoModel()->name(m_mesoIdx), m_mesoPage, [this] () {
		if (m_canSendMesoToServer)
			sendMesocycleFileToServer();
		appMesoModel()->removeMesoManager(m_mesoIdx);
	});

	connect(appMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
		{
			m_mesoIdx = new_meso_idx;
			for (const auto workout_page : std::as_const(m_workoutPages))
				workout_page->setMesoIdx(m_mesoIdx);
			if (m_exercisesPage)
				m_exercisesPage->setMesoIdx(m_mesoIdx);
			if (m_calendarPage)
				m_calendarPage->setMesoIdx(m_mesoIdx);
		}
	});

	if (appMesoModel()->isNewMeso(m_mesoIdx))
	{
		connect(appMesoModel(), &DBMesocyclesModel::isNewMesoChanged, this, [this] (const uint meso_idx) {
			if (meso_idx == m_mesoIdx)
				emit isNewMesoChanged();
		});
		connect(appMesoModel(), &DBMesocyclesModel::newMesoFieldCounterChanged, this, [this] (const uint meso_idx, const uint /*not yet used*/) {
			if (meso_idx == m_mesoIdx)
				setNewMesoFieldCounter(appMesoModel()->newMesoFieldCounter(m_mesoIdx));
		});
	}

	connect(appMesoModel(), &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint meso_field) {
		if (meso_idx == m_mesoIdx)
			mesoChanged(meso_idx, meso_field);
	});
	connect(appMesoModel(), &DBMesocyclesModel::canExportChanged, this, [this] (const uint meso_idx, const bool can_export) {
		if (meso_idx == m_mesoIdx)
		{
			m_bCanExport = can_export;
			emit canExportChanged();
		}
	});

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &QMLMesoInterface::labelsChanged);
}

void QMLMesoInterface::mesoChanged(const uint meso_idx, const uint meso_field)
{
	switch (meso_field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
		case MESOCYCLES_COL_SPLIT:
			if (!appMesoModel()->isNewMeso(meso_idx) && appMesoModel()->newMesoFieldCounter(meso_idx) == NEW_MESO_REQUIRED_FIELDS)
			{
				QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
				return;
			}
		break;
	}
	appDBInterface()->saveMesocycle(m_mesoIdx);
	if (!appMesoModel()->isNewMeso(m_mesoIdx))
	{
		if (!ownMeso())
			appMesoModel()->checkIfCanExport(m_mesoIdx);
		else
			m_canSendMesoToServer = appOsInterface()->tpServerOK();
	}
}

inline bool QMLMesoInterface::isSplitOK(const QString &split) const
{
	static const QRegularExpression rgex{"(?=.*[ABCDEF])(?=.*[R])"_L1};
	return rgex.match(split).hasMatch();
}
