#include "qmlmesointerface.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "qmlmesosplitinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlworkoutinterface.h"
#include "tputils.h"
#include "translationclass.h"

#include <QRegularExpression>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

constexpr uint16_t NEW_MESO_N_REQUIRED_FIELDS{4};
constexpr uint16_t new_meso_required_fields[NEW_MESO_N_REQUIRED_FIELDS]
					{MESOCYCLES_COL_NAME, MESOCYCLES_COL_STARTDATE, MESOCYCLES_COL_ENDDATE, MESOCYCLES_COL_SPLIT};

void QMLMesoInterface::cleanUp()
{
	if (m_mesoComponent)
	{
		delete m_mesoPage;
		delete m_mesoComponent;
	}
	if (m_splitsPage)
		delete m_splitsPage;
	if (m_calendarPage)
		delete m_calendarPage;

	qDeleteAll(m_workoutPages);
}

bool QMLMesoInterface::isMesoNameOK(const QString &meso_name) const
{
	if (meso_name.length() >= 5)
		return !m_mesoModel->mesoPlanExists(meso_name, m_mesoModel->coach(m_mesoIdx), m_mesoModel->client(m_mesoIdx));
	return false;
}

void QMLMesoInterface::setMesoNameOK(const bool nameok)
{
	m_mesoNameOK = nameok;
	emit mesoNameOKChanged();
}

void QMLMesoInterface::setStartDateOK(const bool dateok)
{
	m_startDateOK = dateok;
	emit startDateOKChanged();
}

void QMLMesoInterface::setEndDateOK(const bool dateok)
{
	m_endDateOK = dateok;
	emit endDateOKChanged();
}

bool QMLMesoInterface::realMeso() const
{
	return m_mesoModel->isRealMeso(m_mesoIdx);
}

void QMLMesoInterface::setRealMeso(const bool new_value)
{
	if (m_mesoModel->isRealMeso(m_mesoIdx) != new_value)
	{
		if (!m_mesoModel->isRealMeso(m_mesoIdx) && new_value)
		{
			if (isNewMeso())
				setNewMesoFieldCounter(newMesoFieldCounter() + 1);
		}
		m_mesoModel->setIsRealMeso(m_mesoIdx, new_value);
		setEndDate(new_value ? m_mesoModel->endDate(m_mesoIdx) : maximumMesoEndDate(), !isNewMeso());
		emit realMesoChanged();
	}
}

bool QMLMesoInterface::splitOK() const
{
	return m_mesoModel->isSplitOK(m_mesoIdx);
}

bool QMLMesoInterface::ownMeso() const
{
	std::optional<bool> own_meso{m_mesoModel->isOwnMeso(m_mesoIdx)};
	return own_meso.has_value() ? own_meso.value() : false;
}

bool QMLMesoInterface::isNewMeso() const
{
	return m_mesoModel->isNewMeso(m_mesoIdx);
}

bool QMLMesoInterface::isTempMeso() const
{
	return m_mesoModel->_id(m_mesoIdx) < 0;
}

bool QMLMesoInterface::canExport() const
{
	return m_mesoModel->canExport(m_mesoIdx);
}

bool QMLMesoInterface::coachIsMainUser() const
{
	return appUserModel()->userId(0) == m_mesoModel->coach(m_mesoIdx);
}

QString QMLMesoInterface::name() const
{
	return m_mesoModel->name(m_mesoIdx);
}

void QMLMesoInterface::setName(const QString &new_name)
{
	if (new_name != m_mesoModel->name(m_mesoIdx))
	{
		if (isMesoNameOK(new_name))
		{
			setMesoNameOK(true);
			m_name = new_name;
			emit nameChanged();
			m_mesoModel->removeMesoFile(m_mesoIdx); //remove a -possible- meso file with the previous name
			m_mesoModel->setName(m_mesoIdx, new_name);
			maybeChangeNewMesoFieldCounter();
		}
		else {
			m_nameError = new_name.length() < 5 ? std::move(tr("Error: name too short")): std::move(tr("Error: Name already in use."));
			setMesoNameOK(false);
		}
	}
}

