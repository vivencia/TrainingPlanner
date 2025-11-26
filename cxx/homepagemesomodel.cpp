#include "homepagemesomodel.h"

#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "tputils.h"

using namespace Qt::Literals::StringLiterals;

HomePageMesoModel::HomePageMesoModel(DBMesocyclesModel *meso_model)
	: QAbstractListModel{meso_model}, m_mesoModel{meso_model}
{
	m_roleNames[mesoNameRole] = std::move("mesoName");
	m_roleNames[mesoStartDateRole] = std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole] = std::move("mesoEndDate");
	m_roleNames[mesoSplitRole] = std::move("mesoSplit");
	m_roleNames[mesoCoachRole] = std::move("mesoCoach");
	m_roleNames[mesoClientRole] = std::move("mesoClient");
}

#ifndef Q_OS_ANDROID
void HomePageMesoModel::userSwitchingActions()
{
	emit countChanged();
	emit dataChanged(index(0, 0), index(count() - 1, 0));
}
#endif

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
				return QVariant{"<b>"_L1 + (m_mesoModel->name(meso_idx).length() >= 5 ?
					m_mesoModel->name(meso_idx) : tr("Not set")) + (m_mesoModel->_id(meso_idx) < 0 ? tr(" (Temporary)") : QString{}) + "</b>"_L1};
			case mesoStartDateRole:
				return QVariant{m_mesoModel->startDateLabel() + "<b>"_L1 +
					(!m_mesoModel->strStartDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(m_mesoModel->startDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoEndDateRole:
				return QVariant{m_mesoModel->endDateLabel() + "<b>"_L1 +
					(!m_mesoModel->strEndDate(meso_idx).isEmpty() ?
						appUtils()->formatDate(m_mesoModel->endDate(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoSplitRole:
				return QVariant{m_mesoModel->splitLabel() + "<b>"_L1 +
					(m_mesoModel->isSplitOK(meso_idx) ? m_mesoModel->split(meso_idx) : tr("Not set")) + "</b>"_L1};
			case mesoCoachRole:
				return QVariant{m_mesoModel->coachLabel() + "<b>"_L1 + (!m_mesoModel->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(m_mesoModel->coach(meso_idx)) : tr("Not set")) + "</b>"_L1};
			case mesoClientRole:
				return QVariant{m_mesoModel->clientLabel() + "<b>"_L1 + (!m_mesoModel->coach(meso_idx).isEmpty() ?
					appUserModel()->userNameFromId(m_mesoModel->client(meso_idx)) : tr("Not set")) + "</b>"_L1};
		}
	}
	return QVariant{};
}
