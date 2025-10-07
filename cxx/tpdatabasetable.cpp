#include "tpdatabasetable.h"

#include "dbexerciseslisttable.h"
#include "dbmesocalendartable.h"
#include "dbmesocyclestable.h"
#include "dbusermodel.h"
#include "dbusertable.h"
#include "dbworkoutsorsplitstable.h"
#include "osinterface.h"
#include "tputils.h"

#include <QFile>
#include <QSqlError>

using namespace Qt::Literals::StringLiterals;

static const QString &sqlite{"sqlite3"_L1};

TPDatabaseTable *TPDatabaseTable::createDBTable(const uint table_id, const bool auto_delete)
{
	TPDatabaseTable *db_table{nullptr};
	switch (table_id)
	{
		case EXERCISES_TABLE_ID: db_table = new DBExercisesListTable{nullptr}; break;
		case MESOCYCLES_TABLE_ID: db_table = new DBMesocyclesTable{nullptr}; break;
		case MESOSPLIT_TABLE_ID: db_table = new DBWorkoutsOrSplitsTable{MESOSPLIT_TABLE_ID}; break;
		case MESOCALENDAR_TABLE_ID: db_table = new DBMesoCalendarTable{nullptr}; break;
		case WORKOUT_TABLE_ID: db_table = new DBWorkoutsOrSplitsTable{WORKOUT_TABLE_ID}; break;
		case USERS_TABLE_ID: db_table = new DBUserTable{nullptr}; break;
	}
	if (db_table && auto_delete)
		db_table->deleteLater();
	return db_table;
}

QString TPDatabaseTable::createTableQuery(const uint table_id)
{
	switch (table_id)
	{
		case EXERCISES_TABLE_ID: return QString{DBExercisesListTable::createTableQuery()}.arg(DBExercisesListTable::tableName());
		case MESOCYCLES_TABLE_ID: return QString{DBMesocyclesTable::createTableQuery()}.arg(DBMesocyclesTable::tableName());
		case MESOSPLIT_TABLE_ID: return QString{DBWorkoutsOrSplitsTable::createTableQuery()}.arg(DBWorkoutsOrSplitsTable::tableName(MESOSPLIT_TABLE_ID));
		case MESOCALENDAR_TABLE_ID: return QString{DBMesoCalendarTable::createTableQuery()}.arg(DBMesoCalendarTable::tableName());
		case WORKOUT_TABLE_ID: return QString{DBWorkoutsOrSplitsTable::createTableQuery()}.arg(DBWorkoutsOrSplitsTable::tableName(WORKOUT_TABLE_ID));
		case USERS_TABLE_ID: return QString{DBUserTable::createTableQuery()}.arg(DBUserTable::tableName());
	}
	return QString{};
}

