#ifndef DBMESOCYCLESTABLE_H
#define DBMESOCYCLESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesocyclesModel;

static const QLatin1StringView &DBMesocyclesFileName{"Mesocycles.db.sqlite"_L1};

class DBMesocyclesTable final : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(const QString &dbFilePath, DBMesocyclesModel *model = nullptr);

	void createTable() override final;
	void updateTable() override final;
	void getAllMesocycles();
	void saveMesocycle();

private:
	DBMesocyclesModel *m_model;
};

#endif // DBMESOCYCLESTABLE_H
