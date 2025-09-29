#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "tputils.h"

using namespace Qt::Literals::StringLiterals;

HomePageMesoModel::HomePageMesoModel(QObject *parent)
	: QAbstractListModel{parent}
{
	m_roleNames[mesoNameRole] = std::move("mesoName");
	m_roleNames[mesoStartDateRole] = std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole] = std::move("mesoEndDate");
	m_roleNames[mesoSplitRole] = std::move("mesoSplit");
	m_roleNames[mesoCoachRole] = std::move("mesoCoach");
	m_roleNames[mesoClientRole] = std::move("mesoClient");
}

void HomePageMesoModel::userSwitchingActions()
{
	emit countChanged();
	emit dataChanged(index(0, 0), index(count() - 1, 0));
}

void HomePageMesoModel::appendData(const uint mesoModelRow)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_mesoModelRows.append(mesoModelRow);
	emit countChanged();
	endInsertRows();
}

void HomePageMesoModel::removeRow(const uint row)
{
	if (row < m_mesoModelRows.count())
	{
		beginRemoveRows(QModelIndex{}, row, row);
		m_mesoModelRows.remove(row);
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
				return QVariant{"<b>"_L1 + (appMesoModel()->name(meso_idx).length() >= 5 ?
					appMesoModel()->name(meso_idx) : tr("Not set")) + (appMesoModel()->_id(meso_idx) < 0 ? tr(" (Temporary)") : QString{}) + "</b>"_L1};
			case mesoStartDateRole:
				return QVariant{appMesoModel()->startDateLabel() + "<b>"_L1 +
					(!appMesoModel()->strStartDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(appMesoModel()->startDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoEndDateRole:
				return QVariant{appMesoModel()->endDateLabel() + "<b>"_L1 +
					(!appMesoModel()->strEndDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(appMesoModel()->endDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoSplitRole:
				return QVariant{appMesoModel()->splitLabel() + "<b>"_L1 +
					(appMesoModel()->isSplitOK(meso_idx) ? appMesoModel()->split(meso_idx) : tr("Not set")) + "</b>"_L1};
			case mesoCoachRole:
				return QVariant{appMesoModel()->coachLabel() + "<b>"_L1 + (!appMesoModel()->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(appMesoModel()->coach(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoClientRole:
				return QVariant{appMesoModel()->clientLabel() + "<b>"_L1 + (!appMesoModel()->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(appMesoModel()->client(meso_idx)) : tr("Not set")) + "</b>"_L1};
		}
	}
	return QVariant{};
}
