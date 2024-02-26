#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "runcommands.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTime>

uint DBExercisesTable::m_exercisesTableLastId(1000);

DBExercisesTable::DBExercisesTable(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBExercisesObjectName );
	const QString cnx_name( QStringLiteral("db_exercises_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBExercisesFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < 10; i++)
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
		query.prepare( QStringLiteral(
									"CREATE TABLE IF NOT EXISTS exercises_table ("
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
									")"
								)
		);
		m_result = query.exec();
		mSqlLiteDB.close();
	}
	if (!m_result)
	{
		MSG_OUT("DBExercisesTable createTable Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable createTable Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBExercisesTable createTable SUCCESS")
}

void DBExercisesTable::getAllExercises()
{
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.setForwardOnly( true );
		query.prepare( QStringLiteral("SELECT * FROM exercises_table") );

		if (query.exec())
		{
			if (query.first ())
			{
				QStringList exercise_info;

				const uint n_entries(9);
				uint i(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						exercise_info.append(query.value(static_cast<int>(i)).toString());
					m_model->appendList(exercise_info);
					exercise_info.clear();
				} while ( query.next () );
				QModelIndex index(m_model->index(m_model->count() - 1, 0));
				const uint highest_id (static_cast<DBExercisesModel*>(m_model)->data(index, DBExercisesModel::exerciseIdRole).toUInt());
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_result = true;
			}
		}
		m_model->setReady(true);
		mSqlLiteDB.close();
	}

	if (!m_result)
	{
		MSG_OUT("DBExercisesTable getAllExercises Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable getAllExercises Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
		MSG_OUT("DBExercisesTable getAllExercises SUCCESS")
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		MSG_OUT("DBExercisesTable updateExercisesList m_ExercisesList is empty")
		m_result = false;
		resultFunc(static_cast<TPDatabaseTable*>(this));
		doneFunc(static_cast<TPDatabaseTable*>(this));
		return;
	}

	removePreviousListEntriesFromDB();
	m_result = false;

	if (mSqlLiteDB.open())
	{
		QStringList::const_iterator itr ( m_ExercisesList.constBegin () );
		const QStringList::const_iterator itr_end ( m_ExercisesList.constEnd () );

		QStringList fields;
		QSqlQuery query(mSqlLiteDB);
		query.exec(QStringLiteral("PRAGMA page_size = 4096"));
		query.exec(QStringLiteral("PRAGMA cache_size = 16384"));
		query.exec(QStringLiteral("PRAGMA temp_store = MEMORY"));
		query.exec(QStringLiteral("PRAGMA journal_mode = OFF"));
		query.exec(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE"));
		query.exec(QStringLiteral("PRAGMA synchronous = 0"));

		const QString strWeightUnit (m_appSettings->value("weightUnit").toString());
		const QString query_cmd( QStringLiteral(
								"INSERT INTO exercises_table "
								"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES(%1, \'%2\', \'%3\', \'%4\', 4, 12, 20, \'%5\', \'qrc:/images/no_image.jpg\', 1)") );

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
								<< QString::number(idx) << fields.at(0) << fields.at(1) << fields.at(2).trimmed()
								<< QStringLiteral("4") << QStringLiteral("12") << QStringLiteral("20")
								<< strWeightUnit << QStringLiteral("qrc:/images/no_image.jpg") << QStringLiteral("1") );
			}
		}
		mSqlLiteDB.commit();
		m_result = mSqlLiteDB.lastError().databaseText().isEmpty();
		m_opcode = OP_UPDATE_LIST;
		mSqlLiteDB.close();
	}
	m_ExercisesList.clear();

	if (!m_result)
	{
		MSG_OUT("DBExercisesTable updateExercisesList Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable updateExercisesList Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	else
	{
		MSG_OUT("DBExercisesTable updateExercisesList SUCCESS")
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::newExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		m_data[0] = QString::number(m_exercisesTableLastId++);
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0)")
									.arg(m_data.at(0), m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), m_data.at(7), m_data.at(8)) );
		m_result = query.exec();
		mSqlLiteDB.close();
		if (m_result)
		{
			if (m_model)
			{
				m_data[9] = QString::number(m_model->count());
				m_model->appendList(data());
			}
		}
	}

	if (m_result)
	{
		MSG_OUT("DBExercisesTable newExercise SUCCESS")
	}
	else
	{
		MSG_OUT("DBExercisesTable newExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable newExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::updateExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral(
									"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
									"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\' WHERE id=%9")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5), m_data.at(6),
										m_data.at(7), m_data.at(8), m_data.at(0)) );
		m_result = query.exec();
		mSqlLiteDB.close();
		if (m_result)
		{
			if (m_model)
				m_model->updateList(data(), m_model->currentRow());
		}
	}

	if (m_result)
	{
		MSG_OUT("DBExercisesTable updateExercise SUCCESS");
	}
	else
	{
		MSG_OUT("DBExercisesTable updateExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable updateExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::removeExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM exercises_table WHERE id=") + m_data.at(0) );
		m_result = query.exec();
		mSqlLiteDB.close();
		if (m_result)
		{
			if (m_model)
				m_model->removeFromList(m_model->currentRow());
		}
	}

	if (m_result)
	{
		MSG_OUT("DBExercisesTable removeExercise SUCCESS")
	}
	else
	{
		MSG_OUT("DBExercisesTable removeExercise Database error:  " << mSqlLiteDB.lastError().databaseText())
		MSG_OUT("DBExercisesTable removeExercise Driver error:  " << mSqlLiteDB.lastError().driverText())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::deleteExercisesTable()
{
	QFile mDBFile(mSqlLiteDB.databaseName());
	m_result = mDBFile.remove();
	if (m_result)
	{
		if (m_model)
			m_model->clear();
		m_opcode = OP_DELETE_TABLE;
		MSG_OUT("DBExercisesTable deleteExercisesTable SUCCESS")
	}
	else
	{
		MSG_OUT("DBExercisesTable deleteExercisesTable error: Could not remove file " << mDBFile.fileName())
	}
	resultFunc(static_cast<TPDatabaseTable*>(this));
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBExercisesTable::setData(const QString& id, const QString& mainName, const QString& subName,
						const QString& muscularGroup, const QString& nSets, const QString& nReps,
						const QString& nWeight, const QString& uWeight, const QString& mediaPath)
{
	m_data[0] = id;
	m_data[1] = mainName;
	m_data[2] = subName;
	m_data[3] = muscularGroup;
	m_data[4] = nSets; //QString::number(nSets,'g', 1)
	m_data[5] = nReps;
	m_data[6] = nWeight;
	m_data[7] = uWeight;
	m_data[8] = mediaPath;
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
	QFile exercisesListFile( QStringLiteral(":/extras/exerciseslist.lst") );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
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
