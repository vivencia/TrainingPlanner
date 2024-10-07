#ifndef TPDATABASETABLE_H
#define TPDATABASETABLE_H

#include "tplistmodel.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <functional>
#ifndef QT_NO_DEBUG
#include <source_location>
#endif

class TPDatabaseTable : public QObject
{

public:
	TPDatabaseTable(const TPDatabaseTable& other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable& other) = delete;
	TPDatabaseTable (TPDatabaseTable&& other) = delete;
	TPDatabaseTable& operator() (TPDatabaseTable&& other) = delete;

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
	inline void addExecArg(const QVariant& arg) { m_execArgs.append(arg); }
	inline void clearExecArgs() { m_execArgs.clear(); }
	inline void changeExecArg(const QVariant& arg, const uint pos)
	{
		if (pos < m_execArgs.count())
			m_execArgs[pos] = arg;
	}

	void removeEntry();
	void clearTable();
	void removeDBFile();

	inline bool openDatabase(const bool bReadOnly = false)
	{
		if (bReadOnly)
			mSqlLiteDB.setConnectOptions(u"QSQLITE_OPEN_READONLY"_qs);
		const bool ok = mSqlLiteDB.open();
		#ifndef QT_NO_DEBUG
		if (!ok)
		{
			qDebug() << u"******FAILURE******"_qs;
			const std::source_location& location{std::source_location::current()};
			qDebug() << location.file_name() << u"::" << location.function_name() << u"::"_qs << location.line() << ' ';
			qDebug() << u"Could not open Database file: "_qs << mSqlLiteDB.databaseName();
		}
		#endif
		return ok;
	}

	inline QSqlQuery getQuery()
	{
		QSqlQuery query{mSqlLiteDB};
		if (!mSqlLiteDB.connectOptions().isEmpty())
			query.setForwardOnly(true);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);
		return query;
	}

	#ifndef QT_NO_DEBUG
	#define setResult(result, model, message, location) \
		_setResult(result, location, model, message);

	inline void _setResult(const bool bResultOK, const std::source_location& location, TPListModel* model = nullptr, const QString& message = QString())
	{
		mb_result = bResultOK;
		if (model)
			model->setReady(bResultOK);
		if (!message.isEmpty())
		{
			qDebug() << (bResultOK ? u"******SUCCESS!******"_qs : u"******FAILURE******"_qs);
			qDebug() << location.file_name() << u"::"_qs << location.function_name() << u"::"_qs << location.line();
			if (!bResultOK && !message.isEmpty())
				qDebug() << message;
		}
		mSqlLiteDB.close();
	}
	#else
	#define setResult(result, model, message, location) \
		_setResult(result, model);
	inline void _setResult(const bool bResultOK, TPListModel* model = nullptr)
	{
		mb_result = bResultOK;
		if (model)
			model->setReady(bResultOK);
	}
	#endif

protected:
	explicit inline TPDatabaseTable(QObject* parent = nullptr)
		: QObject{parent}, doneFunc(nullptr), mb_result(false), mb_resolved(false), mb_waitForFinished(false) {}

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
