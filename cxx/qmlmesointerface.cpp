#include "qmlmesointerface.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "qmlitemmanager.h"
#include "qmlmesosplitinterface.h"
#include "qmlmesocalendarinterface.h"
#include "qmlworkoutinterface.h"
#include "tpfileops.h"
#include "tputils.h"
#include "translationclass.h"

#include <QRegularExpression>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QMLMesoInterface::cleanUp()
{
	if (m_mesoComponent) {
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
	emit startDateChanged();
	emit endDateChanged();
	emit minimumStartDateChanged();
	emit weeksChanged();
	emit splitChanged();
	emit notesChanged();
}

void QMLMesoInterface::checkMesoName(const QString &name)
{
	bool name_ok{name != m_mesoModel->name(m_mesoIdx)};
	if (name_ok)
		name_ok = m_mesoModel->checkName(m_mesoIdx, name);
	if (!name_ok)
		m_nameError = name.length() < 5 ? std::move(tr("Error: name too short")): std::move(tr("Error: Name already in use."));
}

bool QMLMesoInterface::mesoNameOK() const
{
	return m_mesoModel->isNameOK(m_mesoIdx);
}

bool QMLMesoInterface::startDateOK() const
{
	return !m_mesoModel->isStartDateOK(m_mesoIdx);
}

bool QMLMesoInterface::endDateOK() const
{
	return m_mesoModel->isEndDateOK(m_mesoIdx);
}

bool QMLMesoInterface::splitOK() const
{
	return m_mesoModel->isSplitOK(m_mesoIdx);
}

void QMLMesoInterface::setMesoIdx(const uint new_value)
{
	m_mesoIdx = new_value;
	emit mesoIdxChanged();
	m_splitsPage->setMesoIdx(m_mesoIdx);
	m_calendarPage->setMesoIdx(m_mesoIdx);
	for (const auto workout_page : std::as_const(m_workoutPages))
		workout_page->setMesoIdx(m_mesoIdx);
}

void QMLMesoInterface::increaseWrongFieldsCounter()
{
	if (m_wrongFieldsCounter < DBMesocyclesModel::MESO_WRONG_FIELDS_COUNT) {
		++m_wrongFieldsCounter;
		emit wrongFieldsCounterChanged();
		if (m_wrongFieldsCounter == 1)
			emit mesoOKChanged();
	}
}

bool QMLMesoInterface::realMeso() const
{
	return m_mesoModel->isRealMeso(m_mesoIdx);
}

void QMLMesoInterface::setRealMeso(const bool new_value)
{
	if (realMeso() != new_value) {
		if (new_value)
			m_mesoModel->setMetaData(m_mesoIdx, DBMesocyclesModel::MD_REAL_MESO);
		else
			m_mesoModel->unsetMetaData(m_mesoIdx, DBMesocyclesModel::MD_REAL_MESO);
		setEndDate(new_value ? m_mesoModel->endDate(m_mesoIdx) : maximumEndDate());
	}
}

bool QMLMesoInterface::ownMeso() const
{
	return m_mesoModel->isOwnMeso(m_mesoIdx);
}

bool QMLMesoInterface::canExport() const
{
	return m_mesoModel->canExport(m_mesoIdx);
}

bool QMLMesoInterface::canSendToClient() const
{
	return mesoForClient() && !m_mesoModel->isProgramSent(m_mesoIdx);
}

bool QMLMesoInterface::mesoForClient() const
{
	return m_mesoModel->type(m_mesoIdx).toUInt() == DBMesocyclesModel::MT_MESO_FOR_CLIENT;
}

bool QMLMesoInterface::mesoOK() const
{
	return m_mesoModel->isMesoOK(m_mesoIdx);
}

QString QMLMesoInterface::name() const
{
	return m_mesoModel->name(m_mesoIdx);
}

void QMLMesoInterface::setName(const QString &new_name)
{
	m_mesoModel->setName(m_mesoIdx, new_name);
	emit nameChanged();
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
	if (m_mesoModel->client(m_mesoIdx) != new_value) {
		m_mesoModel->setClient(m_mesoIdx, new_value);
		emit clientChanged();
		setMinimumStartDate(m_mesoModel->getMesoMinimumStartDate(new_value, m_mesoIdx));
		setStartDate(m_minimumStartDate);
	}
}

QString QMLMesoInterface::type() const
{
	return m_mesoModel->type(m_mesoIdx);
}

void QMLMesoInterface::setType(const QString &new_value)
{
	if (m_mesoModel->type(m_mesoIdx) != new_value) {
		m_mesoModel->setType(m_mesoIdx, new_value);
		emit typeChanged();
	}
}

QDate QMLMesoInterface::startDate() const
{
	return m_mesoModel->startDate(m_mesoIdx);
}

void QMLMesoInterface::setStartDate(const QDate &new_startdate)
{
	if (new_startdate != m_mesoModel->startDate(m_mesoIdx)) {
		if (m_mesoModel->checkStartDate(m_mesoIdx, new_startdate)) {
			m_strStartDate = appUtils()->formatDate(new_startdate);
			m_mesoModel->setStartDate(m_mesoIdx, new_startdate);
			if (realMeso()) {
				QDate end_date{std::move(endDate())};
				const auto meso_days{appUtils()->calculateNumberOfDays(end_date, new_startdate)};
				if (meso_days < DBMesocyclesModel::MESO_MINIMUM_DAYS) {
					setEndDate(new_startdate.addDays(DBMesocyclesModel::MESO_MINIMUM_DAYS));
					m_mesoModel->setWeeks(m_mesoIdx, "4"_L1);
					emit minimumEndDateChanged();
				}
				else if (meso_days > DBMesocyclesModel::MESO_MAXIMUM_DAYS) {
					setEndDate(new_startdate.addDays(DBMesocyclesModel::MESO_MAXIMUM_DAYS));
					m_mesoModel->setWeeks(m_mesoIdx, "26"_L1);
					emit maximumEndDateChanged();
				}
				else
					m_mesoModel->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(new_startdate,
																						m_mesoModel->endDate(m_mesoIdx))));
				emit weeksChanged();
			}
			emit startDateChanged();
		}
	}
}

