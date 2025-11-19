#pragma once

#include <QHash>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(TPDatabaseTable);

class ThreadManager : public QObject
{

Q_OBJECT

public:

	enum StandardOps {
		ReadAllRecords,
		CustomOperation,
		CreateTable,
		InsertRecord,
		InsertRecords,
		UpdateOneField,
		UpdateSeveralFields,
		UpdateRecords,
		DeleteRecord,
		RemoveTemporaries,
		ClearTable,
	};

	explicit inline ThreadManager(QObject *parent = nullptr) : QObject{parent} {}
	inline ThreadManager(const ThreadManager &other) = delete;
	inline ThreadManager &operator()(const ThreadManager &other) = delete;
	inline ~ThreadManager() {}

	void executeExternalQuery(const QString &dbfilename, const QString &query);
	void runAction(TPDatabaseTable *worker, StandardOps operation);

signals:
	void databaseReady(const bool success, const int table_id, const bool has_cmd_file);
	void newThreadedOperation(const int unique_id, ThreadManager::StandardOps operation);

private:
	QHash<int,QThread*> m_subThreadsList;

	static ThreadManager *app_thread_mngr;
	friend ThreadManager *appThreadManager();
};

inline ThreadManager *appThreadManager() { return ThreadManager::app_thread_mngr; }
