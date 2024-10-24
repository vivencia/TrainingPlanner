#include "dbmesosplittable.h"
#include "dbmesosplitmodel.h"
#include "tpglobals.h"

#include <QSqlQuery>
#include <QFile>

#include <random>

DBMesoSplitTable::DBMesoSplitTable(const QString& dbFilePath, DBMesoSplitModel* model)
	: TPDatabaseTable{}, m_model(model)
{
	std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<double> dist(0, 1);

	m_tableName = u"mesocycles_splits"_s;
	m_tableID = MESOSPLIT_TABLE_ID;
	setObjectName(DBMesoSplitObjectName);
	m_UniqueID = QString::number(dist(gen)).remove(0, 2).toUInt();
	const QString& cnx_name(u"db_mesosplit_connection-"_s + QString::number(dist(gen)));
	mSqlLiteDB = QSqlDatabase::addDatabase(u"QSQLITE"_s, cnx_name);
	const QString& dbname(dbFilePath + DBMesoSplitFileName);
	mSqlLiteDB.setDatabaseName(dbname);
}

void DBMesoSplitTable::createTable()
{
	if (openDatabase())
	{
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"CREATE TABLE IF NOT EXISTS mesocycles_splits ("
										"id INTEGER PRIMARY KEY AUTOINCREMENT,"
										"meso_id INTEGER,"
										"splitA TEXT, "
										"splitB TEXT DEFAULT \"\", "
										"splitC TEXT DEFAULT \"\", "
										"splitD TEXT DEFAULT \"\", "
										"splitE TEXT DEFAULT \"\", "
										"splitF TEXT DEFAULT \"\", "
										"splitA_exercisesnames TEXT DEFAULT \"\", "
										"splitA_exercisesset_n TEXT DEFAULT \"\", "
										"splitA_exercisesset_notes TEXT DEFAULT \"\", "
										"splitA_exercisesset_types TEXT DEFAULT \"\", "
										"splitA_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitA_exercisesset_reps TEXT DEFAULT \"\", "
										"splitA_exercisesset_weight TEXT DEFAULT \"\", "
										"splitB_exercisesnames TEXT DEFAULT \"\", "
										"splitB_exercisesset_n TEXT DEFAULT \"\", "
										"splitB_exercisesset_notes TEXT DEFAULT \"\", "
										"splitB_exercisesset_types TEXT DEFAULT \"\", "
										"splitB_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitB_exercisesset_reps TEXT DEFAULT \"\", "
										"splitB_exercisesset_weight TEXT DEFAULT \"\", "
										"splitC_exercisesnames TEXT DEFAULT \"\", "
										"splitC_exercisesset_n TEXT DEFAULT \"\", "
										"splitC_exercisesset_notes TEXT DEFAULT \"\", "
										"splitC_exercisesset_types TEXT DEFAULT \"\", "
										"splitC_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitC_exercisesset_reps TEXT DEFAULT \"\", "
										"splitC_exercisesset_weight TEXT DEFAULT \"\", "
										"splitD_exercisesnames TEXT DEFAULT \"\", "
										"splitD_exercisesset_n TEXT DEFAULT \"\", "
										"splitD_exercisesset_notes TEXT DEFAULT \"\", "
										"splitD_exercisesset_types TEXT DEFAULT \"\", "
										"splitD_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitD_exercisesset_reps TEXT DEFAULT \"\", "
										"splitD_exercisesset_weight TEXT DEFAULT \"\", "
										"splitE_exercisesnames TEXT DEFAULT \"\", "
										"splitE_exercisesset_n TEXT DEFAULT \"\", "
										"splitE_exercisesset_notes TEXT DEFAULT \"\", "
										"splitE_exercisesset_types TEXT DEFAULT \"\", "
										"splitE_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitE_exercisesset_reps TEXT DEFAULT \"\", "
										"splitE_exercisesset_weight TEXT DEFAULT \"\", "
										"splitF_exercisesnames TEXT DEFAULT \"\", "
										"splitF_exercisesset_n TEXT DEFAULT \"\", "
										"splitF_exercisesset_notes TEXT DEFAULT \"\", "
										"splitF_exercisesset_types TEXT DEFAULT \"\", "
										"splitF_exercisesset_subsets TEXT DEFAULT \"\", "
										"splitF_exercisesset_reps TEXT DEFAULT \"\", "
										"splitF_exercisesset_weight TEXT DEFAULT \"\")"_s);

		const bool ok = query.exec(strQuery);
		setResult(ok, nullptr, strQuery, SOURCE_LOCATION);
	}
}

