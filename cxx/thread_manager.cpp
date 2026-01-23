#include "thread_manager.h"

#include "tpdatabasetable.h"

#include <QApplication>
#include <QThread>

ThreadManager *ThreadManager::app_thread_mngr{nullptr};

struct ThreadManager::stQueuedOps
{
	int uniqueid;
	StandardOps op;
	void *extra_param;
	QTimer timer;
};

ThreadManager::ThreadManager(QObject *parent)
	: QObject{parent}
{
	app_thread_mngr = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));
}

void ThreadManager::initThread(TPDatabaseTable *worker)
{
	QThread *thread{m_subThreadsList.value(worker->tableId())};
	if (!thread)
	{
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

void ThreadManager::runAction(TPDatabaseTable *worker, StandardOps operation, void* extra_param)
{
	initThread(worker);
	emit newThreadedOperation(worker->uniqueId(), operation, extra_param);
}

void ThreadManager::queueAction(TPDatabaseTable *worker, StandardOps operation, void *extra_param)
{
	initThread(worker);
	ThreadManager::stQueuedOps* cur_ops{m_queuedOps.value(worker->uniqueId())};
	if (!cur_ops)
	{
		ThreadManager::stQueuedOps *new_op{new ThreadManager::stQueuedOps};
		new_op->uniqueid = worker->uniqueId();
		new_op->op = operation;
		new_op->extra_param = extra_param;
		new_op->timer.callOnTimeout([this,new_op] () {
			emit newThreadedOperation(new_op->uniqueid, new_op->op, new_op->extra_param);
			new_op->timer.stop();
		});
		m_queuedOps.insert(worker->uniqueId(), new_op);
		return;
	}

	switch (operation)
	{
		case CustomOperation:
		case DeleteRecords:
		case ClearTable:
		case ReadAllRecords:
			cur_ops->timer.stop();
			emit newThreadedOperation(worker->uniqueId(), operation, extra_param);
		default:
			return;
		break;
		case UpdateOneField:
			if (cur_ops->op  == UpdateOneField)
				cur_ops->op = UpdateSeveralFields;
			else if (cur_ops->op == InsertRecords)
				cur_ops->op = AlterRecords;
		break;
		case UpdateSeveralFields:
		case UpdateRecords:
			if (cur_ops->op == InsertRecords)
				cur_ops->op = AlterRecords;
		break;
		case InsertRecords:
			if (cur_ops->op != InsertRecords)
				cur_ops->op = AlterRecords;
		break;
	}
	cur_ops->timer.start(10000);
}

void ThreadManager::aboutToExit()
{
	for (const auto queued_op : std::as_const(m_queuedOps))
		queued_op->timer.start(1);
	for (QThread *thread : std::as_const(m_subThreadsList))
	{
		thread->quit();
		thread->wait();
	}
}
