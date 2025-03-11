#include "onlineuserinfo.h"

#include "../tputils.h"
#include "../dbusermodel.h"

constexpr uint totalExtraFields{6};

enum RoleNames {
	idRole = Qt::UserRole + USER_COL_ID,
	nameRole = Qt::UserRole + USER_COL_NAME,
	birthdayRole = Qt::UserRole + USER_COL_BIRTHDAY,
	sexRole = Qt::UserRole + USER_COL_SEX,
	phoneRole = Qt::UserRole + USER_COL_PHONE,
	emailRole = Qt::UserRole + USER_COL_EMAIL,
	socialMediaRole = Qt::UserRole + USER_COL_SOCIALMEDIA,
	userRole = Qt::UserRole + USER_COL_USERROLE,
	coachRole = Qt::UserRole + USER_COL_COACHROLE,
	goalRole = Qt::UserRole + USER_COL_GOAL,
	useModeRole = Qt::UserRole + USER_COL_APP_USE_MODE,
	displayRole = useModeRole+1,
	selectedRole = displayRole+1,
	sourceFileRole = selectedRole+1,
	profileRole = sourceFileRole+1,
	avatarRole = profileRole+1,
	resumeRole = avatarRole+1
};

OnlineUserInfo::OnlineUserInfo(QObject *parent)
	: QAbstractListModel{parent}, m_nselected{0}, m_currentRow{-1}
{
	m_roleNames[idRole] = std::move("id");
	m_roleNames[nameRole] = std::move("name");
	m_roleNames[birthdayRole] = std::move("birthday");
	m_roleNames[sexRole] = std::move("sex");
	m_roleNames[phoneRole] = std::move("phone");
	m_roleNames[emailRole] = std::move("email");
	m_roleNames[socialMediaRole] = std::move("socialMedia");
	m_roleNames[userRole] = std::move("userrole");
	m_roleNames[coachRole] = std::move("coachrole");
	m_roleNames[goalRole] = std::move("goal");
	m_roleNames[useModeRole] = std::move("useMode");
	m_roleNames[displayRole] = std::move("display");
	m_roleNames[selectedRole] = std::move("selected");
	m_roleNames[sourceFileRole] = std::move("source");
}

void OnlineUserInfo::setSelected(const uint row, bool selected)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setSelected", "row out of range");
	if (m_extraInfo.at(row).at(USER_EXTRA_SELECTED) == "1"_L1 && !selected)
		--m_nselected;
	else if (m_extraInfo.at(row).at(USER_EXTRA_SELECTED) == "0"_L1 && selected)
		++m_nselected;
	m_extraInfo[row][USER_EXTRA_SELECTED] = selected ? "1"_L1 : "0"_L1;
	emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << selectedRole);
}

void OnlineUserInfo::setSourceFile(const uint row, const QString &source_file)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setSourceFile", "row out of range");
	m_extraInfo[row][USER_EXTRA_SOURCE] = source_file;
	emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << sourceFileRole);
}

void OnlineUserInfo::setProfile(const uint row, const QString &filename, const bool override)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setProfile", "row out of range");
	m_extraInfo[row][USER_EXTRA_SOURCE+ASSOCIATED_FILES_PROFILE] = override ? filename :
									std::move(m_sourcePath + data(row, USER_COL_ID) + '_' + filename);
	emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << profileRole);
}

void OnlineUserInfo::setAvatar(const uint row, const QString &filename, const bool override)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setAvatar", "row out of range");
	m_extraInfo[row][USER_EXTRA_SOURCE+ASSOCIATED_FILES_AVATAR] = override ? filename :
									std::move(m_sourcePath + data(row, USER_COL_ID) + '_' + filename);
	emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << avatarRole);
}

void OnlineUserInfo::setResume(const uint row, const QString &filename, const bool override)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::setResume", "row out of range");
	m_extraInfo[row][USER_EXTRA_SOURCE+ASSOCIATED_FILES_RESUME] = override ? filename :
									std::move(m_sourcePath + data(row, USER_COL_ID) + '_' + filename);
	emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << resumeRole);
}

bool OnlineUserInfo::dataFromFileSource(const QString &filename, const QString &new_user_id)
{
	beginInsertRows(QModelIndex{}, count(), count());
	bool imported{appUserModel()->_importFromFile(filename, m_modeldata) == APPWINDOW_MSG_READ_FROM_FILE_OK};
	if (imported)
	{
		const qsizetype row{m_modeldata.count()-1};
		m_extraInfo.append(std::move(QStringList{totalExtraFields}));
		if (row == 0)
			m_sourcePath = appUtils()->getFilePath(filename);
		emit countChanged();
		setData(row, USER_COL_ID, new_user_id);
		setData(index(row, 0), m_modeldata.last().at(USER_COL_NAME), displayRole);
		setSelected(row, false);
		setSourceFile(row, filename);
		setProfile(row, "profile.txt"_L1);
		setAvatar(row, "avatar"_L1);
		setResume(row, "resume"_L1);
		setCurrentRow(row);
	}
	endInsertRows();
	return imported;
}

