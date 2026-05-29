#include "tpfilepath.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QDir>

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
	QString sane_path{std::move(appUtils()->sanitizePath(filepath))};
	m_pathOK = filepath.startsWith(appSettings()->localAppFilesDir());
	if (m_pathOK) {
		m_fileName = std::move(appUtils()->getFileName(sane_path));
		auto start_pos{appSettings()->localAppFilesDir().length()};
		QString partial_path{std::move(sane_path.right(sane_path.length() - start_pos))};
		m_ownerUser = std::move(appUtils()->getNthDirInPath(partial_path, 1));
		bool owner_ok{false};
		static_cast<void>(m_ownerUser.toLongLong(&owner_ok));
		if (owner_ok) {
			m_targetUser = std::move(appUtils()->getNthDirInPath(partial_path, 2));
			bool has_target_user{true};
			static_cast<void>(m_targetUser.left(5).toUInt(&has_target_user));
			if (has_target_user) {
				start_pos = partial_path.indexOf('/', start_pos + 1);
				partial_path = partial_path.sliced(start_pos, partial_path.length() - start_pos - 1);
			}
			m_subDirs = std::move(appUtils()->getNthDirInPath(partial_path));
			m_fullPathOK = false;
		}
		else
			m_pathOK = false;
		return;
	}
	setExternalFileName(sane_path);
}
