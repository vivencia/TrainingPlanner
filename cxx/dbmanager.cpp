#include "dbmanager.h"
#include "runcommands.h"

#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "dbmesocylestable.h"
#include "dbmesocyclesmodel.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_model(nullptr), m_insertid(0)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		//First time: initialize all databases
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath, m_appSettings));
		db_exercises->createTable();
		delete db_exercises;
		DBMesocyclesTable* db_mesos(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		db_mesos->createTable();
		delete db_mesos;
	}

	getExercisesListVersion();
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
	{
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings));
		createThread(worker, [worker] () { return worker->updateExercisesList(); } );
	}

	//QML type registration
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesocyclesModel");
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
{
	if (dbObj->result())
	{
		if (dbObj->objectName() == DBExercisesObjectName)
		{
			if (static_cast<DBExercisesTable*>(dbObj)->opCode() == OP_UPDATE_LIST)
				m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion);
		}
		else if (dbObj->objectName() == DBMesocyclesObjectName)
		{
			switch (static_cast<DBMesocyclesTable*>(dbObj)->opCode())
			{
				case OP_READ:
					m_result = static_cast<DBMesocyclesTable*>(dbObj)->data();
				break;
				case OP_ADD:
					m_insertid = static_cast<DBMesocyclesTable*>(dbObj)->data().at(0).toUInt();
				break;
			}
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
		MSG_OUT("starting thread for " << dbObj->objectName())
		m_WorkerLock[dbObj->objectName()] = 2;
		thread->start();
	}
}

void DbManager::cleanUp(TPDatabaseTable* dbObj)
{
	dbObj->disconnect();
	dbObj->deleteLater();
	dbObj->thread()->quit();
	MSG_OUT("calling databaseFree()")
	emit databaseFree();
	MSG_OUT("calling qmlReady()")
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
		MSG_OUT("Database  " << worker->objectName() << "  is busy. Waiting for it to be free")
		connect( this, &DbManager::databaseFree, this, [&,thread, worker] () { return DbManager::startThread(thread, worker); } );
	}
}

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DbManager::getAllExercises()
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	createThread(worker, [worker] () { worker->getAllExercises(); } );
}

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	worker->setData(QStringLiteral("0"), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { worker->newExercise(); } );
}

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->updateExercise(); } );
}

void DbManager::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
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
//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
void DbManager::getAllMesocycles()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	createThread(worker, [worker] () { worker->getAllMesocycles(); } );
}

void DbManager::getMesoInfo(const uint meso_id)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->addExecArg(meso_id);
	createThread(worker, [worker] () { worker->getMesoInfo(); } );
}

void DbManager::getPreviousMesoId(const uint current_meso_id)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(current_meso_id);
	createThread(worker, [worker] () { worker->getPreviousMesoId(); } );
}

void DbManager::getPreviousMesoEndDate(const uint current_meso_id)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(current_meso_id);
	createThread(worker, [worker] () { worker->getPreviousMesoEndDate(); } );
}

void DbManager::getNextMesoStartDate(const uint meso_id)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
	worker->addExecArg(meso_id);
	createThread(worker, [worker] () { worker->getNextMesoStartDate(); } );
}

void DbManager::getLastMesoEndDate()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
	createThread(worker, [worker] () { worker->getLastMesoEndDate(); } );
}

void DbManager::newMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate, const QString& mesoNote,
						const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(QString(), mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->newMesocycle(); } );
}

void DbManager::updateMesocycle(const QString& id, const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
				const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(id, mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->updateMesocycle(); } );
}

void DbManager::removeMesocycle(const QString& id)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(id);
	createThread(worker, [worker] () { return worker->removeMesocycle(); } );
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
