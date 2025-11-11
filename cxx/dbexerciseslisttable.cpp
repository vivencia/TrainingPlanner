#include "dbexerciseslisttable.h"

#include "dbexerciseslistmodel.h"
#include "tputils.h"

#include <QSqlError>
#include <QThread>

DBExercisesListTable::DBExercisesListTable()
	: TPDatabaseTable{EXERCISES_TABLE_ID}, m_exercisesTableLastId{1000}
{
	setTableName(tableName());
	m_uniqueID = appUtils()->generateUniqueId();
	const QString &cnx_name{"db_exercises_connection"_L1 + QString::number(m_uniqueID)};
	m_sqlLiteDB = std::move(QSqlDatabase::addDatabase("QSQLITE"_L1, cnx_name));
	m_sqlLiteDB.setDatabaseName(dbFilePath(m_tableId));
	#ifndef QT_NO_DEBUG
	setObjectName("ExercisesListTable");
	#endif
}

QLatin1StringView DBExercisesListTable::createTableQuery()
{
	return "CREATE TABLE IF NOT EXISTS %1 ("
										"id INTEGER PRIMARY KEY,"
										"primary_name TEXT,"
										"secondary_name TEXT,"
										"muscular_group TEXT,"
										"media_path TEXT,"
										"from_list INTEGER"
									");"_L1;
}

void DBExercisesListTable::getAllExercises()
{
	if (execQuery("SELECT * FROM %1 ORDER BY ROWID;"_L1.arg(tableName()), true, false))
	{
		if (m_workingQuery.first())
		{
			do
			{
				QStringList data{EXERCISES_TOTAL_COLS};
				for (uint i{EXERCISES_LIST_COL_ID}; i < EXERCISES_LIST_COL_ACTUALINDEX; ++i)
					data[i] = std::move(m_workingQuery.value(static_cast<int>(i)).toString());
				data[EXERCISES_LIST_COL_ACTUALINDEX] = std::move(QString::number(appExercisesList()->count()));
				data[EXERCISES_LIST_COL_SELECTED] = std::move("0"_L1);
				appExercisesList()->appendList(std::move(data));
			} while (m_workingQuery.next ());
			const uint highest_id{static_cast<uint>(appExercisesList()->_id(appExercisesList()->count() - 1))};
			if (highest_id >= m_exercisesTableLastId)
				m_exercisesTableLastId = highest_id + 1;
			appExercisesList()->setLastID(m_exercisesTableLastId);
		}
		else //for whatever reason the database table is empty. Populate it with the app provided exercises list
		{
			m_sqlLiteDB.close();
			updateExercisesList();
		}
	}
	emit threadFinished();
}

void DBExercisesListTable::updateExercisesList()
{
	getExercisesList();
	if (m_ExercisesList.isEmpty())
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "****** ERROR ******";
		qDebug() << "DBExercisesListTable::updateExercisesList -> m_ExercisesList is empty"_L1;
		qDebug();
		#endif
		this->thread()->exit();
		return;
	}

	//remove previous list entries from DB
	if (execQuery("DELETE FROM %1 WHERE from_list=1;"_L1.arg(tableName()), false, false))
	{

		QString queryValues;
		const QString &queryStart{u"INSERT INTO %1 "
						"(id,primary_name,secondary_name,muscular_group,media_path,from_list) VALUES "_s.arg(tableName())};

		uint idx{0};
		for (const auto &data : std::as_const(m_ExercisesList))
		{
			QStringList fields{std::move(data.split(';'))};
			appExercisesList()->newExerciseFromList(std::move(fields[0]), std::move(fields[1]), std::move(fields.at(2).trimmed()));
			queryValues += std::move(appExercisesList()->makeTransactionStatementForDataBase(idx));
			++idx;
		}

		queryValues[queryValues.length() - 1] = ';';
		if (m_sqlLiteDB.transaction())
		{
			m_strQuery = std::move(queryStart + queryValues);
			if (!execQuery(m_strQuery, false, false))
				static_cast<void>(m_sqlLiteDB.rollback());
			else
			{
				if (m_sqlLiteDB.commit())
					emit updatedFromExercisesList();
				#ifndef QT_NO_DEBUG
				else
				{
					qDebug() << "****** ERROR ******";
					qDebug() << "DBExercisesListTable::updateExercisesList -> transaction not commited"_L1;
					qDebug() << m_sqlLiteDB.lastError();
					qDebug();
				}
				#endif
			}
		}	
	}
	emit threadFinished(true);
}

void DBExercisesListTable::saveExercises()
{
	uint highest_id{0};
	const QString &queryInsert{u"INSERT INTO %1 (id,primary_name,secondary_name,muscular_group,media_path,from_list)"
									" VALUES(%2, \'%3\', \'%4\', \'%5\', \'%6\', 0),"_s};
	const QString &queryUpdate{u"UPDATE %1 SET primary_name=\'%2\', secondary_name=\'%3\', muscular_group=\'%4\', "
									"media_path=\'%5\', from_list=0 WHERE id=%6 "_s};

	for (uint i{0}; i < appExercisesList()->modifiedIndicesCount(); ++i)
	{
		const uint &idx{appExercisesList()->modifiedIndex(i)};
		const QString &exerciseId{appExercisesList()->id(idx)};
		const bool update{!(exerciseId.isEmpty() || exerciseId.toUInt() > m_exercisesTableLastId)};

		if (appExercisesList()->_id(idx) > highest_id)
			highest_id = appExercisesList()->_id(idx);
		if (update)
		{
			m_strQuery += std::move(queryUpdate.arg(tableName(), appExercisesList()->mainName(idx), appExercisesList()->subName(idx),
						appExercisesList()->muscularGroup(idx), appExercisesList()->mediaPath(idx), exerciseId));
		}
		else
		{
			m_strQuery += std::move(queryInsert.arg(tableName(), exerciseId, appExercisesList()->mainName(idx),
						appExercisesList()->subName(idx), appExercisesList()->muscularGroup(idx), appExercisesList()->mediaPath(idx)));
		}
	}
	bool ok{false};
	if (m_sqlLiteDB.transaction())
	{
		m_strQuery[m_strQuery.length()-1] = ';';
		if (execQuery(m_strQuery, false, false))
		{
			if (m_sqlLiteDB.commit())
			{
				m_exercisesTableLastId = highest_id;
				appExercisesList()->clearModifiedIndices();
				ok = true;
			}
			#ifndef QT_NO_DEBUG
			else
			{
				qDebug() << "****** ERROR ******";
				qDebug() << "DBExercisesListTable::saveExercises -> transaction not commited"_L1;
				qDebug() << m_sqlLiteDB.lastError();
				qDebug();
			}
			#endif
		}
	}
	emit threadFinished(ok);
}

void DBExercisesListTable::getExercisesList()
{
	QFile *exercises_list_file{appUtils()->openFile(":/extras/exerciseslist.lst"_L1)};
	if (exercises_list_file)
	{
		m_ExercisesList.reserve(304);
		QString line{1024, QChar{0}};
		QTextStream stream{exercises_list_file};
		stream.readLineInto(&line); //skip first line
		while (stream.readLineInto(&line))
			m_ExercisesList.append(std::move(line));
		exercises_list_file->close();
		delete exercises_list_file;
	}
}
