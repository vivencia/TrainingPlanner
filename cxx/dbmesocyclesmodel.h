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
	virtual void updateFromModel(TPListModel* model) override;
	//virtual const QString exportExtraInfo() const override;
	virtual bool importExtraInfo(const QString& ) override;
	virtual bool importFromFancyText(QFile* inFile) override;

	virtual inline bool isFieldFormatSpecial (const uint field) const override { return field == MESOCYCLES_COL_STARTDATE || field == MESOCYCLES_COL_ENDDATE; }
	virtual QString formatField(const QString& fieldValue) const override;

	uint getTotalSplits(const uint row) const;
	/*void setSplitInfo(const QString& splitA, const QString& splitB, const QString& splitC,
									const QString& splitD, const QString& splitE, const QString& splitF);*/

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 9; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	Q_INVOKABLE QString getMesoInfo(const int mesoid, const uint field) const;
	Q_INVOKABLE int getPreviousMesoId(const int current_mesoid) const;
	Q_INVOKABLE QDate getPreviousMesoEndDate(const int current_mesoid) const;
	Q_INVOKABLE QDate getNextMesoStartDate(const int mesoid) const;
	Q_INVOKABLE QDate getLastMesoEndDate() const;
	Q_INVOKABLE int mesoThatHasDate(const QDateTime& datetime) const;
};

#endif // DBMESOCYCLESMODEL_H
