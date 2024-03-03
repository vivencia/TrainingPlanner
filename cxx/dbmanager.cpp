#include "dbmanager.h"
#include "runcommands.h"

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
	: QObject (nullptr), m_execId(0), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_runCommands(runcommands),
		m_model(nullptr), m_insertid(0), m_exercisesPage(nullptr), m_splitComponent(nullptr), m_lastUsedSplitMesoID(0),
		m_calPage(nullptr), m_calComponent(nullptr), m_lastUsedCalMesoID(0), m_tDayComponent(nullptr), m_tDayExercisesComponent(nullptr)
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
	mesosCalendarModel = new DBMesoCalendarModel(this);

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("runCmd"), QVariant::fromValue(m_runCommands) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoSplitModel"), QVariant::fromValue(mesoSplitModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesosCalendarModel"), QVariant::fromValue(mesosCalendarModel) });
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
	delete mesosCalendarModel;
	delete m_exercisesPage;
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
			else if (dbObj->objectName() == DBTrainingDayObjectName)
			{
				switch (static_cast<DBTrainingDayTable*>(dbObj)->opCode())
				{
					case OP_READ:
					{
						DBTrainingDayModel* model(static_cast<DBTrainingDayModel*>(static_cast<DBTrainingDayTable*>(dbObj)->model()));
						if (model->exercisesNumber() > 0)
							createExercisesObjects(model);
					}
					break;
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
	MSG_OUT("calling qmlReady()")
	emit qmlReady(dbObj->execId());
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

void DbManager::requestTimerDialog(QQuickItem* requester, const QVariant& args, const QVariant& date)
{
	const QVariantList strargs(args.toList());
	QMetaObject::invokeMethod(m_tDayPages.at(m_tDayObjects.value(date.toDate())), "requestTimerDialog", Q_ARG(QVariant, QVariant::fromValue(requester)),
		Q_ARG(QVariant, strargs.at(0)), Q_ARG(QVariant, strargs.at(1)), Q_ARG(QVariant, strargs.at(2)));
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
		emit getQmlObject(m_exercisesPage, false);
		return;
	}

	m_exercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status status)
					{ if (status == QQmlComponent::Ready) return DbManager::createExercisesListPage(); } );

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
	if (exercisesListModel->isReady())
	{
		if (m_exercisesComponent->status() == QQmlComponent::Ready)
		{
			disconnect( this, &DbManager::databaseFree, this, &DbManager::createExercisesListPage );
			m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->create(m_QMlEngine->rootContext()));
			m_QMlEngine->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
			QQuickWindow* parent(static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0)));
			m_exercisesPage->setParentItem(parent->contentItem());
			m_exercisesPage->setParent(parent);
			emit getQmlObject(m_exercisesPage, true);
		}
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
	delete worker;
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

void DbManager::deleteMesocyclesTable()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, static_cast<DBMesocyclesModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteMesocyclesTable(); } );
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DbManager::getMesoSplit(const int meso_id)
{
	if (meso_id >= 0)
	{
		DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
		worker->addExecArg(meso_id);
		createThread(worker, [worker] () { worker->getMesoSplit(); } );
	}
}

void DbManager::newMesoSplit(const uint meso_id, const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(QString::number(meso_id), splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->newMesoSplit(); } );
}

void DbManager::updateMesoSplit(const uint meso_id, const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(QString::number(meso_id), splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->updateMesoSplit(); } );
}

void DbManager::removeMesoSplit(const uint meso_id)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->setData(QString::number(meso_id));
	createThread(worker, [worker] () { return worker->removeMesoSplit(); } );
}

void DbManager::deleteMesoSplitTable()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteMesoSplitTable(); } );
}

