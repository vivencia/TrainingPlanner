#pragma once

#include "tpglobals.h"
#include "tputils.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <functional>

constexpr uint APP_TABLES_NUMBER{6};
constexpr uint EXERCISES_TABLE_ID{0x0001};
constexpr uint MESOCYCLES_TABLE_ID{0x0002};
constexpr uint MESOSPLIT_TABLE_ID{0x0003};
constexpr uint MESOCALENDAR_TABLE_ID{0x0004};
constexpr uint WORKOUT_TABLE_ID{0x0005};
constexpr uint USERS_TABLE_ID{0x0006};

constexpr QLatin1StringView tablesNames[APP_TABLES_NUMBER+1] = { ""_L1,
									"Database/ExercisesList.db.sqlite"_L1,
									"Database/Mesocycles.db.sqlite"_L1,
									"Database/MesocyclesSplits.db.sqlite"_L1,
									"Database/MesoCalendar.db.sqlite"_L1,
									"Database/Workouts.db.sqlite"_L1,
									"Database/Users.db.sqlite"_L1
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

	inline void setCallbackForDoneFunc( const std::function<void (TPDatabaseTable*)> &func ) { doneFunc = func; }

	static inline QString dbFilePath(const uint table_id) { return appUtils()->localAppFilesDir() + tablesNames[table_id]; }
	inline uint tableId() const { return m_tableId; }
	inline int uniqueId() const { return m_UniqueID; }
	inline void setUniqueId(const int uid) { m_UniqueID = uid; }
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

	bool openDatabase(const bool bReadOnly = false);
	QSqlQuery getQuery() const;

	#ifndef QT_NO_DEBUG
	#define setQueryResult(result, message, location) \
		_setQueryResult(result, location, message)

	void _setQueryResult(const bool bResultOK, const std::source_location &location, const QString &message = QString{});
	#else
	#define setQueryResult(result, message, location) \
		_setQueryResult(result)
	void _setQueryResult(const bool bResultOK);
	#endif

protected:
	explicit inline TPDatabaseTable(const uint table_id, QObject *parent = nullptr)
		: QObject{parent}, m_tableId{table_id}, doneFunc{nullptr}, mb_result{false}, mb_resolved{false}, mb_waitForFinished{false} {}

	QSqlDatabase mSqlLiteDB;
	QVariantList m_execArgs;
	QString m_tableName;

	uint m_tableId;
	int m_UniqueID;

	std::function<void (TPDatabaseTable*)> doneFunc;

private:
	bool mb_result;
	bool mb_resolved;
	bool mb_waitForFinished;
};

