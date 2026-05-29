#include "userinfolistmodel.h"

#include "dbusermodel.h"
#include "tputils.h"

constexpr uint totalExtraFields{4};

enum extraFields {
	EF_AVATAR = DBUserModel::USER_FIELD_AVATAR,
	EF_USERIDX,
	EF_SELECTED,
	EF_VISIBLE,
	EF_ISCOACH,
	EF_ISCLIENT,
	EF_ISCONFIRMED,
	EF_ISAVAILABLE,
	EF_N_FIELDS = 8
};

enum RoleNames {
	createRole(id,				DBUserModel::USER_FIELD_ID)
	createRole(insertTime,		DBUserModel::USER_FIELD_INSERTTIME)
	createRole(onlineAccount,	DBUserModel::USER_FIELD_ONLINEACCOUNT)
	createRole(name,			DBUserModel::USER_FIELD_NAME)
	createRole(birthday,		DBUserModel::USER_FIELD_BIRTHDAY)
	createRole(sex,				DBUserModel::USER_FIELD_SEX)
	createRole(phone,			DBUserModel::USER_FIELD_PHONE)
	createRole(email,			DBUserModel::USER_FIELD_EMAIL)
	createRole(socialMedia,		DBUserModel::USER_FIELD_SOCIALMEDIA)
	createRole(userRole,		DBUserModel::USER_FIELD_USERROLE)
	createRole(coachrole,		DBUserModel::USER_FIELD_COACHROLE)
	createRole(goal,			DBUserModel::USER_FIELD_GOAL)
	createRole(category,		DBUserModel::USER_FIELD_USER_CATEGORY)
	createRole(avatar,			EF_AVATAR)
	createRole(userIdx,			EF_USERIDX)
	createRole(selected,		EF_SELECTED)
	createRole(itemVisible,		EF_VISIBLE)
	createRole(isCoach,			EF_ISCOACH)
	createRole(isClient,		EF_ISCLIENT)
	createRole(isConfirmed,		EF_ISCONFIRMED)
	createRole(isAvailable,		EF_ISAVAILABLE)
};

UserInfoListModel::UserInfoListModel(QObject *parent) : QAbstractListModel{parent}, m_totalCols{DBUserModel::USER_N_FIELS}
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
	roleToString(userIdx)
	roleToString(selected)
	roleToString(itemVisible)
	roleToString(isCoach)
	roleToString(isClient)
	roleToString(isConfirmed)
	roleToString(isAvailable)

	gatherAllUsersInfo();
	connect(appUserModel(), &DBUserModel::userModified, this, &UserInfoListModel::userModified);
}

int UserInfoListModel::userIdx(const uint row) const
{
	return m_extraInfo.at(rowFromVisibleRow(row)).at(EF_USERIDX).toInt();
}

bool UserInfoListModel::isSelected(const uint row, const int column) const
{
	Q_ASSERT_X(row < count(), "UserInfoListModel::setSelected", "row out of range");
	return appUtils()->getCompositeValue(column, m_extraInfo.at(rowFromVisibleRow(row)).at(EF_SELECTED).toString(), fancy_record_separator1) == '1';
}

void UserInfoListModel::setSelected(const uint row, const bool selected, const int column)
{
	if (row < count()) {
		const bool item_already_selected{isSelected(row, column)};
		QString str_selected{std::move(m_extraInfo.at(row).at(EF_SELECTED).toString())};
		if (m_selectEntireRow) {
			for (uint i {0}; i < m_totalCols; ++i)
				appUtils()->setCompositeValue(i, selected ? "1"_L1 : "0"_L1, str_selected, fancy_record_separator1);
			m_extraInfo[rowFromVisibleRow(row)][EF_SELECTED] = std::move(QVariant{str_selected});
			emit dataChanged(index(row, 0), index(row, m_totalCols), QList<int>{selectedRole});
		}
		else {
			appUtils()->setCompositeValue(column, selected ? "1"_L1 : "0"_L1, str_selected, fancy_record_separator1);
			m_extraInfo[rowFromVisibleRow(row)][EF_SELECTED] = std::move(QVariant{str_selected});
			emit dataChanged(index(row, column), index(row, column), QList<int>{selectedRole});
		}
		if (item_already_selected && !selected)
			--m_nSelected;
		else if (!item_already_selected && selected)
			++m_nSelected;
		emit selectedChanged();
	}
}

