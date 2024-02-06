#include "dbmanager.h"
#include "runcommands.h"
#include "dbexercisestable.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlContext>

static uint row(0);

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_model(nullptr), m_exercisesLocked(0)
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
		QThread *thread = new QThread ();
		dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
		connect ( worker, &dbExercisesTable::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesTable::done, worker, &dbExercisesTable::deleteLater );
		connect ( worker, &dbExercisesTable::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesTable::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesTable::updateExercisesList );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		startThread(thread);
	}
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
}

void DbManager::gotResult(const dbExercisesTable* dbObj, const OP_CODES op)
{
	switch (op)
	{
		case OP_ADD: if (m_model) m_model->appendList(dbObj->data()); break;
		case OP_EDIT: if (m_model) m_model->updateList(dbObj->data(), m_model->currentRow()); break;
		case OP_DEL: if (m_model) m_model->removeFromList(m_model->currentRow()); break;
		case OP_UPDATE_LIST: m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion); break;
	}
	m_exercisesLocked--;
	if (m_exercisesLocked == 0)
	{
		emit databaseFree();
		if (m_exercisesLocked == 0)
			{
				qDebug() << "freeLocks and calling qmlReady";
				emit qmlReady();
			}
			else
				qDebug() << "freeLocks but another lock is in place. Not calling qmlReady";
	}
}

void DbManager::freeLocks(const bool res)
{
	if (res)
	{
		m_exercisesLocked--;
		if (m_exercisesLocked == 0)
		{
			emit databaseFree();
			if (m_exercisesLocked == 0)
			{
				qDebug() << "freeLocks and calling qmlReady";
				emit qmlReady();
			}
			else
				qDebug() << "freeLocks but another lock is in place. Not calling qmlReady";
		}
	}
	else
	{
		m_exercisesLocked = 0; //error: resources are not being used then
	}
}

void DbManager::startThread(QThread* thread)
{
	qDebug() << "starting thread: " << thread->isFinished();
	if (!thread->isFinished())
	{
		m_exercisesLocked = 2;
		thread->start();
	}
}
//--------------------EXERCISES TABLE---------------------------------
void DbManager::getAllExercises()
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	QThread *thread = new QThread ();
	connect ( worker, &dbExercisesTable::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesTable::done, worker, &dbExercisesTable::deleteLater );
	connect ( worker, &dbExercisesTable::gotResult, this, &DbManager::gotResult );
	connect ( worker, &dbExercisesTable::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &dbExercisesTable::getAllExercises );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_exercisesLocked == 0)
		startThread(thread);
	else
	{
		qDebug() << "waiting for database to be free";
		connect( this, &DbManager::databaseFree, this, [&,thread] () { return DbManager::startThread(thread); } );
	}
}

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
	worker->setData(QStringLiteral("0"), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);

	QThread *thread = new QThread ();
	connect ( worker, &dbExercisesTable::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesTable::done, worker, &dbExercisesTable::deleteLater );
	connect ( worker, &dbExercisesTable::gotResult, this, &DbManager::gotResult );
	connect ( worker, &dbExercisesTable::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &dbExercisesTable::newExercise );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_exercisesLocked == 0)
		startThread(thread);
	else
		connect( this, &DbManager::databaseFree, this, [&,thread] () { return DbManager::startThread(thread); } );
}

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);

	QThread *thread = new QThread ();
	connect ( worker, &dbExercisesTable::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesTable::done, worker, &dbExercisesTable::deleteLater );
	connect ( worker, &dbExercisesTable::gotResult, this, &DbManager::gotResult );
	connect ( worker, &dbExercisesTable::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &dbExercisesTable::updateExercise );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_exercisesLocked == 0)
		startThread(thread);
	else
		connect( this, &DbManager::databaseFree, this, [&,thread] () { return DbManager::startThread(thread); } );
}

void DbManager::removeExercise(const QString& id)
{
	dbExercisesTable* worker(new dbExercisesTable(m_DBFilePath, m_appSettings));
	worker->setData(id);

	QThread *thread = new QThread ();
	connect ( worker, &dbExercisesTable::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesTable::done, worker, &dbExercisesTable::deleteLater );
	connect ( worker, &dbExercisesTable::gotResult, this, &DbManager::gotResult );
	connect ( worker, &dbExercisesTable::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &dbExercisesTable::removeExercise );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_exercisesLocked == 0)
		startThread(thread);
	else
		connect( this, &DbManager::databaseFree, this, [&,thread] () { return DbManager::startThread(thread); } );
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
