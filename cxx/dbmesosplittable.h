#ifndef DBMESOSPLITTABLE_H
#define DBMESOSPLITTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesoSplitModel;
class DBTrainingDayModel;

static const QString DBMesoSplitFileName(u"MesocyclesSplits.db.sqlite"_qs);

class DBMesoSplitTable final : public TPDatabaseTable
{

public:
	explicit DBMesoSplitTable(const QString& dbFilePath, DBMesoSplitModel* model = nullptr);

	void createTable() override;
	void updateTable() override;
	void getAllMesoSplits();
	void saveMesoSplit();
	void getCompleteMesoSplit(const bool bEmitSignal = true);
	void saveMesoSplitComplete();
	bool mesoHasPlan(const QString& mesoId, const QString& splitLetter);
	void convertTDayExercisesToMesoPlan(const DBTrainingDayModel* const tDayModel);
	inline DBMesoSplitModel* model() const { return m_model; }
	inline void setAnotherModel(DBMesoSplitModel* new_model) { m_model = new_model; }

private:
	DBMesoSplitModel* m_model;
};

#endif // DBMESOSPLITTABLE_H
