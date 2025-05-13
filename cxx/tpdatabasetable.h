#pragma once

#include "tpglobals.h"
#include "tputils.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <functional>

constexpr short APP_TABLES_NUMBER{6};
constexpr short EXERCISES_TABLE_ID{0x0001};
constexpr short MESOCYCLES_TABLE_ID{0x0002};
constexpr short MESOSPLIT_TABLE_ID{0x0003};
constexpr short MESOCALENDAR_TABLE_ID{0x0004};
constexpr short WORKOUT_TABLE_ID{0x0005};
constexpr short USERS_TABLE_ID{0x0006};

constexpr QLatin1StringView tablesNames[APP_TABLES_NUMBER] = { "Database/Users.db.sqlite"_L1,
									"Database/ExercisesList.db.sqlite"_L1,
									"Database/Mesocycles.db.sqlite"_L1,
									"Database/MesocyclesSplits.db.sqlite"_L1,
									"Database/MesoCalendar.db.sqlite"_L1,
									"Database/Workouts.db.sqlite"_L1
								};

class TPDatabaseTable : public QObject
{

public:
	TPDatabaseTable(const TPDatabaseTable &other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable &other) = delete;
	TPDatabaseTable (TPDatabaseTable &&other) = delete;
	TPDatabaseTable& operator() (TPDatabaseTable &&other) = delete;

	virtual void createTable() = 0;
	virtual void updateTable() = 0;
	static TPDatabaseTable *createDBTable(const uint table_id, const bool auto_delete = true);

	inline void setCallbackForDoneFunc( const std::function<void (TPDatabaseTable*)>& func ) { doneFunc = func; }

	static inline QString dbFilePath(const uint table_id) { return appUtils()->localAppFilesDir() + tablesNames[table_id]; }
	inline short tableId() const { return m_tableId; }
	inline uint uniqueId() const { return m_UniqueID; }
	inline void setUniqueId(const uint uid) { m_UniqueID = uid; }
	inline bool resolved() const { return mb_resolved; }
	inline void setResolved(const bool resolved) { mb_resolved = resolved; }
	inline void setWaitForThreadToFinish(const bool wait) { mb_waitForFinished = wait; }
	inline bool waitForThreadToFinish() const { return mb_waitForFinished; }
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

	inline bool openDatabase(const bool bReadOnly = false)
	{
		if (bReadOnly)
			mSqlLiteDB.setConnectOptions("QSQLITE_OPEN_READONLY"_L1);
		const bool ok{mSqlLiteDB.open()};
		#ifndef QT_NO_DEBUG
		if (!ok)
		{
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE("Could not open Database file: "_L1, mSqlLiteDB.databaseName())
		}
		#endif
		return ok;
	}

	inline QSqlQuery getQuery()
	{
		QSqlQuery query{mSqlLiteDB};
		if (!mSqlLiteDB.connectOptions().isEmpty())
			query.setForwardOnly(true);
		static_cast<void>(query.exec("PRAGMA page_size = 4096"_L1));
		static_cast<void>(query.exec("PRAGMA cache_size = 16384"_L1));
		static_cast<void>(query.exec("PRAGMA temp_store = MEMORY"_L1));
		static_cast<void>(query.exec("PRAGMA journal_mode = OFF"_L1));
		static_cast<void>(query.exec("PRAGMA locking_mode = EXCLUSIVE"_L1));
		static_cast<void>(query.exec("PRAGMA synchronous = 0"_L1));
		return query;
	}

	#ifndef QT_NO_DEBUG
	#define setQueryResult(result, message, location) \
		_setQueryResult(result, location, message)

	inline void _setQueryResult(const bool bResultOK, const std::source_location &location, const QString &message = QString{})
	{
		mb_result = bResultOK;
		if (!message.isEmpty())
		{
			if (bResultOK)
				SUCCESS_MESSAGE_WITH_STATEMENT(PRINT_SOURCE_LOCATION)
			else
				ERROR_MESSAGE(message, "")
		}
		if (mSqlLiteDB.connectOptions().isEmpty()) //optimize after modifying the database
		{
			QSqlQuery query{mSqlLiteDB};
			static_cast<void>(query.exec("VACUUM"_L1));
			static_cast<void>(query.exec("PRAGMA optimize"_L1));
		}
		mSqlLiteDB.close();
	}
	#else
	#define setQueryResult(result, message, location) \
		_setQueryResult(result)
	inline void _setQueryResult(const bool bResultOK)
	{
		mb_result = bResultOK;
		if (mSqlLiteDB.connectOptions().isEmpty()) //optimize after modifying the database
		{
			QSqlQuery query{mSqlLiteDB};
			static_cast<void>(query.exec("VACUUM"_L1));
			static_cast<void>(query.exec("PRAGMA optimize"_L1));
		}
		mSqlLiteDB.close();
	}
	#endif

protected:
	explicit inline TPDatabaseTable(const short table_id, QObject *parent = nullptr)
		: QObject{parent}, m_tableId{table_id}, doneFunc{nullptr}, mb_result{false}, mb_resolved{false}, mb_waitForFinished{false} {}

	QSqlDatabase mSqlLiteDB;
	QVariantList m_execArgs;
	QString m_tableName;

	short m_tableId;
	uint m_UniqueID;

	std::function<void (TPDatabaseTable*)> doneFunc;

private:
	bool mb_result;
	bool mb_resolved;
	bool mb_waitForFinished;
};

