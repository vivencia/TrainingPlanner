#include "dbmanager.h"
#include "runcommands.h"

#include "tpmesocycleclass.h"

#include "dbexercisestable.h"
#include "dbexercisesmodel.h"
#include "dbmesocylestable.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"
#include "dbmesocalendartable.h"
#include "dbmesocalendarmodel.h"
#include "dbtrainingdaytable.h"
#include "dbtrainingdaymodel.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine, RunCommands* runcommands)
	: QObject (nullptr), m_MesoId(-2), m_MesoIdx(0), m_execId(0), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_runCommands(runcommands),
		m_model(nullptr), m_insertid(0), m_exercisesPage(nullptr)
{
	m_DBFilePath = m_appSettings->value("dbFilePath").toString();
	QFileInfo f_info(m_DBFilePath + DBExercisesFileName);

	if (!f_info.isReadable())
	{
		DBExercisesTable* db_exercises(new DBExercisesTable(m_DBFilePath, m_appSettings));
		db_exercises->createTable();
		delete db_exercises;
	}
	f_info.setFile(m_DBFilePath + DBMesocyclesFileName);
	if (!f_info.isReadable())
	{
		DBMesocyclesTable* db_mesos(new DBMesocyclesTable(m_DBFilePath, m_appSettings));
		db_mesos->createTable();
		delete db_mesos;
	}
	f_info.setFile(m_DBFilePath + DBMesoSplitFileName);
	if (!f_info.isReadable())
	{
		DBMesoSplitTable* db_split(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
		db_split->createTable();
		delete db_split;
	}
	f_info.setFile(m_DBFilePath + DBMesoCalendarFileName);
	if (!f_info.isReadable())
	{
		DBMesoCalendarTable* db_cal(new DBMesoCalendarTable(m_DBFilePath, m_appSettings));
		db_cal->createTable();
		delete db_cal;
	}
	f_info.setFile(m_DBFilePath + DBTrainingDayFileName);
	if (!f_info.isReadable())
	{
		DBTrainingDayTable* db_tday(new DBTrainingDayTable(m_DBFilePath, m_appSettings));
		db_tday->createTable();
		delete db_tday;
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
	qmlRegisterType<DBMesoSplitModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBTrainingDayModel");

	mesocyclesModel = new DBMesocyclesModel(this);
	pass_object(mesocyclesModel);
	getAllMesocycles();

	exercisesListModel = new DBExercisesModel(this);
	mesoSplitModel  = new DBMesoSplitModel(this);

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("runCmd"), QVariant::fromValue(m_runCommands) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoSplitModel"), QVariant::fromValue(mesoSplitModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("exercisesListModel"), QVariant::fromValue(exercisesListModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("windowHeight"), 640 });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("windowWidth"), 300 });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryLightColor"), QVariant(QColor(187, 222, 251)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryColor"), QVariant(QColor(37, 181, 243)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryDarkColor"), QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor1"), QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor2"), QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("lightIconFolder"), QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("darkIconFolder"), QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("paneBackgroundColor"), QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("accentColor"), QVariant(QColor(37, 181, 243)) });
	m_QMlEngine->rootContext()->setContextProperties(properties);
}

DbManager::~DbManager()
{
	delete mesocyclesModel;
	delete exercisesListModel;
	delete mesoSplitModel;
	delete m_exercisesPage;
	for(uint i(0); i < m_MesoManager.count(); ++i)
		delete m_MesoManager.at(i);
}

