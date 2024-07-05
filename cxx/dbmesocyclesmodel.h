#ifndef DBMESOCYCLESMODEL_H
#define DBMESOCYCLESMODEL_H

#include "tplistmodel.h"

#define MESOCYCLES_COL_ID 0
#define MESOCYCLES_COL_NAME 1
#define MESOCYCLES_COL_STARTDATE 2
#define MESOCYCLES_COL_ENDDATE 3
#define MESOCYCLES_COL_NOTE 4
#define MESOCYCLES_COL_WEEKS 5
#define MESOCYCLES_COL_SPLIT 6
#define MESOCYCLES_COL_DRUGS 7
#define MESOCYCLES_COL_REALMESO 8

class DBMesocyclesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	// Define the role names to be used
	enum RoleNames {
		mesoIdRole = Qt::UserRole,
		mesoNameRole = Qt::UserRole+MESOCYCLES_COL_NAME,
		mesoStartDateRole = Qt::UserRole+MESOCYCLES_COL_STARTDATE,
		mesoEndDateRole = Qt::UserRole+MESOCYCLES_COL_ENDDATE,
		mesoNoteRole = Qt::UserRole+MESOCYCLES_COL_NOTE,
		mesoWeeksRole = Qt::UserRole+MESOCYCLES_COL_WEEKS,
		mesoSplitRole = Qt::UserRole+MESOCYCLES_COL_SPLIT,
		mesoDrugsRole = Qt::UserRole+MESOCYCLES_COL_DRUGS,
		realMesoRole = Qt::UserRole+MESOCYCLES_COL_REALMESO
	};

	explicit DBMesocyclesModel(QObject *parent = 0);
	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual bool importFromFancyText(QFile* inFile, QString& inData) override;

	inline bool isFieldFormatSpecial (const uint field) const { return field == MESOCYCLES_COL_STARTDATE || field == MESOCYCLES_COL_ENDDATE; }
	QString formatFieldToExport(const QString& fieldValue) const;
	QString formatFieldToImport(const QString& fieldValue) const;

	uint getTotalSplits(const uint row) const;

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 9; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	int getMesoIdx(const int mesoId) const;
	Q_INVOKABLE QString getMesoInfo(const int mesoid, const uint field) const;
	int getPreviousMesoId(const int current_mesoid) const;
	QDate getPreviousMesoEndDate(const int current_mesoid) const;
	QDate getNextMesoStartDate(const int mesoid) const;
	QDate getLastMesoEndDate() const;
	Q_INVOKABLE int mesoThatHasDate(const QDateTime& datetime) const;
	Q_INVOKABLE bool isDateWithinCurrentMeso(const QDate& date) const;
	bool isDifferent(const DBMesocyclesModel* model);
};

#endif // DBMESOCYCLESMODEL_H
