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
#define MESOCYCLES_COL_COACH 7
#define MESOCYCLES_COL_CLIENT 8
#define MESOCYCLES_COL_FILE 9
#define MESOCYCLES_COL_TYPE 10
#define MESOCYCLES_COL_REALMESO 11

#define MESOCYCLES_TOTAL_COLS MESOCYCLES_COL_REALMESO + 1

class DBUserModel;

class DBMesocyclesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:

	enum RoleNames {
		mesoNameRole = Qt::UserRole+MESOCYCLES_COL_NAME,
		mesoStartDateRole = Qt::UserRole+MESOCYCLES_COL_STARTDATE,
		mesoEndDateRole = Qt::UserRole+MESOCYCLES_COL_ENDDATE,
		mesoNoteRole = Qt::UserRole+MESOCYCLES_COL_NOTE,
		mesoWeeksRole = Qt::UserRole+MESOCYCLES_COL_WEEKS,
		mesoSplitRole = Qt::UserRole+MESOCYCLES_COL_SPLIT,
		realMesoRole = Qt::UserRole+MESOCYCLES_COL_REALMESO
	};

	explicit DBMesocyclesModel(QObject* parent, DBUserModel* userModel);
	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual bool importFromFancyText(QFile* inFile, QString& inData) override;

	inline bool isFieldFormatSpecial (const uint field) const { return field == MESOCYCLES_COL_STARTDATE || field == MESOCYCLES_COL_ENDDATE; }
	QString formatFieldToExport(const QString& fieldValue) const;
	QString formatFieldToImport(const QString& fieldValue) const;

	Q_INVOKABLE inline bool isRealMeso(const uint row) const {
		return getFast(row, MESOCYCLES_COL_REALMESO) == QStringLiteral("1");
	}

	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;

	uint getTotalSplits(const uint row) const;
	int getMesoIdx(const int mesoId) const;
	Q_INVOKABLE QString getMesoInfo(const int mesoid, const uint field) const;
	int getPreviousMesoId(const int current_mesoid) const;
	QDate getPreviousMesoEndDate(const int current_mesoid) const;
	QDate getNextMesoStartDate(const int mesoid) const;
	QDate getLastMesoEndDate() const;
	Q_INVOKABLE bool isDateWithinCurrentMeso(const QDate& date) const;
	bool isDifferent(const DBMesocyclesModel* model);

private:
	DBUserModel* m_userModel;
};

#endif // DBMESOCYCLESMODEL_H