QString QMLMesoInterface::coachName() const
{
	return appUserModel()->userNameFromId(m_mesoModel->coach(m_mesoIdx));
}

QString QMLMesoInterface::client() const
{
	return appUserModel()->userNameFromId(m_mesoModel->client(m_mesoIdx));
}

void QMLMesoInterface::setClient(const QString &new_value)
{
	if (m_mesoModel->client(m_mesoIdx) != new_value)
	{
		m_mesoModel->setClient(m_mesoIdx, new_value);
		emit clientChanged();
		setMinimumMesoStartDate(m_mesoModel->getMesoMinimumStartDate(new_value, m_mesoIdx));
		setStartDate(m_minimumMesoStartDate);
	}
}

QString QMLMesoInterface::type() const
{
	return m_mesoModel->type(m_mesoIdx);
}

void QMLMesoInterface::setType(const QString &new_value)
{
	if (m_mesoModel->type(m_mesoIdx) != new_value)
	{
		m_mesoModel->setType(m_mesoIdx, new_value);
		emit typeChanged();
	}
}

QString QMLMesoInterface::displayFileName() const
{
	return appUtils()->getFileName(m_mesoModel->file(m_mesoIdx));
}

QString QMLMesoInterface::fileName() const
{
	return m_mesoModel->file(m_mesoIdx);
}

void QMLMesoInterface::setFileName(const QString &new_filename)
{
	if (m_mesoModel->file(m_mesoIdx) != new_filename)
	{
		const QString &good_filepath{appUtils()->getCorrectPath(new_filename)};
		if (!good_filepath.isEmpty())
		{
			if (!appUtils()->canReadFile(good_filepath))
				return;
		}
		m_mesoModel->setFile(m_mesoIdx, good_filepath);
		emit fileNameChanged();
		emit displayFileNameChanged();
	}
}

void QMLMesoInterface::setStartDate(const QDate &new_startdate, const bool modify_new_meso_counter)
{
	if (m_startDate != new_startdate || m_mesoModel->isNewMeso(m_mesoIdx))
	{
		m_startDate = new_startdate;
		m_strStartDate = appUtils()->formatDate(m_startDate);
		emit startDateChanged();
		m_mesoModel->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
		emit weeksChanged();
		setStartDateOK(true);
		if (modify_new_meso_counter)
		{
			m_mesoModel->setStartDate(m_mesoIdx, m_startDate);
			maybeChangeNewMesoFieldCounter();
		}
	}
	else
		setStartDateOK(false);
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate &new_value)
{
	m_minimumMesoStartDate = new_value;
	emit minimumStartDateChanged();
}

void QMLMesoInterface::setEndDate(const QDate &new_enddate, const bool modify_new_meso_counter)
{
	if (m_endDate != new_enddate || m_mesoModel->isNewMeso(m_mesoIdx))
	{
		m_endDate = new_enddate;
		m_strEndDate = appUtils()->formatDate(m_endDate);
		emit endDateChanged();
		m_mesoModel->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_startDate, m_endDate)));
		emit weeksChanged();
		setEndDateOK(true);
		if (modify_new_meso_counter)
		{
			m_mesoModel->setEndDate(m_mesoIdx, m_endDate);
			maybeChangeNewMesoFieldCounter();
		}
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
	return m_mesoModel->nWeeks(m_mesoIdx);
}

QString QMLMesoInterface::split() const
{
	return m_mesoModel->split(m_mesoIdx);
}

void QMLMesoInterface::setSplit(const QString &new_split)
{
	if (m_mesoModel->split(m_mesoIdx) != new_split)
	{
		m_mesoModel->setSplit(m_mesoIdx, new_split);
		emit splitChanged();
		if (m_mesoModel->isSplitOK(new_split, m_mesoIdx))
		{
			maybeChangeNewMesoFieldCounter();	
			emit splitOKChanged();
		}
	}
}

