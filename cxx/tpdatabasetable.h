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
		: QObject{nullptr}, m_appSettings(appSettings), m_model(model), m_result(false), m_opcode(OP_NULL) {}

	virtual void createTable() = 0;
	inline const QStringList& data () const { return m_data; }
	inline OP_CODES opCode() const { return m_opcode; }
	inline bool result() const { return m_result; }

	inline void setCallbackForExecFunc( const std::function<void (void)>& func ) { execFunc = func; }
	inline void setCallbackForDoneFunc( const std::function<void (TPDatabaseTable*)>& func ) { doneFunc = func; }

	inline void execFunction() { execFunc(); }

	inline void addExecArg(const QVariant& arg) { m_execArgs.append(arg); }
	inline TPListModel* model() const { return m_model; }

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

	bool m_result;
	OP_CODES m_opcode;

	std::function<void (void)> execFunc;
	std::function<void (TPDatabaseTable*)> doneFunc;
};

#endif // TPDATABASETABLE_H
