#include "onlineuserinfo.h"

#include "../return_codes.h"
#include "../dbusermodel.h"
#include "../tputils.h"

constexpr uint totalExtraFields{4};

enum RoleNames {
	createRole(id, USER_COL_ID)
	createRole(name, USER_COL_NAME)
	createRole(birthday, USER_COL_BIRTHDAY)
	createRole(sex, USER_COL_SEX)
	createRole(phone, USER_COL_PHONE)
	createRole(email, USER_COL_EMAIL)
	createRole(socialMedia, USER_COL_SOCIALMEDIA)
	createRole(userrole, USER_COL_USERROLE)
	createRole(coachrole, USER_COL_COACHROLE)
	createRole(goal, USER_COL_GOAL)
	createRole(useMode, USER_COL_APP_USE_MODE)
	createRole(extraName, useModeRole + 1)
	createRole(selected, extraNameRole + 1)
	createRole(isCoach, selectedRole + 1)
	createRole(allData, isCoachRole + 1)
	createRole(itemVisible, allDataRole + 1)
};

OnlineUserInfo::OnlineUserInfo(QObject *parent)
	: QAbstractListModel{parent}, m_nselected{0}, m_totalCols{USER_TOTAL_COLS}, m_currentRow{-1}, m_selectEntireRow{false}
{
	roleToString(id)
	roleToString(name)
	roleToString(birthday)
	roleToString(sex)
	roleToString(phone)
	roleToString(email)
	roleToString(socialMedia)
	roleToString(userrole)
	roleToString(coachrole)
	roleToString(goal)
	roleToString(useMode)
	roleToString(extraName)
	roleToString(selected)
	roleToString(isCoach)
	roleToString(allData)
	roleToString(itemVisible)
}

bool OnlineUserInfo::isSelected(const uint row, const int column) const
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setSelected", "row out of range");
	return appUtils()->getCompositeValue(column, m_extraInfo.at(row).at(USER_EXTRA_SELECTED),
																				fancy_record_separator1) == '1';
}

void OnlineUserInfo::setSelected(const uint row, const bool selected, const int column)
{
	if (row < count())
	{
		const bool item_already_selected{isSelected(row, column)};

		if (m_selectEntireRow)
		{
			for (uint i {0}; i < m_totalCols; ++i)
				appUtils()->setCompositeValue(i, selected ? "1"_L1 : "0"_L1, m_extraInfo[row][USER_EXTRA_SELECTED],
																				fancy_record_separator1);
			emit dataChanged(index(row, 0), index(row, m_totalCols), QList<int>{selectedRole});
		}
		else
		{
			appUtils()->setCompositeValue(column, selected ? "1"_L1 : "0"_L1, m_extraInfo[row][USER_EXTRA_SELECTED],
																				fancy_record_separator1);
			emit dataChanged(index(row, column), index(row, column), QList<int>{selectedRole});
		}
		if (item_already_selected && !selected)
			--m_nselected;
		else if (!item_already_selected && selected)
			++m_nselected;
		emit selectedChanged();
	}
}

void OnlineUserInfo::setExtraName(const uint row, const QString &extra_name)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_NAME] = extra_name;
		emit dataChanged(index(row), index(row), QList<int>{extraNameRole});
	}
}

void OnlineUserInfo::setIsCoach(const uint row, bool coach)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_ISCOACH] = coach ? '1' : '0';
		emit dataChanged(index(row), index(row), QList<int>{isCoachRole});
	}
}

void OnlineUserInfo::setVisible(const uint row, bool visible, const int column)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_VISIBLE] = visible ? '1' : '0';
		emit dataChanged(index(row, column), index(row, column), QList<int>{itemVisibleRole});
	}
}

bool OnlineUserInfo::dataFromFileSource(const QString &filename)
{
	bool imported{appUserModel()->importFromFile(filename) == TP_RET_CODE_IMPORT_OK};
	if (imported)
	{
		beginInsertRows(QModelIndex{}, count(), count());
		const qsizetype row{m_modeldata.count()};
		m_modeldata.append(std::move(appUserModel()->tempUserData()));
		setupExtraInfo(row);
		emit countChanged();
		setCurrentRow(row);
		endInsertRows();
	}
	return imported;
}

bool OnlineUserInfo::dataFromString(const QString &user_data)
{
	QStringList tempmodeldata{std::move(user_data.split('\n'))};
	if (tempmodeldata.count() < USER_TOTAL_COLS)
		return false;
	if (tempmodeldata.count() > USER_TOTAL_COLS)
		tempmodeldata.removeLast(); //remove the password field
	beginInsertRows(QModelIndex{}, count(), count());
	const qsizetype row{m_modeldata.count()};
	m_modeldata.append(std::move(tempmodeldata));
	emit countChanged();
	setupExtraInfo(row);
	setCurrentRow(row);
	endInsertRows();
	return true;
}

