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
		const uint mesorow{m_mesoModelRows.at(row)};
		switch(role)
		{
			case mesoNameRole:
				return QVariant{"<b>"_L1 + (appMesoModel()->name(mesorow).isEmpty() ? tr("New Program") :
							appMesoModel()->name(mesorow)) + (appMesoModel()->_id(mesorow) < 0 ? tr(" (Temporary)") : QString{}) + "</b>"_L1};
			case mesoStartDateRole:
				return QVariant{appMesoModel()->columnLabel(MESOCYCLES_COL_STARTDATE) + "<b>"_L1 +
					(!appMesoModel()->isNewMeso(mesorow) ? appUtils()->formatDate(appMesoModel()->startDate(mesorow)) : tr("Not set")) + "</b>"_L1};
			case mesoEndDateRole:
				return QVariant{appMesoModel()->columnLabel(MESOCYCLES_COL_ENDDATE) + "<b>"_L1 +
						(!appMesoModel()->isNewMeso(mesorow) ? appUtils()->formatDate(appMesoModel()->endDate(mesorow)) : tr("Not set")) + "</b>"_L1};
			case mesoSplitRole:
				return QVariant{appMesoModel()->columnLabel(MESOCYCLES_COL_SPLIT) + "<b>"_L1 + appMesoModel()->split(mesorow) + "</b>"_L1};
			case mesoCoachRole:
				if (!appMesoModel()->coach(mesorow).isEmpty())
					return QVariant{appMesoModel()->columnLabel(MESOCYCLES_COL_COACH) + "<b>"_L1 +
											appUserModel()->userNameFromId(appMesoModel()->coach(mesorow)) + "</b>"_L1};
			case mesoClientRole:
				if (!appMesoModel()->client(mesorow).isEmpty())
					return QVariant{appMesoModel()->columnLabel(MESOCYCLES_COL_CLIENT) + "<b>"_L1 +
											appUserModel()->userNameFromId(appMesoModel()->client(mesorow)) + "</b>"_L1};
		}
	}
	return QVariant{};
}
