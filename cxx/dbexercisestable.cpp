#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "tputils.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
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
	m_data.reserve(EXERCISES_COL_SELECTED+1);
	for(uint i(EXERCISES_COL_ID); i <= EXERCISES_COL_SELECTED; i++)
		m_data.append(QString());
}

void DBExercisesTable::createTable()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));
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
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		const QString strWeightUnit(appSettings()->value("weightUnit").toString());
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
				query.exec(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), strWeightUnit));
			}
		}
		else
		{
			for ( ++itr; itr != itr_end; ++itr, ++idx ) //++itr: Jump over version number
			{
				fields = static_cast<QString>(*itr).split(';');
				query.exec(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), strWeightUnit));
				m_model->appendList(QStringList()
								<< QString::number(idx) << fields.at(0) << fields.at(1) << fields.at(2).trimmed() << u"4"_qs << u"12"_qs << u"20"_qs
								<< strWeightUnit << u"qrc:/images/no_image.jpg"_qs << STR_ONE << QString::number(idx) << STR_ZERO );
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

void DBExercisesTable::updateFromModel()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		TPListModel* model(m_execArgs.at(1).value<TPListModel*>());
		static_cast<DBExercisesModel*>(m_model)->updateFromModel(model);

		const QString strWeightUnit(appSettings()->value("weightUnit").toString());
		const QString query_cmd( QStringLiteral(
								"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\' \'%7\', \'%8\', \'%9\', 0)") );

		mSqlLiteDB.transaction();
		uint idx(0);
		for ( uint i(0); i < m_model->modifiedIndicesCount(); ++i)
		{
			idx = m_model->modifiedIndex(i);
			query.exec(query_cmd.arg(m_model->getFast(idx, EXERCISES_COL_ID).arg(m_model->getFast(idx, EXERCISES_COL_MAINNAME),
							m_model->getFast(idx, EXERCISES_COL_SUBNAME), m_model->getFast(idx, EXERCISES_COL_MUSCULARGROUP),
							m_model->getFast(idx, EXERCISES_COL_SETSNUMBER), m_model->getFast(idx, EXERCISES_COL_REPSNUMBER),
							m_model->getFast(idx, EXERCISES_COL_WEIGHT), strWeightUnit, m_model->getFast(idx, EXERCISES_COL_MEDIAPATH))));
		}
		static_cast<DBExercisesModel*>(m_model)->setLastID(idx);
		mSqlLiteDB.commit();
		m_result = mSqlLiteDB.lastError().databaseText().isEmpty();
		if (!m_result)
		{
			MSG_OUT("DBExercisesTable updateExercisesListFromModel Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable updateExercisesListFromModel Driver error:  " << mSqlLiteDB.lastError().driverText())
		}
		else
		{
			m_model->clearModifiedIndices();
			MSG_OUT("DBExercisesTable updateExercisesListFromModel SUCCESS")
		}
		mSqlLiteDB.close();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::saveExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		bool bUpdate(false);
		QString strQuery;
		if (query.exec(QStringLiteral("SELECT id FROM exercises_table WHERE primary_name=%1").arg(m_data.at(EXERCISES_COL_MAINNAME))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			//from_list is set to 0 because an edited exercise, regardless of its id, is considered different from the default list provided exercise
			strQuery =  QStringLiteral(
							"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
							"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\', from_list=0 WHERE id=%9")
								.arg(m_data.at(EXERCISES_COL_MAINNAME), m_data.at(EXERCISES_COL_SUBNAME), m_data.at(EXERCISES_COL_MUSCULARGROUP),
									m_data.at(EXERCISES_COL_SETSNUMBER), m_data.at(EXERCISES_COL_REPSNUMBER), m_data.at(EXERCISES_COL_WEIGHT),
									m_data.at(EXERCISES_COL_WEIGHTUNIT), m_data.at(EXERCISES_COL_MEDIAPATH), m_data.at(EXERCISES_COL_ID));
		}
		else
		{
			m_data[0] = QString::number(static_cast<DBExercisesModel*>(m_model)->lastID());
			strQuery = QStringLiteral(
							"INSERT INTO exercises_table"
							"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
							" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0)")
								.arg(m_data.at(EXERCISES_COL_ID), m_data.at(EXERCISES_COL_MAINNAME), m_data.at(EXERCISES_COL_SUBNAME),
									m_data.at(EXERCISES_COL_MUSCULARGROUP), m_data.at(EXERCISES_COL_SETSNUMBER), m_data.at(EXERCISES_COL_REPSNUMBER),
									m_data.at(EXERCISES_COL_WEIGHT), m_data.at(EXERCISES_COL_WEIGHTUNIT), m_data.at(EXERCISES_COL_MEDIAPATH));
		}
		m_result = query.exec(strQuery);
		if (m_result)
		{
			if (bUpdate)
				m_model->updateList(data(), m_model->currentRow());
			else
			{
				m_data[EXERCISES_COL_FROMAPPLIST] = u"0"_qs;
				m_data[EXERCISES_COL_ACTUALINDEX] = QString::number(m_model->count());
				m_data[EXERCISES_COL_SELECTED] = u"0"_qs;
				m_model->appendList(data());
				static_cast<DBExercisesModel*>(m_model)->setLastID(static_cast<DBExercisesModel*>(m_model)->lastID()+1);
			}
			MSG_OUT("DBExercisesTable saveExercise SUCCESS");
			MSG_OUT(strQuery);
		}
		else
		{
			MSG_OUT("DBExercisesTable saveExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
			MSG_OUT("DBExercisesTable saveExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
			MSG_OUT("--ERROR--");
			MSG_OUT(strQuery);
		}
		mSqlLiteDB.close();
	}
	else
		MSG_OUT("DBExercisesTable saveExercise Could not open Database")
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::setData(const QString& id, const QString& mainName, const QString& subName,
						const QString& muscularGroup, const QString& nSets, const QString& nReps,
						const QString& nWeight, const QString& uWeight, const QString& mediaPath)
{
	m_data[EXERCISES_COL_ID] = id;
	m_data[EXERCISES_COL_MAINNAME] = mainName;
	m_data[EXERCISES_COL_SUBNAME] = subName;
	m_data[EXERCISES_COL_MUSCULARGROUP] = muscularGroup;
	m_data[EXERCISES_COL_SETSNUMBER] = nSets; //QString::number(nSets,'g', 1)
	m_data[EXERCISES_COL_REPSNUMBER] = nReps;
	m_data[EXERCISES_COL_WEIGHT] = nWeight;
	m_data[EXERCISES_COL_WEIGHTUNIT] = uWeight;
	m_data[EXERCISES_COL_MEDIAPATH] = mediaPath;
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
