#pragma once

#include "tpsettings.h"
#include "tputils.h"

QT_FORWARD_DECLARE_CLASS(TPFilePath)

typedef std::shared_ptr<TPFilePath> TPFilePathPtr;

class TPFilePath
{

public:
	static TPFilePathPtr newTPFilePath(const QString &filename = QString{}, const QString &owner_user = QString{},
							const QString &target_user = QString{}, const std::initializer_list<QString> &subdirs = {})
	{
		if (filename.isEmpty())
			return std::make_shared<TPFilePath>();
		else if (owner_user.isEmpty() && target_user.isEmpty() && subdirs.size() == 0)
			return std::make_shared<TPFilePath>(filename);
		else
			return std::make_shared<TPFilePath>(filename, owner_user, target_user, subdirs);
	}

	static TPFilePathPtr newTPFilePath(const TPFilePath &other)
	{
		return std::make_shared<TPFilePath>(other);
	}

	inline TPFilePath() {}
	TPFilePath(const QString &filename, const QString &owner_user, const QString &target_user,
																	const std::initializer_list<QString> &subdirs);
	inline explicit TPFilePath(const QString &filepath) { fromString(filepath); }
	inline TPFilePath(const TPFilePath &other)
		: m_fileName{other.m_fileName}, m_ownerUser{other.m_ownerUser}, m_targetUser{other.m_targetUser},
					m_subDirs{other.m_subDirs}, m_pathOK{other.m_pathOK}, m_fullPathOK{ other.m_fullPathOK},
																	m_useFileExtension{other.m_useFileExtension} {}
	inline TPFilePath(TPFilePath &&other) noexcept
		: m_fileName{other.m_fileName}, m_ownerUser{other.m_ownerUser}, m_targetUser{other.m_targetUser},
					m_subDirs{other.m_subDirs}, m_pathOK{other.m_pathOK}, m_fullPathOK{ other.m_fullPathOK},
																	m_useFileExtension{other.m_useFileExtension} {}

	inline TPFilePath &operator=(const QString &filepath)
	{
		fromString(filepath);
		return *this;
	}

	inline TPFilePath &operator=(const TPFilePath &other)
	{
		if (&other != this) {
			m_fileName = other.m_fileName;
			m_ownerUser = other.m_ownerUser;
			m_targetUser = other.m_targetUser;
			m_subDirs = other.m_subDirs;
			m_pathOK = other.m_pathOK;
			m_fullPathOK = other.m_fullPathOK;
			m_useFileExtension = other.m_useFileExtension;
		}
		return *this;
	}

	inline TPFilePath &operator=(TPFilePath &&other)
	{
		if (&other != this) {
			m_fileName = other.m_fileName;
			m_ownerUser = other.m_ownerUser;
			m_targetUser = other.m_targetUser;
			m_subDirs = other.m_subDirs;
			m_pathOK = other.m_pathOK;
			m_fullPathOK = other.m_fullPathOK;
			m_useFileExtension = other.m_useFileExtension;
		}
		return *this;
	}

	inline QString toString(const bool use_temp_filename = false) const
	{
		if (!m_fullPathOK) {
			const_cast<TPFilePath*>(this)->m_fullPath =
								std::move(appSettings()->localAppFilesDir() % m_ownerUser % '/' % m_subDirs % m_fileName);
			const_cast<TPFilePath*>(this)->m_fullPathOK = true;
		}
		return !use_temp_filename ? m_fullPath : m_fullPath % ".tmp"_L1;
	}
	inline QString filePath() const
	{
		return appUtils()->getFilePath(toString(), false);
	}

	inline bool isOK() const { return m_pathOK; }
	inline bool useFileExtension() const { return m_useFileExtension; }
	inline void setUseFileExtension(const bool extension) { m_useFileExtension = extension; }
	inline const QString &fileName() const
	{
		return m_useFileExtension ? m_fileName : m_tempString = std::move(appUtils()->getFileName(m_fileName, true));
	}
	inline void setFileName(const QString &filename, const bool sanitized)
	{
		m_fileName = filename;
		if (!sanitized)
			m_fileName = std::move(m_fileName.trimmed().remove('/'));
		m_fullPathOK = false;
	}
	inline const QString &externalFilename() const { return m_externalFileName; }
	void setExternalFileName(const QString &filename);
	inline const QString &ownerUser() const { return m_ownerUser; }
	void setOwnerUser(const QString &userid);
	inline const QString &targetUser() const { return m_targetUser; }
	void setTargetUser(const QString &userid) { m_targetUser = userid; m_fullPathOK = false; }
	void setBothUsers(const QString &userid)
	{
		m_ownerUser = userid;
		m_targetUser = userid;
		m_fullPathOK = false;
	}
	const QString &subdirs() const { return m_subDirs; }
	inline void setSubdirs(const QString &subdirs, const bool sanitized)
	{
		m_subDirs = subdirs;
		if (!sanitized)
			m_subDirs = std::move(appUtils()->sanitizePath(m_subDirs));
		m_fullPathOK = false;
	}
	inline void setSubdirs(const std::initializer_list<QString> &subdirs)
	{
		setSubdirs(appUtils()->string_strings(subdirs, QLatin1Char{'/'}), true);
	}
	inline void setSubDirsPlusFilename(const QString &str)
	{
		setFileName(appUtils()->getFileName(str), true);
		setSubdirs(appUtils()->getFilePath(str, false), true);
	}

	inline int generateUniqueId() const
	{
		return fnv1a_hash(m_subDirs % m_fileName);
	}

private:
	QString m_fileName, m_ownerUser, m_targetUser, m_subDirs, m_fullPath, m_externalFileName;
	mutable QString m_tempString;
	bool m_pathOK{false}, m_fullPathOK{false}, m_useFileExtension{true};

	void fromString(const QString &filepath);
};
