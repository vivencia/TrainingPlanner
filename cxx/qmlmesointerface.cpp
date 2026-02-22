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

void QMLMesoInterface::updateInterface()
{
	emit realMesoChanged();
	emit canExportChanged();
	emit typeChanged();
	emit displayFileNameChanged();
	emit fileNameChanged();
	emit startDateChanged();
	emit endDateChanged();
	emit minimumStartDateChanged();
	emit weeksChanged();
	emit splitChanged();
	emit notesChanged();
}

bool QMLMesoInterface::mesoNameOK() const
{
	return !m_mesoModel->isRequiredFieldWrong(m_mesoIdx, MESO_FIELD_NAME);
}

bool QMLMesoInterface::startDateOK() const
{
	return !m_mesoModel->isRequiredFieldWrong(m_mesoIdx, MESO_FIELD_STARTDATE);
}

bool QMLMesoInterface::endDateOK() const
{
	return !m_mesoModel->isRequiredFieldWrong(m_mesoIdx, MESO_FIELD_ENDDATE);
}

bool QMLMesoInterface::splitOK() const
{
	return !m_mesoModel->isRequiredFieldWrong(m_mesoIdx, MESO_FIELD_SPLIT);
}

void QMLMesoInterface::setMesoIdx(const uint new_value)
{
	m_mesoIdx = new_value;
	m_splitsPage->setMesoIdx(m_mesoIdx);
	m_calendarPage->setMesoIdx(m_mesoIdx);
	for (const auto workout_page : std::as_const(m_workoutPages))
		workout_page->setMesoIdx(m_mesoIdx);
}

bool QMLMesoInterface::realMeso() const
{
	return m_mesoModel->isRealMeso(m_mesoIdx);
}

void QMLMesoInterface::setRealMeso(const bool new_value)
{
	if (m_mesoModel->isRealMeso(m_mesoIdx) != new_value)
	{
		m_mesoModel->setIsRealMeso(m_mesoIdx, new_value);
		setEndDate(new_value ? m_mesoModel->endDate(m_mesoIdx) : maximumMesoEndDate());
		emit realMesoChanged();
	}
}