void DbManager::setWorkingMeso(const int mesoId, const uint mesoIdx)
{
	if (mesoId != m_MesoId)
	{
		bool bFound(false);
		for(uint i(0); i < m_MesoManager.count(); ++i)
		{
			if (m_MesoManager.at(i)->mesoId() == mesoId)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			TPMesocycleClass* newMeso(new TPMesocycleClass(mesoId, mesoIdx, m_QMlEngine, this));
			m_MesoManager.append(newMeso);
			connect(newMeso, SIGNAL(itemReady(QQuickItem*,uint)), this, SLOT(&DbManager::getItem(QQuickItem*,uint)));
		}
		m_MesoId = mesoId;
		m_MesoIdx = mesoIdx;
		m_MesoIdStr = QString::number(m_MesoId);
	}
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
{
	if (dbObj->result())
	{
		if (dbObj->opCode() == OP_DELETE_TABLE)
			dbObj->createTable();
		else
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
			else if (dbObj->objectName() == DBMesoCalendarObjectName)
			{
				switch (static_cast<DBTrainingDayTable*>(dbObj)->opCode())
				{
					case OP_READ:
					{
						DBMesoCalendarModel* model(static_cast<DBMesoCalendarModel*>(static_cast<DBMesoCalendarTable*>(dbObj)->model()));
						if (model->count() == 0)
						{
							model->createModel( m_MesoId, mesocyclesModel->getDateFast(m_MesoIdx, 2),
									mesocyclesModel->getDateFast(m_MesoIdx, 3), mesocyclesModel->getFast(m_MesoIdx, 6) );
							createMesoCalendar();
						}
					}
				}
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
	MSG_OUT("calling databaseReady()")
	emit databaseReady(dbObj->execId());
}

void DbManager::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForResultFunc( [&] (TPDatabaseTable* obj) { return gotResult(obj); } );
	worker->setCallbackForDoneFunc( [&] (TPDatabaseTable* obj) { return freeLocks(obj); } );
	worker->setExecId(m_execId);

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

void DbManager::receiveQMLSignal(int id, QVariant param, QQuickItem* qmlObject)
{
	switch(id)
	{
		case 0:
			QMetaObject::invokeMethod(qmlObject, "showHideExercisesPane", Q_ARG(bool, param.toBool()));
		break;
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

void DbManager::deleteExercisesTable()
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteExercisesTable(); } );
}

void DbManager::openExercisesListPage()
{
	if (m_exercisesPage != nullptr)
	{
		emit getItem(m_exercisesPage, 89676);
		return;
	}

	m_exercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, &DbManager::createExercisesListPage);

	exercisesListModel->setReady(exercisesListModel->count() != 0);
	if (!exercisesListModel->isReady())
	{
		exercisesListModel->setReady(false);
		DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
		connect( this, &DbManager::databaseFree, this, &DbManager::createExercisesListPage );
		createThread(worker, [worker] () { return worker->getAllExercises(); } );
	}
}

void DbManager::createExercisesListPage()
{
	#ifdef DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (uint i(0); i < m_exercisesComponent->errors().count(); ++i)
			qDebug() << m_exercisesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (exercisesListModel->isReady())
	{
		disconnect( this, &DbManager::databaseFree, this, &DbManager::createExercisesListPage );
		m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->create(m_QMlEngine->rootContext()));
		m_QMlEngine->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
		QQuickWindow* parent(static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0)));
		m_exercisesPage->setParentItem(parent->contentItem());
		emit getItem(m_exercisesPage, 999);
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
//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
void DbManager::getAllMesocycles()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->getAllMesocycles();
	const uint current_meso_idx(mesocyclesModel->count()-1);
	setWorkingMeso(mesocyclesModel->getInt(current_meso_idx, 0), current_meso_idx );
	delete worker;
}

void DbManager::getMesocycle(const uint meso_idx)
{
	if (meso_idx != m_MesoIdx)
		setWorkingMeso(mesocyclesModel->getInt(meso_idx, 0), meso_idx);

	if (m_MesoManager.at(m_MesoIdx)->getMesoPage() != nullptr)
	{
		emit getItem(m_MesoManager.at(m_MesoIdx)->getMesoPage(), 54321);
		return;
	}
	m_MesoManager.at(m_MesoIdx)->createMesocyclePage();
}

