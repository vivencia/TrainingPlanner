#ifndef DBMESOSPLITTABLE_H
#define DBMESOSPLITTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBMesoSplitModel;
class DBTrainingDayModel;

static const QString DBMesoSplitFileName ( QStringLiteral("MesocyclesSplits.db.sqlite") );

class DBMesoSplitTable : public TPDatabaseTable
{

public:
	explicit DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase() {}
	void getAllMesoSplits();
	void saveMesoSplit();
	void getCompleteMesoSplit(const bool bEmitSignal = true);
	void saveMesoSplitComplete();
	bool mesoHasPlan(const QString& mesoId, const QString& splitLetter);
	void convertTDayExercisesToMesoPlan(DBTrainingDayModel* tDayModel);
};

#endif // DBMESOSPLITTABLE_H
