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
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
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
		const bool ok = query.exec(strQuery);
		setResult(ok, nullptr, strQuery, SOURCE_LOCATION);
	}
}

void DBExercisesTable::updateTable()
{

}

void DBExercisesTable::getAllExercises()
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT * FROM exercises_table"_qs);
		if (query.exec(strQuery))
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
				const uint highest_id (m_model->_id(m_model->count() - 1));
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_model->setLastID(m_exercisesTableLastId);
				ok = true;
			}
			else //for some reason the database table is empty. Populate it with the app provided exercises list
			{
				mSqlLiteDB.close();
				updateExercisesList();
			}
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		setResult(false, m_model, u"DBExercisesTable::updateExercisesList -> m_ExercisesList is empty"_qs, SOURCE_LOCATION);
		doneFunc(static_cast<TPDatabaseTable*>(this));
		return;
	}

	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		QString queryValues;
		QStringList fields(3);
		uint idx(0);

		//remove previous list entries from DB
		const QString& strQuery(u"DELETE FROM exercises_table WHERE from_list=1"_qs);
		ok = query.exec(strQuery);
		if (!ok)
		{
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE(query.lastError().text(), strQuery);
		}
		query.finish();

		const QString& queryStart(u"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES "_qs);

		QStringList::const_iterator itr(m_ExercisesList.constBegin());
		const QStringList::const_iterator& itr_end(m_ExercisesList.constEnd());
		for (++itr; itr != itr_end; ++itr, ++idx) //++itr: Jump over version number
		{
			fields = (*itr).split(';');
			m_model->newExercise(fields.at(0), fields.at(1), fields.at(2).trimmed());
			queryValues += m_model->makeTransactionStatementForDataBase(idx);
		}
		queryValues.chop(1);
		mSqlLiteDB.transaction();
		ok = query.exec(queryStart + queryValues);
		if (!ok)
		{
			mSqlLiteDB.rollback();
			DEFINE_SOURCE_LOCATION
			ERROR_MESSAGE(query.lastError().text(), QString())
		}
		else
			mSqlLiteDB.commit();
		setResult(ok, m_model, queryStart + queryValues, SOURCE_LOCATION);
		if (ok)
			emit updatedFromExercisesList();
	}
	m_ExercisesList.clear();
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::saveExercises()
{
	bool ok(false);
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
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
		ok = mSqlLiteDB.commit();
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
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
