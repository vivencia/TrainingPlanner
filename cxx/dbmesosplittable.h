#ifndef DBMESOSPLITTABLE_H
#define DBMESOSPLITTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoSplitModel;
class DBTrainingDayModel;

static const QString& DBMesoSplitFileName("MesocyclesSplits.db.sqlite"_L1);

struct statsInfo {
	uint mesoIdx;
	QChar splitLetter;
	QStringList exercises;
};

class DBMesoSplitTable final : public TPDatabaseTable
{

public:
	explicit DBMesoSplitTable(const QString& dbFilePath, DBMesoSplitModel* model = nullptr);

	void createTable() override final;
	void updateTable() override final;
	void getAllMesoSplits();
	void saveMesoSplit();
	void getAllSplits();
	void getCompleteMesoSplit(const bool bEmitSignal = true); //only for empty models
	void saveMesoSplitComplete();
	bool mesoHasPlan(const QString& mesoId, const QString& splitLetter);
	void convertTDayExercisesToMesoPlan(const DBTrainingDayModel* const tDayModel);
	inline DBMesoSplitModel* model() const { return m_model; }
	inline void setAnotherModel(DBMesoSplitModel* new_model) { m_model = new_model; }

	//Functions for TPStatistics
	void getExercisesForSplitWithinMeso();
	inline const statsInfo& retrievedStats() const { return m_statsInfo; }


private:
	DBMesoSplitModel* m_model;
	statsInfo m_statsInfo;
};

#endif // DBMESOSPLITTABLE_H
