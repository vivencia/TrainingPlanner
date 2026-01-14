#pragma once

#include "thread_manager.h"

#include <QHash>
#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

constexpr uint8_t APP_TABLES_NUMBER{7};
constexpr uint8_t EXERCISES_TABLE_ID{0x01};
constexpr uint8_t MESOCYCLES_TABLE_ID{0x02};
constexpr uint8_t MESOSPLIT_TABLE_ID{0x03};
constexpr uint8_t MESOCALENDAR_TABLE_ID{0x04};
constexpr uint8_t WORKOUT_TABLE_ID{0x05};
constexpr uint8_t USERS_TABLE_ID{0x06};
constexpr uint8_t CHAT_TABLE_ID{0x07};

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
	std::pair<bool,bool> AlterRecords();
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

	void setDBModelInterface(DBModelInterface *dbmodel_interface);
	template<typename T> inline void setReadAllRecordsFunc(const std::function<bool (void *param)> &func)
	{
		m_threadedFunctions.insert(ThreadManager::ReadAllRecords, [this,func] (void *param) {
			const bool result{func(static_cast<T*>(param))};
			emit actionFinished(ThreadManager::ReadAllRecords, QVariant{result}, QVariant{false});
		});
	}

	inline std::function<void(void *)> threadedFunction(ThreadManager::StandardOps op) const { return m_threadedFunctions.value(op); }
	inline std::function<std::pair<QVariant,QVariant>()> &customQueryFunc() { return m_customQueryFunc; }
	inline void setCustQueryFunction(const std::function<std::pair<QVariant,QVariant>()> &func) { m_customQueryFunc = func; }

public slots:
	void startAction(const int unique_id, ThreadManager::StandardOps operation, void *extra_param);

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
	QHash<ThreadManager::StandardOps, std::function<void(void *param)>> m_threadedFunctions;
	std::function<std::pair<QVariant,QVariant>()> m_customQueryFunc;

	bool createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
									const bool overwrite = false) const;
};

