#pragma once

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
	inline TPFilePath(const QString &filepath) { fromString(filepath); }
	inline TPFilePath(const TPFilePath &other)
		: m_fileName{other.m_fileName}, m_ownerUser{other.m_ownerUser}, m_targetUser{other.m_targetUser},
			m_subDirs{other.m_subDirs}, m_fullPathOK{ other.m_fullPathOK} {}
	inline TPFilePath(TPFilePath &&other) noexcept
		: m_fileName{other.m_fileName}, m_ownerUser{other.m_ownerUser}, m_targetUser{other.m_targetUser},
			m_subDirs{other.m_subDirs}, m_fullPathOK{ other.m_fullPathOK} {}

	inline TPFilePath &operator=(const QString &filepath)
	{
		setBothUsers(QString{});
		setSubdirs(QString{}, true);
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
			m_fullPathOK = other.m_fullPathOK;
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
			m_fullPathOK = other.m_fullPathOK;
		}
		return *this;
	}

	inline QString toString() const
	{
		if (!m_fullPathOK || m_fullPath.isEmpty()) {
			const_cast<TPFilePath*>(this)->m_fullPath = std::move(appUtils()->sanitizePath(
						std::move(_localAppFilesDir % m_ownerUser % '/' % m_targetUser % '/' % m_subDirs % m_fileName)));
			const_cast<TPFilePath*>(this)->m_fullPathOK = true;
		}
		return m_fullPath;
	}
	inline QString filePath() const
	{
		return appUtils()->getFilePath(toString(), false);
	}
	inline QString relativeFilePath() const {
		return m_ownerUser % '/' % m_targetUser % '/' % m_subDirs % m_fileName;
	}

	static void setLocalAppFilesDir();
	static inline const QString &localAppFilesDir() { return _localAppFilesDir; }
	inline const QString &fileName(const bool include_extension = true) const
	{
		return include_extension ? m_fileName : m_tempString = std::move(appUtils()->getFileName(m_fileName, true));
	}
	inline QString filename(const bool include_extension = true) & { return QString{fileName(include_extension)}; }
	inline void setFilename(QString &&filename)
	{
		m_fileName = std::forward<QString>(filename);
		m_fullPathOK = false;
	}
	inline void setFileName(const QString &filename, const bool sanitized)
	{
		if (sanitized) {
			m_fileName = filename;
			m_fullPathOK = false;
		} else {
			setFilename(std::move(filename.trimmed().remove('/')));
		}
	}

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
	inline void swapUsers()
	{
		QString owner_user{std::move(m_ownerUser)};
		m_ownerUser = std::move(m_targetUser);
		m_targetUser = std::move(owner_user);
		m_fullPathOK = false;
	}

	inline const QString &subdirs() const { return m_subDirs; }
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
		return fnv1a_hash(m_fullPath);
	}

	inline bool isOK() const { return !m_fileName.isEmpty() && !m_ownerUser.isEmpty(); }

private:
	QString m_fileName, m_ownerUser, m_targetUser, m_subDirs, m_fullPath;
	mutable QString m_tempString;
	static QString _localAppFilesDir;
	bool m_fullPathOK{false};

	void fromString(const QString &filepath);
};