void OnlineUserInfo::dataFromUserModel(const uint user_idx)
{
	beginInsertRows(QModelIndex{}, count(), count());
	const qsizetype row{m_modeldata.count()};
	m_modeldata.append(appUserModel()->m_usersData.at(user_idx));
	m_extraInfo.append(std::move(QStringList{totalExtraFields}));
	emit countChanged();
	setupExtraInfo(row);
	setCurrentRow(row);
	endInsertRows();
}

void OnlineUserInfo::dataFromOnlineUserInfo(const OnlineUserInfo *other_userinfo, const int other_row)
{
	const uint first_row{other_row != -1 ? static_cast<uint>(other_row) : 0};
	const uint last_row{other_row != -1 ? static_cast<uint>(other_row) : other_userinfo->count() - 1};
	beginInsertRows(QModelIndex{}, count(), count());
	for (uint row{first_row}; row <= last_row; ++row)
	{
		m_modeldata.append(other_userinfo->m_modeldata.at(row));
		m_extraInfo.append(other_userinfo->m_extraInfo.at(row));
	}
	emit countChanged();
	endInsertRows();
}

void OnlineUserInfo::removeUserInfo(const uint row)
{
	if (row < count())
	{
		beginRemoveRows(QModelIndex{}, row, row);
		m_modeldata.remove(row);
		m_extraInfo.remove(row);

		if (m_currentRow >= row)
		{
			m_currentRow--;
			if (m_currentRow < 0 && count() > 0)
				m_currentRow = 0;
		}
		emit countChanged();
		endRemoveRows();
	}
}

bool OnlineUserInfo::sanitize(const QStringList &user_list, const uint field)
{
	const qsizetype n{count()};
	qsizetype i{n};
	while (--i >= 0)
	{
		const QString fieldValue{m_modeldata.at(i).at(field)};
		const auto &it{std::find_if(user_list.cbegin(), user_list.cend(), [fieldValue] (const auto user)
		{
			return user.startsWith(fieldValue);
		})};
		if (it == user_list.cend())
			removeUserInfo(i);
	}
	return n != count();
}

void OnlineUserInfo::clear()
{
	if (count() > 0)
	{
		beginResetModel();
		beginRemoveRows(QModelIndex{}, 0, count() - 1);
		m_modeldata.clear();
		m_extraInfo.clear();
		m_currentRow = -1;
		emit countChanged();
		endRemoveRows();
		endResetModel();
	}
}

void OnlineUserInfo::makeUserDefault(const uint row)
{
	if (row < count())
	{
		if (isUserDefault(row))
			setExtraName(row, m_modeldata.last().at(USER_COL_NAME));
		m_extraInfo.swapItemsAt(0, row);
		m_modeldata.swapItemsAt(0, row);
		setExtraName(0, '*' + m_modeldata.last().at(USER_COL_NAME));
	}
}

int OnlineUserInfo::getRowFromUserIdx(const uint user_idx) const
{
	if (user_idx < appUserModel()->userCount())
	{
		const QString &user_id{appUserModel()->userId(user_idx)};
		uint row{0};
		for (const auto &userdata : m_modeldata)
		{
			if (userdata.at(USER_COL_ID) == user_id)
				return row;
			++row;
		}
	}
	return -1;
}

int OnlineUserInfo::getUserIdx(int row, const bool exact_match) const
{
	int user_idx{-1};
	if (row == -1)
		row = currentRow();
	if (row >= 0 && row < count())
	{
		user_idx = appUserModel()->userIdxFromFieldValue(USER_COL_ID, m_modeldata.at(row).at(USER_COL_ID), exact_match);
		if (user_idx == -1) //temporary online user
			user_idx = appUserModel()->getTemporaryUserInfo(const_cast<OnlineUserInfo*>(this), row);
	}
	return user_idx;
}

bool OnlineUserInfo::containsUser(const QString &userid) const
{
	for (const auto &it: m_modeldata)
	{
		if (it.at(USER_COL_ID) == userid)
			return true;
	}
	return false;
}

void OnlineUserInfo::applyFilter(const QString &filter, int field)
{
	if (field == -1)
		field = USER_COL_NAME;
	for (uint i {0}; i < m_modeldata.count(); ++i)
	{
		const bool visible{m_modeldata.at(i).at(field).contains(filter, Qt::CaseInsensitive)};
		setVisible(i, visible);
	}
}

