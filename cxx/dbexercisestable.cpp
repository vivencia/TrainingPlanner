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
	: QObject(nullptr), m_appSettings(appSettings), m_model(model)
{
	const QString cnx_name ( QStringLiteral("db_worker_connection-%1").arg(QTime::currentTime().toString(QStringLiteral("z"))) );
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), cnx_name );
	const QString dbname(dbFilePath + DBExercisesFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void dbExercisesTable::createTable()
{
	bool ret(false);
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
		ret = query.exec();
		mSqlLiteDB.close();
	}
	if (!ret)
	{
		qDebug() << "ExercisesList createTable Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList createTable Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
		qDebug() << "ExercisesList createTable SUCCESS";
	emit done(ret);
}

void dbExercisesTable::getAllExercises()
{
	bool ret(false);
	mSqlLiteDB.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
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

				const uint n_entries(10);
				uint i(0);
				do
				{
					for (i = 0; i < n_entries; ++i)
						exercise_info.append(query.value(static_cast<int>(i)).toString());
					m_model->appendList(exercise_info);
					exercise_info.clear();
				} while ( query.next () );
				QModelIndex index;
				const uint highest_id (m_model->data(index.sibling(m_model->count() - 1, 0), DBExercisesModel::exerciseIdRole).toUInt());
				if (highest_id > m_exercisesTableLastId)
					m_exercisesTableLastId = highest_id;
				emit gotResult(this, OP_READ);
				ret = true;
				mSqlLiteDB.close();
			}
		}
	}
	if (!ret)
	{
		qDebug() << "ExercisesList getAllExercises Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList getAllExercises Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
		qDebug() << "ExercisesList getAllExercises SUCCESS";
	emit done(ret);
}

void dbExercisesTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		qDebug() << "ExercisesList updateExercisesList m_ExercisesList is empty";
		emit done(false);
		return;
	}

	removePreviousListEntriesFromDB();

	bool ret(false);
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
			ret = query.exec();
		}
		mSqlLiteDB.close();
		emit gotResult(this, OP_UPDATE_LIST);
	}
	m_ExercisesList.clear();
	if (!ret)
	{
		qDebug() << "ExercisesList updateExercisesList Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList updateExercisesList Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	else
	{
		qDebug() << "ExercisesList updateExercisesList success";
	}
	emit done(ret);
}

void dbExercisesTable::newExercise()
{
	bool ret(false);
	if (mSqlLiteDB.open())
	{
		m_data[0] = QString::number(m_exercisesTableLastId);
		QSqlQuery query(mSqlLiteDB);
		qDebug() << QStringLiteral(
									"INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', %4, %5, %6, \'%7\', \'%8\', \'%9\', 0)")
									.arg(m_data.at(0).toInt()).arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), m_data.at(7), m_data.at(8));

		query.prepare( QStringLiteral(
									"INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', 0)")
									.arg(m_data.at(0).toInt()).arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4),
										m_data.at(5), m_data.at(6), m_data.at(7), m_data.at(8)) );
		ret = query.exec();
		m_data.clear();
		mSqlLiteDB.close();
	}
	if (ret)
	{
		m_exercisesTableLastId++;
		emit gotResult(this, OP_ADD);
		qDebug() << "ExercisesList newExercise SUCCESS";
	}
	else
	{
		qDebug() << "ExercisesList newExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList newExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	emit done(ret);
}

void dbExercisesTable::updateExercise()
{
	bool ret(false);
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		qDebug() << QStringLiteral(
									"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
									"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\' WHERE id=%10")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5), m_data.at(6),
										m_data.at(7), m_data.at(8)).arg(m_data.at(0).toInt());
		query.prepare( QStringLiteral(
									"UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
									"sets=\'%4\', reps=\'%5\', weight=\'%6\', weight_unit=\'%7\', media_path=\'%8\' WHERE id=%10")
									.arg(m_data.at(1), m_data.at(2), m_data.at(3), m_data.at(4), m_data.at(5), m_data.at(6),
										m_data.at(7), m_data.at(8)).arg(m_data.at(0).toInt()) );
		ret = query.exec();
		m_data.clear();
		mSqlLiteDB.close();
	}
	if (ret)
	{
		emit gotResult(this, OP_EDIT);
		qDebug() << "ExercisesList updateExercise SUCCESS";
	}
	else
	{
		qDebug() << "ExercisesList updateExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList updateExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	emit done(ret);
}

void dbExercisesTable::removeExercise()
{
	bool ret(false);
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(mSqlLiteDB);
		query.prepare( QStringLiteral("DELETE FROM exercises_table WHERE id=?").arg(m_data.at(0).toInt()) );
		ret = query.exec();
		m_data.clear();
		mSqlLiteDB.close();
	}
	if (ret)
	{
		emit gotResult(this, OP_DEL);
		qDebug() << "ExercisesList removeExercise SUCCESS";
	}
	else
	{
		qDebug() << "ExercisesList removeExercise Database error:  " << mSqlLiteDB.lastError().databaseText();
		qDebug() << "ExercisesList removeExercise Driver error:  " << mSqlLiteDB.lastError().driverText();
	}
	emit done(ret);
}

void dbExercisesTable::setData(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath)
{
	m_data.append(id);
	m_data.append(mainName);
	m_data.append(subName);
	m_data.append(muscularGroup);
	m_data.append(nSets); //QString::number(nSets,'g', 1)
	m_data.append(nReps);
	m_data.append(nWeight);
	m_data.append(uWeight);
	m_data.append(mediaPath);
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