bool OnlineUserInfo::dataFromString(const QString &user_data, const QString &new_user_id)
{
	QStringList tempmodeldata{std::move(user_data.split('\n'))};
	if (tempmodeldata.count() < USER_TOTAL_COLS)
		return false;
	if (tempmodeldata.count() > USER_TOTAL_COLS)
		tempmodeldata.removeLast(); //remove the password field
	beginInsertRows(QModelIndex{}, count(), count());
	const qsizetype row{m_modeldata.count()};
	m_modeldata.append(std::move(tempmodeldata));
	m_modeldata.last()[USER_COL_COACHES].clear(); //not needed. Shouldn't even be downloaded, but it's easier to erase here
	m_modeldata.last()[USER_COL_CLIENTS].clear(); //not needed. Shouldn't even be downloaded, but it's easier to erase here
	m_extraInfo.append(std::move(QStringList{totalExtraFields}));
	emit countChanged();
	setData(row, USER_COL_ID, new_user_id);
	setData(index(row, 0), m_modeldata.last().at(USER_COL_NAME), displayRole);
	setSelected(row, false);
	setSourceFile(row, QString{});
	setProfile(row, "profile.txt"_L1);
	setAvatar(row, "avatar"_L1);
	setResume(row, "resume"_L1);
	setCurrentRow(row);
	endInsertRows();
	return true;
}


void OnlineUserInfo::removeUserInfo(const uint row, const bool remove_source)
{
	Q_ASSERT_X(row < count(), "OnlineUserInfo::data", "row out of range");
	if (remove_source)
		static_cast<void>(QFile::remove(data(row, USER_EXTRA_SOURCE)));
	beginRemoveRows(QModelIndex{}, row, row);
	static_cast<void>(QFile::remove(associatedFile(row, ASSOCIATED_FILES_PROFILE)));
	static_cast<void>(QFile::remove(associatedFile(row, ASSOCIATED_FILES_AVATAR)));
	static_cast<void>(QFile::remove(associatedFile(row, ASSOCIATED_FILES_RESUME)));
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

bool OnlineUserInfo::sanitize(const QStringList &user_list, const uint field)
{
	const qsizetype n{count()};
	qsizetype i{n};
	while (--i >= 0)
	{
		const QString fieldValue{m_modeldata.at(i).at(field)};
		bool found{false};
		for (uint x{0}; x < user_list.count(); ++x)
		{
			if (user_list.at(x) == fieldValue)
			{
				found = true;
				break;
			}
		}
		if (!found)
			removeUserInfo(i, true);
	}
	return n != count();
}

void OnlineUserInfo::clear()
{
	if (count() > 0)
	{
		beginResetModel();
		beginRemoveRows(QModelIndex{}, 0, count()-1);
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
	Q_ASSERT_X(row < count(), "makeUserDefault::data", "row out of range");
	if (row > 0)
	{
		if (isUserDefault(row))
			m_extraInfo[row][USER_EXTRA_NAME].removeFirst();
		m_extraInfo.swapItemsAt(0, row);
		m_modeldata.swapItemsAt(0, row);
		m_extraInfo[0][USER_EXTRA_NAME].prepend('*');
		emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << displayRole);
	}
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
			case displayRole: return m_extraInfo.at(row).at(USER_EXTRA_NAME);
			case selectedRole: return isSelected(row);
			case sourceFileRole: return sourceFile(row);
			case profileRole:
			case avatarRole:
			case resumeRole:
				return associatedFile(row, role-displayRole);
		}
	}
	return QVariant();
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
				return true;
			case displayRole:
				m_extraInfo[row][USER_EXTRA_NAME] = std::move(value.toString());
				emit dataChanged(index, index, QList<int>{} << displayRole);
				return true;
			case sourceFileRole:
				setSourceFile(row, value.toString());
				return true;
			case selectedRole:
				setSelected(row, value.toBool());
				return true;
			case profileRole:
				setProfile(row, value.toString());
				return true;
			case avatarRole:
				setAvatar(row, value.toString());
				return true;
			case resumeRole:
				setResume(row, value.toString());
				return true;
		}
	}
	return false;
}
