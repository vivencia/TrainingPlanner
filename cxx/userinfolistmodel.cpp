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

	setObjectName("UserInfoListModel");
	setFilterField(UsersManager::CATEGORY);
	setFiltersManager(nullptr, true);
	setPersistentFilterCompareFunction(true, UsersManager::CATEGORY, QString{}, [this] (bool positive, int row) -> bool {
		bool ret{false};
		if (row > 0) {
			if (m_showClients)
				ret = positive ? appUserModel()->isClient(row) : !appUserModel()->isClient(row);
			if (m_showCoaches)
				ret |= positive ? appUserModel()->isCoach(row) : !appUserModel()->isCoach(row);
			if (m_showConfirmed)
				ret |= positive ? appUserModel()->isConfirmed(row) : !appUserModel()->isConfirmed(row);
			if (m_showAvailable)
				ret |= positive ? appUserModel()->isAvailable(row) : !appUserModel()->isAvailable(row);
		}
		return ret;
	});
	m_modelData = &appUserModel()->m_usersData;
	syncMetadata(appUserModel()->userCount());
	setRemoveTitleFunction([this] (const QList<int>& remove_rows) -> QString {
		if (remove_rows.count() == 1) {
			const auto user_idx{remove_rows.constFirst()};
			if (appUserModel()->isConfirmed(user_idx))
				return QString{tr("Remove ") % appUserModel()->userName(user_idx) % '?'};
			else
				return QString{tr("Decline ") % appUserModel()->userName(user_idx) % '?'};
		} else {
			return tr("Proceed with the following actions?");
		}
	});

	setRemoveMessageFunction([this] (const QList<int>& remove_rows) -> QString {
		auto epithet = [] (const int user_idx, const bool plural) -> QString {
			return appUserModel()->isClient(user_idx) ? (!plural ? tr("client") : tr("client(s)"))
													  : (!plural ? tr("coach") : tr("coach(es)"));
		};

		if (remove_rows.count() == 1) {
			const auto user_idx{remove_rows.constFirst()};
			if (appUserModel()->isConfirmed(user_idx))
				return tr("The %1 will be notified of your decision, "
									"but might still contact you unless you block them").arg(epithet(user_idx, false));
			else
				return tr("The %1 will receive your reply, "
						"but might choose to send another request unless you block them").arg(epithet(user_idx, false));
		} else {
			QString message;
			bool has_decline{false}, has_remove{false};
			for (const auto &user_idx : remove_rows) {
				if (appUserModel()->isConfirmed(user_idx)) {
					message += QString{tr("Remove ") % appUserModel()->userName(user_idx) % "?<br>"_L1};
					has_remove = true;
				} else {
					message += QString{tr("Decline ") % appUserModel()->userName(user_idx) % "?<br>"_L1};
					has_decline = true;
				}
				if (has_remove) {
					message += tr("The removed %1 will be notified of your decision, "
									"but might still contact you unless you block them").arg(epithet(user_idx, true));
					if (has_decline)
						message += "<br>"_L1;
				}
				if (has_decline)
					message += tr("The declined client(s) will receive your reply, "
						"but might choose to send another request unless you block them").arg(epithet(user_idx, true));
			}
			return message;
		}
	});
	connect(appUserModel(), &UsersManager::userModified, this, &UserInfoListModel::userModified);
	connect(this, &TPListModel::currentRowChanged, this, &UserInfoListModel::currentUserChanged);
	connect(this, &TPListModel::selectedChanged, this, &UserInfoListModel::selectedUsersChanged);
}

QString UserInfoListModel::currentUser() const
{
	return appUserModel()->userId(realRow(currentRow()));
}

void UserInfoListModel::setCurrentUser(const QString &userid)
{
	setCurrentRowViaRealRow(appUserModel()->userIdxFromFieldValue(UsersManager::ID, userid));
}

QStringList UserInfoListModel::selectedUsers() const
{
	const QList<int> &indices{selectedInfo()};
	QStringList userids{indices.count()};
	for (const auto &useridx : indices)
		userids.append(std::move(appUserModel()->userId(useridx)));
	return userids;
}

void UserInfoListModel::setSelectedUsers(const QStringList &userids)
{
	for (const auto &userid : userids)
		setIsSelected(visibleRow(appUserModel()->userIdxFromFieldValue(UsersManager::ID, userid)), true);
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
		beginResetModel();
		endResetModel();
		syncMetadata(appUserModel()->userCount());
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
