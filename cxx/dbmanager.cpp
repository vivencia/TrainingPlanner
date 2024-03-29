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

#include <QGuiApplication>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

DbManager::DbManager(QSettings* appSettings, RunCommands* runcommands)
	: QObject (nullptr), m_MesoId(-2), m_MesoIdx(0), m_appSettings(appSettings), m_runCommands(runcommands),
		m_model(nullptr), m_exercisesPage(nullptr)
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
}

void DbManager::setQmlEngine(QQmlApplicationEngine* QMlEngine)
{
	m_QMlEngine = QMlEngine;

	//QML type registration
	qmlRegisterType<DBExercisesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBExercisesModel");
	qmlRegisterType<DBMesocyclesModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesocyclesModel");
	qmlRegisterType<DBMesoSplitModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoSplitModel");
	qmlRegisterType<DBMesoCalendarModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBMesoCalendarModel");
	qmlRegisterType<DBTrainingDayModel>("com.vivenciasoftware.qmlcomponents", 1, 0, "DBTrainingDayModel");

	exercisesListModel = new DBExercisesModel(this);
	mesoSplitModel  = new DBMesoSplitModel(this);
	mesoCalendarModel = new DBMesoCalendarModel(this);
	mesocyclesModel = new DBMesocyclesModel(this);
	getAllMesocycles();

	QQuickWindow* mainWindow(static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0)));
	//Root context properties. MainWindow app properties
	QList<QQmlContext::PropertyPair> properties;
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appDB"), QVariant::fromValue(this) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("runCmd"), QVariant::fromValue(m_runCommands) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesocyclesModel"), QVariant::fromValue(mesocyclesModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoSplitModel"), QVariant::fromValue(mesoSplitModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("exercisesListModel"), QVariant::fromValue(exercisesListModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mesoCalendarModel"), QVariant::fromValue(mesoCalendarModel) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryLightColor"), QVariant(QColor(187, 222, 251)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryColor"), QVariant(QColor(37, 181, 243)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("primaryDarkColor"), QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor1"), QVariant(QColor(220, 227, 240)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("listEntryColor2"), QVariant(QColor(195, 202, 213)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("lightIconFolder"), QStringLiteral("white/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("darkIconFolder"), QStringLiteral("black/") });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("paneBackgroundColor"), QVariant(QColor(25, 118, 210)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("accentColor"), QVariant(QColor(37, 181, 243)) });
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("mainwindow"), QVariant::fromValue(mainWindow) });

	QQuickItem* appStackView(mainWindow->findChild<QQuickItem*>("appStackView"));
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("appStackView"), QVariant::fromValue(appStackView) });

	QQuickItem* contentItem(appStackView->parentItem());
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("windowHeight"), contentItem->height() }); //mainwindow.height: 640 - footer.height - header.height
	properties.append(QQmlContext::PropertyPair{ QStringLiteral("windowWidth"), contentItem->width() });

	m_QMlEngine->rootContext()->setContextProperties(properties);

	QMetaObject::invokeMethod(mainWindow, "init");
}

DbManager::~DbManager()
{
	delete mesoCalendarModel;
	delete mesoSplitModel;
	delete exercisesListModel;
	delete mesocyclesModel;
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
			newMeso->setMesocycleModel(mesocyclesModel);
			m_MesoManager.append(newMeso);
			connect(newMeso, SIGNAL(pageReady(QQuickItem*,uint)), this, SLOT(bridge(QQuickItem*,uint)));
			connect(newMeso, SIGNAL(itemReady(QQuickItem*,uint)), this, SIGNAL(getItem(QQuickItem*,uint)));
		}
		mesoCalendarModel->clear();
		m_MesoId = mesoId;
		m_MesoIdx = mesoIdx;
		m_MesoIdStr = QString::number(m_MesoId);
	}
}

void DbManager::removeWorkingMeso()
{
	delete m_MesoManager.at(m_MesoIdx);
	m_MesoManager.remove(m_MesoIdx);
	if (m_MesoIdx > 0)
	{
		--m_MesoIdx;
		setWorkingMeso(m_MesoManager.at(m_MesoIdx)->mesoId(), m_MesoIdx);
	}
	else
		m_MesoId = -2;
}

