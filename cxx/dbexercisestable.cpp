#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

DBExercisesTable::DBExercisesTable(const QString& dbFilePath, DBExercisesModel* model)
	: TPDatabaseTable{}, m_model(model), m_exercisesTableLastId(1000)
{
	m_tableName = u"exercises_table"_qs;
	m_tableID = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString& cnx_name(u"db_exercises_connection"_qs + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_qs, cnx_name);
	const QString& dbname(dbFilePath + DBExercisesFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBExercisesTable::createTable()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);
		const QString& strQuery(u"CREATE TABLE IF NOT EXISTS exercises_table ("
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
									")"_qs
		);
		m_result = query.exec(strQuery);
		if (!m_result)
		{
			MSG_OUT("DBExercisesTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(strQuery)
		}
		else
			MSG_OUT("DBExercisesTable createTable SUCCESS")
		mSqlLiteDB.close();
	}
}

void DBExercisesTable::updateTable()
{

}

void DBExercisesTable::getAllExercises()
{
	mSqlLiteDB.setConnectOptions(u"QSQLITE_OPEN_READONLY"_qs);
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		query.setForwardOnly(true);
		if (query.exec(u"SELECT * FROM exercises_table"_qs))
		{
			if (query.first())
			{
				QStringList exercise_info(EXERCISES_TOTAL_COLS);
				exercise_info[EXERCISES_COL_SELECTED] = STR_ZERO;

				uint i(0);
				do
				{
					for (i = EXERCISES_COL_ID; i < EXERCISES_COL_ACTUALINDEX; ++i)
						exercise_info[i] = query.value(static_cast<int>(i)).toString();
					exercise_info[EXERCISES_COL_ACTUALINDEX] = QString::number(i);
					m_model->appendList(exercise_info);
				} while (query.next ());
				//const QModelIndex index(m_model->index(m_model->count() - 1, 0));
				const uint highest_id (m_model->_id(m_model->count() - 1));
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_model->setLastID(m_exercisesTableLastId);
				m_result = true;
			}
			else //for some reason the database table is empty. Populate it with the app provided exercises list
				updateExercisesList();
		}
		m_model->setReady(m_model->count() > 0);
		if (!m_result)
		{
			MSG_OUT("DBExercisesTable getAllExercises Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable getAllExercises Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
			MSG_OUT("DBExercisesTable getAllExercises SUCCESS")
		mSqlLiteDB.close();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		MSG_OUT("DBExercisesTable::updateExercisesList -> m_ExercisesList is empty")
		m_result = false;
		doneFunc(static_cast<TPDatabaseTable*>(this));
		return;
	}

	removePreviousListEntriesFromDB();
	m_result = false;

	if (mSqlLiteDB.open())
	{
		QStringList::const_iterator itr(m_ExercisesList.constBegin());
		const QStringList::const_iterator& itr_end(m_ExercisesList.constEnd());

		QStringList fields(3);
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		const QString& queryStart(u"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES "_qs);
		QString queryValues;

		uint idx(0);
		mSqlLiteDB.transaction();
		for (++itr; itr != itr_end; ++itr, ++idx) //++itr: Jump over version number
		{
			fields = (*itr).split(';');
			m_model->newExercise(fields.at(0), fields.at(1), fields.at(2).trimmed());
			queryValues += m_model->makeTransactionStatementForDataBase(idx);
		}
		queryValues.chop(1);
		query.exec(queryStart + queryValues);
		m_result = mSqlLiteDB.commit();
		if (!m_result)
		{
			MSG_OUT("DBExercisesTable updateExercisesList Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable updateExercisesList Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT(queryStart + queryValues);
		}
		else
		{
			emit updatedFromExercisesList();
			MSG_OUT("DBExercisesTable updateExercisesList SUCCESS")
		}
		mSqlLiteDB.close();
	}
	m_ExercisesList.clear();
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::saveExercises()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		if (mSqlLiteDB.transaction())
		{
			bool bUpdate(false);
			uint highest_id(0);
			QString strQuery;
			const QString& queryInsert(u"INSERT INTO exercises_table"
							"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
							" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0) "_qs);
			const QString& queryUpdate(u"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
							"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\', from_list=0 WHERE id=%9 "_qs);
			for (uint i(0); i < m_model->modifiedIndicesCount(); ++i)
			{
				const uint& idx(m_model->modifiedIndex(i));
				const QString& exerciseId = m_model->id(idx);
				if (m_model->_id(idx) > highest_id)
					highest_id = m_model->_id(idx);
				bUpdate = !(exerciseId.isEmpty() || exerciseId.toUInt() > m_exercisesTableLastId);
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
			query.exec(strQuery);
			m_result = mSqlLiteDB.commit();
			if (m_result)
			{
				m_exercisesTableLastId = highest_id;
				m_model->clearModifiedIndices();
				MSG_OUT("DBExercisesTable saveExercise SUCCESS");
				MSG_OUT(strQuery);
			}
			else
			{
				MSG_OUT("DBExercisesTable saveExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
				MSG_OUT("DBExercisesTable saveExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
				MSG_OUT(strQuery);
			}
			mSqlLiteDB.close();
		}
		else
			MSG_OUT("DBExercisesTable saveExercise Transaction not supported")
	}
	else
		MSG_OUT("DBExercisesTable saveExercise Could not open Database")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::removePreviousListEntriesFromDB()
{
	int ret(0);
	if (mSqlLiteDB.open())
	{
		QSqlQuery query{u"DELETE FROM exercises_table WHERE from_list=1"_qs, mSqlLiteDB};
		ret = query.exec();
		mSqlLiteDB.close();
	}
	if (!ret)
	{
		MSG_OUT("DBExercisesTable removePreviousListEntriesFromDB Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable removePreviousListEntriesFromDB Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
}

void DBExercisesTable::getExercisesList()
{
	QFile exercisesListFile(u":/extras/exerciseslist.lst"_qs);
	if (exercisesListFile.open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		char buf[512];
		qint64 lineLength;
		do
		{
			lineLength = exercisesListFile.readLine(buf, sizeof(buf));
			if (lineLength < 0) continue;
			m_ExercisesList.append(buf);
		} while (!exercisesListFile.atEnd());
		exercisesListFile.close();
	}
}