void QMLMesoInterface::setMinimumStartDate(const QDate &new_value)
{
	m_minimumStartDate = new_value;
	emit minimumStartDateChanged();
}

QDate QMLMesoInterface::endDate() const
{
	return m_mesoModel->endDate(m_mesoIdx);
}

void QMLMesoInterface::setEndDate(const QDate &new_enddate)
{
	if (new_enddate != m_mesoModel->endDate(m_mesoIdx)) {
		if (m_mesoModel->checkEndDate(m_mesoIdx, new_enddate)) {
			m_strEndDate = appUtils()->formatDate(new_enddate);
			m_mesoModel->setEndDate(m_mesoIdx, new_enddate);
			m_mesoModel->setWeeks(m_mesoIdx, QString::number(appUtils()->calculateNumberOfWeeks(m_mesoModel->startDate(m_mesoIdx), new_enddate)));
			emit endDateChanged();
			emit weeksChanged();
		}
	}
}

QDate QMLMesoInterface::minimumEndDate() const
{
	return startDate().addDays(DBMesocyclesModel::MESO_MINIMUM_DAYS);
}

QDate QMLMesoInterface::maximumEndDate() const
{
	return startDate().addDays(DBMesocyclesModel::MESO_MAXIMUM_DAYS);
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

void QMLMesoInterface::sendMesocycleFileToClient()
{
	if (m_mesoModel->isProgramSent(m_mesoIdx))
		return;
	if (!m_mesoFileOps)
		createFileOps();
	m_mesoFileOps->attemptToCreateOrGetFile();
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
	if (!m_mesoComponent) {
		createFileOps();
		setMinimumStartDate(m_mesoModel->getMesoMinimumStartDate(m_mesoModel->client(m_mesoIdx), m_mesoIdx));

		if (new_meso) {
			QString meso_name;
			uint i{1};
			do {
				meso_name = std::move(tr("New Program") + " %1"_L1.arg(QString::number(i++)));
			} while (!m_mesoModel->checkName(m_mesoIdx, meso_name));
			setName(meso_name);
			setStartDate(appUtils()->getNextMonday(QDate::currentDate()));
			setEndDate(appUtils()->getNextSunday(m_mesoModel->startDate(m_mesoIdx).addDays(60)));
		}
		m_strStartDate = std::move(appUtils()->formatDate(m_mesoModel->startDate(m_mesoIdx)));
		m_strEndDate = std::move(appUtils()->formatDate(m_mesoModel->endDate(m_mesoIdx)));
		m_mesoProperties["mesoManager"_L1] = std::move(QVariant::fromValue(this));
		m_mesoProperties["mesoModel"_L1] = std::move(QVariant::fromValue(m_mesoModel));
		m_mesoComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages", "MesocyclePage", QQmlComponent::Asynchronous};
		connect(m_mesoComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getMesocyclePage(false); });
	} else {
		if (!m_mesoPage) {
			switch (m_mesoComponent->status()) {
			case QQmlComponent::Ready:
				m_mesoComponent->disconnect();
				createMesocyclePage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_mesoComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		} else {
			appPagesListModel()->openPage(m_mesoPage);
			showOptionsMenu(true);
		}
	}
}

void QMLMesoInterface::showOptionsMenu(const bool show_indicator, QQuickItem *item)
{
	if (!m_optionsMenuComponent) {
		m_optionsMenuProperties["parentPage"_L1] = std::move(QVariant::fromValue(item ? appItemManager()->appHomePage() : m_mesoPage));
		m_optionsMenuProperties["mesoManager"_L1] = std::move(QVariant::fromValue(this));
		m_optionsMenuComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages", "MesoOptionsMenu", QQmlComponent::Asynchronous};
		connect(m_optionsMenuComponent, &QQmlComponent::statusChanged, this, [this,show_indicator,item]
											(QQmlComponent::Status status) { showOptionsMenu(show_indicator, item); });
	} else {
		if (!m_optionsMenu) {
			switch (m_optionsMenuComponent->status()) {
			case QQmlComponent::Ready:
				m_optionsMenuComponent->disconnect();
				m_optionsMenu = m_optionsMenuComponent->createWithInitialProperties(m_optionsMenuProperties, appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_optionsMenu) {
					qDebug() << m_optionsMenuComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_optionsMenu, QQmlEngine::CppOwnership);
				m_optionsMenu->setProperty("parent", std::move(QVariant::fromValue(item ? appItemManager()->appHomePage() : m_mesoPage)));
				showOptionsMenu(show_indicator, item);
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_optionsMenuComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		} else {
			//change visibility before changing the showIndicator property because the visibility is bound its value in TPPageMenu.entriesList
			QMetaObject::invokeMethod(m_optionsMenu, "changeEntryVisibilityById", Q_ARG(int, OPTION_EXERCISES_PLANNER), Q_ARG(bool, show_indicator));
			m_optionsMenu->setProperty("showIndicator", std::move(QVariant{show_indicator}));
			if (show_indicator)
				m_optionsMenu->setProperty("behaviour_enabled", std::move(QVariant{false}));
			if (m_mesoFileOps)
				m_mesoFileOps->setParentPage(item ? appItemManager()->appHomePage() : m_mesoPage);
			appPagesListModel()->openPopup(m_optionsMenu, item ? appItemManager()->appHomePage() : m_mesoPage, Qt::AlignBaseline, item);
		}
	}
}

void QMLMesoInterface::createFileOps()
{
	if (m_mesoFileOps)
		return;
	m_mesoFileOps = new TPFileOps{};
	m_mesoFileOps->setParentPage(m_mesoPage ? m_mesoPage : appItemManager()->appHomePage());
	m_mesoFileOps->setMesoIdx(m_mesoIdx);
	m_mesoFileOps->setFileName(std::move(*m_mesoModel->suggestedName(m_mesoIdx)));
	m_mesoFileOps->setCanDownloadOrGenerate(true);
	m_mesoFileOps->setFileType(TPUtils::FT_TP_PROGRAM);
	if (mesoForClient()) {
		connect(m_mesoFileOps, &TPFileOps::fileAcquired, this, [this] (const int ret_code) mutable {
			if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS) {
				connect(m_mesoFileOps, &TPFileOps::fileSent, this, [this] (const bool success) {
					//if (success)
					//	m_mesoModel->setMetaData(m_mesoIdx, DBMesocyclesModel::MD_PROGRAM_SENT);
				}, Qt::SingleShotConnection);
				m_mesoFileOps->sendFileTo(TPUtils::MH_TPMESSAGES_MANAGER, QStringList{} <<
											  m_mesoFileOps->tpFileName().targetUser(), tr("Exercises Program"), true);
			}
		});
	}

	m_instructionsFileOps = new TPFileOps{};
	m_instructionsFileOps->setUseControls(true);
	m_instructionsFileOps->setFileName(m_mesoModel->file(m_mesoIdx));
	m_instructionsFileOps->setCanAddFile(ownMeso() || mesoForClient());
	m_instructionsFileOps->setCanDownloadOrGenerate(!ownMeso() && !mesoForClient());
	m_instructionsFileOps->setAddFileFilters(TPUtils::FT_DOCUMENTS);
	if (m_instructionsFileOps->canAddFile()) {
		m_instructionsFileOps->setSuggestedFileNameGenerator([this] (const QString &) -> TPFilePathPtr {
			return m_mesoModel->suggestedName(m_mesoIdx, true);
		});
		connect(m_instructionsFileOps, &TPFileOps::fileAdded, this, [this] (const QString &filepath) {
			m_mesoModel->setFile(m_mesoIdx, filepath);
		});
	}
	connect(m_instructionsFileOps, &TPFileOps::fileRemovalRequested, this, [this] () {
		m_mesoModel->setFile(m_mesoIdx, QString{});
	});

	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const DBMesocyclesModel::MesoFields field) {
		if (meso_idx == m_mesoIdx && field == DBMesocyclesModel::MESO_FIELD_NAME) {
			m_instructionsFileOps->renameFile(m_mesoModel->name(m_mesoIdx));
			m_mesoModel->setFile(m_mesoIdx, m_instructionsFileOps->fileName());
			m_mesoFileOps->setFileName(std::move(*m_mesoModel->suggestedName(m_mesoIdx)));
		}
	});
}

