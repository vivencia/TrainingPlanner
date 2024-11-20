#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include "tplistmodel.h"
#include "tpdatabasetable.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>
#include <QTimer>

class DBMesocyclesModel;
class DBExercisesModel;
class DBMesoSplitModel;
class DBMesoCalendarModel;
class DBTrainingDayModel;
class DBUserModel;
class RunCommands;
class QmlItemManager;

#ifdef Q_OS_ANDROID
class TPAndroidNotification;
#endif

//****THIS CLASS MUST NOT ALTER THE MODELS IT USES, UNLESS WHEN LOADING DATA FROM THE DATABASE*****

class DBInterface : public QObject
{

Q_OBJECT

public:
	explicit inline DBInterface()
		: QObject{nullptr}, mb_importMode(false) { app_db_interface = this; }
	inline DBInterface(const DBInterface& other) = delete;
	inline DBInterface& operator()(const DBInterface& other) = delete;
	inline ~DBInterface() {}

	inline const QString& dbFilesPath() const { return m_DBFilePath; }
	void init();
	void threadFinished(TPDatabaseTable* dbObj);

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	void saveUser(const uint row);
	void removeUser(const uint row, const bool bCoach);
	void deleteUserTable(const bool bRemoveFile);
	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	void getAllExercises();
	void saveExercises();
	void removeExercise(const uint row);
	void deleteExercisesTable(const bool bRemoveFile);
	void updateExercisesList();
	void getExercisesListVersion();
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	void getAllMesocycles();
	void createMesoManager(const uint meso_idx);
	void saveMesocycle(const uint meso_idx);
	void removeMesocycle(const uint meso_idx);
	void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	void saveMesoSplit(const uint meso_idx);
	void removeMesoSplit(const uint meso_idx);
	void deleteMesoSplitTable(const bool bRemoveFile);
	void loadCompleteMesoSplit(const uint meso_idx, const QChar& splitLetter);
	void loadAllSplits(const uint meso_idx);
	void saveMesoSplitComplete(DBMesoSplitModel* model);
	bool mesoHasPlan(const uint meso_id, const QString& splitLetter) const;
	void loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	void getMesoCalendar(const uint meso_idx);
	void changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore);
	void updateMesoCalendarModel(const uint meso_idx, const QDate& date, const QString& splitLetter);
	void updateMesoCalendarEntry(const uint meso_idx, const QDate& date, const QString& trainingDay, const QString& splitLetter);
	void setDayIsFinished(const uint meso_idx, const QDate& date, const bool bFinished);
	void removeMesoCalendar(const uint meso_idx);
	void deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile);
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	void getTrainingDay(DBTrainingDayModel* tDayModel);
	void getTrainingDayExercises(DBTrainingDayModel* tDayModel);
	void verifyTDayOptions(DBTrainingDayModel* tDayModel);
	void loadExercisesFromDate(const QString& strDate, DBTrainingDayModel* tDayModel);
	void loadExercisesFromMesoPlan(DBTrainingDayModel* tDayModel, DBMesoSplitModel* const splitModel);
	void convertTDayToPlan(const DBTrainingDayModel* const tDayModel, DBMesoSplitModel* const splitModel);
	void saveTrainingDay(DBTrainingDayModel* const tDayModel);
	void removeTrainingDay(const uint meso_idx);
	void deleteTrainingDayTable(const bool bRemoveFile);
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------
	void getExercisesForSplitWithinMeso(const uint meso_idx, const QChar& splitLetter);
	void completedDaysForSplitWithinTimePeriod(const QChar& splitLetter, const QDate& startDate, const QDate& endDate);
	void workoutsInfoForTimePeriod(const QStringList& exercises, const QList<QDate>& workoutDates);
	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------

signals:
	void databaseReady(const uint db_id);
	void databaseReadyWithData(const uint table_idx, const QVariant data);

public slots:
	void cleanUpThreads();

private:
	bool mb_importMode;
	QString m_DBFilePath;

	struct workerLocks {
		inline TPDatabaseTable* nextObj() const { return dbObjs.at(++currentIndex); }
		inline TPDatabaseTable* at(const uint index) const { return dbObjs.at(index); }
		inline bool hasNext() const { return (currentIndex + 1) < dbObjs.count(); }
		inline bool canStartThread() const { return (dbObjs.count() == 1 || dbObjs.at(currentIndex)->resolved()); }
		inline uint count() const { return dbObjs.count(); }
		inline void appendObj(TPDatabaseTable* dbobj) { dbObjs.append(dbobj); }
		inline void removeAt(const uint index) { if (index < dbObjs.count()) { dbObjs.removeAt(index); if (currentIndex > 0) currentIndex--; } }
		bool hasID(const uint id) const {
			for (uint i(0); i < dbObjs.count(); ++i)
			{
				if (dbObjs.at(i)->uniqueID() == id)
					return true;
			}
			return false;
		}
		workerLocks() : currentIndex(0) {}

		private:
			QList<TPDatabaseTable*> dbObjs;
			mutable uint currentIndex;
	};
	workerLocks m_WorkerLock[APP_TABLES_NUMBER+1];
	QTimer m_threadCleaner;
	QString m_exercisesListVersion;
	QMap<QChar,DBMesoSplitModel*> m_allSplits;

	void updateDB(TPDatabaseTable* worker);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);

	static DBInterface* app_db_interface;
	friend DBInterface* appDBInterface();
};

inline DBInterface* appDBInterface() { return DBInterface::app_db_interface; }
#endif // DBINTERFACE_H
