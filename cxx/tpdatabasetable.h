#ifndef TPDATABASETABLE_H
#define TPDATABASETABLE_H

#include "tplistmodel.h"

#include <QObject>
#include <QStringList>
#include <QSqlDatabase>
#include <QSettings>

#include <functional>

typedef enum {
	OP_NULL = 0, OP_ADD = 1, OP_EDIT = 2, OP_DEL = 3, OP_READ = 4, OP_UPDATE_LIST = 5, OP_DELETE_TABLE = 6
} OP_CODES;

class TPDatabaseTable : public QObject
{

public:
	explicit TPDatabaseTable(QSettings* appSettings, TPListModel* model)
		: QObject{nullptr}, m_appSettings(appSettings), m_model(model), m_result(false), mb_waitForFinished(false), m_opcode(OP_NULL) {}

	virtual void createTable() = 0;
	virtual void updateDatabase() = 0;

	inline const QStringList& data () const { return m_data; }
	inline OP_CODES opCode() const { return m_opcode; }
	inline bool result() const { return m_result; }

	inline void setCallbackForDoneFunc( const std::function<void (TPDatabaseTable*)>& func ) { doneFunc = func; }

	inline uint uniqueID() const { return m_UniqueID; }
	inline void setUniqueID(const uint uid) { m_UniqueID = uid; }
	inline void addExecArg(const QVariant& arg) { m_execArgs.append(arg); }
	inline void clearExecArgs() { m_execArgs.clear(); }
	inline void changeExecArg(const QVariant& arg, const uint pos) { if (pos < m_execArgs.count()) m_execArgs[pos] = arg; }
	inline TPListModel* model() const { return m_model; }
	void setModel(TPListModel* model) { m_model = model; }

	inline void setWaitForThreadToFinish(const bool wait) { mb_waitForFinished = wait; }
	inline bool waitForThreadToFinish() const { return mb_waitForFinished; }

	void removeEntry();
	void clearTable();
	void removeDBFile();

protected:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;
	QStringList m_data;
	TPListModel* m_model;
	QVariantList m_execArgs;
	QString m_tableName;
	uint m_UniqueID;
	bool m_result;
	bool mb_waitForFinished;
	OP_CODES m_opcode;

	std::function<void (TPDatabaseTable*)> doneFunc;
};

#endif // TPDATABASETABLE_H
