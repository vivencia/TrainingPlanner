#include "thread_manager.h"

#include "tpdatabasetable.h"

#include <QThread>

ThreadManager *ThreadManager::app_thread_mngr{nullptr};

void ThreadManager::runAction(TPDatabaseTable *worker, StandardOps operation )
{
	QThread* thread{m_subThreadsList.value(worker->tableId())};
	if (!thread)
	{
		thread = new QThread{};
		worker->moveToThread(thread);
		m_subThreadsList.insert(worker->tableId(), thread);

		 // Connect the thread's finished signal to delete both the thread and the worker
		QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
		QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

		connect(this, &ThreadManager::newThreadedOperation, worker, &TPDatabaseTable::startAction, Qt::BlockingQueuedConnection);
		connect(worker, &TPDatabaseTable::actionFinished, this, [this,worker] (const bool success, const bool has_cmd_file) {
			emit databaseReady(success, worker->tableId(), has_cmd_file);
		});

		//Enters the thread's event loop, which waits for queued signal deliveries. and sleeps when idle
		thread->start();
	}
	emit newThreadedOperation(worker->uniqueId(), operation);
}