void QMLMesoInterface::createMesocyclePage()
{
	m_mesoPage = static_cast<QQuickItem*>(m_mesoComponent->createWithInitialProperties(m_mesoProperties, appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_mesoPage) {
		qDebug() << m_mesoComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(m_mesoPage, QQmlEngine::CppOwnership);
	m_mesoPage->setParentItem(appItemManager()->appPagesVisualParent());
	appPagesListModel()->openPage(m_mesoPage, std::move(tr("Program: ") % name()), [this] () {
		m_mesoModel->removeMesoManager(m_mesoIdx);
	});
	m_instructionsFileOps->setParentPage(m_mesoPage);
	showOptionsMenu(true);

	connect(m_mesoModel, &DBMesocyclesModel::metaDataChanged, this, [this] (const uint meso_idx, const DBMesocyclesModel::MetaData md_field) {
		if (meso_idx == m_mesoIdx) {
			emit mesoOKChanged();
			switch (md_field) {
			case DBMesocyclesModel::MD_REAL_MESO: emit realMesoChanged(); break;
			case DBMesocyclesModel::MD_PROGRAM_SENT: emit canSendToClientChanged(); break;
			case DBMesocyclesModel::MD_CAN_EXPORT: emit canExportChanged(); break;
			case DBMesocyclesModel::MD_NAME_OK:
				if (mesoNameOK())
					decreaseWrongFieldsCounter();
				else
					increaseWrongFieldsCounter();
				emit mesoNameOKChanged();
				break;
			case DBMesocyclesModel::MD_STARTDATE_OK:
				if (startDateOK())
					decreaseWrongFieldsCounter();
				else
					increaseWrongFieldsCounter();
				emit startDateOKChanged();
				break;
			case DBMesocyclesModel::MD_ENDDATE_OK:
				if (endDateOK())
					decreaseWrongFieldsCounter();
				else
					increaseWrongFieldsCounter();
				emit endDateOKChanged();
				break;
			case DBMesocyclesModel::MD_SPLIT_OK:
				if (splitOK())
					decreaseWrongFieldsCounter();
				else
					increaseWrongFieldsCounter();
				emit splitOKChanged();
				break;
			default: Q_UNREACHABLE();
			}
		}
	});

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

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &QMLMesoInterface::labelsChanged);
	connect(this, &QMLMesoInterface::nameChanged, this, [this] () { appPagesListModel()->changeLabel(m_mesoPage, name()); });
}

void QMLMesoInterface::createOptionsMenu()
{
	QMetaObject::invokeMethod(m_optionsMenu, "setVisible", Q_ARG(int, OPTION_EXERCISES_PLANNER),
												Q_ARG(bool, m_optionsMenuProperties.value("showIndicator").toBool()));
	showOptionsMenu(true);
}
