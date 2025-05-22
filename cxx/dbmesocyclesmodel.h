#pragma once

//Must include the header files of properties that are pointers
#include "homepagemesomodel.h"

#include <QObject>

#define MESOCYCLES_COL_ID 0
#define MESOCYCLES_COL_NAME 1
#define MESOCYCLES_COL_STARTDATE 2
#define MESOCYCLES_COL_ENDDATE 3
#define MESOCYCLES_COL_NOTE 4
#define MESOCYCLES_COL_WEEKS 5
#define MESOCYCLES_COL_SPLIT 6
#define MESOCYCLES_COL_SPLITA 7
#define MESOCYCLES_COL_SPLITB 8
#define MESOCYCLES_COL_SPLITC 9
#define MESOCYCLES_COL_SPLITD 10
#define MESOCYCLES_COL_SPLITE 11
#define MESOCYCLES_COL_SPLITF 12
#define MESOCYCLES_COL_COACH 13
#define MESOCYCLES_COL_CLIENT 14
#define MESOCYCLES_COL_FILE 15
#define MESOCYCLES_COL_TYPE 16
#define MESOCYCLES_COL_REALMESO 17
#define MESOCYCLES_TOTAL_COLS MESOCYCLES_COL_REALMESO + 1

#define MESOCYCLES_COL_MUSCULARGROUP 20

enum MesoRoleNames {
	mesoNameRole = Qt::UserRole+MESOCYCLES_COL_NAME,
	mesoStartDateRole = Qt::UserRole+MESOCYCLES_COL_STARTDATE,
	mesoEndDateRole = Qt::UserRole+MESOCYCLES_COL_ENDDATE,
	mesoSplitRole = Qt::UserRole+MESOCYCLES_COL_SPLIT,
	mesoCoachRole = Qt::UserRole+MESOCYCLES_COL_COACH,
	mesoClientRole = Qt::UserRole+MESOCYCLES_COL_CLIENT
};

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_CLASS(QMLMesoInterface)

QT_FORWARD_DECLARE_CLASS(QFile)

static const QLatin1StringView mesosDir{"mesocycles/"};

class DBMesocyclesModel : public QObject
{

Q_OBJECT

Q_PROPERTY(bool canHaveTodaysWorkout READ canHaveTodaysWorkout NOTIFY canHaveTodaysWorkoutChanged FINAL)
Q_PROPERTY(int currentMesoIdx READ currentMesoIdx WRITE setCurrentMesoIdx NOTIFY currentMesoIdxChanged FINAL)
Q_PROPERTY(homePageMesoModel* ownMesos READ ownMesos CONSTANT FINAL)
Q_PROPERTY(homePageMesoModel* clientMesos READ clientMesos CONSTANT FINAL)

Q_PROPERTY(QString mesoNameLabel READ mesoNameLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString startDateLabel READ startDateLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString endDateLabel READ endDateLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString notesLabel READ notesLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString nWeeksLabel READ nWeeksLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabel READ splitLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelA READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelB READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelC READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelD READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelE READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitLabelF READ splitLabelA NOTIFY labelChanged FINAL)
Q_PROPERTY(QString coachLabel READ coachLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString clientLabel READ clientLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString typeLabel READ typeLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString realMesoLabel READ realMesoLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString nonMesoLabel READ nonMesoLabel NOTIFY labelChanged FINAL)
Q_PROPERTY(QString splitR READ splitR NOTIFY labelChanged FINAL)

public:
	explicit DBMesocyclesModel(QObject *parent = nullptr, const bool bMainAppModel = true);
	~DBMesocyclesModel();

	inline uint fieldCount() const { return MESOCYCLES_TOTAL_COLS; }
	inline uint count() const { return m_mesoData.count(); }
	QMLMesoInterface *mesoManager(const uint meso_idx);
	DBExercisesModel *splitModel(const uint meso_idx, const QChar &split_letter, const bool auto_load = true);

	Q_INVOKABLE void getMesocyclePage(const uint meso_idx);
	Q_INVOKABLE uint startNewMesocycle(const bool bCreatePage, const std::optional<bool> bOwnMeso = std::nullopt);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void getExercisesPlannerPage(const uint meso_idx);
	Q_INVOKABLE void getMesoCalendarPage(const uint meso_idx);
	Q_INVOKABLE inline void todaysWorkout() { openSpecificWorkout(mostRecentOwnMesoIdx(), QDate::currentDate()); }
	void openSpecificWorkout(const uint meso_idx, const QDate &date);

