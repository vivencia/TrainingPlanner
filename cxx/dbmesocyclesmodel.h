#ifndef DBMESOCYCLESMODEL_H
#define DBMESOCYCLESMODEL_H

#include "tplistmodel.h"
#include "dbusermodel.h"

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
class DBMesoSplitModel;
class DBMesoCalendarModel;

class DBMesocyclesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:

	enum RoleNames {
		mesoNameRole = Qt::UserRole+MESOCYCLES_COL_NAME,
		mesoStartDateRole = Qt::UserRole+MESOCYCLES_COL_STARTDATE,
		mesoEndDateRole = Qt::UserRole+MESOCYCLES_COL_ENDDATE,
		mesoSplitRole = Qt::UserRole+MESOCYCLES_COL_SPLIT,
		mesoCoachRole = Qt::UserRole+MESOCYCLES_COL_COACH,
		mesoClientRole = Qt::UserRole+MESOCYCLES_COL_CLIENT
	};

	explicit DBMesocyclesModel(QObject* parent, DBUserModel* userModel);
	~DBMesocyclesModel();

	void newMesocycle(const QStringList& infolist);
	void delMesocycle(const uint meso_idx);
	inline DBMesoSplitModel* mesoSplitModel() { return m_splitModel; }
	inline DBMesoCalendarModel* mesoCalendarModel(const uint meso_idx) const { return m_calendarModelList.value(meso_idx); }

	virtual bool importFromText(QFile* inFile, QString& inData) override;
	virtual inline bool isFieldFormatSpecial (const uint field) const override
	{
		return field == MESOCYCLES_COL_STARTDATE || field == MESOCYCLES_COL_ENDDATE;
	}

	virtual QString formatFieldToExport(const uint field, const QString& fieldValue) const override;
	QString formatFieldToImport(const QString& fieldValue) const;

	Q_INVOKABLE bool setMesoStartDate(const uint row, const QDate& new_date);
	Q_INVOKABLE bool setMesoEndDate(const uint row, const QDate& new_date);
	Q_INVOKABLE bool setMesoSplit(const uint row, const QString& new_split);

	//mesoSplitModel is not accessed directly, only through here
	Q_INVOKABLE QString getSplitLetter(const uint meso_idx, const uint day_of_week) const;
	Q_INVOKABLE QString getMuscularGroup(const uint meso_idx, const QString& splitLetter) const;
	Q_INVOKABLE void setMuscularGroup(const uint meso_idx, const QString& splitLetter, const QString& newSplitValue);

	Q_INVOKABLE inline bool isOwnMeso(const int row) const
	{
		if (row >= 0 && row < m_modeldata.count())
			return getFast(row, MESOCYCLES_COL_CLIENT) == m_userModel->userName(0);
		return false;
	}

	Q_INVOKABLE inline bool isRealMeso(const int row) const
	{
		if (row >= 0 && row < m_modeldata.count())
			return getFast(row, MESOCYCLES_COL_REALMESO) == QStringLiteral("1");
		return false;
	}

	Q_INVOKABLE inline void setIsRealMeso(const uint row, const bool bRealMeso)
	{
		set(row, MESOCYCLES_COL_REALMESO, bRealMeso ? u"1"_qs : u"0"_qs);
		emit realMesoChanged(row);
	}

	Q_INVOKABLE inline QDate getEndDate(const uint row) const
	{
		return isRealMeso(row) ? QDate::fromJulianDay(m_modeldata.at(row).at(MESOCYCLES_COL_ENDDATE).toLongLong()) :
							QDate::currentDate().addDays(730);
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
	void updateColumnLabels();

signals:
	void mesoCalendarFieldsChanged(const uint meso_idx);
	void realMesoChanged(const uint meso_idx);
	void muscularGroupChanged(const int splitIndex, const QString& splitLetter);

private:
	DBUserModel* m_userModel;
	DBMesoSplitModel* m_splitModel;
	QMap<uint,DBMesoCalendarModel*> m_calendarModelList;
};

#endif // DBMESOCYCLESMODEL_H
