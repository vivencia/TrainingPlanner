#include "dbexerciseslist.h"
#include "runcommands.h"

#include <QThread>
#include <QSqlQuery>

dbExercisesList::dbExercisesList(const QString& dbFilePath, QSettings* appSettings)
	: QObject(nullptr), m_appSettings(appSettings), m_query(nullptr)
{
	mSqlLiteDB = QSqlDatabase::addDatabase( QStringLiteral("QSQLITE"), QStringLiteral("db_worker_connection-%1").arg(qintptr(QThread::currentThreadId()), 0, 16) );
	mSqlLiteDB.setDatabaseName(dbFilePath + QStringLiteral("/ExercisesList.db.sqlite"));
}

void dbExercisesList::createTable()
{
	QSqlQuery query(mSqlLiteDB);
	query.prepare( QStringLiteral(
									"`CREATE TABLE IF NOT EXISTS exercises_table ("
										"id INTEGER PRIMARY KEY,"
										"primary_name TEXT,"
										"secondary_name TEXT,"
										"muscular_group TEXT,"
										"sets INTEGER,"
										"reps REAL,"
										"weight REAL,"
										"weight_unit TEXT,"
										"media_path TEXT,"
										"from_list INTEGER"
									")`"
								)
	);
	if (!query.exec())
	{
		qDebug() << "ExercisesList createTable Database error:  " << query.lastError().databaseText();
		qDebug() << "ExercisesList createTable Driver error:  " << query.lastError().driverText();
		emit done(-1);
		return;
	}
	emit done(0);
}

void dbExercisesList::getAllExercises()
{
	QSqlQuery query(mSqlLiteDB);
	query.setForwardOnly( true );
	query.prepare( QStringLiteral("`SELECT * FROM exercises_table`") );

	if (!query.exec())
	{
		qDebug() << "ExercisesList getAllExercises Database error:  " << query.lastError().databaseText();
		qDebug() << "ExercisesList getAllExercises Driver error:  " << query.lastError().driverText();
		emit done(-1);
		return;
	}
	if (query.first ())
	{
		QStringList exercise_info;
		DBExercisesModel model;
		const uint n_entries(10);
		uint i(0);
		do
		{
			for (i = 0; i < n_entries; ++i)
				exercise_info.append(query.value(static_cast<int>(i)).toString());
			model.appendList(exercise_list);
			exercise_info.clear();
		} while ( query.next () );
		emit gotResult(model);
		emit done(0);
		return;
	}
	qDebug() << "ExercisesList getAllExercises query.first() fail:  " << query.lastError().databaseText();
	emit done(-1);
}

void dbExercisesList::newExercise()
{
	QSqlQuery query(mSqlLiteDB);
	query.prepare( QStringLiteral(
									"`INSERT INTO exercises_table"
									"(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
									" VALUES(%1, \'%2\', \'%3\', %4, %5, %6, \'%7\', \'%8\', \'%9\', \'%10\')`")
									.arg(m_data.at(0).toInt()).arg(m_data.at(1), m_data.at(2), m_data.at(3)).arg(m_data.at(4).toFloat())
									.arg(m_data.at(5).toFloat()).arg(m_data.at(6).toFloat()).arg(m_data.at(7), m_data.at(8), m_data.at(9)) );
	if (!query.exec())
	{
		qDebug() << "newExercise createTable Database error:  " << query.lastError().databaseText();
		qDebug() << "newExercise createTable Driver error:  " << query.lastError().driverText();
		emit done(-1);
		return;
	}
	emit done(0);
}

void dbExercisesList::updateExercise()
{
	QSqlQuery query(mSqlLiteDB);
	query.prepare( QStringLiteral(
									"`UPDATE exercises_table SET primary_name=\'%1\', secondary_name=\'%2\', muscular_group=\'%3\', "
									"sets=%4, reps=%5, weight=%6, weight_unit=\'%7\', media_path=\'%8\', from_list=\'%9\' WHERE id=%10)`")
									.arg(m_data.at(0).toInt()).arg(m_data.at(1), m_data.at(2), m_data.at(3)).arg(m_data.at(4).toFloat())
									.arg(m_data.at(5).toFloat()).arg(m_data.at(6).toFloat()).arg(m_data.at(7), m_data.at(8), m_data.at(9)) );
	if (!query.exec())
	{
		qDebug() << "newExercise createTable Database error:  " << query.lastError().databaseText();
		qDebug() << "newExercise createTable Driver error:  " << query.lastError().driverText();
		emit done(-1);
		return;
	}
	emit done(0);
}

void dbExercisesList::setData(const int id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath)
{
	m_data.append(QString::number(id));
	m_data.append(mainName);
	m_data.append(subName);
	m_data.append(muscularGroup);
	m_data.append(QString::number(nSets,'g', 1));
	m_data.append(QString::number(nReps,'g', 1));
	m_data.append(QString::number(nWeight,'g', 1));
	m_data.append(uWeight);
	m_data.append(mediaPath);
}

void dbExercisesList::removePreviousListEntriesFromDB()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(QStringLiteral("DELETE FROM exercises_table WHERE from_list=1"), mSqlLiteDB);
		if (!query.exec())
		{
			qDebug() << "ExercisesList removePreviousListEntriesFromDB Database error:  " << query.lastError().databaseText();
			qDebug() << "ExercisesList removePreviousListEntriesFromDB Driver error:  " << query.lastError().driverText();
			emit done(-1);
		}
	}
	else
	{
		qDebug() << "ExercisesList removePreviousListEntriesFromDB Database not opened:   " << mSqlLiteDB.lastError();
	}
}

void dbExercisesList::updateExercisesList()
{
	RunCommands *runCmd(new RunCommands);
	const QStringList exercisesList( runCmd->getExercisesList() );
	delete runCmd;
	if (exercisesList.isEmpty())
		return;

	removePreviousListEntriesFromDB();

	if (mSqlLiteDB.open())
	{
		QStringList::const_iterator itr ( exercisesList.constBegin () );
		const QStringList::const_iterator itr_end ( exercisesList.constEnd () );

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
			if (!query.exec())
			{
				qDebug() << "ExercisesList updateExercisesList Database error:  " << query.lastError().databaseText();
				qDebug() << "ExercisesList updateExercisesList Driver error:  " << query.lastError().driverText();
				emit done(-1);
			}
		}
	}
	else
	{
		qDebug() << "ExercisesList SQLite database could not be opened:" << mSqlLiteDB.databaseName();
		emit done(-1);
	}
	emit done(0);
}