	const uint newMesocycle(QStringList &&infolist);
	inline DBMesoCalendarManager *mesoCalendarManager() const { return m_calendarModel; }

	inline homePageMesoModel *currentHomePageMesoModel() { return m_curMesos; }
	Q_INVOKABLE inline homePageMesoModel *ownMesos() const { return m_ownMesos; }
	Q_INVOKABLE inline homePageMesoModel *clientMesos() const { return m_clientMesos; }

	inline bool isNewMeso(const uint meso_idx) const { return m_isNewMeso.at(meso_idx) != 0; }
	Q_INVOKABLE inline bool isNewMeso() const { return currentMesoIdx() >= 0 ? isNewMeso(currentMesoIdx()) : true; }

	[[nodiscard]] inline bool canHaveTodaysWorkout() const { return m_bCanHaveTodaysWorkout; }
	void changeCanHaveTodaysWorkout(const uint meso_idx);

	Q_INVOKABLE inline void setCurrentlyViewedMeso(const uint meso_idx, const bool bEmitSignal = true)
	{
		m_curMesos = isOwnMeso(meso_idx) ? m_ownMesos : m_clientMesos;
		checkIfCanExport(meso_idx, bEmitSignal);
	}

	void setModified(const uint meso_idx, const uint field);

	int idxFromId(const uint meso_id) const;
	inline const QString &id(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_ID);
	}
	inline const int _id(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_ID).toInt();
	}
	inline void setId(const uint meso_idx, const QString &new_id)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_ID] = new_id;
	}

	inline const QString &name(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_NAME);
	}
	void setName(const uint meso_idx, const QString &new_name);

	inline const QString &strStartDate(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_STARTDATE);
	}
	Q_INVOKABLE inline QDate startDate(const int meso_idx) const
	{
		return meso_idx >= 0 ? QDate::fromJulianDay(m_mesoData.at(meso_idx).at(MESOCYCLES_COL_STARTDATE).toLongLong()) : QDate();
	}
	void setStartDate(const uint meso_idx, const QDate &new_date);

	inline const QString &strEndDate(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_ENDDATE);
	}
	Q_INVOKABLE inline QDate endDate(const int meso_idx) const
	{
		return meso_idx >= 0 ? isRealMeso(meso_idx) ?
			QDate::fromJulianDay(m_mesoData.at(meso_idx).at(MESOCYCLES_COL_ENDDATE).toLongLong()) : QDate::currentDate().addDays(730) : QDate();
	}
	void setEndDate(const uint meso_idx, const QDate &new_date);

	inline const QString &notes(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_NOTE);
	}
	inline void setNotes(const uint meso_idx, const QString &new_notes)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_NOTE] = new_notes;
		setModified(meso_idx, MESOCYCLES_COL_NOTE);
	}

	inline const QString &nWeeks(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_WEEKS);
	}
	inline void setWeeks(const uint meso_idx, const QString &new_weeks)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_WEEKS] = new_weeks;
		setModified(meso_idx, MESOCYCLES_COL_WEEKS);
	}

	inline const QString &split(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLIT);
	}
	void setSplit(const uint meso_idx, const QString &new_split);

	inline const QString &splitA(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITA);
	}
	inline void setSplitA(const uint meso_idx, const QString &new_splitA)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITA] = new_splitA;
		setModified(meso_idx, MESOCYCLES_COL_SPLITA);
	}

	inline const QString &splitB(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITB);
	}
	inline void setSplitB(const uint meso_idx, const QString &new_splitB)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITB] = new_splitB;
		setModified(meso_idx, MESOCYCLES_COL_SPLITB);
	}

	inline const QString &splitC(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITC);
	}
	inline void setSplitC(const uint meso_idx, const QString &new_splitC)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITC] = new_splitC;
		setModified(meso_idx, MESOCYCLES_COL_SPLITC);
	}

	inline const QString &splitD(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITD);
	}
	inline void setSplitD(const uint meso_idx, const QString &new_splitD)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITD] = new_splitD;
		setModified(meso_idx, MESOCYCLES_COL_SPLITD);
	}

	inline const QString &splitE(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITE);
	}
	inline void setSplitE(const uint meso_idx, const QString &new_splitE)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITE] = new_splitE;
		setModified(meso_idx, MESOCYCLES_COL_SPLITE);
	}

	inline const QString &splitF(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_SPLITF);
	}
	inline void setSplitF(const uint meso_idx, const QString &new_splitF)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_SPLITF] = new_splitF;
		setModified(meso_idx, MESOCYCLES_COL_SPLITF);
	}
	inline const QString splitR() const
	{
		return std::move(tr("Rest day"));
	}

	Q_INVOKABLE QString muscularGroup(const uint meso_idx, const QChar &splitLetter) const;
	void setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue);

	inline const QString &coach(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_COACH);
	}
	void setCoach(const uint meso_idx, const QString &new_coach);

	inline const QString &client(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_CLIENT);
	}
	void setClient(const uint meso_idx, const QString &new_client);
	Q_INVOKABLE QString mesoClient(const uint meso_idx) const { return meso_idx < m_mesoData.count() ? client(meso_idx) : QString {}; }

	Q_INVOKABLE std::optional<bool> isOwnMeso(const int meso_idx) const;
	void setOwnMeso(const uint meso_idx);

	inline const QString &file(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_FILE);
	}
	inline void setFile(const uint meso_idx, const QString &new_file)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_FILE] = new_file;
		setModified(meso_idx, MESOCYCLES_COL_FILE);
	}

	inline const QString &type(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_TYPE);
	}
	inline void setType(const uint meso_idx, const QString &new_type)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_TYPE] = new_type;
		setModified(meso_idx, MESOCYCLES_COL_TYPE);
	}

	inline const QString &realMeso(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESOCYCLES_COL_REALMESO);
	}
	inline bool isRealMeso(const int meso_idx) const
	{
		return meso_idx >= 0 ? realMeso(meso_idx) == '1' : false;
	}
	inline void setIsRealMeso(const uint meso_idx, const bool bRealMeso)
	{
		m_mesoData[meso_idx][MESOCYCLES_COL_REALMESO] = bRealMeso ? '1' : '0';
		setModified(meso_idx, MESOCYCLES_COL_REALMESO);
	}

	inline const QString mesoNameLabel() const { return tr("Program's name: "); }
	inline const QString startDateLabel() const { return tr("Start date: "); }
	inline const QString endDateLabel() const { return tr("End date: "); }
	inline const QString notesLabel() const { return tr("Program's considerations: "); }
	inline const QString nWeeksLabel() const { return tr("Number of weeks: "); }
	inline const QString splitLabel() const { return tr("Weekly training division: "); }
	inline const QString splitLabelA() const { return tr("Muscular groups for division A: "); }
	inline const QString splitLabelB() const { return tr("Muscular groups for division B: "); }
	inline const QString splitLabelC() const { return tr("Muscular groups for division C: "); }
	inline const QString splitLabelD() const { return tr("Muscular groups for division D: "); }
	inline const QString splitLabelE() const { return tr("Muscular groups for division E: "); }
	inline const QString splitLabelF() const { return tr("Muscular groups for division F: "); }
	inline const QString coachLabel() const { return tr("Coach/Trainer: "); }
	inline const QString clientLabel() const { return tr("Client: "); }
	inline const QString typeLabel() const { return tr("Type: "); }
	inline const QString realMesoLabel() const { return tr("Mesocycle-style program: "); }
	inline const QString nonMesoLabel() const { return tr("Mesocycle-style program: "); }

	inline uint newMesoFieldCounter(const uint meso_idx) const { return m_newMesoFieldCounter.at(meso_idx); }
	inline void setNewMesoFieldCounter(const uint meso_idx, const uint field_counter) { m_newMesoFieldCounter[meso_idx] = field_counter; }

	inline QString splitLetter(const uint meso_idx, const uint day_of_week) const
	{
		return day_of_week <= 6 ? split(meso_idx).at(day_of_week) : QString{};
	}

	inline int currentMesoIdx() const { return m_currentMesoIdx; }
	void setCurrentMesoIdx(const int meso_idx, const bool bEmitSignal = true);
	inline int mostRecentOwnMesoIdx() const { return m_mostRecentOwnMesoIdx; }
	Q_INVOKABLE inline QString usedSplits(const uint meso_idx) const { return m_usedSplits.at(meso_idx); }
	void makeUsedSplits(const uint meso_idx);

	inline bool newMesoCalendarChanged(const uint meso_idx) const { return m_newMesoCalendarChanged.at(meso_idx); }
	inline void setNewMesoCalendarChanged(const uint meso_idx, const bool changed) { m_newMesoCalendarChanged[meso_idx] = changed; }
	bool isDateWithinMeso(const int meso_idx, const QDate &date) const;
	bool mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const;
	void findNextOwnMeso();
	int getPreviousMesoId(const QString &userid, const int current_mesoid) const;
	QDate getMesoMinimumStartDate(const QString &userid, const uint exclude_idx) const;
	QDate getMesoMaximumEndDate(const QString &userid, const uint exclude_idx) const;

	[[nodiscard]] inline bool canExport(const uint meso_idx) const { return meso_idx < m_canExport.count() ? m_canExport.at(meso_idx) : false; }
	void checkIfCanExport(const uint meso_idx, const bool bEmitSignal = true);

	//When importing a complete program: importIdx() will be set to -1 because we will be getting a new meso model. When other parts of the code
	//check importIdx() and get a -1, they will act in accordance with whole program import. After the meso model has been succesfully imported
	//and incorporated into the database and appMesoModel(), any other model that depends on a meso_idx can query mesoIdx() which will now reflect
	//the recently added meso
	inline int importIdx() const { return m_importMesoIdx; }
	inline void setImportIdx(const int new_import_idx) { m_importMesoIdx = new_import_idx; }
	int exportToFile(const uint meso_idx, const QString &filename) const;
	int exportToFormattedFile(const uint meso_idx, const QString &filename) const;
	int importFromFile(const uint meso_idx, const QString &filename);
	int importFromFormattedFile(const uint meso_idx, const QString &filename);
	//bool updateFromModel(const uint meso_idx, TPListModel *model);

	inline bool isFieldFormatSpecial (const uint field) const
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

	QString formatFieldToExport(const uint field, const QString &fieldValue) const;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;

	QString mesoFileName(const uint meso_idx) const;
	void removeMesoFile(const uint meso_idx);
	Q_INVOKABLE void sendMesoToUser(const uint meso_idx);
	int newMesoFromFile(const QString &filename, const std::optional<bool> &file_formatted = std::nullopt);
	int importSplitFromFile(const QString &filename, const uint meso_idx, uint split,
									const std::optional<bool> &file_formatted = std::nullopt);
	void viewOnlineMeso(const QString &coach, const QString &mesoFileName);
	void scanTemporaryMesocycles();

