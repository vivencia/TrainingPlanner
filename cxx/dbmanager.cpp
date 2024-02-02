#include "dbmanager.h"
#include "runcommands.h"
#include "dbexerciseslist.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

DbManager::DbManager(const QString& dbFilename, QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_DBFileName(dbFilename), m_appSettings(appSettings), m_QMlEngine(QMlEngine)
{}

void DbManager::updateExercisesList()
{
	QThread *thread = new QThread ();
	dbExercisesList* worker (new dbExercisesList(m_DBFileName, m_appSettings));
	worker->moveToThread ( thread );

	connect ( thread, &QThread::started, worker, &dbExercisesList::updateExercisesList );
	connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
	connect ( mSearchWorker, &dbExercisesList::gotResult, this, &searchUI::insertFoundInfo );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start ();
}

