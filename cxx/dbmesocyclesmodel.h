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

	inline const QString& id(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ID);
	}
	inline const int _id(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ID).toInt();
	}
	void setId(const uint meso_idx, const QString& new_id);

	inline const QString& name(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_NAME);
	}
	inline void setName(const uint meso_idx, const QString& new_name)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_NAME] = new_name;
		setModified(meso_idx, MESOCYCLES_COL_NAME);
	}

	inline const QString& strStartDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE);
	}
	inline QDate startDate(const uint meso_idx) const
	{
		return QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE).toLongLong());
	}
	void setStartDate(const uint meso_idx, const QDate& new_date);

	inline const QString& strEndDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE);
	}
	inline QDate endDate(const uint meso_idx) const
	{
		return isRealMeso(meso_idx) ? QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE).toLongLong()) :
							QDate::currentDate().addDays(730);
	}
	void setEndDate(const uint meso_idx, const QDate& new_date);

	inline const QString& notes(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_NOTE);
	}
	inline void setNotes(const uint meso_idx, const QString& new_notes)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_NOTE] = new_notes;
		setModified(meso_idx, MESOCYCLES_COL_NOTE);
	}

	inline const QString& nWeeks(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_WEEKS);
	}
	inline void setWeeks(const uint meso_idx, const QString& new_weeks)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_WEEKS] = new_weeks;
		setModified(meso_idx, MESOCYCLES_COL_WEEKS);
	}

	inline const QString& split(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_SPLIT);
	}
	void setSplit(const uint meso_idx, const QString& new_split);

	inline const QString& coach(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_COACH);
	}
	inline void setCoach(const uint meso_idx, const QString& new_coach)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_COACH] = new_coach;
		setModified(meso_idx, MESOCYCLES_COL_COACH);
	}

	inline const QString& client(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT);
	}
	inline void setClient(const uint meso_idx, const QString& new_client)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_CLIENT] = new_client;
		setModified(meso_idx, MESOCYCLES_COL_CLIENT);
	}

	inline bool isOwnMeso(const int meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isOwnMeso", "out of range meso_idx");
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT) == m_userModel->userName(0);
	}
	void setOwnMeso(const uint meso_idx, const bool bOwnMeso);

	inline const QString& file(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_FILE);
	}
	inline void setFile(const uint meso_idx, const QString& new_file)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_FILE] = new_file;
		setModified(meso_idx, MESOCYCLES_COL_FILE);
	}

	inline const QString& type(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_TYPE);
	}
	inline void setType(const uint meso_idx, const QString& new_type)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_TYPE] = new_type;
		setModified(meso_idx, MESOCYCLES_COL_TYPE);
	}

	inline const QString& realMeso(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_REALMESO);
	}
	inline bool isRealMeso(const int meso_idx) const
	{
		Q_ASSERT_X(meso_idx >= 0 && meso_idx < m_modeldata.count(), "DBMesocyclesModel::isRealMeso", "out of range meso_idx");
		return realMeso(meso_idx) == STR_ONE;
	}
	inline void setIsRealMeso(const uint meso_idx, const bool bRealMeso)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_REALMESO] = bRealMeso ? STR_ONE : STR_ZERO;
		setModified(meso_idx, MESOCYCLES_COL_REALMESO);
	}

	QString muscularGroup(const uint meso_idx, const QString& splitLetter) const;
	void setMuscularGroup(const uint meso_idx, const QString& splitLetter, const QString& newSplitValue, const uint initiator_id);

	QString splitLetter(const uint meso_idx, const uint day_of_week) const;
	QVariant data(const QModelIndex &index, int role) const override;
	inline int currentMesoIdx() const { return m_currentMesoIdx; }
	void setCurrentMesoIdx(const uint meso_idx);
	inline int mostRecentOwnMesoIdx() const { return m_mostRecentOwnMesoIdx; }

	inline bool newMesoCalendarChanged(const uint meso_idx) const { return m_newMesoCalendarChanged.at(meso_idx); }
	inline void setNewMesoCalendarChanged(const uint meso_idx, const bool changed) { m_newMesoCalendarChanged[meso_idx] = changed; }
	bool isDateWithinMeso(const int meso_idx, const QDate& date) const;
	void findNextOwnMeso();
	int getPreviousMesoId(const int current_mesoid) const;
	QDate getPreviousMesoEndDate(const int current_mesoid) const;
	QDate getNextMesoStartDate(const int mesoid) const;
	QDate getLastMesoEndDate() const;

	bool isDifferent(const TPListModel* const model);
	void updateColumnLabels();

	int exportToFile(const QString& filename, const bool = true, const bool = true) const override;
	int importFromFile(const QString& filename) override;
	bool updateFromModel(const uint meso_idx, TPListModel* model);

	inline bool isFieldFormatSpecial (const uint field) const override
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
	void mesoIdxChanged(const uint old_meso_idx, const uint new_meso_idx);
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
	QList<uchar> m_isNewMeso;
	QList<bool> m_newMesoCalendarChanged;
	int m_currentMesoIdx, m_mostRecentOwnMesoIdx;

	static DBMesocyclesModel* app_meso_model;
	friend DBMesocyclesModel* appMesoModel();
};

inline DBMesocyclesModel* appMesoModel() { return DBMesocyclesModel::app_meso_model; }
#endif // DBMESOCYCLESMODEL_H
