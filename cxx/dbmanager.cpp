#include "dbmanager.h"
#include "runcommands.h"
#include "dbexerciseslist.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_workingRow(0)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + dbExercisesList::DBFileName());

	if (!f_info.isReadable())
	{
		//First time: initialize all databases
		dbExercisesList* db_exercises(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
		db_exercises->createTable();
		delete db_exercises;
	}

	getExercisesListVersion();
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
	{
		m_exercisesLocked = 2;
		QThread *thread = new QThread ();
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
		connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
		connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesList::updateExercisesList );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		thread->start ();
	}
}

void DbManager::gotResult(const uint db_id, const OP_CODES op)
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
void DbManager::getAllExercises()
{
	QThread *thread = new QThread ();
	dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
	connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
	connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
	connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
	connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
	connect ( thread, &QThread::started, worker, &dbExercisesList::getAllExercises );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );
	thread->start ();
	dbExercisesList::setExercisesTableLastId(dbExercisesList::exercisesTableLastId() + 1);
}

void DbManager::newExercise( const uint row, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_workingRow = row;
		dbExercisesList::setData(QString::number(dbExercisesList::exercisesTableLastId()), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);

		QThread *thread = new QThread ();
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
		connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
		connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesList::newExercise );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		thread->start ();
		dbExercisesList::setExercisesTableLastId(dbExercisesList::exercisesTableLastId() + 1);
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

		QThread *thread = new QThread ();
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
		connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
		connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesList::updateExercise );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		thread->start ();
	}
}

void DbManager::removeExercise(const uint row)
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_workingRow = row;
		dbExercisesList::setData(m_dbExercisesModel.data(row, DBExercisesModel::exerciseIdRole));

		QThread *thread = new QThread ();
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, &m_dbExercisesModel));
		connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
		connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesList::removeExercise );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		thread->start ();
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