void DbManager::createNewMesocycle(const bool bRealMeso, const QString& name)
{
	QDate startDate, endDate, minimumStartDate;
	if (mesocyclesModel->count() == 0)
	{
		minimumStartDate.setDate(2023, 0, 2); //first monday of that year
		startDate = QDate::currentDate();
		endDate = m_runCommands->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (mesocyclesModel->getInt(mesocyclesModel->count() - 1, 8) == 1)
			minimumStartDate = m_runCommands->getMesoStartDate(mesocyclesModel->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		endDate = m_runCommands->createFutureDate(minimumStartDate, 0, 2, 0);
	}
	const QStringList mesoInfo( QStringList() << u"-1"_qs << name << QString::number(startDate.toJulianDay()) <<
		(bRealMeso ? QString::number(endDate.toJulianDay()) : u"0"_qs) << QString() <<
		(bRealMeso ? QString::number(m_runCommands->calculateNumberOfWeeks(startDate, endDate)) : u"0"_qs) <<
		u"ABCR"_qs << QString() << (bRealMeso ? u"1"_qs : u"0"_qs) );
	mesocyclesModel->appendList(mesoInfo);
	setWorkingMeso(-1, 0);
	m_MesoManager.at(m_MesoIdx)->createMesocyclePage(minimumStartDate, bRealMeso ?
			m_runCommands->createFutureDate(startDate,0,6,0) : QDate(2026,11,31), startDate);
}

void DbManager::newMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate, const QString& mesoNote,
						const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(QString(), mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->newMesocycle(); } );
}

void DbManager::updateMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
				const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(m_MesoIdStr, mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->updateMesocycle(); } );
}

void DbManager::removeMesocycle()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	worker->setData(m_MesoIdStr);
	createThread(worker, [worker] () { return worker->removeMesocycle(); } );
}

void DbManager::deleteMesocyclesTable()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteMesocyclesTable(); } );
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DbManager::getMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->addExecArg(m_MesoId);
	createThread(worker, [worker] () { worker->getMesoSplit(); } );
}

void DbManager::newMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(m_MesoIdStr, splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->newMesoSplit(); } );
}

void DbManager::updateMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(m_MesoIdStr, splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->updateMesoSplit(); } );
}

void DbManager::removeMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(m_MesoIdStr);
	createThread(worker, [worker] () { return worker->removeMesoSplit(); } );
}

void DbManager::deleteMesoSplitTable()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteMesoSplitTable(); } );
}

void DbManager::getCompleteMesoSplit(const QString& mesoSplit)
{
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());
	QChar splitLetter;
	QString createdSplits;

	do {
		splitLetter = static_cast<QChar>(*itr);
		if (splitLetter == QChar('R'))
			continue;

		if (m_MesoManager.at(m_MesoIdx)->getSplitPage(splitLetter) != nullptr)
		{
			emit getItem(m_MesoManager.at(m_MesoIdx)->getSplitPage(splitLetter), static_cast<uint>(splitLetter.toLatin1()));
			continue;
		}

		if (createdSplits.indexOf(splitLetter) == -1)
		{
			createdSplits.append(splitLetter);
			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->getSplitModel(splitLetter)));
			worker->addExecArg(m_MesoId);
			worker->addExecArg(static_cast<QChar>(*itr));
			connect( this, &DbManager::databaseReady, this, [&](uint) { return m_MesoManager.at(m_MesoIdx)->createMesoSplitPage(); } );
			createThread(worker, [worker] () { return worker->getCompleteMesoSplit(); } );
		}
	} while (++itr != itr_end);
}

void DbManager::updateMesoSplitComplete(const QString& splitLetter)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->addExecArg(m_MesoId);
	worker->addExecArg(splitLetter);
	createThread(worker, [worker] () { worker->updateMesoSplitComplete(); } );
}

