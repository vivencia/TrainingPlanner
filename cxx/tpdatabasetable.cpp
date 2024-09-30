#include "tpdatabasetable.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

void TPDatabaseTable::removeEntry()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		const QString& strQuery(u"DELETE FROM "_qs + m_tableName + u" WHERE id="_qs + m_execArgs.at(0).toString());
		m_result = query.exec(strQuery);
		if (m_result)
		{
			MSG_OUT(m_tableName << " removeEntry SUCCESS")
			MSG_OUT(strQuery)
		}
		else
		{
			MSG_OUT(m_tableName << " removeEntry Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT(m_tableName << " removeEntry Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery)
		}
		mSqlLiteDB.close();
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::clearTable()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		const QString& strQuery(u" DROP TABLE "_qs + m_tableName);
		m_result = query.exec(strQuery);
		if (m_result)
		{
			MSG_OUT(m_tableName << " clearTable SUCCESS")
			MSG_OUT(strQuery)
		}
		else
		{
			MSG_OUT(m_tableName << " clearTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT(m_tableName << " clearTable Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery)
		}
		mSqlLiteDB.close();
	}
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeDBFile()
{
	m_result = QFile::remove(mSqlLiteDB.databaseName());
	if (m_result)
	{
		createTable();
		MSG_OUT(m_tableName << " removeDBFile SUCCESS")
	}
	else
		MSG_OUT(m_tableName << " removeDBFile error: Could not remove file " << mSqlLiteDB.databaseName())
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}
