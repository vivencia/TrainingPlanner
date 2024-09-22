#ifndef TPDATABASETABLE_H
#define TPDATABASETABLE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>

#include <functional>

class TPListModel;

class TPDatabaseTable : public QObject
{

public:
	explicit inline TPDatabaseTable(TPListModel* model)
		: QObject{nullptr}, m_model(model), mb_resolved(false), m_result(false),
			mb_waitForFinished(false), doneFunc(nullptr) {}

	virtual void createTable() = 0;
	virtual void updateDatabase() = 0;

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

	inline TPListModel* model() const { return m_model; }
	inline void setModel(TPListModel* model) { m_model = model; }

	void removeEntry();
	void clearTable();
	void removeDBFile();

protected:
	QSqlDatabase mSqlLiteDB;
	TPListModel* m_model;
	QVariantList m_execArgs;
	QString m_tableName;
	uint m_tableID;
	uint m_UniqueID;
	bool mb_resolved;
	bool m_result;
	bool mb_waitForFinished;

	std::function<void (TPDatabaseTable*)> doneFunc;
};

#endif // TPDATABASETABLE_H