void DbManager::gotResult(TPDatabaseTable* dbObj)
{
	if (dbObj->result())
	{
		switch (static_cast<DBMesocyclesTable*>(dbObj)->opCode())
		{
			default:
			break;
			case OP_DELETE_TABLE:
				dbObj->createTable();
			break;
			case OP_UPDATE_LIST:
				if (static_cast<DBExercisesTable*>(dbObj)->opCode() == OP_UPDATE_LIST)
					m_appSettings->setValue("exercisesListVersion", m_exercisesListVersion);
			break;
			case OP_ADD:
				if (dbObj->objectName() == DBMesocyclesObjectName)
				{
					m_MesoId = static_cast<DBMesocyclesTable*>(dbObj)->data().at(0).toUInt();
					m_MesoManager.at(m_MesoIdx)->setMesoId(m_MesoId);
					m_MesoManager.at(m_MesoIdx)->getMesoPage()->setProperty("mesoId", m_MesoId);
					m_MesoIdStr = QString::number(m_MesoId);
				}
			break;
			case OP_READ:
				if (dbObj->objectName() == DBMesoCalendarObjectName)
				{
					if (mesoCalendarModel->count() == 0)
					{
						mesoCalendarModel->createModel( m_MesoId, mesocyclesModel->getDateFast(m_MesoIdx, 2),
								mesocyclesModel->getDateFast(m_MesoIdx, 3), mesocyclesModel->getFast(m_MesoIdx, 6) );
						createMesoCalendar();
					}
				}
				else if (dbObj->objectName() == DBTrainingDayObjectName)
				{
					DBTrainingDayModel* tempModel(static_cast<DBTrainingDayModel*>(static_cast<DBTrainingDayTable*>(dbObj)->model()));
					m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("previousTDays", tempModel->count() > 0 ?
						QVariant::fromValue(tempModel->getRow_const(0)) : QVariant::fromValue(QVariantList()));
					delete tempModel;
				}
			break;
			case OP_DEL:
				if (dbObj->objectName() == DBMesocyclesObjectName)
					removeWorkingMeso();
			break;
		}
	}

	m_WorkerLock[dbObj->objectName()]--;
	if (m_WorkerLock[dbObj->objectName()] <= 0)
		cleanUp(dbObj);
}

void DbManager::startThread(QThread* thread, TPDatabaseTable* dbObj)
{
	if (!thread->isFinished())
	{
		MSG_OUT("starting thread for " << dbObj->objectName())
		m_WorkerLock[dbObj->objectName()]++;
		thread->start();
	}
}

void DbManager::cleanUp(TPDatabaseTable* dbObj)
{
	MSG_OUT("cleanUp: " << dbObj->objectName());
	dbObj->disconnect();
	dbObj->deleteLater();
	dbObj->thread()->quit();
	MSG_OUT("calling databaseReady()");
	emit databaseReady();
}

void DbManager::createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc )
{
	worker->setCallbackForDoneFunc( [&] (TPDatabaseTable* obj) { return gotResult(obj); } );

	QThread *thread = new QThread ();
	connect ( thread, &QThread::started, worker, execFunc );
	connect ( thread, &QThread::finished, thread, &QThread::deleteLater);
	worker->moveToThread ( thread );

	if (m_WorkerLock[worker->objectName()] <= 0)
		startThread(thread, worker);
	else
	{
		MSG_OUT("Database  " << worker->objectName() << "  Waiting for it to be free: " << m_WorkerLock[worker->objectName()])
		connect( this, &DbManager::databaseReady, this, [&,thread, worker] () {
			return DbManager::startThread(thread, worker); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	}
}

void DbManager::bridge(QQuickItem* item, const uint id) {
	MSG_OUT("bridge  id " << id)
	MSG_OUT("bridge item " << item->objectName())
	if (id == m_expectedPageId)
	{
		emit getPage(item, id);
		emit internalSignal(id);
	}
}

//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
void DbManager::getAllExercises()
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	createThread(worker, [worker] () { worker->getAllExercises(); } );
}

void DbManager::newExercise( const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(QStringLiteral("0"), mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { worker->newExercise(); } );
}

void DbManager::updateExercise( const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath )
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(id, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath);
	createThread(worker, [worker] () { return worker->updateExercise(); } );
}