QStringList UserInfoListModel::selectedUsers() const
{
	QStringList selected;
	for(int i{0}; i < m_extraInfo.count(); ++i) {
		if (rowVisible(i)) {
			if (appUtils()->getCompositeValue(0, m_extraInfo.at(i).at(EF_SELECTED).toString(), fancy_record_separator1) == '1')
				selected.append(data(DBUserModel::USER_FIELD_ID, i));
		}
	}
	return selected;
}

void UserInfoListModel::applyFilter(const QString &filter, int field)
{
	if (filter != m_filter || field != m_fieldFilter) {
		if (field >= -1 && field < DBUserModel::USER_N_FIELS) {
			m_filter = filter;
			m_fieldFilter = field;
			changeVisibilityAsPerCategory();
		}
	}
}

#ifndef Q_OS_ANDROID
bool UserInfoListModel::dataFromString(const QString &users_data)
{
	QStringList tempmodeldata{std::move(users_data.split('\n'))};
	if (tempmodeldata.count() < DBUserModel::USER_N_FIELS)
		return false;
	if (tempmodeldata.count() > DBUserModel::USER_N_FIELS)
		tempmodeldata.removeLast(); //remove the password field
	m_allUsersData.append(std::move(tempmodeldata));
	return true;
}
#endif

QVariant UserInfoListModel::data(const QModelIndex &index, int role) const
{
	const int row{rowFromVisibleRow(index.row())};
	if (row >= 0 && row < count()) {
		const int user_idx{userIdx(row)};
		if (user_idx >= 0) {
#ifndef Q_OS_ANDROID
			if (m_allUsers)
				return allUsersData(role, row, index.column());
#endif
			switch (role) {
				case idRole: return appUserModel()->userId(user_idx);
				case insertTimeRole: return QDateTime::fromMSecsSinceEpoch(appUserModel()->m_usersData.at(user_idx).at(
																DBUserModel::USER_FIELD_INSERTTIME).toLongLong()).toString();
				case onlineAccountRole: return appUserModel()->onlineAccount();
				case nameRole: return appUserModel()->userName(user_idx);
				case birthdayRole: return appUserModel()->birthDateFancy(user_idx);
				case sexRole: return appUserModel()->sex(user_idx);
				case phoneRole: return appUserModel()->phoneNumber(user_idx);
				case emailRole: return appUserModel()->email(user_idx);
				case socialMediaRole: return appUserModel()->_socialMedia(user_idx);
				case userRoleRole: return appUserModel()->userRole(user_idx);
				case coachroleRole: return appUserModel()->coachRole(user_idx);
				case goalRole: return appUserModel()->goal(user_idx);
				case categoryRole: return appUserModel()->userCategory(user_idx);
				case avatarRole: return appUserModel()->avatar(user_idx);
				case userIdxRole: return user_idx;
				case selectedRole: return isSelected(rowFromVisibleRow(row), index.column());
				case itemVisibleRole: return rowVisible(rowFromVisibleRow(row));
				case isCoachRole: return appUserModel()->isCoach(user_idx);
				case isClientRole: return appUserModel()->isClient(user_idx);
				case isConfirmedRole: return appUserModel()->isConfirmed(user_idx);
				case isAvailableRole: return appUserModel()->isAvailable(user_idx);
				default: break;
			}
		}
	}
	return QVariant{};
}

