#include "tpdatabasetable.h"

#include <QSqlQuery>
#include <QSqlError>

void TPDatabaseTable::removeEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM ") + m_tableName + QStringLiteral("WHERE id=") + m_data.at(0) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		m_opcode = OP_DEL;
		if (m_model)
			m_model->removeFromList(m_model->currentRow());
		MSG_OUT(m_tableName << " removeEntry SUCCESS")
	}
	else
	{
		MSG_OUT(m_tableName << " removeEntry Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT(m_tableName << " removeEntry Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::clearTable()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare(QStringLiteral(" DROP TABLE ") + m_tableName);
		m_result = query.exec();
		mSqlLiteDB.close();
	}
	if (m_result)
	{
		MSG_OUT(m_tableName << " clearTable SUCCESS")
	}
	else
	{
		MSG_OUT(m_tableName << " clearTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT(m_tableName << " clearTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
}

void TPDatabaseTable::removeDBFile()
{
	QFile mDBFile(mSqlLiteDB.databaseName());
	m_result = mDBFile.remove();
	if (m_result)
	{
		if (m_model)
			m_model->clear();
		m_opcode = OP_DELETE_TABLE;
		MSG_OUT(m_tableName << " removeDBFile SUCCESS")
	}
	else
		MSG_OUT(m_tableName << " removeDBFile error: Could not remove file " << mDBFile.fileName())
	doneFunc(static_cast<TPDatabaseTable*>(this));
}
