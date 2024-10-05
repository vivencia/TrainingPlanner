#ifndef TPDATABASETABLE_H
#define TPDATABASETABLE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>

#include <functional>

class TPDatabaseTable : public QObject
{

public:
	TPDatabaseTable(const TPDatabaseTable& other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable& other) = delete;
	TPDatabaseTable& operator() (const TPDatabaseTable other) = delete;

	virtual void createTable() = 0;
	virtual void updateTable() = 0;

	inline bool result() const { return m_result; }
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

protected:
	explicit inline TPDatabaseTable(QObject* parent = nullptr)
		: QObject{parent}, m_result(false), doneFunc(nullptr), mb_resolved(false), mb_waitForFinished(false) {}

	QSqlDatabase mSqlLiteDB;
	QVariantList m_execArgs;
	QString m_tableName;
	uint m_tableID;
	uint m_UniqueID;
	bool m_result;

	std::function<void (TPDatabaseTable*)> doneFunc;

private:
	bool mb_resolved;
	bool mb_waitForFinished;
};

#endif // TPDATABASETABLE_H
