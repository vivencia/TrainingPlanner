#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "tpglobals.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

uint DBExercisesTable::m_exercisesTableLastId(1000);

DBExercisesTable::DBExercisesTable(const QString& dbFilePath, DBExercisesModel* model)
	: TPDatabaseTable(static_cast<TPListModel*>(model))
{
	m_tableName = u"exercises_table"_qs;
	m_tableID = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);
	m_UniqueID = QTime::currentTime().msecsSinceStartOfDay();
	const QString cnx_name(u"db_exercises_connection"_qs + QString::number(m_UniqueID));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_qs, cnx_name);
	const QString dbname(dbFilePath + DBExercisesFileName);
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
		const QString strQuery(u"CREATE TABLE IF NOT EXISTS exercises_table ("
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

void DBExercisesTable::getAllExercises()
{
	mSqlLiteDB.setConnectOptions(u"QSQLITE_OPEN_READONLY"_qs);
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly(true);
		if (query.exec(u"SELECT * FROM exercises_table"_qs))
		{
			if (query.first ())
			{
				QStringList exercise_info;
				uint i(0);
				uint nExercises(0);

				do
				{
					for (i = EXERCISES_COL_ID; i < EXERCISES_COL_ACTUALINDEX; ++i)
						exercise_info.append(query.value(static_cast<int>(i)).toString());
					exercise_info.append(QString::number(nExercises++)); //EXERCISES_COL_ACTUALINDEX
					exercise_info.append(u"0"_qs); //EXERCISES_COL_SELECTED
					m_model->appendList(exercise_info);
					exercise_info.clear();
				} while ( query.next () );
				QModelIndex index(m_model->index(m_model->count() - 1, 0));
				const uint highest_id (static_cast<DBExercisesModel*>(m_model)->data(index, DBExercisesModel::exerciseIdRole).toUInt());
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				static_cast<DBExercisesModel*>(m_model)->setLastID(m_exercisesTableLastId);
				m_result = true;
			}
		}
		else //for some reason the database table is empty. Populate it with the app provided exercises list
			updateExercisesList();
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
		MSG_OUT("DBExercisesTable updateExercisesList m_ExercisesList is empty")
		m_result = false;
		doneFunc(static_cast<TPDatabaseTable*>(this));
		return;
	}

	removePreviousListEntriesFromDB();
	m_result = false;

	if (mSqlLiteDB.open())
	{
		QStringList::const_iterator itr(m_ExercisesList.constBegin());
		const QStringList::const_iterator itr_end(m_ExercisesList.constEnd());

		QStringList fields;
		QSqlQuery query(mSqlLiteDB);
		query.exec(u"PRAGMA page_size = 4096"_qs);
		query.exec(u"PRAGMA cache_size = 16384"_qs);
		query.exec(u"PRAGMA temp_store = MEMORY"_qs);
		query.exec(u"PRAGMA journal_mode = OFF"_qs);
		query.exec(u"PRAGMA locking_mode = EXCLUSIVE"_qs);
		query.exec(u"PRAGMA synchronous = 0"_qs);

		const QString query_cmd(u"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES(%1, \'%2\', \'%3\', \'%4\', 4, 12, 20, \'%5\', \'qrc:/images/no_image.jpg\', 1)"_qs);

		uint idx ( 0 );
		mSqlLiteDB.transaction();
		if (!m_model)
		{
			for ( ++itr; itr != itr_end; ++itr, ++idx ) //++itr: Jump over version number
			{
				fields = static_cast<QString>(*itr).split(';');
				query.exec(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), u"(kg)"_qs));
			}
		}
		else
		{
			for ( ++itr; itr != itr_end; ++itr, ++idx ) //++itr: Jump over version number
			{
				fields = static_cast<QString>(*itr).split(';');
				query.exec(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), u"(kg)"_qs));
				m_model->appendList(QStringList()
								<< QString::number(idx) << fields.at(0) << fields.at(1) << fields.at(2).trimmed() << u"4"_qs << u"12"_qs << u"20"_qs
								<< u"(kg)"_qs << u"qrc:/images/no_image.jpg"_qs << STR_ONE << QString::number(idx) << STR_ZERO );
			}
		}
		mSqlLiteDB.commit();
		m_result = mSqlLiteDB.lastError().databaseText().isEmpty();
		if (!m_result)
		{
			MSG_OUT("DBExercisesTable updateExercisesList Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable updateExercisesList Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
		{
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
		DBExercisesModel* model(static_cast<DBExercisesModel*>(m_model));

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
			QString strQuery;
			const QString& queryInsert(u"INSERT INTO exercises_table"
							"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
							" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0) "_qs);
			const QString& queryUpdate(u"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
							"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\', from_list=0 WHERE id=%9 "_qs);
			for (uint i(0); i < model->modifiedIndicesCount(); ++i)
			{
				const uint& idx(model->modifiedIndex(i));
				const QString& exerciseId = model->getFast(idx, EXERCISES_COL_ID);
				bUpdate = !(exerciseId.isEmpty() || exerciseId.toUInt() > m_exercisesTableLastId);
				if (bUpdate)
				{
					strQuery += queryUpdate.arg(model->getFast(idx, EXERCISES_COL_MAINNAME),
							model->getFast(idx, EXERCISES_COL_SUBNAME), model->getFast(idx, EXERCISES_COL_MUSCULARGROUP),
							model->getFast(idx, EXERCISES_COL_SETSNUMBER), model->getFast(idx, EXERCISES_COL_REPSNUMBER),
							model->getFast(idx, EXERCISES_COL_WEIGHT), model->getFast(idx, EXERCISES_COL_WEIGHTUNIT),
							model->getFast(idx, EXERCISES_COL_MEDIAPATH), exerciseId);
				}
				else
				{
					if (exerciseId.isEmpty()) //When adding a new exercise. When importing, those fields come already filled
					{
						model->setFast(idx, EXERCISES_COL_ID, QString::number(model->lastID()+1));
						model->setFast(idx, EXERCISES_COL_FROMAPPLIST, STR_ZERO);
						model->setFast(idx, EXERCISES_COL_ACTUALINDEX, QString::number(idx));
						model->setFast(idx, EXERCISES_COL_SELECTED, STR_ZERO);
						model->setLastID(model->lastID()+1);
					}
					strQuery += queryInsert.arg(exerciseId, model->getFast(idx, EXERCISES_COL_MAINNAME),
							model->getFast(idx, EXERCISES_COL_SUBNAME), model->getFast(idx, EXERCISES_COL_MUSCULARGROUP),
							model->getFast(idx, EXERCISES_COL_SETSNUMBER), model->getFast(idx, EXERCISES_COL_REPSNUMBER),
							model->getFast(idx, EXERCISES_COL_WEIGHT), model->getFast(idx, EXERCISES_COL_WEIGHTUNIT),
							model->getFast(idx, EXERCISES_COL_MEDIAPATH));
				}
			}
			query.exec(strQuery);
			m_result = mSqlLiteDB.commit();
			if (m_result)
			{
				model->setModified(false);
				model->clearModifiedIndices();
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
		QSqlQuery query(QStringLiteral("DELETE FROM exercises_table WHERE from_list=1"), mSqlLiteDB);
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
		char buf[1024];
		qint64 lineLength;
		do
		{
			lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
			if (lineLength < 0) continue;
			m_ExercisesList.append(buf);
		} while (!exercisesListFile.atEnd());
		exercisesListFile.close();
	}
}
