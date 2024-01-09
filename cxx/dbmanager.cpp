#include "dbmanager.h"
#include "runcommands.h"

#include <QSqlQuery>
#include <QSqlError>

DbManager::DbManager(const QString& dbFileName, QSettings* appSettings, QQmlApplicationEngine *QMlEngine, float mostRecentListVersion)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_mostRecentListVersion(mostRecentListVersion)
{
	mSqlLiteDB = QSqlDatabase::addDatabase("QSQLITE");
	mSqlLiteDB.setDatabaseName(dbFileName);
}

bool DbManager::updateExercisesList(const QStringList &exercisesList)
{
	//databaseName will be "" the first time the app is run. By the time the execution pointer reaches this function we will
	//have a new SQLite db created on the QML side. We just need to get its name before we proceed
	if (mSqlLiteDB.databaseName().isEmpty())
	{
		RunCommands* runCmd (new RunCommands);
		mSqlLiteDB.setDatabaseName( runCmd->searchForDatabaseFile(m_QMlEngine->offlineStoragePath()) );
		delete runCmd;
	}
	else
		removePreviousListEntriesFromDB();

	int ret (mSqlLiteDB.open());
	if (ret)
	{
		QStringList::const_iterator itr ( exercisesList.constBegin () );
		const QStringList::const_iterator itr_end ( exercisesList.constEnd () );
		QStringList fields;
		QSqlQuery query;
		const QString strWeightUnit (m_appSettings->value("weightUnit").toString());
		const QString query_cmd(QStringLiteral("INSERT INTO exercises_table (id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list)"
								" VALUES(%1, \'%2\', \'%3\', \'%4\', 4, 12, 20, \'%5\', \'qrc:/images/no_image.jpg\', 1)"));
		uint idx ( 0 );
		for ( ++itr; itr != itr_end; ++itr, ++idx ) //++itr Jump over version number
		{
			fields = static_cast<QString>(*itr).split(';');
			ret = query.exec(query_cmd.arg(idx).arg(fields.at(0), fields.at(1), fields.at(2).trimmed(), strWeightUnit));
			if (!ret)
			{
				qDebug() << "updateExercisesList Database error:  " << query.lastError().databaseText();
				qDebug() << "updateExercisesList Driver error:  " << query.lastError().driverText();
			}
		}
		m_appSettings->setValue("exercisesListVersion", m_mostRecentListVersion);
	}
	else {
		qDebug() << "SQLite database could not be opened:" << mSqlLiteDB.databaseName();
	}
	return ret;
}

void DbManager::removePreviousListEntriesFromDB()
{
	if (mSqlLiteDB.open())
	{
		QSqlQuery query;
		if (!query.exec(QStringLiteral("DELETE FROM exercises_table WHERE from_list=1")))
		{
			qDebug() << "removePreviousListEntriesFromDB Database error:  " << query.lastError().databaseText();
			qDebug() << "removePreviousListEntriesFromDB Driver error:  " << query.lastError().driverText();
		}
	}
}