QString OnlineUserInfo::fieldValueFromAnotherFieldValue(const uint target_field, const uint needle_field,
															const QString &needle) const
{
	const auto &user{std::find_if(m_modeldata.cbegin(), m_modeldata.cend(), [needle_field,needle] (const auto &user_info) {
		return user_info.at(needle_field) == needle;
	})};
	if (user != m_modeldata.cend())
		return user->at(target_field);
	return QString{};
}

QVariant OnlineUserInfo::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0 && row < m_modeldata.count())
	{
		switch (role)
		{
			case idRole: return m_modeldata.at(row).at(USER_COL_ID);
			case nameRole: return m_modeldata.at(row).at(USER_COL_NAME);
			case birthdayRole: return m_modeldata.at(row).at(USER_COL_BIRTHDAY);
			case sexRole: return m_modeldata.at(row).at(USER_COL_SEX);
			case phoneRole: return m_modeldata.at(row).at(USER_COL_PHONE);
			case emailRole: return m_modeldata.at(row).at(USER_COL_EMAIL);
			case socialMediaRole: return m_modeldata.at(row).at(USER_COL_SOCIALMEDIA);
			case userroleRole: return m_modeldata.at(row).at(USER_COL_USERROLE);
			case coachroleRole: return m_modeldata.at(row).at(USER_COL_COACHROLE);
			case goalRole: return m_modeldata.at(row).at(USER_COL_GOAL);
			case useModeRole: return m_modeldata.at(row).at(USER_COL_APP_USE_MODE);
			case extraNameRole: return extraName(row);
			case selectedRole: return isSelected(row, index.column());
			case isCoachRole: return isCoach(row);
			case allDataRole: return m_modeldata.at(row).at(index.column());
			case itemVisibleRole: return visible(row);
		}
	}
	return QVariant{};
}

bool OnlineUserInfo::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && row < m_modeldata.count())
	{
		switch (role)
		{
			case idRole:
			case nameRole:
			case birthdayRole:
			case sexRole:
			case phoneRole:
			case emailRole:
			case socialMediaRole:
			case userroleRole:
			case coachroleRole:
			case goalRole:
			case useModeRole:
				setData(row, role-Qt::UserRole, std::move(value.toString()));
			break;
			case extraNameRole:
				setExtraName(row, value.toString());
			break;
			case selectedRole:
				setSelected(row, value.toBool(), index.column());
			break;
			break;
			case isCoachRole:
				setIsCoach(row, value.toBool());
			break;
			case itemVisibleRole:
				setVisible(row, value.toBool());
			break;
			default:
				return false;
		}
		return true;
	}
	return false;
}

QVariant OnlineUserInfo::headerData(int section, Qt::Orientation orientation, int header_role) const
{
	if (header_role == Qt::DisplayRole)
	{
		if (orientation == Qt::Vertical)
			return section;
		else
		{
			switch (section)
			{
				case USER_COL_ID: return appUserModel()->idLabel().section(':', 0, 0);
				case USER_COL_INSERTTIME: return QString{};
				case USER_COL_ONLINEACCOUNT: return appUserModel()->onlineAccountUserLabel().section(':', 0, 0);
				case USER_COL_NAME: return appUserModel()->nameLabel().section(':', 0, 0);
				case USER_COL_BIRTHDAY: return appUserModel()->birthdayLabel().section(':', 0, 0);
				case USER_COL_SEX: return appUserModel()->sexLabel().section(':', 0, 0);
				case USER_COL_PHONE: return appUserModel()->phoneLabel().section(':', 0, 0);
				case USER_COL_EMAIL: return appUserModel()->emailLabel().section(':', 0, 0);
				case USER_COL_SOCIALMEDIA: return appUserModel()->socialMediaLabel().section(':', 0, 0);
				case USER_COL_USERROLE: return appUserModel()->userRoleLabel().section(':', 0, 0);
				case USER_COL_COACHROLE: return appUserModel()->coachRoleLabel().section(':', 0, 0);
				case USER_COL_GOAL: return appUserModel()->goalLabel().section(':', 0, 0);
				case USER_COL_APP_USE_MODE: return appUserModel()->appUseModelLabel().section(':', 0, 0);
			}
		}
	}
	return QVariant{};
}

void OnlineUserInfo::setupExtraInfo(const uint row)
{
	m_extraInfo.append(std::move(QStringList{totalExtraFields}));
	setExtraName(row, m_modeldata.last().at(USER_COL_NAME));
	setSelected(row, false);
	setVisible(row, true);
}
