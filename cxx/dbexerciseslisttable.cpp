#include "dbexerciseslisttable.h"

#include "dbexerciseslistmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBExercisesTable::DBExercisesTable(DBExercisesListModel *model)
	: TPDatabaseTable{EXERCISES_TABLE_ID}, m_model{model}, m_exercisesTableLastId{1000}
{
	m_tableName = std::move("exercises_table"_L1);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_exercises_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	mSqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName("ExercisesListTable");
	#endif
}

void DBExercisesTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS %1 ("
										"id INTEGER PRIMARY KEY,"
										"primary_name TEXT,"
										"secondary_name TEXT,"
										"muscular_group TEXT,"
										"media_path TEXT,"
										"from_list INTEGER"
									")"_L1.arg(m_tableName)
		};
		const bool ok{query.exec(strQuery)};
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBExercisesTable::getAllExercises()
{
	if (openDatabase(true))
	{
		bool ok{false};
		QSqlQuery query{std::move(getQuery())};
		const QString &strQuery{"SELECT * FROM %1 ORDER BY ROWID"_L1.arg(m_tableName)};
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do
				{
					QStringList data{EXERCISES_TOTAL_COLS};
					for (uint i{EXERCISES_LIST_COL_ID}; i < EXERCISES_LIST_COL_ACTUALINDEX; ++i)
						data[i] = std::move(query.value(static_cast<int>(i)).toString());
					data[EXERCISES_LIST_COL_ACTUALINDEX] = std::move(QString::number(m_model->count()));
					data[EXERCISES_LIST_COL_SELECTED] = STR_ZERO;
					m_model->appendList(std::move(data));
				} while (query.next ());
				const uint highest_id{static_cast<uint>(m_model->_id(m_model->count() - 1))};
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_model->setLastID(m_exercisesTableLastId);
				ok = true;
			}
			else //for whatever reason the database table is empty. Populate it with the app provided exercises list
			{
				query.finish();
				mSqlLiteDB.close();
				updateExercisesList();
			}
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		setQueryResult(false, "DBExercisesTable::updateExercisesList -> m_ExercisesList is empty"_L1, SOURCE_LOCATION);
		doneFunc(static_cast<TPDatabaseTable*>(this));
		return;
	}

	if (openDatabase())
	{
		bool ok{false};
		QSqlQuery query{std::move(getQuery())};
		QString queryValues;

		//remove previous list entries from DB
		const QString &strQuery{"DELETE FROM %1 WHERE from_list=1"_L1.arg(m_tableName)};
		ok = query.exec(strQuery);
		if (!ok)
		{
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE(query.lastError().text(), strQuery);
		}
		query.finish();

		const QString &queryStart{u"INSERT INTO %1 "
								"(id,primary_name,secondary_name,muscular_group,media_path,from_list) VALUES "_s.arg(m_tableName)};

		uint idx{0};
		for (const auto &data : std::as_const(m_ExercisesList))
		{
			const QStringList &fields{data.split(';')};
			m_model->newExercise(fields.at(0), fields.at(1), fields.at(2).trimmed());
			queryValues += std::move(m_model->makeTransactionStatementForDataBase(idx));
			++idx;
		}
		queryValues.chop(1);
		if ((ok = mSqlLiteDB.transaction()))
		{
			ok = query.exec(queryStart + queryValues);
			if (!ok)
			{
				static_cast<void>(mSqlLiteDB.rollback());
				DEFINE_SOURCE_LOCATION
				ERROR_MESSAGE(query.lastError().text(), QString{})
			}
			else
				ok = mSqlLiteDB.commit();
		}
		setQueryResult(ok, queryStart + queryValues, SOURCE_LOCATION);
		if (ok)
			emit updatedFromExercisesList();
	}
	m_ExercisesList.clear();
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::saveExercises()
{
	if (openDatabase())
	{
		QSqlQuery query{std::move(getQuery())};
		uint highest_id{0};
		QString strQuery;

		const QString &queryInsert{u"INSERT INTO %1"
							"(id,primary_name,secondary_name,muscular_group,media_path,from_list)"
							" VALUES(%2, \'%3\', \'%4\', \'%5\', \'%6\', 0) "_s};
		const QString &queryUpdate{u"UPDATE %1 SET primary_name=\'%2\', secondary_name=\'%3\', muscular_group=\'%4\', "
							"media_path=\'%5\', from_list=0 WHERE id=%6 "_s};
		for (uint i{0}; i < m_model->modifiedIndicesCount(); ++i)
		{
			const uint &idx{m_model->modifiedIndex(i)};
			const QString &exerciseId{m_model->id(idx)};
			if (m_model->_id(idx) > highest_id)
				highest_id = m_model->_id(idx);
			const bool bUpdate{!(exerciseId.isEmpty() || exerciseId.toUInt() > m_exercisesTableLastId)};
			if (bUpdate)
			{
				strQuery += std::move(queryUpdate.arg(m_tableName, m_model->mainName(idx), m_model->subName(idx),
							m_model->muscularGroup(idx), m_model->mediaPath(idx), exerciseId));
			}
			else
			{
				strQuery += std::move(queryInsert.arg(m_tableName, exerciseId, m_model->mainName(idx),
							m_model->subName(idx), m_model->muscularGroup(idx), m_model->mediaPath(idx)));
			}
		}
		bool ok{mSqlLiteDB.transaction()};
		if (ok)
		{
			ok = query.exec(strQuery);
			if (ok)
				ok = mSqlLiteDB.commit();
		}
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
		if (ok)
		{
			m_exercisesTableLastId = highest_id;
			m_model->clearModifiedIndices();
		}
	}	
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::getExercisesList()
{
	QFile exercisesListFile{":/extras/exerciseslist.lst"_L1};
	if (exercisesListFile.open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		char buf[512];
		if (exercisesListFile.readLine(buf, sizeof(buf)) < 0) //read version
			return;
		do
		{
			if (exercisesListFile.readLine(buf, sizeof(buf)) < 0)
				continue;
			m_ExercisesList.append(buf);
		} while (!exercisesListFile.atEnd());
		exercisesListFile.close();
	}
}
