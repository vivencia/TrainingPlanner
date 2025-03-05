#ifndef TPDATABASETABLE_H
#define TPDATABASETABLE_H

#include "tpglobals.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <functional>

class TPDatabaseTable : public QObject
{

public:
	TPDatabaseTable(const TPDatabaseTable &other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable &other) = delete;
	TPDatabaseTable (TPDatabaseTable &&other) = delete;
	TPDatabaseTable& operator() (TPDatabaseTable &&other) = delete;

	virtual void createTable() = 0;
	virtual void updateTable() = 0;

	inline void setCallbackForDoneFunc( const std::function<void (TPDatabaseTable*)>& func ) { doneFunc = func; }

	inline uint tableID() const { return m_tableID; }
	inline uint uniqueID() const { return m_UniqueID; }
	inline void setUniqueID(const uint uid) { m_UniqueID = uid; }
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

	inline void _setQueryResult(const bool bResultOK, const std::source_location &location, const QString &message = QString())
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
	}
	#endif

protected:
	explicit inline TPDatabaseTable(QObject *parent = nullptr)
		: QObject{parent}, doneFunc{nullptr}, mb_result{false}, mb_resolved{false}, mb_waitForFinished{false} {}

	QSqlDatabase mSqlLiteDB;
	QVariantList m_execArgs;
	QString m_tableName;
	uint m_tableID;
	uint m_UniqueID;

	std::function<void (TPDatabaseTable*)> doneFunc;

private:
	bool mb_result;
	bool mb_resolved;
	bool mb_waitForFinished;
};

#endif // TPDATABASETABLE_H
