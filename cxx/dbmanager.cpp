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

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQmlContext>

DbManager::DbManager(QSettings* appSettings, QQmlApplicationEngine *QMlEngine, RunCommands* runcommands)
	: QObject (nullptr), m_appSettings(appSettings), m_QMlEngine(QMlEngine), m_runCommands(runcommands), m_model(nullptr), m_insertid(0)
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

	DBMesocyclesModel* mesocyclesModel(new DBMesocyclesModel(this));
	pass_object(mesocyclesModel);
	getAllMesocycles();

	DBExercisesModel* exercisesListModel(new DBExercisesModel(this));
	DBMesoSplitModel* mesoSplitModel(new DBMesoSplitModel(this));
	DBMesoCalendarModel* mesosCalendarModel(new DBMesoCalendarModel(this));

	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ "appDB", QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ "runCmd", QVariant::fromValue(m_runCommands) });
	properties.append(QQmlContext::PropertyPair{ "mesocyclesModel", QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ "mesoSplitModel", QVariant::fromValue(mesoSplitModel) });
	properties.append(QQmlContext::PropertyPair{ "mesosCalendarModel", QVariant::fromValue(mesosCalendarModel) });
	properties.append(QQmlContext::PropertyPair{ "exercisesListModel", QVariant::fromValue(exercisesListModel) });
	properties.append(QQmlContext::PropertyPair{ "windowHeight", 640 });
	properties.append(QQmlContext::PropertyPair{ "windowWidth", 300 });
	properties.append(QQmlContext::PropertyPair{ "primaryLightColor", QVariant(QColor(187, 222, 251)) });
	properties.append(QQmlContext::PropertyPair{ "primaryColor", QVariant(QColor(37, 181, 243)) });
	properties.append(QQmlContext::PropertyPair{ "primaryDarkColor", QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ "listEntryColor1", QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ "listEntryColor2", QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ "lightIconFolder", QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ "darkIconFolder", QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ "paneBackgroundColor", QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ "accentColor", QVariant(QColor(37, 181, 243)) });
	m_QMlEngine->rootContext()->setContextProperties(properties);
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

void DbManager::deleteExercisesTable()
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, static_cast<DBExercisesModel*>(m_model)));
	createThread(worker, [worker] () { return worker->deleteExercisesTable(); } );
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
	QString createdSplits;
	DBMesoSplitModel* splitModel(nullptr);

	m_qmlObjectParent = m_QMlEngine->rootObjects().at(0)->findChild<QQuickItem*>((QStringLiteral("exercisesPlanner")));
	m_qmlObjectContainer = m_qmlObjectParent->findChild<QQuickItem*>((QStringLiteral("splitSwipeView")));

	m_splitProperties.insert("mesoId", meso_id);
	m_splitProperties.insert("mesoIdx", meso_idx);
	m_splitProperties.insert("parentItem", QVariant::fromValue(m_qmlObjectParent));
	m_splitProperties.insert("splitLetter", QString(QChar('A')));
	m_splitProperties.insert("splitModel", QVariant());

	do {
		if (static_cast<QChar>(*itr) == QChar('R')) continue;

		if (createdSplits.indexOf(static_cast<QChar>(*itr)) == -1)
		{
			createdSplits.append(static_cast<QChar>(*itr));
			splitModel = new DBMesoSplitModel(this);
			m_splitModels.insert(static_cast<QChar>(*itr).cell(), splitModel);

			QQmlComponent* component( new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/MesoSplitPlanner.qml"_qs), QQmlComponent::Asynchronous));
			connect(component, &QQmlComponent::statusChanged, this,
				[&](QQmlComponent::Status status) { if (status == QQmlComponent::Ready) return DbManager::createMesoSlitPlanner(); } );
			m_splitComponents.insert(static_cast<QChar>(*itr).cell(), component);

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
	static QString createdSplits;
	QMapIterator<uchar,QQmlComponent*> i(m_splitComponents);
	i.toFront();
	while (i.hasNext()) {
		i.next();
		if (i.value()->isReady())
		{
			if (m_splitModels.value(i.key())->isReady())
			{
				if (createdSplits.indexOf(static_cast<QChar>(i.key())) == -1)
				{
					createdSplits.append(static_cast<QChar>(i.key()));
					m_splitProperties["splitLetter"] = QString(static_cast<QChar>(i.key()));
					m_splitProperties["splitModel"] = QVariant::fromValue(m_splitModels.value(i.key()));
					QQuickItem* item (static_cast<QQuickItem*>(i.value()->createWithInitialProperties(m_splitProperties, m_QMlEngine->rootContext())));
					QQmlEngine::setObjectOwnership(item, QQmlEngine::JavaScriptOwnership);
					item->setParentItem(m_qmlObjectParent);
					item->setParent(m_qmlObjectParent);
					connect(item, SIGNAL(requestExercisesPaneAction(int,QVariant,QQuickItem*)), this, SLOT(receiveQMLSignal(int,QVariant,QQuickItem*)) );
					QMetaObject::invokeMethod(m_qmlObjectContainer, "insertItem", Q_ARG(int, static_cast<int>(i.key()) - static_cast<int>('A')), Q_ARG(QQuickItem*, item));
				}
			}
		}
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
