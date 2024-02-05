#include "dbmanager.h"
#include "runcommands.h"
#include "dbexercisestable.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>

static uint row(0);

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_model(nullptr)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		//First time: initialize all databases
		dbExercisesList* db_exercises(new dbExercisesList(m_DBFilePath, m_appSettings));
		db_exercises->createTable();
		delete db_exercises;
	}

	getExercisesListVersion();
	if (m_exercisesListVersion != m_appSettings->value("exercisesListVersion").toString())
	{
		m_exercisesLocked = 2;
		QThread *thread = new QThread ();
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings));
		connect ( worker, &dbExercisesList::done, thread, &QThread::quit );
		connect ( worker, &dbExercisesList::done, worker, &dbExercisesList::deleteLater );
		connect ( worker, &dbExercisesList::gotResult, this, &DbManager::gotResult );
		connect ( worker, &dbExercisesList::done, this, &DbManager::freeLocks );
		connect ( thread, &QThread::started, worker, &dbExercisesList::updateExercisesList );
		connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
		worker->moveToThread ( thread );
		thread->start ();
	}

	/*DBExercisesModel *model = new DBExercisesModel();
	connect(model, &TPListModel::countChanged, this, [this,model] () { printOutInfo(); });
	m_model = model;
	getAllExercises();*/
}

void DbManager::printOutInfo()
{
	qDebug() << "Row: " << row;
	qDebug() << m_model->data(row, DBExercisesModel::mainNameRole);
	row++;
}

void DbManager::gotResult(const dbExercisesList* dbObj, const OP_CODES op)
{
	switch (op)
	{
		case OP_ADD: if (m_model) m_model->appendList(dbObj->data()); break;
		case OP_EDIT: if (m_model) m_model->updateList(dbObj->data(), m_model->currentRow()); break;
		case OP_DEL: if (m_model) m_model->removeFromList(m_model->currentRow()); break;
		case OP_UPDATE_LIST: m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion); break;
	}
	m_exercisesLocked--;
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
	dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
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

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath, TPListModel* model )
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_model = model;
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings));
		worker->setData(QString::number(dbExercisesList::exercisesTableLastId()), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);

		QThread *thread = new QThread ();
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

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath, TPListModel* model )
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_model = model;
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings));
		worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);

		QThread *thread = new QThread ();
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

void DbManager::removeExercise(const QString& id, TPListModel* model)
{
	if (m_exercisesLocked == 0) {
		m_exercisesLocked = 2;
		m_model = model;
		dbExercisesList* worker(new dbExercisesList(m_DBFilePath, m_appSettings));
		worker->setData(id);

		QThread *thread = new QThread ();
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