void DbManager::removeExercise(const QString& id)
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	worker->setData(id);
	createThread(worker, [worker] () { return worker->removeExercise(); } );
}

void DbManager::deleteExercisesTable()
{
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	createThread(worker, [worker] () { return worker->deleteExercisesTable(); } );
}

void DbManager::openExercisesListPage()
{
	if (m_exercisesPage != nullptr)
	{
		emit getPage(m_exercisesPage, 89676);
		return;
	}
	DBExercisesTable* worker(new DBExercisesTable(m_DBFilePath, m_appSettings, exercisesListModel));
	connect( this, &DbManager::databaseReady, this, &DbManager::createExercisesListPage, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	createThread(worker, [worker] () { return worker->getAllExercises(); } );

	m_exercisesComponent = new QQmlComponent(m_QMlEngine, QUrl(u"qrc:/qml/ExercisesDatabase.qml"_qs), QQmlComponent::Asynchronous);
	connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [&](QQmlComponent::Status) {
		return createExercisesListPage(); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
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
	if (exercisesListModel->isReady() && m_exercisesComponent->status() == QQmlComponent::Ready)
	{
		if (m_exercisesPage == nullptr)
		{
			m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->create(m_QMlEngine->rootContext()));
			m_QMlEngine->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
			QQuickWindow* parent(static_cast<QQuickWindow*>(m_QMlEngine->rootObjects().at(0)));
			m_exercisesPage->setParentItem(parent->contentItem());
			emit getPage(m_exercisesPage, 999);
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
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->getAllMesocycles();
	const int current_meso_idx(mesocyclesModel->count()-1);
	if (current_meso_idx >= 0)
		setWorkingMeso(mesocyclesModel->getInt(static_cast<uint>(current_meso_idx), 0), static_cast<uint>(current_meso_idx));
	delete worker;
}

void DbManager::getMesocycle(const uint meso_idx)
{
	if (meso_idx != m_MesoIdx)
		setWorkingMeso(mesocyclesModel->getInt(meso_idx, 0), meso_idx);

	if (m_MesoManager.at(m_MesoIdx)->getMesoPage() != nullptr)
	{
		emit getPage(m_MesoManager.at(m_MesoIdx)->getMesoPage(), mesoPageCreateId);
		return;
	}
	m_expectedPageId = mesoPageCreateId;
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
	setWorkingMeso(-1, mesocyclesModel->count() - 1);
	m_expectedPageId = mesoPageCreateId;
	m_MesoManager.at(m_MesoIdx)->createMesocyclePage(minimumStartDate, bRealMeso ?
			m_runCommands->createFutureDate(startDate,0,6,0) : QDate(2026,11,31), startDate);
}

void DbManager::newMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate, const QString& mesoNote,
						const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->addExecArg(m_MesoIdx);
	worker->setData(QString(), mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->newMesocycle(); } );
}

void DbManager::updateMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
				const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs)
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	worker->addExecArg(m_MesoIdx);
	worker->setData(m_MesoIdStr, mesoName, QString::number(mesoStartDate.toJulianDay()), QString::number(mesoEndDate.toJulianDay()),
						mesoNote, mesoWeeks, mesoSplit, mesoDrugs);
	createThread(worker, [worker] () { worker->updateMesocycle(); } );
}

void DbManager::removeMesocycle()
{
	if (mesocyclesModel->getInt(m_MesoIdx, 0) == -1)
	{
		mesocyclesModel->removeFromList(m_MesoIdx);
		removeWorkingMeso();
	}
	else
	{
		removeMesoCalendar();
		removeMesoSplit();
		DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
		worker->addExecArg(m_MesoIdx);
		worker->setData(m_MesoIdStr);
		createThread(worker, [worker] () { return worker->removeMesocycle(); } );
	}
}