signals:
	void deferredActionFinished(const uint action_id, const int action_result);
	void mesoIdxChanged(const uint old_meso_idx, const uint new_meso_idx);
	void labelChanged();
	void isNewMesoChanged(const uint meso_idx, const uint = 9999); //2nd parameter only need by TPWorkoutsCalendar
	void canExportChanged(const uint meso_idx, const bool can_export);
	void newMesoFieldCounterChanged(const uint meso_idx, const uint field);
	void mesoChanged(const uint meso_idx, const uint field);
	void mesoCalendarFieldsChanged(const uint meso_idx, const uint field);
	void muscularGroupChanged(const uint meso_idx, const uint splitIndex, const QChar &splitLetter);
	void mostRecentOwnMesoChanged(const int meso_idx);
	void currentMesoIdxChanged();
	void canHaveTodaysWorkoutChanged();
	void todaysWorkoutFinished();
	void usedSplitsChanged(const uint meso_idx);

private:	
	QList<QStringList> m_mesoData;
	QList<QMLMesoInterface*> m_mesoManagerList;
	QList<QMap<QChar,DBExercisesModel*>> m_splitModels;
	DBMesoCalendarManager* m_calendarModel;
	homePageMesoModel *m_curMesos, *m_ownMesos, *m_clientMesos;
	QList<short> m_isNewMeso;
	QList<short> m_newMesoFieldCounter;
	QList<bool> m_newMesoCalendarChanged;
	QList<bool> m_canExport;
	QStringList m_usedSplits;
	int m_currentMesoIdx, m_mostRecentOwnMesoIdx, m_importMesoIdx, m_lowestTempMesoId;
	bool m_bCanHaveTodaysWorkout;

	static DBMesocyclesModel *app_meso_model;
	friend DBMesocyclesModel *appMesoModel();

	inline QString newMesoTemporaryId() { return QString::number(m_lowestTempMesoId--); }
	void loadSplits(const uint meso_idx, const uint thread_id);
	int continueExport(const uint meso_idx, const QString &filename, const bool formatted) const;
	int exportToFile_splitData(const uint meso_idx, QFile *mesoFile, const bool formatted) const;

signals:
	void internalSignal(const uint _meso_idx, const uint _id, const bool _result);
};

inline DBMesocyclesModel *appMesoModel() { return DBMesocyclesModel::app_meso_model; }
