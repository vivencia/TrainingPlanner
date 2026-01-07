#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "tpsettings.h"
#include "tputils.h"

#include <ranges>

constexpr QLatin1StringView ownMesosCurIndexSettingsName{"ownMesosCurIndex"};
constexpr QLatin1StringView clientMesosCurIndexSettingsName{"clientMesosCurIndex"};

enum MesoRoleNames {
	mesoNameRole		=	Qt::UserRole + static_cast<int>(MESO_FIELD_NAME),
	mesoStartDateRole	=	Qt::UserRole + static_cast<int>(MESO_FIELD_STARTDATE),
	mesoEndDateRole		=	Qt::UserRole + static_cast<int>(MESO_FIELD_ENDDATE),
	mesoSplitRole		=	Qt::UserRole + static_cast<int>(MESO_FIELD_SPLIT),
	mesoCoachRole		=	Qt::UserRole + static_cast<int>(MESO_FIELD_COACH),
	mesoClientRole		=	Qt::UserRole + static_cast<int>(MESO_FIELD_CLIENT),
	mesoIdxRole			=	mesoClientRole + 1,
	mesoExportableRole	=	mesoIdxRole + 1,
	mesoSplitsRole		=	mesoExportableRole + 1,
	canHaveWorkoutRole	=	mesoSplitsRole + 1,
	haveCalendarRole	=	canHaveWorkoutRole + 1,
};

using namespace Qt::Literals::StringLiterals;

HomePageMesoModel::HomePageMesoModel(DBMesocyclesModel *meso_model, const bool own_mesos)
	: QAbstractListModel{meso_model}, m_mesoModel{meso_model}, m_ownMesos{own_mesos}
{
	m_roleNames[mesoNameRole]		= std::move("mesoName");
	m_roleNames[mesoStartDateRole]	= std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole]	= std::move("mesoEndDate");
	m_roleNames[mesoSplitRole]		= std::move("mesoSplit");
	m_roleNames[mesoCoachRole]		= std::move("mesoCoach");
	m_roleNames[mesoClientRole]		= std::move("mesoClient");
	m_roleNames[mesoIdxRole]		= std::move("mesoIdx");
	m_roleNames[mesoExportableRole]	= std::move("mesoExportable");
	m_roleNames[mesoSplitsRole]		= std::move("mesoSplitsAvailable");
	m_roleNames[canHaveWorkoutRole]	= std::move("canHaveWorkout");
	m_roleNames[haveCalendarRole]	= std::move("haveCalendar");
	m_curIndex = appSettings()->getCustomValue(m_ownMesos ? ownMesosCurIndexSettingsName : clientMesosCurIndexSettingsName, -1).toInt();

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
						emit dataChanged(index(row, 0), index(row, 0), QList<int>{1, Qt::UserRole + static_cast<int>(field)});
					break;
					default:
						if (field >= MESO_FIELD_SPLITA && field <= MESO_FIELD_SPLITF)
							emit dataChanged(index(row, 0), index(row, 0), QList<int>{1, mesoSplitsRole});
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
	return data(index(m_curIndex), canHaveWorkoutRole).toBool();
}

void HomePageMesoModel::setCurrentIndex(const int new_index)
{
	if (m_curIndex != new_index)
	{
		m_curIndex = new_index;
		appSettings()->setCustomValue(m_ownMesos ?
									ownMesosCurIndexSettingsName : clientMesosCurIndexSettingsName, m_curIndex);

		emit currentIndexChanged();
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
			case mesoSplitsRole:
				return m_mesoModel->isSplitOK(meso_idx);
			case canHaveWorkoutRole:
				return m_ownMesos && m_mesoModel->isDateWithinMeso(meso_idx, QDate::currentDate());
			case haveCalendarRole:
				return m_mesoModel->isMesoOK(meso_idx);
		}
	}
	return QVariant{};
}
