#pragma once

#include <QHash>
#include <QObject>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(TPDatabaseTable)

class ThreadManager : public QObject
{

Q_OBJECT

public:

	enum StandardOps {
		NoOp,
		ReadAllRecords,
		CustomOperation,
		CreateTable,
		InsertRecords,
		AlterRecords,
		UpdateOneField,
		UpdateSeveralFields,
		UpdateRecords,
		DeleteRecords,
		RemoveTemporaries,
		ClearTable,
	};

	explicit ThreadManager(QObject *parent = nullptr);
	inline ThreadManager(const ThreadManager &other) = delete;
	inline ThreadManager &operator()(const ThreadManager &other) = delete;
	inline ~ThreadManager() {}

	void runAction(TPDatabaseTable *worker, StandardOps operation, void *extra_param = nullptr);
	void queueAction(TPDatabaseTable *worker, StandardOps operation, void *extra_param = nullptr);

signals:
	void newThreadedOperation(const int unique_id, ThreadManager::StandardOps operation, void *extra_param);

public slots:
	void aboutToExit();

private:
	QT_FORWARD_DECLARE_STRUCT(stQueuedOps)
	QHash<int,QThread*> m_subThreadsList;
	QHash<int,ThreadManager::stQueuedOps*> m_queuedOps;
	static ThreadManager *app_thread_mngr;
	friend ThreadManager *appThreadManager();

	void initThread(TPDatabaseTable *worker);
};

inline ThreadManager *appThreadManager() { return ThreadManager::app_thread_mngr; }
