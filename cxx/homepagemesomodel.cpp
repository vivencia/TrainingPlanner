#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "tpsettings.h"
#include "tputils.h"

#include <ranges>

constexpr QLatin1StringView ownMesosCurIndexSettingsName{"ownMesosCurIndex"};
constexpr QLatin1StringView clientMesosCurIndexSettingsName{"clientMesosCurIndex"};

enum MesoRoleNames {
	createRole(mesoName, MESO_FIELD_NAME)
	createRole(mesoStartDate, MESO_FIELD_STARTDATE)
	createRole(mesoEndDate, MESO_FIELD_ENDDATE)
	createRole(mesoSplit, MESO_FIELD_SPLIT)
	createRole(mesoCoach, MESO_FIELD_COACH)
	createRole(mesoClient, MESO_FIELD_CLIENT)
	createRole(mesoIdx, mesoClientRole + 1)
	createRole(mesoExportable, mesoIdxRole + 1)
	createRole(mesoSplitsAvailable, mesoExportableRole + 1)
	createRole(haveCalendar, mesoSplitsAvailableRole + 1)
};

using namespace QLiterals;

HomePageMesoModel::HomePageMesoModel(DBMesocyclesModel *meso_model, const bool own_mesos)
	: QAbstractListModel{meso_model}, m_mesoModel{meso_model}, m_ownMesos{own_mesos}, m_curIndex{-1}
{
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

	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, [this] (const uint meso_idx, const uint field)
	{
		if (m_mesoModel->isOwnMeso(meso_idx) == m_ownMesos)
		{
			const int row{findLocalIdx(meso_idx)};
			if (row >= 0)
			{
				switch (field)
				{
					case MESO_FIELD_NAME:
					case MESO_FIELD_STARTDATE:
					case MESO_FIELD_ENDDATE:
					case MESO_FIELD_SPLIT:
					case MESO_FIELD_COACH:
					case MESO_FIELD_CLIENT:
						emit canHaveTodaysWorkoutChanged();
						emit dataChanged(index(row, 0), index(row, 0), QList<int>{1, Qt::UserRole + static_cast<int>(field)});
					break;
					default:
						if (field >= MESO_FIELD_SPLITA && field <= MESO_FIELD_SPLITF)
							emit dataChanged(index(row, 0), index(row, 0), QList<int>{1, mesoSplitsAvailableRole});
				}
			}
		}
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
		return m_ownMesos && m_mesoModel->isDateWithinMeso(m_curIndex, QDate::currentDate());
	return false;
}

void HomePageMesoModel::setCurrentIndex(const int new_index)
{
	if (m_curIndex != new_index)
	{
		if (m_curIndex != -1)
			appSettings()->setCustomValue(m_ownMesos ? ownMesosCurIndexSettingsName : clientMesosCurIndexSettingsName, m_curIndex);
		m_curIndex = new_index;
		emit currentIndexChanged();
		emit canHaveTodaysWorkoutChanged();
	}
}

void HomePageMesoModel::appendMesoIdx(const uint meso_idx)
{
	if (!m_mesoModelRows.contains(meso_idx))
	{
		beginInsertRows(QModelIndex{}, count(), count());
		m_mesoModelRows.append(meso_idx);
		emit countChanged();
		setCurrentIndex(m_mesoModelRows.count() - 1);
		endInsertRows();
	}
	#ifndef QT_NO_DEBUG
	else
		qDebug() << "WARNING!!! Attempt to reinsert the same meso_index into a HomePageMesoModel!!";
	#endif
}

void HomePageMesoModel::removeMesoIdx(const uint meso_idx)
{
	int row{findLocalIdx(meso_idx)};
	if (row >= 0)
	{
		beginRemoveRows(QModelIndex{}, row, row);
		m_mesoModelRows.remove(row);
		if (row < m_mesoModelRows.count())
		{
			for (auto &mesoidx : m_mesoModelRows | std::views::drop(row))
				--mesoidx;
		}
		if (m_curIndex >= 0)
			setCurrentIndex(m_curIndex == 1 ? 0 : m_curIndex - 1);
		emit countChanged();
		endRemoveRows();
	}
}

QVariant HomePageMesoModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_mesoModelRows.count())
	{
		const uint meso_idx{m_mesoModelRows.at(row)};
		switch (role)
		{
			case mesoNameRole:
				return QVariant{"<b>"_L1 % (m_mesoModel->name(meso_idx).length() >= 5 ?
					m_mesoModel->name(meso_idx) : tr("Not set")) % (m_mesoModel->_id(meso_idx) < 0 ? tr(" (Temporary)") : QString{}) % "</b>"_L1};
			case mesoStartDateRole:
				return QVariant{m_mesoModel->startDateLabel() % "<b>"_L1 %
					(!m_mesoModel->strStartDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(m_mesoModel->startDate(meso_idx)) : tr("Not set")) % "</b>"_L1};
			case mesoEndDateRole:
				return QVariant{m_mesoModel->endDateLabel() % "<b>"_L1 %
					(!m_mesoModel->strEndDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(m_mesoModel->endDate(meso_idx)) : tr("Not set")) % "</b>"_L1};
			case mesoSplitRole:
				return QVariant{m_mesoModel->splitLabel() % "<b>"_L1 %
					(m_mesoModel->isSplitOK(meso_idx) ? m_mesoModel->split(meso_idx) : tr("Not set")) % "</b>"_L1};
			case mesoCoachRole:
				return QVariant{m_mesoModel->coachLabel() % "<b>"_L1 % (!m_mesoModel->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(m_mesoModel->coach(meso_idx)) : tr("Not set")) % "</b>"_L1};
			case mesoClientRole:
				return QVariant{m_mesoModel->clientLabel() % "<b>"_L1 % (!m_mesoModel->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(m_mesoModel->client(meso_idx)) : tr("Not set")) % "</b>"_L1};
			case mesoIdxRole:
				return meso_idx;
			case mesoExportableRole:
				return m_mesoModel->canExport(meso_idx);
			case mesoSplitsAvailableRole:
				return m_mesoModel->isSplitOK(meso_idx);				
			case haveCalendarRole:
				return m_mesoModel->isMesoOK(meso_idx);
		}
	}
	return QVariant{};
}
