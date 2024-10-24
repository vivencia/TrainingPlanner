#ifndef DBMESOCYLESTABLE_H
#define DBMESOCYLESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>

class DBMesocyclesModel;

static const QString DBMesocyclesFileName(u"Mesocycles.db.sqlite"_s);

class DBMesocyclesTable final : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(const QString& dbFilePath, DBMesocyclesModel* model = nullptr);

	void createTable() override;
	void updateTable() override;
	void getAllMesocycles();
	void saveMesocycle();

private:
	DBMesocyclesModel* m_model;
};

#endif // DBMESOCYLESTABLE_H