void DbManager::deleteMesocyclesTable()
{
	DBMesocyclesTable* worker(new DBMesocyclesTable(m_DBFilePath, m_appSettings, mesocyclesModel));
	createThread(worker, [worker] () { return worker->deleteMesocyclesTable(); } );
}
//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
void DbManager::getMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoId);
	createThread(worker, [worker] () { worker->getMesoSplit(); } );
}

void DbManager::newMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->setData(m_MesoIdStr, splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->newMesoSplit(); } );
}

void DbManager::updateMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
								const QString& splitD, const QString& splitE, const QString& splitF)
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoIdx);
	worker->setData(m_MesoIdStr, splitA, splitB, splitC, splitD, splitE, splitF);
	createThread(worker, [worker] () { worker->updateMesoSplit(); } );
}

void DbManager::removeMesoSplit()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
	worker->addExecArg(m_MesoIdx);
	worker->setData(m_MesoIdStr);
	createThread(worker, [worker] () { return worker->removeMesoSplit(); } );
}

void DbManager::deleteMesoSplitTable()
{
	DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, mesoSplitModel));
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
			emit getPage(m_MesoManager.at(m_MesoIdx)->getSplitPage(splitLetter), static_cast<int>(splitLetter.toLatin1()) - static_cast<int>('A'));
			continue;
		}

		if (createdSplits.indexOf(splitLetter) == -1)
		{
			createdSplits.append(splitLetter);
			DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->getSplitModel(splitLetter)));
			worker->addExecArg(m_MesoId);
			worker->addExecArg(static_cast<QChar>(*itr));
			connect( this, &DbManager::databaseReady, this, [&] { return m_MesoManager.at(m_MesoIdx)->createMesoSplitPage(); },
						static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
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

bool DbManager::mesoHasPlan(const uint meso_id, const QString& splitLetter) const
{
	if (splitLetter != u"R"_qs)
	{
		DBMesoSplitTable* meso_split(new DBMesoSplitTable(m_DBFilePath, m_appSettings));
		const bool ret(meso_split->mesoHasPlan(QString::number(meso_id), splitLetter));
		delete meso_split;
		return ret;
	}
	return false;
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
void DbManager::getMesoCalendar(const bool bCreatePage)
{
	if (!mesoCalendarModel->isReady())
	{
		DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
		worker->addExecArg(m_MesoId);
		if (bCreatePage)
			connect( this, &DbManager::databaseReady, this, [&,bCreatePage] {
				return getMesoCalendar(bCreatePage); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		createThread(worker, [worker] () { worker->getMesoCalendar(); });
		return;
	}
	if (bCreatePage)
	{
		if (m_MesoManager.at(m_MesoIdx)->getCalendarPage() != nullptr)
		{
			emit getPage(m_MesoManager.at(m_MesoIdx)->getCalendarPage(), calPageCreateId);
			return;
		}
		m_MesoManager.at(m_MesoIdx)->setMesoCalendarModel(mesoCalendarModel);
		m_expectedPageId = calPageCreateId;
		m_MesoManager.at(m_MesoIdx)->createMesoCalendarPage();
	}
}

void DbManager::createMesoCalendar()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	createThread(worker, [worker] () { worker->createMesoCalendar(); } );
}

void DbManager::changeMesoCalendar(const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday)
{
	if (!mesoCalendarModel->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,newStartDate,newEndDate,newSplit,bPreserveOldInfo,bPreserveOldInfoUntilToday] ()
		{
			return changeMesoCalendar(newStartDate,newEndDate,newSplit,bPreserveOldInfo,bPreserveOldInfoUntilToday);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(m_MesoId);
	worker->addExecArg(newStartDate);
	worker->addExecArg(newEndDate);
	worker->addExecArg(newSplit);
	worker->addExecArg(bPreserveOldInfo);
	worker->addExecArg(bPreserveOldInfoUntilToday);
	createThread(worker, [worker] () { worker->changeMesoCalendar(); } );
}

void DbManager::updateMesoCalendarModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter, const QString& tDay)
{
	if (!mesoCalendarModel->isReady())
	{
		connect(this, &DbManager::databaseReady, this, [&,mesoSplit,startDate,splitLetter,tDay] ()
		{
			return updateMesoCalendarModel(mesoSplit,startDate,splitLetter,tDay);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		getMesoCalendar(false);
		return;
	}
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(m_MesoId); //needed for DBMesoCalendarTable::removeMesoCalendar()
	worker->addExecArg(mesoSplit);
	worker->addExecArg(startDate);
	worker->addExecArg(splitLetter);
	worker->addExecArg(tDay);
	createThread(worker, [worker] () { worker->updateMesoCalendar(); } );
}

void DbManager::updateMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit)
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(calDate);
	worker->setData(m_MesoIdStr, QString::number(calNDay), calSplit);
	createThread(worker, [worker] () { worker->updateMesoCalendarEntry(); } );
}

void DbManager::removeMesoCalendar()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	worker->addExecArg(m_MesoIdStr);
	createThread(worker, [worker] () { return worker->removeMesoCalendar(); } );
}

void DbManager::deleteMesoCalendarTable()
{
	DBMesoCalendarTable* worker(new DBMesoCalendarTable(m_DBFilePath, m_appSettings, mesoCalendarModel));
	createThread(worker, [worker] () { return worker->deleteMesoCalendarTable(); } );
}
//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
void DbManager::getTrainingDay(const QDate& date)
{
	if (m_MesoManager.at(m_MesoIdx)->gettDayPage(date) != nullptr)
	{
		m_MesoManager.at(m_MesoIdx)->setCurrenttDay(date);
		emit getPage(m_MesoManager.at(m_MesoIdx)->gettDayPage(date), tDayPageCreateId);
		return;
	}

	m_expectedPageId = tDayPageCreateId;
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->gettDayModel(date)));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date] { return m_MesoManager.at(m_MesoIdx)->createTrainingDayPage(date); },
			static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	connect(this, &DbManager::internalSignal, this, [&,date] (const uint id )
		{ if (id == tDayPageCreateId) return getTrainingDayExercises(date); }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	createThread(worker, [worker] () { return worker->getTrainingDay(); } );
}

