#pragma once

#include "tpdatabasetable.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(DBExercisesListModel)
QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBUserModel)

//****THIS CLASS MUST NOT ALTER THE MODELS IT USES, UNLESS WHEN LOADING DATA FROM THE DATABASE*****

class DBInterface : public QObject
{

Q_OBJECT

public:
	explicit inline DBInterface()
		: QObject{nullptr} { app_db_interface = this; }
	inline DBInterface(const DBInterface& other) = delete;
	inline DBInterface &operator()(const DBInterface& other) = delete;
	inline ~DBInterface() {}

	void init();
	void sanityCheck();
	void threadFinished(TPDatabaseTable *dbObj);

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	void saveUser(const uint row);
	void removeUser(const uint row);
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
	void getMesoSplit(DBExercisesModel *model);
	void saveMesoSplit(DBExercisesModel *model);
	void removeMesoSplit(DBExercisesModel *model);
	void removeAllMesoSplits(const uint meso_idx);
	void deleteMesoSplitTable(const bool bRemoveFile);
	bool mesoHasAllSplitPlans(const uint meso_idx) const;
	bool mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter) const;
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	void getMesoCalendar(const uint meso_idx);
	void saveMesoCalendar(const uint meso_idx);
	void remakeMesoCalendar(const uint meso_idx);
	void removeMesoCalendar(const uint meso_idx);
	void deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile);
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------
	void getWorkout(DBExercisesModel *model);
	void saveWorkout(DBExercisesModel *model);
	void removeWorkout(DBExercisesModel *model);
	void removeAllWorkouts(const uint meso_idx);
	void deleteWorkoutsTable(const bool bRemoveFile);
	//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------
	void getExercisesForSplitWithinMeso(const uint meso_idx, const QChar &splitLetter);
	void completedDaysForSplitWithinTimePeriod(const QChar &splitLetter, const QDate &startDate, const QDate &endDate);
	void workoutsInfoForTimePeriod(const QStringList &exercises, const QList<QDate> &workoutDates);
	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------

signals:
	void databaseReady(const uint db_id);
	void databaseReadyWithData(const uint table_idx, const QVariant &data);

public slots:
	void cleanUpThreads();

private:
	struct workerLocks {
		inline TPDatabaseTable *nextObj() const { return dbObjs.at(++currentIndex); }
		inline TPDatabaseTable *at(const uint index) const { return dbObjs.at(index); }
		inline bool hasNext() const { return (currentIndex + 1) < dbObjs.count(); }
		inline bool canStartThread() const { return (dbObjs.count() == 1 || dbObjs.at(currentIndex)->resolved()); }
		inline uint count() const { return dbObjs.count(); }
		inline void appendObj(TPDatabaseTable *dbobj) { dbObjs.append(dbobj); }
		inline void removeAt(const uint index) { if (index < dbObjs.count()) { dbObjs.removeAt(index); if (currentIndex > 0) currentIndex--; } }
		bool hasID(const uint id) const {
			for (uint i{0}; i < dbObjs.count(); ++i)
			{
				if (dbObjs.at(i)->uniqueId() == id)
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

	void updateDB(TPDatabaseTable *worker);
	void createThread(TPDatabaseTable *worker, const std::function<void(void)> &execFunc);

	static DBInterface *app_db_interface;
	friend DBInterface *appDBInterface();
};

inline DBInterface *appDBInterface() { return DBInterface::app_db_interface; }
