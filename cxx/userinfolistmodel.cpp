#include "userinfolistmodel.h"

#include "usersmanager.h"
#include "tputils.h"

static const QString empty_string{};

enum RoleNames {
	createRole(id,				UsersManager::ID)
	createRole(insertTime,		UsersManager::INSERTTIME)
	createRole(onlineAccount,	UsersManager::ONLINEACCOUNT)
	createRole(name,			UsersManager::NAME)
	createRole(birthday,		UsersManager::BIRTHDAY)
	createRole(sex,				UsersManager::SEX)
	createRole(phone,			UsersManager::PHONE)
	createRole(email,			UsersManager::EMAIL)
	createRole(socialMedia,		UsersManager::SOCIALMEDIA)
	createRole(userRole,		UsersManager::USERROLE)
	createRole(coachrole,		UsersManager::COACHROLE)
	createRole(goal,			UsersManager::GOAL)
	createRole(category,		UsersManager::CATEGORY)
	createRole(avatar,			UsersManager::AVATAR)
};

UserInfoListModel::UserInfoListModel(QObject *parent) : TPListModel{parent, UsersManager::USER_N_FIELDS}
{
	roleToString(id)
	roleToString(insertTime)
	roleToString(onlineAccount)
	roleToString(name)
	roleToString(birthday)
	roleToString(sex)
	roleToString(phone)
	roleToString(email)
	roleToString(socialMedia)
	roleToString(userRole)
	roleToString(coachrole)
	roleToString(goal)
	roleToString(category)
	roleToString(avatar)

	setFilterField(UsersManager::CATEGORY);
	setFiltersManager(nullptr, true);
	m_modelData = &appUserModel()->m_usersData;
	connect(appUserModel(), &UsersManager::userModified, this, &UserInfoListModel::userModified);
}

void UserInfoListModel::setShowClients(const bool show)
{
	if (show != m_showClients) {
		m_showClients = show;
		if (show)
			filtersManager()->addValue(UsersManager::CATEGORY, UsersManager::UC_CLIENT);
		else
			filtersManager()->delValue(UsersManager::CATEGORY, UsersManager::UC_CLIENT);
		emit showClientsChanged();
	}
}

void UserInfoListModel::setShowCoaches(const bool show)
{
	if (show != m_showCoaches) {
		m_showCoaches = show;
		if (show)
			filtersManager()->addValue(UsersManager::CATEGORY, UsersManager::UC_COACH);
		else
			filtersManager()->delValue(UsersManager::CATEGORY, UsersManager::UC_COACH);
		emit showCoachesChanged();
	}
}

void UserInfoListModel::setShowConfirmed(const bool show)
{
	if (show != m_showConfirmed) {
		m_showConfirmed = show;
		if (show)
			filtersManager()->addValue(UsersManager::CATEGORY, UsersManager::UC_CONFIRMED);
		else
			filtersManager()->delValue(UsersManager::CATEGORY, UsersManager::UC_CONFIRMED);
		emit showConfirmedChanged();
	}
}

void UserInfoListModel::setShowAvailable(const bool show)
{
	if (show != m_showAvailable) {
		m_showAvailable = show;
		if (show)
			filtersManager()->addValue(UsersManager::CATEGORY, UsersManager::UC_YET_AVAILABLE);
		else
			filtersManager()->delValue(UsersManager::CATEGORY, UsersManager::UC_YET_AVAILABLE);
		emit showClientsChanged();
	}
}

const QString &UserInfoListModel::dataValue(const uint real_row, const uint column) const
{
	if (real_row < m_modelData->count() && column < m_modelData->at(real_row).count())
		return m_modelData->at(real_row).at(column);
	return empty_string;
}

QVariant UserInfoListModel::data(const uint role, const uint row, const int column) const
{
	return dataValue(row, role-Qt::UserRole);
}

bool UserInfoListModel::setData(const uint role, const uint row, const QVariant &value, const int column)
{
	if (row < m_modelData->count() && (role - Qt::UserRole) < m_modelData->at(row).count()) {
		(*m_modelData)[row][role - Qt::UserRole] = std::move(value.toString());
		return true;
	}
	return false;
}

QVariant UserInfoListModel::headerData(int section, Qt::Orientation orientation, int header_role) const
{
	if (header_role == Qt::DisplayRole) {
		if (orientation == Qt::Vertical) {
			return section;
		} else {
			switch (section) {
			case UsersManager::ID:				return appUserModel()->idLabel().section(':', 0, 0);
			case UsersManager::INSERTTIME:		return tr("Insert Time: ");
			case UsersManager::ONLINEACCOUNT:	return appUserModel()->onlineAccountUserLabel().section(':', 0, 0);
			case UsersManager::NAME:			return appUserModel()->nameLabel().section(':', 0, 0);
			case UsersManager::BIRTHDAY:		return appUserModel()->birthdayLabel().section(':', 0, 0);
			case UsersManager::SEX:				return appUserModel()->sexLabel().section(':', 0, 0);
			case UsersManager::PHONE:			return appUserModel()->phoneLabel().section(':', 0, 0);
			case UsersManager::EMAIL:			return appUserModel()->emailLabel().section(':', 0, 0);
			case UsersManager::SOCIALMEDIA:		return appUserModel()->socialMediaLabel().section(':', 0, 0);
			case UsersManager::USERROLE:		return appUserModel()->userRoleLabel().section(':', 0, 0);
			case UsersManager::COACHROLE:		return appUserModel()->coachRoleLabel().section(':', 0, 0);
			case UsersManager::GOAL:			return appUserModel()->goalLabel().section(':', 0, 0);
			case UsersManager::CATEGORY:		return appUserModel()->categoryLabel().section(':', 0, 0);
			}
		}
	}
	return QVariant{};
}

void UserInfoListModel::userModified(const uint user_idx, const uint field)
{
	switch (field) {
	case USER_MODIFIED_SWITCHING:
		emit layoutAboutToBeChanged();
		emit layoutChanged();
		break;
	case USER_MODIFIED_CREATED:
	case USER_MODIFIED_IMPORTED:
		beginInsertRows(QModelIndex{}, user_idx, user_idx);
		endInsertRows();
		break;
	case USER_MODIFIED_REMOVED:
		beginRemoveRows(QModelIndex{}, user_idx, user_idx);
		endRemoveRows();
		break;
	default:
		emit dataChanged(index(user_idx, field), index(user_idx, field));
	}
}