void DbManager::getTrainingDayExercises(const QDate& date)
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date] { return verifyTDayOptions(date); },
		static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); } );
}

void DbManager::verifyTDayOptions(const QDate& date, const QString& splitLetter)
{
	if (m_MesoManager.at(m_MesoIdx)->currenttDayModel()->exercisesNumber() > 0)
	{
		m_MesoManager.at(m_MesoIdx)->createExercisesObjects();
		return;
	}

	const QString splitletter(splitLetter.isEmpty() ? mesoCalendarModel->getSplitLetter(date.month(), date.day()-1) : splitLetter);
	m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasMesoPlan", mesoHasPlan(m_MesoId, splitletter));

	if (splitletter >= u"A"_qs && splitletter <= u"F"_qs)
	{
		DBTrainingDayModel* tempModel(new DBTrainingDayModel(this));
		DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, tempModel));
		worker->addExecArg(splitletter);
		worker->addExecArg(QString::number(date.toJulianDay()));
		createThread(worker, [worker] () { return worker->getPreviousTrainingDays(); } );
	}

	m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasPreviousTDays", false);
}

void DbManager::loadExercisesFromDate(const QString& strDate)
{
	const QDate date(m_runCommands->getDateFromStrDate(strDate));
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	worker->addExecArg(QString::number(date.toJulianDay()));
	connect( this, &DbManager::databaseReady, this, [&,date] {
		m_MesoManager.at(m_MesoIdx)->currenttDayModel()->setModified(true);
		return m_MesoManager.at(m_MesoIdx)->createExercisesObjects();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
	createThread(worker, [worker] () { return worker->getTrainingDayExercises(); });
	QMetaObject::invokeMethod(m_MesoManager.at(m_MesoIdx)->currenttDayPage(), "createNavButtons");
	m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasMesoPlan", false);
	m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasPreviousTDays", false);
}

void DbManager::loadExercisesFromMesoPlan(const QString& splitLetter)
{
	const QChar splitletter(splitLetter.at(0));
	if (!m_MesoManager.at(m_MesoIdx)->getSplitModel(splitletter)->isReady())
	{
		DBMesoSplitTable* worker(new DBMesoSplitTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->getSplitModel(splitletter)));
		worker->addExecArg(m_MesoId);
		worker->addExecArg(splitLetter.at(0));
		connect( this, &DbManager::databaseReady, this, [&,splitLetter] { return loadExercisesFromMesoPlan(splitLetter); },
				static_cast<Qt::ConnectionType>(Qt::SingleShotConnection) );
		createThread(worker, [worker] () { return worker->getCompleteMesoSplit(); } );
	}
	else
	{
		QMetaObject::invokeMethod(m_MesoManager.at(m_MesoIdx)->currenttDayPage(), "createNavButtons");
		m_MesoManager.at(m_MesoIdx)->currenttDayModel()->convertMesoModelToTDayModel(m_MesoManager.at(m_MesoIdx)->getSplitModel(splitletter));
		m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasMesoPlan", false);
		m_MesoManager.at(m_MesoIdx)->currenttDayPage()->setProperty("bHasPreviousTDays", false);
		m_MesoManager.at(m_MesoIdx)->createExercisesObjects();
	}
}

void DbManager::newTrainingDay()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->newTrainingDay(); } );
}

