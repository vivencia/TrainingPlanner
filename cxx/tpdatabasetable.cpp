#include "tpdatabasetable.h"

#include "dbmodelinterface.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>

#include <ranges>

using namespace QLiterals;

TPDatabaseTable::TPDatabaseTable(const uint table_id, DBModelInterface *dbmodel_interface)
	: QObject{nullptr}, m_tableId{table_id}, m_dbModelInterface{dbmodel_interface},
		m_deleteAfterFinished{false}
{
	m_threadedFunctions.insert(ThreadManager::CustomOperation, [this] (void*) {
		if (m_customQueryFunc)
		{
			auto result{m_customQueryFunc()};
			emit actionFinished(ThreadManager::CustomOperation, result.first, result.second);
		}
		else
			emit actionFinished(ThreadManager::CustomOperation, QVariant{}, QVariant{});
	});
	m_threadedFunctions.insert(ThreadManager::CreateTable, [this] (void*) {
		auto result{createTable()};
		emit actionFinished(ThreadManager::CreateTable, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::InsertRecords, [this] (void*) {
		auto result{insertRecord()};
		emit actionFinished(ThreadManager::InsertRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::AlterRecords, [this] (void*) {
		auto result{AlterRecords()};
		emit actionFinished(ThreadManager::AlterRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateOneField, [this] (void*) {
		auto result{updateRecord()};
		emit actionFinished(ThreadManager::UpdateOneField, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateSeveralFields, [this] (void*) {
		auto result{updateFieldsOfRecord()};
		emit actionFinished(ThreadManager::UpdateSeveralFields, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateRecords, [this] (void*) {
		auto result{updateRecords()};
		emit actionFinished(ThreadManager::UpdateRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::DeleteRecords, [this] (void*) {
		auto result{removeRecords()};
		emit actionFinished(ThreadManager::DeleteRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::RemoveTemporaries, [this] (void*) {
		auto result{removeTemporaries()};
		emit actionFinished(ThreadManager::RemoveTemporaries, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::ClearTable, [this] (void*) {
		auto result{clearTable()};
		emit actionFinished(ThreadManager::ClearTable, result.first, result.second);
	});

	connect(this, &TPDatabaseTable::actionFinished, this, [this]
				(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2)
	{
		m_sqlLiteDB.close();
		switch (op)
		{
			case ThreadManager::ReadAllRecords: break;
			case ThreadManager::CustomOperation: break;
			default:
				if (return_value2.toBool())
				{
					if (appUserModel()->mainUserLoggedIn())
						emit appUserModel()->cmdFileCreated(dbFilePath());
				}
			break;
		}
	});
}

void TPDatabaseTable::startAction(const int unique_id, ThreadManager::StandardOps operation, void *extra_param)
{
	if (unique_id == uniqueId())
	{
		if (m_threadedFunctions.contains(operation))
			m_threadedFunctions.value(operation)(extra_param);
		#ifndef QT_NO_DEBUG
		else
			qDebug() << "Cannot start action: " << operation << " for table: " << unique_id <<
																	" because it's not inserted in the functions container";
		#endif
	}
}

void TPDatabaseTable::setUpConnection()
{
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{*m_tableName % "_connection"_L1 % QString::number(m_uniqueID)};
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));

	QString dbfilename{std::move(dbFileName())};
	m_sqlLiteDB.setDatabaseName(dbfilename);
	if (!m_databaseFilenamesPool.contains(dbfilename))
		m_databaseFilenamesPool.append(std::move(dbfilename));
}

QString TPDatabaseTable::dbFilePath() const
{
	return appSettings()->currentUserDir() + subDir();
}

std::pair<bool, bool> TPDatabaseTable::createTable()
{
	bool success{true}, cmd_ok{false};
	if (!QFile::exists(dbFileName()))
	{
		if (appUtils()->mkdir(dbFilePath()))
		{
			m_strQuery = {std::move("CREATE TABLE IF NOT EXISTS "_L1 % *m_tableName % " ("_L1)};
			for (uint i{0}; i < m_fieldCount; ++i)
				m_strQuery += std::move(m_fieldNames[i][0] % ' ' % m_fieldNames[i][1]) % ',';
			m_strQuery.chop(1);
			m_strQuery += std::move(");"_L1);
			success = execSingleWriteQuery(m_strQuery);
			if (success)
				cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
		}
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool, bool> TPDatabaseTable::insertRecord()
{
	bool success{false}, cmd_ok{false};
	const uint modified_row{m_dbModelInterface->modifiedIndices().cbegin().key()};
	const QStringList &data{m_dbModelInterface->modelData().at(modified_row)};
	const bool auto_increment{m_fieldNames[0][1].contains("AUTOINCREMENT"_L1)};

	m_strQuery = std::move("INSERT INTO "_L1 % *m_tableName % " ("_L1);
	for (int i{auto_increment ? 1 : 0}; i < m_fieldCount; ++i)
		m_strQuery += std::move(m_fieldNames[i][0] % ',');
	m_strQuery.chop(1);
	m_strQuery += std::move(") VALUES ("_L1);

	QMap<uint, QList<int>>::const_iterator itr{m_dbModelInterface->modifiedIndices().constBegin()};
	const QMap<uint, QList<int>>::const_iterator itr_end{m_dbModelInterface->modifiedIndices().constEnd()};
	while (itr != itr_end)
	{
		auto modified_row{itr.key()};
		uint field{0};

		for (const auto &data : std::as_const(m_dbModelInterface->modelData().at(modified_row)))
		{
			if (field != 0 || !auto_increment)
				m_strQuery += std::move((m_fieldNames[field][1] == "TEXT"_L1 ? QString{'\'' % data % '\''} : data) % ',');
			++field;
		}
		m_strQuery.chop(1);
		m_strQuery += std::move("),("_L1);
		++itr;
	}
	m_strQuery.chop(2);
	m_strQuery += ';';

	if (execSingleWriteQuery(m_strQuery))
	{
		if (auto_increment)
		{
			int last_id{m_workingQuery.lastInsertId().toInt()};
			auto n{m_dbModelInterface->modifiedIndices().count()};
			for (auto &data : m_dbModelInterface->modelData() | std::views::reverse)
			{
				data[0] = std::move(QString::number(last_id--));
				if (--n == 0)
					break;
			}
		}
		success = true;
		m_strQuery.prepend("PRAGMA busy_timeout = 5000;"_L1);
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	m_dbModelInterface->modifiedIndices().clear();
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::AlterRecords()
{
	bool success{false}, cmd_ok{false};
	bool has_insert{false};
	const bool auto_increment{m_fieldNames[0][1].contains("AUTOINCREMENT"_L1)};
	QString str_query;
	QStringList queries;
	const QString &update_cmd{"UPDATE "_L1 % *m_tableName % " SET "_L1};
	QString insert_cmd{std::move("INSERT INTO "_L1 % *m_tableName % " ("_L1)};
	for (int i{auto_increment ? 1 : 0}; i < m_fieldCount; ++i)
		insert_cmd += std::move(m_fieldNames[i][0] % ',');
	insert_cmd.chop(1);
	insert_cmd += std::move(") VALUES ("_L1);

	QMap<uint, QList<int>>::const_iterator itr{m_dbModelInterface->modifiedIndices().constBegin()};
	const QMap<uint, QList<int>>::const_iterator itr_end{m_dbModelInterface->modifiedIndices().constEnd()};
	while (itr != itr_end)
	{
		auto modified_row{itr.key()};
		if (m_dbModelInterface->modelData().at(modified_row).at(0).toInt() < 0 || itr.value().at(0) < 0)
		{
			int field{0};
			has_insert = true;
			str_query = insert_cmd;
			for (const auto &data : std::as_const(m_dbModelInterface->modelData().at(modified_row)))
			{
				if (field != 0 || !auto_increment)
					str_query += std::move((m_fieldNames[field][1] == "TEXT"_L1 ? QString{'\'' % data % '\''} : data) % ',');
				++field;
			}
			str_query.chop(1);
			str_query.append(");"_L1);
		}
		else
		{
			str_query = update_cmd;
			for (const auto field : std::as_const(itr.value()))
			{
				str_query += std::move(m_fieldNames[field][0] % '=' % (m_fieldNames[field][1] == "TEXT"_L1 ?
						'\'' % m_dbModelInterface->modelData().at(modified_row).at(field) % '\'' :
									m_dbModelInterface->modelData().at(modified_row).at(field)) % ',');
			}
			str_query.chop(1);
			const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
			str_query += std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
		}
		++itr;
		queries.append(std::move(str_query));
	}
	m_dbModelInterface->clearModifiedIndices();

	const bool query_id_back{has_insert && auto_increment};
	if (execMultipleWritesQuery(queries))
	{
		if (query_id_back)
		{
			int last_insert_id{m_workingQuery.lastInsertId().toInt()};
			for (auto &data : m_dbModelInterface->modelData() | std::views::reverse)
			{
				if (data.at(0).toInt() < 0)
					data[0] = std::move(QString::number(last_insert_id--));
			}
		}
		m_strQuery = std::move("PRAGMA busy_timeout = 5000;"_L1);
		for (auto &&query : queries)
			m_strQuery += std::move(query);
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateRecord()
{
	bool success{false}, cmd_ok{false};
	const uint modified_row{m_dbModelInterface->modifiedIndices().cbegin().key()};
	const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
	const int modified_field{m_dbModelInterface->modifiedIndices().cbegin().value().first()};
	const QString &new_value{m_dbModelInterface->modelData().at(modified_row).at(modified_field)};

	m_strQuery = std::move("UPDATE "_L1 % *m_tableName % u" SET %1=%2 WHERE %3=%4;"_s.arg(
		m_fieldNames[modified_field][0], m_fieldNames[modified_field][1] == "TEXT"_L1 ?
											'\'' % new_value % '\'' : new_value, m_fieldNames[0][0], id));
	success = execSingleWriteQuery(m_strQuery);
	if (success)
	{
		m_strQuery.prepend("PRAGMA busy_timeout = 5000;"_L1);
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	m_dbModelInterface->modifiedIndices().remove(modified_row);
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateFieldsOfRecord()
{
	bool success{false}, cmd_ok{false};
	const uint modified_row{m_dbModelInterface->modifiedIndices().cbegin().key()};
	const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
	const QList<int> &fields{m_dbModelInterface->modifiedIndices().value(modified_row)};

	m_strQuery = std::move("UPDATE "_L1 % *m_tableName % " SET "_L1);
	for (uint i{0}; i < fields.count(); ++i)
	{
		m_strQuery += std::move(m_fieldNames[i][0] % '=' % (m_fieldNames[i][1] == "TEXT"_L1 ?
							'\'' % m_dbModelInterface->modelData().at(modified_row).at(i) % '\'' :
												m_dbModelInterface->modelData().at(modified_row).at(i)) % ',');
	}
	m_strQuery.chop(1);
	m_strQuery += std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
	success = execSingleWriteQuery(m_strQuery);
	if (success)
	{
		m_strQuery.prepend("PRAGMA busy_timeout = 5000;"_L1);
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	m_dbModelInterface->modifiedIndices().remove(modified_row);
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateRecords()
{
	bool success{false}, cmd_ok{false};
	const QString &query_cmd{"UPDATE "_L1 % *m_tableName % " SET "_L1};
	QString str_query;
	uint modified_row{0};
	QStringList queries;
	for (const auto &fields : m_dbModelInterface->modifiedIndices())
	{
		str_query = query_cmd;
		for (const auto field : std::as_const(fields))
		{
			str_query += std::move(m_fieldNames[field][0] % '=' % (m_fieldNames[field][1] == "TEXT"_L1 ?
						'\'' % m_dbModelInterface->modelData().at(modified_row).at(field) % '\'' :
											m_dbModelInterface->modelData().at(modified_row).at(field)));
		}
		const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
		str_query += std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
		queries.append(std::move(str_query));
		++modified_row;
	}
	m_dbModelInterface->clearModifiedIndices();

	if (execMultipleWritesQuery(queries))
	{
		success = true;
		m_strQuery = std::move("PRAGMA busy_timeout = 5000;"_L1);
		for (auto &&query : queries)
			m_strQuery += std::move(query);
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::removeRecords()
{
	bool success{false}, cmd_ok{false};
	for (const auto value : std::as_const(m_dbModelInterface->removalInfo()))
	{
		m_strQuery = std::move("DELETE FROM "_L1 % *m_tableName % " WHERE "_L1);
		for (uint i{0}; i < value->fields.count(); ++i)
		{
			if (i == 0)
				m_strQuery += std::move(m_fieldNames[value->fields.at(i)][0] % '=' % value->values.at(i));
			else
				m_strQuery += std::move(u" AND %1=%2"_s).arg(m_fieldNames[value->fields.at(i)][0], value->values.at(i));
		}
		m_strQuery += ';';
		success = execSingleWriteQuery(m_strQuery);
		if (success)
		{
			m_strQuery.prepend("PRAGMA busy_timeout = 5000;"_L1);
			cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
		}
		m_dbModelInterface->clearRemovalIndices();
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool, bool> TPDatabaseTable::clearTable()
{
	bool cmd_ok{false};
	m_strQuery = std::move("DELETE FROM "_L1 % *m_tableName % ';');
	const bool success{execSingleWriteQuery(m_strQuery)};
	if (success)
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::removeTemporaries()
{
	const bool success{execSingleWriteQuery("DELETE FROM "_L1 % *m_tableName % " WHERE %1<0;"_L1.arg(m_fieldNames[0][0]))};
	return std::pair<bool,bool>{success, false};
}

void TPDatabaseTable::parseCmdFile(const QString &filename)
{
	QFile *cmd_file{appUtils()->openFile(filename)};
	if (cmd_file)
	{
		QString line{1024, QChar{0}};
		QString affected_file, command;
		QTextStream stream{cmd_file};
		int execution_module_found{-1};
		static const QStringList execution_modules{QStringList{} << std::move("sqlite3"_L1) };
		while (stream.readLineInto(&line))
		{
			if (line.startsWith('#'))
				continue;
			if (line.contains('='))
			{
				QString value{std::move(line.section('=', 1, 1 , QString::SectionSkipEmpty))};
				if (execution_module_found == -1)
				{
					const int module{static_cast<int>(execution_modules.indexOf(value))};
					if (module >= 0)
						execution_module_found = module;
				}
				else
				{
					switch (execution_module_found)
					{
						case 0: //sqlite statement
							if (affected_file.isEmpty())
							{
								for (const auto &dbname : std::as_const(m_databaseFilenamesPool))
								{
									if (value == dbname)
									{
										affected_file = dbname;
										break;
									}
								}
								if (!affected_file.isEmpty())
									break;
							}
							if (!affected_file.isEmpty())
							{
								if (!command.isEmpty())
									break;
								command = std::move(value);
								//appThreadManager()->executeExternalQuery(affected_file, command);
							}
						break;
						default: return;
					}
				}
			}
		}
	}
}

#define prepareQuery(forward_only) \
		QSqlQuery query{m_sqlLiteDB}; \
		if (forward_only) \
			query.setForwardOnly(true); \
		m_workingQuery = std::move(query); \
		static_cast<void>(m_workingQuery.exec("PRAGMA page_size = 4096"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA cache_size = 16384"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA temp_store = MEMORY"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA journal_mode = OFF"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA locking_mode = EXCLUSIVE"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA synchronous = 0"_L1)); \

//optimize after modifying the database
#define optimizeTable() \
		static_cast<void>(m_workingQuery.exec("VACUUM;"_L1)); \
		static_cast<void>(m_workingQuery.exec("PRAGMA optimize;"_L1)); \

bool TPDatabaseTable::execReadOnlyQuery(const QString &str_query)
{
	bool ok{false};
	m_sqlLiteDB.setConnectOptions("QSQLITE_OPEN_READONLY"_L1);
	if (m_sqlLiteDB.open())
	{
		prepareQuery(true);
		ok = m_workingQuery.exec(str_query);
		#ifndef QT_NO_DEBUG
		if (!ok)
		{
			qDebug() << "****** ERROR ******";
			qDebug() << str_query;
			qDebug() << m_sqlLiteDB.lastError().text();
			qDebug();
		}
		#endif
	}
	return ok;
}

bool TPDatabaseTable::execSingleWriteQuery(const QString &str_query)
{
	bool ok{false};
	m_sqlLiteDB.setConnectOptions("QSQLITE_BUSY_TIMEOUT=0"_L1);
	if (m_sqlLiteDB.open())
	{
		prepareQuery(false);
		ok = m_workingQuery.exec(str_query);
		#ifndef QT_NO_DEBUG
		if (!ok)
		{
			qDebug() << "****** ERROR ******";
			qDebug() << str_query;
			qDebug() << m_sqlLiteDB.lastError().text();
			qDebug();
		}
		#endif
		if (ok)
			optimizeTable();
	}
	return ok;
}

bool TPDatabaseTable::execMultipleWritesQuery(const QStringList &queries)
{
	bool ok{false};
	m_sqlLiteDB.setConnectOptions("QSQLITE_BUSY_TIMEOUT=0"_L1);
	if (m_sqlLiteDB.open())
	{
		if (!m_sqlLiteDB.transaction())
			return false;
		ok = true;
		prepareQuery(false);
		for (const QString &str_query : std::as_const(queries))
		{
			if (!m_workingQuery.exec(str_query))
			{
				#ifndef QT_NO_DEBUG
				qDebug() << "****** ERROR ******";
				qDebug() << str_query;
				qDebug() << m_sqlLiteDB.lastError().text();
				qDebug();
				#endif
				ok = false;
				break;
			}
		}

		if (ok)
		{
			if ((ok = m_sqlLiteDB.commit()))
				optimizeTable();
		}
	}
	return ok;
}

void TPDatabaseTable::setDBModelInterface(DBModelInterface *dbmodel_interface)
{
	m_dbModelInterface = dbmodel_interface;
}

bool TPDatabaseTable::createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
												const bool overwrite) const
{
	bool cmd_ok{false};
	if (command_parts.size() > 0)
	{
		QString cmd_string{"#Device_ID "_L1 % appOsInterface()->deviceID() % '\n'};
		int n_part{1};
		QStringList var_list;
		QString vars_cmd;
		for (QString cmd_part : command_parts)
		{
			if (n_part == 1)
			{
				var_list.append(std::move("VAR_1="_L1 % cmd_part % '\n'));
				vars_cmd = std::move("$VAR_1"_L1);
			}
			else
			{
				if (cmd_part.contains('\''))
					cmd_part.replace('\'', R"('\'')");
				var_list.append(std::move("VAR_"_L1 % QString::number(n_part) % "='"_L1 % cmd_part % "'\n"_L1));
				if (cmd_part.count(' ') > 0)
					vars_cmd += std::move(" \"${VAR_"_L1 % QString::number(n_part) % "}\""_L1);
				else
					vars_cmd += std::move(" $VAR_"_L1 % QString::number(n_part));
			}
			++n_part;
		}
		for (QString &var : var_list)
			cmd_string += std::move(var);
		cmd_string += std::move(vars_cmd % "\n#Downloads 1"_L1);

		const QString &cmd_filename{dir % QString::number(QDateTime::currentSecsSinceEpoch()) % ".cmd"_L1};
		if (!QFile::exists(cmd_filename) || overwrite)
		{
			QFile *cmd_file{appUtils()->openFile(cmd_filename, false, true, false, true)};
			if (cmd_file)
			{
				cmd_ok = true;
				cmd_file->write(cmd_string.toUtf8().constData());
				cmd_file->flush();
				cmd_file->close();
			}
		}
	}
	return cmd_ok;
}
