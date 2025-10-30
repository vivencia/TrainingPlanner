#include "onlineuserinfo.h"

#include "../return_codes.h"
#include "../dbusermodel.h"
#include "../tputils.h"

constexpr uint totalExtraFields{4};

enum RoleNames {
	idRole			= Qt::UserRole		+ USER_COL_ID,
	nameRole		= Qt::UserRole		+ USER_COL_NAME,
	birthdayRole	= Qt::UserRole		+ USER_COL_BIRTHDAY,
	sexRole			= Qt::UserRole		+ USER_COL_SEX,
	phoneRole		= Qt::UserRole		+ USER_COL_PHONE,
	emailRole		= Qt::UserRole		+ USER_COL_EMAIL,
	socialMediaRole	= Qt::UserRole		+ USER_COL_SOCIALMEDIA,
	userRole		= Qt::UserRole		+ USER_COL_USERROLE,
	coachRole		= Qt::UserRole		+ USER_COL_COACHROLE,
	goalRole		= Qt::UserRole		+ USER_COL_GOAL,
	useModeRole		= Qt::UserRole		+ USER_COL_APP_USE_MODE,
	extraNameRole	= useModeRole		+ 1,
	selectedRole	= extraNameRole		+ 1,
	sourceFileRole	= selectedRole		+ 1,
	isCoachRole		= sourceFileRole	+ 1,
	allDataRole		= isCoachRole		+ 1,
	visibleRole		= allDataRole		+ 1,
};

OnlineUserInfo::OnlineUserInfo(QObject *parent)
	: QAbstractListModel{parent}, m_nselected{0}, m_totalCols{USER_TOTAL_COLS}, m_currentRow{-1}, m_selectEntireRow{false}
{
	m_roleNames[idRole]				= std::move(QByteArray{"id"});
	m_roleNames[nameRole]			= std::move(QByteArray{"name"});
	m_roleNames[birthdayRole]		= std::move(QByteArray{"birthday"});
	m_roleNames[sexRole]			= std::move(QByteArray{"sex"});
	m_roleNames[phoneRole]			= std::move(QByteArray{"phone"});
	m_roleNames[emailRole]			= std::move(QByteArray{"email"});
	m_roleNames[socialMediaRole]	= std::move(QByteArray{"socialMedia"});
	m_roleNames[userRole]			= std::move(QByteArray{"userrole"});
	m_roleNames[coachRole]			= std::move(QByteArray{"coachrole"});
	m_roleNames[goalRole]			= std::move(QByteArray{"goal"});
	m_roleNames[useModeRole]		= std::move(QByteArray{"useMode"});
	m_roleNames[extraNameRole]		= std::move(QByteArray{"extraName"});
	m_roleNames[selectedRole]		= std::move(QByteArray{"selected"});
	m_roleNames[sourceFileRole]		= std::move(QByteArray{"source"});
	m_roleNames[isCoachRole]		= std::move(QByteArray{"iscoach"});
	m_roleNames[allDataRole]		= std::move(QByteArray{"allData"});
	m_roleNames[Qt::DisplayRole]	= std::move(QByteArray{"display"});
	m_roleNames[visibleRole]		= std::move(QByteArray{"visible"});
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
			emit dataChanged(index(row, 0), index(row, m_totalCols), QList<int>{} << selectedRole);
		}
		else
		{
			appUtils()->setCompositeValue(column, selected ? "1"_L1 : "0"_L1, m_extraInfo[row][USER_EXTRA_SELECTED],
																				fancy_record_separator1);
			emit dataChanged(index(row, column), index(row, column), QList<int>{} << selectedRole);
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
		emit dataChanged(index(row), index(row), QList<int>{} << extraNameRole);
	}
}

void OnlineUserInfo::setSourceFile(const uint row, const QString &source_file)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_SOURCE] = source_file;
		emit dataChanged(index(row), index(row), QList<int>{} << sourceFileRole);
	}
}

void OnlineUserInfo::setIsCoach(const uint row, bool coach)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_ISCOACH] = coach ? '1' : '0';
		emit dataChanged(index(row), index(row), QList<int>{} << isCoachRole);
	}
}

void OnlineUserInfo::setVisible(const uint row, bool visible, const int column)
{
	if (row < count())
	{
		m_extraInfo[row][USER_EXTRA_VISIBLE] = visible ? '1' : '0';
		emit dataChanged(index(row, column), index(row, column), QList<int>{} << visibleRole);
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
		if (row == 0)
			m_sourcePath = std::move(appUtils()->getFilePath(filename));
		setupExtraInfo(row, filename);
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

void OnlineUserInfo::removeUserInfo(const uint row, const bool remove_source)
{
	if (row < count())
	{
		if (remove_source)
			static_cast<void>(QFile::remove(data(row, USER_EXTRA_SOURCE)));
		beginRemoveRows(QModelIndex{}, row, row);
		m_modeldata.remove(row);
		m_extraInfo.remove(row);
		if (count() == 0)
			m_sourcePath.clear();
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
			removeUserInfo(i, true);
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
		m_sourcePath.clear();
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

int OnlineUserInfo::getUserIdx(int row) const
{
	if (row == -1)
		row = currentRow();
	if (row >= 0 && row < count())
		return appUserModel()->userIdxFromFieldValue(USER_COL_ID, m_modeldata.at(row).at(USER_COL_ID));
	return -1;
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
			case userRole: return m_modeldata.at(row).at(USER_COL_USERROLE);
			case coachRole: return m_modeldata.at(row).at(USER_COL_COACHROLE);
			case goalRole: return m_modeldata.at(row).at(USER_COL_GOAL);
			case useModeRole: return m_modeldata.at(row).at(USER_COL_APP_USE_MODE);
			case extraNameRole: return extraName(row);
			case selectedRole: return isSelected(row, index.column());
			case sourceFileRole: return sourceFile(row);
			case isCoachRole: return isCoach(row);
			case allDataRole: return m_modeldata.at(row).at(index.column());
			case visibleRole: return visible(row);
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
			case userRole:
			case coachRole:
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
			case sourceFileRole:
				setSourceFile(row, value.toString());
			break;
			case isCoachRole:
				setIsCoach(row, value.toBool());
			break;
			case visibleRole:
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

void OnlineUserInfo::setupExtraInfo(const uint row, const QString &source_file)
{
	m_extraInfo.append(std::move(QStringList{totalExtraFields}));
	setExtraName(row, m_modeldata.last().at(USER_COL_NAME));
	setSelected(row, false);
	setSourceFile(row, source_file);
	setVisible(row, true);
}