bool DbManager::previousMesoHasPlan(const uint prev_meso_id, const QString& splitLetter) const
{
	DBMesoSplitTable* meso_split(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
	const bool ret(meso_split->mesoHasPlan(QString::number(prev_meso_id), splitLetter.toStdString().c_str()[0]));
	delete meso_split;
	return ret;
}

void DbManager::loadSplitFromPreviousMeso(const uint prev_meso_id, const QString& splitLetter)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->addExecArg(prev_meso_id);
	worker->addExecArg(splitLetter);
	createThread(worker, [worker] () { worker->getCompleteMesoSplit(); } );
}
//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
void DbManager::getMesoCalendar()
{
	if (m_MesoManager.at(m_MesoIdx)->getCalendarPage() != nullptr)
	{
		emit getItem(m_MesoManager.at(m_MesoIdx)->getCalendarPage(), 12345);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->getCalendarModel()));
	worker->addExecArg(m_MesoId);
	connect( this, &DbManager::databaseReady, this, [&](uint) { return m_MesoManager.at(m_MesoIdx)->createMesoCalendarPage(); } );
	createThread(worker, [worker] () { worker->getMesoCalendar(); });
}

void DbManager::createMesoCalendar()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	createThread(worker, [worker] () { worker->createMesoCalendar(); } );
}

void DbManager::newMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	worker->setData(QString(), m_MesoIdStr, QString::number(calDate.toJulianDay()), QString::number(calNDay), calSplit);
	createThread(worker, [worker] () { worker->newMesoCalendarEntry(); } );
}

void DbManager::updateMesoCalendarEntry(const uint id, const QDate& calDate, const uint calNDay, const QString& calSplit)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	worker->setData(QString::number(id), m_MesoIdStr, QString::number(calDate.toJulianDay()), QString::number(calNDay), calSplit);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DbManager::deleteMesoCalendar(const uint id)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	worker->setData(QString::number(id));
	createThread(worker, [worker] () { return worker->removeMesoCalendar(); } );
}

void DbManager::deleteMesoCalendarTable()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteMesoCalendarTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DbManager::getTrainingDay(const QDate& date)
{
	if (m_MesoManager.at(m_MesoIdx)->gettDayPage(date) != nullptr)
	{
		m_MesoManager.at(m_MesoIdx)->setCurrenttDay(date);
		emit getItem(m_MesoManager.at(m_MesoIdx)->gettDayPage(date), static_cast<uint>(date.toJulianDay()));
		return;
	}

	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->gettDayModel(date)));
	worker->setData(QString(), QString(), QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date](uint) { return m_MesoManager.at(m_MesoIdx)->createTrainingDayPage(date); } );
	createThread(worker, [worker] () { return worker->getTrainingDay(); } );
}

void DbManager::getTrainingDayExercises(const QDate& date)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->setData(QString(), QString(), QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&](uint) { return m_MesoManager.at(m_MesoIdx)->createExercisesObjects(); } );
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DbManager::newTrainingDay(const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->setData(QString(), m_MesoIdStr, QString::number(date.toJulianDay()), QString::number(trainingDayNumber),
						splitLetter, timeIn, timeOut, location, notes);
	createThread(worker, [worker] () { return worker->newTrainingDay(); } );
}

void DbManager::updateTrainingDay(const uint id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->setData(QString::number(id), m_MesoIdStr, QString::number(date.toJulianDay()), QString::number(trainingDayNumber),
						splitLetter, timeIn, timeOut, location, notes);
	createThread(worker, [worker] () { return worker->updateTrainingDay(); } );
}

void DbManager::updateTrainingDayExercises(const uint id)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->updateTrainingDayExercises(); } );
}

void DbManager::removeTrainingDay(const uint id)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->setData(QString::number(id));
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DbManager::deleteTrainingDayTable()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->deleteTrainingDayTable(); } );
}

void DbManager::createExerciseObject(const QString& exerciseName)
{
	m_MesoManager.at(m_MesoIdx)->createExerciseObject(exerciseName);
}

void DbManager::removeExerciseObject(const uint exercise_idx)
{
	m_MesoManager.at(m_MesoIdx)->removeExercise(exercise_idx);
}

void DbManager::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx)
{
	m_MesoManager.at(m_MesoIdx)->currenttDayModel()->newSet(exercise_idx, set_number, set_type);
	m_MesoManager.at(m_MesoIdx)->createSetObject(set_type, set_number, exercise_idx);
}

void DbManager::removeSetObject(const uint set_number, const uint exercise_idx)
{
	m_MesoManager.at(m_MesoIdx)->removeSet(set_number, exercise_idx);
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