QString QMLMesoInterface::notes() const
{
	return m_mesoModel->notes(m_mesoIdx);
}

void QMLMesoInterface::setNotes(const QString &new_value)
{
	m_mesoModel->setNotes(m_mesoIdx, new_value);
	emit notesChanged();
}

QString QMLMesoInterface::muscularGroupA() const
{
	return m_mesoModel->splitA(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupA(const QString &new_value)
{
	if (m_mesoModel->splitA(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitA(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupB() const
{
	return m_mesoModel->splitB(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupB(const QString &new_value)
{
	if (m_mesoModel->splitB(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitB(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupC() const
{
	return m_mesoModel->splitC(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupC(const QString &new_value)
{
	if (m_mesoModel->splitC(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitC(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupD() const
{
	return m_mesoModel->splitD(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupD(const QString &new_value)
{
	if (m_mesoModel->splitD(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitD(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupE() const
{
	return m_mesoModel->splitE(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupE(const QString &new_value)
{
	if (m_mesoModel->splitE(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitE(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupF() const
{
	return m_mesoModel->splitF(m_mesoIdx);
}

void QMLMesoInterface::setMuscularGroupF(const QString &new_value)
{
	if (m_mesoModel->splitF(m_mesoIdx) != new_value)
	{
		m_mesoModel->setSplitF(m_mesoIdx, new_value);
		emit splitOKChanged();
	}
}

QString QMLMesoInterface::muscularGroupR() const
{
	return m_mesoModel->splitR();
}

void QMLMesoInterface::getCalendarPage()
{
	if (!m_calendarPage)
		m_calendarPage = new QmlMesoCalendarInterface{this, m_mesoModel, m_mesoIdx};
	m_calendarPage->getMesoCalendarPage();
}

void QMLMesoInterface::getExercisesPlannerPage()
{
	if (!m_splitsPage)
		m_splitsPage = new QmlMesoSplitInterface{this, m_mesoModel, m_mesoIdx};
	m_splitsPage->getExercisesPlannerPage();
}

void QMLMesoInterface::getWorkoutPage(const QDate &date)
{
	QmlWorkoutInterface *workoutPage(m_workoutPages.value(date));
	if (!workoutPage)
	{
		workoutPage = new QmlWorkoutInterface{this, m_mesoModel, m_mesoIdx, date};
		m_workoutPages.insert(date, workoutPage);
	}
	workoutPage->getWorkoutPage();
}

void QMLMesoInterface::getMesocyclePage()
{
	if (!m_mesoComponent)
		createMesocyclePage();
	else
		appPagesListModel()->openPage(m_mesoPage);
}

void QMLMesoInterface::sendMesocycleFileToClient()
{
	m_mesoModel->sendMesoToUser(m_mesoIdx);
}

void QMLMesoInterface::incorporateMeso()
{
	m_mesoModel->incorporateMeso(m_mesoIdx);
}

void QMLMesoInterface::createMesocyclePage()
{
	if (!m_mesoModel->isNewMeso(m_mesoIdx))
	{
		m_mesoModel->loadSplits(m_mesoIdx);
		m_newMesoFieldCounter = -1;
		m_name = m_mesoModel->name(m_mesoIdx);
		setStartDate(m_mesoModel->startDate(m_mesoIdx), false);
		setEndDate(m_mesoModel->endDate(m_mesoIdx), false);
		setMinimumMesoStartDate(m_mesoModel->getMesoMinimumStartDate(m_mesoModel->client(m_mesoIdx), m_mesoIdx));
		setMaximumMesoEndDate(m_mesoModel->getMesoMaximumEndDate(m_mesoModel->client(m_mesoIdx), m_mesoIdx));
	}
	else
	{
		m_newMesoFieldCounter = 1;
		const QDate &minimumStartDate{m_mesoModel->getMesoMinimumStartDate(m_mesoModel->client(m_mesoIdx), 99999)};
		const QDate &minimumEndDate{appUtils()->createDate(QDate::currentDate(), 0, 2, 0)};
		setMinimumMesoStartDate(minimumStartDate);
		setMaximumMesoEndDate(appUtils()->createDate(QDate::currentDate(), 0, 6, 0));

		if (m_mesoModel->isNewMesoFieldSet(m_mesoIdx, MESOCYCLES_COL_NAME))
		{
			QString meso_name;
			uint i{1};
			do {
				meso_name = std::move(tr("New Program") + " %1"_L1.arg(QString::number(i++)));
			} while (!isMesoNameOK(meso_name));
			setName(meso_name);
			m_newMesoFieldCounter++;
		}

		if (m_mesoModel->isNewMesoFieldSet(m_mesoIdx, MESOCYCLES_COL_STARTDATE))
			m_newMesoFieldCounter++;
		setStartDate(appUtils()->getNextMonday(QDate::currentDate()), false);

		if (m_mesoModel->isNewMesoFieldSet(m_mesoIdx, MESOCYCLES_COL_ENDDATE))
			m_newMesoFieldCounter++;
		setEndDate(appUtils()->getNextSunday(minimumEndDate), false);

		if (m_mesoModel->isNewMesoFieldSet(m_mesoIdx, MESOCYCLES_COL_SPLIT))
			m_newMesoFieldCounter++;
	}

	m_mesoProperties.insert("mesoManager"_L1, QVariant::fromValue(this));
	m_mesoProperties.insert("mesoModel"_L1, QVariant::fromValue(m_mesoModel));
	m_mesoComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesocyclePage.qml"_L1}, QQmlComponent::Asynchronous};

	switch (m_mesoComponent->status())
	{
		case QQmlComponent::Ready:
			createMesocyclePage_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createMesocyclePage_part2();
			}, Qt::SingleShotConnection);
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_mesoComponent->errorString();
			#endif
		break;
	}
}

void QMLMesoInterface::createMesocyclePage_part2()
{
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"_L1));

	appPagesListModel()->openPage(m_mesoPage, std::move(tr("Program: ") + name()),
												[this] () { m_mesoModel->removeMesoManager(m_mesoIdx); });

	connect(m_mesoModel, &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
		{
			m_mesoIdx = new_meso_idx;
			for (const auto workout_page : std::as_const(m_workoutPages))
				workout_page->setMesoIdx(m_mesoIdx);
			if (m_splitsPage)
				m_splitsPage->setMesoIdx(m_mesoIdx);
			if (m_calendarPage)
				m_calendarPage->setMesoIdx(m_mesoIdx);
		}
	});

	if (m_mesoModel->isNewMeso(m_mesoIdx))
	{
		connect(m_mesoModel, &DBMesocyclesModel::isNewMesoChanged, this, [this] (const uint meso_idx) {
			if (meso_idx == m_mesoIdx)
				emit isNewMesoChanged();
		});
	}

	connect(m_mesoModel, &DBMesocyclesModel::canExportChanged, this, [this] (const uint meso_idx, const bool can_export) {
		if (meso_idx == m_mesoIdx)
			emit canExportChanged();
	});

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &QMLMesoInterface::labelsChanged);

	if (m_mesoModel->isNewMeso(m_mesoIdx))
		maybeChangeNewMesoFieldCounter();

	connect(this, &QMLMesoInterface::nameChanged, this, [this] () {
		appPagesListModel()->changeLabel(m_mesoPage, name());
	});
}

void QMLMesoInterface::maybeChangeNewMesoFieldCounter()
{
	if (--m_newMesoFieldCounter >= 0)
	{
		for (uint i{0}; i < NEW_MESO_N_REQUIRED_FIELDS; ++i)
		{
			if (m_mesoModel->isNewMesoFieldSet(m_mesoIdx, new_meso_required_fields[i]))
			{
				emit newMesoFieldCounterChanged(new_meso_required_fields[i]);
				return;
			}
		}
		emit newMesoFieldCounterChanged(0);
	}
}

