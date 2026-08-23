#include "thread_manager.h"

#include "tpdatabasetable.h"

#include <QApplication>
#include <QThread>

ThreadManager *ThreadManager::app_thread_mngr{nullptr};

struct ThreadManager::stQueuedOps
{
	StandardOps op;
	QTimer timer;
	DBModelInterface *data{nullptr};
	TPDatabaseTable *worker{nullptr};
};

ThreadManager::ThreadManager(QObject *parent) : QObject{parent}
{
	app_thread_mngr = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));
}

void ThreadManager::initThread(TPDatabaseTable *worker)
{
	QThread *thread{m_subThreadsList.value(worker->tableId())};
	if (!thread) {
		thread = new QThread{};
		worker->moveToThread(thread);
		m_subThreadsList.insert(worker->tableId(), thread);

		// Connect the thread's finished signal to delete both the thread and the worker
		QObject::connect(thread, &QThread::finished, worker, &TPDatabaseTable::deleteLater);
		QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
		connect(this, &ThreadManager::newThreadedOperation, worker, &TPDatabaseTable::startAction, Qt::QueuedConnection);

		//Enters the thread's event loop, which waits for queued signal deliveries. and sleeps when idle
		thread->start();
	}
}

//The purpose of stQueuedOps is not to stall the execution of a thread if there a previou job running(this is accomplished
//by the mutex on TPDatabaseTable; it is to avoid succesives calls that might overlap; i. e.: the AlterRecords function
//can deal with insertions and updates and UpdataSeveralFields does what the name says. So, instead of several small
//database alteration calls, wait to see if in the next five seconds, other calls might be made and, possibly,
//save some write operations
void ThreadManager::runAction(TPDatabaseTable *worker, StandardOps operation, DBModelInterface *data)
{
	bool do_timer{false};
	switch (operation) {
	case NoOp:
		return;
	case DeleteRecords:
	case AlterRecords:
		do_timer = true;
		break;
	case UpdateOneField:
		if (operation == UpdateOneField) //accumulate more than one field
			operation = UpdateSeveralFields;
		else if (operation == InsertRecords) //an update and one or more insertions: use alter records that can deal with both
			operation = AlterRecords;
		break;
	case UpdateSeveralFields: //already an accumulation
	case UpdateRecords:
		if (operation == InsertRecords) //updates and one or more insertions: use alter records that can deal with both
			operation = AlterRecords;
		break;
	case InsertRecords:
		if (operation != InsertRecords) //updates and one or more insertions, use alter records that can deal with both
			operation = AlterRecords;
		break;
	default: break;
	}

	initThread(worker);
	if (!do_timer) {
		emit newThreadedOperation(worker->uniqueId(), operation, data);
	} else {
		ThreadManager::stQueuedOps* cur_ops{m_queuedOps.value(worker->uniqueId())};
		if (!cur_ops) {
			ThreadManager::stQueuedOps *new_op{new ThreadManager::stQueuedOps};
			new_op->op = operation;
			new_op->data = data;
			new_op->worker = worker;
			new_op->timer.callOnTimeout( [this,new_op] () {
				qDebug() << "emit newThreadedOperation(" << new_op->worker->uniqueId() << ", " << new_op->op << ")";
				emit newThreadedOperation(new_op->worker->uniqueId(), new_op->op, new_op->data);
			});
			m_queuedOps.insert(worker->uniqueId(), new_op);
			return;
		} else { //if there are queued operations, reset timer
			if (cur_ops->timer.isActive())
				cur_ops->timer.stop();
		}
		cur_ops->timer.start(5000);
	}
}

void ThreadManager::startUnManagedThread(QObject *worker)
{
	QThread *thread{new QThread{}};
	worker->moveToThread(thread);
	// Connect the thread's finished signal to delete both the thread and the worker
	QObject::connect(thread, &QThread::finished, worker, &TPDatabaseTable::deleteLater);
	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
	//Enters the thread's event loop, which waits for signal deliveries. and sleeps when idle
	thread->start();
}

void ThreadManager::aboutToExit()
{
	if (!m_queuedOps.isEmpty()) {
		for (const auto queued_op : std::as_const(m_queuedOps))
			emit newThreadedOperation(queued_op->worker->uniqueId(), queued_op->op, queued_op->data);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	for (QThread *thread : std::as_const(m_subThreadsList)) {
		thread->quit();
		thread->wait();
	}
}
