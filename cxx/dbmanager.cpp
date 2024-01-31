#include "dbmanager.h"
#include "runcommands.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

DbManager::DbManager(const QString& dbFilename, QSettings* appSettings, QQmlApplicationEngine *QMlEngine, const QString& mostRecentListVersion)
	: QObject (nullptr), m_DBFileName(dbFilename), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_exercisesListVersion(mostRecentListVersion)
{}

void DbManager::updateExercisesList()
{
	QThread *thread = new QThread ();
	dbWorkerThread* worker (new dbWorkerThread(m_DBFileName, m_appSettings));
	worker->moveToThread ( thread );

	connect ( thread, &QThread::started, worker, &dbWorkerThread::updateExercisesList );
	connect ( worker, &dbWorkerThread::done, thread, &QThread::quit );
	connect ( worker, &dbWorkerThread::done, worker, &dbWorkerThread::deleteLater );
	//connect ( mSearchWorker, &dbWorkerThread::gotResult, this, &searchUI::insertFoundInfo );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start ();
}

dbWorkerThread::dbWorkerThread(const QString& dbFileName, QSettings* appSettings)
	: QObject(nullptr), m_appSettings(appSettings)
{
	mSqlLiteDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("db_worker_connection"));
	mSqlLiteDB.setDatabaseName(dbFileName);
}

void dbWorkerThread::removePreviousListEntriesFromDB()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query(QStringLiteral("DELETE FROM exercises_table WHERE from_list=1"), mSqlLiteDB);
		if (!query.exec())
		{
			qDebug() << "removePreviousListEntriesFromDB Database error:  " << query.lastError().databaseText();
			qDebug() << "removePreviousListEntriesFromDB Driver error:  " << query.lastError().driverText();
			emit done();
		}
	}
	else
	{
		qDebug() << "removePreviousListEntriesFromDB Database not opened:   " << mSqlLiteDB.lastError();
	}
}

void dbWorkerThread::updateExercisesList()
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
				qDebug() << "updateExercisesList Database error:  " << query.lastError().databaseText();
				qDebug() << "updateExercisesList Driver error:  " << query.lastError().driverText();
			}
		}
	}
	else {
		qDebug() << "SQLite database could not be opened:" << mSqlLiteDB.databaseName();
	}
	emit done();
}
