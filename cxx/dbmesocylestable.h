#ifndef DBMESOCYLESTABLE_H
#define DBMESOCYLESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBMesocyclesModel;

static const QString DBMesocyclesFileName ( QStringLiteral("Mesocycles.db.sqlite") );

class DBMesocyclesTable : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(const QString& dbFilePath, QSettings* appSettings, DBMesocyclesModel* model = nullptr);

	virtual void createTable();
	virtual void updateDatabase();
	void getAllMesocycles();
	void saveMesocycle();
};

#endif // DBMESOCYLESTABLE_H
