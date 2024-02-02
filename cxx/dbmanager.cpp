#include "dbmanager.h"
#include "runcommands.h"
#include "dbexerciseslist.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_workingRow(0), m_exercisesTableLastId(1000)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + dbExercisesList::DBFileName());
	if (!f_info.isReadable())
	{
		//First time: initialize all databases
		runTaksInAnotherThread(0, false, [&] () { return dbExercisesList::createTable(); });
	}

	getExercisesListVersion();
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
		runTaksInAnotherThread(0, false, [&] () { return dbExercisesList::updateExercisesList; });
}

void DbManager::getResult(const uint db_id, const OP_CODES op)
{
	switch (db_id)
	{
		case 0:
			switch (op)
			{
				case OP_ADD: m_dbExercisesModel.appendList(dbExercisesList::data()); break;
				case OP_EDIT: m_dbExercisesModel.updateList(dbExercisesList::data(), m_workingRow); break;
				case OP_DEL: m_dbExercisesModel.removeFromList(m_workingRow); break;
			}
			m_exercisesLocked--;
			emit dbExercisesModelModified();
		break;
	}
}

void DbManager::runTaksInAnotherThread( const uint db_id, const bool bCanReceiveResult, const std::function<void ()> &task_func )
{
	QThread *thread = new QThread ();
	QObject* worker(nullptr);

	switch (db_id)
	{
		case 0:
			worker = new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel);
			connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
			connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
			if (bCanReceiveResult)
				connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		break;
	}
	connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &task_func );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );
	thread->start ();
}

void DbManager::freeLocks(const int res)
{
	if ( res == 0 )
	{
		m_exercisesLocked--;
	}
	else
	{
		m_exercisesLocked = 0; //error: resources are not being used then
	}
}

//--------------------EXERCISES TABLE---------------------------------
void DbManager::newExercise( const uint row, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_workingRow = row;
		dbExercisesList::setData(m_exercisesTableLastId, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
		runTaksInAnotherThread(0, true, [&] () { return dbExercisesList::newExercise(); });
		m_exercisesTableLastId++;
	}
}

void DbManager::updateExercise( const uint row, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_workingRow = row;
		dbExercisesList::setData(m_dbExercisesModel.data(row, DBExercisesModel::exerciseIdRole), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
		runTaksInAnotherThread(0, true, [&] () { return dbExercisesList::updateExercise(); });
	}
}

void DbManager::removeExercise(const uint row)
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_workingRow = row;
		dbExercisesList::setData(m_dbExercisesModel.data(row, DBExercisesModel::exerciseIdRole));
		runTaksInAnotherThread(0, true, [&] () { return dbExercisesList::removeExercise(); });
	}
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
		if (lineLength < 0) return 0;
		line = buf;
		if (line.startsWith(QStringLiteral("#Vers")))
			m_exercisesListVersion = line.split(';').at(1).trimmed();
		exercisesListFile.close();
	}
}
//--------------------EXERCISES TABLE---------------------------------
