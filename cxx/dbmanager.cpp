#include "dbmanager.h"
#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "runcommands.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>

static uint row(0);

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_model(nullptr)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		//First time: initialize all databases
		dbExercisesTable* db_exercises(new dbExercisesTable(m_DBFilePath, m_appSettings));
		db_exercises->createTable();
		delete db_exercises;
	}

	getExercisesListVersion();
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
	{
		dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
		createThread(worker, [worker] () { return worker->updateExercisesList(); } );
	}
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
{
	if (dbObj->objectName() == DBExercisesObjectName)
	{
		switch (static_cast<dbExercisesTable*>(dbObj)->opCode())
		{
			case OP_ADD: if (m_model) m_model->appendList(static_cast<dbExercisesTable*>(dbObj)->data()); break;
			case OP_EDIT: if (m_model) m_model->updateList(static_cast<dbExercisesTable*>(dbObj)->data(), m_model->currentRow()); break;
			case OP_DEL: if (m_model) m_model->removeFromList(m_model->currentRow()); break;
			case OP_UPDATE_LIST: m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion); break;
		}
	}
	m_WorkerLock[dbObj->objectName()]--;
	if (m_WorkerLock[dbObj->objectName()] == 0)
		cleanUp(dbObj);
}

void DbManager::freeLocks(TPDatabaseTable *dbObj)
{
	if (dbObj->result())
	{
		m_WorkerLock[dbObj->objectName()]--;
		if (m_WorkerLock[dbObj->objectName()] == 0)
			cleanUp(dbObj);
	}
	else
	{
		m_WorkerLock[dbObj->objectName()] = 0; //error: resources are not being used then
	}
}

void DbManager::startThread(QThread* thread, TPDatabaseTable* dbObj)
{
	if (!thread->isFinished())
	{
		qDebug() << "starting thread for " << dbObj->objectName();
		m_WorkerLock[dbObj->objectName()] = 2;
		thread->start();
	}
}

void DbManager::cleanUp(TPDatabaseTable* dbObj)
{
	dbObj->disconnect();
	dbObj->deleteLater();
	dbObj->thread()->quit();
	qDebug() << "calling databaseFree()";
	emit databaseFree();
	qDebug() << "calling qmlReady()";
	emit qmlReady();
}

void DbManager::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForResultFunc( [&] (TPDatabaseTable* obj) { return gotResult(obj); } );
	worker->setCallbackForDoneFunc( [&] (TPDatabaseTable* obj) { return freeLocks(obj); } );

	QThread *thread = new QThread ();
	connect ( thread, &QThread::started, worker, execFunc );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_WorkerLock[worker->objectName()] == 0)
		startThread(thread, worker);
	else
	{
		qDebug() << "Database  " << worker->objectName() << "  is busy. Waiting for it to be free";
		connect( this, &DbManager::databaseFree, this, [&,thread, worker] () { return DbManager::startThread(thread, worker); } );
	}
}

//--------------------EXERCISES TABLE---------------------------------
void DbManager::getAllExercises()
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	createThread(worker, [worker] () { worker->getAllExercises(); } );
}

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	worker->setData(QStringLiteral("0"), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { worker->newExercise(); } );
}

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	qDebug() << "Updating exercise id: " << id;
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->updateExercise(); } );
}

void DbManager::removeExercise(const QString& id)
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
	worker->setData(id);
	createThread(worker, [worker] () { return worker->removeExercise(); } );
}

void DbManager::getExercisesListVersion()
{
	m_exercisesListVersion = QStringLiteral("0");
	QFile exercisesListFile( QStringLiteral(":/extras/exerciseslist.lst") );
	if ( exercisesListFile.open( QIODeviceBase::ReadOnly|QIODeviceBase::Text ) )
	{
		char buf[20] = { 0 };
		qint64 lineLength;
		QString line;
		lineLength = exercisesListFile.readLine( buf, sizeof(buf) );
		if (lineLength > 0)
		{
			line = buf;
			if (line.startsWith(QStringLiteral("#Vers")))
				m_exercisesListVersion = line.split(';').at(1).trimmed();
		}
		exercisesListFile.close();
	}
}
//--------------------EXERCISES TABLE---------------------------------