bool UserInfoListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < count()) {
		const int user_idx{userIdx(row)};
		if (user_idx >= 0) {
#ifndef Q_OS_ANDROID
			if (m_allUsers)
				return setAllUsersData(user_idx, row, index.column(), role, value);
#endif
			switch (role) {
			case onlineAccountRole: appUserModel()->setOnlineAccount(value.toBool(), user_idx); break;
			case nameRole: appUserModel()->setUserName(user_idx, value.toString()); break;
			case birthdayRole: appUserModel()->setBirthDate(user_idx, value.toDate()); break;
			case sexRole: appUserModel()->setSex(user_idx, value.toBool()); break;
			case phoneRole: appUserModel()->setPhone(user_idx, value.toString()); break;
			case emailRole: appUserModel()->setEmail(user_idx, value.toString()); break;
			case socialMediaRole: appUserModel()->setSocialMedia(user_idx, value.toString()); break;
			case userRoleRole: appUserModel()->setUserRole(user_idx, value.toString()); break;
			case coachroleRole: appUserModel()->setCoachRole(user_idx, value.toString()); break;
			case goalRole: appUserModel()->setGoal(user_idx, value.toString()); break;
			case avatarRole: appUserModel()->setAvatar(user_idx, value.toString()); break;
			case selectedRole: setSelected(rowFromVisibleRow(row),value.toBool(), index.column()); break;
			case itemVisibleRole: setRowVisible(rowFromVisibleRow(row), value.toBool(), index.column()); break;
			case isCoachRole: appUserModel()->setIsCoach(user_idx, value.toBool()); break;
			case isClientRole: appUserModel()->setIsClient(user_idx, value.toBool()); break;
			case isConfirmedRole: appUserModel()->setIsConfirmed(user_idx, value.toBool()); break;
			case isAvailableRole: appUserModel()->setIsAvailable(user_idx, value.toBool()); break;
			default: return false;
			}
			emit dataChanged(index, index, QList<int>{} << role);
			return true;
		}
	}
	return false;
}

QVariant UserInfoListModel::headerData(int section, Qt::Orientation orientation, int header_role) const
{
	if (header_role == Qt::DisplayRole) {
		if (orientation == Qt::Vertical)
			return section;
		else {
			switch (section) {
			case DBUserModel::USER_FIELD_ID:			return appUserModel()->idLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_INSERTTIME:	return tr("Insert Time: ");
			case DBUserModel::USER_FIELD_ONLINEACCOUNT:	return appUserModel()->onlineAccountUserLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_NAME:			return appUserModel()->nameLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_BIRTHDAY:		return appUserModel()->birthdayLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_SEX:			return appUserModel()->sexLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_PHONE:			return appUserModel()->phoneLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_EMAIL:			return appUserModel()->emailLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_SOCIALMEDIA:	return appUserModel()->socialMediaLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_USERROLE:		return appUserModel()->userRoleLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_COACHROLE:		return appUserModel()->coachRoleLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_GOAL:			return appUserModel()->goalLabel().section(':', 0, 0);
			case DBUserModel::USER_FIELD_USER_CATEGORY:	return appUserModel()->categoryLabel().section(':', 0, 0);
			}
		}
	}
	return QVariant{};
}

#ifndef Q_OS_ANDROID
void UserInfoListModel::clear()
{
	m_allUsersData.clear();
	m_extraInfo.clear();
	m_filter.clear();
	m_nSelected = 0;
	m_currentRow = -1;
	m_fieldFilter = -1;
}