bool QMLMesoInterface::ownMeso() const
{
	std::optional<bool> own_meso{m_mesoModel->isOwnMeso(m_mesoIdx)};
	return own_meso.has_value() ? own_meso.value() : false;
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
		if (m_mesoModel->isMesoNameOK(m_mesoIdx, new_name))
		{
			m_mesoModel->removeMesoFile(m_mesoIdx); //remove a -possible- meso file with the previous name
			m_mesoModel->setName(m_mesoIdx, new_name);
			emit nameChanged();
			verifyMesoRequiredFieldsStatus();
			emit mesoNameOKChanged();
		}
		else
			m_nameError = new_name.length() < 5 ? std::move(tr("Error: name too short")): std::move(tr("Error: Name already in use."));
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

QDate QMLMesoInterface::startDate() const
{
	return m_mesoModel->startDate(m_mesoIdx);
}

void QMLMesoInterface::setStartDate(const QDate &new_startdate)
{
	if (new_startdate != m_mesoModel->startDate(m_mesoIdx))
	{
		if (m_mesoModel->isStartDateOK(m_mesoIdx, new_startdate))
		{
			m_strStartDate = appUtils()->formatDate(new_startdate);
			m_mesoModel->setStartDate(m_mesoIdx, new_startdate);
			m_mesoModel->setWeeks(m_mesoIdx,
							QString::number(appUtils()->calculateNumberOfWeeks(new_startdate, m_mesoModel->endDate(m_mesoIdx))));
			emit startDateChanged();
			emit weeksChanged();
			verifyMesoRequiredFieldsStatus();
			emit startDateOKChanged();
		}
	}
}

void QMLMesoInterface::setMinimumMesoStartDate(const QDate &new_value)
{
	m_minimumMesoStartDate = new_value;
	emit minimumStartDateChanged();
}

QDate QMLMesoInterface::endDate() const
{
	return m_mesoModel->endDate(m_mesoIdx);
}

void QMLMesoInterface::setEndDate(const QDate &new_enddate)
{
	if (new_enddate != m_mesoModel->endDate(m_mesoIdx))
	{
		if (m_mesoModel->isEndDateOK(m_mesoIdx, new_enddate))
		{
			m_strEndDate = appUtils()->formatDate(new_enddate);
			m_mesoModel->setEndDate(m_mesoIdx, new_enddate);
			m_mesoModel->setWeeks(m_mesoIdx,
						QString::number(appUtils()->calculateNumberOfWeeks(m_mesoModel->startDate(m_mesoIdx), new_enddate)));
			emit endDateChanged();
			emit weeksChanged();
			verifyMesoRequiredFieldsStatus();
			emit endDateOKChanged();
		}
	}
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
	if (new_split != m_mesoModel->split(m_mesoIdx)) {
		m_mesoModel->setSplit(m_mesoIdx, new_split);
		emit splitChanged();
		verifyMesoRequiredFieldsStatus();
		emit splitOKChanged();
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

QString QMLMesoInterface::muscularGroup(const QString &split) const
{
	return !split.isEmpty() ? m_mesoModel->muscularGroup(m_mesoIdx, split.at(0)) : QString{};
}

void QMLMesoInterface::setMuscularGroup(const QString &split, const QString &new_value)
{
	m_mesoModel->setMuscularGroup(m_mesoIdx, split.at(0), new_value);
}

void QMLMesoInterface::getCalendarPage()
{
	if (!m_calendarPage)
		m_calendarPage = new QmlMesoCalendarInterface{this, m_mesoModel, m_mesoModel->calendar(m_mesoIdx), m_mesoIdx};
	m_calendarPage->getMesoCalendarPage();
}

void QMLMesoInterface::getExercisesPlannerPage()
{
	if (!m_splitsPage) {
		m_mesoModel->loadSplits(m_mesoIdx);
		m_splitsPage = new QmlMesoSplitInterface{this, m_mesoModel, m_mesoIdx};
	}
	m_splitsPage->getExercisesPlannerPage();
}

void QMLMesoInterface::getWorkoutPage(const QDate &date)
{
	QmlWorkoutInterface *workoutPage(m_workoutPages.value(date));
	if (!workoutPage) {
		workoutPage = new QmlWorkoutInterface{this, m_mesoModel, m_mesoIdx, date};
		m_workoutPages.insert(date, workoutPage);
	}
	m_mesoModel->setWorkingWorkout(m_mesoIdx, m_mesoModel->workoutForDay(m_mesoIdx, date));
	workoutPage->getWorkoutPage();
}

void QMLMesoInterface::getMesocyclePage(const bool new_meso)
{
	if (!m_mesoComponent)
		createMesocyclePage(new_meso);
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

void QMLMesoInterface::createMesocyclePage(const bool new_meso)
{
	setMinimumMesoStartDate(m_mesoModel->getMesoMinimumStartDate(m_mesoModel->client(m_mesoIdx), m_mesoIdx));
	setMaximumMesoEndDate(m_mesoModel->getMesoMaximumEndDate(m_mesoModel->client(m_mesoIdx), m_mesoIdx));

	if (new_meso) {
		QString meso_name;
		uint i{1};
		do {
			meso_name = std::move(tr("New Program") + " %1"_L1.arg(QString::number(i++)));
		} while (!m_mesoModel->isMesoNameOK(m_mesoIdx, meso_name));
		setName(meso_name);
	}
	if (!startDateOK())
		setStartDate(appUtils()->getNextMonday(QDate::currentDate()));
	else
		m_strStartDate = appUtils()->formatDate(m_mesoModel->startDate(m_mesoIdx));
	if (!endDateOK())
		setEndDate(appUtils()->getNextSunday(m_mesoModel->startDate(m_mesoIdx).addDays(60)));
	else
		m_strEndDate = appUtils()->formatDate(m_mesoModel->endDate(m_mesoIdx));
	m_mesoProperties.insert("mesoManager"_L1, QVariant::fromValue(this));
	m_mesoProperties.insert("mesoModel"_L1, QVariant::fromValue(m_mesoModel));
	m_mesoComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesocyclePage.qml"_L1}, QQmlComponent::Asynchronous};

	switch (m_mesoComponent->status()) {
		case QQmlComponent::Ready:
			createMesocyclePage_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createMesocyclePage_part2();
			}, Qt::SingleShotConnection);
		break;
		#ifndef QT_NO_DEBUG
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_mesoComponent->errorString();
		break;
		#endif
	}
}

void QMLMesoInterface::createMesocyclePage_part2()
{
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (!m_mesoPage)
		qDebug() << m_mesoComponent->errorString();
	#endif

	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"_L1));
	verifyMesoRequiredFieldsStatus();

	appPagesListModel()->openPage(m_mesoPage, std::move(tr("Program: ") + name()),
												[this] () { m_mesoModel->removeMesoManager(m_mesoIdx); });

	connect(m_mesoModel, &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx) {
			m_mesoIdx = new_meso_idx;
			for (const auto workout_page : std::as_const(m_workoutPages))
				workout_page->setMesoIdx(m_mesoIdx);
			if (m_splitsPage)
				m_splitsPage->setMesoIdx(m_mesoIdx);
			if (m_calendarPage)
				m_calendarPage->setMesoIdx(m_mesoIdx);
		}
	});

	connect(m_mesoModel, &DBMesocyclesModel::canExportChanged, this, [this] (const uint meso_idx, const bool can_export) {
		if (meso_idx == m_mesoIdx)
			emit canExportChanged();
	});

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &QMLMesoInterface::labelsChanged);
	connect(this, &QMLMesoInterface::nameChanged, this, [this] () { appPagesListModel()->changeLabel(m_mesoPage, name()); });
}

void QMLMesoInterface::verifyMesoRequiredFieldsStatus()
{
	if (m_mesoPage) {
		int n_required_fields{0}, first_required_field{-1};
		for (uint i{0}; i < DBMesocyclesModel::MESO_N_REQUIRED_FIELDS; ++i) {
			if (m_mesoModel->isRequiredFieldWrong(m_mesoIdx, DBMesocyclesModel::meso_required_fields[i])) {
				if (first_required_field == -1)
					first_required_field = DBMesocyclesModel::meso_required_fields[i];
				n_required_fields++;
			}
		}
		QMetaObject::invokeMethod(m_mesoPage, "wrongFieldValueMessageHandler",
															Q_ARG(int, n_required_fields), Q_ARG(int, first_required_field));
	}
}

