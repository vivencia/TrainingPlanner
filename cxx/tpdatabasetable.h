#pragma once

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

QT_FORWARD_DECLARE_CLASS(QFile)

using namespace Qt::Literals::StringLiterals;

class TPDatabaseTable : public QObject
{

Q_OBJECT

public:
	static constexpr QLatin1StringView databaseSubDir{"Database/"};

	static constexpr QLatin1StringView databaseFileNames[APP_TABLES_NUMBER+1] = { ""_L1,
									"ExercisesList.db.sqlite"_L1,
									"Mesocycles.db.sqlite"_L1,
									"MesocyclesSplits.db.sqlite"_L1,
									"MesoCalendar.db.sqlite"_L1,
									"Workouts.db.sqlite"_L1,
									"Users.db.sqlite"_L1,
	};

	static constexpr QLatin1StringView sqliteApp{"sqlite3"_L1};

	TPDatabaseTable(const TPDatabaseTable &other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable &other) = delete;
	TPDatabaseTable (TPDatabaseTable &&other) = delete;
	TPDatabaseTable& operator() (TPDatabaseTable &&other) = delete;
	inline ~TPDatabaseTable() { m_sqlLiteDB.close(); }

	static TPDatabaseTable *createDBTable(const uint table_id, const bool auto_delete = true);
	void createTableQuery(const uint table_id);
	virtual inline bool createTable()
	{
		createTableQuery(m_tableId);
		if (execQuery(m_strQuery, false))
			return createCmdFile();
		return false;
	}
	virtual void updateTable() = 0;

	bool createCmdFile();
	virtual QString dbFilePath(const uint table_id, const bool path_only = false);
	inline uint tableId() const { return m_tableId; }
	inline int uniqueId() const { return m_uniqueID; }
	inline void setUniqueId(const int uid) { m_uniqueID = uid; }
	inline bool deleteAfterThreadFinished() const { return m_deleteAfterFinished; }
	inline QThread *originalThread() const { return m_originalThread; }
	inline void addExecArg(const QVariant &arg) { m_execArgs.append(arg); }
	inline void clearExecArgs() { m_execArgs.clear(); }
	inline void changeExecArg(const QVariant &arg, const uint pos)
	{
		if (pos < m_execArgs.count())
			m_execArgs[pos] = arg;
	}

	void removeEntry(const bool bUseMesoId = false);
	void removeTemporaries(const bool bUseMesoId = false);
	void clearTable();
	void removeDBFile();

	bool openDatabase(const bool read_only = false);
	QSqlQuery getQuery() const;
	bool execQuery(const QString &str_query, const bool read_only = true, const bool close_db = true);
	inline const QString &strQuery() const { return m_strQuery; }

	bool executeCmdFile(const QString &cmd_file, const QString &success_message, const bool remove_file = true) const;

signals:
	void threadFinished(const bool send_to_server = false);

protected:
	explicit inline TPDatabaseTable(const uint table_id, QObject *parent = nullptr)
		: QObject{parent}, m_tableId{table_id}, m_deleteAfterFinished{true} { m_originalThread = thread(); }

	inline void setTableName(const QLatin1StringView &table_name) { m_tableName = std::move(QString{table_name}); }
	QSqlDatabase m_sqlLiteDB;
	QSqlQuery m_workingQuery;
	QString m_strQuery;
	QVariantList m_execArgs;

	uint m_tableId;
	int m_uniqueID;
	bool m_deleteAfterFinished;

private:
	QString m_tableName;
	QThread *m_originalThread;

	QString createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
									const bool overwrite = false) const;
};