QVariant UserInfoListModel::allUsersData(int role, int row, const int column) const
{
	if (row == -1)
		row = m_currentRow;
	const int app_usermodel_useridx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID,
																  m_allUsersData.at(row).at(DBUserModel::USER_FIELD_ID))};
	switch (role) {
		case avatarRole: return appUserModel()->avatar(app_usermodel_useridx);
		case userIdxRole: return row;
		case selectedRole: return isSelected(rowFromVisibleRow(row), column);
		case itemVisibleRole: return rowVisible(rowFromVisibleRow(row));
		case isCoachRole: return appUserModel()->isCoach(app_usermodel_useridx);
		case isClientRole: return appUserModel()->isClient(app_usermodel_useridx);
		case isConfirmedRole: return appUserModel()->isConfirmed(app_usermodel_useridx);
		case isAvailableRole: return appUserModel()->isAvailable(app_usermodel_useridx);
		default: return m_allUsersData.at(row).at(role - Qt::UserRole);
	}
	return QVariant{};
}

bool UserInfoListModel::setAllUsersData(const uint user_idx, const int row, const int column, int role, const QVariant &value)
{
	const int app_usermodel_useridx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID,
																  m_allUsersData.at(row).at(DBUserModel::USER_FIELD_ID))};
	switch (role) {
		case avatarRole: appUserModel()->setAvatar(app_usermodel_useridx, value.toString()); break;
		case userIdxRole: return false;
		case selectedRole: setSelected(rowFromVisibleRow(row),value.toBool(), column); break;
		case itemVisibleRole: setRowVisible(rowFromVisibleRow(row), value.toBool(), column); break;
		case isCoachRole: appUserModel()->setIsCoach(app_usermodel_useridx, value.toBool()); break;
		case isClientRole: appUserModel()->setIsClient(app_usermodel_useridx, value.toBool()); break;
		case isConfirmedRole: appUserModel()->setIsConfirmed(app_usermodel_useridx, value.toBool()); break;
		case isAvailableRole: appUserModel()->setIsAvailable(app_usermodel_useridx, value.toBool()); break;
		default: m_allUsersData[row][role - Qt::UserRole] = std::move(value.toString()); break;
	}
	emit dataChanged(QAbstractListModel::index(row, column), QAbstractListModel::index(row, column), QList<int>{} << role);
	return true;
}

void UserInfoListModel::removeUserInfo(const int row)
{
	beginRemoveRows(QModelIndex{}, row, row);
	m_extraInfo.removeAt(row);
	m_allUsersData.removeAt(row);
	endRemoveRows();
}
#endif

void UserInfoListModel::userModified(const uint user_idx, const uint field)
{
	const int row{findRow(user_idx)};
	switch (field) {
#ifndef Q_OS_ANDROID
	case USER_MODIFIED_SWITCHING:
		clear();
		gatherAllUsersInfo();
		break;
#endif
	case USER_MODIFIED_CREATED:
	case USER_MODIFIED_IMPORTED:
		beginInsertRows(QModelIndex{}, row, row);
		insertUserInfo(user_idx);
		endInsertRows();
		break;
	case USER_MODIFIED_REMOVED:
		beginRemoveRows(QModelIndex{}, row, row);
		m_extraInfo.removeAt(user_idx);
		endRemoveRows();
		break;
	case DBUserModel::USER_FIELD_AVATAR:
		m_extraInfo[user_idx][EF_AVATAR] = std::move(appUserModel()->avatar(user_idx));
		break;
	case DBUserModel::USER_FIELD_USER_CATEGORY:
		m_extraInfo[user_idx][EF_ISCOACH] = std::move(appUserModel()->isCoach(user_idx));
		emit dataChanged(index(row, 0), index(row, 0), QList<int>{isCoachRole});
		m_extraInfo[user_idx][EF_ISCLIENT] = std::move(appUserModel()->isClient(user_idx));
		emit dataChanged(index(row, 0), index(row, 0), QList<int>{isClientRole});
		m_extraInfo[user_idx][EF_ISCONFIRMED] = std::move(appUserModel()->isConfirmed(user_idx));
		emit dataChanged(index(row, 0), index(row, 0), QList<int>{isConfirmedRole});
		m_extraInfo[user_idx][EF_ISAVAILABLE] = std::move(appUserModel()->isAvailable(user_idx));
		emit dataChanged(index(row, 0), index(row, 0), QList<int>{isAvailableRole});
		changeVisibilityAsPerCategory();
		break;
	default: break;
	}
}

