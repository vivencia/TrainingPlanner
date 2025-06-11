#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "tputils.h"

using namespace Qt::Literals::StringLiterals;

homePageMesoModel::homePageMesoModel(QObject *parent)
	: QAbstractListModel{parent}
{
	m_roleNames[mesoNameRole] = std::move("mesoName");
	m_roleNames[mesoStartDateRole] = std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole] = std::move("mesoEndDate");
	m_roleNames[mesoSplitRole] = std::move("mesoSplit");
	m_roleNames[mesoCoachRole] = std::move("mesoCoach");
	m_roleNames[mesoClientRole] = std::move("mesoClient");
}

void homePageMesoModel::appendData(const QStringList& modeldata, const uint mesoModelRow)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_modeldata2.append(&modeldata);
	m_mesoModelRows.append(mesoModelRow);
	emit countChanged();
	endInsertRows();
}

void homePageMesoModel::removeRow(const uint row)
{
	if (row < m_modeldata2.count())
	{
		beginRemoveRows(QModelIndex{}, row, row);
		m_modeldata2.remove(row);
		m_mesoModelRows.remove(row);
		emit countChanged();
		endRemoveRows();
	}
}

QVariant homePageMesoModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if(row >= 0 && row < m_modeldata2.count())
	{
		const uint meso_idx{m_mesoModelRows.at(row)};
		switch(role)
		{
			case mesoNameRole:
				return QVariant{"<b>"_L1 + appMesoModel()->name(meso_idx) + (appMesoModel()->_id(meso_idx) < 0 ? tr(" (Temporary)") : QString{}) + "</b>"_L1};
			case mesoStartDateRole:
				return QVariant{appMesoModel()->startDateLabel() + "<b>"_L1 +
					(!appMesoModel()->isNewMeso(meso_idx) ? appUtils()->formatDate(appMesoModel()->startDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoEndDateRole:
				return QVariant{appMesoModel()->endDateLabel() + "<b>"_L1 +
						(!appMesoModel()->isNewMeso(meso_idx) ? appUtils()->formatDate(appMesoModel()->endDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoSplitRole:
				return QVariant{appMesoModel()->splitLabel() + "<b>"_L1 + appMesoModel()->split(meso_idx) + "</b>"_L1};
			case mesoCoachRole:
				if (!appMesoModel()->coach(meso_idx).isEmpty())
					return QVariant{appMesoModel()->coachLabel() + "<b>"_L1 +
											appUserModel()->userNameFromId(appMesoModel()->coach(meso_idx)) + "</b>"_L1};
			case mesoClientRole:
				if (!appMesoModel()->client(meso_idx).isEmpty())
					return QVariant{appMesoModel()->clientLabel() + "<b>"_L1 +
											appUserModel()->userNameFromId(appMesoModel()->client(meso_idx)) + "</b>"_L1};
		}
	}
	return QVariant{};
}
