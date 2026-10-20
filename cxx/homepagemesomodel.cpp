#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "usersmanager.h"
#include "qmlitemmanager.h"
#include "qmlmesointerface.h"
#include "tpsettings.h"
#include "tputils.h"

constexpr QLatin1StringView settings_fieldname[DBMesocyclesModel::MT_TYPE_COUNT] {
										"currentClientMeso"_L1 , "currentCoachMeso"_L1, "currentSelfMeso"_L1};

enum MesoRoleNames {
	createRole(mesoName, DBMesocyclesModel::MESO_FIELD_NAME)
	createRole(mesoStartDate, DBMesocyclesModel::MESO_FIELD_STARTDATE)
	createRole(mesoEndDate, DBMesocyclesModel::MESO_FIELD_ENDDATE)
	createRole(mesoSplit, DBMesocyclesModel::MESO_FIELD_SPLIT)
	createRole(mesoCoach, DBMesocyclesModel::MESO_FIELD_COACH)
	createRole(mesoClient, DBMesocyclesModel::MESO_FIELD_CLIENT)
	createRole(mesoIdx, mesoClientRole + 1)
	createRole(mesoExportable, mesoIdxRole + 1)
	createRole(mesoSplitsAvailable, mesoExportableRole + 1)
	createRole(haveCalendar, mesoSplitsAvailableRole + 1)
};

using namespace QLiterals;

HomePageMesoModel::HomePageMesoModel(DBMesocyclesModel *meso_model, const int type, const int view_index)
	: TPListModel{meso_model, DBMesocyclesModel::MESO_TOTAL_FIELDS},
													m_mesoModel{meso_model}, m_type{type}, m_viewIndex{view_index}
{
	setParentPage(appItemManager()->appHomePage());
	setFieldsNames();
	addFilters();
	m_removeField = DBMesocyclesModel::MESO_FIELD_NAME;
	roleToString(mesoName)
	roleToString(mesoStartDate)
	roleToString(mesoEndDate)
	roleToString(mesoSplit)
	roleToString(mesoCoach)
	roleToString(mesoClient)
	roleToString(mesoIdx)
	roleToString(mesoExportable)
	roleToString(mesoSplitsAvailable)
	roleToString(haveCalendar)

	connect(m_mesoModel, &DBMesocyclesModel::canExportChanged, this, [this] (const uint meso_idx, const bool can_export) {
		emit dataChanged(index(meso_idx), index(meso_idx), QList<int>{mesoExportableRole});
	});
	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const DBMesocyclesModel::MesoFields field) {
		if (m_type == m_mesoModel->mesoType(meso_idx)) {
			const int row{visibleRow(meso_idx)};
			if (row >= 0) {
				switch (field) {
				case DBMesocyclesModel::MESO_FIELD_NAME:
				case DBMesocyclesModel::MESO_FIELD_STARTDATE:
				case DBMesocyclesModel::MESO_FIELD_ENDDATE:
				case DBMesocyclesModel::MESO_FIELD_SPLIT:
				case DBMesocyclesModel::MESO_FIELD_COACH:
				case DBMesocyclesModel::MESO_FIELD_CLIENT:
					emit canHaveTodaysWorkoutChanged();
					emit dataChanged(index(row, 0), index(row, 0), QList<int>{Qt::UserRole + static_cast<int>(field)});
					break;
				default:
					if (field >= DBMesocyclesModel::MESO_FIELD_SPLITA && field <= DBMesocyclesModel::MESO_FIELD_SPLITF)
						emit dataChanged(index(row, 0), index(row, 0), QList<int>{mesoSplitsAvailableRole});
				}
			}
		}
	});
	connect(m_mesoModel, &DBMesocyclesModel::mesoDataLoaded, this, [this] () {
		syncMetadata(m_mesoModel->count(), true);
	});
	connect(this, &TPListModel::currentRowChanged, this, [this] () {
		if (currentRow() != -1)
			appSettings()->setCustomValue(settings_fieldname[m_type], currentRow());
		emit canHaveTodaysWorkoutChanged();
	});
}

#ifndef Q_OS_ANDROID
void HomePageMesoModel::userSwitchingActions()
{
	emit countChanged();
	emit dataChanged(index(0, 0), index(count() - 1, 0));
}
#endif

bool HomePageMesoModel::canHaveTodaysWorkout() const
{
	if (m_mesoModel->isMesoOK(currentMesoIdx()))
		return m_mesoModel->isDateWithinMeso(currentMesoIdx(), QDate::currentDate());
	return false;
}

QString HomePageMesoModel::viewTitle() const
{
	switch (m_type) {
	case DBMesocyclesModel::MT_MESO_FROM_COACH: return tr("My Coaches's Programs");
	case DBMesocyclesModel::MT_MESO_FOR_SELF: return tr("My Own Programs");
	case DBMesocyclesModel::MT_MESO_FOR_CLIENT: return tr("My Clients' Programs");
	default: Q_UNREACHABLE();
	}
}