void DBMesoSplitTable::updateTable()
{

}

void DBMesoSplitTable::getAllMesoSplits()
{
	if (openDatabase(true))
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT id,meso_id,splitA,splitB,splitC,splitD,splitE,splitF FROM mesocycles_splits"_s);

		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				uint meso_idx(0);
				do
				{
					for (uint i(0); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
						m_model->setFast(meso_idx, i, query.value(i).toString());
					++meso_idx;
				} while (query.next ());
				ok = true;
			}
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
}


void DBMesoSplitTable::saveMesoSplit()
{
	if (openDatabase())
	{
		bool ok(false);
		QSqlQuery query{getQuery()};
		const uint row(m_execArgs.at(0).toUInt());
		bool bUpdate(false);

		if (query.exec(u"SELECT id FROM mesocycles_splits WHERE meso_id=%1"_s.arg(m_model->id(row))))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		QString strQuery;
		if (!bUpdate)
		{
			strQuery = u"INSERT INTO mesocycles_splits (meso_id, splitA, splitB, splitC, splitD, splitE, splitF)"
						" VALUES(%1, \'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\')"_s
							.arg(m_model->mesoId(row), m_model->splitA(row), m_model->splitB(row), m_model->splitC(row), m_model->splitD(row),
							m_model->splitE(row), m_model->splitF(row));
		}
		else
		{
			strQuery = u"UPDATE mesocycles_splits SET splitA=\'%1\', splitB=\'%2\', splitC=\'%3\', splitD=\'%4\', splitE=\'%5\', "
					   "splitF=\'%6\' WHERE meso_id=%7"_s
						.arg(m_model->splitA(row), m_model->splitB(row), m_model->splitC(row), m_model->splitD(row),
							m_model->splitE(row), m_model->splitF(row), m_model->mesoId(row));
		}
		ok = query.exec(strQuery);
		if (ok)
		{
			if (!bUpdate)
				m_model->setId(row, query.lastInsertId().toString());
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::getAllSplits()
{
	if (openDatabase(true))
	{
		const QString& mesoId(m_execArgs.at(0).toString());
		QMap<QChar,DBMesoSplitModel*>* allSplits(m_execArgs.at(1).value<QMap<QChar,DBMesoSplitModel*>*>());
		QMap<QChar,DBMesoSplitModel*>::const_iterator splitModel(allSplits->constBegin());
		QStringList split_info(COMPLETE_MESOSPLIT_TOTAL_COLS);
		split_info[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;
		QSqlQuery query{getQuery()};

		for(char c('A'); c <= char('F'); ++c)
		{
			const QString& strQuery(u"SELECT split%1_exercisesnames, split%1_exercisesset_n, split%1_exercisesset_notes, "
						"split%1_exercisesset_types, split%1_exercisesset_subsets, split%1_exercisesset_reps, "
						"split%1_exercisesset_weight, split%1 FROM mesocycles_splits WHERE meso_id=%2"_s.arg(c).arg(mesoId));
			bool ok(false);

			if (query.exec(strQuery))
			{
				if (query.first ())
				{

					(*splitModel)->setMuscularGroup(query.value(7).toString());
					const QStringList& exercises(query.value(MESOSPLIT_COL_EXERCISENAME).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setsnumber(query.value(MESOSPLIT_COL_SETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setsnotes(query.value(MESOSPLIT_COL_NOTES).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setstypes(query.value(MESOSPLIT_COL_SETTYPE).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setssubsets(query.value(MESOSPLIT_COL_SUBSETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setsreps(query.value(MESOSPLIT_COL_REPSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
					const QStringList& setsweight(query.value(MESOSPLIT_COL_WEIGHT).toString().split(record_separator, Qt::SkipEmptyParts));

					for(uint i(0); i < exercises.count(); ++i)
					{
						split_info[MESOSPLIT_COL_EXERCISENAME] = exercises.at(i);
						split_info[MESOSPLIT_COL_SETSNUMBER] = setsnumber.at(i);
						split_info[MESOSPLIT_COL_NOTES] = setsnotes.at(i);
						split_info[MESOSPLIT_COL_SETTYPE] = setstypes.at(i);
						split_info[MESOSPLIT_COL_SUBSETSNUMBER] = setssubsets.at(i);
						split_info[MESOSPLIT_COL_REPSNUMBER] = setsreps.at(i);
						split_info[MESOSPLIT_COL_WEIGHT] = setsweight.at(i);
						(*splitModel)->appendListMove(split_info);
					}
					ok = true;
				}
			}
			setResult(ok, (*splitModel), strQuery, SOURCE_LOCATION);
			++splitModel;
		}
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::getCompleteMesoSplit(const bool bEmitSignal)
{
	if (openDatabase(true))
	{
		bool ok(false);
		const QString& mesoId(m_execArgs.at(0).toString());
		const QChar& splitLetter(m_execArgs.at(1).toChar());

		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT split%1_exercisesnames, split%1_exercisesset_n, split%1_exercisesset_notes, "
						"split%1_exercisesset_types, split%1_exercisesset_subsets, split%1_exercisesset_reps, "
						"split%1_exercisesset_weight, split%1 FROM mesocycles_splits WHERE meso_id=%2"_s.arg(splitLetter).arg(mesoId));

		if (query.exec(strQuery))
		{
			if (query.first ())
			{
				m_model->setMuscularGroup(query.value(7).toString());

				const QStringList& exercises(query.value(MESOSPLIT_COL_EXERCISENAME).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setsnumber(query.value(MESOSPLIT_COL_SETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setsnotes(query.value(MESOSPLIT_COL_NOTES).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setstypes(query.value(MESOSPLIT_COL_SETTYPE).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setssubsets(query.value(MESOSPLIT_COL_SUBSETSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setsreps(query.value(MESOSPLIT_COL_REPSNUMBER).toString().split(record_separator, Qt::SkipEmptyParts));
				const QStringList& setsweight(query.value(MESOSPLIT_COL_WEIGHT).toString().split(record_separator, Qt::SkipEmptyParts));

				QStringList split_info(COMPLETE_MESOSPLIT_TOTAL_COLS);
				split_info[MESOSPLIT_COL_WORKINGSET] = STR_ZERO;
				for(uint i(0); i < exercises.count(); ++i)
				{
					split_info[MESOSPLIT_COL_EXERCISENAME] = exercises.at(i);
					split_info[MESOSPLIT_COL_SETSNUMBER] = setsnumber.at(i);
					split_info[MESOSPLIT_COL_NOTES] = setsnotes.at(i);
					split_info[MESOSPLIT_COL_SETTYPE] = setstypes.at(i);
					split_info[MESOSPLIT_COL_SUBSETSNUMBER] = setssubsets.at(i);
					split_info[MESOSPLIT_COL_REPSNUMBER] = setsreps.at(i);
					split_info[MESOSPLIT_COL_WEIGHT] = setsweight.at(i);
					m_model->appendListMove(split_info);
				}
				ok = true;
			}
		}
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	if (bEmitSignal)
		doneFunc(static_cast<TPDatabaseTable*>(this));
}

void DBMesoSplitTable::saveMesoSplitComplete()
{
	if (openDatabase())
	{
		bool ok(false);
		QString exercises;
		QString setsnumber;
		QString setsnotes;
		QString setstypes;
		QString setssubsets;
		QString setsreps;
		QString setsweight;
		for(uint i(0); i < m_model->count(); ++i)
		{
			exercises += m_model->_exerciseName(i) + record_separator;
			setsnumber += m_model->_setsNumber(i) + record_separator;
			setsnotes += m_model->_setsNotes(i) + record_separator;
			setstypes += m_model->_setsTypes(i) + record_separator;
			setssubsets += m_model->_setsSubSets(i) + record_separator;
			setsreps += m_model->_setsReps(i) + record_separator;
			setsweight += m_model->_setsWeights(i) + record_separator;
		}

		const QString& mesoId(m_execArgs.at(0).toString());
		QSqlQuery query{getQuery()};
		bool bUpdate(false);
		QString strQuery;

		if (query.exec(u"SELECT id FROM mesocycles_splits WHERE meso_id=%1"_s.arg(mesoId)))
		{
			if (query.first())
				bUpdate = query.value(0).toUInt() >= 0;
			query.finish();
		}

		if (bUpdate)
		{
			strQuery = u"UPDATE mesocycles_splits SET split%1_exercisesnames=\'%2\', "
								"split%1_exercisesset_n=\'%3\', split%1_exercisesset_notes=\'%4\', "
								"split%1_exercisesset_types=\'%5\', split%1_exercisesset_subsets=\'%6\', "
								"split%1_exercisesset_reps=\'%7\', split%1_exercisesset_weight=\'%8\', "
								"split%1=\'%9\' WHERE meso_id=%10"_s
								.arg(m_model->splitLetter(), exercises, setsnumber, setsnotes, setstypes, setssubsets,
										setsreps, setsweight, m_model->muscularGroup(), mesoId);
		}
		else
		{
			strQuery = u"INSERT INTO mesocycles_splits (meso_id, split%1_exercisesnames, split%1_exercisesset_n, split%1_exercisesset_notes, "
								"split%1_exercisesset_types, split%1_exercisesset_subsets, split%1_exercisesset_reps, "
								"split%1_exercisesset_weight, split%1)"
								" VALUES(\'%2\', \'%3\', \'%4\', \'%5\', \'%6\', \'%7\', \'%8\', \'%9\', \'%10\')"_s
								.arg(m_model->splitLetter(), mesoId, exercises, setsnumber, setsnotes, setstypes, setssubsets,
										setsreps, setsweight, m_model->muscularGroup());
		}
		ok = query.exec(strQuery);
		if (ok && !bUpdate)
			m_model->setId(0, query.lastInsertId().toString()); //Not used -yet-. But might be, someday. Anyway, it costs nothings
		setResult(ok, m_model, strQuery, SOURCE_LOCATION);
	}
	doneFunc(static_cast<TPDatabaseTable*>(this));
}

bool DBMesoSplitTable::mesoHasPlan(const QString& mesoId, const QString& splitLetter)
{
	bool ok(false);
	if (openDatabase(true))
	{
		QSqlQuery query{getQuery()};
		const QString& strQuery(u"SELECT split%1_exercisesnames FROM mesocycles_splits WHERE meso_id=%2"_s.arg(splitLetter, mesoId));
		ok = query.exec(strQuery);
		if (ok)
		{
			ok = query.first();
			if (ok)
				ok = query.value(0).toString().length() > 0;
		}
		setResult(ok, nullptr, strQuery, SOURCE_LOCATION);
	}
	return ok;
}

void DBMesoSplitTable::convertTDayExercisesToMesoPlan(const DBTrainingDayModel* const tDayModel)
{
	m_model->convertFromTDayModel(tDayModel);
	saveMesoSplitComplete();
}
