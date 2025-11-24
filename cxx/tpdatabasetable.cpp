#include "tpdatabasetable.h"

#include "dbmodelinterface.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>

using namespace Qt::Literals::StringLiterals;

TPDatabaseTable::TPDatabaseTable(const uint table_id, DBModelInterface *dbmodel_interface)
	: QObject{nullptr}, m_tableId{table_id}, m_dbModelInterface{dbmodel_interface},
		m_deleteAfterFinished{false}
{
	m_dbModelInterface->setTPDatabaseTable(this);
	m_threadedFunctions.insert(ThreadManager::CustomOperation, [this] () {
		if (m_customQueryFunc)
		{
			auto result{m_customQueryFunc()};
			emit actionFinished(ThreadManager::CustomOperation, result.first, result.second);
		}
		else
			emit actionFinished(ThreadManager::CustomOperation, QVariant{}, QVariant{});
	});
	m_threadedFunctions.insert(ThreadManager::CreateTable, [this] () {
		auto result{createTable()};
		emit actionFinished(ThreadManager::CreateTable, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::InsertRecord, [this] () {
		auto result{insertRecord()};
		emit actionFinished(ThreadManager::InsertRecord, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::alterRecords, [this] () {
		auto result{alterRecords()};
		emit actionFinished(ThreadManager::alterRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateOneField, [this] () {
		auto result{updateRecord()};
		emit actionFinished(ThreadManager::UpdateOneField, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateSeveralFields, [this] () {
		auto result{updateFieldsOfRecord()};
		emit actionFinished(ThreadManager::UpdateSeveralFields, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::UpdateRecords, [this] () {
		auto result{updateRecords()};
		emit actionFinished(ThreadManager::UpdateRecords, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::DeleteRecord, [this] () {
		auto result{removeRecord()};
		emit actionFinished(ThreadManager::DeleteRecord, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::RemoveTemporaries, [this] () {
		auto result{removeTemporaries()};
		emit actionFinished(ThreadManager::RemoveTemporaries, result.first, result.second);
	});
	m_threadedFunctions.insert(ThreadManager::ClearTable, [this] () {
		auto result{clearTable()};
		emit actionFinished(ThreadManager::ClearTable, result.first, result.second);
	});

	connect(this, &TPDatabaseTable::actionFinished, this, [this]
				(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2) {
		if (op != ThreadManager::CustomOperation && return_value2.toBool())
			appUserModel()->sendUnsentCmdFiles(subDir());
	});
}

void TPDatabaseTable::startAction(const int unique_id, ThreadManager::StandardOps operation)
{
	if (unique_id == uniqueId())
		m_threadedFunctions.value(operation)();
}

void TPDatabaseTable::setUpConnection()
{
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{*m_tableName + "_connection"_L1 + QString::number(m_uniqueID)};
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
	bool success{false}, cmd_ok{false};
	if (!QFile::exists(dbFileName()))
	{
		if (appUtils()->mkdir(dbFilePath()))
		{
			m_strQuery = {std::move("CREATE TABLE IF NOT EXISTS "_L1 + *m_tableName + " ("_L1)};
			for (uint i{0}; i < m_fieldCount; ++i)
				m_strQuery += std::move(m_fieldNames[i][0] + ' ' + m_fieldNames[i][1]) + ',';
			m_strQuery.chop(1);
			m_strQuery += std::move(");"_L1);
			success = execQuery(m_strQuery, false);
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

	m_strQuery = std::move("INSERT INTO "_L1 + *m_tableName + " ("_L1);
	for (int i{0}; i < m_fieldCount; ++i)
		m_strQuery += std::move(m_fieldNames[i][0] + ',');
	m_strQuery += std::move(") VALUES("_L1);
	for (int i{0}; i < m_fieldCount; ++i)
		m_strQuery += std::move((m_fieldNames[i][1] == "TEXT"_L1 ? '\'' + data.at(i) + '\'' : data.at(i)) + ',');
	m_strQuery += std::move(");"_L1);

	const bool query_id_back{m_fieldNames[modified_row][1].contains("AUTOINCREMENT"_L1)};
	if (execQuery(m_strQuery, false, !query_id_back))
	{
		if (query_id_back)
		{
			m_dbModelInterface->modelData()[modified_row][0] = std::move(m_workingQuery.lastInsertId().toString());
			m_sqlLiteDB.close();
		}
		success = true;
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	}
	m_dbModelInterface->modifiedIndices().remove(modified_row);
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::alterRecords()
{
	bool success{false}, cmd_ok{false};
	if (m_sqlLiteDB.transaction())
	{
		QString insert_cmd{std::move("INSERT INTO "_L1 + *m_tableName + " ("_L1)};
		for (int i{0}; i < m_fieldCount; ++i)
				insert_cmd += std::move(m_fieldNames[i][0] + ',');
		insert_cmd += std::move(") VALUES("_L1);
		const QString &update_cmd{"UPDATE "_L1 + *m_tableName + "SET "_L1};
		bool has_insert{false};
		const bool  auto_increment{m_fieldNames[0][1].contains("AUTOINCREMENT"_L1)};

		QHash<uint, QList<uint>>::const_iterator itr{m_dbModelInterface->modifiedIndices().constBegin()};
		const QHash<uint, QList<uint>>::const_iterator itr_end{m_dbModelInterface->modifiedIndices().constEnd()};
		while (itr != itr_end)
		{
			auto modified_row{itr.key()};
			if (m_dbModelInterface->modelData().at(modified_row).at(0).toInt() < 0)
			{
				int field{0};
				has_insert = true;
				m_strQuery += insert_cmd;
				for (const auto &data : std::as_const(m_dbModelInterface->modelData().at(modified_row)))
				{
					if (!(auto_increment && field != 0))
						m_strQuery += std::move(m_fieldNames[field][1] == "TEXT"_L1 ? QString{'\'' + data + '\''} : QString{data + ','});
					++field;
				}
				m_strQuery.append(';');
			}
			else
			{
				m_strQuery += update_cmd;
				for (const auto field : std::as_const(itr.value()))
				{
					m_strQuery += std::move(m_fieldNames[field][0] + '=' + (m_fieldNames[field][1] == "TEXT"_L1 ?
							'\'' + m_dbModelInterface->modelData().at(modified_row).at(field) + '\'' :
												m_dbModelInterface->modelData().at(modified_row).at(field)));
				}
				const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
				m_strQuery = std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
			}
			++itr;
		}
		m_dbModelInterface->clearModifiedIndices();

		const bool query_id_back{has_insert && auto_increment};
		if (execQuery(m_strQuery, false, !query_id_back))
		{
			success = m_sqlLiteDB.commit();
			if (success)
			{
				if (query_id_back)
				{
					int last_insert_id{m_workingQuery.lastInsertId().toInt()};
					m_sqlLiteDB.close();
					for (auto &data : m_dbModelInterface->modelData() | std::views::reverse)
					{
						if (data.at(0).toInt() < 0)
							data[0] = std::move(QString::number(last_insert_id--));
					}
				}
				cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
			}
		}
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateRecord()
{
	bool success{false}, cmd_ok{false};
	const uint modified_row{m_dbModelInterface->modifiedIndices().cbegin().key()};
	const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
	const uint modified_field{m_dbModelInterface->modifiedIndices().cbegin().value().first()};
	const QString &new_value{m_dbModelInterface->modelData().at(modified_row).at(modified_field)};

	m_strQuery = std::move("UPDATE "_L1 + *m_tableName + u"SET %1=%2 WHERE %3=%4;"_s.arg(
		m_fieldNames[modified_field][0], m_fieldNames[modified_field][1] == "TEXT"_L1 ?
											'\'' + new_value + '\'' : new_value, m_fieldNames[0][0], id));
	success = execQuery(m_strQuery, false);
	if (success)
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	m_dbModelInterface->modifiedIndices().remove(modified_row);
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateFieldsOfRecord()
{
	bool success{false}, cmd_ok{false};
	const uint modified_row{m_dbModelInterface->modifiedIndices().cbegin().key()};
	const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
	const QList<uint> &fields{m_dbModelInterface->modifiedIndices().value(modified_row)};

	m_strQuery = std::move("UPDATE "_L1 + *m_tableName + "SET "_L1);
	for (uint i{0}; i < fields.count(); ++i)
	{
		m_strQuery += std::move(m_fieldNames[i][0] + '=' + (m_fieldNames[i][1] == "TEXT"_L1 ?
							'\'' + m_dbModelInterface->modelData().at(modified_row).at(i) + '\'' :
												m_dbModelInterface->modelData().at(modified_row).at(i)));
	}
	m_strQuery = std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
	success = execQuery(m_strQuery, false);
	if (success)
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	m_dbModelInterface->modifiedIndices().remove(modified_row);
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::updateRecords()
{
	bool success{false}, cmd_ok{false};
	if (m_sqlLiteDB.transaction())
	{
		const QString &query_cmd{"UPDATE "_L1 + *m_tableName + "SET "_L1};
		uint modified_row{0};
		for (const auto &fields : m_dbModelInterface->modifiedIndices())
		{
			for (const auto field : std::as_const(fields))
			{
				m_strQuery += std::move(m_fieldNames[field][0] + '=' + (m_fieldNames[field][1] == "TEXT"_L1 ?
							'\'' + m_dbModelInterface->modelData().at(modified_row).at(field) + '\'' :
												m_dbModelInterface->modelData().at(modified_row).at(field)));
			}
			const QString &id{m_dbModelInterface->modelData().at(modified_row).at(0)};
			m_strQuery = std::move(" WHERE %1=%2;"_L1.arg(m_fieldNames[0][0], id));
			++modified_row;
		}
		m_dbModelInterface->clearModifiedIndices();

		if (execQuery(m_strQuery, false, false))
		{
			if (m_sqlLiteDB.commit())
			{
				success = true;
				cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
			}
		}
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::removeRecord()
{
	bool success{false}, cmd_ok{false};
	for (const auto value : std::as_const(m_dbModelInterface->removalInfo()))
	{
		m_strQuery = std::move("DELETE FROM "_L1 + *m_tableName + " WHERE "_L1);
		for (uint i{0}; i < value->fields.count(); ++i)
		{
			if (i == 0)
				m_strQuery += std::move(m_fieldNames[value->fields.at(i)][0] + '=' + value->values.at(i));
			else
				m_strQuery += std::move(u" AND %1=%2"_s).arg(m_fieldNames[value->fields.at(i)][0], value->values.at(i));
		}
		m_strQuery += ';';
		success = execQuery(m_strQuery, false);
		if (success)
			cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
		m_dbModelInterface->clearRemovalIndices();
	}
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool, bool> TPDatabaseTable::clearTable()
{
	bool cmd_ok{false};
	m_strQuery = std::move("DELETE FROM "_L1 + *m_tableName + ';');
	const bool success{execQuery(m_strQuery, false)};
	if (success)
		cmd_ok = createServerCmdFile(dbFilePath(), {sqliteApp, dbFileName(false), m_strQuery});
	return std::pair<bool,bool>{success, cmd_ok};
}

std::pair<bool,bool> TPDatabaseTable::removeTemporaries()
{
	const bool success{execQuery("DELETE FROM "_L1 + *m_tableName + " WHERE %1<0;"_L1.arg(m_fieldNames[0][0]), false, true)};
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

bool TPDatabaseTable::openDatabase(const bool read_only)
{
	m_sqlLiteDB.setConnectOptions(read_only ? "QSQLITE_OPEN_READONLY"_L1 : "QSQLITE_BUSY_TIMEOUT=0"_L1);
	const bool ok{m_sqlLiteDB.open()};
	#ifndef QT_NO_DEBUG
	if (!ok)
	{
		qDebug() << "****** ERROR ******   " << m_sqlLiteDB.databaseName();
		qDebug() << m_sqlLiteDB.lastError().text();
		qDebug();
	}
	#endif
	return ok;
}

QSqlQuery TPDatabaseTable::getQuery() const
{
	QSqlQuery query{m_sqlLiteDB};
	query.setForwardOnly(true);
	static_cast<void>(query.exec("PRAGMA page_size = 4096"_L1));
	static_cast<void>(query.exec("PRAGMA cache_size = 16384"_L1));
	static_cast<void>(query.exec("PRAGMA temp_store = MEMORY"_L1));
	static_cast<void>(query.exec("PRAGMA journal_mode = OFF"_L1));
	static_cast<void>(query.exec("PRAGMA locking_mode = EXCLUSIVE"_L1));
	static_cast<void>(query.exec("PRAGMA synchronous = 0"_L1));
	return query;
}

bool TPDatabaseTable::execQuery(const QString &str_query, const bool read_only, const bool close_db)
{
	if (m_sqlLiteDB.isOpen())
	{
		if (m_sqlLiteDB.connectOptions().contains("READONLY"_L1))
		{
			if (!read_only)
				m_sqlLiteDB.close();
		}
		else
		{
			if (read_only)
				m_sqlLiteDB.close();
		}
	}

	if (!m_sqlLiteDB.isOpen())
	{
		if (!openDatabase(read_only))
			return false;
	}

	m_workingQuery.finish();
	m_workingQuery = std::move(getQuery());
	const bool ok{m_workingQuery.exec(str_query)};
	#ifndef QT_NO_DEBUG
	if (!ok)
	{
		qDebug() << "****** ERROR ******";
		qDebug() << str_query;
		qDebug() << m_sqlLiteDB.lastError().text();
		qDebug();
	}
	#endif

	if (!read_only) //optimize after modifying the database
	{
		QSqlQuery query{m_sqlLiteDB};
		static_cast<void>(query.exec("VACUUM"_L1));
		static_cast<void>(query.exec("PRAGMA optimize"_L1));
	}
	if (close_db)
		m_sqlLiteDB.close();
	return ok;
}

bool TPDatabaseTable::createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
												const bool overwrite) const
{
	bool cmd_ok{false};
	if (command_parts.size() > 0)
	{
		QString cmd_string{"#Device_ID "_L1 + appOsInterface()->deviceID() + '\n'};
		int n_part{1};
		QStringList var_list;
		QString vars_cmd;
		for (const QString &cmd_part : command_parts)
		{
			if (n_part == 1)
			{
				var_list.append(std::move("VAR_1="_L1 + cmd_part + '\n'));
				vars_cmd = "$VAR_1"_L1;
			}
			else
			{
				var_list.append(std::move("VAR_"_L1 + QString::number(n_part) + "=\""_L1 + cmd_part + "\"\n"_L1));
				if (cmd_part.count(' ') > 0)
					vars_cmd += " \"${VAR_"_L1 + QString::number(n_part) + "}\""_L1;
				else
					vars_cmd += " $VAR_"_L1 + QString::number(n_part);
			}
			++n_part;
		}
		for (const QString &var : std::as_const(var_list))
			cmd_string += var;
		cmd_string += vars_cmd + "\n#Downloads 1"_L1;

		const QString &cmd_filename{dir + QString::number(QDateTime::currentSecsSinceEpoch()) + ".cmd"_L1};
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