void DbManager::getCompleteMesoSplit(const uint meso_id, const uint meso_idx, const QString& mesoSplit)
{
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());
	QChar splitLetter;
	QString createdSplits;
	DBMesoSplitModel* splitModel(nullptr);

	if (meso_id != m_lastUsedSplitMesoID)
	{
		m_lastUsedSplitMesoID = meso_id;
		m_createdSplits.clear();
		m_splitProperties.clear();
		m_splitModels.clear();
		m_splitItems.clear();

		m_qmlSplitObjectParent = m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("exercisesPlanner"));
		m_qmlSplitObjectContainer = m_qmlSplitObjectParent->findChild<QQuickItem*>(QStringLiteral("splitSwipeView"));
		m_splitProperties.insert(QStringLiteral("mesoId"), meso_id);
		m_splitProperties.insert(QStringLiteral("mesoIdx"), meso_idx);
		m_splitProperties.insert(QStringLiteral("parentItem"), QVariant::fromValue(m_qmlSplitObjectParent));
		m_splitProperties.insert(QStringLiteral("splitLetter"), QString(QChar('A')));
		m_splitProperties.insert(QStringLiteral("splitModel"), QVariant());
	}

	if (m_splitComponent == nullptr)
	{
		m_splitComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous);
			connect(m_splitComponent, &QQmlComponent::statusChanged, this,
				[&](QQmlComponent::Status status) { if (status == QQmlComponent::Ready) return DbManager::createMesoSlitPlanner(); } );
	}

	do {
		splitLetter = static_cast<QChar>(*itr);
		if (splitLetter == QChar('R'))
			continue;

		if (m_splitItems.value(splitLetter) != nullptr )
		{
			QMetaObject::invokeMethod(m_qmlSplitObjectContainer, "insertItem", Q_ARG(int, static_cast<int>(splitLetter.cell()) - static_cast<int>('A')),
				Q_ARG(QQuickItem*, m_splitItems.value(splitLetter)));
			continue;
		}

		if (createdSplits.indexOf(splitLetter) == -1)
		{
			createdSplits.append(splitLetter);
			splitModel = new DBMesoSplitModel(this);
			m_splitModels.insert(splitLetter, splitModel);

			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, splitModel));
			worker->addExecArg(meso_id);
			worker->addExecArg(static_cast<QChar>(*itr));
			connect( this, &DbManager::qmlReady, this, &DbManager::createMesoSlitPlanner );
			createThread(worker, [worker] () { return worker->getCompleteMesoSplit(); } );
		}
	} while (++itr != itr_end);
}

void DbManager::createMesoSlitPlanner()
{
	if (m_splitComponent->status() == QQmlComponent::Ready )
	{
		QMapIterator<QChar,DBMesoSplitModel*> i(m_splitModels);
		i.toFront();
		while (i.hasNext()) {
			i.next();
			if (i.value()->isReady())
			{
				if (m_createdSplits.indexOf(i.key()) == -1)
				{
					disconnect( this, &DbManager::qmlReady, this, &DbManager::createMesoSlitPlanner );
					m_createdSplits.append(i.key());
					m_splitProperties[QStringLiteral("splitLetter")] = QString(i.key());
					m_splitProperties[QStringLiteral("splitModel")] = QVariant::fromValue(m_splitModels.value(i.key()));
					QQuickItem* item (static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, m_QMlEngine->rootContext())));
					m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
					item->setParentItem(m_qmlSplitObjectParent);
					connect(item, SIGNAL(requestExercisesPaneAction(int,QVariant,QQuickItem*)), this, SLOT(receiveQMLSignal(int,QVariant,QQuickItem*)) );
					QMetaObject::invokeMethod(m_qmlSplitObjectContainer, "insertItem", Q_ARG(int, static_cast<int>(i.key().cell()) - static_cast<int>('A')),
						Q_ARG(QQuickItem*, item));
					m_splitItems.insert(i.key(), item);
				}
			}
		}
	}
}

void DbManager::updateMesoSplitComplete(const uint meso_id, const QString& splitLetter)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, static_cast<DBMesoSplitModel*>(m_model)));
	worker->addExecArg(meso_id);
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
void DbManager::getMesoCalendar(const int meso_id)
{
	if (meso_id >= 0)
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
		worker->addExecArg(meso_id);
		createThread(worker, [worker] () { worker->getMesoCalendar(); } );
	}
}

void DbManager::createMesoCalendar()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	createThread(worker, [worker] () { worker->createMesoCalendar(); } );
}

void DbManager::createMesoCalendarPage(const uint meso_id, const uint meso_idx)
{
	if (meso_id != m_lastUsedCalMesoID)
	{
		m_lastUsedCalMesoID = meso_id;
		if (m_calPage != nullptr)
		{
			delete m_calPage;
			m_calPage = nullptr;
		}
		if (mesosCalendarModel->getMesoId() != meso_id)
			mesosCalendarModel->clear();
		if (mesosCalendarModel->count() == 0)
		{
			m_model = static_cast<TPListModel*>(mesosCalendarModel);
			connect( this, &DbManager::qmlReady, this, &DbManager::createMesoCalendarPage_part2 );
			getMesoCalendar(meso_id);
		}
	}
	if (m_calComponent == nullptr)
	{
		m_calProperties.insert(QStringLiteral("mesoId"), meso_id);
		m_calProperties.insert(QStringLiteral("mesoIdx"), meso_idx);

		m_calComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoContent.qml"_qs), QQmlComponent::Asynchronous, m_appStackView);
		connect(m_calComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status status)
					{ if (status == QQmlComponent::Ready) return DbManager::createMesoCalendarPage_part2(); } );
		#ifdef DEBUG
		if (m_calComponent->status() == QQmlComponent::Error)
		{
			qDebug() << m_calComponent->errorString();
			for (uint i(0); i < m_calComponent->errors().count(); ++i)
				qDebug() << m_calComponent->errors().at(i).description();
		}
		#endif
	}
	else
		createMesoCalendarPage_part2();

}

