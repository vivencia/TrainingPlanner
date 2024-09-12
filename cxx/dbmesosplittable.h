#ifndef DBMESOSPLITTABLE_H
#define DBMESOSPLITTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoSplitModel;
class DBTrainingDayModel;

static const QString DBMesoSplitFileName(u"MesocyclesSplits.db.sqlite"_qs);

class DBMesoSplitTable : public TPDatabaseTable
{

public:
	explicit DBMesoSplitTable(const QString& dbFilePath, DBMesoSplitModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllMesoSplits();
	void saveMesoSplit();
	void getCompleteMesoSplit(const bool bEmitSignal = true);
	void saveMesoSplitComplete();
	bool mesoHasPlan(const QString& mesoId, const QString& splitLetter);
	void convertTDayExercisesToMesoPlan(const DBTrainingDayModel* const tDayModel);
};

#endif // DBMESOSPLITTABLE_H
