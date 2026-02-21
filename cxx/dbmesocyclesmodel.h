#pragma once

#include "dbmodelinterface.h"
//Must include the header files of properties of custom types
#include "homepagemesomodel.h"

#include <QObject>

enum MesoFields
{
	MESO_FIELD_ID,
	MESO_FIELD_NAME,
	MESO_FIELD_STARTDATE,
	MESO_FIELD_ENDDATE,
	MESO_FIELD_NOTE,
	MESO_FIELD_WEEKS,
	MESO_FIELD_SPLIT,
	MESO_FIELD_SPLITA,
	MESO_FIELD_SPLITB,
	MESO_FIELD_SPLITC,
	MESO_FIELD_SPLITD,
	MESO_FIELD_SPLITE,
	MESO_FIELD_SPLITF,
	MESO_FIELD_COACH,
	MESO_FIELD_CLIENT,
	MESO_FIELD_FILE,
	MESO_FIELD_TYPE,
	MESO_FIELD_REALMESO,
	MESO_TOTAL_FIELDS
};

QT_FORWARD_DECLARE_CLASS(DBCalendarModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarTable)
QT_FORWARD_DECLARE_CLASS(DBMesocyclesTable)
QT_FORWARD_DECLARE_CLASS(DBModelInterfaceMesocycle)
QT_FORWARD_DECLARE_CLASS(DBWorkoutsOrSplitsTable)
QT_FORWARD_DECLARE_CLASS(QMLMesoInterface)
QT_FORWARD_DECLARE_CLASS(QFile)

using DBSplitModel = DBExercisesModel;

constexpr QLatin1StringView mesos_subdir{"mesocycles/"};

class DBMesocyclesModel : public QObject
{

Q_OBJECT

Q_PROPERTY(HomePageMesoModel* ownMesos READ ownMesos CONSTANT FINAL)
Q_PROPERTY(HomePageMesoModel* clientMesos READ clientMesos CONSTANT FINAL)
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

	static constexpr uint8_t MESO_N_REQUIRED_FIELDS{4};
	static constexpr uint8_t meso_required_fields[MESO_N_REQUIRED_FIELDS]
					{MESO_FIELD_NAME, MESO_FIELD_STARTDATE, MESO_FIELD_ENDDATE, MESO_FIELD_SPLIT};

	explicit DBMesocyclesModel(QObject *parent = nullptr);

	inline uint fieldCount() const { return MESO_TOTAL_FIELDS; }
	inline uint count() const { return m_mesoData.count(); }
	QMLMesoInterface *mesoManager(const uint meso_idx);
	void removeMesoManager(const uint meso_idx);
	void incorporateMeso(const uint meso_idx);

	Q_INVOKABLE void startNewMesocycle(const bool own_meso);

	Q_INVOKABLE void getMesocyclePage(const uint meso_idx, const bool new_meso);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void getExercisesPlannerPage(const uint meso_idx);
	Q_INVOKABLE void getMesoCalendarPage(const uint meso_idx);
	Q_INVOKABLE inline void startTodaysWorkout(const uint meso_idx) { openSpecificWorkout(meso_idx, QDate::currentDate()); }
	void openSpecificWorkout(const uint meso_idx, const QDate &date);

	inline HomePageMesoModel *ownMesos() const { return m_ownMesos; }
	inline HomePageMesoModel *clientMesos() const { return m_clientMesos; }
	void setCurrentMesosView(const bool own_mesos_view);
	inline int currentWorkingMeso() const { return m_currentWorkingMeso; }

	inline bool isMesoOK(const uint meso_idx) const { return meso_idx <= m_isMesoOK.count() ? m_isMesoOK.at(meso_idx) == 0 : false; }
	bool isRequiredFieldWrong(const uint meso_idx, const uint field) const;
	void setModified(const uint meso_idx, const uint field);

