#include "dbexercisestable.h"
#include "runcommands.h"

#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTime>
#include <QMutexLocker>

uint dbExercisesTable::m_exercisesTableLastId(1000);

dbExercisesTable::dbExercisesTable(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model)
	: TPDatabaseTable(appSettings, static_cast<TPListModel*>(model))
{
	setObjectName( DBExercisesObjectName );
	const QString cnx_name( QStringLiteral("db_worker_connection-") + QTime::currentTime().toString(QStringLiteral("z")) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname( dbFilePath + DBExercisesFileName );
	mSqlLiteDB.setDatabaseName( dbname );
	for(uint i(0); i < 10; i++)
		m_data.append(QString());
}

void dbExercisesTable::createTable()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
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
		qDebug() << "ExercisesList createTable Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList createTable Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
		qDebug() << "ExercisesList createTable SUCCESS";
}

void dbExercisesTable::getAllExercises()
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
				uint idx(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						exercise_info.append(query.value(static_cast<int>(i)).toString());
					exercise_info.append(QString::number(idx++)); //actualIndex
					m_model->appendList(exercise_info);
					exercise_info.clear();
				} while ( query.next () );
				QModelIndex index(m_model->index(m_model->count() - 1, 0));
				const uint highest_id (static_cast<DBExercisesModel*>(m_model)->data(index, DBExercisesModel::exerciseIdRole).toUInt());
				if (highest_id >= m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id + 1;
				m_opcode = OP_READ;
				m_result = true;
				mSqlLiteDB.close();
				resultFunc(static_cast<TPDatabaseTable*>(this));
			}
		}
	}

	if (!m_result)
	{
		qDebug() << "ExercisesList getAllExercises Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList getAllExercises Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
		qDebug() << "ExercisesList getAllExercises SUCCESS";
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void dbExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		qDebug() << "ExercisesList updateExercisesList m_ExercisesList is empty";
		m_result = false;
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
		const QString strWeightUnit (m_appSettings->value("weightUnit").toString());
		const QString query_cmd(QStringLiteral("INSERT INTO exercises_table (id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES(%1, \'%2\', \'%3\', \'%4\', 4, 12, 20, \'%5\', \'qrc:/images/no_image.jpg\', 1)"));

		uint idx ( 0 );
		for ( ++itr; itr != itr_end; ++itr, ++idx ) //++itr: Jump over version number
		{
			fields = static_cast<QString>(*itr).split(';');
			query.prepare(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), strWeightUnit));
			query.exec();
		}
		m_result = mSqlLiteDB.lastError().databaseText().isEmpty();
		mSqlLiteDB.close();
		if (m_result)
		{
			m_opcode = OP_UPDATE_LIST;
			resultFunc(static_cast<TPDatabaseTable*>(this));
		}
	}
	m_ExercisesList.clear();

	if (!m_result)
	{
		qDebug() << "ExercisesList updateExercisesList Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList updateExercisesList Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
	{
		qDebug() << "ExercisesList updateExercisesList success";
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void dbExercisesTable::newExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		m_data[0] = QString::number(m_exercisesTableLastId++);
		m_data[9] = QString::number(m_model->count());
		QSqlQuery query(mSqlLiteDB);
		qDebug() << QStringLiteral(
									"INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0)")
									.arg(m_data.at(0), m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), m_data.at(7), m_data.at(8));

		query.prepare( QStringLiteral(
									"INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0)")
									.arg(m_data.at(0), m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), m_data.at(7), m_data.at(8)) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		qDebug() << "ExercisesList newExercise SUCCESS";
		m_opcode = OP_ADD;
		resultFunc(static_cast<TPDatabaseTable*>(this));
	}
	else
	{
		qDebug() << "ExercisesList newExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList newExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void dbExercisesTable::updateExercise()
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
	}

	if (m_result)
	{
		qDebug() << "ExercisesList updateExercise SUCCESS";
		m_opcode = OP_EDIT;
		resultFunc(static_cast<TPDatabaseTable*>(this));
	}
	else
	{
		qDebug() << "ExercisesList updateExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList updateExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void dbExercisesTable::removeExercise()
{
	m_result = false;
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM exercises_table WHERE id=") + m_data.at(0) );
		m_result = query.exec();
		mSqlLiteDB.close();
	}

	if (m_result)
	{
		qDebug() << "ExercisesList removeExercise SUCCESS";
		m_opcode = OP_DEL;
		resultFunc(static_cast<TPDatabaseTable*>(this));
	}
	else
	{
		qDebug() << "ExercisesList removeExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList removeExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void dbExercisesTable::setData(const QString& id, const QString& mainName, const QString& subName,
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

void dbExercisesTable::removePreviousListEntriesFromDB()
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
		qDebug() << "ExercisesList removePreviousListEntriesFromDB Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList removePreviousListEntriesFromDB Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
}

void dbExercisesTable::getExercisesList()
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
