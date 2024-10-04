#ifndef DBMESOCYCLESMODEL_H
#define DBMESOCYCLESMODEL_H

#include "tplistmodel.h"
#include "dbusermodel.h"
#include "tputils.h"

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

#define MESOCYCLES_COL_MUSCULARGROUP 100

class DBMesoSplitModel;
class DBMesoCalendarModel;

//mesoSplitModel is not accessed directly, only through here
class DBMesocyclesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int currentMesoIdx READ currentMesoIdx WRITE setCurrentMesoIdx NOTIFY currentMesoIdxChanged)
Q_PROPERTY(bool isNewMeso READ isNewMeso NOTIFY isNewMesoChanged)

public:

	enum RoleNames {
		mesoNameRole = Qt::UserRole+MESOCYCLES_COL_NAME,
		mesoStartDateRole = Qt::UserRole+MESOCYCLES_COL_STARTDATE,
		mesoEndDateRole = Qt::UserRole+MESOCYCLES_COL_ENDDATE,
		mesoSplitRole = Qt::UserRole+MESOCYCLES_COL_SPLIT,
		mesoCoachRole = Qt::UserRole+MESOCYCLES_COL_COACH,
		mesoClientRole = Qt::UserRole+MESOCYCLES_COL_CLIENT
	};

	explicit DBMesocyclesModel(QObject* parent = nullptr);
	~DBMesocyclesModel();
	void setUserModel(DBUserModel* usermodel);

	const uint newMesocycle(const QStringList& infolist);
	void delMesocycle(const uint meso_idx);
	void finishedLoadingFromDatabase();
	inline DBMesoSplitModel* mesoSplitModel() { return m_splitModel; }
	inline DBMesoCalendarModel* mesoCalendarModel(const uint meso_idx) const { return m_calendarModelList.value(meso_idx); }

	inline bool isNewMeso(const int meso_idx = -1) const { return m_isNewMeso.at(meso_idx >= 0 ? meso_idx : currentMesoIdx()) != 0; }
	void setModified(const uint meso_idx, const uint field);

	inline const QString& id(const uint meso_idx) const { return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ID); }
	inline const int _id(const uint meso_idx) const { return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ID).toInt(); }
	void setId(const uint meso_idx, const QString& new_id);

	Q_INVOKABLE inline QString name(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoName", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_NAME);
	}
	Q_INVOKABLE void setName(const uint meso_idx, const QString& new_name);

	inline const QString& strStartDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE);
	}
	Q_INVOKABLE inline QDate startDate(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::getStartDate", "out of range meso_idx");
		return QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE).toLongLong());
	}
	Q_INVOKABLE inline QString startDateFancy(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::startDateFancy", "out of range meso_idx");
		return appUtils()->formatDate(QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE).toLongLong()));
	}
	Q_INVOKABLE bool setStartDate(const uint meso_idx, const QDate& new_date);

	inline const QString& strEndDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE);
	}
	Q_INVOKABLE inline QDate endDate(const uint meso_idx) const
	{
		return isRealMeso(meso_idx) ? QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE).toLongLong()) :
							QDate::currentDate().addDays(730);
	}
	Q_INVOKABLE inline QString endDateFancy(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::startDateFancy", "out of range meso_idx");
		return appUtils()->formatDate(QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE).toLongLong()));
	}
	Q_INVOKABLE bool setEndDate(const uint meso_idx, const QDate& new_date);

	Q_INVOKABLE inline QString notes(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoNotes", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_NOTE);
	}
	Q_INVOKABLE void setNotes(const uint meso_idx, const QString& new_notes);

	Q_INVOKABLE inline QString nWeeks(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoWeeks", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_WEEKS);
	}
	Q_INVOKABLE void setWeeks(const uint meso_idx, const QString& new_weeks);

	Q_INVOKABLE inline QString split(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoSplit", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_SPLIT);
	}
	Q_INVOKABLE bool setSplit(const uint meso_idx, const QString& new_split);

	Q_INVOKABLE inline QString coach(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoCoach", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_COACH);
	}
	Q_INVOKABLE void setCoach(const uint meso_idx, const QString& new_coach);

	Q_INVOKABLE inline QString client(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoClient", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT);
	}
	Q_INVOKABLE void setClient(const uint meso_idx, const QString& new_client);

	Q_INVOKABLE inline bool isOwnMeso(const int meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isOwnMeso", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == m_userModel->userName(0);
	}
	Q_INVOKABLE void setOwnMeso(const uint meso_idx, const bool bOwnMeso);

	Q_INVOKABLE inline QString file(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoFile", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_FILE);
	}

	Q_INVOKABLE inline QString fileFancy(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoFile", "out of range meso_idx");
		return appUtils()->getFileName(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_FILE));
	}
	Q_INVOKABLE bool setFile(const uint meso_idx, const QString& new_file);

	Q_INVOKABLE inline QString type(const uint meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::mesoType", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_TYPE);
	}
	Q_INVOKABLE void setType(const uint meso_idx, const QString& new_type);

	inline const QString& realMeso(const uint meso_idx) const { return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_REALMESO); }
	Q_INVOKABLE inline bool isRealMeso(const int meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isRealMeso", "out of range meso_idx");
		return realMeso(meso_idx) == STR_ONE;
	}
	Q_INVOKABLE void setIsRealMeso(const uint meso_idx, const bool bRealMeso);

	Q_INVOKABLE QString muscularGroup(const uint meso_idx, const QString& splitLetter) const;
	Q_INVOKABLE void setMuscularGroup(const uint meso_idx, const QString& splitLetter, const QString& newSplitValue, const uint initiator_id);

	Q_INVOKABLE QString splitLetter(const uint meso_idx, const uint day_of_week) const;
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE inline int currentMesoIdx() const { return m_currentMesoIdx; }
	Q_INVOKABLE void setCurrentMesoIdx(const uint meso_idx);
	Q_INVOKABLE inline int mostRecentOwnMesoIdx() const { return m_mostRecentOwnMesoIdx; }

	inline bool newMesoCalendarChanged(const uint meso_idx) const { return m_newMesoCalendarChanged.at(meso_idx); }
	inline void setNewMesoCalendarChanged(const uint meso_idx, const bool changed) { m_newMesoCalendarChanged[meso_idx] = changed; }
	bool isDateWithinMeso(const int meso_idx, const QDate& date) const;
	inline uint totalSplits(const uint meso_idx) const { return m_totalSplits.value(meso_idx); }
	void findTotalSplits(const uint meso_idx);
	void findNextOwnMeso();
	int getPreviousMesoId(const int current_mesoid) const;
	QDate getPreviousMesoEndDate(const int current_mesoid) const;
	QDate getNextMesoStartDate(const int mesoid) const;
	QDate getLastMesoEndDate() const;

	bool isDifferent(const TPListModel* const model);
	void updateColumnLabels();

	virtual int exportToFile(const QString& filename, const bool = true, const bool = true) const override;
	virtual int importFromFile(const QString& filename) override;
	bool updateFromModel(const uint meso_idx, const TPListModel* const model);

	virtual inline bool isFieldFormatSpecial (const uint field) const override
	{
		switch (field)
		{
			case MESOCYCLES_COL_STARTDATE:
			case MESOCYCLES_COL_ENDDATE:
			case MESOCYCLES_COL_COACH:
			case MESOCYCLES_COL_CLIENT:
			case MESOCYCLES_COL_REALMESO:
				return true;
			default: return false;
		}
	}

	QString formatFieldToExport(const uint field, const QString& fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString& fieldValue, const QString& fieldName) const;

signals:
	void isNewMesoChanged(const uint meso_idx);
	void isOwnMesoChanged(const uint meso_idx);
	void mesoChanged(const uint meso_idx, const uint field);
	void mesoCalendarFieldsChanged(const uint meso_idx);
	void muscularGroupChanged(const uint meso_idx, const uint initiator_id, const uint splitIndex, const QChar& splitLetter);
	void mostRecentOwnMesoChanged(const int meso_idx);
	void currentMesoIdxChanged();

private:
	DBUserModel* m_userModel;
	DBMesoSplitModel* m_splitModel;
	QList<DBMesoCalendarModel*> m_calendarModelList;
	QList<uint> m_totalSplits;
	QList<uchar> m_isNewMeso;
	QList<bool> m_newMesoCalendarChanged;
	int m_currentMesoIdx, m_mostRecentOwnMesoIdx;

	static DBMesocyclesModel* app_meso_model;
	friend DBMesocyclesModel* appMesoModel();
};

inline DBMesocyclesModel* appMesoModel() { return DBMesocyclesModel::app_meso_model; }
#endif // DBMESOCYCLESMODEL_H