QString HomePageMesoModel::backgroundColor() const {
	switch (m_type) {
	case DBMesocyclesModel::MT_MESO_FROM_COACH: return appSettings()->primaryLightColor();
	case DBMesocyclesModel::MT_MESO_FOR_SELF: return appSettings()->primaryColor();
	case DBMesocyclesModel::MT_MESO_FOR_CLIENT: return appSettings()->primaryDarkColor();
	default: Q_UNREACHABLE();
	}
}

void HomePageMesoModel::showOptionsMenu(QQuickItem *tpbutton, const int meso_idx)
{
	m_mesoModel->mesoManager(meso_idx)->showOptionsMenu(false, tpbutton);
}

void HomePageMesoModel::setFieldsNames()
{
	m_identifier = std::move(tr("Programs Overview"));
	m_fieldsNames.append(std::move(m_mesoModel->mesoNameLabel().chopped(2)));
	m_fieldsNames.append(std::move(m_mesoModel->startDateLabel().chopped(2)));
	m_fieldsNames.append(std::move(m_mesoModel->endDateLabel().chopped(2)));
	m_fieldsNames.append(std::move(m_mesoModel->coachLabel().chopped(2)));
	m_fieldsNames.append(std::move(m_mesoModel->clientLabel().chopped(2)));
	m_fieldsNames.append(std::move(m_mesoModel->splitLabel().chopped(2)));
}

const QString &HomePageMesoModel::dataValue(const uint real_row, const uint column) const
{
	if (real_row < m_mesoModel->count() && column < DBMesocyclesModel::MESO_TOTAL_FIELDS)
		return m_mesoModel->m_mesoData.at(real_row).at(column);
	else
		return m_dummy;
}

QVariant HomePageMesoModel::data(const uint role, const uint row, const int column) const
{
	switch (role) {
	case mesoNameRole:
		return QVariant{"<b>"_L1 % m_mesoModel->name(row) % "</b>"_L1};
	case mesoStartDateRole:
		return QVariant{m_mesoModel->startDateLabel() % "<b>"_L1 % (!m_mesoModel->strStartDate(row).isEmpty() ?
							appUtils()->formatDate(m_mesoModel->startDate(row)) : tr("Not set")) % "</b>"_L1};
	case mesoEndDateRole:
		return QVariant{m_mesoModel->endDateLabel() % "<b>"_L1 % (!m_mesoModel->strEndDate(row).isEmpty() ?
							appUtils()->formatDate(m_mesoModel->endDate(row)) : tr("Not set")) % "</b>"_L1};
	case mesoSplitRole:
		return QVariant{m_mesoModel->splitLabel() % "<b>"_L1 % (m_mesoModel->isSplitOK(row) ?
														m_mesoModel->split(row) : tr("Not set")) % "</b>"_L1};
	case mesoCoachRole:
		return QVariant{m_mesoModel->coachLabel() % "<b>"_L1 % (!m_mesoModel->coach(row).isEmpty() ?
						appUserModel()->userNameFromId(m_mesoModel->coach(row)) : tr("Not set")) % "</b>"_L1};
	case mesoClientRole:
		return QVariant{m_mesoModel->clientLabel() % "<b>"_L1 % (!m_mesoModel->coach(row).isEmpty() ?
						appUserModel()->userNameFromId(m_mesoModel->client(row)) : tr("Not set")) % "</b>"_L1};
	case mesoIdxRole:
		return row;
	case mesoExportableRole:
		return m_mesoModel->canExport(row);
	case mesoSplitsAvailableRole:
		return m_mesoModel->isSplitOK(row);
	case haveCalendarRole:
		return m_mesoModel->isMesoOK(row);
	}
	return QVariant{};
}

void HomePageMesoModel::addFilters()
{
	TPFilterModel *filter{new TPFilterModel{this}};
	filter->setDataAcquisitionFunc([this] (uint index, uint column) -> QPair<bool,QString> {
		if (index < m_mesoModel->count())
			return {true, dataValue(index, column)};
		else
			return {false, QString{}};
	});
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_NAME, [this] () -> QString {
													return m_mesoModel->mesoNameLabel().chopped(2); }, std::move(QString{}));
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_STARTDATE, [this] () -> QString {
													return m_mesoModel->startDateLabel().chopped(2); }, std::move(QString{}));
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_ENDDATE, [this] () -> QString {
													return m_mesoModel->endDateLabel().chopped(2); }, std::move(QString{}));
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_COACH, [this] () -> QString {
													return m_mesoModel->coachLabel().chopped(2); }, std::move(QString{}));
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_CLIENT, [this] () -> QString {
													return m_mesoModel->clientLabel().chopped(2); }, std::move(QString{}));
	filter->addFilterField(DBMesocyclesModel::MESO_FIELD_SPLIT, [this] () -> QString {
													return m_mesoModel->splitLabel().chopped(2); }, std::move(QString{}));
	setFiltersManager(filter);
}
