#ifndef DBMESOSPLITTABLE_H
#define DBMESOSPLITTABLE_H

#include "tpdatabasetable.h"

#include <QObject>
#include <QSettings>

class DBMesoSplitModel;

static const QString DBMesoSplitFileName ( QStringLiteral("MesocyclesSplits.db.sqlite") );
static const QString DBMesoSplitObjectName ( QStringLiteral("MesocyclesSplits") );
static const uint MESOSPLIT_TABLE_ID = 0x0003;

class DBMesoSplitTable : public TPDatabaseTable
{

public:
	explicit DBMesoSplitTable(const QString& dbFilePath, QSettings* appSettings, DBMesoSplitModel* model = nullptr);

	void createTable();
	void getMesoSplit();
	void newMesoSplit();
	void updateMesoSplit();
	void removeMesoSplit();

	//Call before starting a thread
	void setData(const QString& mesoId, const QString& splitA = QString(), const QString& splitB = QString(), const QString& splitC = QString(),
					const QString& splitD = QString(), const QString& splitE = QString(), const QString& splitF = QString());
};

#endif // DBMESOSPLITTABLE_H
