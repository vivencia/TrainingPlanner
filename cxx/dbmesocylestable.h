#ifndef DBMESOCYLESTABLE_H
#define DBMESOCYLESTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBMesocyclesModel;

static const QString DBMesocyclesFileName ( QStringLiteral("Mesocycles.db.sqlite") );
static const QString DBMesocyclesObjectName ( QStringLiteral("Mesocycles") );
static const uint MESOCYCLES_TABLE_ID = 0x0002;

class DBMesocyclesTable : public TPDatabaseTable
{

public:
	explicit DBMesocyclesTable(const QString& dbFilePath, QSettings* appSettings, DBMesocyclesModel* model = nullptr);

	virtual void createTable();
	void getAllMesocycles();
	void newMesocycle();
	void updateMesocycle();
	void removeMesocycle();
	void deleteMesocyclesTable();

	//Call before starting a thread
	void setData(const QString& id, const QString& mesoName = QString(), const QString& mesoStartDate = QString(),
						const QString& mesoEndDate = QString(), const QString& mesoNote = QString(),
						const QString& mesoWeeks = QString(), const QString& mesoSplit = QString(),
						const QString& mesoDrugs = QString());
};

#endif // DBMESOCYLESTABLE_H
