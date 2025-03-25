#include "dbexercisestable.h"

#include "dbexercisesmodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBExercisesTable::DBExercisesTable(const QString &dbFilePath, DBExercisesModel *model)
	: TPDatabaseTable{}, m_model(model), m_exercisesTableLastId(1000)
{
	m_tableName = std::move("exercises_table"_L1);
	m_tableID = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);
	m_UniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_exercises_connection"_L1 + QString::number(m_UniqueID)};
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name);
	const QString &dbname(dbFilePath + DBExercisesFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBExercisesTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString &strQuery{"CREATE TABLE IF NOT EXISTS exercises_table ("
										"id INTEGER PRIMARY KEY,"
										"primary_name TEXT,"
										"secondary_name TEXT,"
										"muscular_group TEXT,"
										"sets TEXT,"
										"reps TEXT,"
										"weight TEXT,"
										"weight_unit TEXT,"
										"media_path TEXT,"
										"from_list INTEGER"
									")"_L1
		};
		const bool ok = query.exec(strQuery);
		setQueryResult(ok, strQuery, SOURCE_LOCATION);
	}
}

void DBExercisesTable::updateTable()
{

}

void DBExercisesTable::getAllExercises()
{
	if (openDatabase(true))
	{
		bool ok{false};
		QSqlQuery query{getQuery()};
		const QString &strQuery{"SELECT * FROM exercises_table ORDER BY ROWID"_L1};
		if (query.exec(strQuery))
		{
			if (query.first())
			{
				do
				{
					m_model->appendList(std::move(QStringList(EXERCISES_TOTAL_COLS)));
					for (uint i(EXERCISES_COL_ID); i < EXERCISES_COL_ACTUALINDEX; ++i)
						m_model->lastRow()[i] = std::move(query.value(static_cast<int>(i)).toString());
					m_model->lastRow()[EXERCISES_COL_ACTUALINDEX] = std::move(QString::number(m_model->count()));
					m_model->lastRow()[EXERCISES_COL_SELECTED] = STR_ZERO;
				} while (query.next ());
				const uint highest_id (m_model->_id(m_model->count() - 1));
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_model->setLastID(m_exercisesTableLastId);
				ok = true;
			}
			else //for whatever reason the database table is empty. Populate it with the app provided exercises list
			{
				mSqlLiteDB.close();
				updateExercisesList();
			}
			m_model->setReady(true);
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
		QSqlQuery query{getQuery()};
		QString queryValues;
		uint idx{0};

		//remove previous list entries from DB
		const QString &strQuery{"DELETE FROM exercises_table WHERE from_list=1"_L1};
		ok = query.exec(strQuery);
		if (!ok)
		{
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE(query.lastError().text(), strQuery);
		}
		query.finish();

		const QString &queryStart{"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES "_L1};

		QStringList::const_iterator itr{m_ExercisesList.constBegin()};
		const QStringList::const_iterator &itr_end{m_ExercisesList.constEnd()};
		for (++itr; itr != itr_end; ++itr, ++idx) //++itr: Jump over version number
		{
			const QStringList &fields{(*itr).split(';')};
			m_model->newExercise(fields.at(0), fields.at(1), fields.at(2).trimmed());
			queryValues += std::move(m_model->makeTransactionStatementForDataBase(idx));
		}
		queryValues.chop(1);
		static_cast<void>(mSqlLiteDB.transaction());
		ok = query.exec(queryStart + queryValues);
		if (!ok)
		{
			static_cast<void>(mSqlLiteDB.rollback());
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE(query.lastError().text(), QString())
		}
		else
			static_cast<void>(mSqlLiteDB.commit());
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
		QSqlQuery query{getQuery()};
		uint highest_id{0};
		QString strQuery;

		const QString &queryInsert{"INSERT INTO exercises_table"
							"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
							" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0) "_L1};
		const QString &queryUpdate{"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
							"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\', from_list=0 WHERE id=%9 "_L1};
		for (uint i(0); i < m_model->modifiedIndicesCount(); ++i)
		{
			const uint &idx{m_model->modifiedIndex(i)};
			const QString &exerciseId = m_model->id(idx);
			if (m_model->_id(idx) > highest_id)
				highest_id = m_model->_id(idx);
			const bool bUpdate = !(exerciseId.isEmpty() || exerciseId.toUInt() > m_exercisesTableLastId);
			if (bUpdate)
			{
				strQuery += queryUpdate.arg(m_model->mainName(idx), m_model->subName(idx), m_model->muscularGroup(idx),
							m_model->setsNumber(idx), m_model->repsNumber(idx), m_model->weight(idx), m_model->weightUnit(idx),
							m_model->mediaPath(idx), exerciseId);
			}
			else
			{
				strQuery += queryInsert.arg(exerciseId, m_model->mainName(idx), m_model->subName(idx), m_model->muscularGroup(idx),
							m_model->setsNumber(idx), m_model->repsNumber(idx), m_model->weight(idx), m_model->weightUnit(idx),
							m_model->mediaPath(idx));
			}
		}
		static_cast<void>(mSqlLiteDB.commit());
		const bool ok{query.exec(strQuery)};
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
		do
		{
			if (exercisesListFile.readLine(buf, sizeof(buf)) < 0)
				continue;
			m_ExercisesList.append(buf);
		} while (!exercisesListFile.atEnd());
		exercisesListFile.close();
	}
}