void DbManager::createMesoCalendarPage_part2()
{
	#ifdef DEBUG
	if (m_calComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_calComponent->errorString();
		for (uint i(0); i < m_calComponent->errors().count(); ++i)
			qDebug() << m_calComponent->errors().at(i).description();
	}
	#endif
	if (m_calPage == nullptr)
	{
		if (m_calComponent->status() == QQmlComponent::Ready && mesosCalendarModel->isReady())
		{
			disconnect( this, &DbManager::qmlReady, this, &DbManager::createMesoCalendarPage_part2 );
			if (mesosCalendarModel->count() == 0)
			{
				uint mesoIdx(m_calProperties.value(QStringLiteral("mesoIdx")).toUInt());
				mesosCalendarModel->createModel( m_calProperties.value(QStringLiteral("mesoId")).toUInt(), mesocyclesModel->getDateFast(mesoIdx, 2),
									mesocyclesModel->getDateFast(mesoIdx, 3), mesocyclesModel->getFast(mesoIdx, 6) );
				createMesoCalendar();
			}

			m_calPage = static_cast<QQuickItem*>(m_calComponent->createWithInitialProperties(m_calProperties, m_QMlEngine->rootContext()));
			m_QMlEngine->setObjectOwnership(m_calPage, QQmlEngine::CppOwnership);
			m_calPage->setParentItem(m_appStackView);
			emit getQmlObject(m_calPage, true);
		}
	}
	else
		emit getQmlObject(m_calPage, true);
}

void DbManager::newMesoCalendarEntry(const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	worker->setData(QString(), QString::number(mesoId), QString::number(calDate.toJulianDay()), QString::number(calNDay), calSplit);
	createThread(worker, [worker] () { worker->newMesoCalendarEntry(); } );
}

void DbManager::updateMesoCalendarEntry(const uint id, const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, static_cast<DBMesoCalendarModel*>(m_model)));
	worker->setData(QString::number(id), QString::number(mesoId), QString::number(calDate.toJulianDay()), QString::number(calNDay), calSplit);
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
void DbManager::getTrainingDay(const uint meso_idx, const QDate& date)
{
	if (m_tDayObjects.contains(date))
	{
		const uint modelidx(m_tDayObjects.value(date));
		emit getQmlObject(m_tDayPages.at(modelidx), false);
		return;
	}

	m_tDayObjects[date] = m_tDayModels.count();
	DBTrainingDayModel* tDayModel(new DBTrainingDayModel(this));
	tDayModel->setReady(false);
	m_tDayModels.append(tDayModel);

	m_tDayProperties.insert(QStringLiteral("mesoId"), mesocyclesModel->getFast(meso_idx, 0).toUInt());
	m_tDayProperties.insert(QStringLiteral("mesoIdx"), meso_idx);
	m_tDayProperties.insert(QStringLiteral("mainDate"), date);
	m_tDayProperties.insert(QStringLiteral("modelIdx"), m_tDayModels.count()-1);
	m_tDayProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(tDayModel));

	if (m_tDayComponent == nullptr)
	{
		m_tDayComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/TrainingDayInfo.qml"_qs), QQmlComponent::Asynchronous);
		connect(m_tDayComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status status)
					{ if (status == QQmlComponent::Ready) return DbManager::createTrainingDayPage(); } );
	}

	m_execId = 1000;
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, tDayModel));
	worker->setData(QString(), QString(), QString::number(date.toJulianDay()));
	connect( this, &DbManager::qmlReady, this, &DbManager::createTrainingDayPage );
	createThread(worker, [worker] () { return worker->getTrainingDay(); } );
}

void DbManager::createTrainingDayPage(const int exec_id)
{
	if (exec_id >= 0 && exec_id != 1000)
		return;
	if (m_tDayModels.last()->isReady())
	{
		if (m_tDayComponent->status() == QQmlComponent::Ready)
		{
			disconnect( this, &DbManager::qmlReady, this, &DbManager::createTrainingDayPage );
			QQuickItem* item (static_cast<QQuickItem*>(m_tDayComponent->createWithInitialProperties(m_tDayProperties, m_QMlEngine->rootContext())));
			m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
			item->setParentItem(m_appStackView);
			m_tDayPages.append(item);
			emit getQmlObject(item, true);
		}
	}
}

