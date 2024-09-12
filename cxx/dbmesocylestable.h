#ifndef DBMESOCYLESTABLE_H
#define DBMESOCYLESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesocyclesModel;

static const QString DBMesocyclesFileName(u"Mesocycles.db.sqlite"_qs);

class DBMesocyclesTable : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(const QString& dbFilePath, DBMesocyclesModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase();
	void getAllMesocycles();
	void saveMesocycle();
};

#endif // DBMESOCYLESTABLE_H