void TPDatabaseTable::removeEntry(const bool bUseMesoId)
{
	static_cast<void>(execQuery("DELETE FROM "_L1 + m_tableName + (bUseMesoId ? " WHERE meso_id="_L1 : " WHERE id="_L1) +
					m_execArgs.at(0).toString(), false));
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeTemporaries(const bool bUseMesoId)
{
	static_cast<void>(execQuery("DELETE FROM "_L1 + m_tableName + (bUseMesoId ? " WHERE meso_id<0"_L1 :
						" WHERE id<0"_L1), false, true, false));
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::clearTable()
{
	static_cast<void>(execQuery("DELETE FROM "_L1 + m_tableName, false));
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void TPDatabaseTable::removeDBFile()
{
	const bool ok{QFile::remove(mSqlLiteDB.databaseName())};
	if (ok)
		createTable();
	if (doneFunc)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

bool TPDatabaseTable::openDatabase(const bool read_only)
{
	mSqlLiteDB.setConnectOptions(read_only ? "QSQLITE_OPEN_READONLY"_L1 : "QSQLITE_BUSY_TIMEOUT=0"_L1);
	const bool ok{mSqlLiteDB.open()};
	#ifndef QT_NO_DEBUG
	if (!ok)
	{
		qDebug() << "****** ERROR ******   " << mSqlLiteDB.databaseName();
		qDebug() << mSqlLiteDB.lastError().text();
		qDebug();
	}
	#endif
	return ok;
}

QSqlQuery TPDatabaseTable::getQuery() const
{
	QSqlQuery query{mSqlLiteDB};
	//if (!mSqlLiteDB.connectOptions().isEmpty())
	query.setForwardOnly(true);
	static_cast<void>(query.exec("PRAGMA page_size = 4096"_L1));
	static_cast<void>(query.exec("PRAGMA cache_size = 16384"_L1));
	static_cast<void>(query.exec("PRAGMA temp_store = MEMORY"_L1));
	static_cast<void>(query.exec("PRAGMA journal_mode = OFF"_L1));
	static_cast<void>(query.exec("PRAGMA locking_mode = EXCLUSIVE"_L1));
	static_cast<void>(query.exec("PRAGMA synchronous = 0"_L1));
	return query;
}

bool TPDatabaseTable::execQuery(const QString &str_query, const bool read_only, const bool close_db, const bool send_to_server)
{
	if (!mSqlLiteDB.isOpen())
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
		qDebug() << mSqlLiteDB.lastError().text();
		qDebug();
	}
	#endif

	if (!read_only) //optimize after modifying the database
	{
		QSqlQuery query{mSqlLiteDB};
		static_cast<void>(query.exec("VACUUM"_L1));
		static_cast<void>(query.exec("PRAGMA optimize"_L1));
	}
	if (close_db)
		mSqlLiteDB.close();
	if (send_to_server && !read_only && ok && appUserModel()->onlineAccount())
	{
		const QString &cmd_filename{createServerCmdFile(dbFilePath(true), {sqlite, databaseFileNames[m_tableId], str_query})};
		if (!cmd_filename.isEmpty())
			appUserModel()->sendCmdFileToServer(cmd_filename);
	}
	return ok;
}

QString TPDatabaseTable::createServerCmdFile(const QString &dir, const std::initializer_list<QString> &command_parts,
												const bool overwrite) const
{
	QString cmd_string{"#Device_ID "_L1 + appOsInterface()->deviceID() + '\n'};
	int n_part{0};
	QStringList var_list;
	for (const QString &cmd_part : command_parts)
	{
		QString var{std::move("VAR_"_L1 + QString::number(n_part) + "=\""_L1 + cmd_part + "\"\n"_L1)};
		if (cmd_part.count(' ') > 0)
			var_list.append(std::move("\"${"_L1 + var + "}\""_L1));
		else
			var_list.append(std::move('$' + var + ' '));
		++n_part;
	}
	for (const QString &var : std::as_const(var_list))
		cmd_string += var;

	cmd_string += "\n#Downloads 1";
	std::hash<std::string> str_hash;
	const QString &cmd_filename{dir + QString::number(str_hash(cmd_string.toStdString())) + ".cmd"_L1};
	if (!QFile::exists(cmd_filename) || overwrite)
	{
		QFile *cmd_file{appUtils()->openFile(cmd_filename, false, true, true)};
		if (cmd_file)
		{
			cmd_file->write(cmd_string.toUtf8().constData());
			cmd_file->flush();
			cmd_file->close();
		}
		else
			return QString{};
	}
	return cmd_filename;
}

//TODO
bool TPDatabaseTable::executeCmdFile(const QString &cmd_file, const QString &success_message, const bool remove_file) const
{
	QFile *cmdfile{appUtils()->openFile(cmd_file)};
	if (cmdfile)
	{
		QString command{1024, QChar{0}};
		QTextStream stream{cmdfile};

		while (stream.readLineInto(&command))
		{
			if (command.isEmpty() || command.startsWith('#'))
				continue;
			break;
		}

	}
	return false;
}