void DbManager::getTrainingDayExercises(const QDate& date)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_tDayModels.at(m_tDayObjects.value(date))));
	worker->setData(QString(), QString(), QString::number(date.toJulianDay()));
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DbManager::newTrainingDay(const uint meso_id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, static_cast<DBTrainingDayModel*>(m_model)));
	worker->setData(QString(), QString::number(meso_id), QString::number(date.toJulianDay()), QString::number(trainingDayNumber),
						splitLetter, timeIn, timeOut, location, notes);
	createThread(worker, [worker] () { return worker->newTrainingDay(); } );
}

void DbManager::updateTrainingDay(const uint id, const uint meso_id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, static_cast<DBTrainingDayModel*>(m_model)));
	worker->setData(QString::number(id), QString::number(meso_id), QString::number(date.toJulianDay()), QString::number(trainingDayNumber),
						splitLetter, timeIn, timeOut, location, notes);
	createThread(worker, [worker] () { return worker->updateTrainingDay(); } );
}

void DbManager::updateTrainingDayExercises(const uint id)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, static_cast<DBTrainingDayModel*>(m_model)));
	worker->addExecArg(id);
	createThread(worker, [worker] () { return worker->updateTrainingDayExercises(); } );
}

void DbManager::removeTrainingDay(const uint id)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, static_cast<DBTrainingDayModel*>(m_model)));
	worker->setData(QString::number(id));
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DbManager::deleteTrainingDayTable()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, static_cast<DBTrainingDayModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteTrainingDayTable(); } );
}

void DbManager::createExerciseObject(const QString& exerciseName, QQuickItem* parentLayout, const uint modelIdx)
{
	m_tDayModels[modelIdx]->newExercise(exerciseName, m_tDayModels[modelIdx]->exercisesNumber());
	if (m_tDayExercisesComponent == nullptr)
	{
		m_tDayExerciseEntryProperties.insert(QStringLiteral("parentLayout"), QVariant::fromValue(parentLayout));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(m_tDayModels.at(modelIdx)));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status status)
					{ if (status == QQmlComponent::Ready) return DbManager::createExerciseObject_part2(); } );
		#ifdef DEBUG
		if (m_tDayExercisesComponent->status() == QQmlComponent::Error)
		{
			qDebug() << m_tDayExercisesComponent->errorString();
			for (uint i(0); i < m_tDayExercisesComponent->errors().count(); ++i)
				qDebug() << m_tDayExercisesComponent->errors().at(i).description();
		}
		#endif
	}
	else
		createExerciseObject_part2();
}

void DbManager::createExerciseObject_part2(const int object_idx)
{
	#ifdef DEBUG
	if (m_tDayExercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_tDayExercisesComponent->errorString();
		for (uint i(0); i < m_tDayExercisesComponent->errors().count(); ++i)
			qDebug() << m_tDayExercisesComponent->errors().at(i).description();
	}
	#endif

	m_tDayExerciseEntryProperties.insert(QStringLiteral("thisObjectIdx"), object_idx >= 0 ? object_idx : m_tDayExercises.count());
	QQuickItem* item (static_cast<QQuickItem*>(m_tDayExercisesComponent->createWithInitialProperties(
													m_tDayExerciseEntryProperties, m_QMlEngine->rootContext())));
	m_QMlEngine->setObjectOwnership(item, QQmlEngine::CppOwnership);
	QQuickItem* parent(m_tDayExerciseEntryProperties.value(QStringLiteral("parentLayout")).value<QQuickItem*>());
	item->setParentItem(parent);
	connect( item, SIGNAL(requestTimerDialogSignal(QQuickItem*,const QVariant&,const QVariant&)), this,
						SLOT(requestTimerDialog(QQuickItem*,const QVariant&,const QVariant&)) );
	m_tDayExercises.append(item);
	emit getQmlObject(item, true);
}

void DbManager::createExercisesObjects(const DBTrainingDayModel* model)
{
	if (m_tDayExercisesComponent == nullptr)
	{
		QQuickItem* parentLayout(m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>(QStringLiteral("tDayExercisesLayout")));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("parentLayout"), QVariant::fromValue(parentLayout));
		m_tDayExerciseEntryProperties.insert(QStringLiteral("tDayModel"), QVariant::fromValue(model));

		m_tDayExercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExerciseEntry.qml"_qs), QQmlComponent::Asynchronous, parentLayout);
		connect(m_tDayExercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status status)
					{ if (status == QQmlComponent::Ready) return DbManager::createExercisesObjects(model); } );
	}
	else {
		for(uint i(0); i < model->exercisesNumber(); ++i)
			createExerciseObject_part2(i);
	}
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