void DbManager::updateTrainingDay()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->updateTrainingDay(); } );
}

void DbManager::updateTrainingDayExercises()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->updateTrainingDayExercises(); } );
}

void DbManager::removeTrainingDay()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->removeTrainingDay(); } );
}

void DbManager::deleteTrainingDayTable()
{
	DBTrainingDayTable* worker(new DBTrainingDayTable(m_DBFilePath, m_appSettings, m_MesoManager.at(m_MesoIdx)->currenttDayModel()));
	createThread(worker, [worker] () { return worker->deleteTrainingDayTable(); } );
}

void DbManager::createExerciseObject(const QString& exerciseName, const QString& nSets, const QString& nReps, const QString& nWeight)
{
	m_MesoManager.at(m_MesoIdx)->createExerciseObject(exerciseName, nSets, nReps, nWeight);
}

void DbManager::removeExerciseObject(const uint exercise_idx)
{
	m_MesoManager.at(m_MesoIdx)->removeExercise(exercise_idx);
}

void DbManager::moveExerciseObject(const uint exercise_idx, const uint new_idx)
{
	m_MesoManager.at(m_MesoIdx)->moveExercise(exercise_idx, new_idx);
}

QQuickItem* DbManager::getExerciseObject(const uint exercise_idx)
{
	return m_MesoManager.at(m_MesoIdx)->getExerciseObject(exercise_idx);
}

void DbManager::createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const QString& nReps, const QString& nWeight)
{
	if (set_number == 0)
		m_MesoManager.at(m_MesoIdx)->currenttDayModel()->newFirstSet(exercise_idx, set_type, nReps, nWeight);
	else
		m_MesoManager.at(m_MesoIdx)->currenttDayModel()->newSet(exercise_idx, set_number, set_type);
	m_MesoManager.at(m_MesoIdx)->createSetObject(set_type, set_number, exercise_idx);
}

void DbManager::createSetObjects(const uint exercise_idx)
{
	if (!m_MesoManager.at(m_MesoIdx)->setsLoaded(exercise_idx))
	{
		const uint nsets(m_MesoManager.at(m_MesoIdx)->currenttDayModel()->setsNumber(exercise_idx));
		for (uint i(0); i < nsets; ++i)
			m_MesoManager.at(m_MesoIdx)->createSetObject(m_MesoManager.at(m_MesoIdx)->currenttDayModel()->setType(i, exercise_idx), i, exercise_idx);
	}
}

void DbManager::removeSetObject(const uint set_number, const uint exercise_idx)
{
	m_MesoManager.at(m_MesoIdx)->removeSet(set_number, exercise_idx);
}
//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