void UserInfoListModel::removeUserInfo(const uint user_idx)
{
	m_extraInfo.removeAt(user_idx);
	for (auto idx{user_idx}; idx < m_extraInfo.count(); ++idx)
		m_extraInfo[idx][EF_USERIDX] = std::move(QVariant{idx});
	emit countChanged();
}

void UserInfoListModel::insertUserInfo(const uint user_idx)
{
	QVariantList extra_infolist;
	extra_infolist.append(std::move(appUserModel()->avatar(user_idx)));
	extra_infolist.append(std::move(user_idx));
	extra_infolist.append(std::move(false));
	extra_infolist.append(std::move(QString{}));
	extra_infolist.append(std::move(true));
	extra_infolist.append(std::move(appUserModel()->isCoach(user_idx)));
	extra_infolist.append(std::move(appUserModel()->isClient(user_idx)));
	extra_infolist.append(std::move(appUserModel()->isConfirmed(user_idx)));
	m_extraInfo.append(std::move(extra_infolist));
}

inline const bool UserInfoListModel::rowVisible(const uint row) const
{
	return m_extraInfo.at(row).at(EF_VISIBLE).toBool();
}

void UserInfoListModel::setRowVisible(const uint row, bool visible, const int column)
{
	if (row < count()) {
		const bool current_visibility{rowVisible(row)};
		if (current_visibility != visible) {
			if (visible && row > m_currentRow)
				setCurrentRow(row);
			else if (!visible && row <= m_currentRow) {
				if (m_currentRow < 0 && count() > 0)
					m_currentRow = 0;
			}
			m_extraInfo[row][EF_VISIBLE] = std::move(visible);
			emit dataChanged(index(row, column), index(row, column), QList<int>{itemVisibleRole});
		}
	}
}

void UserInfoListModel::changeVisibilityAsPerCategory()
{
	for (auto i{0}; i < m_extraInfo.count(); ++i) {
		bool _visible{m_showCoaches && m_extraInfo.at(i).at(EF_ISCOACH).toBool()};
		if (!_visible)
			_visible &= m_showClients && m_extraInfo.at(i).at(EF_ISCLIENT).toBool();
		if (_visible) {
			if (!m_showPending || m_showPending && m_extraInfo.at(i).at(EF_ISCONFIRMED).toBool())
				_visible = false;
			else {
				if (!m_showAvailable || m_showAvailable && !m_extraInfo.at(i).at(EF_ISAVAILABLE).toBool())
					_visible = false;
				else {
					if (m_fieldFilter != -1) {
						if (!m_filter.isEmpty())
							_visible = appUserModel()->m_usersData.at(i).at(m_fieldFilter).contains(m_filter);
					}
				}
			}
		}
		setRowVisible(i, _visible);
	}
}

int UserInfoListModel::findRow(const uint user_idx) const
{
	int row{-1};
	for(int i{0}; i < m_extraInfo.count(); ++i) {
		if (m_extraInfo.at(i).at(EF_VISIBLE).toBool()) {
			++row;
			if (m_extraInfo.at(i).at(EF_USERIDX).toUInt() == user_idx)
				break;
		}
	}
	return row;
}

inline int UserInfoListModel::rowFromVisibleRow(const uint visible_row) const
{
	int row{0};
	for(int i{0}; i < m_extraInfo.count(); ++i) {
		if (rowVisible(i)) {
			if (row == visible_row)
				break;
			++row;
		}
	}
	return row;
}

void UserInfoListModel::gatherAllUsersInfo()
{
	const auto n_users{appUserModel()->m_usersData.count()};
	m_extraInfo.reserve(n_users);
	for (auto user_idx{0}; user_idx < n_users; ++user_idx)
		insertUserInfo(user_idx);
}
