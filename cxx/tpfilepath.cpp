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
	}
	else //when transfering files between users and/or the server, filename will contain the subdirs(see TPUtils::BFIF_FILEPATH)
		setSubDirsPlusFilename(filename);
}

//TPFilePath accepts any filepath/name in the system so the same object can be used by other classes(like TPFileOps)
//to do operations on files we have permission to do.
void TPFilePath::setExternalFileName(const QString &filename)
{
	if (QFile::exists(filename)) {
		m_externalFileName = filename;
		m_fileName = std::move(appUtils()->getFileName(m_externalFileName));
	}
}

void TPFilePath::setOwnerUser(const QString &userid)
{
	bool owner_ok{false};
	static_cast<void>(userid.toLongLong(&owner_ok));
	if (!userid.isEmpty() && owner_ok) {
		m_ownerUser = userid;
		m_fullPathOK = false;
		if (!m_pathOK) { //transform a given filename into a TPFilePath, now that owner user is ok
			m_fileName = std::move(appUtils()->getFileName(m_fileName));
			m_pathOK = true;
		}
	}
}

void TPFilePath::fromString(const QString &filepath)
{
	QString good_path{std::move(appUtils()->sanitizePath(filepath))};
	m_pathOK = good_path.startsWith(_localAppFilesDir);
	if (m_pathOK) {
		good_path.remove(0, _localAppFilesDir.length());
		m_fileName = std::move(appUtils()->getFileName(good_path));
		good_path = std::move(good_path.chopped(m_fileName.length()));
		m_ownerUser = std::move(appUtils()->getNthDirInPath(good_path, 1).chopped(1));
		bool user_ok{false};
		static_cast<void>(m_ownerUser.toLongLong(&user_ok));
		if (user_ok) {
			good_path.remove(0, m_ownerUser.length() + 1);
			m_targetUser = std::move(appUtils()->getNthDirInPath(good_path, 1).chopped(1));
			static_cast<void>(m_targetUser.left(5).toUInt(&user_ok));
			if (user_ok)
				good_path.remove(0, m_targetUser.length() + 1);
			else
				m_targetUser.clear();
			m_subDirs = std::move(good_path);
			m_fullPathOK = false;
		}
		else
			m_pathOK = false;
		return;
	}
	else {
		if (good_path.contains('/'))
			setSubDirsPlusFilename(good_path);
		else
			m_fileName = std::move(good_path);
	}
	setExternalFileName(good_path);
}