	int idxFromId(const QString &meso_id) const;
	inline const QString &id(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_ID);
	}
	inline const int _id(const uint meso_idx) const
	{
		return meso_idx < m_mesoData.count() ? m_mesoData.at(meso_idx).at(MESO_FIELD_ID).toInt() : -1;
	}
	inline void setId(const uint meso_idx, const QString &new_id)
	{
		m_mesoData[meso_idx][MESO_FIELD_ID] = new_id;
	}

	inline const QString &name(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_NAME);
	}
	Q_INVOKABLE inline QString name_QML(const uint meso_idx) const
	{
		return name(meso_idx);
	}
	void setName(const uint meso_idx, const QString &new_name);

	inline const QString &strStartDate(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_STARTDATE);
	}
	Q_INVOKABLE inline QDate startDate(const int meso_idx) const
	{
		return QDate::fromJulianDay(m_mesoData.at(meso_idx).at(MESO_FIELD_STARTDATE).toULong());
	}
	void setStartDate(const uint meso_idx, const QDate &new_date);

	inline const QString &strEndDate(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_ENDDATE);
	}
	Q_INVOKABLE inline QDate endDate(const int meso_idx) const
	{
		return isRealMeso(meso_idx) ? QDate::fromJulianDay(m_mesoData.at(meso_idx).at(MESO_FIELD_ENDDATE).toULong()) :
				QDate::currentDate().addDays(730);
	}
	void setEndDate(const uint meso_idx, const QDate &new_date);

	inline const QString &notes(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_NOTE);
	}
	inline void setNotes(const uint meso_idx, const QString &new_notes)
	{
		m_mesoData[meso_idx][MESO_FIELD_NOTE] = new_notes;
		setModified(meso_idx, MESO_FIELD_NOTE);
	}

	inline const QString &nWeeks(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_WEEKS);
	}
	//Don't call setModified(). After a call to setWeeks() invariably will follow a call to setStartDate() or setEndDate()
	//which will call setModified() themselves
	inline void setWeeks(const uint meso_idx, const QString &new_weeks)
	{
		m_mesoData[meso_idx][MESO_FIELD_WEEKS] = new_weeks;
	}

	inline const QString &split(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLIT);
	}
	void setSplit(const uint meso_idx, const QString &new_split);

	inline const QString &splitA(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITA);
	}
	inline void setSplitA(const uint meso_idx, const QString &new_splitA)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITA] = new_splitA;
		setModified(meso_idx, MESO_FIELD_SPLITA);
	}

	inline const QString &splitB(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITB);
	}
	inline void setSplitB(const uint meso_idx, const QString &new_splitB)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITB] = new_splitB;
		setModified(meso_idx, MESO_FIELD_SPLITB);
	}

	inline const QString &splitC(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITC);
	}
	inline void setSplitC(const uint meso_idx, const QString &new_splitC)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITC] = new_splitC;
		setModified(meso_idx, MESO_FIELD_SPLITC);
	}

	inline const QString &splitD(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITD);
	}
	inline void setSplitD(const uint meso_idx, const QString &new_splitD)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITD] = new_splitD;
		setModified(meso_idx, MESO_FIELD_SPLITD);
	}

	inline const QString &splitE(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITE);
	}
	inline void setSplitE(const uint meso_idx, const QString &new_splitE)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITE] = new_splitE;
		setModified(meso_idx, MESO_FIELD_SPLITE);
	}

	inline const QString &splitF(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_SPLITF);
	}
	inline void setSplitF(const uint meso_idx, const QString &new_splitF)
	{
		m_mesoData[meso_idx][MESO_FIELD_SPLITF] = new_splitF;
		setModified(meso_idx, MESO_FIELD_SPLITF);
	}
	inline const QString splitR() const
	{
		return std::move(tr("Rest day"));
	}

	Q_INVOKABLE QString muscularGroup(const uint meso_idx, const QChar &splitLetter) const;
	void setMuscularGroup(const uint meso_idx, const QChar &splitLetter, const QString &newSplitValue);

	inline const QString &coach(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_COACH);
	}
	void setCoach(const uint meso_idx, const QString &new_coach);

	inline const QString &client(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_CLIENT);
	}
	void setClient(const uint meso_idx, const QString &new_client);
	Q_INVOKABLE QString mesoClient(const uint meso_idx) const { return meso_idx < m_mesoData.count() ? client(meso_idx) : QString {}; }

	Q_INVOKABLE bool isOwnMeso(const uint meso_idx) const;
	void addSubMesoModel(const uint meso_idx, const bool own_meso);

	inline const QString &file(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_FILE);
	}
	inline void setFile(const uint meso_idx, const QString &new_file)
	{
		m_mesoData[meso_idx][MESO_FIELD_FILE] = new_file;
		setModified(meso_idx, MESO_FIELD_FILE);
	}

	inline const QString &type(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_TYPE);
	}
	inline void setType(const uint meso_idx, const QString &new_type)
	{
		m_mesoData[meso_idx][MESO_FIELD_TYPE] = new_type;
		setModified(meso_idx, MESO_FIELD_TYPE);
	}

	inline const QString &realMeso(const uint meso_idx) const
	{
		return m_mesoData.at(meso_idx).at(MESO_FIELD_REALMESO);
	}
	inline bool isRealMeso(const int meso_idx) const
	{
		return meso_idx >= 0 ? realMeso(meso_idx) == '1' : false;
	}
	inline void setIsRealMeso(const uint meso_idx, const bool bRealMeso)
	{
		m_mesoData[meso_idx][MESO_FIELD_REALMESO] = bRealMeso ? '1' : '0';
		setModified(meso_idx, MESO_FIELD_REALMESO);
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

	inline QString splitLetter(const uint meso_idx, const uint day_of_week) const
	{
		return day_of_week <= 6 ? split(meso_idx).at(day_of_week) : QString{};
	}

	Q_INVOKABLE inline QString usedSplits(const uint meso_idx) const { return m_usedSplits.at(meso_idx); }
	void removeSplitsForMeso(const uint meso_idx);
	void makeUsedSplits(const uint meso_idx);
	void loadSplits(const uint meso_idx);
	void loadSplit(const uint meso_idx, const QChar& splitletter);
	void removeSplit(const uint meso_idx, const QChar &split_letter);
	inline DBSplitModel *splitModel(const uint meso_idx, const QChar &split_letter) const
	{
		return m_splitModels.value(meso_idx).value(split_letter);
	}
	inline QMap<QChar,DBSplitModel*> splitModelsForMeso(const uint meso_idx) const { return m_splitModels.value(meso_idx); }

	bool isDateWithinMeso(const int meso_idx, const QDate &date) const;
	int mesoPlanExists(const QString &mesoName, const QString &coach, const QString &client) const;
	int getPreviousMesoId(const QString &userid, const int current_mesoid) const;
	QDate getMesoMinimumStartDate(const QString &userid, const uint exclude_idx) const;
	QDate getMesoMaximumEndDate(const QString &userid, const uint exclude_idx) const;

	void removeCalendarForMeso(const uint meso_idx, const bool remake_calendar);
	void getCalendarForMeso(const uint meso_idx);
	uint populateCalendarDays(const uint meso_idx);
	inline DBCalendarModel *calendar(const uint meso_idx) const { return m_calendars.value(meso_idx); }
	inline DBCalendarModel *workingCalendar() const { return m_workingCalendar; }
	void setWorkingCalendar(const uint meso_idx);
	inline DBExercisesModel *workingWorkout(const uint meso_idx) const { return m_workingWorkouts.value(meso_idx); }
	DBExercisesModel *workingWorkout() const;
	void setWorkingWorkout(const uint meso_idx, DBExercisesModel* model);
	DBExercisesModel *workoutForDay(const uint meso_idx, const int calendar_day);
	Q_INVOKABLE inline DBExercisesModel *workoutForDay(const uint meso_idx, const QDate &date)
	{
		return workoutForDay(meso_idx, static_cast<int>(startDate(meso_idx).daysTo(date)));
	}
	void newWorkoutFromFile(const QString &filename, const bool formatted, const QVariant &workout_info);

	[[nodiscard]] inline bool canExport(const uint meso_idx) const { return meso_idx < m_canExport.count() ? m_canExport.at(meso_idx) : false; }
	void checkIfCanExport(const uint meso_idx, const bool bEmitSignal = true);

	//When importing a complete program: importIdx() will be set to -1 because we will be getting a new meso model. When other parts of the code
	//check importIdx() and get a -1, they will act in accordance with whole program import. After the meso model has been succesfully imported
	//and incorporated, any other model that depends on a meso_idx can query mesoIdx() which will now reflect the recently added meso
	inline int importIdx() const { return m_importMesoIdx; }
	inline void setImportIdx(const int new_import_idx) { m_importMesoIdx = new_import_idx; }
	void exportToFile(const uint meso_idx, const QString &filename, const bool export_splits = true);
	void exportToFormattedFile(const uint meso_idx, const QString &filename);
	int importFromFile(const uint meso_idx, const QString &filename);
	int importFromFormattedFile(const uint meso_idx, const QString &filename);

	inline bool isFieldFormatSpecial (const uint field) const
	{
		switch (field)
		{
			case MESO_FIELD_STARTDATE:
			case MESO_FIELD_ENDDATE:
			case MESO_FIELD_COACH:
			case MESO_FIELD_CLIENT:
			case MESO_FIELD_REALMESO:
				return true;
			default: return false;
		}
	}

	QString formatFieldToExport(const uint field, const QString &fieldValue) const;
	QString formatFieldToImport(const uint field, const QString &fieldValue) const;

	QString mesoFileName(const uint meso_idx) const;
	void removeMesoFile(const uint meso_idx);
	Q_INVOKABLE void sendMesoToUser(const uint meso_idx);
	int newMesoFromFile(const QString &filename, const bool own_meso, const std::optional<bool> &file_formatted = std::nullopt);
	void viewOnlineMeso(const QString &coach, const QString &filename);
	void scanTemporaryMesocycles();

	inline bool isMesoNameOK(const int meso_idx = -1, const QString &meso_name = QString{}) const
	{
		switch (meso_name.length())
		{
			case 0: return meso_idx >= 0 ? (!name(meso_idx).isEmpty() ?
										mesoPlanExists(name(meso_idx), coach(meso_idx), client(meso_idx)) == meso_idx : false)
										: false;
			case 1: case 2: case 3: case 4: return false;
			default: return mesoPlanExists(meso_name, coach(meso_idx), client(meso_idx)) == -1;
		}

	}
	inline bool isStartDateOK(const int meso_idx = -1, const QDate &date = QDate{}) const
	{
		if (date.isValid())
			return date >= getMesoMinimumStartDate(client(meso_idx), 99999);
		else
			return meso_idx >= 0 ? !strStartDate(meso_idx).isEmpty() : false;
	}
	inline bool isEndDateOK(const int meso_idx = -1, const QDate &date = QDate{}) const
	{
		if (date.isValid())
			return date >= startDate(meso_idx).addDays(isRealMeso(meso_idx) ? 60 : 1);
		else
			return meso_idx >= 0 ? (isRealMeso(meso_idx) ? !strEndDate(meso_idx).isEmpty() : true) : false;
	}
	//static const QRegularExpression rgex{"(?=.*[ABCDEF])(?=.*[R])"_L1};
	//return rgex.match(strsplit).hasMatch()
	bool isSplitOK(const uint meso_idx, const QString &strsplit = QString{}) const
	{
		bool ok{false};
		const QString &_split{strsplit.isEmpty() ? split(meso_idx) : strsplit};
		for (const auto &splitletter : _split)
		{
			switch (splitletter.toLatin1())
			{
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					ok = !muscularGroup(meso_idx, splitletter).isEmpty();
				break;
				case 'R':
					ok = true;
				break;
				default:
					ok = false;
				break;
			}
			if (!ok)
				break;
		}
		return ok;
	}

signals:
	void mesoIdxChanged(const uint old_meso_idx, const uint new_meso_idx);
	void labelChanged();
	void canExportChanged(const uint meso_idx, const bool can_export);
	void mesoExported(const uint meso_idx, const QString& filename, const int return_code);
	void mesoChanged(const uint meso_idx, const uint field);
	void todaysWorkoutFinished();
	void usedSplitsChanged(const uint meso_idx);
	void splitLoaded(const uint meso_idx, const QChar &splitletter);
#ifndef QT_NO_DEBUG
	void mesoDataLoaded();
#endif

private:	
	QList<QStringList> m_mesoData;
	QHash<uint,QMLMesoInterface*> m_mesoManagerList;
	QHash<uint,QMap<uint,DBExercisesModel*>> m_workouts;
	QHash<uint,DBCalendarModel*> m_calendars;
	QMap<uint, QMap<QChar,DBSplitModel*>> m_splitModels;

	HomePageMesoModel *m_ownMesos, *m_clientMesos;
	QList<int8_t> m_isMesoOK;
	QList<bool> m_canExport;
	QStringList m_usedSplits;
	int m_importMesoIdx, m_currentWorkingMeso;

	DBModelInterfaceMesocycle *m_dbModelInterface;
	DBMesocyclesTable *m_db;
	DBMesoCalendarTable *m_calendarDB;
	DBWorkoutsOrSplitsTable *m_splitsDB, *m_workoutsDB;
	QHash<uint,DBExercisesModel*> m_workingWorkouts;
	DBCalendarModel* m_workingCalendar;

	friend class DBModelInterfaceMesocycle;

	inline bool isMesoTemporary(const uint meso_idx) const { return _id(meso_idx) < 0; }
	const uint newMesoData(QStringList &&infolist);
	void getAllMesocycles();
	void exportToFile_splitData(const uint meso_idx, QFile *meso_file, const bool formatted);

signals:
	void calendarReady(const uint meso_idx);
};

class DBModelInterfaceMesocycle : public DBModelInterface
{

public:
	explicit inline DBModelInterfaceMesocycle(DBMesocyclesModel *meso_model) : DBModelInterface{meso_model} {}
	inline const QList<QStringList> &modelData() const { return model<DBMesocyclesModel>()->m_mesoData; }
	inline QList<QStringList> &modelData() { return model<DBMesocyclesModel>()->m_mesoData; }
};
