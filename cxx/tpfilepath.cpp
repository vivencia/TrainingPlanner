#include "tpfilepath.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QDir>
#include <QStandardPaths>

QString TPFilePath::_localAppFilesDir{};

void TPFilePath::setLocalAppFilesDir()
{
	if (TPFilePath::_localAppFilesDir.isEmpty())
		TPFilePath::_localAppFilesDir = std::move(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
																										% QLatin1Char('/');
}

TPFilePath::TPFilePath(const QString &filename, const QString &owner_user, const QString &target_user,
																			const std::initializer_list<QString> &subdirs)
{
	setOwnerUser(owner_user);
	setTargetUser(target_user);
	if (!filename.contains('/')) {
		setFileName(filename, false);
		setSubdirs(subdirs);
	} else //when transfering files between users and/or the server, filename will contain the subdirs(see TPUtils::BFIF_FILEPATH)
		setSubDirsPlusFilename(filename);
}

void TPFilePath::setOwnerUser(const QString &userid)
{
	bool owner_ok{false};
	static_cast<void>(userid.toLongLong(&owner_ok));
	if (!userid.isEmpty() && owner_ok) {
		m_ownerUser = userid;
		m_fullPathOK = false;
	}
}

void TPFilePath::fromString(const QString &filepath)
{
	bool user_ok{false};
	QString good_path{std::move(appUtils()->sanitizePath(filepath))};
	if (good_path.startsWith(_localAppFilesDir))
		good_path.remove(0, _localAppFilesDir.length());

	m_fileName = std::move(appUtils()->getFileName(good_path));
	good_path = std::move(good_path.chopped(m_fileName.length()));

	m_tempString = std::move(appUtils()->getNthDirInPath(good_path, 1));
	if (!m_tempString.isEmpty()) {
		m_tempString.chop(1);
		static_cast<void>(m_tempString.toLongLong(&user_ok));
		if (user_ok) {
			m_ownerUser = std::move(m_tempString);
			good_path.remove(0, m_ownerUser.length() + 1);
		}
	}
	if (!user_ok)
		setOwnerUser(appSettings()->currentUser());
	else
		user_ok = false;

	m_tempString = std::move(appUtils()->getNthDirInPath(good_path, 1));
	if (!m_tempString.isEmpty()) {
		m_tempString.chop(1);
		static_cast<void>(m_tempString.toLongLong(&user_ok));
		if (user_ok) {
			m_targetUser = std::move(m_tempString);
			good_path.remove(0, m_targetUser.length() + 1);
		}
	}
	if (!user_ok)
		m_targetUser.clear();

	if (!good_path.isEmpty())
		m_subDirs = std::move(good_path);
	else
		m_subDirs.clear();
	m_fullPathOK = false;
}
