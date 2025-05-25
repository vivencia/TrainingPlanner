#include "qmlmesointerface.h"

#include "dbinterface.h"
#include "dbmesocalendarmanager.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "qmlmesosplitinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlworkoutinterface.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QMLMesoInterface::~QMLMesoInterface()
{
	if (m_mesoComponent)
	{
		emit removePageFromMainMenu(m_mesoPage);
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

void QMLMesoInterface::setName(const QString &new_value, const bool bFromQml)
{
	if (bFromQml)
	{
		if (m_name != new_value)
		{
			if (new_value.length() >= 5)
			{
				if (!appMesoModel()->mesoPlanExists(new_value, appMesoModel()->coach(m_mesoIdx), appMesoModel()->client(m_mesoIdx)))
				{
					m_name = new_value;
					emit nameChanged();
					if (!isNewMeso())
						acceptName();
					setMesoNameOK(true);
					return;
				}
			}
			setMesoNameOK(false);
		}
	}
	else
	{
		m_name = new_value;
		setMesoNameOK(true, false);
	}
}

void QMLMesoInterface::acceptName()
{
	appMesoModel()->setName(m_mesoIdx, m_name);
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

QString QMLMesoInterface::fileName() const
{
	return appUtils()->getFileName(appMesoModel()->mesoFileName(m_mesoIdx));
}

QString QMLMesoInterface::file() const
{
	return appMesoModel()->file(m_mesoIdx);
}

void QMLMesoInterface::setFile(const QString &new_value)
{
	if (appMesoModel()->mesoFileName(m_mesoIdx) != new_value)
	{
		const QString &good_filepath{appUtils()->getCorrectPath(new_value)};
		if (!appUtils()->canReadFile(good_filepath))
			return;
		appMesoModel()->setFile(m_mesoIdx, good_filepath);
		emit fileChanged();
		emit fileNameChanged();
	}
}

QDate QMLMesoInterface::startDate() const
{
	return appMesoModel()->startDate(m_mesoIdx);
}

void QMLMesoInterface::setStartDate(const QDate &new_value, const bool bFromQml)
{
	if (m_startDate != new_value)
	{
		m_strStartDate = std::move(appUtils()->formatDate(new_value));
		if (bFromQml)
		{
			emit startDateChanged();
			if (!isNewMeso())
				acceptStartDate();
		}
	}
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate &new_value)
{
	m_minimumMesoStartDate = new_value;
	emit minimumStartDateChanged();
}

void QMLMesoInterface::acceptStartDate()
{
	appMesoModel()->setStartDate(m_mesoIdx, m_startDate);
	appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
	emit weeksChanged();
}

void QMLMesoInterface::setEndDate(const QDate &new_value, const bool bFromQml)
{
	if (m_endDate != new_value)
	{
		m_endDate = new_value;
		m_strEndDate = std::move(appUtils()->formatDate(new_value));
		if (bFromQml)
		{
			emit endDateChanged();
			if (!isNewMeso())
				acceptEndDate();
		}
	}
}

void QMLMesoInterface::setMaximumMesoEndDate(const QDate &new_value)
{
	m_maximumMesoEndDate = new_value;
}

void QMLMesoInterface::acceptEndDate()
{
	appMesoModel()->setEndDate(m_mesoIdx, m_endDate);
	appMesoModel()->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
	emit weeksChanged();
}

QString QMLMesoInterface::weeks() const
{
	return appMesoModel()->nWeeks(m_mesoIdx);
}

QString QMLMesoInterface::split() const
{
	return appMesoModel()->split(m_mesoIdx);
}

void QMLMesoInterface::setSplit(const QString &new_value)
{
	if (new_value.contains("R"_L1))
	{
		appMesoModel()->setSplit(m_mesoIdx, new_value);
		emit splitChanged();
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

void QMLMesoInterface::setMuscularGroupA(const QString &new_value)
{
	if (appMesoModel()->splitA(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitA(m_mesoIdx, new_value);
		emit muscularGroupAChanged();
	}
}

void QMLMesoInterface::setMuscularGroupB(const QString &new_value)
{
	if (appMesoModel()->splitB(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitB(m_mesoIdx, new_value);
		emit muscularGroupBChanged();
	}
}

void QMLMesoInterface::setMuscularGroupC(const QString &new_value)
{
	if (appMesoModel()->splitC(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitC(m_mesoIdx, new_value);
		emit muscularGroupCChanged();
	}
}

void QMLMesoInterface::setMuscularGroupD(const QString &new_value)
{
	if (appMesoModel()->splitD(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitD(m_mesoIdx, new_value);
		emit muscularGroupDChanged();
	}
}

void QMLMesoInterface::setMuscularGroupE(const QString &new_value)
{
	if (appMesoModel()->splitE(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitE(m_mesoIdx, new_value);
		emit muscularGroupEChanged();
	}
}

void QMLMesoInterface::setMuscularGroupF(const QString &new_value)
{
	if (appMesoModel()->splitF(m_mesoIdx) != new_value)
	{
		appMesoModel()->setSplitF(m_mesoIdx, new_value);
		emit muscularGroupFChanged();
	}
}

void QMLMesoInterface::changeMesoCalendar(const bool preserve_old_cal)
{
	appMesoModel()->mesoCalendarManager()->remakeMesoCalendar(m_mesoIdx, preserve_old_cal);
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
		emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);
}

void QMLMesoInterface::sendMesocycleFileToServer()
{
	appMesoModel()->sendMesoToUser(m_mesoIdx);
}

void QMLMesoInterface::incorporateMeso()
{
	appDBInterface()->saveMesocycle(m_mesoIdx);
}

void QMLMesoInterface::createMesocyclePage()
{
	if (!appMesoModel()->isNewMeso(m_mesoIdx))
	{
		setName(appMesoModel()->name(m_mesoIdx), false);
		setStartDate(appMesoModel()->startDate(m_mesoIdx), false);
		setEndDate(appMesoModel()->endDate(m_mesoIdx), false);
		setMinimumMesoStartDate(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
		setMaximumMesoEndDate(appMesoModel()->getMesoMaximumEndDate(appMesoModel()->client(m_mesoIdx), m_mesoIdx));
	}
	else
	{
		setName(std::move(tr("New Program")), false);
		const QDate &minimumStartDate{appUtils()->getNextMonday(appMesoModel()->getMesoMinimumStartDate(appMesoModel()->client(m_mesoIdx), 99999))};
		const QDate &currentDate{QDate::currentDate()};
		setStartDate(currentDate, false);
		setEndDate(appUtils()->createDate(currentDate, 0, 2, 0), false);
		setMinimumMesoStartDate(minimumStartDate);
		setMaximumMesoEndDate(appUtils()->createDate(currentDate, 0, 6, 0));
	}

	setNewMesoFieldCounter(appMesoModel()->newMesoFieldCounter(m_mesoIdx));
	m_mesoProperties.insert("mesoManager"_L1, QVariant::fromValue(this));

	m_mesoComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesocyclePage.qml"_L1}, QQmlComponent::Asynchronous};
	if (m_mesoComponent->status() != QQmlComponent::Ready)
	{
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
					createMesocyclePage_part2();
		}, Qt::SingleShotConnection);
	}
	else
		createMesocyclePage_part2();
}

void QMLMesoInterface::createMesocyclePage_part2()
{
	#ifndef QT_NO_DEBUG
	if (m_mesoComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_mesoComponent->errorString();
		for (uint i(0); i < m_mesoComponent->errors().count(); ++i)
			qDebug() << m_mesoComponent->errors().at(i).description();
		return;
	}
	#endif
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"_L1));

	connect(this, &QMLMesoInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QMLMesoInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(appMesoModel()->name(m_mesoIdx), m_mesoPage);

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
	connect(appMesoModel(), &DBMesocyclesModel::mesoCalendarFieldsChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx)
			QMetaObject::invokeMethod(m_mesoPage, "showCalendarChangedDialog");
	});
	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar &splitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			switch (splitLetter.cell())
			{
				case 'A': emit muscularGroupAChanged(); break;
				case 'B': emit muscularGroupBChanged(); break;
				case 'C': emit muscularGroupCChanged(); break;
				case 'D': emit muscularGroupDChanged(); break;
				case 'E': emit muscularGroupEChanged(); break;
				case 'F': emit muscularGroupFChanged(); break;
			}
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
		{
			if (!appMesoModel()->isNewMeso(m_mesoIdx))
			{
				appDBInterface()->saveMesocycle(m_mesoIdx);
				if (!ownMeso())
					appMesoModel()->checkIfCanExport(m_mesoIdx);
				else
					sendMesocycleFileToServer();
			}
		}
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
