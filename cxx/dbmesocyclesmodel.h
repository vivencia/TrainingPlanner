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

#define MESOCYCLES_COL_MUSCULARGROUP 100

class DBMesoSplitModel;
class DBMesoCalendarModel;
class QMLMesoInterface;

//mesoSplitModel is not accessed directly, only through here
class DBMesocyclesModel : public TPListModel
{

Q_OBJECT

Q_PROPERTY(bool canHaveTodaysWorkout READ canHaveTodaysWorkout NOTIFY canHaveTodaysWorkoutChanged FINAL)
Q_PROPERTY(uint currentMesoIdx READ currentMesoIdx WRITE setCurrentMesoIdx NOTIFY currentMesoIdxChanged FINAL)

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
	void fillColumnNames();
	QMLMesoInterface* mesoManager(const uint meso_idx);

	Q_INVOKABLE void getMesocyclePage(const uint meso_idx);
	Q_INVOKABLE uint createNewMesocycle(const bool bCreatePage);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void getExercisesPlannerPage(const uint meso_idx);
	Q_INVOKABLE void getMesoCalendarPage(const uint meso_idx);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo);
	Q_INVOKABLE void todaysWorkout();

	const uint newMesocycle(QStringList&& infolist);
	void finishedLoadingFromDatabase();
	inline DBMesoSplitModel* mesoSplitModel() { return m_splitModel; }
	inline DBMesoCalendarModel* mesoCalendarModel(const uint meso_idx) const { return m_calendarModelList.value(meso_idx); }

	inline bool isNewMeso(const uint meso_idx) const { return m_isNewMeso.at(meso_idx) != 0; }
	inline bool canHaveTodaysWorkout() const { return m_bCanHaveTodaysWorkout; }
	void changeCanHaveTodaysWorkout();

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
		emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoNameRole);
	}

	inline const QString& strStartDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE);
	}
	Q_INVOKABLE inline QDate startDate(const int meso_idx) const
	{
		return meso_idx >= 0 ? QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_STARTDATE).toLongLong()) : QDate();
	}
	void setStartDate(const uint meso_idx, const QDate& new_date);

	inline const QString& strEndDate(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE);
	}
	Q_INVOKABLE inline QDate endDate(const int meso_idx) const
	{
		return meso_idx >= 0 ? isRealMeso(meso_idx) ?
			QDate::fromJulianDay(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_ENDDATE).toLongLong()) : QDate::currentDate().addDays(730) : QDate();
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
		emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoCoachRole);
	}

	inline const QString& client(const uint meso_idx) const
	{
		return m_modeldata.at(meso_idx).at(MESOCYCLES_COL_CLIENT);
	}
	inline void setClient(const uint meso_idx, const QString& new_client)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_CLIENT] = new_client;
		setModified(meso_idx, MESOCYCLES_COL_CLIENT);
		emit dataChanged(index(meso_idx, 0), index(meso_idx, 0), QList<int>() << mesoClientRole);
	}

	bool isOwnMeso(const int meso_idx) const;
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
		return realMeso(meso_idx) == '1';
	}
	inline void setIsRealMeso(const uint meso_idx, const bool bRealMeso)
	{
		m_modeldata[meso_idx][MESOCYCLES_COL_REALMESO] = bRealMeso ? '1' : '0';
		setModified(meso_idx, MESOCYCLES_COL_REALMESO);
	}

	Q_INVOKABLE QString muscularGroup(const uint meso_idx, const QChar& splitLetter) const;
	void setMuscularGroup(const uint meso_idx, const QChar& splitLetter, const QString& newSplitValue, const uint initiator_id);

	QString splitLetter(const uint meso_idx, const uint day_of_week) const;
	QVariant data(const QModelIndex &index, int role) const override;
	inline int currentMesoIdx() const { return m_currentMesoIdx; }
	void setCurrentMesoIdx(const uint meso_idx);
	inline int mostRecentOwnMesoIdx() const { return m_mostRecentOwnMesoIdx; }
	Q_INVOKABLE inline QStringList usedSplits(const uint meso_idx) const { return m_usedSplits.at(meso_idx); }
	void makeUsedSplits(const uint meso_idx);

	inline bool newMesoCalendarChanged(const uint meso_idx) const { return m_newMesoCalendarChanged.at(meso_idx); }
	inline void setNewMesoCalendarChanged(const uint meso_idx, const bool changed) { m_newMesoCalendarChanged[meso_idx] = changed; }
	bool isDateWithinMeso(const int meso_idx, const QDate& date) const;
	void findNextOwnMeso();
	int getPreviousMesoId(const int current_mesoid) const;
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
	void mesoChanged(const uint meso_idx, const uint field);
	void mesoCalendarFieldsChanged(const uint meso_idx);
	void muscularGroupChanged(const uint meso_idx, const uint initiator_id, const uint splitIndex, const QChar& splitLetter);
	void mostRecentOwnMesoChanged(const int meso_idx);
	void currentMesoIdxChanged();
	void canHaveTodaysWorkoutChanged();
	void usedSplitsChanged(const uint meso_idx);

private:
	QList<QMLMesoInterface*> m_mesoManagerList;
	DBMesoSplitModel* m_splitModel;
	QList<DBMesoCalendarModel*> m_calendarModelList;
	QList<uchar> m_isNewMeso;
	QList<bool> m_newMesoCalendarChanged;
	QList<QStringList> m_usedSplits;
	int m_currentMesoIdx, m_mostRecentOwnMesoIdx;
	bool m_bCanHaveTodaysWorkout;

	static DBMesocyclesModel* app_meso_model;
	friend DBMesocyclesModel* appMesoModel();
};

inline DBMesocyclesModel* appMesoModel() { return DBMesocyclesModel::app_meso_model; }
#endif // DBMESOCYCLESMODEL_H
