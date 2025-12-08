#pragma once

#include "thread_manager.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

constexpr uint APP_TABLES_NUMBER{7};
constexpr uint EXERCISES_TABLE_ID{0x0001};
constexpr uint MESOCYCLES_TABLE_ID{0x0002};
constexpr uint MESOSPLIT_TABLE_ID{0x0003};
constexpr uint MESOCALENDAR_TABLE_ID{0x0004};
constexpr uint WORKOUT_TABLE_ID{0x0005};
constexpr uint USERS_TABLE_ID{0x0006};
constexpr uint CHAT_TABLE_ID{0x0007};

QT_FORWARD_DECLARE_CLASS(DBModelInterface)
QT_FORWARD_DECLARE_CLASS(QFile)

using namespace Qt::Literals::StringLiterals;

class TPDatabaseTable : public QObject
{

Q_OBJECT

public:
	static constexpr QLatin1StringView sqliteApp{"sqlite3"_L1};

	TPDatabaseTable(const TPDatabaseTable &other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable &other) = delete;
	TPDatabaseTable (TPDatabaseTable &&other) = delete;
	TPDatabaseTable& operator() (TPDatabaseTable &&other) = delete;

	virtual QString subDir() const { return "Database/"_L1; }
	virtual QString dbFilePath() const;
	virtual QString dbFileName(const bool fullpath = true) const = 0;
	virtual void updateTable() = 0;

	inline uint tableId() const { return m_tableId; }
	inline int uniqueId() const { return m_uniqueID; }
	inline void setUniqueId(const int uid) { m_uniqueID = uid; }
	inline bool deleteAfterThreadFinished() const { return m_deleteAfterFinished; }

	std::pair<bool,bool> createTable();
	std::pair<bool,bool> insertRecord();
	std::pair<bool,bool> alterRecords();
	std::pair<bool,bool> updateRecord();
	std::pair<bool,bool> updateFieldsOfRecord();
	std::pair<bool,bool> updateRecords();
	std::pair<bool,bool> removeRecords();
	std::pair<bool,bool> clearTable();
	std::pair<bool,bool> removeTemporaries();

	void parseCmdFile(const QString &filename);

	inline const QStringList &databaseFilenamesPool() const { return m_databaseFilenamesPool;}
	bool execReadOnlyQuery(const QString &str_query);
	bool execSingleWriteQuery(const QString &str_query);
	bool execMultipleWritesQuery(const QStringList &queries);

	inline void setDBModelInterface(DBModelInterface *dbmodel_interface) { m_dbModelInterface = dbmodel_interface; }
	inline std::function<void()> threadedFunction(ThreadManager::StandardOps op) const { return m_threadedFunctions.value(op); }
	void setReadAllRecordsFunc(const std::function<bool()> &func);
	inline std::function<std::pair<QVariant,QVariant>()> &customQueryFunc() { return m_customQueryFunc; }
	inline void setCustQueryFunction(const std::function<std::pair<QVariant,QVariant>()> &func) { m_customQueryFunc = func; }

public slots:
	void startAction(const int unique_id, ThreadManager::StandardOps operation);

signals:
	void actionFinished(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2);

protected:
	explicit TPDatabaseTable(const uint table_id, DBModelInterface *dbmodel_interface = nullptr);

	static constexpr QLatin1StringView dbfile_extension{ ".db.sqlite"_L1 };
	void setUpConnection();

	QSqlDatabase m_sqlLiteDB;
	QSqlQuery m_workingQuery;
	QString m_strQuery;
	QStringList m_databaseFilenamesPool;
	const QLatin1StringView *m_tableName;
	const QLatin1StringView (*m_fieldNames)[2];
	int m_uniqueID, m_fieldCount;
	uint m_tableId;
	bool m_deleteAfterFinished;

	DBModelInterface *m_dbModelInterface;

private:
	QHash<ThreadManager::StandardOps, std::function<void()>> m_threadedFunctions;
	std::function<std::pair<QVariant,QVariant>()> m_customQueryFunc;

	bool createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
									const bool overwrite = false) const;
};

